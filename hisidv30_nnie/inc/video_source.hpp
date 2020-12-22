#ifndef VIDEO_SOURCE_HPP
#define VIDEO_SOURCE_HPP

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "mpp_video.h"



// 视频源类型
typedef enum
{
	SOURCE_SINGLE_SENSOR = 0, //单路摄像头作为视频源
	SOURCE_DOUBLE_SENSOR,     //双路摄像头作为视频源
	SOURCE_VIDEO_FILE,           //视频文件作为视频源
	
}VIDEO_SOURCE_TYPE_E;

typedef struct
{
	VIDEO_SOURCE_TYPE_E type;
	SIZE_S                          imagesize[SENSOR_NUM];
}VIDEO_SOURCE_ST;
class VideoSource
{

	public:
		VideoSource();

		~VideoSource();
	public:
		int Init(VIDEO_SOURCE_ST *video_source);
		//获取一帧YUV 图像
		int GetFrame(SENSOR_NUM_E sensor_num,STREAM_NUM_E stream_num,VIDEO_FRAME_INFO_S *pstVideoFrame);
		//释放帧图像，需与获取帧图像配合使用
		int ReleaseFrame(SENSOR_NUM_E sensor_num,STREAM_NUM_E stream_num,VIDEO_FRAME_INFO_S *pstVideoFrame);
		//发送帧数据到视频显示
		int SendToDisplay(SENSOR_NUM_E sensor_num,VIDEO_FRAME_INFO_S *pstVideoFrame);
		
		void Deinit();
	protected:


		VIDEO_SOURCE_ST        VideoSource_;

};
#endif //VIDEO_SOURCE_HPP

