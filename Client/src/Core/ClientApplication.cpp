#include "ClientApplication.h"

#include "imgui-sfml.h"
#include "imgui.h"

#include "GameObjects/NetworkPlayer.h"
#include "GameObjects/Block.h"
#include "GameObjects/Projectile.h"

#include "MathUtils.h"
#include "Constants.h"


ClientApplication::ClientApplication()
	: m_Window(sf::VideoMode(static_cast<unsigned int >(WORLD_WIDTH), static_cast<unsigned int>(WORLD_HEIGHT)), "CMP303 Client"), m_Player(m_Window)
{
    m_Window.setVerticalSyncEnabled(true);

    ImGui::SFML::Init(m_Window);
    
    m_Player.setPosition(0.5f * m_Window.getSize().x, 0.5f * m_Window.getSize().y);

    m_RedBackground.setFillColor(LightRedTeamColor);
    m_BlueBackground.setFillColor(LightBlueTeamColor);

    ChangeTurfLine(0.5f * WORLD_WIDTH);

    m_GhostBlock = new Block(INVALID_BLOCK_ID, m_Player.GetTeam(), { 0, 0 });
    m_GhostBlock->setFillColor(GetGhostBlockColour(m_Player.GetTeam(), true));

    // setup network system
    m_NetworkSystem.Init(&m_Player, &m_NetworkPlayers, &m_Projectiles, &m_Blocks, &m_GameState, [this](float turfLine) { this->ChangeTurfLine(turfLine); });
}

ClientApplication::~ClientApplication()
{
    for (auto projectile : m_Projectiles)
        delete projectile;

    for (auto block : m_Blocks)
        delete block;

    if (m_GhostBlock) delete m_GhostBlock;

    ImGui::SFML::Shutdown();
}

void ClientApplication::Run()
{
    while (m_Window.isOpen())
    {
        sf::Time deltaTime = m_Clock.restart();
        float dt = deltaTime.asSeconds();
        
        m_UpdateFPSTimer += dt;
        if (m_UpdateFPSTimer > 1.0f)
        {
            m_FPS = 1.0f / dt;
            m_UpdateFPSTimer = 0.0f;
        }

        // input
        sf::Event event;
        while (m_Window.pollEvent(event))
        {
            if (!m_Window.hasFocus()) continue;

            ImGui::SFML::ProcessEvent(event);

            // check if imgui has captured the event
            ImGuiIO& io = ImGui::GetIO();
            if (io.WantCaptureMouse || io.WantCaptureKeyboard)
                continue;

            if (event.type == sf::Event::Closed)
                m_Window.close();
            else if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Button::Left)
                {
                    if (m_GameState == GameState::FightMode)
                        TryFireProjectile();
                }
            }
        }

        if (m_Window.hasFocus()) HandleInput(dt);


        // update
        ImGui::SFML::Update(m_Window, deltaTime);
        Update(dt);


        // render
        m_Window.clear(sf::Color::Magenta);
        Render();

        // gui
        GUI();
        ImGui::SFML::Render(m_Window);

        m_Window.display();
    }

}

void ClientApplication::HandleInput(float dt)
{
    // perform player movement
    sf::Vector2f movement = m_Player.CalculateMovement(dt);
    sf::Vector2f oldPos = m_Player.getPosition();
    sf::Vector2f newPos = oldPos + movement;
    m_Player.setPosition(newPos);
    m_Player.setRotation(0.0f); // temporarily remove rotation for collision detection

    // collision detection
    for (auto block : m_Blocks)
    {
        if (m_Player.getGlobalBounds().intersects(block->getGlobalBounds()))
        {
            // collision occurred: work out which direction
            sf::Vector2f dir = block->getPosition() - newPos;
            if (fabsf(dir.x) > fabsf(dir.y))
            {
                // horizontal collision
                if (dir.x > 0) // moving right
                    newPos.x = block->getPosition().x - 0.5f * (block->getSize().x + PLAYER_SIZE);
                else // moving left
                {
                    newPos.x = block->getPosition().x + 0.5f * (block->getSize().x + PLAYER_SIZE);
                }
            }
            else
            {
                // vertical collision
                if (dir.y > 0) // moving downwards
                    newPos.y = block->getPosition().y - 0.5f * (block->getSize().y + PLAYER_SIZE);
                else // moving upwards
                    newPos.y = block->getPosition().y + 0.5f * (block->getSize().y + PLAYER_SIZE);
            }
            m_Player.setPosition(newPos);
        }
    }

    if (m_GameState == GameState::BuildMode)
    {
        // dont allow the player to cross the middle in build mode
        if (m_Player.GetTeam() == PlayerTeam::Red)
        {
            if (newPos.x + 0.5f * PLAYER_SIZE > m_TurfLine)
            {
                newPos.x = m_TurfLine - 0.5f * PLAYER_SIZE;
                m_Player.setPosition(newPos);
            }
        }
        else
        {
            if (newPos.x - 0.5f * PLAYER_SIZE < m_TurfLine)
            {
                newPos.x = m_TurfLine + 0.5f * PLAYER_SIZE;
                m_Player.setPosition(newPos);
            }
        }
    }

    m_Player.UpdateRotation();


    // build mode
    if (m_GameState == GameState::BuildMode)
    {
        // update ghost block
        sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(m_Window));

        float s = m_GhostBlock->getSize().x;
        sf::Vector2f blockPos{ s * roundf(mousePos.x / s), s * roundf(mousePos.y / s) };

        m_GhostBlock->setPosition(blockPos);

        bool canPlace = CanPlaceBlock();

        // update colour
        m_GhostBlock->setFillColor(GetGhostBlockColour(m_Player.GetTeam(), !canPlace));

        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && canPlace)
            PlaceBlock();
    }
    // fight mode
    else if (m_GameState == GameState::FightMode)
    {
    }
}

