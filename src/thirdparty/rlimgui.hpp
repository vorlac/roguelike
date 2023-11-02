#pragma once

#include "core/numeric_types.hpp"
#include "thirdparty/icons_font_awesome.hpp"
#include "thirdparty/raylib.hpp"

constexpr inline rl::i32 FONT_AWESOME_ICON_SIZE{ 12 };

namespace rlimgui
{
    // High level API. This API is designed in the style of raylib and meant to work with
    // reaylib code. It will manage it's own ImGui context and call common ImGui functions (like
    // NewFrame and Render) for you for a lower level API that matches the other ImGui
    // platforms, please see imgui_impl_raylib.h

    /**
     * @brief Sets up ImGui, loads fonts and themes
     * Calls ImGui_ImplRaylib_Init and sets the theme. Will install Font awesome by default
     *
     * @param darkTheme when true(default) the dark theme is used, when false the light theme is
     * used
     * */
    void Setup(bool darkTheme);

    /**
     * @brief Starts a new ImGui Frame
     * Calls ImGui_ImplRaylib_NewFrame, ImGui_ImplRaylib_ProcessEvents, and ImGui::NewFrame
     * together
     * */
    void Begin();

    /**
     * @brief Ends an ImGui frame and submits all ImGui drawing to raylib for processing.
     * Calls ImGui:Render, an d ImGui_ImplRaylib_RenderDrawData to draw to the current raylib
     * render target
     * */
    void End();

    /**
     * @brief Cleanup ImGui and unload font atlas
     * Calls ImGui_ImplRaylib_Shutdown
     * */
    void Shutdown();

    // Advanced StartupAPI

    /**
     * @brief Custom initialization. Not needed if you call Setup. Only needed if you want to
     * add custom setup code. must be followed by EndInitImGui Called by
     * ImGui_ImplRaylib_Init, and does the first part of setup, before fonts are rendered
     * */
    void BeginInitImGui();

    /**
     * @brief End Custom initialization. Not needed if you call Setup. Only needed if you want
     * to add custom setup code. must be proceeded by BeginInitImGui Called by
     * ImGui_ImplRaylib_Init and does the second part of setup, and renders fonts.
     * */
    void EndInitImGui();

    /**
     * @brief Forces the font texture atlas to be recomputed and re-cached
     * */
    void ReloadFonts();

    /**
     * @section Advanced Update API
     */

    /**
     * @brief Starts a new ImGui Frame with a specified delta time
     *
     * @param delta_time any value < 0 will use raylib GetFrameTime
     * */
    void BeginDelta(rl::f32 deltaTime);

    /**
     * ImGui Image API extensions
     * Purely for convenience in working with raylib textures as images.
     * If you want to call ImGui image functions directly, simply pass them the pointer to the
     * texture.
     * */

    /**
     * @brief Draw a texture as an image in an ImGui Context* Uses the current ImGui Cursor
     * position and the full texture size.
     *
     * @param image The raylib texture to draw
     * */
    void Image(const raylib::Texture* image);

    /**
     * @brief Draw a texture as an image in an ImGui Context at a specific size
     * Uses the current ImGui Cursor position and the specified width and height
     * The image will be scaled up or down to fit as needed
     *
     * @param image The raylib texture to draw
     * @param width The width of the drawn image
     * @param height The height of the drawn image
     * */
    void ImageSize(const raylib::Texture* image, int width, int height);

    /**
     * @brief Draw a texture as an image in an ImGui Context at a specific size
     * Uses the current ImGui Cursor position and the specified size
     * The image will be scaled up or down to fit as needed
     *
     * @param mage The raylib texture to draw
     * @param size The size of drawn image
     * */
    void ImageSizeV(const raylib::Texture* image, raylib::Vector2 size);

    /**
     * @brief Draw a portion texture as an image in an ImGui Context at a defined size
     * Uses the current ImGui Cursor position and the specified size
     * The image will be scaled up or down to fit as needed
     *
     * @param image The raylib texture to draw
     * @param destWidth The width of the drawn image
     * @param destHeight The height of the drawn image
     * @param sourceRect The portion of the texture to draw as an image. Negative values
     * for the width and height will flip the image
     * */
    void ImageRect(const raylib::Texture* image, int destWidth, int destHeight,
                   raylib::Rectangle sourceRect);

    /**
     * @brief Draws a render texture as an image an ImGui Context, automatically flipping the Y
     * axis so it will show correctly on screen
     *
     * @param image The render texture to draw
     * */
    void ImageRenderTexture(const raylib::RenderTexture* image);

    /**
     * @brief Draws a render texture as an image an ImGui Context, automatically flipping the Y
     * axis so it will show correctly on screen Fits the render texture to the available content
     * area
     *
     * @param image The render texture to draw
     * @param center When true the image will be centered in the content area
     * */
    void ImageRenderTextureFit(const raylib::RenderTexture* image, bool center);

    /**
     * @brief Draws a texture as an image button in an ImGui context. Uses the current ImGui
     * cursor position and the full size of the texture
     *
     * @param name The display name and ImGui ID for the button
     * @param image The texture to draw
     * @returns True if the button was clicked
     * */
    bool ImageButton(const char* name, const raylib::Texture* image);

    /**
     * @brief Draws a texture as an image button in an ImGui context. Uses the current ImGui
     * cursor position and the specified size.
     *
     * @param name The display name and ImGui ID for the button
     * @param image The texture to draw
     * @param size The size of the button
     * @returns true if the button was clicked
     * */
    bool ImageButtonSize(const char* name, const raylib::Texture* image, struct ImVec2 size);
}
