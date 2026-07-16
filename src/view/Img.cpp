#include "../../include/view/Img.h"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
    const std::string kWindowName = "Image";

    // cv::imread cannot open non-ASCII paths on Windows, so the file is
    // read through std::ifstream (which handles Unicode paths natively)
    // and decoded from memory.
    cv::Mat decodeImageFile(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file) return {};
        std::vector<char> bytes((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());
        if (bytes.empty()) return {};
        return cv::imdecode(cv::Mat(1, static_cast<int>(bytes.size()), CV_8UC1, bytes.data()),
                            cv::IMREAD_UNCHANGED);
    }
}

Img::Img() {
    // Constructor - img is automatically initialized as empty
}

Img& Img::read(const std::filesystem::path& path,
               const std::pair<int, int>& size,
               bool keep_aspect,
               int interpolation) {
    img = decodeImageFile(path);
    if (img.empty()) {
        throw std::runtime_error("Cannot load image: " + path.string());
    }

    if (size.first != 0 && size.second != 0) {  // Check if size is not empty
        int target_w = size.first;
        int target_h = size.second;
        int h = img.rows;
        int w = img.cols;

        if (keep_aspect) {
            double scale = std::min(static_cast<double>(target_w) / w, 
                                   static_cast<double>(target_h) / h);
            int new_w = static_cast<int>(w * scale);
            int new_h = static_cast<int>(h * scale);
            cv::resize(img, img, cv::Size(new_w, new_h), 0, 0, interpolation);
        } else {
            cv::resize(img, img, cv::Size(target_w, target_h), 0, 0, interpolation);
        }
    }

    return *this;
}

void Img::draw_on(Img& other_img, int x, int y) const {
    if (img.empty() || other_img.img.empty()) {
        throw std::runtime_error("Both images must be loaded before drawing.");
    }

    // A 4-channel source keeps its alpha (the blend below composites it
    // onto a 3- or 4-channel target); only an opaque 3-channel source
    // needs channel equalization before the plain copy.
    cv::Mat source_img = img;
    cv::Mat target_img = other_img.img;

    if (source_img.channels() == 3 && target_img.channels() == 4) {
        cv::cvtColor(source_img, source_img, cv::COLOR_BGR2BGRA);
    }

    int h = source_img.rows;
    int w = source_img.cols;
    int H = target_img.rows;
    int W = target_img.cols;

    if (y + h > H || x + w > W) {
        throw std::runtime_error("Image does not fit at the specified position.");
    }

    cv::Rect roi_rect(x, y, w, h);
    cv::Mat roi = target_img(roi_rect);

    if (source_img.channels() == 4) {
        std::vector<cv::Mat> channels;
        cv::split(source_img, channels);

        // Per-pixel alpha blend (result = src*a + dst*(1-a)), so the
        // anti-aliased semi-transparent edge pixels a downscale produces
        // merge smoothly with the board instead of being copied opaque.
        cv::Mat alpha;
        channels[3].convertTo(alpha, CV_32F, 1.0 / 255.0);
        cv::Mat alpha_bgr;
        cv::merge(std::vector<cv::Mat>{alpha, alpha, alpha}, alpha_bgr);

        cv::Mat src_bgr;
        cv::merge(std::vector<cv::Mat>{channels[0], channels[1], channels[2]}, src_bgr);
        src_bgr.convertTo(src_bgr, CV_32FC3);

        cv::Mat roi_bgr;
        if (roi.channels() == 4) {
            cv::cvtColor(roi, roi_bgr, cv::COLOR_BGRA2BGR);
        } else {
            roi_bgr = roi.clone();
        }
        roi_bgr.convertTo(roi_bgr, CV_32FC3);

        cv::Mat inverse_alpha;
        cv::subtract(cv::Scalar::all(1.0), alpha_bgr, inverse_alpha);
        cv::Mat blended = src_bgr.mul(alpha_bgr) + roi_bgr.mul(inverse_alpha);
        blended.convertTo(blended, CV_8UC3);

        if (roi.channels() == 4) {
            cv::cvtColor(blended, roi, cv::COLOR_BGR2BGRA);
        } else {
            blended.copyTo(roi);
        }
    } else {
        source_img.copyTo(roi);
    }
}

void Img::put_text(const std::string& txt, int x, int y, double font_size,
                   const cv::Scalar& color, int thickness) {
    if (img.empty()) {
        throw std::runtime_error("Image not loaded.");
    }
    
    cv::putText(img, txt, cv::Point(x, y),
                cv::FONT_HERSHEY_SIMPLEX, font_size,
                color, thickness, cv::LINE_AA);
}

void Img::fill_rect(int x, int y, int width, int height, const cv::Scalar& color) {
    if (img.empty()) {
        throw std::runtime_error("Image not loaded.");
    }
    if (width <= 0 || height <= 0) return;

    cv::rectangle(img, cv::Rect(x, y, width, height), color, cv::FILLED);
}

Img Img::clone() const {
    Img copy;
    copy.img = img.clone();
    return copy;
}

void Img::show() {
    if (img.empty()) {
        throw std::runtime_error("Image not loaded.");
    }

    cv::imshow(kWindowName, img);
    cv::waitKey(0);
    cv::destroyAllWindows();
}

void Img::show(int wait_ms) {
    if (img.empty()) {
        throw std::runtime_error("Image not loaded.");
    }

    cv::imshow(kWindowName, img);
    cv::waitKey(wait_ms);
}

void Img::wait_for_key() {
    cv::waitKey(0);
}