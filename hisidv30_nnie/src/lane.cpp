//
// Created by surui on 2020/7/1.
//

#include "lane.h"


Lane::Lane(dictionary *config)
{
    params.modelPath =  iniparser_getstring(config, "lane:modelPath", "./models/lane/lane_inst.wk");;
    // You can reference your prototxt to fill these field.
    params.batchSize = 1;
    params.inputHeight = 288;
    params.inputWidth = 800;
    params.inputC = 3;

    if (!validateGparams(params))
    {
        perror("[ERROR] Check your gparams !\n\n");
    }
}



bool Lane::validateGparams(nnie::gParams gparams)
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

Lane::~Lane()
{
    net.clear();
}

bool Lane::build()
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


bool Lane::doInference(cv::Mat& img, GlobalInfo& globalInfo)
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
#ifdef __DEBUG__
        struct timeval tv1;
        struct timeval tv2;
        long t1, t2, time;
        gettimeofday(&tv1, NULL);
#endif
        net.run(in.im);
        nnie::Mat out = net.getOutputTensor(0);
#ifdef __DEBUG__
        gettimeofday(&tv2, NULL);
        t1 = tv2.tv_sec - tv1.tv_sec;
        t2 = tv2.tv_usec - tv1.tv_usec;
        time = (long) (t1 * 1000 + t2 / 1000);
        printf("lane detection time : %dms\n", time);
#endif
        std::vector<float> loc(36,0);
        postProcess(out,loc);

        if(!goodLane(loc))
            globalInfo.laneInformation.departSignal = -2; //未检测到车道线
        else
        {
            /* 如果检测到车道线,需要判断车道线是否向左或者向右偏离 */
            //std::vector<cv::Point> leftLanePoints,rightLanePoints;
            bool findLaneStartPoint = false;
            int lx,rx;
            for(int k = 17;k>=0;--k)
            {
                if(loc[k*2+0] > 0 && loc[k*2+1] > 0)
                {  
                    int lx0 = int(loc[k*2+0]*col_sample_w*img.cols/params.inputWidth)-1;
                    int ly0 = int(img.rows - (17-k)*20)-1;
                    int lx1  = int(loc[k*2+1]*col_sample_w*img.cols/params.inputWidth)-1;
                    int ly1 = int(img.rows - (17-k)*20)-1;
                    // leftLanePoints.emplace_back(cv::Point(lx0,ly0));
                    // rightLanePoints.emplace_back(cv::Point(lx1,ly1));
                    globalInfo.laneInformation.lanePoints.emplace_back(std::make_pair(cv::Point(lx0,ly0),cv::Point(lx1,ly1)));
                    if(!findLaneStartPoint)
                    {
                        lx = lx0;
                        rx = lx1;
                        findLaneStartPoint = true;
                    }
                }
            }
            // leftLaneSlope = estiameLineSlopeRate(leftLanePoints);
            // rightLaneSlope = estiameLineSlopeRate(rightLanePoints);
            float carPosition = ((img.cols/2.) - (lx + rx)/2.)*(3.7/(rx - lx));
            globalInfo.laneInformation.departDistance = carPosition;
            //判断车道左右偏离
            if(carPosition > 0.76){
                globalInfo.laneInformation.departSignal = 1; //右偏离
            }
            else if (carPosition < -0.76){
                globalInfo.laneInformation.departSignal = -1; //左偏离
            }
            else{
                globalInfo.laneInformation.departSignal = 0; // 未偏离
            }
// #ifdef __DEBUG__
//             char tmp[20];
//             sprintf(tmp,"%.2lf",abs(carPosition));
//             std::string distance(tmp);
//             std::string car_pos_text;
//             if(carPosition >0)
//                 car_pos_text = "Right deviation " + distance + " m.";
//             else
//                 car_pos_text = "Left deviation " + distance + " m.";
//             cv::putText(img,car_pos_text,cv::Point(1000,100),cv::FONT_HERSHEY_SIMPLEX,1,(0,0,255),2);
//             if(laneDepartInfo==-1)
//                 cv::putText(img,"Warning: Serious Left Deviation!",cv::Point(1000,150),cv::FONT_HERSHEY_SIMPLEX,1,(0,0,255),2);
//             else if(laneDepartInfo ==1)
//                 cv::putText(img,"Warning: Serious Right Deviation!",cv::Point(1000,150),cv::FONT_HERSHEY_SIMPLEX,1,(0,0,255),2);
// #endif
        }

