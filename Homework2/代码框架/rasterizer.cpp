// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


#include <cassert>
#include <limits>

rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    // 叉乘&&结果全大于0或全小于0

    //assert(std::fpclassify(_v[0].z()) ==  FP_ZERO);
    //assert(std::fpclassify(_v[1].z()) ==  FP_ZERO);
    //assert(std::fpclassify(_v[2].z()) ==  FP_ZERO);

    Eigen::Vector3f a(_v[1].x() - _v[0].x(), _v[1].y() - _v[0].y(), 0);
    Eigen::Vector3f b(_v[2].x() - _v[1].x(), _v[2].y() - _v[1].y(), 0);
    Eigen::Vector3f c(_v[0].x() - _v[2].x(), _v[0].y() - _v[2].y(), 0);


    Eigen::Vector3f i(x - _v[0].x(), y - _v[0].y(), 0);
    Eigen::Vector3f j(x - _v[1].x(), y - _v[1].y(), 0);
    Eigen::Vector3f k(x - _v[2].x(), y - _v[2].y(), 0);

    // 叉乘&取模
    float m = a.cross(i).z();
    float n = b.cross(j).z();
    float o = c.cross(k).z();

    return m > 0.0f && n > 0.0f && o > 0.0f;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            // ppt05's Canonical Cube to Screen
            // Mviewport
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);

            // 我们之前的z轴是进行了翻转的,也就是最后乘了一个镜像矩阵
            assert(vert.z() < 0.0f);
            // 这里只是将z从[-1, 1]映射到[n, f], 并没有进行翻转
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}


//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    // 这里不应该直接将w的值设置为1, 其实它的值应该是-z
    
    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle

    // todo fix:这里存在类型转换
    auto leftBound = std::numeric_limits<float>::max();
    auto rightBound = std::numeric_limits<float>::min();
    auto upBound = std::numeric_limits<float>::max();
    auto buttonBound = std::numeric_limits<float>::min();
    for (const auto& k : v) {
        leftBound = std::min(leftBound, k.x());
        upBound = std::min(upBound, k.y());

        rightBound = std::max(rightBound, k.x());
        buttonBound = std::max(buttonBound, k.y());
    }

    std::array<float, 5> offset{0.25f, 0.25f, 0.75f, 0.75f, 0.25f};
    /*
        (i+0.25, j+0.25)     (i+0.25, j+0.75)
        (i+0.75, j+0.75)     (i+0.75, j+0.75)
    */
    // todo: msaa
    int horizontalMin = std::floor(leftBound), horizontalMax = std::ceil(rightBound);
    int verticalMin = std::floor(upBound), verticalMax = std::ceil(buttonBound);
    for (float i = horizontalMin; i <= rightBound; ++i) {
        for (float j = verticalMin; j <= verticalMax; ++j) {
            // msaa
            auto mass_index = get_index(i, j) * 4;
            float minDepth = -std::numeric_limits<float>::infinity();
            for (int k = 0; k < 4; ++k) {
                if(insideTriangle(i + offset[k], j + offset[k+1], t.v)) {
                    auto[alpha, beta, gamma] = computeBarycentric2D(i + offset[k], j + offset[k+1], t.v);
                    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                    z_interpolated *= w_reciprocal;
                    assert(z_interpolated > 0.0f);
                    if (z_interpolated > msaa_depth_buf[mass_index + k]) {
                        msaa_depth_buf[mass_index + k] = z_interpolated;
                        msaa_frame_buf[mass_index + k] = t.getColor() / 4;
                        minDepth = std::max(minDepth, msaa_depth_buf[mass_index + k]);
                    }
                }
            }

            auto color = msaa_frame_buf[mass_index] + msaa_frame_buf[mass_index + 1] + msaa_frame_buf[mass_index + 2] + msaa_frame_buf[mass_index + 3];
            set_pixel({i, j, 1}, color);
            depth_buf[get_index(i, j)] = minDepth;
            
            
            /*
            if(insideTriangle(i, j, t.v)) {
                auto[alpha, beta, gamma] = computeBarycentric2D(i + 0.5, j + 0.5, t.v);
                float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                z_interpolated *= w_reciprocal;
                if (z_interpolated > depth_buf[get_index(i, j)]) {
                    set_pixel({i, j, 1}, t.getColor()); // 更新颜色
                    depth_buf[get_index(i, j)] = z_interpolated;
                }
            }
            */
            
        }
    }
    // If so, use the following code to get the interpolated z value.
    //auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
    //float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
    //float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
    //z_interpolated *= w_reciprocal;

    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
}

/*
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    float alpha, beta, gamma, lmin=INT_MAX, rmax=INT_MIN, tmax=INT_MIN, bmin=INT_MAX;
    // TODO : Find out the bounding box of current triangle.
    for(auto &k:v){//找到bounding box的边界坐标
        lmin = int(std::min(lmin,k.x()));
        rmax = std::max(rmax,k.x());rmax = rmax == int(rmax) ? int(rmax)-1 : rmax;
        tmax = std::max(tmax,k.y());tmax = tmax == int(tmax) ? int(tmax)-1 : tmax;
        bmin = int(std::min(bmin,k.y()));
    }
    // iterate through the pixel and find if the current pixel is inside the triangle
    for(float i = lmin; i <= rmax; i++){//遍历bounding box像素
        for(float j = bmin; j <= tmax; j++){
            if(insideTriangle(i, j, t.v)){//如果在三角形内
                // If so, use the following code to get the interpolated z value.
                std::tie(alpha, beta, gamma) = computeBarycentric2D(i+0.5, j+0.5, t.v);//对当前像素坐标z值插值

                float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                z_interpolated *= w_reciprocal;
                //assert(w_reciprocal > 0.0f);
                assert(z_interpolated < 0.0f);
                if(z_interpolated > depth_buf[get_index(i,j)]){//如果当前z值比像素z值小（这里是把z值换成正数比较的）
                    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
                    set_pixel({i,j,1},t.getColor());
                    depth_buf[get_index(i,j)] = z_interpolated;//设置像素颜色，修改像素当前深度
                }
            }
        }
    }
}
*/

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});

        // set msaa buffer
        std::fill(msaa_frame_buf.begin(), msaa_frame_buf.end(), Eigen::Vector3f{0, 0, 0});
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        //std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
        // 初始化为-∞
        std::fill(depth_buf.begin(), depth_buf.end(), -std::numeric_limits<float>::infinity());
        // init msaa buffer to -infinity
        std::fill(msaa_depth_buf.begin(), msaa_depth_buf.end(), -std::numeric_limits<float>::infinity());
    }
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);

    // init msaa buffer
    msaa_frame_buf.resize(w * h *4);
    msaa_depth_buf.resize(w * h * 4);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on