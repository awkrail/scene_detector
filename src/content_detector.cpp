#include "opencv2/opencv.hpp"
#include "content_detector.hpp"
#include "frame_timecode.hpp"

#include <vector>
#include <optional>

namespace content_detector {

    std::optional<int32_t> ContentDetector::process_frame(video_frame::VideoFrame& next_frame) {
        frame_score_ = _calculate_frame_score(next_frame);
        const bool is_above_threshold = (frame_score_ > threshold_);
        const int32_t frame_num = next_frame.position.get_frame_num();
        std::optional<int32_t> cut = flash_filter_.filter(frame_num, is_above_threshold);
        return cut;
    }

    float ContentDetector::_calculate_frame_score(video_frame::VideoFrame& next_frame) {
        cv::cvtColor(next_frame.frame, next_frame.frame, cv::COLOR_BGR2HSV);
        if(!last_frame_.has_value()) {
            last_frame_ = next_frame.frame;
            return 0.0f;
        }
        const float delta = _mean_pixel_distance(next_frame);
        last_frame_ = next_frame.frame;
        return delta;
    }

    float ContentDetector::_mean_pixel_distance(video_frame::VideoFrame& next_frame) const {
        const int32_t num_pixels = next_frame.num_pixels;        
        cv::Mat pixel_diff;
        cv::absdiff(last_frame_.value(), next_frame.frame, pixel_diff);
        cv::Scalar pixel_diff_channel_sum = cv::sum(pixel_diff);
        const float pixel_diff_sum = static_cast<float>((pixel_diff_channel_sum[0] + pixel_diff_channel_sum[1] + pixel_diff_channel_sum[2]) / num_pixels / 3.0);
        return pixel_diff_sum;
    }
}