#include "gui/imageview.hpp"
#include "gui/screen.hpp"
#include "gui/window.hpp"
#include "sdl/defs.hpp"

SDL_C_LIB_BEGIN
#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
SDL_C_LIB_END

#include <cmath>

#include "gui/theme.hpp"

#pragma warning(disable : 4244)

namespace rl::gui {
    namespace {
        std::vector<std::string> split_string(const std::string& text, const std::string& delimiter)
        {
            using std::string;
            using std::vector;
            vector<string> strings;
            string::size_type current = 0;
            string::size_type previous = 0;
            while ((current = text.find(delimiter, previous)) != string::npos)
            {
                strings.push_back(text.substr(previous, current - previous));
                previous = current + 1;
            }
            strings.push_back(text.substr(previous));
            return strings;
        }
    }

    ImageView::ImageView(Widget* parent, SDL3::SDL_Texture* texture)
        : Widget{ parent }
        , mTexture{ texture }
        , m_scale{ 1.0f }
        , m_offset{ Vector2f::zero() }
        , m_fixed_scale{ false }
        , m_fixed_offset{ false }
        , m_pixel_info_callback{ nullptr }
    {
        update_image_params();
    }

    ImageView::~ImageView()
    {
    }

    void ImageView::bind_image(SDL3::SDL_Texture* texture)
    {
        mTexture = texture;
        update_image_params();
        fit();
    }

    Vector2f ImageView::image_coord_at(const Vector2f& position) const
    {
        auto imagePosition = position - m_offset;
        return imagePosition / m_scale;
    }

    Vector2f ImageView::clamped_image_coord_at(const Vector2f& position) const
    {
        Vector2f imageCoordinate = image_coord_at(position);
        return imageCoordinate.cmax({ 0, 0 }).cmin(image_size_flt());
    }

    Vector2f ImageView::positionForCoordinate(const Vector2f& imageCoordinate) const
    {
        return imageCoordinate * m_scale + m_offset;
    }

    void ImageView::set_image_coord_at(const Vector2f& position, const Vector2f& imageCoordinate)
    {
        // Calculate where the new offset must be in order to satisfy the image position equation.
        // Round the floating point values to balance out the floating point to integer conversions.
        m_offset = position - (imageCoordinate * m_scale);  // .unaryExpr([](float x) { return
                                                            // std::round(x); });
        // Clamp offset so that the image remains near the screen.
        m_offset = m_offset.cmin(size_flt()).cmax(-scaled_image_size_flt());
    }

    void ImageView::center()
    {
        m_offset = (size_flt() - scaled_image_size_flt()) / 2;
    }

    void ImageView::fit()
    {
        // Calculate the appropriate scaling factor.
        m_scale = (size_flt().cquotient(image_size_flt())).min_coeff();
        center();
    }

    void ImageView::set_scale_centered(float scale)
    {
        auto centerPosition = size_flt() / 2;
        auto p = image_coord_at(centerPosition);
        m_scale = scale;
        set_image_coord_at(centerPosition, p);
    }

    void ImageView::move_offset(const Vector2f& delta)
    {
        // Apply the delta to the offset.
        m_offset += delta;

        // Prevent the image from going out of bounds.
        auto scaledSize = this->scaled_image_size_flt();
        if (m_offset.x + scaledSize.x < 0)
            m_offset.x = -scaledSize.x;
        if (m_offset.x > size_flt().x)
            m_offset.x = size_flt().x;
        if (m_offset.y + scaledSize.y < 0)
            m_offset.y = -scaledSize.y;
        if (m_offset.y > size_flt().y)
            m_offset.y = size_flt().y;
    }

    void ImageView::zoom(int amount, const Vector2f& focusPosition)
    {
        auto focusedCoordinate = this->image_coord_at(focusPosition);
        float scaleFactor = std::pow(m_zoom_sensitivity, amount);
        m_scale = std::max(0.01f, scaleFactor * m_scale);
        this->set_image_coord_at(focusPosition, focusedCoordinate);
    }

    bool ImageView::mouse_drag_event(const Vector2i& p, const Vector2i& rel, int button,
                                     int /*modifiers*/)
    {
        if ((button & (1 << SDL_BUTTON_LEFT)) != 0 && !m_fixed_offset)
        {
            this->set_image_coord_at((p + rel).tofloat(), this->image_coord_at(p.cast<float>()));
            return true;
        }
        return false;
    }

    bool ImageView::grid_visible() const
    {
        return (m_grid_threshold != -1) && (m_scale > m_grid_threshold);
    }

    bool ImageView::pixel_info_visible() const
    {
        return m_pixel_info_callback && (m_pixel_info_threshold != -1.0f) &&
               (m_scale > m_pixel_info_threshold);
    }

