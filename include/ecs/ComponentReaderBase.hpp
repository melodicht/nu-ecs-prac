#ifndef COMPONENTREADERBASE_HPP
#define COMPONENTREADERBASE_HPP

#include "Scene.hpp"
#include "SceneView.hpp"

// This acts as an interface for the behavior a Component
// This allows for this class to be passed into the ECSManager regardless of type of component being searched for
// and prevents circular dependency
// It automatically passes itself into to the single existing ECSManager
class ComponentReaderBase{
    public:
        ComponentReaderBase();

        // Sets a given scene as the scene to iterate over on update
        virtual void InsertScene(Scene* setScene) = 0;

        // Uses the implemented behavior on every component
        virtual void fullStart() = 0;

        // Uses the implemented behavior on every component
        virtual void fullUpdate() = 0;
};

#endif