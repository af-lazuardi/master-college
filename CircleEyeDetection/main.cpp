/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <opencv2/imgproc.hpp>


#include <opencv2/imgproc/types_c.h>


/* 
 * File:   main.cpp
 * Author: RIF <arif.lazuardi@infoglobal.co.id>
 *
 * Created on September 12, 2017, 3:06 PM
 */

#include <cstdlib>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "GUI.h"

#define PUPIL 1
#define IRIS 2
#define PI 3.14159265359


static cv::String WIN_NAME_ORI = "Original Eye";
static cv::String WIN_NAME_IRIS = "Iris Eye";
static cv::String WIN_NAME_PUPIL = "Iris Eye Without Pupil";
static cv::String WIN_NAME_IRIS_LINE_DETECT = "Iris Eye Line Detection";
static cv::String WIN_NAME_RECT = "Iris Transform to Rectangle";


cv::Mat laplacian_line_detection(cv::Mat imsource) {
    cv::Mat gray,dst,abs_dst;
    int ddepth = CV_8U;
    int kernel_size = 3;
    int scale = 2;
    int delta = 0;
    
    cv::GaussianBlur( imsource, imsource, cv::Size(3,3), 0, 0, cv::BORDER_DEFAULT );
    cv::cvtColor(imsource,gray,cv::COLOR_RGB2GRAY);
    cv::Laplacian( gray, dst, ddepth, kernel_size, scale, delta, cv::BORDER_DEFAULT );
    cv::convertScaleAbs( dst, abs_dst );
    
    return abs_dst;
}

cv::Mat sobel_line_detection(cv::Mat imsource) {
    cv::Mat grad_x, grad_y;
    cv::Mat abs_grad_x, abs_grad_y;
    cv::Mat gray, grad;
    int ddepth = CV_8U;
    int scale = 1;
    int delta = 0;
    
    cv::GaussianBlur( imsource, imsource, cv::Size(3,3), 0, 0, cv::BORDER_DEFAULT );
    cv::cvtColor(imsource,gray,cv::COLOR_RGB2GRAY);
    cv::Sobel( gray, grad_x, ddepth, 1, 0, 3, scale, delta, cv::BORDER_DEFAULT );
    cv::Sobel( gray, grad_y, ddepth, 0, 1, 3, scale, delta, cv::BORDER_DEFAULT );
    cv::convertScaleAbs( grad_x, abs_grad_x );
    cv::convertScaleAbs( grad_y, abs_grad_y );
    
    addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );
    
    return grad;
}

int circumference(int radius) {
    return 2*PI*radius;
}

cv::Mat get_half_rect(cv::Mat im_src) {
    
}

cv::Point RotatePoint(const cv::Point& p, float rad)
{
    const float x = std::cos(rad) * p.x - std::sin(rad) * p.y;
    const float y = std::sin(rad) * p.x + std::cos(rad) * p.y;

    const cv::Point rot_p((int)round(x),(int)round(y));
    return rot_p;
}

cv::Point RotatePoint(const cv::Point& cen_pt, const cv::Point& p, float rad)
{
    const cv::Point trans_pt = p - cen_pt;
    const cv::Point rot_pt   = RotatePoint(trans_pt, rad);
    const cv::Point fin_pt   = rot_pt + cen_pt;

    return fin_pt;
}

float degree_to_rad(float degree) {
    return degree * (PI/180.0f);
}

float rad_to_degree(float rad) {
    return rad * (180.0f/PI);
}

float find_degree_from_2point(cv::Point p1, cv::Point p2) {
    float angle = atan2(p1.y - p2.y, p1.x - p2.x);
    return rad_to_degree(angle);
}

cv::Mat mask_triangle(cv::Mat im_src, cv::Point a, cv::Point b, cv::Point c) {
    cv::Mat mask = cv::Mat::zeros(im_src.size(),im_src.type());
    int lineType = 8;
    
    cv::Point rook_points[1][3];
    rook_points[0][0] = a;
    rook_points[0][1] = b;
    rook_points[0][2] = c;

    const cv::Point* ppt[1] = { rook_points[0] };
    int npt[] = { 3 };
    
    cv::fillPoly( mask, ppt, npt,1, cv::Scalar(255),lineType );
  
    cv::Mat im_src_color(im_src.size(),CV_64FC3);
    cv::cvtColor(im_src,im_src_color,cv::COLOR_GRAY2RGB);
    
    cv::Mat dst(im_src.size(),CV_64FC3,cv::Scalar(255,255,255));
    im_src.copyTo(dst,mask); // copy values of img to dst if mask is > 0.
    
    int distance = (int)round(GUI::get_euclidean_distance(b, c));
    cv::Point c_mirror(a.x+round(distance/2.0),im_src.rows);
    
    
    
    float degree = find_degree_from_2point(c,c_mirror);
    std::cout << degree << std::endl;
    return dst;
}


cv::Mat circle_to_rectangle(cv::Mat im_src) {
    cv::Mat cp_im_src = im_src.clone();
    
    int width, height;
    std::cout << im_src.rows << ":"  << im_src.cols << std::endl;
    height = (int) round(im_src.rows/2.0);
    width = circumference(height);
    
    cv::Point center(height,height);
    cv::Point end(0,height);
    cv::Point new_end = end;
    int i =0;
    float degree_rotate = 0;
    while(i < 360) {
        GUI::create_line(cp_im_src,center,new_end);
        
        degree_rotate += 45;
        i = (int)degree_rotate;
        
        cv::Point temp = new_end;
        new_end = RotatePoint(center,end,degree_to_rad(degree_rotate));
        
        GUI::create_line(cp_im_src,temp,new_end);
        
        //get triangle
        cv::Mat mask = mask_triangle(im_src,center,temp,new_end);
        cv::imshow("mask"+i,mask);
        
    }
    
    return cp_im_src;
}

/*
 * 
 */
int main(int argc, char** argv) {
    cv::namedWindow(WIN_NAME_ORI,cv::WINDOW_AUTOSIZE);
    
    cv::Mat im_orig_eye = cv::imread("blue-iris-529x351.jpg");
    
    cv::Mat im_iris = GUI::getCircle(im_orig_eye, IRIS, WIN_NAME_ORI);
    //cv::imshow(WIN_NAME_IRIS,im_iris);
    
    cv::Mat im_pupil = GUI::getCircle(im_iris, PUPIL, WIN_NAME_ORI);
    cv::imshow(WIN_NAME_PUPIL,im_pupil);
    
    cv::Mat im_line_detection = laplacian_line_detection(im_pupil);
    cv::imshow(WIN_NAME_IRIS_LINE_DETECT,im_line_detection);
    
    cv::Mat im_rectangle = circle_to_rectangle(im_line_detection);
    cv::imshow(WIN_NAME_RECT,im_rectangle);
    
    cv::waitKey(0);
    //cv::Rect rect = GUI::getRect(im_orig_eye, WIN_NAME);
    return 0;
}

