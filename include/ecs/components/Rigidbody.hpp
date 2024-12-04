#ifndef RIGIDBODY_HPP
#define RIGIDBODY_HPP

#include "ComponentReader.hpp"
#include "TransformComponent.hpp"

struct Rigidbody {
  // Cached transform to update position of
  TransformComponent* givenTransform{nullptr};
  // X and Y velocities
  float v_x{0.5};
  float v_y{0.5};
};

// 
class RigidBodyBehavior : public ComponentReader<Rigidbody>{
    // Caches transform
    void start(Rigidbody* givenComp, EntityID givenEnt) override;

    // Draws the given circle based off of transform and color
    void update(Rigidbody* givenComp, EntityID givenEnt) override;
    
    // Passes itself into ECSManager
    RigidBodyBehavior();

    // Constructs an instance of the behvaior to pass into the ECSManager
    static RigidBodyBehavior base;
};
#endif