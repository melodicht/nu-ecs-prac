#ifndef IMGUI_H

// Inspired by https://solhsa.com/imgui/index.html

// TODO(imguigenid): This doesn't work if making widgets in a for
// loop, which isn't inconceivable!
// If you're going to render widgets to the same UI from different
// source files, you can avoid ID collisions by defining IMGUI_SRC_ID
// before this define block:

#define GEN_ID __COUNTER__ + 1

struct UIState
{
    int mousex;
    int mousey;
    int mousedown;

    int hotitem;
    int activeitem;
}
uistate = {0, 0, 0, 0, 0};

#define IMGUI_H
#endif
