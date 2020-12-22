#include "util.h"
float distance(float x1,float y1, float x2, float y2)
{
    float distance =  powf((x1-x2),2) + powf((y1-y2),2);
    return sqrtf(distance);
}

void computeFaceStatus(cv::Mat& img,nnie::Mat& landmark, nnie::Mat& pose, int& eyeStaus, int& mouthStatus)
{
    /*
        根据landmark计算人的状态
        return:
            eyeStaus: 0 - 睁眼  1- 闭眼
            mouthStatus: -1 - 闭合状态  0 - 正常状态  1 - 打哈欠状态
    */
    float d2 = distance(landmark.data[62*2],landmark.data[62*2+1],landmark.data[66*2],landmark.data[66*2+1]);
    cv::circle(img,cv::Point(landmark.data[62*2],landmark.data[62*2+1]),1,(0,255,255),2);
    cv::circle(img,cv::Point(landmark.data[66*2],landmark.data[66*2+1]),1,(0,255,255),2);
    float d5 = distance(landmark.data[70*2],landmark.data[70*2+1],landmark.data[74*2],landmark.data[74*2+1]);
    cv::circle(img,cv::Point(landmark.data[70*2],landmark.data[70*2+1]),1,(0,255,255),2);
    cv::circle(img,cv::Point(landmark.data[74*2],landmark.data[74*2+1]),1,(0,255,255),2);
    float d_mean = (d2+d5)/2.;
    float d7 = distance(landmark.data[60*2],landmark.data[60*2+1],landmark.data[64*2],landmark.data[64*2+1]);
    cv::circle(img,cv::Point(landmark.data[60*2],landmark.data[60*2+1]),1,(0,255,255),2);
    cv::circle(img,cv::Point(landmark.data[64*2],landmark.data[64*2+1]),1,(0,255,255),2);
    float d8 = distance(landmark.data[68*2],landmark.data[68*2+1],landmark.data[72*2],landmark.data[72*2+1]);
    cv::circle(img,cv::Point(landmark.data[68*2],landmark.data[68*2+1]),1,(0,255,255),2);
    cv::circle(img,cv::Point(landmark.data[72*2],landmark.data[72*2+1]),1,(0,255,255),2);
    float d_reference = (d7+d8)/2.;
    float EAR =  d_mean/d_reference;
    //std::cout <<"d2: " << d2 << "d5: " << d5 << "d7: " << d7 << "d8: "<< d8 << "d_mean: "<< d_mean << "d_reference: " << d_reference << "EAR: "<< EAR << std::endl;
    eyeStaus = EAR<0.16;

    float d9 = distance(landmark.data[78*2],landmark.data[78*2+1],landmark.data[86*2],landmark.data[86*2+1]);
    cv::circle(img,cv::Point(landmark.data[78*2],landmark.data[78*2+1]),1,(0,255,255),2);
    cv::circle(img,cv::Point(landmark.data[86*2],landmark.data[86*2+1]),1,(0,255,255),2);
    float d10 = distance(landmark.data[80*2],landmark.data[80*2+1],landmark.data[84*2],landmark.data[84*2+1]);
    cv::circle(img,cv::Point(landmark.data[80*2],landmark.data[80*2+1]),1,(0,255,255),2);
    cv::circle(img,cv::Point(landmark.data[84*2],landmark.data[84*2+1]),1,(0,255,255),2);
    float d11 = distance(landmark.data[76*2],landmark.data[76*2+1],landmark.data[82*2],landmark.data[82*2+1]);
    cv::circle(img,cv::Point(landmark.data[76*2],landmark.data[76*2+1]),1,(0,255,255),2);
    cv::circle(img,cv::Point(landmark.data[82*2],landmark.data[82*2+1]),1,(0,255,255),2);
    float mouthAR = (d9+d10)/(2.0*d11);
    //std::cout << "d9: " << d9 << "d10: " << d10 << "d11: " << d11 << "mouthAR: " << mouthAR << std::endl;
    mouthStatus = mouthAR>1?1:mouthAR<0.3?-1:0;
    cv::putText(img,std::to_string(EAR),cv::Point(450,150),cv::FONT_HERSHEY_SIMPLEX,0.5,cv::Scalar(0, 255, 255),2);
    cv::putText(img,std::to_string(mouthAR),cv::Point(450,200),cv::FONT_HERSHEY_SIMPLEX,0.5,cv::Scalar(0, 255, 255),2);

}

/**
 * 直线拟合,获取斜率
 * */
float estiameLineSlopeRate(std::vector<cv::Point>& points)
{
    cv::Vec4f line_para;
    cv::fitLine(points,line_para,cv::DIST_L2,0,1e-2,1e-2);
    return line_para[1] / line_para[0];
}