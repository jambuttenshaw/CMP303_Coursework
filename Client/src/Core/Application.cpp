#include "Application.h"

#include "imgui-sfml.h"
#include "imgui.h"


Application::Application()
	: m_Window(sf::VideoMode(1200, 675), "CMP303 Client"), m_Player(m_Window)
{
	ImGui::SFML::Init(m_Window);

    m_Player.SetTeam(PlayerTeam::Blue);

    m_RedBackground.setFillColor(LightRedColor);
    m_RedBackground.setSize(sf::Vector2f{ m_Window.getSize().x * 0.5f, static_cast<float>(m_Window.getSize().y) });

    m_BlueBackground.setFillColor(LightBlueColor);
    m_BlueBackground.setSize(sf::Vector2f{ m_Window.getSize().x * 0.5f, static_cast<float>(m_Window.getSize().y) });
    m_BlueBackground.setPosition({ m_Window.getSize().x * 0.5f, 0 });
}

Application::~Application()
{
    ImGui::SFML::Shutdown();

}

void Application::Run()
{
    while (m_Window.isOpen())
    {
        sf::Time deltaTime = m_Clock.restart();

        // input
        sf::Event event;
        while (m_Window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed)
                m_Window.close();
        }

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

void Application::Update(float dt)
{
    m_Player.Update(dt);
}

void Application::Render()
{
    m_Window.draw(m_RedBackground);
    m_Window.draw(m_BlueBackground);

    m_Window.draw(m_Player);
}

void Application::GUI()
{
}
