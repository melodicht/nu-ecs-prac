#ifndef COMPONENTREADER_HPP
#define COMPONENTREADER_HPP

// This acts as an abstract base of the behavior a Component
// It automatically passes itself into to the single existing ECSManager 
// Relys on ECS to call behaviors on corresponding type
template<typename ComponentType>
class ComponentReader{
    // The behavior that a component initiates on creation
    virtual void Start(ComponentType givenComp);

    // The behavior that a component initiates every frame
    virtual void Update(ComponentType givenComp);

    // The behavior that a component initiates when the given Component is going away
    virtual void Destroy(ComponentType givenComp);

};
#endif