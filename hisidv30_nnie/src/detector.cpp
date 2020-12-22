//
// Created by surui on 2020/7/1.
//

#include "detector.h"


Detector::Detector(dictionary *config)
{
    params.modelPath =  iniparser_getstring(config, "detector:modelPath", "./models/detector/det_inst.wk");;
    // You can reference your prototxt to fill these field.
    params.batchSize = 1;
    params.inputHeight = 416;
    params.inputWidth = 416;
    params.inputC = 3;

    if (!validateGparams(params))
    {
        perror("[ERROR] Check your gparams !\n\n");
    }
}



bool Detector::validateGparams(nnie::gParams gparams)
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

Detector::~Detector()
{
    net.clear();
}

bool Detector::build()
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


bool Detector::doInference(cv::Mat& img,GlobalInfo& globalInfo)
{
    try
    {
        cv::Mat input_img;
        if (img.rows != params.inputHeight || img.cols != params.inputWidth)
            cv::resize(img, input_img, cv::Size(params.inputWidth, params.inputHeight));
        else
            input_img = img;

        nnie::Mat in;
        nnie::resize_bilinear(input_img, in, params.inputWidth, params.inputHeight, params.inputC);

        net.run(in.im);
        nnie::Mat outputs[3];
        outputs[0] = net.getOutputTensor(0);
        outputs[1] = net.getOutputTensor(1);
        outputs[2] = net.getOutputTensor(2);

        std::vector<int> ids;
        std::vector<cv::Rect> boxes;
        std::vector<float> confidences;

        for(int yolo_id = 0;yolo_id<3;++yolo_id)
            parseYolov3Feature(img.cols,
                            img.rows,
                            numClasses,
                            kBoxPerCell,
                            yolo_id,
                            confThreshold,
                            anchors[yolo_id],
                            outputs[yolo_id],
                            ids,
                            boxes,
                            confidences);
        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);
        ObjectInfo obj;
        for(size_t i = 0;i<indices.size();++i)
        {
            obj.box = boxes[indices[i]];
            obj.idx = ids[indices[i]];
            //std::cout << "idx: " << obj.idx << std::endl;
            obj.score = confidences[i];
            //cv::rectangle(img,box,(255,255,255),2);
            int xc = obj.box.x + obj.box.width/2;
            int yc = obj.box.y + obj.box.height/2;
            
            //判断目标是否在前方行驶方向
            if( yc > 260 && yc < 590 && xc > (yc-590)/(-0.567)+200 && xc < (yc-590)/0.611 + 1400)
            {
                obj.isFrontal = true;
                switch (obj.idx)
                {
                    //车辆距离估计
                    case 89:
                    case 90:
                    case 91:
                    case 92:
                    case 93:
                    case 94:
                        obj.distance = actualCarWidth / obj.box.width * focalDist;
                        break;
                    //行人距离估计
                    case 88:
                        obj.distance = actualPersonHeight / obj.box.height * focalDist;
                        break;
                    default:
                        break;
                }
            }
            else{
                obj.isFrontal = false;
            }
            globalInfo.objects.emplace_back(obj);
        }

        
    // #ifdef __DEBUG__
    //     printf("----------- output0 tensor -------------");
    //     printf("\n\nTensor h : %d\n", outputs[0].height);
    //     printf("Tensor w : %d\n", outputs[0].width);
    //     printf("Tensor c : %d\n", outputs[0].channel);
    //     printf("Tensor n : %d\n", outputs[0].n);

    // #endif
        free(in.im);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    return true;

}
void Detector::preProcess(cv::Mat& img,cv::Mat& input_img)
{
    int h = img.rows, w = img.cols;
    int left,right,top,bottom;
    if(h>w)
    {
        left = (h-w)/2;
        right = h-w - left;
        top = 0;
        bottom =0;
    }
    else
    {
        top = (w-h)/2;
        bottom = h - top;
        left = 0;
        right =0;
    }    
    cv::copyMakeBorder(img,input_img,top,bottom,left,right,cv::BORDER_CONSTANT,(128.128,128));
}