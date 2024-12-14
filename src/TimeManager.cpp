#include "TimeManager.hpp"
#include "iostream"

float TimeManager::deltaTime = 0;
float TimeManager::lastTime = 0;
float TimeManager::timeScale = 1;
void TimeManager::init(){
    lastTime = ((float)clock()) / CLOCKS_PER_SEC;
}

void TimeManager::update(){
    float newTime = ((float)clock()) / CLOCKS_PER_SEC;
    deltaTime = newTime - lastTime;
    lastTime = newTime;
}


float TimeManager::getFixedDelta(){
    return deltaTime;
}

float TimeManager::getDelta(){
    return deltaTime * timeScale;
}

void TimeManager::setTimeScale(float setScale){
    timeScale = setScale;
}