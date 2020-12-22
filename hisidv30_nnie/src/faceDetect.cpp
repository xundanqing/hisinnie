//
// Created by surui on 2020/7/1.
//

#include "faceDetect.h"


FaceDetector::FaceDetector(dictionary *config)
{
    params.modelPath =  iniparser_getstring(config, "faceDetector:modelPath", "./models/faceDetector/slim-320_inst.wk");;
    // You can reference your prototxt to fill these field.
    params.batchSize = 1;
    params.inputHeight = 240;
    params.inputWidth = 320;
    params.inputC = 3;

    if (!validateGparams(params))
    {
        perror("[ERROR] Check your gparams !\n\n");
    }
}



bool FaceDetector::validateGparams(nnie::gParams gparams)
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

FaceDetector::~FaceDetector()
{
    net.clear();
}

bool FaceDetector::build()
{
    try
    {   
        define_img_size();
        generate_priors();
        net.load_model(params.modelPath.c_str());
        return true;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }

}


bool FaceDetector::doInference(cv::Mat& img, cv::Rect& max_face_box)
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

        nnie::Mat boxes = net.getOutputTensor(0);
        nnie::Mat scores = net.getOutputTensor(1);
        find_max_face(boxes,scores,img.rows,img.cols,max_face_box);
    //cout << max_face_box.x << " " << max_face_box.y << " " << max_face_box.width << " " << max_face_box.height << endl;
    //    cv::rectangle(img,cv::Point(max_face_box.x,max_face_box.y),cv::Point(max_face_box.x+max_face_box.width-1,max_face_box.y+max_face_box.height-1),cv::Scalar(0,0,255),4);
    //     cv::imwrite("output.jpg",img);


    #ifdef __DEBUG__
        printf("---------  boxes information -----");
        printf("\n\nTensor h : %d\n", boxes.height);
        printf("Tensor w : %d\n", boxes.width);
        printf("Tensor c : %d\n", boxes.channel);
        printf("Tensor n : %d\n", boxes.n);
        printf("---------  scores information -----");
        printf("\n\nTensor h : %d\n", scores.height);
        printf("Tensor w : %d\n", scores.width);
        printf("Tensor c : %d\n", scores.channel);
        printf("Tensor n : %d\n", scores.n);
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


void FaceDetector::define_img_size()
{
    for(int i = 0;i<image_size.size();++i)
    {   std::vector<int> feature_map;
        for(int j = 0;j<strides.size();++j)
        {
            feature_map.emplace_back(std::ceil(image_size[i]/strides[j]));
        }
        feature_map_w_h_list.emplace_back(feature_map);
    }
    #ifdef __DEBUG__
    std::cout<< "feature_map_w_h_list:" <<std::endl;
    print<int>(feature_map_w_h_list);
    #endif
    for(int i = 0;i<image_size.size();++i)
    {
        shrinkage_list.emplace_back(strides);
    }
    #ifdef __DEBUG__
    std::cout<< "shrinkage_list:" <<std::endl;
    print<float>(shrinkage_list);
    #endif

}

void FaceDetector::generate_priors()
{
    for(int index = 0;index < feature_map_w_h_list[0].size(); ++index)
    {
        float scale_w = image_size[0] / shrinkage_list[0][index];
        float scale_h = image_size[1] / shrinkage_list[1][index];
        for(int j = 0;j < feature_map_w_h_list[1][index];++j)
        {
            for(int i = 0; i < feature_map_w_h_list[0][index];++i)
            {
                float x_center = CLIP_ZERO_ONE((i+0.5) / scale_w);
                float y_center = CLIP_ZERO_ONE((j+0.5) / scale_h);
                for(int t = 0; t < min_boxes[index].size(); ++t)
                {
                    float min_box = min_boxes[index][t];
                    float w = CLIP_ZERO_ONE(min_box / image_size[0]);
                    float h = CLIP_ZERO_ONE(min_box / image_size[1]);
                    std::vector<float> pbox = {x_center,y_center,w,h};
                    priors.emplace_back(pbox);
                }

            }
        }
    }
    #ifdef __DEBUG__
    std::cout << "priors nums: " << priors.size() << endl;
    print<float>(priors);
    #endif
}

void FaceDetector::find_max_face(nnie::Mat& boxes, nnie::Mat& scores,int img_height,int img_width,cv::Rect& max_face_box)
{
    std::vector<cv::Rect> keep_boxes;
    std::vector<float> confidences;
    int n = boxes.channel;
    for(int i = 0;i<n; ++i)
    {
        if(scores.data[i*2+1] < conf_threshold)
            continue;
        
        float x_c = boxes.data[i*4+0] * center_variance * priors[i][2] + priors[i][0];
        float y_c = boxes.data[i*4+1] * center_variance * priors[i][3] + priors[i][1];
        float w = exp(boxes.data[i*4+2] * size_varicance) * priors[i][2];
        float h = exp(boxes.data[i*4+3] * size_varicance) * priors[i][3];

        float x1 = (x_c - w/2)*img_width;
        float y1 = (y_c - h/2)*img_height;
        float x2 = (x_c + w/2)*img_width;
        float y2 = (y_c + h/2)*img_height;
        w = x2-x1;
        h = y2-y1;
        keep_boxes.emplace_back(cv::Rect(x1,y1,w,h));
        confidences.emplace_back(scores.data[i*2+1]);
    }
    std::vector<int> indices;
    cv::dnn::NMSBoxes(keep_boxes,confidences,0.5,0.4,indices);
    if(indices.size()<1)
        return;
    int maxFaceIdx=indices[0],maxFaceArea=0;
    for(auto i:indices)
    {
        int faceArea = keep_boxes[i].width*keep_boxes[i].height;
        
        if(faceArea>maxFaceArea)
        {
            maxFaceIdx = i;
            maxFaceArea = faceArea;        
        }
        
    }
    max_face_box = keep_boxes[maxFaceIdx];
}