    bool ImageView::debug_overlays_visible() const
    {
        return this->grid_visible() || this->pixel_info_visible();
    }

    bool ImageView::scroll_event(const Vector2i& p, const Vector2f& rel)
    {
        if (m_fixed_scale)
            return false;

        float v = rel.y;
        if (std::abs(v) < 1)
            v = std::copysign(1.0f, v);

        this->zoom(static_cast<int>(v), (p - this->relative_position()).tofloat());
        return true;
    }

    bool ImageView::kb_button_event(int key, int /*scancode*/, int action, int modifiers)
    {
        if (action)
        {
            switch (key)
            {
                case SDL3::SDLK_LEFT:
                    if (!m_fixed_offset)
                    {
                        if (SDL3::SDLK_LCTRL & modifiers)
                            this->move_offset(Vector2f(30, 0));
                        else
                            this->move_offset(Vector2f(10, 0));
                        return true;
                    }
                    break;
                case SDL3::SDLK_RIGHT:
                    if (!m_fixed_offset)
                    {
                        if (SDL3::SDLK_LCTRL & modifiers)
                            this->move_offset(Vector2f(-30, 0));
                        else
                            this->move_offset(Vector2f(-10, 0));
                        return true;
                    }
                    break;
                case SDL3::SDLK_DOWN:
                    if (!m_fixed_offset)
                    {
                        if (SDL3::SDLK_LCTRL & modifiers)
                            this->move_offset(Vector2f(0, -30));
                        else
                            this->move_offset(Vector2f(0, -10));
                        return true;
                    }
                    break;
                case SDL3::SDLK_UP:
                    if (!m_fixed_offset)
                    {
                        if (SDL3::SDLK_LCTRL & modifiers)
                            this->move_offset(Vector2f(0, 30));
                        else
                            this->move_offset(Vector2f(0, 10));
                        return true;
                    }
                    break;
            }
        }
        return false;
    }

    bool ImageView::kb_character_event(unsigned int codepoint)
    {
        switch (codepoint)
        {
            case '-':
                if (!m_fixed_scale)
                {
                    this->zoom(-1, size_flt() / 2);
                    return true;
                }
                break;
            case '+':
                if (!m_fixed_scale)
                {
                    this->zoom(1, size_flt() / 2);
                    return true;
                }
                break;
            case 'c':
                if (!m_fixed_offset)
                {
                    center();
                    return true;
                }
                break;
            case 'f':
                if (!m_fixed_offset && !m_fixed_scale)
                {
                    fit();
                    return true;
                }
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                if (!m_fixed_scale)
                {
                    set_scale_centered(1 << (codepoint - '1'));
                    return true;
                }
                break;
            default:
                return false;
        }
        return false;
    }

    Vector2i ImageView::preferred_size(SDL3::SDL_Renderer* /*ctx*/) const
    {
        return m_image_size;
    }

    void ImageView::perform_layout(SDL3::SDL_Renderer* ctx)
    {
        Widget::perform_layout(ctx);
        center();
    }

    void ImageView::draw(SDL3::SDL_Renderer* renderer)
    {
        Widget::draw(renderer);

        SDL3::SDL_Point ap = get_absolute_pos();

        const Screen* screen = dynamic_cast<const Screen*>(this->window()->parent());
        runtime_assert(screen != nullptr, "ImageView: drawing to invalid screen");

        Vector2f screenSize = screen->size().tofloat();
        Vector2f scaleFactor = image_size_flt().cquotient(screenSize) * m_scale;
        Vector2f positionInScreen = absolute_position().tofloat();
        Vector2f positionAfterOffset = positionInScreen + m_offset;
        Vector2f imagePosition = positionAfterOffset.cquotient(screenSize);

        if (mTexture)
        {
            Vector2f borderPosition = Vector2f{
                static_cast<float>(ap.x),
                static_cast<float>(ap.y),
            } + m_offset;
            Vector2f borderSize = scaled_image_size_flt();

            SDL3::SDL_FRect br{ borderPosition.x + 1, borderPosition.y + 1, borderSize.x - 2,
                                borderSize.y - 2 };

            PntRect r = srect2pntrect(br);
            PntRect wr = { ap.x, ap.y, ap.x + width(), ap.y + height() };

            if (r.x1 <= wr.x1)
                r.x1 = wr.x1;
            if (r.x2 >= wr.x2)
                r.x2 = wr.x2;
            if (r.y1 <= wr.y1)
                r.y1 = wr.y1;
            if (r.y2 >= wr.y2)
                r.y2 = wr.y2;

            int ix = 0, iy = 0;
            int iw = r.x2 - r.x1;
            int ih = r.y2 - r.y1;
            if (positionAfterOffset.x <= ap.x)
            {
                ix = ap.x - static_cast<int>(positionAfterOffset.x);
                iw = m_image_size.x - ix;
                positionAfterOffset.x = absolute_position().x;
            }
            if (positionAfterOffset.y <= ap.y)
            {
                iy = ap.y - static_cast<int>(positionAfterOffset.y);
                ih = m_image_size.y - iy;
                positionAfterOffset.y = absolute_position().y;
            }

            SDL3::SDL_FRect imgrect{
                static_cast<float>(ix),
                static_cast<float>(iy),
                static_cast<float>(iw),
                static_cast<float>(ih),
            };

            SDL3::SDL_FRect rect{
                std::round(positionAfterOffset.x),
                std::round(positionAfterOffset.y),
                imgrect.w,
                imgrect.h,
            };

            SDL3::SDL_RenderTexture(renderer, mTexture, &imgrect, &rect);
        }

        draw_widget_border(renderer, ap);
        draw_image_border(renderer, ap);

        if (debug_overlays_visible())
            draw_debug_overlays(renderer);
    }

