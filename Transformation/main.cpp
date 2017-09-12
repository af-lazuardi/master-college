/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: rif
 *
 * Created on September 4, 2017, 11:21 AM
 */

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <cstdlib>
#include <limits>

using namespace std;

cv::String WINDOW_NAME = "Transformation";

void normalize_point(int width, int height, cv::Point *point) {
    point->x = (width/2) + point->x;
    point->y = (height/2) - point->y;
}

vector<cv::Point> create_vector_point(cv::Point bottom_left, int width, int height) {
    cv::Point pt_1, pt_2, pt_3, pt_4, pt_5;
    
    pt_1.x = bottom_left.x; pt_1.y = bottom_left.y;
    pt_2.x = bottom_left.x+width; pt_2.y = bottom_left.y;
    pt_3.x = bottom_left.x+width; pt_3.y = bottom_left.y+height;
    pt_4.x = bottom_left.x; pt_4.y = bottom_left.y+height;
    pt_5.x = bottom_left.x; pt_5.y = bottom_left.y;
  
    vector<cv::Point> vector_pt;
    vector_pt.push_back(pt_1);
    vector_pt.push_back(pt_2);
    vector_pt.push_back(pt_3);
    vector_pt.push_back(pt_4);
    vector_pt.push_back(pt_5);
    
    return vector_pt;
}

void create_line(cv::Mat im, cv::Point start, cv::Point end, cv::Scalar color) {
    int thickness = 1;
    int line_type = cv::LINE_8;
    
    normalize_point(im.cols,im.rows,&start);
    normalize_point(im.cols,im.rows,&end);
    
    cv::line(im,start,end,color,thickness,line_type);
}

void draw_rectangle(cv::Mat im, vector<cv::Point> vector_pt, cv::Scalar color) {
    for(int i=0; i<vector_pt.size()-1; i++) {
        create_line(im,vector_pt[i],vector_pt[i+1],color);
    }
}

vector<cv::Point> m_rotasi(vector<cv::Point> vect_pt, double theta) {
    cv::Mat M(3,3,CV_64F);
    
    M.at<float>(0,0) = cos(theta);
    M.at<float>(0,1) = -sin(theta);
    M.at<float>(0,2) = 0;
    
    M.at<float>(1,0) = sin(theta);
    M.at<float>(1,1) = cos(theta);
    M.at<float>(1,2) = 0;
    
    M.at<float>(2,0) = 0;
    M.at<float>(2,1) = 0;
    M.at<float>(2,2) = 1;
    
    for(int i=0; i<vect_pt.size(); i++) {
        vect_pt[i].x = (M.at<float>(0,0)*vect_pt[i].y) + (M.at<float>(0,1)*vect_pt[i].x) + M.at<float>(0,2);
        vect_pt[i].y = (M.at<float>(1,0)*vect_pt[i].y) + (M.at<float>(1,1)*vect_pt[i].x) + M.at<float>(1,2);
    }
    return vect_pt;
}

vector<cv::Point> m_translasi(vector<cv::Point> vect_pt, int delta_x, int delta_y) {
    cv::Mat M(3,3,CV_64F);
    
    M.at<float>(0,0) = 1;
    M.at<float>(0,1) = 0;
    M.at<float>(0,2) = delta_x;
    
    M.at<float>(1,0) = 0;
    M.at<float>(1,1) = 1;
    M.at<float>(1,2) = delta_y;
    
    M.at<float>(2,0) = 0;
    M.at<float>(2,1) = 0;
    M.at<float>(2,2) = 1;
    
    for(int i=0; i<vect_pt.size(); i++) {
        vect_pt[i].x = M.at<float>(0,0)*vect_pt[i].x + M.at<float>(0,1)*vect_pt[i].y + M.at<float>(0,2);
        vect_pt[i].y = M.at<float>(1,0)*vect_pt[i].x + M.at<float>(1,1)*vect_pt[i].y + M.at<float>(1,2);
    }
    return vect_pt;
}

vector<cv::Point>  m_skala(vector<cv::Point> vect_pt, int s_x, int s_y) {
    cv::Mat M(3,3,CV_64F);
    
    M.at<float>(0,0) = s_x;
    M.at<float>(0,1) = 0;
    M.at<float>(0,2) = 0;
    
    M.at<float>(1,0) = 0;
    M.at<float>(1,1) = s_y;
    M.at<float>(1,2) = 0;
    
    M.at<float>(2,0) = 0;
    M.at<float>(2,1) = 0;
    M.at<float>(2,2) = 1;
    
    for(int i=0; i<vect_pt.size(); i++) {
        vect_pt[i].x = M.at<float>(0,0)*vect_pt[i].x + M.at<float>(0,1)*vect_pt[i].y + M.at<float>(0,2);
        vect_pt[i].y = M.at<float>(1,0)*vect_pt[i].x + M.at<float>(1,1)*vect_pt[i].y + M.at<float>(1,2);
    }
    return vect_pt;
}

/*
 * 
 */
int main(int argc, char** argv) {
    //CONTAINER
    cv::Mat container(480,720,CV_64FC3,cv::Scalar(0,0,0));
    
    cv::Scalar RED(0,0,255);
    cv::Scalar BLUE(255,0,0);
    cv::Scalar WHITE(255,255,255);
    cv::Point start(-30,-30);
    
    vector<cv::Point> vect_point;
    
    vect_point = m_rotasi(create_vector_point(start,60,60),45);
    draw_rectangle(container,vect_point,RED);
    
    vect_point = m_translasi(create_vector_point(start,40,40),120,120);
    draw_rectangle(container,vect_point,BLUE);
    
    vect_point = m_translasi(create_vector_point(start,20,20),170,170);
    draw_rectangle(container,vect_point,WHITE);
    
    /**
    vector<cv::Point> vect_rotasi = m_rotasi(create_vector_point(start,20,20),30);
    vect_translasi = m_translasi(vect_rotasi,0,120);
    draw_rectangle(container,vect_translasi,WHITE);*/
    
    
    
    //DISPLAY
    cv::namedWindow(WINDOW_NAME,cv::WINDOW_AUTOSIZE);
    cv::imshow(WINDOW_NAME,container);
    cv::waitKey(0);
    return 0;
}

