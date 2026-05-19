#ifndef CV_UTILS_H
#define CV_UTILS_H

#include <godot_cpp/classes/ref_counted.hpp>
#include "opencv_image.h"

namespace godot {

class CVUtils : public RefCounted {
    GDCLASS(CVUtils, RefCounted);

protected:
    static void _bind_methods();

public:
    CVUtils();
    ~CVUtils();

    enum ColorCode {
        COLOR_BGR2GRAY = 6, COLOR_RGB2GRAY = 7,
        COLOR_GRAY2BGR = 8, COLOR_GRAY2RGB = 8,
        COLOR_BGR2HSV = 40, COLOR_RGB2HSV = 41,
        COLOR_HSV2BGR = 54, COLOR_HSV2RGB = 55,
        COLOR_BGR2RGB = 4, COLOR_RGB2BGR = 4,
        COLOR_BGR2LAB = 44, COLOR_RGB2LAB = 45,
    };

    enum ThresholdType {
        THRESH_BINARY = 0, THRESH_BINARY_INV = 1,
        THRESH_TRUNC = 2, THRESH_TOZERO = 3,
        THRESH_TOZERO_INV = 4, THRESH_OTSU = 8,
    };

    enum MorphType {
        MORPH_ERODE = 0, MORPH_DILATE = 1, MORPH_OPEN = 2,
        MORPH_CLOSE = 3, MORPH_GRADIENT = 4,
        MORPH_TOPHAT = 5, MORPH_BLACKHAT = 6,
    };

    enum InterpolationFlags {
        INTER_NEAREST = 0, INTER_LINEAR = 1, INTER_CUBIC = 2,
        INTER_AREA = 3, INTER_LANCZOS4 = 4,
    };

    enum ContourMode {
        RETR_EXTERNAL = 0, RETR_LIST = 1, RETR_CCOMP = 2, RETR_TREE = 3,
    };

    enum ContourMethod {
        CHAIN_APPROX_NONE = 1, CHAIN_APPROX_SIMPLE = 2,
    };

    static Ref<CVImage> match_template(const Ref<CVImage> &image, const Ref<CVImage> &templ, int method);
    static String get_opencv_version();
};

} // namespace godot

VARIANT_ENUM_CAST(CVUtils::ColorCode);
VARIANT_ENUM_CAST(CVUtils::ThresholdType);
VARIANT_ENUM_CAST(CVUtils::MorphType);
VARIANT_ENUM_CAST(CVUtils::InterpolationFlags);
VARIANT_ENUM_CAST(CVUtils::ContourMode);
VARIANT_ENUM_CAST(CVUtils::ContourMethod);

#endif
