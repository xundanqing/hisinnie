//
// Created by surui on 2020/7/1.
//

#include "faceRecognition.h"


FaceRecognition::FaceRecognition(dictionary *config)
{
    params.modelPath =  iniparser_getstring(config, "faceRcognition:modelPath", "./models/faceRecog/mobilefacenet_inst.wk");;
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



bool FaceRecognition::validateGparams(nnie::gParams gparams)
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

FaceRecognition::~FaceRecognition()
{
    net.clear();
}

bool FaceRecognition::build()
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


bool FaceRecognition::doInference(cv::Mat input_img, std::vector<float>& features)
{
    try
    {
        if (input_img.rows != params.inputHeight || input_img.cols != params.inputWidth)
            cv::resize(input_img, input_img, cv::Size(params.inputWidth, params.inputHeight));

        nnie::Mat in;
        nnie::resize_bilinear(input_img, in, params.inputWidth, params.inputHeight, params.inputC);

        net.run(in.im);

        nnie::Mat feat = net.getOutputTensor(0);
        features.reserve(512);
        float sum = 0.;
        for(int i = 0;i< 512;i++)
        {
            float value = feat.data[i] ;
            sum = sum + value*value;
        } 
        std::cout << "sum: " << sum << std::endl;
  

    #ifdef __DEBUG__
        printf("----------- feat tensor -------------");
        printf("\n\nTensor h : %d\n", feat.height);
        printf("Tensor w : %d\n", feat.width);
        printf("Tensor c : %d\n", feat.channel);
        printf("Tensor n : %d\n", feat.n);
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
