#ifndef TIMEMANAGER_HPP
#define TIMEMANAGER_HPP

#include <cstdlib>
#include <ctime>

// Represents the time within the system of the game engine
// Provides a single point of information to gather time data out of 
class TimeManager{

    public:
        // DeltaTime will be set to 0
        // Clock will be started
        static void init();

        // Updates the given delta time
        // 
        static void update();

        // Gets the current delta time scaled to time scale
        // 0 on initialization
        static float getDelta();

        // Gets the current delta time
        // 0 on initialization
        static float getFixedDelta();

        // Sets how the scale the expresses how fast unfixed
        // time will pass
        static void setTimeScale(float setScale);

    private:
        // The difference of time between update calls
        // The difference of time before the game has started if no update calls have been made
        static float deltaTime;

        // The last time that update was called
        // used to calculate deltaTime
        static float lastTime;

        // The scale at which unfixed time passes in game compared to real world time
        // Default is 1
        static float timeScale;

        // Whether the given manager has been initialized
        static bool hasInited;
};

#endif