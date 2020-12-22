#ifndef NNIE_VIDEO_CAPUTRE_H
#define NNIE_VIDEO_CAPUTRE_H
#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include "iniparser.h"
#include "dictionary.h"
#include "video_source.hpp"
#include "svp_convert.hpp"

using namespace std;
using namespace cv;

class NNIEVideoCapture
{
public:
	NNIEVideoCapture(dictionary *config);
	~NNIEVideoCapture();
	bool isOpened();
	bool read(SENSOR_NUM_E num,cv::Mat& frame);

    /*modify*/
    bool read(SENSOR_NUM_E num,STREAM_NUM_E CHAN_EN,VIDEO_FRAME_INFO_S *pstFrame);

	void release();
	bool display(SENSOR_NUM_E num,cv::Rect& box,cv::Point aline_point[]);
private:
	VIDEO_SOURCE_ST        VideoSourceInfo_;
	VideoSource                   video_source_;
	Svp_Convert                  svp_convert_;
	bool                               openflag_ = false;
	VIDEO_FRAME_INFO_S    VideoFrameInfo_[SENSOR_NUM][STREAM_NUM];
};


#endif //NNIE_VIDEO_CAPUTRE_H
