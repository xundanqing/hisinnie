//
// Created by yushe.cao@seaway on 2020/7/20.
//

#ifndef SEAWAY_FACE_RECOGNITION_H
#define SEAWAY_FACE_RECOGNITION_H

#include <iostream>
#include <Net.hpp>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include "iniparser.h"
#include "dictionary.h"

class FaceRecognition
{

public:

    FaceRecognition(dictionary *config);

    ~FaceRecognition();

    bool build();

    bool doInference(cv::Mat input_img, std::vector<float> &features);


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
};


#endif //SEAWAY_FACE_ALIGN_H
