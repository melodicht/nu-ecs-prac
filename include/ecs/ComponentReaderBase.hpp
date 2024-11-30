#ifndef COMPONENTREADERBASE_HPP
#define COMPONENTREADERBASE_HPP

#include "Scene.hpp"
#include "SceneView.hpp"
#include "ECSManager.hpp"

// This acts as an abstract base of the behavior a Component
// It automatically passes itself into to the single existing ECSManager
class ComponentReaderBase{
    public:
        ComponentReaderBase();

        // Sets a given scene as the scene to iterate over on update
        void InsertScene(Scene* setScene);

        // Uses the implemented behavior on every component
        virtual void fullStart() = 0;

        // Uses the implemented behavior on every component
        virtual void fullUpdate() = 0;

    protected:
        // The Scene to gleam relavant components to update from
        Scene* givenScene;
};

#endif