#include "ECSManager.hpp"

Scene ECSManager::getScene(){
    return instance.baseScene;
}

void ECSManager::insertReader(ComponentReaderBase* setReader){
    instance.readers.push_back(setReader);
}

void ECSManager::update(){
    // Loops through readers to call their corresponding update implementations
    for (unsigned int readerIter = 0; readerIter < instance.readers.size(); readerIter++){
        instance.readers[readerIter]->fullUpdate();
    }
}

ECSManager::ECSManager(){
}

ECSManager ECSManager::instance = ECSManager();