void ClientApplication::Update(float dt)
{
    m_Indicator.setPosition(m_Player.getPosition());

    // update projectiles
    for (auto projectile : m_Projectiles)
        projectile->Update(dt);

    // network
    m_NetworkSystem.Update(dt);
}

void ClientApplication::Render()
{
    // fill with spawn colour
    m_Window.clear(DarkNoTeamColor);

    m_Window.draw(m_RedBackground);
    m_Window.draw(m_BlueBackground);

    for (auto block : m_Blocks)
        m_Window.draw(*block);

    for (auto projectile : m_Projectiles)
        m_Window.draw(*projectile);

    for (auto player : m_NetworkPlayers)
        m_Window.draw(*player);

    m_Window.draw(m_Player);
    m_Window.draw(m_Indicator);

    if (m_GameState == GameState::BuildMode)
        m_Window.draw(*m_GhostBlock);
}

void ClientApplication::GUI()
{
    ImGui::Text("Application:");
    ImGui::Text("FPS: %0.1f", m_FPS);

    ImGui::Separator();
    ImGui::Text("Network:");
    m_NetworkSystem.GUI();

    ImGui::Separator();
}


bool ClientApplication::CanPlaceBlock()
{
    if (Length(m_GhostBlock->getPosition() - m_Player.getPosition()) > BLOCK_PLACE_RADIUS) return false;
    if (!OnTeamTurf(m_GhostBlock->getPosition(), m_Player.GetTeam())) return false;
    if (m_GhostBlock->getGlobalBounds().intersects(m_Player.getGlobalBounds())) return false;

    for (auto block : m_Blocks)
    {
        if (block->getPosition() == m_GhostBlock->getPosition()) return false;
    }

    return true;
}

void ClientApplication::PlaceBlock()
{
    if (m_GhostBlock->getPosition() != m_LastPlaceLocation)
    {
        m_NetworkSystem.RequestPlaceBlock(m_GhostBlock->getPosition());
        m_LastPlaceLocation = m_GhostBlock->getPosition();
    }
}

void ClientApplication::TryFireProjectile()
{
    float dir = m_Player.getRotation();
    m_NetworkSystem.RequestShoot(m_Player.getPosition(), { cosf(DegToRad(dir)), sinf(DegToRad(dir)) });
}

void ClientApplication::ChangeTurfLine(float turfLine)
{
    m_TurfLine = turfLine;

    m_RedBackground.setSize(sf::Vector2f{ m_TurfLine - SPAWN_WIDTH, WORLD_HEIGHT });
    m_RedBackground.setPosition({ SPAWN_WIDTH, 0.0f });

    m_BlueBackground.setSize(sf::Vector2f{ WORLD_WIDTH - m_TurfLine - SPAWN_WIDTH, WORLD_HEIGHT });
    m_BlueBackground.setPosition({ m_TurfLine, 0.0f });
}

bool ClientApplication::OnTeamTurf(const sf::Vector2f& pos, PlayerTeam team) const
{
    switch (team)
    {
    case PlayerTeam::None: return false;
    case PlayerTeam::Red:  return pos.x < m_TurfLine && pos.x > SPAWN_WIDTH;
    case PlayerTeam::Blue: return pos.x > m_TurfLine && pos.x < WORLD_WIDTH - SPAWN_WIDTH;
    }
    return false;
}