//行驶前方区域绘图
//#ifdef __DEBUG__
            // //画出行驶前方的危险区域
            // cv::Point Points[4];
            // Points[0] = cv::Point(200,590);
            // Points[1] = cv::Point(780,280);
            // Points[2] = cv::Point(1400,590);
            // Points[3] = cv::Point(860,280);
            //cv::fillConvexPoly(img,Points,2,(128,128,255));
            // cv::line(img,Points[0],Points[1],(0,255,128),2,cv::LINE_4);
            // cv::line(img,Points[2],Points[3],(0,255,128),2,cv::LINE_4);
//#endif
//画车道线
//  #ifdef __DEBUG__
//         for(int k = 17;k>=1;--k)
//         {
//             if(loc[k*2+0] > 0 && loc[k*2+1] > 0 && loc[(k-1)*2+0] > 0 && loc[(k-1)*2+1] > 0)
//             {
//                 cv::Point p0 = cv::Point(int(loc[k*2+0]*col_sample_w*img.cols/params.inputWidth)-1,int(img.rows - (17-k)*20)-1);
//                 cv::Point p1 = cv::Point(int(loc[k*2+1]*col_sample_w*img.cols/params.inputWidth)-1,int(img.rows - (17-k)*20)-1);
//                 cv::Point p2 = cv::Point(int(loc[(k-1)*2+0]*col_sample_w*img.cols/params.inputWidth)-1,int(img.rows - (17-k+1)*20)-1);
//                 cv::Point p3 = cv::Point(int(loc[(k-1)*2+1]*col_sample_w*img.cols/params.inputWidth)-1,int(img.rows - (17-k+1)*20)-1);
//                 cv::circle(img,p0,5,(0,0,255),-1);
//                 cv::circle(img,p1,5,(0,0,255),-1);
//                 cv::line(img,p0,p2,(255,255,255),1);
//                 cv::line(img,p1,p3,(255,255,255),1);
//                 cv::Point PointArray[4];
//                 PointArray[0] = p2;
//                 PointArray[1] = p3;
//                 PointArray[2] = p1;
//                 PointArray[3] = p0;

//                 //cv::fillConvexPoly(img,PointArray,4,(0,255,255));
//             }
//         }
//         std::string quality = goodLane(loc)?"good lane!":"bad lane!";
//         std::cout << goodLane(loc) << quality << std::endl;
//         cv::putText(img,quality,cv::Point(1000,50),cv::FONT_HERSHEY_SIMPLEX,1,(0,0,255),2);
//         // cv::imwrite(savePath,img); 
//         // std::cout << savePath << std::endl;
// #endif       
        
// #ifdef __DEBUG__
//         printf("----------- out tensor -------------");
//         printf("\n\nTensor h : %d\n", out.height);
//         printf("Tensor w : %d\n", out.width);
//         printf("Tensor c : %d\n", out.channel);
//         printf("Tensor n : %d\n", out.n);
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
void Lane::postProcess(nnie::Mat& out,std::vector<float>& loc)
{
    std::vector<float> prob(200*18*2,0);
    for(int w = 1;w<3;++w)
    {
        for(int h = 0;h< 18;++h)
        {
            float max = std::numeric_limits<float>::min();
            float max_id = 0;
            float sum = 0.0;
            for(int c = 0;c <200;++c)
            {
                int id_old = c*72+h*4+w;
                if(max < out.data[id_old])
                {
                   max = out.data[id_old];
                   max_id = c;
                }
            }
            if(max < out.data[200*72+h*4+w])
                max_id = 200;
            for(int c= 0;c<200;++c)
            {
                int id_old = c*72+h*4+w, id_new = c*36+h*2+w-1;
                prob[id_new] = expf(out.data[id_old]-max);
                sum += prob[id_new];
            }
            if(max_id == 200)
                continue;
            for(int c=0;c<200;++c)
            {
                    int id_new = c*36+h*2+w-1;
                    prob[id_new] /= sum;
                    prob[id_new] *= c;
                    loc[h*2+w-1] += prob[id_new];
            }

        }
    }
    
}

bool Lane::goodLane(std::vector<float>& loc)
{
    int detCount = 0,wrongDet = 0;
    for(int k = 17;k>=0;--k)
    {
        if(int(loc[k*2+1])!=0 && int(loc[k*2+0])!=0)
        {
            ++detCount;
            if(k>0&&(loc[k*2+1] - loc[k*2+0]) < (loc[(k-1)*2+1] - loc[(k-1)*2+0]))
                ++wrongDet;
        }
    }
// #ifdef __DEBUG__
//     std::cout << "detCount: " << detCount << std::endl;
//     std::cout << "wrongDet: " << wrongDet << std::endl;
//     std::cout << (wrongDet < detCount/3) << std::endl;
// #endif
    if(detCount > 3 && wrongDet <3)
        return true;
    return false;
}