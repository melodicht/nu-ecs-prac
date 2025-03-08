#ifndef IMGUI_H

// Inspired by https://solhsa.com/imgui/index.html

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
