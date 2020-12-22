
#ifndef _MPP_VIDEO_H_
#define _MPP_VIDEO_H_

#include "sample_comm.h"


#ifdef __cplusplus
extern "C" {
#endif


typedef enum
{
	SENSOR_0 = 0,
	SENSOR_1 = 1,
	SENSOR_NUM = 2,
}SENSOR_NUM_E;

typedef enum
{
	BASE_STREAM = 0,
	EXT_STREAM = 1,	
	STREAM_NUM = 2,
}STREAM_NUM_E;
HI_S32 Mpp_Video_Start_Single_ViVpssVo(SIZE_S extsize);

HI_VOID Mpp_Video_Stop_Single_ViVpssVo(void);

HI_S32 Mpp_Video_Start_Double_ViVpssVo(SIZE_S extsize0,SIZE_S extsize1);

HI_VOID Mpp_Video_Stop_Double_ViVpssVo(void);

HI_S32 Mpp_Video_Start_File_Vdec_Vpss(SIZE_S extsize);

HI_VOID  Mpp_Video_Stop_File_Vdec_Vpss(void);

HI_S32 Mpp_Video_Get_Frame_From_Vpss(SENSOR_NUM_E sensor_num,STREAM_NUM_E stream_num,VIDEO_FRAME_INFO_S *pstVideoFrame);

HI_S32 Mpp_Video_Send_Frame_To_Vo(SENSOR_NUM_E sensor_num,VIDEO_FRAME_INFO_S *pstVideoFrame);

HI_S32 Mpp_Video_Release_VpssFrame(SENSOR_NUM_E sensor_num,STREAM_NUM_E stream_num,VIDEO_FRAME_INFO_S *pstVideoFrame);

#ifdef __cplusplus
}
#endif

#endif