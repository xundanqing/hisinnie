#ifndef UTIL_H
#define UTIL_H
#include <string>
#include <vector>
#include <utility>
#include <opencv2/opencv.hpp>
#include "Net.hpp"


#ifdef __cplusplus
extern "C"
{
#endif

#define CLIP_ZERO_ONE(x) std::max<float>(0.0,std::min<float>(1.0,x))

struct ObjectInfo{
    cv::Rect box;
    int idx;
    float score;
    bool isFrontal;
    float distance;
};
struct LaneInfo
{
    std::vector<std::pair<cv::Point, cv::Point> > lanePoints;
    int departSignal;
    float departDistance;
    void reset(){lanePoints.clear();departSignal=-2;departDistance=0;}
};
struct GlobalInfo
{
    std::vector<ObjectInfo> objects;
    LaneInfo laneInformation;
    void reset(){objects.clear();laneInformation.reset();};
    void putOnImage(cv::Mat& img){
        if(laneInformation.departSignal != -2)
        {
            char tmp[20];
            sprintf(tmp,"%.2lf",abs(laneInformation.departDistance));
            std::string distance(tmp);
            std::string car_pos_text;
            if(laneInformation.departDistance >0)
                car_pos_text = "Right deviation " + distance + " m.";
            else
                car_pos_text = "Left deviation " + distance + " m.";
            cv::putText(img,car_pos_text,cv::Point(1000,100),cv::FONT_HERSHEY_SIMPLEX,1,(0,0,255),2);
            if(laneInformation.departDistance==-1)
                cv::putText(img,"Warning: Serious Left Deviation!",cv::Point(1000,150),cv::FONT_HERSHEY_SIMPLEX,1,(0,0,255),2);
            else if(laneInformation.departDistance ==1)
                cv::putText(img,"Warning: Serious Right Deviation!",cv::Point(1000,150),cv::FONT_HERSHEY_SIMPLEX,1,(0,0,255),2);
            
            for(size_t i = 0;i<laneInformation.lanePoints.size()-1;++i)
            {
                cv::Point pt0 =  laneInformation.lanePoints[i].first;
                cv::Point pt1 = laneInformation.lanePoints[i].second;
                cv::Point pt2 = laneInformation.lanePoints[i+1].first;
                cv::Point pt3 = laneInformation.lanePoints[i+1].second;
                cv::circle(img,pt0,5,(0,0,255),-1);
                cv::circle(img,pt1,5,(0,0,255),-1);
                cv::line(img,pt0,pt2,(255,255,255),1);
                cv::line(img,pt1,pt3,(255,255,255),1);
            }
        }

        bool haveObjectAfrontal = false;
        float minFrontObjectDist = 100000;
        for(size_t i = 0;i<objects.size();++i)
        {
            ObjectInfo obj = objects[i];
            if(obj.isFrontal)
            {
                haveObjectAfrontal = true;
                cv::rectangle(img,obj.box,(255,255,0),2);
                char tmp[20];
                sprintf(tmp,"%.2f",obj.distance/100.);
                std::string distance(tmp);
                
                minFrontObjectDist = std::min<float>(minFrontObjectDist,obj.distance/100.);
                cv::putText(img,distance +" m",cv::Point(obj.box.x,obj.box.y),cv::FONT_HERSHEY_SIMPLEX,std::min<float>(2,15./obj.box.width),(255,255,0),2);
            }
            else{
                cv::rectangle(img,obj.box,(255,255,0),2);
            }
        }
        if(haveObjectAfrontal)
        {
            char tmp[20];
            sprintf(tmp,"%.2f",minFrontObjectDist);
            std::string minDistance(tmp);
            cv::putText(img,"frontal object distance: " + minDistance +" m",cv::Point(1000,200),cv::FONT_HERSHEY_SIMPLEX,1,(0,0,255),2);
        }
        else
        {
            cv::putText(img,"frontal object distance: no",cv::Point(1000,200),cv::FONT_HERSHEY_SIMPLEX,1,(0,0,255),2);
        }

        
    }
};

inline float Sigmoid(float x)
{
    return 1.0 / (1.0 + exp(-x));
} ;

inline void Softmax(std::vector<float> &classes)
{
    float sum = 0;
    std::transform(classes.begin(), classes.end(), classes.begin(),
                   [&sum](float score) -> float
                   {
                       float exp_score = exp(score);
                       sum += exp_score;
                       return exp_score;
                   });
    std::transform(classes.begin(), classes.end(), classes.begin(),
                   [sum](float score) -> float
                   { return score / sum; });
} ;

inline void parseYolov3Feature(int img_width,
                               int img_height,
                               int num_classes,
                               int kBoxPerCell,
                               int feature_index,
                               float conf_threshold,
                               const std::vector<cv::Size2f> &anchors,
                               const nnie::Mat feature,
                               std::vector<int> &ids,
                               std::vector<cv::Rect> &boxes,
                               std::vector<float> &confidences)
{
    const float downscale = static_cast<float>(std::pow(2, feature_index) / 32); // downscale, 1/32, 1/16, 1/8
    int cell_h = feature.height;
    int cell_w = feature.width;
    for (int cy = 0; cy < cell_h; ++cy)
    {
        for (int cx = 0; cx < cell_w; ++cx)
        {
            for (int b = 0; b < kBoxPerCell; ++b)
            {
                int channel = b * (num_classes + 5);

                float tc = feature.data[cx + (cy * feature.width) + (channel + 4) * feature.height * feature.width];

                float confidence = Sigmoid(tc);

                if (confidence >= conf_threshold)
                {

                    float tx = feature.data[cx + (cy * feature.width) + channel * feature.height * feature.width];
                    float ty = feature.data[cx + (cy * feature.width) + (channel + 1) * feature.height * feature.width];
                    float tw = feature.data[cx + (cy * feature.width) + (channel + 2) * feature.height * feature.width];
                    float th = feature.data[cx + (cy * feature.width) + (channel + 3) * feature.height * feature.width];
                    float tc = feature.data[cx + (cy * feature.width) + (channel + 4) * feature.height * feature.width];

                    float x = (cx + Sigmoid(tx)) / cell_w;
                    float y = (cy + Sigmoid(ty)) / cell_h;
                    float w = exp(tw) * anchors[b].width * downscale / cell_w;
                    float h = exp(th) * anchors[b].height * downscale / cell_h;
                    // CV_LOG_INFO(NULL, "cx:" << cx << " cy:" << cy);
                    // CV_LOG_INFO(NULL, "box:" << x << " " << y << " " << w << " " << h << " c:" << confidence
                    //                          << " index:" << index);
                    std::vector<float> classes(num_classes);
                    for (int i = 0; i < num_classes; ++i)
                    {
                        classes[i] = feature.data[cx + (cy * feature.width) +
                                                  (channel + 5 + i) * feature.height * feature.width];
                    }
                    Softmax(classes);
                    auto max_itr = std::max_element(classes.begin(), classes.end());
                    int index = static_cast<int>(max_itr - classes.begin());
                    if (num_classes > 1)
                    {
                        confidence = confidence * classes[index];
                    }

                    int left = std::max<int>(0,(x - w/2)*img_width);
                    int right = std::min<int>(img_width-1,(x + w/2)*img_width);
                    int top = std::max<int>(0,(y - h/2)*img_height);
                    int bottom = std::min<int>(img_height-1,(y  +h/2)*img_height);

                    // CV_LOG_INFO(NULL,
                    //             "box:" << left << " " << top << " " << width << " " << height << " c:" << confidence
                    //                    << " index:" << index
                    //                    << " p:" << classes[index]);

                    ids.push_back(index);
                    boxes.emplace_back(left, top, right-left, bottom-top);
                    confidences.push_back(confidence);
                }
            }
        }
    }
} ;


float distance(float x1,float y1, float x2, float y2);

void computeFaceStatus(cv::Mat& img,nnie::Mat& landmark, nnie::Mat& pose, int& eyeStaus, int& mouthStatus);
    /*
        根据landmark计算人的状态
        return:
            eyeStaus: 0 - 睁眼  1- 闭眼
            mouthStatus: -1 - 闭合状态  0 - 正常状态  1 - 打哈欠状态
    */

float estiameLineSlopeRate(std::vector<cv::Point>& points);
/**
 * 直线拟合,获取直线的斜率
 **/


#ifdef __cplusplus
}
#endif
#endif