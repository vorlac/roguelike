/*******************************************************************************************
*
*   rTexPacker example - Load atlas descriptor from .rtpa text file
*
*   This example has been created using exported data from rTexPacker v2.0
*
*   DEPENDENCIES:
*       raylib 4.1-dev      - Windowing/input management and texture drawing
*
*   LICENSE: zlib/libpng
*
*   Copyright (c) 2022 raylib technologies (@raylibtech).
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>
#include <stdio.h>

// Atlas sprite structure
typedef struct AtlasSprite {
    char nameId[128];           // Sprite original filename (without extension)
    int originX, originY;       // Sprite origin (pivot point), useful in some cases
    int positionX, positionY;   // Sprite position in the atlas
    int sourceWidth;            // Sprite source width (before trim)
    int sourceHeight;           // Sprite source height (before trim)
    int padding;                // Sprite padding, must be added to source size
    int trimmed;                // Sprite is trimmed (removed blank space for better packing)
    int trimX, trimY, trimWidth, trimHeight; // Sprite trim rectangle 
    
    // Glyph info, in case sprite is a font character
    // NOTE: This data could probably be ommited 
    int value;                  // Character value (Unicode)
    int offsetX, offsetY;       // Character offset when drawing
    int advanceX;               // Character advance position X
} AtlasSprite;

// Load sprite data from .rtpb file
static AtlasSprite *LoadAtlasSprite(const char *rtpaFilename, int *spriteCount);

// Get one sprite from the array by nameId
static AtlasSprite GetSprite(AtlasSprite *sprites, int spriteCount, const char *nameId);

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "rTexPacker - load sprite from atlas code file");
    
    Texture2D atlas = LoadTexture("resources/koala.png");
    
    int spriteCount = 0;
    AtlasSprite *spriteList = LoadAtlasSprite("resources/koala.rtpa", &spriteCount);

    // Get one specific sprite from the array
    AtlasSprite sprite = GetSprite(spriteList, spriteCount, "title_titletext");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // TODO: Update your variables here
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);
            
            DrawTextureRec(atlas, 
                (Rectangle){ sprite.positionX, sprite.positionY, sprite.sourceWidth, sprite.sourceHeight }, 
                (Vector2){ GetScreenWidth()/2 - sprite.sourceWidth/2, GetScreenHeight()/2 - sprite.sourceHeight/2 }, WHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

// Load sprite data from .rtpb file
static AtlasSprite *LoadAtlasSprite(const char *rtpaFilename, int *spriteCount)
{
    #define MAX_LINE_BUFFER_SIZE    512

    int count = 0;
    AtlasSprite *sprites = NULL;

    // rTPa file contains data in text format, organized in lines:
    // Atlas info:  a <imagePath> <width> <height> <spriteCount> <isFont> <fontSize>
    // Sprite info: s <nameId> <originX> <originY> <positionX> <positionY> <sourceWidth> <sourceHeight> <padding> <trimmed> <trimRecX> <trimRecY> <trimRecWidth> <trimRecHeight>

    FILE *rtpaFile = fopen(rtpaFilename, "rt");
    
    // Define some variables (most of them unused)
    char imagePath[512] = { 0 };
    int atlasWidth = 0;
    int atlasHeight = 0;
    int isFont = 0;
    int fontSize = 0;
    int spriteCounter = 0;
    
    if (rtpaFile != NULL)
    {
        char buffer[MAX_LINE_BUFFER_SIZE] = { 0 };
        fgets(buffer, MAX_LINE_BUFFER_SIZE, rtpaFile);

        while (!feof(rtpaFile))
        {
            switch (buffer[0])
            {
                case 'a':
                {
                    // Atlas info: a <imagePath> <width> <height> <spriteCount> <isFont> <fontSize>
                    // NOTE: Most of this data is not required, we load the atlas image directly in our code
                    sscanf(buffer, "a %s %d %d %d %d", &imagePath, &atlasWidth, &atlasHeight, &count, &isFont, &fontSize);
                    
                    // Init sprite array with the number of sprites included
                    sprites = (AtlasSprite *)RL_CALLOC(count, sizeof(AtlasSprite));

                } break;
                case 's':
                {
                    AtlasSprite sprite = { 0 };
                    
                    if (isFont == 0)
                    {
                        // Sprite info: s <nameId> <originX> <originY> <positionX> <positionY> <sourceWidth> <sourceHeight> <padding> <trimmed> <trimRecX> <trimRecY> <trimRecWidth> <trimRecHeight>
                        sscanf(buffer, "s %s %d %d %d %d %d %d %d %d %d %d %d %d", 
                            sprite.nameId, 
                            &sprite.originX,
                            &sprite.originY,
                            &sprite.positionX,
                            &sprite.positionY,
                            &sprite.sourceWidth,
                            &sprite.sourceHeight,
                            &sprite.padding,
                            &sprite.trimmed,
                            &sprite.trimX,
                            &sprite.trimY,
                            &sprite.trimWidth,
                            &sprite.trimHeight);
                    }
                    else
                    {
                        // Sprite info: s <nameId> <originX> <originY> <positionX> <positionY> <sourceWidth> <sourceHeight> <padding> <trimmed> <trimRecX> <trimRecY> <trimRecWidth> <trimRecHeight> <charValue> <charOfssetX> <charOffsetY> <charAdvanceX>
                        sscanf(buffer, "s %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
                            sprite.nameId, 
                            &sprite.originX,
                            &sprite.originY,
                            &sprite.positionX,
                            &sprite.positionY,
                            &sprite.sourceWidth,
                            &sprite.sourceHeight,
                            &sprite.padding,
                            &sprite.trimmed,
                            &sprite.trimX,
                            &sprite.trimY,
                            &sprite.trimWidth,
                            &sprite.trimHeight,
                            &sprite.value,
                            &sprite.offsetX,
                            &sprite.offsetY,
                            &sprite.advanceX);
                    }
                    
                    sprites[spriteCounter] = sprite;
                    spriteCounter++;
                    
                } break;
                default: break;
            }

            fgets(buffer, MAX_LINE_BUFFER_SIZE, rtpaFile);
        }

        fclose(rtpaFile);
    }
    
    *spriteCount = count;
    return sprites;
}

// Get one sprite from the array by nameId
static AtlasSprite GetSprite(AtlasSprite *sprites, int spriteCount, const char *nameId)
{
    AtlasSprite sprite = { 0 };
    
    for (int i = 0; i < spriteCount; i++)
    {
        if (TextIsEqual(nameId, sprites[i].nameId))
        {
            sprite = sprites[i];
            break;
        }
    }

    return sprite;
}