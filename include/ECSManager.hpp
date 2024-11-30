#ifndef ECSMANAGER_HPP
#define ECSMANAGER_HPP

#include <vector>

#include "ComponentReader.hpp"
#include "Scene.hpp"
#include "SceneView.hpp"

// Handles the regular updates/behaviors of the components within a given scene
// ONLY SUPPORTS A SINGLE INSTANCE
// TODO LATER: Scene functionality, currently only one scene is loaded and preseneted
class ECSManager{
    public:
        // Gets the scene that the current ECSManager is updating and providing behaviors for
        static Scene getScene();

        // Inserts a reader that determines the behavior of a range of components
        // Updates the given scene to the corresponding behaviors
        static void insertReader(ComponentReader setReader);

        // Updates the given components within the scene according to the corresponding readers.
        static void update();
    private:
        // Privated to prevent multiple instances of the manager
        ECSManager();

        // The sole instance of this class
        static ECSManager instance;
        std::vector<ComponentReader> readers;
        Scene base;
        SceneView givenScene;
};

#endif