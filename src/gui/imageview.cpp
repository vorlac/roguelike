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

#pragma warning(push)
#pragma warning(disable : 4244)

namespace rl::gui {

    namespace {
        std::vector<std::string> splitString(const std::string& text, const std::string& delimiter)
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
        : Widget(parent)
        , mTexture(texture)
        , mScale(1.0f)
        , mOffset(Vector2f::Zero())
        , mFixedScale(false)
        , mFixedOffset(false)
        , mPixelInfoCallback(nullptr)
    {
        updateImageParameters();
    }

    ImageView::~ImageView()
    {
    }

    void ImageView::bindImage(SDL3::SDL_Texture* texture)
    {
        mTexture = texture;
        updateImageParameters();
        fit();
    }

    Vector2f ImageView::imageCoordinateAt(const Vector2f& position) const
    {
        auto imagePosition = position - mOffset;
        return imagePosition / mScale;
    }

    Vector2f ImageView::clampedImageCoordinateAt(const Vector2f& position) const
    {
        Vector2f imageCoordinate = imageCoordinateAt(position);
        return imageCoordinate.cmax({ 0, 0 }).cmin(imageSizeF());
    }

    Vector2f ImageView::positionForCoordinate(const Vector2f& imageCoordinate) const
    {
        return imageCoordinate * mScale + mOffset;
    }

    void ImageView::setImageCoordinateAt(const Vector2f& position, const Vector2f& imageCoordinate)
    {
        // Calculate where the new offset must be in order to satisfy the image position equation.
        // Round the floating point values to balance out the floating point to integer conversions.
        mOffset = position - (imageCoordinate * mScale);  // .unaryExpr([](float x) { return
                                                          // std::round(x); });
        // Clamp offset so that the image remains near the screen.
        mOffset = mOffset.cmin(sizeF()).cmax(-scaledImageSizeF());
    }

    void ImageView::center()
    {
        mOffset = (sizeF() - scaledImageSizeF()) / 2;
    }

    void ImageView::fit()
    {
        // Calculate the appropriate scaling factor.
        mScale = (sizeF().cquotient(imageSizeF())).minCoeff();
        center();
    }

    void ImageView::setScaleCentered(float scale)
    {
        auto centerPosition = sizeF() / 2;
        auto p = imageCoordinateAt(centerPosition);
        mScale = scale;
        setImageCoordinateAt(centerPosition, p);
    }

    void ImageView::moveOffset(const Vector2f& delta)
    {
        // Apply the delta to the offset.
        mOffset += delta;

        // Prevent the image from going out of bounds.
        auto scaledSize = scaledImageSizeF();
        if (mOffset.x + scaledSize.x < 0)
            mOffset.x = -scaledSize.x;
        if (mOffset.x > sizeF().x)
            mOffset.x = sizeF().x;
        if (mOffset.y + scaledSize.y < 0)
            mOffset.y = -scaledSize.y;
        if (mOffset.y > sizeF().y)
            mOffset.y = sizeF().y;
    }

    void ImageView::zoom(int amount, const Vector2f& focusPosition)
    {
        auto focusedCoordinate = imageCoordinateAt(focusPosition);
        float scaleFactor = std::pow(mZoomSensitivity, amount);
        mScale = std::max(0.01f, scaleFactor * mScale);
        setImageCoordinateAt(focusPosition, focusedCoordinate);
    }

    bool ImageView::mouseDragEvent(const Vector2i& p, const Vector2i& rel, int button,
                                   int /*modifiers*/)
    {
        if ((button & (1 << SDL_BUTTON_LEFT)) != 0 && !mFixedOffset)
        {
            setImageCoordinateAt((p + rel).tofloat(), imageCoordinateAt(p.cast<float>()));
            return true;
        }
        return false;
    }

    bool ImageView::gridVisible() const
    {
        return (mGridThreshold != -1) && (mScale > mGridThreshold);
    }

    bool ImageView::pixelInfoVisible() const
    {
        return mPixelInfoCallback && (mPixelInfoThreshold != -1.0f) &&
               (mScale > mPixelInfoThreshold);
    }

