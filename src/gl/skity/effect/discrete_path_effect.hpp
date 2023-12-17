#ifndef SKITY_SRC_EFFECT_DISCRETE_PATH_EFFECT_HPP
#define SKITY_SRC_EFFECT_DISCRETE_PATH_EFFECT_HPP

#include "gl/skity/effect/path_effect.hpp"

namespace skity {

    class DiscretePathEffect : public PathEffect
    {
    public:
        DiscretePathEffect(float seg_length, float deviation, uint32_t seed_assist);
        ~DiscretePathEffect() override = default;

    protected:
        bool onFilterPath(Path* dst, const Path& src, bool stroke, const Paint&) const override;

    private:
        float seg_length_;
        float perterb_;
        uint32_t seed_assist_;
    };

}  // namespace skity

#endif  // SKITY_SRC_EFFECT_DISCRETE_PATH_EFFECT_HPP
