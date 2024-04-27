/*******************************************************************************************
*
*   rTexPacker example - Use atlas descriptor exported as code file (.h)
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

#include "resources/koala.h"          // Atlas descriptor as code file

// Get one sprite index position from the array by nameId
static int GetSpriteIndex(rtpAtlasSprite *spriteDesc, int spriteCount, const char *nameId);

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "rTexPacker - load sprite from atlas code file");
    
    Texture2D atlas = LoadTexture(TextFormat("resources/%s", ATLAS_KOALA_IMAGE_PATH));
    
    // Get sprite from the array given a specific index
    rtpAtlasSprite sprite = rtpDescKoala[GetSpriteIndex(rtpDescKoala, ATLAS_KOALA_SPRITE_COUNT, "title_titletext")];

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

// Get one sprite index position from the array by nameId
static int GetSpriteIndex(rtpAtlasSprite *spriteDesc, int spriteCount, const char *nameId)
{
    int index = 0;
    
    for (int i = 0; i < spriteCount; i++)
    {
        if (TextIsEqual(nameId, spriteDesc[i].nameId))
        {
            index = i;
            break;
        }
    }
    
    return index;
}