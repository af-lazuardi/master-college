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
static cv::String WIN_NAME_RECT = "Iris Transformation";
static cv::String WIN_NAME_IRIS_FROM_RECT = "Invert Transformation";


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


double interpolate(cv::Mat im, double xR, double yR) {
    double v;
    int xf,xc,yf,yc;
    
    xf = floor(xR);
    xc = ceil(xR);
    yf = floor(yR);
    yc = ceil(yR);
    
    if((xf==xc) && (yc==yf)) {
        v = im.at<double>(xc,yc);
    }
    else if(xf==xc) {
        v = im.at<double>(xf,yf) + ((yf-yR)*(im.at<double>(xf,yc) - im.at<double>(xf,yf)));
    }
    else if(yf==yc) {
        v = im.at<double>(xf,yf) + ((xf-xR)*(im.at<double>(xc,yf) - im.at<double>(xf,yf)));
    }
    else {
        cv::Mat A = (cv::Mat_<double>(4,4) << xf, yf, xf*yf, 1, xf, yc, xf*yc, 1, xc, yf, xc*yf, 1, xc, yc, xc*yc, 1);
        cv::Mat r = (cv::Mat_<double>(4,1) << im.at<double>(xf,yf), im.at<double>(xf,yc), im.at<double>(xc,yf), im.at<double>(xc,yc));
        cv::Mat a(4,1,CV_64F);
        cv::solve(A,r,a,cv::DECOMP_LU);
        cv::Mat w = (cv::Mat_<double>(1,4) << xR, yR, xR*yR, 1);
        
        cv::Mat V = w*a;
        v= V.at<double>(0,0);
        
    }
    //std::cout << v << std::endl;
    return v;
}

double interpolate(cv::Mat im, double r, double t, double rMin, double rMax, int M, int N, double delR, double delT) {
    double ri,ti,v;
    ri = 1+ (r-rMin)/delR;
    ti = 1+ t/delT;
    
    int rf,rc,tf,tc;
    rf = floor(ri);
    rc = ceil(ri);
    tf = floor(ti);
    tc = ceil(tc);
    
    if(tc > N)
        tc = tf;
    
    if((rf==rc) && (tc==tf)) {
        v = im.at<double>(rc,tc);
    }
    else if(rf==rc) {
        v = im.at<double>(rf,tf) + (ti-tf)*(im.at<double>(rf,tc)-im.at<double>(rf,tf));
    }
    else if(tf==tc) {
        v = im.at<double>(rf,tf) + (ri-tf)*(im.at<double>(rc,tf)-im.at<double>(rf,tf));
    }
    else {
        cv::Mat A = (cv::Mat_<double>(4,4) << rf, tf, rf*tf, 1, rf, tc, rf*tc, 1, rc, tf, rc*tf, 1, rc, tc, rc*tc, 1);
        cv::Mat z = (cv::Mat_<double>(4,1) << im.at<double>(rf,tf), im.at<double>(rf,tc), im.at<double>(rc,tf), im.at<double>(rc,tc));
        cv::Mat a(4,1,CV_64F);
        cv::solve(A,z,a,cv::DECOMP_LU);
        cv::Mat w = (cv::Mat_<double>(1,4) << ri, ti, ri*ti, 1);
        
        cv::Mat V = w*a;
        v= V.at<double>(0,0);
    }
    
    return v;
}

cv::Mat iris_to_polar(cv::Mat im_src, int r_iris, int r_pupil) {
    cv::Mat im_src_double;
    cv::Mat im_src_cp = im_src.clone();
    im_src_cp.convertTo(im_src_double,CV_64F);
    im_src_double = im_src_double/255.0;
    
    double delR,delT,rMax,rMin,r,t,x,y,xR,yR,Om,On,sx,sy;
    int Mr,Nr,M,N;
      
    Mr = im_src_double.rows;
    Nr = im_src_double.cols;

    Om = (Mr+1)/2.0;
    On = (Nr+1)/2.0;
    
    sx = (Mr-1)/2.0;
    sy = (Nr-1)/2.0;
    
    M = r_iris;
    N = 360;
    
    
    rMax = (r_iris*2)/(double)im_src.rows;
    rMin = ((r_pupil+2)*2)/(double)im_src.rows;
    
    delR = (rMax-rMin)/(double)(M-1);
    delT = (2*PI)/(double)N;
    
    cv::Mat imP = cv::Mat::zeros(M,N,CV_64F);
   
    for(int ri=0; ri<M; ri++) {
        for(int ti=0; ti<N; ti++) {
            r = rMin + (ri-1)*delR;
            t = ti*delT;
            x = r*cos(t);
            y = r*sin(t);
            xR = x*sx+Om;
            yR = y*sy+On;
            
            imP.at<double>(ri,ti) = interpolate(im_src_double,xR,yR);
        }
    }
    
    return imP;
}

cv::Mat polar_to_iris(cv::Mat im_src, int r_iris, int r_pupil, int Mr, int Nr) {
    cv::Mat im_src_cp = im_src.clone();
    cv::Mat imR = cv::Mat::zeros(Mr,Nr,CV_64F);
    
    int M,N;
    double Om,On,sx,sy,rMax,rMin,delR,delT,x,y,r,t;
    
    Om = (Mr+1)/2.0;
    On = (Nr+1)/2.0;
    
    sx = (Mr-1)/2.0;
    sy = (Nr-1)/2.0;
    
    M = im_src_cp.rows;
    N = im_src_cp.cols;
    
    rMax = (r_iris*2)/(double)Mr;
    rMin = ((r_pupil+2)*2)/(double)Mr;
    
    delR = (rMax-rMin)/(double)(M-1);
    delT = (2*PI)/(double)N;

    for(int xi=0; xi<Mr; xi++) {
        for(int yi=0; yi<Nr; yi++) {
            x = (xi+1-Om)/sx;
            y = (yi+1-On)/sy;
            
            r = sqrt(pow(x,2)+pow(y,2));
            
            if(r >= rMin && r <= rMax) {
                t = atan2(y,x);
                if(t < 0) {
                    t = t + 2*PI;
                }
                imR.at<double>(xi,yi) = interpolate(im_src_cp,r,t,rMin,rMax,M,N,delR,delT);
            }
        }
    }
    return imR;
}


/*
 * 
 */
int main(int argc, char** argv) {
    cv::namedWindow(WIN_NAME_ORI,cv::WINDOW_AUTOSIZE);
    
    cv::Mat im_orig_eye = cv::imread("blue-iris-529x351.jpg");
    
    cv::Mat im_iris = GUI::getCircle(im_orig_eye, IRIS, WIN_NAME_ORI);
    cv::Mat im_pupil = GUI::getCircle(im_iris, PUPIL, WIN_NAME_ORI);
    cv::imshow(WIN_NAME_PUPIL,im_pupil);
    
    cv::Mat im_line_detection = laplacian_line_detection(im_pupil);
    cv::imshow(WIN_NAME_IRIS_LINE_DETECT,im_line_detection);
    
    cv::Mat im_polar = iris_to_polar(im_line_detection, GUI::radius_iris, GUI::radius_pupil);
    cv::imshow(WIN_NAME_RECT,im_polar);
    
    cv::Mat im_iris_from_polar = polar_to_iris(im_polar, GUI::radius_iris, GUI::radius_pupil, im_iris.rows, im_iris.cols);
    cv::imshow(WIN_NAME_IRIS_FROM_RECT,im_iris_from_polar);
    
    cv::waitKey(0);
    return 0;
}

