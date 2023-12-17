
#include <cassert>

#include "gl/skity/geometry/conic.hpp"
#include "gl/skity/geometry/geometry.hpp"
#include "gl/skity/geometry/math.hpp"
#include "gl/skity/geometry/point_priv.hpp"

namespace skity {

    static Vector eval_cubic_derivative(const Point src[4], float t)
    {
        QuadCoeff coeff;
        glm::vec2 P0 = FromPoint(src[0]);
        glm::vec2 P1 = FromPoint(src[1]);
        glm::vec2 P2 = FromPoint(src[2]);
        glm::vec2 P3 = FromPoint(src[3]);

        coeff.A = P3 + glm::vec2{ 3, 3 } * (P1 - P2) - P0;
        coeff.B = Times2(P2 - Times2(P1) + P0);
        coeff.C = P1 - P0;
        glm::vec2 ret = coeff.eval(t);
        return Vector{ ret.x, ret.y, 0, 0 };
    }

    static Vector eval_cubic_2ndDerivative(const Point src[4], float t)
    {
        glm::vec2 P0 = FromPoint(src[0]);
        glm::vec2 P1 = FromPoint(src[1]);
        glm::vec2 P2 = FromPoint(src[2]);
        glm::vec2 P3 = FromPoint(src[3]);
        glm::vec2 A = P3 + glm::vec2{ 3, 3 } * (P1 - P2) - P0;
        glm::vec2 B = P2 - Times2(P1) + P0;

        glm::vec2 vec = A * glm::vec2{ t, t } + B;
        return glm::vec4{ vec.x, vec.y, 0, 0 };
    }

    static float calc_cubic_precision(const Point src[4])
    {
        return 0;
    }

    QuadCoeff::QuadCoeff(const std::array<Point, 3>& src)
    {
        C = FromPoint(src[0]);
        glm::vec2 P1 = FromPoint(src[1]);
        glm::vec2 P2 = FromPoint(src[2]);
        B = Times2(P1 - C);
        A = P2 - Times2(P1) + C;
    }

    QuadCoeff::QuadCoeff(const std::array<glm::vec2, 3>& src)
    {
        C = src[0];
        glm::vec2 P1 = src[1];
        glm::vec2 P2 = src[2];
        B = Times2(P1 - C);
        A = P2 - Times2(P1) + C;
    }

    Point QuadCoeff::evalAt(float t)
    {
        return Point{ eval(t), 0, 1 };
    }

    glm::vec2 QuadCoeff::eval(float t)
    {
        return eval(glm::vec2{ t, t });
    }

    glm::vec2 QuadCoeff::eval(const glm::vec2& tt)
    {
        return (A * tt + B) * tt + C;
    }

    Point QuadCoeff::EvalQuadAt(const std::array<Point, 3>& src, float t)
    {
        return ToPoint(QuadCoeff{ src }.eval(t));
    }

    void QuadCoeff::EvalQuadAt(const std::array<Point, 3>& src, float t, Point* outP,
                               Vector* outTangent)
    {
        if (t < 0)
            t = 0;
        if (t > Float1)
            t = Float1;

        if (outP)
            *outP = EvalQuadAt(src, t);

        if (outTangent)
            *outTangent = EvalQuadTangentAt(src, t);
    }

    Vector QuadCoeff::EvalQuadTangentAt(const std::array<Point, 3>& src, float t)
    {
        if ((t == 0 && src[0] == src[1]) || (t == 1 && src[1] == src[2]))
            return src[2] - src[0];

        glm::vec2 P0 = FromPoint(src[0]);
        glm::vec2 P1 = FromPoint(src[1]);
        glm::vec2 P2 = FromPoint(src[2]);

        glm::vec2 B = P1 - P0;
        glm::vec2 A = P2 - P1 - B;
        glm::vec2 T = A * glm::vec2{ t, t } + B;

        return Vector{ T + T, 0, 0 };
    }

    glm::vec2 QuadCoeff::EvalQuadTangentAt(const glm::vec2& p1, const glm::vec2& p2,
                                           const glm::vec2& p3, float t)
    {
        glm::vec2 B = p2 - p1;
        glm::vec2 A = p3 - p2 - B;
        glm::vec2 T = A * glm::vec2{ t, t } + B;

        return glm::normalize(T);
    }

