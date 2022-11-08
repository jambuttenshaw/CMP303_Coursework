#include <SFML/Graphics.hpp>
#include "imgui-sfml.h"
#include "imgui.h"

#include "Common.h"

int main()
{
    sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");

    ImGui::SFML::Init(window);

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    Player p;

    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Time deltaTime = clock.getElapsedTime();

        // input
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // update

        ImGui::SFML::Update(window, deltaTime);


        // render
        window.clear();
        window.draw(shape);
        window.draw(p.GetShape());

        // gui
        ImGui::Begin("Debug");
        ImGui::Text("Debug Info");
        ImGui::End();

        ImGui::SFML::Render(window);

        window.display();

    }

    ImGui::SFML::Shutdown();

    return 0;
}