#ifndef SKITY_SRC_GRAPHIC_PATH_MEASURE_HPP
#define SKITY_SRC_GRAPHIC_PATH_MEASURE_HPP

#include "gl/skity/geometry/contour_measure.hpp"
#include "gl/skity/graphic/path.hpp"

namespace skity {

    /**
     * @class PathMeasure
     *	Util class to measure path length
     */
    class PathMeasure final
    {
    public:
        PathMeasure();
        PathMeasure(const Path& path, bool forceClosed, float resScale = 1.f);

        ~PathMeasure();

        void setPath(const Path* path, bool forceClosed);

        /**
         * Return the total length of the current contour, or 0 if no path is
         * associated.
         *
         * @return total length of current contour
         */
        float getLength();

        /**
         * @brief Get the Pos Tan object
         *
         * @param distance
         * @param position
         * @param tangent
         * @return true
         * @return false
         */
        bool getPosTan(float distance, Point* position, Vector* tangent);

        bool getSegment(float startD, float stopD, Path* dst, bool startWithMoveTo);

        bool isClosed();

        bool nextContour();

    private:
        ContourMeasureIter iter_;
        std::shared_ptr<ContourMeasure> contour_;
    };

}  // namespace skity

#endif  // SKITY_SRC_GRAPHIC_PATH_MEASURE_HPP
