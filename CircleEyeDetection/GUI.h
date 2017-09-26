/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GUI.h
 * Author: RIF <arif.lazuardi@infoglobal.co.id>
 *
 * Created on September 12, 2017, 3:23 PM
 */

#ifndef GUI_H
#define GUI_H

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

class GUI {
public:
    GUI();
    GUI(const GUI& orig);
    virtual ~GUI();
    
    static void screen_log(cv::Mat im_draw, const cv::String text);
    static void onMouseRect(int event, int x, int y, int flags, void *param);
    static cv::Rect getRect(const cv::Mat im, const cv::String win_name);
    
    static void onMouseCircle(int event, int x, int y, int flags, void* param);
    static cv::Mat getCircle(const cv::Mat im, int type_circle ,const cv::String win_name);
    
    static double get_euclidean_distance(cv::Point pt_1, cv::Point pt_2);
    static void create_line(cv::Mat im, cv::Point start, cv::Point end);
    
    static cv::String window;
    static cv::Mat im_select;
    static bool tl_set;
    static bool br_set;
    static cv::Point tl;
    static cv::Point br;
    static int radius_iris;
    static int radius_pupil;
private:

};

#endif /* GUI_H */

