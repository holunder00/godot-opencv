#include "opencv_image.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void CVImage::_bind_methods() {
    ClassDB::bind_method(D_METHOD("create", "width", "height", "type"), &CVImage::create, DEFVAL(0));
    ClassDB::bind_method(D_METHOD("create_from_image", "image"), &CVImage::create_from_image);
    ClassDB::bind_method(D_METHOD("to_image"), &CVImage::to_image);
    ClassDB::bind_method(D_METHOD("to_image_texture"), &CVImage::to_image_texture);
    ClassDB::bind_method(D_METHOD("load", "path"), &CVImage::load);
    ClassDB::bind_method(D_METHOD("save", "path"), &CVImage::save);

    ClassDB::bind_method(D_METHOD("get_width"), &CVImage::get_width);
    ClassDB::bind_method(D_METHOD("get_height"), &CVImage::get_height);
    ClassDB::bind_method(D_METHOD("get_channels"), &CVImage::get_channels);
    ClassDB::bind_method(D_METHOD("is_empty"), &CVImage::is_empty);
    ClassDB::bind_method(D_METHOD("get_size"), &CVImage::get_size);

    ClassDB::bind_method(D_METHOD("cvt_color", "code"), &CVImage::cvt_color);
    ClassDB::bind_method(D_METHOD("to_grayscale"), &CVImage::to_grayscale);
    ClassDB::bind_method(D_METHOD("to_rgb"), &CVImage::to_rgb);
    ClassDB::bind_method(D_METHOD("to_hsv"), &CVImage::to_hsv);

    ClassDB::bind_method(D_METHOD("blur", "ksize"), &CVImage::blur);
    ClassDB::bind_method(D_METHOD("gaussian_blur", "ksize", "sigma_x", "sigma_y"), &CVImage::gaussian_blur, DEFVAL(0.0));
    ClassDB::bind_method(D_METHOD("median_blur", "ksize"), &CVImage::median_blur);
    ClassDB::bind_method(D_METHOD("bilateral_filter", "d", "sigma_color", "sigma_space"), &CVImage::bilateral_filter);

    ClassDB::bind_method(D_METHOD("canny", "threshold1", "threshold2", "aperture_size", "l2_gradient"), &CVImage::canny, DEFVAL(3), DEFVAL(false));
    ClassDB::bind_method(D_METHOD("sobel", "ddepth", "dx", "dy", "ksize"), &CVImage::sobel, DEFVAL(3));
    ClassDB::bind_method(D_METHOD("laplacian", "ddepth", "ksize"), &CVImage::laplacian, DEFVAL(1));

    ClassDB::bind_method(D_METHOD("threshold", "thresh", "maxval", "type"), &CVImage::threshold);
    ClassDB::bind_method(D_METHOD("adaptive_threshold", "maxval", "method", "type", "block_size", "c"), &CVImage::adaptive_threshold);

    ClassDB::bind_method(D_METHOD("dilate", "ksize", "iterations"), &CVImage::dilate, DEFVAL(1));
    ClassDB::bind_method(D_METHOD("erode", "ksize", "iterations"), &CVImage::erode, DEFVAL(1));
    ClassDB::bind_method(D_METHOD("morphology_ex", "op", "ksize"), &CVImage::morphology_ex);

    ClassDB::bind_method(D_METHOD("resize", "new_size", "interpolation"), &CVImage::resize, DEFVAL(1));
    ClassDB::bind_method(D_METHOD("crop", "rect"), &CVImage::crop);
    ClassDB::bind_method(D_METHOD("flip", "flip_code"), &CVImage::flip);
    ClassDB::bind_method(D_METHOD("rotate", "rotate_code"), &CVImage::rotate);
    ClassDB::bind_method(D_METHOD("warp_affine", "matrix", "dsize"), &CVImage::warp_affine);

    ClassDB::bind_method(D_METHOD("draw_line", "pt1", "pt2", "color", "thickness"), &CVImage::draw_line, DEFVAL(1));
    ClassDB::bind_method(D_METHOD("draw_rectangle", "rect", "color", "thickness"), &CVImage::draw_rectangle, DEFVAL(1));
    ClassDB::bind_method(D_METHOD("draw_circle", "center", "radius", "color", "thickness"), &CVImage::draw_circle, DEFVAL(1));
    ClassDB::bind_method(D_METHOD("draw_text", "text", "org", "font_face", "font_scale", "color", "thickness"), &CVImage::draw_text, DEFVAL(0), DEFVAL(1.0), DEFVAL(Color(1, 1, 1)), DEFVAL(1));

    ClassDB::bind_method(D_METHOD("detect_haar_cascade", "cascade_path", "scale_factor", "min_neighbors", "min_size", "max_size"), &CVImage::detect_haar_cascade, DEFVAL(1.1), DEFVAL(3), DEFVAL(Vector2i(0, 0)), DEFVAL(Vector2i(0, 0)));

    ClassDB::bind_method(D_METHOD("equalize_hist"), &CVImage::equalize_hist);
    ClassDB::bind_method(D_METHOD("find_contours", "mode", "method"), &CVImage::find_contours);

    ClassDB::bind_method(D_METHOD("get_pixel", "x", "y"), &CVImage::get_pixel);
    ClassDB::bind_method(D_METHOD("set_pixel", "x", "y", "color"), &CVImage::set_pixel);

    ClassDB::bind_method(D_METHOD("add", "other"), &CVImage::add);
    ClassDB::bind_method(D_METHOD("subtract", "other"), &CVImage::subtract);
    ClassDB::bind_method(D_METHOD("multiply", "scalar"), &CVImage::multiply);
    ClassDB::bind_method(D_METHOD("bitwise_and", "other"), &CVImage::bitwise_and);
    ClassDB::bind_method(D_METHOD("bitwise_or", "other"), &CVImage::bitwise_or);
    ClassDB::bind_method(D_METHOD("bitwise_not"), &CVImage::bitwise_not);
    ClassDB::bind_method(D_METHOD("absdiff", "other"), &CVImage::absdiff);
    ClassDB::bind_method(D_METHOD("in_range", "lower", "upper"), &CVImage::in_range);
}

