#include "ClientApplication.h"

#include "imgui-sfml.h"
#include "imgui.h"

#include "Network/NetworkPlayer.h"
#include "GameObjects/Block.h"
#include "GameObjects/Projectile.h"

#include "MathUtils.h"
#include "Constants.h"


ClientApplication::ClientApplication()
	: m_Window(sf::VideoMode(static_cast<unsigned int >(WORLD_WIDTH + 300), static_cast<unsigned int>(WORLD_HEIGHT)), "CMP303 Client"), m_Player(m_Window)
{
    m_Window.setVerticalSyncEnabled(true);

    ImGui::SFML::Init(m_Window);
    
    // setup game objects
    m_Player.setPosition(0.5f * m_Window.getSize().x, 0.5f * m_Window.getSize().y);

    m_RedBackground.setFillColor(LightRedTeamColor);
    m_BlueBackground.setFillColor(LightBlueTeamColor);

    m_GUIBackground.setFillColor({ 50, 50, 50 });
    m_GUIBackground.setPosition({ WORLD_WIDTH, 0.0f });
    m_GUIBackground.setSize({ m_GUIWidth, WORLD_HEIGHT });

    ChangeTurfLine(0.5f * WORLD_WIDTH);

    m_GhostBlock = new Block(INVALID_BLOCK_ID, m_Player.GetTeam(), { 0, 0 });
    m_GhostBlock->setFillColor(GetGhostBlockColour(m_Player.GetTeam(), true));

    // setup network system
    // the network system will interface with all of these objects
    m_NetworkSystem.Init(&m_Player, &m_NetworkPlayers, &m_Projectiles, &m_Blocks, &m_GameState, [this](float turfLine) { this->ChangeTurfLine(turfLine); }, &m_BuildModeBlocks, &m_Ammo);
}

ClientApplication::~ClientApplication()
{
    // cleanup
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
        // calculate fps
        sf::Time deltaTime = m_Clock.restart();
        float dt = deltaTime.asSeconds();
        
        m_UpdateFPSTimer += dt;
        if (m_UpdateFPSTimer > 1.0f)
        {
            m_FPS = 1.0f / dt;
            m_UpdateFPSTimer -= 1.0f;
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

            // process event
            if (event.type == sf::Event::Closed)
                m_Window.close();
            else if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Button::Left)
                    TryFireProjectile();
            }
        }

        // handle input
        HandleInput(dt);


        // update
        ImGui::SFML::Update(m_Window, deltaTime);
        Update(dt);


        // render
        m_Window.clear(sf::Color::Magenta);
        Render();

        // gui
        ImGui::SetNextWindowPos({ WORLD_WIDTH, 0 });
        ImGui::SetNextWindowSize({ m_GUIWidth, WORLD_HEIGHT });

        ImGui::Begin("GUI");
        GUI();
        ImGui::End();
        ImGui::SFML::Render(m_Window);

        m_Window.display();
    }

}

