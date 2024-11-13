#include <iostream>

// for the ball game we make with the ECS
#define BALL_RADIUS 10

typedef uint16_t EntityID;

class Component {}

//////////////// COMPONENTS ////////////////
// Represents a point in a 2D space as a Component
class TransformComponent : Component {
    float x_pos;
    float y_pos;
}
class BallPhysics : Component {
    
}
// A Scene is a collection of Entities and Components
class Scene {
    // an Entity Entry that describes the ID and Components it uses
    struct EntityEntry {
        EntityID id;
        std::vector<Component*> components;
    }

    std::vector<EntityEntry> entities;
    std::vector<Component> component;
    EntityID next_id = 0;


    // Creates a new Entity in this scene and 
    EntityID NewEntity() {
        entities.push_back({ next_id, ComponentMask() });
        next_id++;
        return entities.back().id;
    }

    // GetComponent : Gets a component, under the assumption that a object cannot
    //                have more than one component of the same type.

    // Creates a Component from the given typename and assigns it to an Entity
    // identified by the given EntityID. Typename MUST be a Component
    template<typename T>
    Component* AssignNewComponent(EntityID id) {
        T* new_comp = new T();
        if (!std::is_base_of_v<Component, T>) {
            std::cout << "BAD BAD BAD BAD\n";
        }
        components.push_back(&new_comp);
        entities[id].components.push_back(component);
        return new_comp;
    }
}



int main() {
    Scene* my_scene = new Scene();
    EntityID my_entity = my_scene->NewEntity();
    TransformComponent* transform = my_scene->AssignNewComponent<TransformComponent>(my_entity);
    // I'm not smart enough to know if this design sucks :3
    transform = MoveEntity(transform);
}