    bool ImageView::helpersVisible() const
    {
        return gridVisible() || pixelInfoVisible();
    }

    bool ImageView::scrollEvent(const Vector2i& p, const Vector2f& rel)
    {
        if (mFixedScale)
            return false;
        float v = rel.y;
        if (std::abs(v) < 1)
            v = std::copysign(1.0f, v);
        zoom(static_cast<int>(v), (p - position()).tofloat());
        return true;
    }

    bool ImageView::keyboardEvent(int key, int /*scancode*/, int action, int modifiers)
    {
        if (action)
        {
            switch (key)
            {
                case SDL3::SDLK_LEFT:
                    if (!mFixedOffset)
                    {
                        if (SDL3::SDLK_LCTRL & modifiers)
                            moveOffset(Vector2f(30, 0));
                        else
                            moveOffset(Vector2f(10, 0));
                        return true;
                    }
                    break;
                case SDL3::SDLK_RIGHT:
                    if (!mFixedOffset)
                    {
                        if (SDL3::SDLK_LCTRL & modifiers)
                            moveOffset(Vector2f(-30, 0));
                        else
                            moveOffset(Vector2f(-10, 0));
                        return true;
                    }
                    break;
                case SDL3::SDLK_DOWN:
                    if (!mFixedOffset)
                    {
                        if (SDL3::SDLK_LCTRL & modifiers)
                            moveOffset(Vector2f(0, -30));
                        else
                            moveOffset(Vector2f(0, -10));
                        return true;
                    }
                    break;
                case SDL3::SDLK_UP:
                    if (!mFixedOffset)
                    {
                        if (SDL3::SDLK_LCTRL & modifiers)
                            moveOffset(Vector2f(0, 30));
                        else
                            moveOffset(Vector2f(0, 10));
                        return true;
                    }
                    break;
            }
        }
        return false;
    }

    bool ImageView::keyboardCharacterEvent(unsigned int codepoint)
    {
        switch (codepoint)
        {
            case '-':
                if (!mFixedScale)
                {
                    zoom(-1, sizeF() / 2);
                    return true;
                }
                break;
            case '+':
                if (!mFixedScale)
                {
                    zoom(1, sizeF() / 2);
                    return true;
                }
                break;
            case 'c':
                if (!mFixedOffset)
                {
                    center();
                    return true;
                }
                break;
            case 'f':
                if (!mFixedOffset && !mFixedScale)
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
                if (!mFixedScale)
                {
                    setScaleCentered(1 << (codepoint - '1'));
                    return true;
                }
                break;
            default:
                return false;
        }
        return false;
    }

    Vector2i ImageView::preferredSize(SDL3::SDL_Renderer* /*ctx*/) const
    {
        return mImageSize;
    }

    void ImageView::performLayout(SDL3::SDL_Renderer* ctx)
    {
        Widget::performLayout(ctx);
        center();
    }

