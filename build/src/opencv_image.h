#ifndef CV_IMAGE_H
#define CV_IMAGE_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/features2d.hpp>

namespace godot {

class CVImage : public RefCounted {
    GDCLASS(CVImage, RefCounted);

private:
    cv::Mat mat;

protected:
    static void _bind_methods();

public:
    CVImage();
    ~CVImage();

    void create(int width, int height, int type);
    void create_from_image(const Ref<Image> &p_image);
    Ref<Image> to_image() const;
    Ref<ImageTexture> to_image_texture() const;

    bool load(const String &path);
    bool save(const String &path);

    int get_width() const;
    int get_height() const;
    int get_channels() const;
    bool is_empty() const;
    Vector2i get_size() const;

    Ref<CVImage> cvt_color(int code) const;
    Ref<CVImage> to_grayscale() const;
    Ref<CVImage> to_rgb() const;
    Ref<CVImage> to_hsv() const;

    Ref<CVImage> blur(const Vector2i &ksize) const;
    Ref<CVImage> gaussian_blur(const Vector2i &ksize, float sigma_x, float sigma_y) const;
    Ref<CVImage> median_blur(int ksize) const;
    Ref<CVImage> bilateral_filter(int d, double sigma_color, double sigma_space) const;

    Ref<CVImage> canny(double threshold1, double threshold2, int aperture_size, bool l2_gradient) const;
    Ref<CVImage> sobel(int ddepth, int dx, int dy, int ksize) const;
    Ref<CVImage> laplacian(int ddepth, int ksize) const;

    Ref<CVImage> threshold(double thresh, double maxval, int type) const;
    Ref<CVImage> adaptive_threshold(double maxval, int method, int type, int block_size, double c) const;

    Ref<CVImage> dilate(const Vector2i &ksize, int iterations) const;
    Ref<CVImage> erode(const Vector2i &ksize, int iterations) const;
    Ref<CVImage> morphology_ex(int op, const Vector2i &ksize) const;

    Ref<CVImage> resize(const Vector2i &new_size, int interpolation) const;
    Ref<CVImage> crop(const Rect2i &rect) const;
    Ref<CVImage> flip(int flip_code) const;
    Ref<CVImage> rotate(int rotate_code) const;
    Ref<CVImage> warp_affine(const PackedFloat64Array &matrix, const Vector2i &dsize) const;

    void draw_line(const Vector2i &pt1, const Vector2i &pt2, const Color &color, int thickness);
    void draw_rectangle(const Rect2i &rect, const Color &color, int thickness);
    void draw_circle(const Vector2i &center, int radius, const Color &color, int thickness);
    void draw_text(const String &text, const Vector2i &org, int font_face, double font_scale, const Color &color, int thickness);

    Array detect_haar_cascade(const String &cascade_path, double scale_factor, int min_neighbors, const Vector2i &min_size, const Vector2i &max_size);

    Ref<CVImage> equalize_hist() const;
    Array find_contours(int mode, int method) const;

    Color get_pixel(int x, int y) const;
    void set_pixel(int x, int y, const Color &color);

    Ref<CVImage> add(const Ref<CVImage> &other) const;
    Ref<CVImage> subtract(const Ref<CVImage> &other) const;
    Ref<CVImage> multiply(double scalar) const;
    Ref<CVImage> bitwise_and(const Ref<CVImage> &other) const;
    Ref<CVImage> bitwise_or(const Ref<CVImage> &other) const;
    Ref<CVImage> bitwise_not() const;
    Ref<CVImage> absdiff(const Ref<CVImage> &other) const;
    Ref<CVImage> in_range(const Color &lower, const Color &upper) const;

    cv::Mat &get_mat();
    const cv::Mat &get_mat() const;
    void set_mat(const cv::Mat &p_mat);

    static Ref<CVImage> _from_mat(const cv::Mat &p_mat);

private:
    static cv::Scalar color_to_scalar(const Color &c);
};

} // namespace godot

#endif
