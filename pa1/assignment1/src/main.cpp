#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double M_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate <<
        1, 0, 0, -eye_pos[0],
        0, 1, 0, -eye_pos[1],
        0, 0, 1,-eye_pos[2],
        0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.

    float radian = M_PI * rotation_angle / 180; 
    float sina = std::sin(radian);
    float cosa = std::cos(radian);
    model <<    cosa,   -sina,  0,  0,
                        sina,   cosa,   0,  0,
                        0,          0,     1,  0,
                        0,          0,     0,  1;

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.

    float rad_fov = M_PI * eye_fov / 180;
    float n = -zNear;
    float f = -zFar;
    float t = -n * std::tan(rad_fov/2);
    float b = -t;
    float r = t * aspect_ratio;
    float l = -r;

    Eigen::Matrix4f p2o,o;

    p2o <<  n,  0,  0,  0,
                    0,  n,  0,  0,
                    0,  0,  n+f, -n*f,
                    0,  0,  1,  0;

    o <<    2/(r-l),    0,              0,              -(r+l)/2,
                0,              2/(t-b),   0,               -(t+b)/2,
                0,              0,              2/(n-f),    -(n+f)/2,
                0,              0,              0,                  1;

    projection = o * p2o;

    return projection;
}

// bonus
Eigen::Matrix4f get_rotation(Vector3f axis, float angle)
{
    float radian = M_PI * angle / 180;
    Vector3f n = axis.normalized();
    Eigen::Matrix3f I = Eigen::Matrix3f::Identity();
    Eigen::Matrix3f c;
    c << 0, -n.z(), n.y(),
        n.z(), 0, -n.x(),
        -n.y(), n.x(), 0;
    float cs = std::cos(radian);
    Eigen::Matrix3f mat3 = I * cs + n * n.transpose() * (1 - cs) + c * std::sin(radian);
    Eigen::Matrix4f mat4;
    mat4 <<
        mat3(0, 0), mat3(0, 1), mat3(0, 2), 0,
        mat3(1, 0), mat3(1, 1), mat3(1, 2), 0,
        mat3(2, 0), mat3(2, 1), mat3(2, 2), 0,
        0, 0, 0, 1;
    return mat4;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
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
        //r.set_model(get_rotation(Vector3f(0.0f, 0.0f, 1.0f), angle)); // bonus
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') {
            angle += 10;
        }
        else if (key == 'd') {
            angle -= 10;
        }
    }

    return 0;
}
