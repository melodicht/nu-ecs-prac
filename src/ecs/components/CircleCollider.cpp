#include "CircleCollider.hpp"

void CircleColliderBehavior::start(CircleCollider* givenComp, EntityID givenEnt) {
    givenComp->transform = givenSceneView.pScene->Get<TransformComponent>(givenEnt);
    givenComp->rigidbody = givenSceneView.pScene->Get<Rigidbody>(givenEnt);
}

void CircleColliderBehavior::update(CircleCollider* givenComp, EntityID givenEnt) {
  for (EntityID ent : givenSceneView)
  {
    CircleCollider* cc = givenSceneView.pScene->Get<CircleCollider>(ent);
    if(ent != givenEnt){
      double diffX = cc->transform->x_pos - givenComp->transform->x_pos;
      double diffY = cc->transform->y_pos - givenComp->transform->y_pos;
      double distance = sqrt(diffX * diffX + diffY * diffY);
      if(distance < cc->transform->radius + givenComp->transform->radius){
        double normX = diffX/distance;
        double normY = diffY/distance;
        double thisSpeedMag = -sqrt(givenComp->rigidbody->v_x * givenComp->rigidbody->v_x + givenComp->rigidbody->v_y * givenComp->rigidbody->v_y);
        givenComp->rigidbody->v_x = normX * thisSpeedMag;
        givenComp->rigidbody->v_y = normY * thisSpeedMag;
        double speedMag = sqrt(cc->rigidbody->v_x * cc->rigidbody->v_x + cc->rigidbody->v_y * cc->rigidbody->v_y);
        cc->rigidbody->v_x = normX * speedMag;
        cc->rigidbody->v_y = normY * speedMag;
      }
    }
  }

  // Collision check x-axis
  if ((givenComp->transform->x_pos - givenComp->transform->radius) < 0 || (givenComp->transform->x_pos + givenComp->transform->radius) > 800)
  {
    givenComp->rigidbody->v_x *= -1;
  }

  // Collision check y-axis
  if ((givenComp->transform->y_pos - givenComp->transform->radius) < 0 || (givenComp->transform->y_pos + givenComp->transform->radius) > 600)
  {
    givenComp->rigidbody->v_y *= -1;
  }

}

CircleColliderBehavior::CircleColliderBehavior() : ComponentReader<CircleCollider, CircleColliderBehavior>(){
}

CircleColliderBehavior CircleColliderBehavior::base = CircleColliderBehavior();