    void QuadCoeff::ChopQuadAt(const Point src[3], Point dst[5], float t)
    {
        assert(t > 0 && t < Float1);

        Vec2 p0 = FromPoint(src[0]);
        Vec2 p1 = FromPoint(src[1]);
        Vec2 p2 = FromPoint(src[2]);
        Vec2 tt{ t };

        Vec2 p01 = Interp(p0, p1, tt);
        Vec2 p12 = Interp(p1, p2, tt);

        dst[0] = ToPoint(p0);
        dst[1] = ToPoint(p01);
        dst[2] = ToPoint(Interp(p01, p12, tt));
        dst[3] = ToPoint(p12);
        dst[4] = ToPoint(p2);
    }

    CubicCoeff::CubicCoeff(const std::array<Point, 4>& src)
    {
        glm::vec2 P0 = FromPoint(src[0]);
        glm::vec2 P1 = FromPoint(src[1]);
        glm::vec2 P2 = FromPoint(src[2]);
        glm::vec2 P3 = FromPoint(src[3]);
        glm::vec2 three{ 3, 3 };

        A = P3 + three * (P1 - P2) - P0;
        B = three * (P2 - Times2(P1) + P0);
        C = three * (P1 - P0);
        D = P0;
    }

    Point CubicCoeff::evalAt(float t)
    {
        return Point{ eval(t), 0, 1 };
    }

    glm::vec2 CubicCoeff::eval(float t)
    {
        return eval(glm::vec2{ t, t });
    }

    glm::vec2 CubicCoeff::eval(const glm::vec2& t)
    {
        return ((A * t + B) * t + C) * t + D;
    }

    void CubicCoeff::EvalCubicAt(const Point src[4], float t, Point* loc, Vector* tangent,
                                 Vector* curvature)
    {
        if (loc)
            *loc = ToPoint(CubicCoeff({ src[0], src[1], src[2], src[3] }).eval(t));

        if (tangent)
        {
            // The derivative equation returns a zero tangent vector when t is 0 or 1,
            // and the adjacent control point is equal to the end point. In this case,
            // use the next control point or the end points to compute the tangent.
            if ((t == 0 && src[0] == src[1]) || (t == 1 && src[2] == src[3]))
            {
                if (t == 0)
                    *tangent = src[2] - src[0];
                else
                    *tangent = src[3] - src[1];

                if (!tangent->x && !tangent->y)
                    *tangent = src[3] - src[0];
            }
            else
            {
                *tangent = eval_cubic_derivative(src, t);
            }
        }

        if (curvature)
            *curvature = eval_cubic_2ndDerivative(src, t);
    }

    void CubicCoeff::ChopCubicAt(const Point src[4], Point dst[7], float t)
    {
        glm::vec2 p0 = FromPoint(src[0]);
        glm::vec2 p1 = FromPoint(src[1]);
        glm::vec2 p2 = FromPoint(src[2]);
        glm::vec2 p3 = FromPoint(src[3]);
        glm::vec2 tt{ t, t };

        glm::vec2 ab = Interp(p0, p1, tt);
        glm::vec2 bc = Interp(p1, p2, tt);
        glm::vec2 cd = Interp(p2, p3, tt);
        glm::vec2 abc = Interp(ab, bc, tt);
        glm::vec2 bcd = Interp(bc, cd, tt);
        glm::vec2 abcd = Interp(abc, bcd, tt);

        dst[0] = ToPoint(p0);
        dst[1] = ToPoint(ab);
        dst[2] = ToPoint(abc);
        dst[3] = ToPoint(abcd);
        dst[4] = ToPoint(bcd);
        dst[5] = ToPoint(cd);
        dst[6] = ToPoint(p3);
    }

    ConicCoeff::ConicCoeff(const Conic& conic)
    {
        glm::vec2 P0 = FromPoint(conic.pts[0]);
        glm::vec2 P1 = FromPoint(conic.pts[1]);
        glm::vec2 P2 = FromPoint(conic.pts[2]);
        glm::vec2 ww{ conic.w, conic.w };

        glm::vec2 p1w = P1 * ww;
        numer.C = P0;
        numer.A = P2 - Times2(p1w) + P0;
        numer.B = Times2(p1w - P0);

        denom.C = glm::vec2{ 1, 1 };
        denom.B = Times2(ww - denom.C);
        denom.A = glm::vec2{ 0, 0 } - denom.B;
    }

