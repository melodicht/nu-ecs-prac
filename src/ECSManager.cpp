#include "ECSManager.hpp"

Scene ECSManager::getScene(){
    return instance.baseScene;
}

// Inserts a reader that determines the behavior of a range of components
// Updates the given scene to the corresponding behaviors
void ECSManager::insertReader(ComponentReaderBase* setReader){
    instance.readers.push_back(setReader);
}

// Updates the given components within the scene according to the corresponding readers.
void ECSManager::update(){
    // Loops through readers to call their corresponding update implementations
    for (unsigned int readerIter = 0; readerIter < instance.readers.size(); readerIter++){
        instance.readers[readerIter]->fullUpdate();
    }
}

// Privated to prevent multiple instances of the manager
ECSManager::ECSManager(){

}