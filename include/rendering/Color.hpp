#ifndef COLOR_HPP
#define COLOR_HPP

#include <cstdint>

// Represents a color to be rendered
// Always a solid color
struct Color {
    // Red value of the color
    uint8_t r;
    // Green value of the color
    uint8_t g;
    // Blue value of the color
    uint8_t b;

    // Constructs a color with the given RGB values
    Color(uint8_t r, uint8_t g, uint8_t b);

    // Constructs a pure black color
    Color();
};

#endif 