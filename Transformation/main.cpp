/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: RIF <arif.lazuardi@infoglobal.co.id>
 *
 * Created on September 4, 2017, 9:34 AM
 */

#include <cstdlib>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace std;

void normalize_point_at_center(int width, int height, cv::Point *point) {
    point->x = point->x + width/2;
    point->y = point->y + height/2;
}

void create_line(cv::Mat im, cv::Point start, cv::Point end) {
    int thickness = 1;
    int lineType = cv::LINE_8;
    
    normalize_point_at_center(im.cols,im.rows,&start);
    normalize_point_at_center(im.cols,im.rows,&end);
    cv::line( im, start, end, cv::Scalar( 0, 0, 0 ), thickness, lineType );
}

void create_rectangle(cv::Mat im, vector<cv::Point> points) {
    for(int i=0; i<points.size()-1; i++) {
        create_line(im,points[i],points[i+1]);
    }
}

/*
 * 
 */
int main(int argc, char** argv) {
    //BASIC CONTAINER
    cv::Mat container(480,720,CV_64FC3,cv::Scalar(255,255,255));
    
    //DRAW LINE
    cv::Point pt_1, pt_2, pt_3, pt_4, pt_5;
    pt_1.x = 0; pt_1.y = 0;
    pt_2.x = 20; pt_2.y = 0; 
    pt_3.x = 20; pt_3.y = 20;
    pt_4.x = 0; pt_4.y = 20;
    pt_5.x = 0; pt_5.y = 0;
    
    vector<cv::Point> vector_point;
    vector_point.push_back(pt_1);
    vector_point.push_back(pt_2);
    vector_point.push_back(pt_3);
    vector_point.push_back(pt_4);
    vector_point.push_back(pt_5);
    

    
    create_rectangle(container,vector_point);
    
    //DRAW
    cv::namedWindow("Transformation",cv::WINDOW_AUTOSIZE);
    cv::imshow("Transformation",container);
    cv::waitKey(0);
    //END DRAW
    return 0;
}



