#include "ClientApplication.h"

#include "imgui-sfml.h"
#include "imgui.h"

#include "GameObjects/Block.h"
#include "GameObjects/Projectile.h"

#include "MathUtils.h"
#include <iostream>

ClientApplication::ClientApplication()
	: m_Window(sf::VideoMode(1200, 675), "CMP303 Client"), m_Player(m_Window)
{
    m_Window.setVerticalSyncEnabled(true);

    ImGui::SFML::Init(m_Window);
    
    m_Player.setPosition(0.5f * m_Window.getSize().x, 0.5f * m_Window.getSize().y);

    m_TurfLine = m_Window.getSize().x * 0.5f;

    m_RedBackground.setFillColor(LightRedTeamColor);
    m_RedBackground.setSize(sf::Vector2f{ m_TurfLine, static_cast<float>(m_Window.getSize().y) });

    m_BlueBackground.setFillColor(LightBlueTeamColor);
    m_BlueBackground.setSize(sf::Vector2f{ m_TurfLine, static_cast<float>(m_Window.getSize().y) });
    m_BlueBackground.setPosition({ m_TurfLine, 0 });


    m_GhostBlock = new Block(m_Player.GetTeam(), { 0, 0 });
    m_GhostBlock->setFillColor(GetGhostBlockColour(m_Player.GetTeam(), true));

    // setup network system
    m_NetworkSystem.Init(&m_Player, &m_NetworkPlayers);
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

        if (m_Window.hasFocus()) HandleInput(deltaTime.asSeconds());


        // update
        ImGui::SFML::Update(m_Window, deltaTime);
        Update(deltaTime.asSeconds());


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
                    newPos.x = block->getPosition().x - 0.5f * (block->getSize().x + m_Player.GetDimensions().x);
                else // moving left
                {
                    newPos.x = block->getPosition().x + 0.5f * (block->getSize().x + m_Player.GetDimensions().x);
                }
            }
            else
            {
                // vertical collision
                if (dir.y > 0) // moving downwards
                    newPos.y = block->getPosition().y - 0.5f * (block->getSize().y + m_Player.GetDimensions().y);
                else // moving upwards
                    newPos.y = block->getPosition().y + 0.5f * (block->getSize().y + m_Player.GetDimensions().y);
            }
            m_Player.setPosition(newPos);
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
    // update projectiles
    for (auto it = m_Projectiles.begin(); it < m_Projectiles.end();)
    {
        Projectile* projectile = *it;
        
        projectile->Update(dt);
        
        bool hitBlock = false;
        for (auto block_it = m_Blocks.begin(); block_it < m_Blocks.end();)
        {
            Block* block = *block_it;
            if (block->getGlobalBounds().intersects(projectile->getGlobalBounds()))
            {
                if (block->GetTeam() != projectile->GetTeam())
                    m_Blocks.erase(block_it);
                hitBlock = true;
                break;
            }
            block_it++;
        }

        if (hitBlock || projectile->OffScreen(static_cast<sf::Vector2f>(m_Window.getSize())))
            it = m_Projectiles.erase(it);
        else
            it++;
    }

    // network
    m_NetworkSystem.Update(dt);

    for (auto& player : m_NetworkPlayers)
        player->Update(dt);
}

void ClientApplication::Render()
{
    m_Window.draw(m_RedBackground);
    m_Window.draw(m_BlueBackground);

    for (auto block : m_Blocks)
        m_Window.draw(*block);

    for (auto projectile : m_Projectiles)
        m_Window.draw(*projectile);

    for (auto player : m_NetworkPlayers)
        m_Window.draw(*player);

    m_Window.draw(m_Player);

    if (m_GameState == GameState::BuildMode)
        m_Window.draw(*m_GhostBlock);
}

void ClientApplication::GUI()
{
    if (m_NetworkSystem.Connected())
    {
        if (ImGui::Button("Disconnect")) m_NetworkSystem.Disconnect();
    }
    else
    {
        if (ImGui::Button("Connect")) m_NetworkSystem.Connect();
    }

    ImGui::Text("Connected Players: %d", m_NetworkPlayers.size());
}


bool ClientApplication::CanPlaceBlock()
{
    if (Length(m_GhostBlock->getPosition() - m_Player.getPosition()) > m_BlockPlaceRadius) return false;
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
    Block* block = new Block(m_Player.GetTeam(), m_GhostBlock->getPosition());
    m_Blocks.push_back(block);
}

void ClientApplication::TryFireProjectile()
{
    Projectile* projectile = new Projectile(m_Player.GetTeam(), m_Player.getPosition(), m_Player.getRotation());
    m_Projectiles.push_back(projectile);
}

bool ClientApplication::OnTeamTurf(const sf::Vector2f& pos, PlayerTeam team) const
{
    switch (team)
    {
    case PlayerTeam::None: return false;
    case PlayerTeam::Red:  return pos.x < m_TurfLine;
    case PlayerTeam::Blue: return pos.x > m_TurfLine;
    }
    return false;
}

