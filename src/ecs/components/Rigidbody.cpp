#include "Rigidbody.hpp"

void RigidBodyBehavior::start(Rigidbody* givenComp, EntityID givenEnt){
    givenComp->givenTransform = givenSceneView.pScene->Get<TransformComponent>(givenEnt);
}

void RigidBodyBehavior::update(Rigidbody* givenComp, EntityID givenEnt){
    givenComp->givenTransform->x_pos += givenComp->v_x * TimeManager::getDelta();
    givenComp->givenTransform->y_pos += givenComp->v_y * TimeManager::getDelta(); 
}

RigidBodyBehavior::RigidBodyBehavior() : ComponentReader<Rigidbody>(){
}

RigidBodyBehavior RigidBodyBehavior::base = RigidBodyBehavior();