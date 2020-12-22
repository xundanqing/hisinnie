//
// Created by yushe.cao@seaway on 2020/7/20.
//

#ifndef SEAWAY_FACE_ALIGN_H
#define SEAWAY_FACE_ALIGN_H

#include <iostream>
#include <Net.hpp>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include "iniparser.h"
#include "dictionary.h"

#define PI 3.141592654

class FaceAlign
{

public:

    FaceAlign(dictionary *config);

    ~FaceAlign();

    bool build();

    bool doInference(cv::Mat& img, cv::Rect& box,nnie::Mat& landmarks,nnie::Mat& pose);


private:

    nnie::gParams params;

    nnie::Net net;

private:
    /**
        * Check the valid of parameters.
        * @param gparams
        * @return
        */
    bool validateGparams(nnie::gParams gparams);
    void preProcess(cv::Mat& img,cv::Rect& box,cv::Mat& faceImg);
};


#endif //SEAWAY_FACE_ALIGN_H
