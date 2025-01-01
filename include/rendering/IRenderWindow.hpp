#ifndef IRENDERWINDOW_HPP
#define IRENDERWINDOW_HPP

#include "InputCycle.hpp"
#include "Scene.hpp"
#include "Color.hpp"

// Represents a window on the player's screen.
// Allow's the functionality of rendering a scene onto the window
// and gathering input from the window.
class IRenderWindow {
    public:

        // Gets the background color of the window.
        virtual Color getBackColor() = 0;

        // Gets the current width of the window this window.
        virtual int getWindowWidth() = 0;

        // Gets the current height of the window this window.
        virtual int getWindowHeight() = 0;

        // Sets a number of parameters of the render window.
        // Template T is the class that implements this interface.
        template <typename T>
        class IRenderWindowBuilder {
            // Sets the background color of the window to be created.
            virtual void setBuildBackColor(Color setColor) = 0;

            // Sets the width of the window to be created.
            virtual void setBuildWidth(int setWidth) = 0;

            // Sets the height of the window to be created.
            virtual void setWindowHeight(int setHeight) = 0;

            // Sets the title of the window
            virtual void setWindowTitle(std::string setTitle) = 0;

            // Builds a window based on parameters.
            virtual T build() = 0;
        };

        // Checks if the window is active or not
        virtual bool isActive();

        // Polls all inputs from a player recorded in the window.
        virtual InputCycle pollEvent() = 0;

        // Renders the scene given to it.
        virtual void renderScene(Scene& givenScene) = 0;

};

#endif