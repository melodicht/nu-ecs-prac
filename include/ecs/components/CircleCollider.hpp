#ifndef CIRCLECOLLIDER_HPP
#define CIRCLECOLLIDER_HPP

#include "ComponentReader.hpp"

struct CircleCollider{
    float radius;
};

class CircleColliderBehavior : ComponentReader<CircleCollider, CircleColliderBehavior>
{
    
};

#endif
