#pragma once

#include <raylib.h>

enum UIAlignment
{
	TOP_LEFT = 0,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT,
	CENTER
};

class UIElement
{
	Rectangle rect;
	RenderTexture2D texture;
	UIAlignment alignment;
	bool initialized;
public:
	UIElement();
	~UIElement();
	void Resize(int w, int h);
	Rectangle GetRect();
	RenderTexture2D GetTexture();
	UIAlignment GetAlignment();
};