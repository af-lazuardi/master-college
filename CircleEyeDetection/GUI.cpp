/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   GUI.cpp
 * Author: RIF <arif.lazuardi@infoglobal.co.id>
 * 
 * Created on September 12, 2017, 3:23 PM
 */

#include "GUI.h"

cv::String GUI::window;
cv::Mat GUI::im_select;
bool GUI::tl_set;
bool GUI::br_set;
cv::Point GUI::tl;
cv::Point GUI::br;

GUI::GUI() {
}

GUI::GUI(const GUI& orig) {
}

GUI::~GUI() {
}

void GUI::screen_log(cv::Mat im_draw, const cv::String text) {
    int font = cv::FONT_HERSHEY_SIMPLEX;
    float font_scale = 0.5;
    int thickness = 1;
    int baseline;

    cv::Size text_size = cv::getTextSize(text, font, font_scale, thickness, &baseline);

    cv::Point bl_text = cv::Point(0,text_size.height);
    cv::Point bl_rect = bl_text;

    bl_rect.y += baseline;

    cv::Point tr_rect = bl_rect;
    tr_rect.x = im_draw.cols; //+= text_size.width;
    tr_rect.y -= text_size.height + baseline;

    rectangle(im_draw, bl_rect, tr_rect, cv::Scalar(0,0,0), -1);

    putText(im_draw, text, bl_text, font, font_scale, cv::Scalar(255,255,255));
}

cv::Rect GUI::getRect(const cv::Mat im, const cv::String win_name) {
    GUI::window = win_name;
    GUI::im_select = im;
    GUI::tl_set = false;
    GUI::br_set = false;

    setMouseCallback(GUI::window, onMouseRect);

    //Dummy call to get drawing right
    onMouseRect(0,0,0,0,0);

    while(!GUI::br_set)
    {
        cv::waitKey(10);
    }

    cv::setMouseCallback(GUI::window, NULL);

    GUI::im_select.release(); //im_select is in global scope, so we call release manually

    return cv::Rect(GUI::tl,GUI::br);
}

void GUI::onMouseRect(int event, int x, int y, int flags, void* param) {
    cv::Mat im_draw;
    GUI::im_select.copyTo(im_draw);

    if(event == CV_EVENT_LBUTTONUP && !GUI::tl_set)
    {
        GUI::tl = cv::Point(x,y);
        GUI::tl_set = true;
    }

    else if(event == CV_EVENT_LBUTTONUP && GUI::tl_set)
    {
        GUI::br = cv::Point(x,y);
        GUI::br_set = true;
        screen_log(im_draw, "Initializing...");
    }

    if (!GUI::tl_set) screen_log(im_draw, "Click on the top left corner of the object");
    else
    {
        rectangle(im_draw, GUI::tl, cv::Point(x, y), cv::Scalar(255,0,0));

        if (!GUI::br_set) screen_log(im_draw, "Click on the bottom right corner of the object");
    }

    imshow(GUI::window, im_draw);
}

void GUI::onMouseCircle(int event, int x, int y, int flags, void* param) {
    cv::Mat im_draw;
    GUI::im_select.copyTo(im_draw);

    if(event == CV_EVENT_LBUTTONUP && !GUI::tl_set)
    {
        GUI::tl = cv::Point(x,y);
        GUI::tl_set = true;
    }

    else if(event == CV_EVENT_LBUTTONUP && GUI::tl_set)
    {
        GUI::br = cv::Point(x,y);
        GUI::br_set = true;
        //screen_log(im_draw, "Initializing...");
    }

    //if (!GUI::tl_set) screen_log(im_draw, "klik di tengah lingkaran pupil mata");
    //else
    if (GUI::tl_set)
    {
        GUI::create_line(im_draw,GUI::tl,cv::Point(x,y));
        //if (!GUI::br_set) screen_log(im_draw, "klik tepi lingkaran untuk mendapatkan nilai radius");
    }

    imshow(GUI::window, im_draw);
}



cv::Mat GUI::getCircle(const cv::Mat im, int type_circle ,const cv::String win_name) {
    GUI::window = win_name;
    GUI::im_select = im;
    GUI::tl_set = false;
    GUI::br_set = false;

    setMouseCallback(GUI::window, onMouseCircle);

    if(type_circle == 1) {
        int center = im.rows/2;
        GUI::tl = cv::Point(center,center);
        GUI::tl_set = true;
         //screen_log(GUI::im_select, "klik di tengah lingkaran pupil mata");
    }
    else if(type_circle == 2) {
         //screen_log(GUI::im_select, "klik di tengah lingkaran iris mata");
    }
    
    //Dummy call to get drawing right
    onMouseCircle(0,0,0,0,0);

    while(!GUI::br_set)
    {
        cv::waitKey(10);
    }

    cv::setMouseCallback(GUI::window, NULL);

    cv::Mat dst;
    
    
    if(type_circle == 1) {
        int radius = (int) GUI::get_euclidean_distance(GUI::tl,GUI::br);
        cv::Mat mask = cv::Mat(GUI::im_select.rows,GUI::im_select.cols, CV_8UC1,cv::Scalar(255,255,255));
        cv::circle(mask, GUI::tl, radius, cv::Scalar(0,0,0), -1, 8, 0);
        GUI::im_select.copyTo(dst,mask); // copy values of img to dst if mask is > 0.
    }
    else if(type_circle == 2) {
        int radius = (int) GUI::get_euclidean_distance(GUI::tl,GUI::br);
        cv::Mat ROI(GUI::im_select, cv::Rect( GUI::tl.x-radius, GUI::tl.y-radius, radius*2, radius*2 ));
        
        cv::Point center_of_ROI;
        center_of_ROI.x = ROI.cols/2;
        center_of_ROI.y = ROI.rows/2;
        cv::Mat mask = cv::Mat::zeros( ROI.rows, ROI.cols, CV_8UC1 );
        cv::circle(mask, center_of_ROI, radius, cv::Scalar(255,255,255), -1, 8, 0);
        ROI.copyTo(dst,mask); // copy values of img to dst if mask is > 0.
        
        cv::circle(dst, center_of_ROI, 2, cv::Scalar(255,0,0));
    }
  
    GUI::im_select.release(); //im_select is in global scope, so we call release manually  
    return dst;
}

double GUI::get_euclidean_distance(cv::Point pt_1, cv::Point pt_2) {
    cv::Point diff = pt_1 - pt_2;
    return cv::sqrt(diff.x*diff.x + diff.y*diff.y);
}

void GUI::create_line(cv::Mat im, cv::Point start, cv::Point end) {
    int thickness = 1;
    int lineType = cv::LINE_8;
    
    //normalize_point_at_center(im.cols,im.rows,&start);
    //normalize_point_at_center(im.cols,im.rows,&end);
    cv::line( im, start, end, cv::Scalar( 255, 255, 255 ), thickness, lineType );
}