CVImage::CVImage() {}
CVImage::~CVImage() {}

cv::Scalar CVImage::color_to_scalar(const Color &c) {
    return cv::Scalar(c.b * 255, c.g * 255, c.r * 255, c.a * 255);
}

Ref<CVImage> CVImage::_from_mat(const cv::Mat &p_mat) {
    Ref<CVImage> img;
    img.instantiate();
    img->mat = p_mat;
    return img;
}

cv::Mat &CVImage::get_mat() { return mat; }
const cv::Mat &CVImage::get_mat() const { return mat; }
void CVImage::set_mat(const cv::Mat &p_mat) { mat = p_mat; }

void CVImage::create(int width, int height, int type) {
    mat = cv::Mat::zeros(height, width, type == 0 ? CV_8UC3 : type);
}

void CVImage::create_from_image(const Ref<Image> &p_image) {
    if (p_image.is_null()) return;
    Ref<Image> img = p_image->duplicate();
    int w = img->get_width();
    int h = img->get_height();
    Image::Format fmt = img->get_format();

    if (fmt == Image::FORMAT_RGBA8) {
        PackedByteArray data = img->get_data();
        mat = cv::Mat(h, w, CV_8UC4);
        memcpy(mat.data, data.ptr(), data.size());
        cv::cvtColor(mat, mat, cv::COLOR_RGBA2BGRA);
    } else if (fmt == Image::FORMAT_RGB8) {
        PackedByteArray data = img->get_data();
        mat = cv::Mat(h, w, CV_8UC3);
        memcpy(mat.data, data.ptr(), data.size());
        cv::cvtColor(mat, mat, cv::COLOR_RGB2BGR);
    } else if (fmt == Image::FORMAT_L8) {
        PackedByteArray data = img->get_data();
        mat = cv::Mat(h, w, CV_8UC1);
        memcpy(mat.data, data.ptr(), data.size());
    } else {
        img->convert(Image::FORMAT_RGBA8);
        PackedByteArray data = img->get_data();
        mat = cv::Mat(h, w, CV_8UC4);
        memcpy(mat.data, data.ptr(), data.size());
        cv::cvtColor(mat, mat, cv::COLOR_RGBA2BGRA);
    }
}

