#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

#include <cassert>
#include <cmath>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos) {
  Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

  Eigen::Matrix4f translate;
  translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1, -eye_pos[2],
      0, 0, 0, 1;

  view = translate * view;

  return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle) {
  Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

  // TODO: Implement this function
  // Create the model matrix for rotating the triangle around the Z axis.
  // Then return it.

  Eigen::Matrix4f rotate;
  auto radian = rotation_angle / 180.0f * MY_PI;
  auto cosAngle = std::cos(radian);
  auto sinAngle = std::sin(radian);
  rotate << cosAngle, -sinAngle, 0, 0, sinAngle, cosAngle, 0, 0, 0, 0, 1, 0, 0,
      0, 0, 1;
  model = rotate * model;

  return model;
}

// https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
Eigen::Matrix4f get_model_matrix(Eigen::Vector3f axis, float rotation_angle) {
  double radian = rotation_angle / 180.0f * MY_PI;

  Eigen::Vector4f axis_col;
  Eigen::RowVector4f axis_row;
  axis_col << axis.x(), axis.y(), axis.z(), 0;
  axis_row << axis.x(), axis.y(), axis.z(), 0;

  Eigen::Matrix4f N;
  N << 0, -axis.z(), axis.y(), 0, axis.z(), 0, -axis.x(), 0, -axis.y(),
      axis.x(), 0, 0, 0, 0, 0, 1;

  Eigen::Matrix4f model = std::cos(radian) * Eigen::Matrix4f::Identity() +
                          (1 - std::cos(radian)) * (axis_col * axis_row) +
                          std::sin(radian) * N;

  model(3, 3) = 1;

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
  // 所以需要移动-(zNear + zFar) / 2
  trans << 1, 0, 0, 0,              //
      0, 1, 0, 0,                   //
      0, 0, 1, -(zNear + zFar) / 2, //
      0, 0, 0, 1;                   //

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

int main(int argc, const char **argv) {
  float angle = 0;
  bool command_line = false;
  std::string filename = "output.png";

  if (argc >= 3) {
    command_line = true;
    angle = std::stof(argv[2]); // -r by default
    if (argc == 4) {
      filename = std::string(argv[3]);
    }
  }

  rst::rasterizer r(700, 700);

  Eigen::Vector3f eye_pos = {0, 0, 5};

  std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

  std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

  auto pos_id = r.load_positions(pos);
  auto ind_id = r.load_indices(ind);

  int key = 0;
  int frame_count = 0;

  if (command_line) {
    r.clear(rst::Buffers::Color | rst::Buffers::Depth);

    r.set_model(get_model_matrix(angle));
    r.set_view(get_view_matrix(eye_pos));
    r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

    r.draw(pos_id, ind_id, rst::Primitive::Triangle);
    cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
    image.convertTo(image, CV_8UC3, 1.0f);

    cv::imwrite(filename, image);

    return 0;
  }

  while (key != 27) {
    r.clear(rst::Buffers::Color | rst::Buffers::Depth);

    r.set_model(get_model_matrix(angle));
    r.set_view(get_view_matrix(eye_pos));
    r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

    // 这里其实就是生成一个用于显示的数组(实现上将二维数组一维化了)
    // 在set_pixel中更新了frame_buf数组
    r.draw(pos_id, ind_id, rst::Primitive::Triangle);

    cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
    image.convertTo(image, CV_8UC3, 1.0f);
    cv::imshow("image", image);
    key = cv::waitKey(10);

    std::cout << "frame count: " << frame_count++ << '\n';

    if (key == 'a') {
      angle += 10;
    } else if (key == 'd') {
      angle -= 10;
    }
  }

  return 0;
}
