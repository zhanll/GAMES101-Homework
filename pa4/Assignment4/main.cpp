#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <unordered_map>

std::vector<cv::Point2f> control_points;

/// AA begin

float color(float y, float x, cv::Mat& window)
{
	return window.at<cv::Vec3b>(y, x)[1];
}

float lerp(float x, float v0, float v1)
{
	return v0 + x * (v1 - v0);
}

int AA(const cv::Point2f& point, cv::Mat& window)
{
	// horizontal
	int R = point.y;
	float u = R + 0.5f;

	int C = point.x;
	float v = C + 0.5f;

	int R0 = std::clamp(R - 1, 0, 699);
	int R2 = std::clamp(R + 1, 0, 699);

	int u0 = R;
	int u1 = R2;
	if (point.y <= u)
	{
		u0 = R0;
		u1 = R;
	}
	float s = std::abs(point.y - R);

	// vertical
	int C0 = std::clamp(C - 1, 0, 699);
	int C2 = std::clamp(C + 1, 0, 699);

	int v0 = C;
	int v1 = C2;
	if (point.x <= v)
	{
		v0 = C0;
		v1 = C;
	}
	float t = std::abs(point.x - C);

	// lerp
	float a0 = lerp(s, color(u0, v0, window), color(u0, v1, window));
	float a1 = lerp(s, color(u1, v0, window), color(u1, v1, window));

	return lerp(t, a0, a1);
}

/// AA end

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm
    if (control_points.size() == 1)
    {
        return control_points[0];
    }
    
    std::vector<cv::Point2f> new_points;
    for (int iP=0; iP<control_points.size()-1; ++iP)
    {
        const auto& p1 = control_points[iP];
        const auto& p2 = control_points[iP+1];
        cv::Point2f np(p1.x*t+p2.x*(1-t), p1.y*t+p2.y*(1-t));
        new_points.push_back(np);
    }

    return recursive_bezier(new_points,  t);
}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
    std::vector<cv::Point2f> vPoints;
    for (float t=0.0; t<=1.0; t+=0.001)
    {
        auto point = recursive_bezier(control_points, t);
        window.at<cv::Vec3b>(point.y, point.x)[1] = 255;
        vPoints.push_back(point);
    }

    // AA
    std::vector<int> vColor;
    for (auto p : vPoints)
    {
        vColor.push_back( AA(p, window) );
    }

    for (int i=0; i<vColor.size(); ++i)
    {
        const auto p = vPoints[i];
        window.at <cv::Vec3b>(p.y, p.x)[1] = vColor[i];
    }
}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4) 
        {
            naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