void ClientApplication::HandleInput(float dt)
{
    // perform player movement
    sf::Vector2f movement{ 0.0f, 0.0f };
    if (ControllablePlayer::AutomoveEnabled())
        movement = ControllablePlayer::Automove(dt);
    else if (m_Window.hasFocus())
        movement = m_Player.CalculateMovement(dt);

    // update position
    sf::Vector2f oldPos = m_Player.getPosition();
    sf::Vector2f newPos = oldPos + movement;
    m_Player.setPosition(newPos);

    float oldRot = m_Player.getRotation();
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

    // keep player in the world
    if (newPos.x + 0.5f * PLAYER_SIZE > WORLD_WIDTH)
        newPos.x = WORLD_WIDTH - 0.5f * PLAYER_SIZE;
    if (newPos.x + 0.5f * PLAYER_SIZE < 0.0f)
        newPos.x = 0.5f * PLAYER_SIZE;

    if (newPos.y + 0.5f * PLAYER_SIZE > WORLD_HEIGHT)
        newPos.y = WORLD_HEIGHT - 0.5f * PLAYER_SIZE;
    if (newPos.y - 0.5f * PLAYER_SIZE < 0.0f)
        newPos.y = 0.5f * PLAYER_SIZE;

    // dont allow the player to cross the middle in build mode
    if (m_GameState == GameState::BuildMode)
    {
        if (m_Player.GetTeam() == PlayerTeam::Red)
        {
            if (newPos.x + 0.5f * PLAYER_SIZE > m_TurfLine)
                newPos.x = m_TurfLine - 0.5f * PLAYER_SIZE;
        }
        else
        {
            if (newPos.x - 0.5f * PLAYER_SIZE < m_TurfLine)
                newPos.x = m_TurfLine + 0.5f * PLAYER_SIZE;
        }
    }
    m_Player.setPosition(newPos);

    if (m_Window.hasFocus())
    {
        // face the mouse
        m_Player.UpdateRotation();

        // build mode
        if (m_GameState == GameState::BuildMode)
        {
            // update ghost block
            sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(m_Window));

            float s = m_GhostBlock->getSize().x;
            sf::Vector2f blockPos{ s * roundf(mousePos.x / s), s * roundf(mousePos.y / s) };

            m_GhostBlock->setPosition(blockPos);

            // check if the ghost block is on a valid location
            bool canPlace = CanPlaceBlock();

            // update colour
            m_GhostBlock->setFillColor(GetGhostBlockColour(m_Player.GetTeam(), !canPlace));

            // try place block
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && canPlace)
                PlaceBlock();
        }
    }
    else m_Player.setRotation(oldRot);
}

void ClientApplication::Update(float dt)
{
    // update player indicator
    m_Indicator.setPosition(m_Player.getPosition());
    m_Indicator.Update();

    // simulate projectiles
    for (auto proj_it = m_Projectiles.begin(); proj_it != m_Projectiles.end();)
    {
        auto projectile = *proj_it;
        // movement update
        projectile->Update(dt);

        // check if the projectile has hit a block
        bool hitBlock = false;
        for (auto block_it = m_Blocks.begin(); block_it != m_Blocks.end(); block_it++)
        {
            // dont collide with local blocks (not that this should really ever happen anyway...)
            if ((*block_it)->GetID() == INVALID_BLOCK_ID) continue;
            
            if (projectile->getGlobalBounds().intersects((*block_it)->getGlobalBounds()))
            {
                hitBlock = true;
                break;
            }
        }

        auto p = projectile->getPosition();
        // also check if projectile is out of bounds
        if (hitBlock || p.x - PROJECTILE_RADIUS < 0 || p.x + PROJECTILE_RADIUS > WORLD_WIDTH
                     || p.y - PROJECTILE_RADIUS < 0 || p.y + PROJECTILE_RADIUS > WORLD_HEIGHT)
        {
            // delete projectile
            proj_it = m_Projectiles.erase(proj_it);
        }
        else
            proj_it++;
    }

    // reloading
    if (m_Ammo < MAX_AMMO_HELD)
    {
        m_ReloadTimer += dt;
        if (m_ReloadTimer > RELOAD_TIME)
        {
            m_Ammo++;
            m_ReloadTimer -= RELOAD_TIME;
        }
    }

    // network
    m_NetworkSystem.Update(dt);
}

void ClientApplication::Render()
{
    // fill with spawn colour
    m_Window.clear(DarkNoTeamColor);

    // draw backgrounds
    m_Window.draw(m_RedBackground);
    m_Window.draw(m_BlueBackground);
    m_Window.draw(m_GUIBackground);

    // draw blocks
    for (auto block : m_Blocks)
        m_Window.draw(*block);

    // draw projectiles
    for (auto projectile : m_Projectiles)
        m_Window.draw(*projectile);

    // draw other players 
    for (auto player : m_NetworkPlayers)
        m_Window.draw(*player);

    // draw player and indicator
    m_Window.draw(m_Player);
    m_Window.draw(m_Indicator);

    // draw ghost block in build mode
    if (m_GameState == GameState::BuildMode)
        m_Window.draw(*m_GhostBlock);
}

