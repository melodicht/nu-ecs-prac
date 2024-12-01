#include "Rigidbody.hpp"

void RigidBodyBehavior::start(Rigidbody* givenComp, EntityID givenEnt){
    givenComp->givenTransform = givenSceneView.pScene->Get<TransformComponent>(givenEnt);
}

void RigidBodyBehavior::update(Rigidbody* givenComp, EntityID givenEnt){
    givenComp->givenTransform->x_pos += givenComp->v_x;
    givenComp->givenTransform->y_pos += givenComp->v_y;
}

RigidBodyBehavior::RigidBodyBehavior() : ComponentReader<Rigidbody, RigidBodyBehavior>(){
}

RigidBodyBehavior RigidBodyBehavior::base = RigidBodyBehavior();