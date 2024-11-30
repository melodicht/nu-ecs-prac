#ifndef CIRCLERENDERER_HPP
#define CIRCLERENDERER_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "TransformComponent.hpp"
#include "ComponentReader.hpp"

struct CircleRenderer{
    // Caches transform
    TransformComponent* transform;
    // The window this renderer draws from
    // A bit scuffed for now, I think a universal singleton window might be useful to counteract this need but as a proof of concept it will suffice
    sf::RenderWindow* window{nullptr};
    // Color to render as
    sf::Color renderColor{sf::Color::Green};
};

class CircleRenderBehavior : public ComponentReader<CircleRenderer, CircleRenderBehavior>{
    // Caches transform
    void start(CircleRenderer* givenComp, EntityID givenEnt) override;

    // Draws the given circle based off of transform and color
    void update(CircleRenderer* givenComp, EntityID givenEnt) override;
    
    // Passes itself into ECSManager
    CircleRenderBehavior();
};


#endif 