    void ImageView::update_image_params()
    {
        int w, h;
        SDL3::SDL_QueryTexture(mTexture, nullptr, nullptr, &w, &h);
        m_image_size = Vector2i(w, h);
    }

    void ImageView::draw_widget_border(SDL3::SDL_Renderer* renderer, const SDL3::SDL_Point& ap) const
    {
        SDL3::SDL_Color lc = m_theme->m_border_light.sdl_color();

        SDL3::SDL_FRect lr{
            static_cast<float>(ap.x - 1),
            static_cast<float>(ap.y - 1),
            static_cast<float>(m_size.x + 2),
            static_cast<float>(m_size.y + 2),
        };

        SDL3::SDL_SetRenderDrawColor(renderer, lc.r, lc.g, lc.b, lc.a);
        SDL3::SDL_RenderRect(renderer, &lr);

        SDL3::SDL_Color dc = m_theme->m_border_dark.sdl_color();
        SDL3::SDL_FRect dr{
            (float)ap.x - 1,
            (float)ap.y - 1,
            (float)m_size.x + 2,
            (float)m_size.y + 2,
        };

        SDL3::SDL_SetRenderDrawColor(renderer, dc.r, dc.g, dc.b, dc.a);
        SDL3::SDL_RenderRect(renderer, &dr);
    }

    void ImageView::draw_image_border(SDL3::SDL_Renderer* renderer, const SDL3::SDL_Point& ap) const
    {
        Vector2i borderPosition = Vector2i{ ap.x, ap.y } + m_offset.toint();
        Vector2i borderSize = scaled_image_size_flt().toint();

        SDL3::SDL_FRect br{
            static_cast<float>(borderPosition.x + 1),
            static_cast<float>(borderPosition.y + 1),
            static_cast<float>(borderSize.x - 2),
            static_cast<float>(borderSize.y - 2),
        };

        PntRect r = srect2pntrect(br);
        PntRect wr = { ap.x, ap.y, ap.x + width(), ap.y + height() };

        if (r.x1 <= wr.x1)
            r.x1 = wr.x1;
        if (r.x2 >= wr.x2)
            r.x2 = wr.x2;
        if (r.y1 <= wr.y1)
            r.y1 = wr.y1;
        if (r.y2 >= wr.y2)
            r.y2 = wr.y2;

        SDL3::SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        if (r.x1 > wr.x1)
            SDL3::SDL_RenderLine(renderer, r.x1, r.y1, r.x1, r.y2 - 1);
        if (r.y1 > wr.y1)
            SDL3::SDL_RenderLine(renderer, r.x1, r.y1, r.x2 - 1, r.y1);
        if (r.x2 < wr.x2)
            SDL3::SDL_RenderLine(renderer, r.x2, r.y1, r.x2, r.y2 - 1);
        if (r.y2 < wr.y2)
            SDL3::SDL_RenderLine(renderer, r.x1, r.y2, r.x2 - 1, r.y2);
    }

    void ImageView::draw_debug_overlays(SDL3::SDL_Renderer* renderer) const
    {
        Vector2f upperLeftCorner = positionForCoordinate(Vector2f{ 0, 0 }) + positionF();
        Vector2f lowerRightCorner = positionForCoordinate(image_size_flt()) + positionF();
        // Use the scissor method in NanoVG to display only the correct part of the grid.
        Vector2f scissorPosition = upperLeftCorner.cmax(positionF());
        Vector2f sizeOffsetDifference = size_flt() - m_offset;
        Vector2f scissorSize = sizeOffsetDifference.cmin(size_flt());

        SDL3::SDL_Rect r{ (int)std::round(scissorPosition.x), (int)std::round(scissorPosition.y),
                          (int)std::round(scissorSize.x), (int)std::round(scissorSize.y) };
        if (grid_visible())
            draw_pixel_grid(renderer, upperLeftCorner, lowerRightCorner, m_scale);
        if (pixel_info_visible())
            draw_pixel_info(renderer, m_scale);
    }

