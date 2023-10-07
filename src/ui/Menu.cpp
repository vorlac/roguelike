
#include "ui/Menu.h"

#include "ui/Window.h"

Menu::Menu()
{
    currentSelection = 0;
}

void Menu::AddSelection(std::string newItem)
{
    selections.push_back(newItem);
}

void Menu::Next()
{
    if (currentSelection == selections.size() - 1)
        currentSelection = 0;
    else
        currentSelection++;
}

void Menu::Prev()
{
    if (currentSelection == 0)
        currentSelection = static_cast<int>(selections.size() - 1);
    else
        currentSelection--;
}

int Menu::GetSelected()
{
    return currentSelection;
}

std::vector<std::string> Menu::GetSelections()
{
    return selections;
}

void DrawMenu(Menu menu, UIElement* uiElement)
{
    Font font = GetFontDefault();

    // setup texture size
    Rectangle rect = uiElement->GetRect();
    uiElement->Resize(300, 300);

    // settings
    Vector2 offset = { rect.x, rect.y };
    Color baseColor = { 155, 155, 155, 255 };
    Color selectedColor = { 255, 255, 255, 255 };
    int padding = 25;

    // find widest
    size_t widest = 0;
    for (std::string item : menu.GetSelections())
        if (item.length() > widest)
            widest = item.length();

    Vector2 textSize = MeasureTextEx(font, "A", 18, 1);
    int windowWidth = static_cast<int>((widest * textSize.x) + padding);
    int windowHeight = static_cast<int>((menu.GetSelections().size() * textSize.y) + padding);

    Color topBorderColor = { 100, 109, 110, 255 };
    Color bgColor = { 78, 87, 87, 255 };
    Color bottomBorderColor = { 58, 67, 67, 255 };

    // draw

    BeginTextureMode(uiElement->GetTexture());

    DrawWindow(0, 0, windowWidth, windowHeight, "poo", bgColor, topBorderColor, bottomBorderColor);

    int i = 0;
    for (std::string item : menu.GetSelections())
    {
        Color textColor = baseColor;
        if (menu.GetSelected() == i)
            textColor = selectedColor;
        Vector2 loc = { offset.x, offset.y + (i * 20) };
        // DrawText(item.c_str(), , 18, textColor);
        DrawTextEx(font, item.c_str(), loc, 18, 1, textColor);
        i++;
    }
    EndTextureMode();
}
