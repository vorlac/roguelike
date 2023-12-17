#ifndef SKITY_SRC_GEOMETRY_CONIC_HPP
#define SKITY_SRC_GEOMETRY_CONIC_HPP

#include <cstring>

#include "gl/skity/geometry/geometry.hpp"
#include "gl/skity/geometry/point.hpp"

namespace skity {

    struct Conic
    {
        enum {
            kMaxConicsForArc = 5,
            kMaxConicToQuadPOW2 = 5,
        };

        static int BuildUnitArc(const Vec2& start, const Vec2& stop, RotationDirection dir,
                                Matrix* matrix, Conic conics[kMaxConicsForArc]);

        Conic() = default;

        Conic(const Point& p0, const Point& p1, const Point& p2, float weight)
            : pts{ p0, p1, p2 }
            , w(weight)
        {
        }

        Conic(const Point p[3], float weight);

        void set(const Point p[3], float weight)
        {
            std::memcpy(pts, p, 3 * sizeof(Point));
            w = weight;
        }

        void set(const glm::vec3 p[3], float weight)
        {
            pts[0] = Point(p[0], 1.f);
            pts[1] = Point(p[1], 1.f);
            pts[2] = Point(p[2], 1.f);
            w = weight;
        }

        void set(const Point& p0, const Point& p1, const Point& p2, float weight)
        {
            pts[0] = p0;
            pts[1] = p1;
            pts[2] = p2;
            w = weight;
        }

        void set(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, float weight)
        {
            pts[0] = Point(p0, 1);
            pts[1] = Point(p1, 1);
            pts[2] = Point(p2, 1);
            w = weight;
        }

        void chop(Conic conics[2]) const;
        bool chopAt(float t, Conic dst[2]) const;
        void chopAt(float t1, float t2, Conic* dst) const;

        void evalAt(float t, Point* pos, Vector* tangent = nullptr) const;
        Point evalAt(float t) const;
        Vector evalTangentAt(float t) const;
        /**
         * @brief Chop this conic into N quads, stored continguously in pts
         *
         * @param pts   quad storage
         * @param pow2  number of quads N = 1 << pow2
         * @return      number of quad storaged in pts
         */
        uint32_t chopIntoQuadsPOW2(Point pts[], uint32_t pow2);
        Point pts[3] = {};
        float w = 0.f;
    };

}  // namespace skity

#endif  // SKITY_SRC_GEOMETRY_CONIC_HPP