void ClientApplication::GUI()
{
    // gui
    ImGui::Text("Application:");
    ImGui::Text("FPS: %0.1f", m_FPS);

    ImGui::Separator();
    ImGui::Text("Network:");
    m_NetworkSystem.GUI();

    ImGui::Separator();
    ImGui::Text("Client Settings:");
    ControllablePlayer::SettingsGUI();
    ImGui::Text("Remote Settings:");
    NetworkPlayer::SettingsGUI();

    ImGui::Separator();
    ImGui::Text("HUD");
    if (m_GameState == GameState::BuildMode)
    {
        ImGui::Text("Blocks remaining: %d", m_BuildModeBlocks);
    }
    else if (m_GameState == GameState::FightMode)
    {
        ImGui::Text("Ammo remaining: %d", m_Ammo);
        ImGui::Text("Reload time: %.1f", RELOAD_TIME - m_ReloadTimer);
    }
}


bool ClientApplication::CanPlaceBlock()
{
    // check if block place is legal
    // the server will verify and tell us its verdict later
 
    // check if we still have blocks to place
    if (!m_BuildModeBlocks) return false;
    // dont allow to place too far away
    if (Length(m_GhostBlock->getPosition() - m_Player.getPosition()) > BLOCK_PLACE_RADIUS) return false;
    // only place blocks on your own turf
    if (!OnTeamTurf(m_GhostBlock->getPosition(), m_Player.GetTeam())) return false;
    // cant place blocks on top of the player
    if (m_GhostBlock->getGlobalBounds().intersects(m_Player.getGlobalBounds())) return false;
    // cant place blocks on top of other blocks
    for (auto block : m_Blocks)
    {
        if (block->getPosition() == m_GhostBlock->getPosition()) return false;
    }

    return true;
}

void ClientApplication::PlaceBlock()
{
    // dont repeatedly attempt to place blocks in the same place
    if (m_GhostBlock->getPosition() == m_LastPlaceLocation) return;

    // reqeuest from the server to place a block
    m_NetworkSystem.RequestPlaceBlock(m_GhostBlock->getPosition());
    m_LastPlaceLocation = m_GhostBlock->getPosition();
}

void ClientApplication::TryFireProjectile()
{
    // only attmept to fire projectiles when we have ammo and the game is in fight mode
    if (m_GameState != GameState::FightMode) return;
    if (!m_Ammo) return;

    // request to shoot a projectile from the server
    float dir = m_Player.getRotation();
    m_NetworkSystem.RequestShoot(m_Player.getPosition(), { cosf(DegToRad(dir)), sinf(DegToRad(dir)) });
}

void ClientApplication::ChangeTurfLine(float turfLine)
{
    // update the turf line and change the size of the backgrounds
    m_TurfLine = turfLine;

    m_RedBackground.setSize(sf::Vector2f{ m_TurfLine - SPAWN_WIDTH, WORLD_HEIGHT });
    m_RedBackground.setPosition({ SPAWN_WIDTH, 0.0f });

    m_BlueBackground.setSize(sf::Vector2f{ WORLD_WIDTH - m_TurfLine - SPAWN_WIDTH, WORLD_HEIGHT });
    m_BlueBackground.setPosition({ m_TurfLine, 0.0f });
}

bool ClientApplication::OnTeamTurf(const sf::Vector2f& pos, PlayerTeam team) const
{
    // check if a position is on a teams turf
    switch (team)
    {
    case PlayerTeam::None: return false;
    case PlayerTeam::Red:  return pos.x < m_TurfLine && pos.x > SPAWN_WIDTH;
    case PlayerTeam::Blue: return pos.x > m_TurfLine && pos.x < WORLD_WIDTH - SPAWN_WIDTH;
    }
    return false;
}

