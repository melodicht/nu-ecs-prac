#ifndef IMGUI_H

// Inspired by https://solhsa.com/imgui/index.html

// If you're going to render widgets to the same
// UI from different source files, you can avoid
// ID collisions by defining IMGUI_SRC_ID before
// this define block:
#ifdef IMGUI_SRC_ID
#define GEN_ID ((IMGUI_SRC_ID) + (__LINE__))
#else
#define GEN_ID (__LINE__)
#endif

struct UIState
{
  int mousex;
  int mousey;
  int mousedown;

  int hotitem;
  int activeitem;
} 
uistate = {0,0,0,0,0};

#define IMGUI_H
#endif