Ref<Image> CVImage::to_image() const {
    if (mat.empty()) return Ref<Image>();
    cv::Mat converted;
    Image::Format fmt;
    switch (mat.channels()) {
        case 1: fmt = Image::FORMAT_L8; converted = mat; break;
        case 3: cv::cvtColor(mat, converted, cv::COLOR_BGR2RGB); fmt = Image::FORMAT_RGB8; break;
        case 4: cv::cvtColor(mat, converted, cv::COLOR_BGRA2RGBA); fmt = Image::FORMAT_RGBA8; break;
        default: return Ref<Image>();
    }
    if (!converted.isContinuous()) converted = converted.clone();
    PackedByteArray data;
    data.resize(converted.total() * converted.elemSize());
    memcpy(data.ptrw(), converted.data, data.size());
    return Image::create_from_data(converted.cols, converted.rows, false, fmt, data);
}

Ref<ImageTexture> CVImage::to_image_texture() const {
    Ref<Image> img = to_image();
    if (img.is_null()) return Ref<ImageTexture>();
    return ImageTexture::create_from_image(img);
}

bool CVImage::load(const String &path) {
    std::string std_path = std::string(path.utf8().get_data());
    mat = cv::imread(std_path, cv::IMREAD_UNCHANGED);
    return !mat.empty();
}

bool CVImage::save(const String &path) {
    if (mat.empty()) return false;
    std::string std_path = std::string(path.utf8().get_data());
    return cv::imwrite(std_path, mat);
}

int CVImage::get_width() const { return mat.cols; }
int CVImage::get_height() const { return mat.rows; }
int CVImage::get_channels() const { return mat.channels(); }
bool CVImage::is_empty() const { return mat.empty(); }
Vector2i CVImage::get_size() const { return Vector2i(mat.cols, mat.rows); }

Ref<CVImage> CVImage::cvt_color(int code) const {
    cv::Mat result; cv::cvtColor(mat, result, code); return _from_mat(result);
}

Ref<CVImage> CVImage::to_grayscale() const {
    if (mat.channels() == 1) return _from_mat(mat.clone());
    cv::Mat result;
    cv::cvtColor(mat, result, mat.channels() == 4 ? cv::COLOR_BGRA2GRAY : cv::COLOR_BGR2GRAY);
    return _from_mat(result);
}

Ref<CVImage> CVImage::to_rgb() const {
    cv::Mat result;
    if (mat.channels() == 1) cv::cvtColor(mat, result, cv::COLOR_GRAY2BGR);
    else if (mat.channels() == 4) cv::cvtColor(mat, result, cv::COLOR_BGRA2BGR);
    else result = mat.clone();
    return _from_mat(result);
}

Ref<CVImage> CVImage::to_hsv() const {
    cv::Mat bgr, result;
    if (mat.channels() == 1) cv::cvtColor(mat, bgr, cv::COLOR_GRAY2BGR);
    else if (mat.channels() == 4) cv::cvtColor(mat, bgr, cv::COLOR_BGRA2BGR);
    else bgr = mat;
    cv::cvtColor(bgr, result, cv::COLOR_BGR2HSV);
    return _from_mat(result);
}

Ref<CVImage> CVImage::blur(const Vector2i &ksize) const {
    cv::Mat result; cv::blur(mat, result, cv::Size(ksize.x, ksize.y)); return _from_mat(result);
}

Ref<CVImage> CVImage::gaussian_blur(const Vector2i &ksize, float sigma_x, float sigma_y) const {
    cv::Mat result; cv::GaussianBlur(mat, result, cv::Size(ksize.x, ksize.y), sigma_x, sigma_y); return _from_mat(result);
}

Ref<CVImage> CVImage::median_blur(int ksize) const {
    cv::Mat result; cv::medianBlur(mat, result, ksize); return _from_mat(result);
}

Ref<CVImage> CVImage::bilateral_filter(int d, double sigma_color, double sigma_space) const {
    cv::Mat result; cv::bilateralFilter(mat, result, d, sigma_color, sigma_space); return _from_mat(result);
}

Ref<CVImage> CVImage::canny(double threshold1, double threshold2, int aperture_size, bool l2_gradient) const {
    cv::Mat gray, result;
    if (mat.channels() > 1) cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY); else gray = mat;
    cv::Canny(gray, result, threshold1, threshold2, aperture_size, l2_gradient);
    return _from_mat(result);
}