    void ImageView::draw(SDL3::SDL_Renderer* renderer)
    {
        Widget::draw(renderer);

        SDL3::SDL_Point ap = getAbsolutePos();

        const Screen* screen = dynamic_cast<const Screen*>(this->window()->parent());
        assert(screen);
        Vector2f screenSize = screen->size().tofloat();
        Vector2f scaleFactor = imageSizeF().cquotient(screenSize) * mScale;
        Vector2f positionInScreen = absolutePosition().tofloat();
        Vector2f positionAfterOffset = positionInScreen + mOffset;
        Vector2f imagePosition = positionAfterOffset.cquotient(screenSize);

        if (mTexture)
        {
            Vector2f borderPosition = Vector2f{
                static_cast<float>(ap.x),
                static_cast<float>(ap.y),
            } + mOffset;
            Vector2f borderSize = scaledImageSizeF();

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
                iw = mImageSize.x - ix;
                positionAfterOffset.x = absolutePosition().x;
            }
            if (positionAfterOffset.y <= ap.y)
            {
                iy = ap.y - static_cast<int>(positionAfterOffset.y);
                ih = mImageSize.y - iy;
                positionAfterOffset.y = absolutePosition().y;
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

        drawWidgetBorder(renderer, ap);
        drawImageBorder(renderer, ap);

        if (helpersVisible())
            drawHelpers(renderer);
    }

    void ImageView::updateImageParameters()
    {
        int w, h;
        SDL3::SDL_QueryTexture(mTexture, nullptr, nullptr, &w, &h);
        mImageSize = Vector2i(w, h);
    }

    void ImageView::drawWidgetBorder(SDL3::SDL_Renderer* renderer, const SDL3::SDL_Point& ap) const
    {
        SDL3::SDL_Color lc = mTheme->mBorderLight.toSdlColor();

        SDL3::SDL_FRect lr{
            static_cast<float>(ap.x - 1),
            static_cast<float>(ap.y - 1),
            static_cast<float>(mSize.x + 2),
            static_cast<float>(mSize.y + 2),
        };

        SDL3::SDL_SetRenderDrawColor(renderer, lc.r, lc.g, lc.b, lc.a);
        SDL3::SDL_RenderRect(renderer, &lr);

        SDL3::SDL_Color dc = mTheme->mBorderDark.toSdlColor();
        SDL3::SDL_FRect dr{
            (float)ap.x - 1,
            (float)ap.y - 1,
            (float)mSize.x + 2,
            (float)mSize.y + 2,
        };

        SDL3::SDL_SetRenderDrawColor(renderer, dc.r, dc.g, dc.b, dc.a);
        SDL3::SDL_RenderRect(renderer, &dr);
    }

    void ImageView::drawImageBorder(SDL3::SDL_Renderer* renderer, const SDL3::SDL_Point& ap) const
    {
        Vector2i borderPosition = Vector2i{ ap.x, ap.y } + mOffset.toint();
        Vector2i borderSize = scaledImageSizeF().toint();

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

    void ImageView::drawHelpers(SDL3::SDL_Renderer* renderer) const
    {
        Vector2f upperLeftCorner = positionForCoordinate(Vector2f{ 0, 0 }) + positionF();
        Vector2f lowerRightCorner = positionForCoordinate(imageSizeF()) + positionF();
        // Use the scissor method in NanoVG to display only the correct part of the grid.
        Vector2f scissorPosition = upperLeftCorner.cmax(positionF());
        Vector2f sizeOffsetDifference = sizeF() - mOffset;
        Vector2f scissorSize = sizeOffsetDifference.cmin(sizeF());

        SDL3::SDL_Rect r{ (int)std::round(scissorPosition.x), (int)std::round(scissorPosition.y),
                          (int)std::round(scissorSize.x), (int)std::round(scissorSize.y) };
        if (gridVisible())
            drawPixelGrid(renderer, upperLeftCorner, lowerRightCorner, mScale);
        if (pixelInfoVisible())
            drawPixelInfo(renderer, mScale);
    }

    void ImageView::drawPixelGrid(SDL3::SDL_Renderer* renderer, const Vector2f& upperLeftCorner,
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

    void ImageView::drawPixelInfo(SDL3::SDL_Renderer* renderer, const float stride) const
    {
        // Extract the image coordinates at the two corners of the widget.
        Vector2f currentPixelF = clampedImageCoordinateAt({ 0, 0 });
        Vector2f lastPixelF = clampedImageCoordinateAt(sizeF());
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
        auto fontSize = stride * mFontScaleFactor;
        constexpr static float maxFontSize = 30.0f;
        fontSize = fontSize > maxFontSize ? maxFontSize : fontSize;

        /* nvgSave(ctx);
         nvgBeginPath(ctx);
         nvgFontSize(ctx, fontSize);
         nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_TOP);
         nvgFontFace(ctx, "sans");
         while (currentPixel.y() != lastPixel.y())
         {
             while (currentPixel.x() != lastPixel.x())
             {
                 writePixelInfo(ctx, currentCellPosition, currentPixel, stride);
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

    void ImageView::writePixelInfo(SDL3::SDL_Renderer* renderer, const Vector2f& cellPosition,
                                   const Vector2i& pixel, const float stride) const
    {
        /*   auto pixelData = mPixelInfoCallback(pixel);
           auto pixelDataRows = splitString(pixelData.first, "\n");

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

#pragma warning(pop)
