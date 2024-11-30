#include "ComponentReaderBase.hpp"

ComponentReaderBase::ComponentReaderBase(){
    ECSManager::insertReader(this);
}

void ComponentReaderBase::InsertScene(Scene* setScene){
    givenScene = setScene;
}