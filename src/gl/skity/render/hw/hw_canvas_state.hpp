#ifndef SKITY_SRC_RENDER_HW_HW_CANVAS_STATE_HPP
#define SKITY_SRC_RENDER_HW_HW_CANVAS_STATE_HPP

#include <functional>
#include <vector>

#include "gl/skity/graphic/path.hpp"
#include "gl/skity/render/hw/hw_draw.hpp"

namespace skity {

    class HWCanvasState
    {
    public:
        struct ClipStackValue
        {
            uint32_t stack_depth = {};
            HWDrawRange front_range = {};
            HWDrawRange back_range = {};
            HWDrawRange bound_range = {};
            Matrix stack_matrix = {};
        };

        HWCanvasState();
        ~HWCanvasState() = default;

        void Save();
        void Restore();
        void RestoreToCount(int save_count);
        void Translate(float dx, float dy);
        void Scale(float dx, float dy);
        void Rotate(float degree);
        void Rotate(float degree, float px, float py);
        void Concat(const Matrix& matrix);

        void SaveClipPath(const HWDrawRange& front_range, const HWDrawRange& back_range,
                          const HWDrawRange& bound_range, const Matrix& matrix);

        bool ClipStackEmpty();

        bool NeedRevertClipStencil();

        ClipStackValue CurrentClipStackValue();

        void ForEachClipStackValue(const std::function<void(const ClipStackValue&, size_t)>& func);

        Matrix CurrentMatrix();

        bool HasClip();

        bool MatrixDirty();
        void ClearMatrixDirty();

    private:
        void PushMatrixStack();
        void PopMatrixStack();
        void PopClipStack();

    private:
        std::vector<Matrix> matrix_state_ = {};
        std::vector<ClipStackValue> clip_stack_ = {};
        bool matrix_dirty_ = true;
    };

}  // namespace skity

#endif  // SKITY_SRC_RENDER_HW_HW_CANVAS_STATE_HPP
