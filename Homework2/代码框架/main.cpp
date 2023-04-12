// clang-format off
#include <iostream>
#include <opencv2/opencv.hpp>
#include "rasterizer.hpp"
#include "global.hpp"
#include "Triangle.hpp"

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1,0,0,-eye_pos[0],
                 0,1,0,-eye_pos[1],
                 0,0,1,-eye_pos[2],
                 0,0,0,1;

    view = translate*view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    return model;
}

// 这里的zNear和zFar表示的是距离, 而非坐标
// 参考: https://zhuanlan.zhihu.com/p/509902950
// 参考: https://zhuanlan.zhihu.com/p/464638515 => OpenGL NDS是左手系(x, y, -z)
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar) {
  // Students will implement this function

  Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

  // TODO: Implement this function
  // Create the projection matrix for the given parameters.
  // Then return it.

  assert(zNear > 0.0f);
  assert(zFar > 0.0f);

  // 修复中心对称问题
  // 见ppt4, 有标注

  // assert(zNear > zFar); // assert failed
  // 也就是说, 这里我们这里并不是看向-z

  float fovY = eye_fov / 180.0f * MY_PI;
  float t = std::tan(fovY) * zNear;
  float r = t * aspect_ratio;

  Eigen::Matrix4f trans;
  // fix, 因为near和far我们看作距离, 所以他们的坐标是-near和-far
  // 所以等于-1(-zNear + (-)(zFar)) / 2 = (zNear + zFar) / 2
  trans << 1, 0, 0, 0,             //
      0, 1, 0, 0,                  //
      0, 0, 1, -(zNear + zFar) / 2, //
      0, 0, 0, 1;                  //

  Eigen::Matrix4f scale;
  // 鉴于zNear > 0 && zFar > 0 && zNear < zFar, 在scale变换时要取abs,
  // 或者直接zFar - zNear
  scale << 1 / r, 0, 0, 0,         //
      0, 1 / t, 0, 0,              //
      0, 0, 2 / (zFar - zNear), 0, //
      0, 0, 0, 1;                  //

  Eigen::Matrix4f PerspToOrtho;
  // 注意第四行的参数是-1, 我们可以带入 矩阵A*(x, y, z, 1) = (x, y, z, 1)
  // z=-n时, 第四行第三列只能是-1
  PerspToOrtho << zNear, 0, 0, 0,           //
      0, zNear, 0, 0,                       //
      0, 0, -(zNear + zFar), -zNear * zFar, //
      0, 0, -1, 0;                          //

  Eigen::Matrix4f mirror;
  mirror << 1, 0, 0, 0, //
      0, 1, 0, 0,       //
      0, 0, -1, 0,      //
      0, 0, 0, 1;       //

  projection = mirror * scale * trans * PerspToOrtho * projection;
  return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc == 2)
    {
        command_line = true;
        filename = std::string(argv[1]);
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0,0,5};


    std::vector<Eigen::Vector3f> pos
            {
                    {2, 0, -2},
                    {0, 2, -2},
                    {-2, 0, -2},
                    {3.5, -1, -5},
                    {2.5, 1.5, -5},
                    {-1, 0.5, -5}
            };

    std::vector<Eigen::Vector3i> ind
            {
                    {0, 1, 2},
                    {3, 4, 5}
            };

    std::vector<Eigen::Vector3f> cols
            {
                    {217.0, 238.0, 185.0},
                    {217.0, 238.0, 185.0},
                    {217.0, 238.0, 185.0},
                    {185.0, 217.0, 238.0},
                    {185.0, 217.0, 238.0},
                    {185.0, 217.0, 238.0}
            };

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);
    auto col_id = r.load_colors(cols);

    int key = 0;
    int frame_count = 0;

    if (command_line)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);

        cv::imwrite(filename, image);

        return 0;
    }

    while(key != 27)
    {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, col_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::cvtColor(image, image, cv::COLOR_RGB2BGR);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';
    }

    return 0;
}
// clang-format on