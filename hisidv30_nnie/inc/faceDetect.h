//
// Created by yushe.cao@seaway on 2020/7/20.
//

#ifndef SEAWAY_FACE_DETECT_H
#define SEAWAY_FACE_DETECT_H

#include <iostream>
#include <string>
#include <Net.hpp>
#include <unordered_map>
#include <opencv2/opencv.hpp>
#include <vector>
#include "iniparser.h"
#include "dictionary.h"
#include "util.h"

using namespace std;

class FaceDetector
{

public:

    FaceDetector(dictionary *config);

    ~FaceDetector();

    bool build();

    bool doInference(cv::Mat& img, cv::Rect& max_face_box);


private:

    nnie::gParams params;

    nnie::Net net;

private:

    /* 中间变量 */
    std::vector<int> image_size = {320,240};
    std::vector<std::vector<float> > priors;
    float center_variance = 0.1;
    float size_varicance = 0.2;
    float conf_threshold = 0.7;
    float nms_threshold =  0.3;
    std::vector<std::vector<float> > shrinkage_list;
    std::vector<std::vector<int> > feature_map_w_h_list;
    std::vector<float> strides = {8.0, 16.0, 32.0, 64.0};
    std::vector<std::vector<float> > min_boxes = {{10.0, 16.0, 24.0}, {32.0, 48.0}, {64.0, 96.0}, {128.0, 192.0, 256.0}};
    /**
        * Check the valid of parameters.
        * @param gparams
        * @return
        */
       
    bool validateGparams(nnie::gParams gparams);
    void define_img_size();
    void generate_priors();
    void find_max_face(nnie::Mat& boxes, nnie::Mat& scores,int img_height,int img_width,cv::Rect& max_face_box);

    #ifdef __DEBUG__
    //help function
    template<typename T>
    void print(std::vector<T> &vec)
    {
        for(int i = 0;i < vec.size();++i)
        {
            std::cout << vec[i] << " ";
        }
        std::cout << std::endl;
    }
    template<typename T>
    void print(std::vector<std::vector<T> > &vec)
    {
        for(int i = 0;i<vec.size();++i)
        {
            for(int j = 0;j<vec[i].size();++j)
            {
                std::cout << vec[i][j] <<" ";
            }
            std::cout << std::endl;
        }
    }
    #endif

};


#endif //NNIE_LITE_GENERALCLS_H