Ref<CVImage> CVImage::sobel(int ddepth, int dx, int dy, int ksize) const {
    cv::Mat result; cv::Sobel(mat, result, ddepth, dx, dy, ksize); return _from_mat(result);
}

Ref<CVImage> CVImage::laplacian(int ddepth, int ksize) const {
    cv::Mat result; cv::Laplacian(mat, result, ddepth, ksize); return _from_mat(result);
}

Ref<CVImage> CVImage::threshold(double thresh, double maxval, int type) const {
    cv::Mat result; cv::threshold(mat, result, thresh, maxval, type); return _from_mat(result);
}

Ref<CVImage> CVImage::adaptive_threshold(double maxval, int method, int type, int block_size, double c) const {
    cv::Mat gray, result;
    if (mat.channels() > 1) cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY); else gray = mat;
    cv::adaptiveThreshold(gray, result, maxval, method, type, block_size, c);
    return _from_mat(result);
}

Ref<CVImage> CVImage::dilate(const Vector2i &ksize, int iterations) const {
    cv::Mat result, kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ksize.x, ksize.y));
    cv::dilate(mat, result, kernel, cv::Point(-1, -1), iterations); return _from_mat(result);
}

Ref<CVImage> CVImage::erode(const Vector2i &ksize, int iterations) const {
    cv::Mat result, kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ksize.x, ksize.y));
    cv::erode(mat, result, kernel, cv::Point(-1, -1), iterations); return _from_mat(result);
}

Ref<CVImage> CVImage::morphology_ex(int op, const Vector2i &ksize) const {
    cv::Mat result, kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(ksize.x, ksize.y));
    cv::morphologyEx(mat, result, op, kernel); return _from_mat(result);
}

Ref<CVImage> CVImage::resize(const Vector2i &new_size, int interpolation) const {
    cv::Mat result; cv::resize(mat, result, cv::Size(new_size.x, new_size.y), 0, 0, interpolation); return _from_mat(result);
}

Ref<CVImage> CVImage::crop(const Rect2i &rect) const {
    cv::Rect roi(rect.position.x, rect.position.y, rect.size.x, rect.size.y);
    return _from_mat(mat(roi).clone());
}

Ref<CVImage> CVImage::flip(int flip_code) const {
    cv::Mat result; cv::flip(mat, result, flip_code); return _from_mat(result);
}

Ref<CVImage> CVImage::rotate(int rotate_code) const {
    cv::Mat result; cv::rotate(mat, result, rotate_code); return _from_mat(result);
}

Ref<CVImage> CVImage::warp_affine(const PackedFloat64Array &matrix, const Vector2i &dsize) const {
    if (matrix.size() < 6) return _from_mat(mat.clone());
    cv::Mat m = (cv::Mat_<double>(2, 3) << matrix[0], matrix[1], matrix[2], matrix[3], matrix[4], matrix[5]);
    cv::Mat result; cv::warpAffine(mat, result, m, cv::Size(dsize.x, dsize.y)); return _from_mat(result);
}

void CVImage::draw_line(const Vector2i &pt1, const Vector2i &pt2, const Color &color, int thickness) {
    cv::line(mat, cv::Point(pt1.x, pt1.y), cv::Point(pt2.x, pt2.y), color_to_scalar(color), thickness);
}

void CVImage::draw_rectangle(const Rect2i &rect, const Color &color, int thickness) {
    cv::rectangle(mat, cv::Rect(rect.position.x, rect.position.y, rect.size.x, rect.size.y), color_to_scalar(color), thickness);
}

void CVImage::draw_circle(const Vector2i &center, int radius, const Color &color, int thickness) {
    cv::circle(mat, cv::Point(center.x, center.y), radius, color_to_scalar(color), thickness);
}

void CVImage::draw_text(const String &text, const Vector2i &org, int font_face, double font_scale, const Color &color, int thickness) {
    std::string std_text = std::string(text.utf8().get_data());
    cv::putText(mat, std_text, cv::Point(org.x, org.y), font_face, font_scale, color_to_scalar(color), thickness);
}

