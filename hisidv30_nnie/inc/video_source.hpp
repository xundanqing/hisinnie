#ifndef VIDEO_SOURCE_HPP
#define VIDEO_SOURCE_HPP

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "mpp_video.h"



// ��ƵԴ����
typedef enum
{
	SOURCE_SINGLE_SENSOR = 0, //��·����ͷ��Ϊ��ƵԴ
	SOURCE_DOUBLE_SENSOR,     //˫·����ͷ��Ϊ��ƵԴ
	SOURCE_VIDEO_FILE,           //��Ƶ�ļ���Ϊ��ƵԴ
	
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
		//��ȡһ֡YUV ͼ��
		int GetFrame(SENSOR_NUM_E sensor_num,STREAM_NUM_E stream_num,VIDEO_FRAME_INFO_S *pstVideoFrame);
		//�ͷ�֡ͼ�������ȡ֡ͼ�����ʹ��
		int ReleaseFrame(SENSOR_NUM_E sensor_num,STREAM_NUM_E stream_num,VIDEO_FRAME_INFO_S *pstVideoFrame);
		//����֡���ݵ���Ƶ��ʾ
		int SendToDisplay(SENSOR_NUM_E sensor_num,VIDEO_FRAME_INFO_S *pstVideoFrame);
		
		void Deinit();
	protected:


		VIDEO_SOURCE_ST        VideoSource_;

};
#endif //VIDEO_SOURCE_HPP

