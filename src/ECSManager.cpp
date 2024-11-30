#include "ECSManager.hpp"

Scene& ECSManager::getScene(){
    return (instance.baseScene);
}

void ECSManager::insertReader(ComponentReaderBase* setReader){
    instance.readers.push_back(setReader);
}

void ECSManager::start(){
    // Ensure that the manager has not been started
    if(!instance.started){
        // Loops through readers to call their corresponding update implementations
        for (unsigned int readerIter = 0; readerIter < instance.readers.size(); readerIter++){
            instance.readers[readerIter]->fullStart();
        }
        instance.started = true;
    } else{
        printf("ERROR::START CALLED MULTIPLE TIMES");
    }
}

void ECSManager::update(){
    // Ensure that the manager has already been started
    if(instance.started){
        // Loops through readers to call their corresponding update implementations
        for (unsigned int readerIter = 0; readerIter < instance.readers.size(); readerIter++){
            instance.readers[readerIter]->fullUpdate();
        }
    } else{
        printf("ERROR::START HAS NOT BEEN CALLED YET");
    }
}

ECSManager::ECSManager(){
}

ECSManager ECSManager::instance = ECSManager();