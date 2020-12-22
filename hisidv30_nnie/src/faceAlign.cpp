//
// Created by surui on 2020/7/1.
//

#include "faceAlign.h"


FaceAlign::FaceAlign(dictionary *config)
{
    params.modelPath =  iniparser_getstring(config, "faceAlign:modelPath", "./models/faceAlign/landmark_inst.wk");;
    // You can reference your prototxt to fill these field.
    params.batchSize = 1;
    params.inputHeight = 112;
    params.inputWidth = 112;
    params.inputC = 3;

    if (!validateGparams(params))
    {
        perror("[ERROR] Check your gparams !\n\n");
    }
}



bool FaceAlign::validateGparams(nnie::gParams gparams)
{
    if (gparams.inputHeight < 1 || gparams.inputWidth < 1 || gparams.inputC < 1)
    {
        perror("[ERROR] You have to assign the resize info and channel !\n\n");
        return false;
    }
    if (gparams.batchSize < 1)
    {
        perror("[ERROR] You have to assign the batch size !\n\n");
        return false;
    }
    if (gparams.modelPath.empty())
    {
        perror("[ERROR] You have to assign the engine path !\n\n");
        return false;
    }
    return true;
}

FaceAlign::~FaceAlign()
{
    net.clear();
}

bool FaceAlign::build()
{
    try
    {
        net.load_model(params.modelPath.c_str());
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

}


bool FaceAlign::doInference(cv::Mat& img, cv::Rect& box,nnie::Mat& landmarks,nnie::Mat& pose)
{
    try
    {
        cv::Mat cropped;
        int size_w = int(std::max<int>(box.width,box.height)*0.9);
        int size_h = size_w;
        int cx = box.x + box.width/2, cy = box.y + box.height/2;
        int x1 = cx - size_w/2, x2 = x1+ size_w;
        int y1 = cy - int(size_h*0.4), y2 = y1+ size_h;

        
        int left = x1<0?-x1:0;
        int top = y1<0?-y1:0;
        int right = x2>=img.cols?x2-img.cols:0;
        int bottom = y2>=img.rows?y2-img.rows:0;

        x1 = std::max<int>(0,x1);
        y1 = std::max<int>(0,y1);
        x2 = std::min<int>(img.cols,x2);
        y2 = std::min<int>(img.rows,y2);

        cropped = img(cv::Rect(x1,y1,x2-x1,y2-y1));
        cv::copyMakeBorder(cropped,cropped,top,bottom,left,right,cv::BORDER_CONSTANT,0);
        
        if (cropped.rows != params.inputHeight || cropped.cols != params.inputWidth)
            cv::resize(cropped, cropped, cv::Size(params.inputWidth, params.inputHeight));

        nnie::Mat in;
        nnie::resize_bilinear(cropped, in, params.inputWidth, params.inputHeight, params.inputC);

        net.run(in.im);

        landmarks = net.getOutputTensor(0);
        pose = net.getOutputTensor(1);

        int n = landmarks.width /2;
        for(int i = 0;i< n;++i)
        {
            landmarks.data[i*2] = landmarks.data[i*2]*size_w + x1 -left;;
            landmarks.data[i*2+1] = landmarks.data[i*2+1]*size_h + y1 -bottom;
            cv::circle(img,cv::Point(landmarks.data[i*2],landmarks.data[i*2+1]),1,(0,255,255),2);
        }
        for(int i =0;i<3;++i)
        {
            pose.data[i] = pose.data[i] * 180 / PI;
        }
        
    #ifdef __DEBUG__
        printf("----------- landmark tensor -------------");
        printf("\n\nTensor h : %d\n", landmarks.height);
        printf("Tensor w : %d\n", landmarks.width);
        printf("Tensor c : %d\n", landmarks.channel);
        printf("Tensor n : %d\n", landmarks.n);
        printf("----------- pose tensor -------------");
        printf("\n\nTensor h : %d\n", pose.height);
        printf("Tensor w : %d\n", pose.width);
        printf("Tensor c : %d\n", pose.channel);
        printf("Tensor n : %d\n", pose.n);
    #endif
        free(in.im);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    return true;

}
void FaceAlign::preProcess(cv::Mat& img,cv::Rect& box,cv::Mat& faceImg)
{
    int size_w = int(std::max<int>(box.width,box.height)*0.9);
    int size_h = size_w;
    int cx = box.x + box.width/2, cy = box.y + box.height/2;
    int x1 = cx - size_w/2, x2 = x1+ size_w;
    int y1 = cy - int(size_h*0.4), y2 = y1+ size_h;

    
    int left = x1<0?-x1:0;
    int top = y1<0?-y1:0;
    int right = x2>=img.cols?x2-img.cols:0;
    int bottom = y2>=img.rows?y2-img.rows:0;

    x1 = std::max<int>(0,x1);
    y1 = std::max<int>(0,y1);
    x2 = std::min<int>(img.cols,x2);
    y2 = std::min<int>(img.rows,y2);

    faceImg = img(cv::Rect(x1,y1,x2-x1,y2-y1));
    cv::copyMakeBorder(faceImg,faceImg,top,bottom,left,right,cv::BORDER_CONSTANT,0);
    //std::cout<< "coped img : " << faceImg.rows << " " << faceImg.cols << std::endl;

}