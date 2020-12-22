//
// Created by yushe.cao@seaway on 2020/7/20.
//

#ifndef LANE_H
#define LANE_H

#include <iostream>
#include <Net.hpp>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include "iniparser.h"
#include "dictionary.h"
#include <limits>
#include "util.h"

class Lane
{

public:

    Lane(dictionary *config);

    ~Lane();

    bool build();

    /**
     * @param
     *     GlobalInfo: 存储检测信息的结构体
     *      
     **/
    bool doInference(cv::Mat& img,GlobalInfo& globalInfo);

private:

    nnie::gParams params;

    nnie::Net net;

private:
    float col_sample_w = 4.015074;
    /**
        * Check the valid of parameters.
        * @param gparams
        * @return
        */
    bool validateGparams(nnie::gParams gparams);
    /**
     * 后处理函数
     * */
    void postProcess(nnie::Mat& out,std::vector<float>& loc);
    /**
     * 判断检测到的车道线是否可用
     * */
    bool goodLane(std::vector<float>& loc);

};


#endif //SEAWAY_FACE_ALIGN_H
