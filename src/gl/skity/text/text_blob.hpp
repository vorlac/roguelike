#ifndef SKITY_TEXT_TEXT_BLOB_HPP
#define SKITY_TEXT_TEXT_BLOB_HPP

#include <string>
#include <vector>

#include "gl/skity/graphic/paint.hpp"
#include "gl/skity/text/text_run.hpp"

namespace skity {

    class Typeface;

    /**
     * Immutable container which to hold TextRun.
     *
     */
    class TextBlob final
    {
    public:
        TextBlob(std::vector<TextRun> runs)
            : text_run_(std::move(runs))
        {
        }

        ~TextBlob() = default;

        const std::vector<TextRun>& getTextRun() const
        {
            return text_run_;
        }

        Vec2 getBoundSize() const;

        float getBlobAscent() const;

        float getBlobDescent() const;

    private:
        std::vector<TextRun> text_run_ = {};
    };

    class TypefaceDelegate
    {
    public:
        virtual ~TypefaceDelegate() = default;

        virtual std::shared_ptr<Typeface> fallback(GlyphID glyph_id, const Paint& text_paint) = 0;

        virtual std::vector<std::vector<GlyphID>> breakTextRun(const char* text) = 0;

        static std::unique_ptr<TypefaceDelegate> CreateSimpleFallbackDelegate(
            const std::vector<std::shared_ptr<Typeface>>& typefaces);
    };

    class TextBlobBuilder final
    {
    public:
        TextBlobBuilder() = default;
        ~TextBlobBuilder() = default;

        std::shared_ptr<TextBlob> buildTextBlob(const char* text, const Paint& paint,
                                                TypefaceDelegate* delegate = nullptr);

        std::shared_ptr<TextBlob> buildTextBlob(const std::string& text, const Paint& paint);

    private:
        std::shared_ptr<TextBlob> GenerateBlobWithoutDelegate(const char* text, const Paint& paint);

        std::shared_ptr<TextBlob> GenerateBlobWithDelegate(const char* text, const Paint& paint,
                                                           TypefaceDelegate* delegate);

        std::shared_ptr<TextBlob> GenerateBlobWithMultiRun(
            const std::vector<std::vector<GlyphID>>& glyph_ids, const Paint& paint,
            TypefaceDelegate* delegate);

        std::vector<TextRun> GenerateTextRuns(const std::vector<GlyphID>& glyphs,
                                              const std::shared_ptr<Typeface>& typeface,
                                              const Paint& paint, TypefaceDelegate* delegate);

        TextRun GenerateTextRun(const std::vector<GlyphID>& glyphs,
                                const std::shared_ptr<Typeface>& typeface, float font_size,
                                bool need_path);
    };

}  // namespace skity

#endif  // SKITY_TEXT_TEXT_BLOB_HPP
