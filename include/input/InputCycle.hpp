#ifndef INPUTCYCLE_HPP
#define INPUTCYCLE_HPP

#include "Input.hpp"

#include <vector>

/**
 * Represents the inputs a single player makes in a cycle of logic separate from the game logic.
 */
struct InputCycle{
    // The button inputs recording within the cycle.
    // The order of inputs within this vector does not mean anything.
    std::vector<InputID> buttonInputs;

    // Represents the x and y coordinates of the mouse at the end of a given cycle.
    // [0] = x
    // [1] = y
    std::array<float, 2> mousePos;

    // Creates an input cycle with no button inputs and 0,0 mouse pos.
    InputCycle();
};

#endif