Array CVImage::detect_haar_cascade(const String &cascade_path, double scale_factor, int min_neighbors, const Vector2i &min_size, const Vector2i &max_size) {
    Array result;
    cv::CascadeClassifier cascade;
    std::string std_path = std::string(cascade_path.utf8().get_data());
    if (!cascade.load(std_path)) {
        UtilityFunctions::printerr("CVImage: Failed to load cascade: ", cascade_path);
        return result;
    }
    cv::Mat gray;
    if (mat.channels() > 1) cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY); else gray = mat;
    std::vector<cv::Rect> detections;
    cascade.detectMultiScale(gray, detections, scale_factor, min_neighbors, 0, cv::Size(min_size.x, min_size.y), cv::Size(max_size.x, max_size.y));
    for (const auto &r : detections) result.append(Rect2i(r.x, r.y, r.width, r.height));
    return result;
}

Ref<CVImage> CVImage::equalize_hist() const {
    cv::Mat gray, result;
    if (mat.channels() > 1) cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY); else gray = mat;
    cv::equalizeHist(gray, result); return _from_mat(result);
}

Array CVImage::find_contours(int mode, int method) const {
    Array result;
    cv::Mat gray;
    if (mat.channels() > 1) cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY); else gray = mat;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(gray, contours, mode, method);
    for (const auto &contour : contours) {
        PackedVector2Array pts;
        for (const auto &pt : contour) pts.append(Vector2(pt.x, pt.y));
        result.append(pts);
    }
    return result;
}

Color CVImage::get_pixel(int x, int y) const {
    if (x < 0 || y < 0 || x >= mat.cols || y >= mat.rows) return Color();
    switch (mat.channels()) {
        case 1: { uint8_t v = mat.at<uint8_t>(y, x); float f = v / 255.0f; return Color(f, f, f, 1.0f); }
        case 3: { cv::Vec3b v = mat.at<cv::Vec3b>(y, x); return Color(v[2]/255.0f, v[1]/255.0f, v[0]/255.0f, 1.0f); }
        case 4: { cv::Vec4b v = mat.at<cv::Vec4b>(y, x); return Color(v[2]/255.0f, v[1]/255.0f, v[0]/255.0f, v[3]/255.0f); }
    }
    return Color();
}

void CVImage::set_pixel(int x, int y, const Color &color) {
    if (x < 0 || y < 0 || x >= mat.cols || y >= mat.rows) return;
    switch (mat.channels()) {
        case 1: mat.at<uint8_t>(y, x) = (uint8_t)(color.r * 255); break;
        case 3: mat.at<cv::Vec3b>(y, x) = cv::Vec3b((uint8_t)(color.b*255), (uint8_t)(color.g*255), (uint8_t)(color.r*255)); break;
        case 4: mat.at<cv::Vec4b>(y, x) = cv::Vec4b((uint8_t)(color.b*255), (uint8_t)(color.g*255), (uint8_t)(color.r*255), (uint8_t)(color.a*255)); break;
    }
}

Ref<CVImage> CVImage::add(const Ref<CVImage> &other) const { cv::Mat r; cv::add(mat, other->mat, r); return _from_mat(r); }
Ref<CVImage> CVImage::subtract(const Ref<CVImage> &other) const { cv::Mat r; cv::subtract(mat, other->mat, r); return _from_mat(r); }
Ref<CVImage> CVImage::multiply(double scalar) const { cv::Mat r; mat.convertTo(r, -1, scalar); return _from_mat(r); }
Ref<CVImage> CVImage::bitwise_and(const Ref<CVImage> &other) const { cv::Mat r; cv::bitwise_and(mat, other->mat, r); return _from_mat(r); }
Ref<CVImage> CVImage::bitwise_or(const Ref<CVImage> &other) const { cv::Mat r; cv::bitwise_or(mat, other->mat, r); return _from_mat(r); }
Ref<CVImage> CVImage::bitwise_not() const { cv::Mat r; cv::bitwise_not(mat, r); return _from_mat(r); }
Ref<CVImage> CVImage::absdiff(const Ref<CVImage> &other) const { cv::Mat r; cv::absdiff(mat, other->mat, r); return _from_mat(r); }
Ref<CVImage> CVImage::in_range(const Color &lower, const Color &upper) const { cv::Mat r; cv::inRange(mat, color_to_scalar(lower), color_to_scalar(upper), r); return _from_mat(r); }
