#ifndef ECSMANAGER_HPP
#define ECSMANAGER_HPP

#include <vector>

#include "ComponentReaderBase.hpp"
#include "Scene.hpp"
#include "SceneView.hpp"

// Handles the regular updates/behaviors of the components within a given scene
// ONLY SUPPORTS A SINGLE INSTANCE
// TODO LATER: Scene functionality, currently only one scene is loaded and preseneted
class ECSManager{
    public:
        // Initializes the behaviors at the current game state
        // Can only happen a single time
        static void start();

        // Gets the scene that the current ECSManager is updating and providing behaviors for
        static Scene& getScene();

        // Inserts a reader that determines the behavior of a range of components
        // Updates the given scene to the corresponding behaviors
        static void insertReader(ComponentReaderBase* setReader);

        // Updates the given components within the scene according to the corresponding readers
        // Blocked if update has not been called yet
        static void update();
    protected:
        // Privated to prevent multiple instances of the manager
        ECSManager();

        // Whether the game has started or not
        // Blocks update if false
        bool started{false};
        // The sole instance of this class
        static ECSManager instance;
        // The current game state
        Scene baseScene;
        // The readers that define the behavior of the ecs
        std::vector<ComponentReaderBase*> readers;
};  

#endif