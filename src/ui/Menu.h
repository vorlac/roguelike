#pragma once

#include "ui/UIElement.h"

#include <raylib.h>
#include <string>
#include <vector>

class Menu
{
    int currentSelection;
    std::vector<std::string> selections;

public:
    Menu();
    void AddSelection(std::string newItem);
    void Next();
    void Prev();
    int GetSelected();
    std::vector<std::string> GetSelections();
};

void DrawMenu(Menu menu, UIElement* uiElement);
