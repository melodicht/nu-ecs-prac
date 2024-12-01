#include "CircleRenderer.hpp"

void CircleRenderBehavior::start(CircleRenderer* givenComp, EntityID givenEnt){
    givenComp->transform = givenSceneView.pScene->Get<TransformComponent>(givenEnt);
}

void CircleRenderBehavior::update(CircleRenderer* givenComp, EntityID givenEnt){
    std::cout << "Woaw" << std::endl;
    // Render to window
    sf::CircleShape shape(givenComp->transform->radius);
    shape.setFillColor(sf::Color(100, 250, 50));

    shape.setPosition(givenComp->transform->x_pos, givenComp->transform->y_pos);
    
    givenComp->window->draw(shape);
}

// Passes itself into ECSManager
CircleRenderBehavior::CircleRenderBehavior() : ComponentReader<CircleRenderer, CircleRenderBehavior>(){
}

CircleRenderBehavior CircleRenderBehavior::base = CircleRenderBehavior();