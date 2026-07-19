#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <filesystem>
#include "input/MouseEvent.h"

class Img {
public:
    Img();
    
    /**
     * Load image from path and optionally resize.
     * 
     * @param path Image file to load
     * @param size Target size in pixels (width, height). If empty, keep original
     * @param keep_aspect If true, shrink so the longer side fits size while preserving aspect ratio
     * @param interpolation OpenCV interpolation flag (e.g., cv::INTER_AREA for shrink, cv::INTER_LINEAR for enlarge)
     * @return Reference to this object for method chaining
     */
    Img& create(int width, int height, const cv::Scalar& color);

    Img& read(const std::filesystem::path& path,
              const std::pair<int, int>& size = {},
              bool keep_aspect = false,
              int interpolation = cv::INTER_AREA);
    
    /**
     * Draw this image onto another image at position (x, y)
     * 
     * @param other_img The target image to draw on
     * @param x X coordinate for top-left corner
     * @param y Y coordinate for top-left corner
     */
    void draw_on(Img& other_img, int x, int y) const;
    
    /**
     * Put text on the image
     * 
     * @param txt Text to draw
     * @param x X coordinate for text position
     * @param y Y coordinate for text position (baseline)
     * @param font_size Font scale factor
     * @param color Text color (BGR or BGRA)
     * @param thickness Text thickness
     */
    void put_text(const std::string& txt, int x, int y, double font_size,
                  const cv::Scalar& color = cv::Scalar(255, 255, 255, 255),
                  int thickness = 1);

    /**
     * Fill a solid rectangle on the image
     *
     * @param x X coordinate of the top-left corner
     * @param y Y coordinate of the top-left corner
     * @param width Rectangle width in pixels
     * @param height Rectangle height in pixels
     * @param color Fill color (BGR or BGRA)
     */
    void fill_rect(int x, int y, int width, int height, const cv::Scalar& color);

    static int text_width(const std::string& txt, double font_size, int thickness = 1);
    
    Img clone() const;

    /**
     * Display the image in a window
     */
    void show();

    /**
     * Display the image and pump window events for wait_ms.
     *
     * @return Code of the key pressed during the wait, or -1 if none
     */
    int show(int wait_ms);

    /**
     * Block until a key is pressed in any open image window
     */
    static void wait_for_key();

    /**
     * Pump window events (mouse, keyboard) for wait_ms.
     *
     * @return Code of the key pressed during the wait, or -1 if none
     */
    static int wait_key(int wait_ms);

    /**
     * Register the handler that receives mouse clicks on the image window.
     * Creates the window if needed; a later call replaces the handler.
     */
    static void set_mouse_handler(MouseHandler handler);
    
    /**
     * Get the underlying OpenCV Mat
     */
    const cv::Mat& get_mat() const { return img; }
    
    /**
     * Check if image is loaded
     */
    bool is_loaded() const { return !img.empty(); }

    int width() const { return img.cols; }
    int height() const { return img.rows; }

private:
    cv::Mat img;
}; 