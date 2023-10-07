#include "ui/UIElement.h"

UIElement::UIElement()
{
    initialized = false;
    rect.x = 0;
    rect.y = 0;
    rect.width = 0;
    rect.height = 0;
}

UIElement::~UIElement()
{
    if (initialized == true)
        UnloadRenderTexture(texture);
}

Rectangle UIElement::GetRect()
{
    return rect;
}

RenderTexture2D UIElement::GetTexture()
{
    return texture;
}

UIAlignment UIElement::GetAlignment()
{
    return alignment;
}

void UIElement::Resize(int w, int h)
{
    if (w == rect.width && h == rect.height && initialized == true)
    {
    }
    else
    {
        if (initialized == true)
        {
            UnloadRenderTexture(texture);
            TraceLog(LOG_INFO, "UIElement::Unloading UIElementTexture");
        }
        initialized = true;
        TraceLog(LOG_INFO, "UIElement::New UIElementTexture");
        texture = LoadRenderTexture(w, h);
        rect.width = static_cast<float>(w);
        rect.height = static_cast<float>(h);
    }
}
