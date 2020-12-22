#include "stdio.h"
#include "video_source.hpp"


VideoSource::VideoSource()
{
	
}

VideoSource::~VideoSource()
{
	if(VideoSource_.type == SOURCE_SINGLE_SENSOR)
	{
		Mpp_Video_Stop_Single_ViVpssVo();
	}
	else if(VideoSource_.type == SOURCE_DOUBLE_SENSOR)
	{
		Mpp_Video_Stop_Double_ViVpssVo();
	}
	else
	{
		Mpp_Video_Stop_File_Vdec_Vpss();
	}
}

int VideoSource::Init(VIDEO_SOURCE_ST *video_source)
{
	int ret = -1;
	memcpy(&VideoSource_,video_source,sizeof(VIDEO_SOURCE_ST));
	
	if(VideoSource_.type== SOURCE_SINGLE_SENSOR)
	{
		ret = Mpp_Video_Start_Single_ViVpssVo(VideoSource_.imagesize[SENSOR_0]);
	}
	else if(VideoSource_.type == SOURCE_DOUBLE_SENSOR)
	{
		ret = Mpp_Video_Start_Double_ViVpssVo(VideoSource_.imagesize[SENSOR_0],VideoSource_.imagesize[SENSOR_1]);
	}
	else if(VideoSource_.type == SOURCE_VIDEO_FILE)
	{
		ret = Mpp_Video_Start_File_Vdec_Vpss(VideoSource_.imagesize[SENSOR_0]);
	}
	else
	{
		printf("VideoSrctype_ error type = %d\n",VideoSource_.type);			
	}
	if(ret != HI_SUCCESS)
	{
		printf("VideoSource init failed\n");	
		return -1;
	}
	return ret;
	
}

int VideoSource::GetFrame(SENSOR_NUM_E sensor_num,STREAM_NUM_E stream_num,VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	int ret = -1;
	if(VideoSource_.type == SOURCE_SINGLE_SENSOR || VideoSource_.type == SOURCE_VIDEO_FILE)
	{
		sensor_num = SENSOR_0;
	}
	ret = Mpp_Video_Get_Frame_From_Vpss(sensor_num,stream_num,pstVideoFrame);
	if(ret != HI_SUCCESS)
	{
		printf("Mpp_Video_Get_Frame_From_Vpss error \n");
		return -1;
	}

	return ret;
}

int VideoSource::ReleaseFrame(SENSOR_NUM_E sensor_num,STREAM_NUM_E stream_num,VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	int ret = -1;
	if(VideoSource_.type == SOURCE_SINGLE_SENSOR || VideoSource_.type == SOURCE_VIDEO_FILE)
	{
		sensor_num = SENSOR_0;
	}
	ret = Mpp_Video_Release_VpssFrame(sensor_num,stream_num,pstVideoFrame);
	if(ret != HI_SUCCESS)
	{
		printf("Mpp_Video_Release_VpssFrame error \n");
		return -1;
	}

	return ret;
}

int VideoSource::SendToDisplay(SENSOR_NUM_E sensor_num,VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	int ret = -1;
	if(VideoSource_.type == SOURCE_SINGLE_SENSOR || VideoSource_.type == SOURCE_VIDEO_FILE)
	{
		sensor_num = SENSOR_0;
	}
	ret = Mpp_Video_Send_Frame_To_Vo(sensor_num,pstVideoFrame);
	if(ret != HI_SUCCESS)
	{
		printf("Mpp_Video_Send_Frame_To_Vo error \n");
		return -1;
	}

	return ret;
}


void VideoSource::Deinit()
{
	 if(VideoSource_.type == SOURCE_SINGLE_SENSOR)
	{
		Mpp_Video_Stop_Single_ViVpssVo();
	}
	else if(VideoSource_.type == SOURCE_DOUBLE_SENSOR)
	{
		Mpp_Video_Stop_Double_ViVpssVo();
	}
	else
	{
		Mpp_Video_Stop_File_Vdec_Vpss();
	}
}
