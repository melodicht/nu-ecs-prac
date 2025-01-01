#ifndef INPUT_HPP
#define INPUT_HPP

#include <cstdint>

// Inputs are identified by an ID system where in the input gathered is activates certain ID's.
// This allows for actual input to be abstract away from the results of said input (i.e. one person could bind kick to c while another can bind kick to d)
typedef uint8_t InputID;

#endif