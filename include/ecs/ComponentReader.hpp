#ifndef COMPONENTREADER_HPP
#define COMPONENTREADER_HPP

#include "Scene.hpp"
#include "SceneView.hpp"
#include "ComponentReaderBase.hpp"
#include "ECSManager.hpp"
// This allows for an easier implementation of behaviors into the ECS as behaviors are
// plugged in just by defining the correct types
// The ComponentType is the component it processes
// The BaseType is the class that inherits this, requires a empty constructor
template<typename ComponentType, typename BaseType>
class ComponentReader : public ComponentReaderBase{
    public:
        // Takes in the current scene from the ECSManager
        void insertScene(Scene& setScene) override{
            givenSceneView = SceneView<ComponentType>(setScene);
        }

        // Uses the implemented behavior on every component
        void fullStart() override{
            for (EntityID ent : givenSceneView)
            {
                start(givenSceneView.pScene->template Get<ComponentType>(ent), ent);
            }
        }

        // Uses the implemented behavior on every component
        void fullUpdate() override {
            for (EntityID ent : givenSceneView)
            {
                update(givenSceneView.pScene->template Get<ComponentType>(ent), ent);
            }
        }
    protected:
        // The behavior that a component initiates when the ECSManager actually starts the game
        virtual void start(ComponentType* givenComp, EntityID givenEnt) = 0;
        // The behavior that a component initiates every frame
        virtual void update(ComponentType* givenComp, EntityID givenEnt) = 0;

        // Plugs itself into the ECSManager
        ComponentReader() : ComponentReaderBase(), givenSceneView(ECSManager::getScene()){
            ECSManager::insertReader(this);
        }

        // Ensures that the component is properly plugged into the ECSManager
        static BaseType base;
        // The Scene to gleam relavant components to update from
        SceneView<ComponentType> givenSceneView;
};
#endif