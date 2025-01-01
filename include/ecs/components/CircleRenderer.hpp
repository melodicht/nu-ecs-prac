#ifndef CIRCLERENDERER_HPP
#define CIRCLERENDERER_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include "Color.hpp"
#include "TransformComponent.hpp"
#include "ComponentReader.hpp"

struct CircleRenderer{
    // Caches transform
    TransformComponent* transform;

    // Color to render as
    Color renderColor{Color(0,0,0)};
};

class CircleRenderBehavior : public ComponentReader<CircleRenderer>{
    // Caches transform
    void start(CircleRenderer* givenComp, EntityID givenEnt) override;

    // Draws the given circle based off of transform and color
    void update(CircleRenderer* givenComp, EntityID givenEnt) override;
    
    // Passes itself into ECSManager
    CircleRenderBehavior();

    // Constructs an instance of the behvaior to pass into the ECSManager
    // static CircleRenderBehavior base;
};


#endif 