    void ImageView::draw_pixel_grid(SDL3::SDL_Renderer* renderer, const Vector2f& upperLeftCorner,
                                    const Vector2f& lowerRightCorner, const float stride)
    {
        SDL3::SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        // Draw the vertical lines for the grid
        float currentX = std::floor(upperLeftCorner.x);
        while (currentX <= lowerRightCorner.x)
        {
            SDL3::SDL_RenderLine(renderer, std::floor(currentX), std::floor(upperLeftCorner.y),
                                 std::floor(currentX), std::floor(lowerRightCorner.y));
            currentX += stride;
        }
        // Draw the horizontal lines for the grid.
        float currentY = std::floor(upperLeftCorner.y);
        while (currentY <= lowerRightCorner.y)
        {
            SDL3::SDL_RenderLine(renderer, std::floor(upperLeftCorner.x), std::floor(currentY),
                                 std::floor(lowerRightCorner.x), std::floor(currentY));
            currentY += stride;
        }
    }

    void ImageView::draw_pixel_info(SDL3::SDL_Renderer* renderer, const float stride) const
    {
        // Extract the image coordinates at the two corners of the widget.
        Vector2f currentPixelF = clamped_image_coord_at({ 0, 0 });
        Vector2f lastPixelF = clamped_image_coord_at(size_flt());
        // Round the top left coordinates down and bottom down coordinates up.
        // This is done so that the edge information does not pop up suddenly when it gets in range.
        currentPixelF = currentPixelF.floor();
        lastPixelF = lastPixelF.ceil();
        Vector2i currentPixel = currentPixelF.cast<int>();
        Vector2i lastPixel = lastPixelF.cast<int>();

        // Extract the positions for where to draw the text.
        Vector2f currentCellPosition = (positionF() + positionForCoordinate(currentPixelF));
        float xInitialPosition = currentCellPosition.x;
        int xInitialIndex = currentPixel.x;

        // Properly scale the pixel information for the given stride.
        auto font_size = stride * m_fond_scale_factor;
        constexpr static float maxFontSize = 30.0f;
        font_size = font_size > maxFontSize ? maxFontSize : font_size;

        /* nvgSave(ctx);
         nvgBeginPath(ctx);
         nvgFontSize(ctx, font_size);
         nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
         nvgFontFace(ctx, "sans");
         while (currentPixel.y() != lastPixel.y())
         {
             while (currentPixel.x() != lastPixel.x())
             {
                 write_pixel_info(ctx, currentCellPosition, currentPixel, stride);
                 currentCellPosition.x() += stride;
                 ++currentPixel.x();
             }
             currentCellPosition.x() = xInitialPosition;
             currentCellPosition.y() += stride;
             ++currentPixel.y();
             currentPixel.x() = xInitialIndex;
         }
         nvgRestore(ctx);*/
    }

    void ImageView::write_pixel_info(SDL3::SDL_Renderer* renderer, const Vector2f& cellPosition,
                                     const Vector2i& pixel, const float stride) const
    {
        /*   auto pixelData = m_pixel_info_callback(pixel);
           auto pixelDataRows = split_string(pixelData.first, "\n");

           // If no data is provided for this pixel then simply return.
           if (pixelDataRows.empty())
               return;

           nvgFillColor(ctx, pixelData.second);
           auto padding = stride / 10;
           auto maxSize = stride - 2 * padding;

           // Measure the size of a single line of text.
           float bounds[4];
           nvgTextBoxBounds(ctx, 0.0f, 0.0f, maxSize, pixelDataRows.front().data(), nullptr,
           bounds); auto rowHeight = bounds[3] - bounds[1]; auto totalRowsHeight = rowHeight *
           pixelDataRows.size();

           // Choose the initial y offset and the index for the past the last visible row.
           auto yOffset = 0.0f;
           auto lastIndex = 0;

           if (totalRowsHeight > maxSize) {
               yOffset = padding;
               lastIndex = (int) (maxSize / rowHeight);
           } else {
               yOffset = (stride - totalRowsHeight) / 2;
               lastIndex = (int) pixelDataRows.size();
           }

           for (int i = 0; i != lastIndex; ++i) {
               nvgText(ctx, cellPosition.x() + stride / 2, cellPosition.y() + yOffset,
                       pixelDataRows[i].data(), nullptr);
               yOffset += rowHeight;
           }
       */
    }
}
