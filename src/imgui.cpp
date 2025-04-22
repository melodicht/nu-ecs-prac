#include "imgui.h"

void ImguiPrepare()
{
    uistate.hotitem = 0;
}

// If mouse isn't down, we need to clear the active item in order not
// to make the widgets confused on the active state (and to enable the
// next clicked widget to become active). If the mouse is clicked, but
// no widget is active, we need to mark the active item unavailable so
// that we won't activate the next widget we drag the cursor onto.
void ImguiFinish()
{
    if (uistate.mousedown == 0)
    {
        uistate.activeitem = 0;
    }
    else
    {
        if (uistate.activeitem == 0)
            uistate.activeitem = -1;
    }
}

// Check whether current mouse position is within a rectangle
// Return 1 if so, 0 otherwise.
int RegionHit(int x, int y, int w, int h)
{
    if (uistate.mousex < x ||
        uistate.mousey < y ||
        uistate.mousex >= x + w ||
        uistate.mousey >= y + h)
        return 0;
    return 1;
}

// Creates a button of a given ID and a given width and height
// size. If Button is clicked, returns 1, 0 otherwise.  Click means
// button is hot and active, but mouse button is not down (because
// they just let it go after holding the button down.)
int Button(int id, sf::RenderWindow &window,
           int x, int y, int w, int h)
{
    // Update uistate's hot/active item.
    if (RegionHit(x, y, w, h))
    {
        uistate.hotitem = id;
        if (uistate.activeitem == 0 && uistate.mousedown)
            uistate.activeitem = id;
    }

    // Rendering button.
    u32 shadowOffsetX = 8;
    u32 shadowOffsetY = 8;
    u32 pressedOffsetX = 2;
    u32 pressedOffsetY = 2;

    DrawRect(window, x + shadowOffsetX, y + shadowOffsetY, w, h, 0, 0, 0);
    if (uistate.hotitem == id)
    {
        if (uistate.activeitem == id)
        {
            // Button is both 'hot' and 'active'
            DrawRect(window, x + pressedOffsetX, y + pressedOffsetY, w, h, 255, 255, 255);
        }
        else
        {
            // Button is merely 'hot'
            DrawRect(window, x, y, w, h, 255, 255, 255);
        }
    }
    else
    {
        // button is not hot, but it may be active    
        DrawRect(window, x, y, w, h, 170, 170, 170);
    }

    // Is the mouse clicked?
    if (uistate.mousedown == 0 &&
        uistate.hotitem == id &&
        uistate.activeitem == id)
        return 1;

    return 0;
}
