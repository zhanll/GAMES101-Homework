//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        u = std::max(u, 0.0f);
        v = std::max(v, 0.0f);
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

    Eigen::Vector3f getColorBilinear(float u, float v)
    {
		u = std::max(u, 0.0f);
		v = std::max(v, 0.0f);

        float u_ = width * u;
        float v_ = height * (1 - v);

        int x0 = u_;
        int x1 = std::min(x0 + 1, width - 1);
        int y0 = v_;
        int y1 = std::min(y0 + 1, height - 1);

        float s = u_ - x0;
        float t = v_ - y0;

        auto c00 = image_data.at<cv::Vec3b>(y0, x0);
        auto c10 = image_data.at<cv::Vec3b>(y0, x1);
        auto c01 = image_data.at<cv::Vec3b>(y1, x0);
        auto c11 = image_data.at<cv::Vec3b>(y1, x1);

        auto c0 = c00 + s * (c10 - c00);
        auto c1 = c01 + s * (c11 - c01);
        auto c = c0 + t * (c1 - c0);

        return Eigen::Vector3f(c[0], c[1], c[2]);
    }

};
#endif //RASTERIZER_TEXTURE_H
