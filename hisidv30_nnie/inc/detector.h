//
// Created by yushe.cao@seaway on 2020/7/20.
//

#ifndef DETECTOR_H
#define DETECTOR_H

#include <iostream>
#include <Net.hpp>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include "iniparser.h"
#include "dictionary.h"
#include "util.h"

#define PI 3.141592654

class Detector
{

public:

    Detector(dictionary *config);

    ~Detector();

    bool build();

    bool doInference(cv::Mat& img,GlobalInfo& globalInfo);


private:

    nnie::gParams params;

    nnie::Net net;

private:
    /*距离估计的参数*/
    float focalDist = 820.0;
    float actualCarWidth = 160.0;
    float actualPersonHeight = 170.0;
    /*yolov3的参数*/
    int numClasses = 95;
    int kBoxPerCell = 3;

    float confThreshold = 0.45;
    float nmsThreshold = 0.45;

    const std::vector<std::vector<cv::Size2f>> anchors = {
            {{66, 81}, {92, 131}, {141, 209}},
            {{12, 49}, {30, 40}, {42, 63}},
            {{6, 10}, {12, 18}, {20, 27}}};
    /**
        * Check the valid of parameters.
        * @param gparams
        * @return
        */
    bool validateGparams(nnie::gParams gparams);
    void preProcess(cv::Mat& img,cv::Mat& input_img);
};


#endif //SEAWAY_FACE_ALIGN_H
