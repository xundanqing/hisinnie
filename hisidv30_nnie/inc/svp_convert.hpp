/******************************************************************************

  Copyright (C), 2001-2011, DCN Co., Ltd.

 ******************************************************************************
  File Name     : svp_convert.h
  Version       : Initial Draft
  Author        : x
  Created       : 2020/7/23
  Last Modified :
  Description   : Svp  图像先处理类
  Function List :
  History       :
  1.Date        : 2020/7/23
    Author      : x
    Modification: Created file

******************************************************************************/

#ifndef  _SVP_CONVERT_H
#define  _SVP_CONVERT_H

#include <opencv2/opencv.hpp>
#include "mpi_ive.h"
#include "hi_debug.h"
#include "hi_comm_ive.h"
#include "hi_comm_vgs.h"
#include "hi_comm_video.h"
#include "sample_comm_ive.h"
#include "sample_comm_nnie.h"

/*****************************************************/
#define  ALIGN2(val)   (val - val%2)
#define  ALIGN16(val) ((((val) + 16 - 1)/16)*16)


/*****************************************************/
typedef struct  st_SVP_NNIE_RECT_ARRAY_S
 {
     HI_U32 u32TotalNum;
     SAMPLE_IVE_RECT_S astRect[SAMPLE_SVP_NNIE_MAX_ROI_NUM_OF_CLASS];
 } st_SVP_NNIE_RECT_ARRAY_S;


class  Svp_Convert
{


  public:
    
      static   HI_S32   Svp_CoverRect(VIDEO_FRAME_INFO_S *pstFrmInfo, st_SVP_NNIE_RECT_ARRAY_S* pstRect, HI_U32 u32Color);
  
      static   HI_S32   Svp_CoverPoint(VIDEO_FRAME_INFO_S *pstFrmInfo, VGS_DRAW_LINE_S*   stVgsDrawLine,int  num);
      
      static   HI_S32   Svp_CscImg_Creat(VIDEO_FRAME_INFO_S *   stExtFrmInfo,IVE_IMAGE_S  *pImg,IVE_CSC_CTRL_S mode,IVE_IMAGE_TYPE_E  dstype);
      
      static   HI_S32   Svp_CscImg_Free(IVE_IMAGE_S  *pImg);
      
      static   HI_S32   Svp_CscImg_Convert(VIDEO_FRAME_INFO_S *   stExtFrmInfo,IVE_IMAGE_S  *pImg,IVE_CSC_MODE_E mode,IVE_IMAGE_TYPE_E  dstype);
      
      static   HI_S32   Svp_CropImg_Free(IVE_IMAGE_S* pstDstData);
      
      static    HI_S32  Svp_CropImg_Convet(IVE_IMAGE_S* pstSrcData,IVE_IMAGE_S* pstDstData, POINT_S  point_start,int  width,int  height);
      
      static   HI_S32   Svp_ResizeImg_Free(IVE_IMAGE_S* pstDstData);
      
      static   HI_S32   Svp_ResizeImg_Convert(IVE_IMAGE_S* pstSrcData,IVE_IMAGE_S* pstDstData,int  width,int  height);

      static   HI_S32   Svp_BgrImg_Dump(IVE_IMAGE_S* pstSrcData,char  *path);

      static   HI_S32   Svp_YuvImg_Dump();

	static HI_S32   Svp_Bgr2Mat(cv::Mat& img, IVE_IMAGE_S* pstSrcData);
  
};


#endif
