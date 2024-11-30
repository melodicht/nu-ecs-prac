#ifndef COMPONENTREADER_HPP
#define COMPONENTREADER_HPP

#include "Scene.hpp"
#include "SceneView.hpp"
#include "ComponentReaderBase.hpp"
#include "ECSManager.hpp"
// This allows for an easier implementation of behaviors into the ECS as behaviors are
// plugged in just by defining the correct types
// The ComponentType is the component it processes
// The BaseType is the class of the  
template<typename ComponentType, typename BaseType>
class ComponentReader : public ComponentReaderBase{
    public:
        ComponentReader() : ComponentReaderBase(){
            ECSManager::insertReader(this);
        }

        void InsertScene(Scene* setScene){
            givenScene = setScene;
        }

        // The behavior that a component initiates when the ECSManager actually starts the game
        virtual void start(ComponentType givenComp);
        // Uses the implemented behavior on every component
        void fullStart() override{
            for (EntityID ent : SceneView<ComponentType>(givenScene))
            {
                start(givenScene->Scene::Get<ComponentType>(ent));
            }
        }

        // The behavior that a component initiates every frame
        virtual void update(ComponentType givenComp);
        // Uses the implemented behavior on every component
        void fullUpdate() override {
            for (EntityID ent : SceneView<ComponentType>(givenScene))
            {
                update(givenScene->Scene::Get<ComponentType>(ent));
            }
        }
    protected:
        // Ensures that the component is properly plugged into the ECSManager
        static BaseType base;
        // The Scene to gleam relavant components to update from
        Scene* givenScene;
};
#endif