    glm::vec2 ConicCoeff::eval(float t)
    {
        glm::vec2 tt{ t, t };
        glm::vec2 n = numer.eval(tt);
        glm::vec2 d = denom.eval(tt);
        return n / d;
    }

    float pt_to_line(const Point& pt, const Point& lineStart, const Point& lineEnd)
    {
        Vector dxy = lineEnd - lineStart;
        Vector ab0 = pt - lineStart;

        float number = VectorDotProduct(dxy, ab0);
        float denom = VectorDotProduct(dxy, dxy);
        float t = SkityIEEEFloatDivided(number, denom);
        if (t >= 0 && t <= 1)
        {
            Point hit;
            hit.x = lineStart.x * (1 - t) + lineEnd.x * t;
            hit.y = lineStart.y * (1 - t) + lineEnd.y * t;
            hit.z = 0;
            hit.w = 1;
            return PointDistanceToSqd(hit, pt);
        }
        else
        {
            return PointDistanceToSqd(pt, lineStart);
        }
    }

    void SubDividedCubic(const Point cubic[4], Point sub_cubic1[4], Point sub_cubic2[4])
    {
        Point p1 = (cubic[0] + cubic[1]) / 2.f;
        Point p2 = (cubic[1] + cubic[2]) / 2.f;
        Point p3 = (cubic[2] + cubic[3]) / 2.f;
        Point p4 = (p1 + p2) / 2.f;
        Point p5 = (p2 + p3) / 2.f;
        Point p6 = (p4 + p5) / 2.f;

        Point p0 = cubic[0];
        Point p7 = cubic[3];

        sub_cubic1[0] = p0;
        sub_cubic1[1] = p1;
        sub_cubic1[2] = p4;
        sub_cubic1[3] = p6;

        sub_cubic2[0] = p6;
        sub_cubic2[1] = p5;
        sub_cubic2[2] = p3;
        sub_cubic2[3] = p7;
    }

    void SubDividedCubic2(const Point cubic[4], Point sub_cubic[8])
    {
        SubDividedCubic(cubic, sub_cubic, sub_cubic + 4);
    }

    void SubDividedCubic4(const Point cubic[4], Point sub_cubic[16])
    {
        SubDividedCubic(cubic, sub_cubic, sub_cubic + 8);
        SubDividedCubic2(sub_cubic, sub_cubic);
        SubDividedCubic2(sub_cubic + 8, sub_cubic + 8);
    }

    void SubDividedCubic8(const Point cubic[4], Point sub_cubic[32])
    {
        SubDividedCubic(cubic, sub_cubic, sub_cubic + 16);
        SubDividedCubic4(sub_cubic, sub_cubic);
        SubDividedCubic4(sub_cubic + 16, sub_cubic + 16);
    }

    void SubDividedQuad(const Point quad[3], Point sub_quad1[3], Point sub_quad2[3])
    {
        Point p1 = (quad[0] + quad[1]) * 0.5f;
        Point p2 = (quad[1] + quad[2]) * 0.5f;
        Point p3 = (p1 + p2) * 0.5f;

        sub_quad1[0] = quad[0];
        sub_quad1[1] = p1;
        sub_quad1[2] = p3;

        sub_quad2[0] = p3;
        sub_quad2[1] = p2;
        sub_quad2[2] = quad[2];
    }

    void SubDividedQuad(const Vec2 quad[3], Vec2 sub_quad1[3], Vec2 sub_quad2[3])
    {
        Vec2 p1 = (quad[0] + quad[1]) * 0.5f;
        Vec2 p2 = (quad[1] + quad[2]) * 0.5f;
        Vec2 p3 = (p1 + p2) * 0.5f;

        sub_quad1[0] = quad[0];
        sub_quad1[1] = p1;
        sub_quad1[2] = p3;

        sub_quad2[0] = p3;
        sub_quad2[1] = p2;
        sub_quad2[2] = quad[2];
    }

    void CubicToQuadratic(const Point cubic[4], Point quad[3])
    {
        quad[0] = cubic[0];
        quad[1] = (3.f * (cubic[1] + cubic[2]) - (cubic[0] + cubic[3])) / 4.f;
        quad[2] = cubic[3];
    }

}  // namespace skity
