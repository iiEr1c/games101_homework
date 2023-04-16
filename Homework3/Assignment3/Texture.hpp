//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture {
private:
  cv::Mat image_data;

public:
  Texture(const std::string &name) {
    image_data = cv::imread(name);
    cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
    width = image_data.cols;
    height = image_data.rows;
  }

  int width, height;

  // https://zhuanlan.zhihu.com/p/419872527
  Eigen::Vector3f getColor(float u, float v) {
    // 坐标限定
    // 解决uv坐标溢出问题
    if (u < 0)
      u = 0;
    if (u > 1)
      u = 1;
    if (v < 0)
      v = 0;
    if (v > 1)
      v = 1;

    // 注意这里的uv坐标是基于左下角的
    // 而屏幕显示起点是左上角
    auto u_img = u * width;
    auto v_img = (1 - v) * height;
    auto color = image_data.at<cv::Vec3b>(v_img, u_img);
    return Eigen::Vector3f(color[0], color[1], color[2]);
  }

  Eigen::Vector3f getBilinearColor(float u, float v) {
    // 坐标限定
    // 解决uv坐标溢出问题
    if (u < 0)
      u = 0;
    if (u > 1)
      u = 1;
    if (v < 0)
      v = 0;
    if (v > 1)
      v = 1;


    auto u_img = u * width;
    auto v_img = (1 - v) * height;

    int u_low = std::floor(u_img), u_up = std::ceil(u_img);
    int v_low = std::floor(v_img), v_up = std::ceil(v_img);

    // 这里严格使用ppt上的位置定义
    float s = u_img - float(u_low);
    float t = v_img - float(v_low);

    // 左上->左下
    auto u0 = (1 - s) * image_data.at<cv::Vec3b>(v_low, u_low) + s *
    image_data.at<cv::Vec3b>(v_low, u_up);
    // 右上 -> 右下
    auto u1 = (1 - s) * image_data.at<cv::Vec3b>(v_up, u_low) + s *
    image_data.at<cv::Vec3b>(v_up, u_up);

    auto color = (1 - t) * u0 + t * u1;
    return Eigen::Vector3f(color[0], color[1], color[2]);
  }
};
#endif // RASTERIZER_TEXTURE_H
