#include "svp_convert.hpp"

/*****************************************************************************
 Prototype    : Svp_Convert.Svp_CoverRect
 Description  : 叠加矩形信息在视频码流
 Input        : VIDEO_FRAME_INFO_S *pstFrmInfo     
                st_SVP_NNIE_RECT_ARRAY_S* pstRect   矩形框数组 
                HI_U32 u32Color                     矩形框颜色          
 Output       : None
 Return Value : HI_S32
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2020/7/23
    Author       : x
    Modification : Created function

*****************************************************************************/
HI_S32  Svp_Convert:: Svp_CoverRect(VIDEO_FRAME_INFO_S *pstFrmInfo, st_SVP_NNIE_RECT_ARRAY_S* pstRect, HI_U32 u32Color)
{
        HI_U32 i,j;
        HI_S32 s32Ret;
        VGS_HANDLE VgsHandle = -1;
        VGS_TASK_ATTR_S stVgsTask;
        VGS_ADD_COVER_S stVgsAddCover;
        static HI_U32   u32Frm = 0;
    
    
        u32Frm++;
        if (0 == pstRect->u32TotalNum)
        {
            return s32Ret;
        }
        s32Ret = HI_MPI_VGS_BeginJob(&VgsHandle);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("Vgs begin job fail,Error(%#x)\n", s32Ret);
            return s32Ret;
        }
    
        memcpy(&stVgsTask.stImgIn ,pstFrmInfo,  sizeof(VIDEO_FRAME_INFO_S));
        memcpy(&stVgsTask.stImgOut,pstFrmInfo,  sizeof(VIDEO_FRAME_INFO_S));

        stVgsAddCover.u32Color = u32Color;
        stVgsAddCover.enCoverType = COVER_QUAD_RANGLE;
        stVgsAddCover.stQuadRangle.bSolid = HI_FALSE;
        stVgsAddCover.stQuadRangle.u32Thick = 2;
    
        for (j = 0; j < pstRect->u32TotalNum; j++)
        {
                memcpy(stVgsAddCover.stQuadRangle.stPoint, pstRect->astRect[j].astPoint, sizeof(pstRect->astRect[j].astPoint));            
                s32Ret = HI_MPI_VGS_AddCoverTask(VgsHandle, &stVgsTask, &stVgsAddCover);
                if (s32Ret != HI_SUCCESS)
                {
                    SAMPLE_PRT("HI_MPI_VGS_AddCoverTask fail,Error(%#x)\n", s32Ret);
                    HI_MPI_VGS_CancelJob(VgsHandle);
                    return s32Ret;
                }
    
        }
    
    
        s32Ret = HI_MPI_VGS_EndJob(VgsHandle);
        if (s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("HI_MPI_VGS_EndJob fail,Error(%#x)\n", s32Ret);
            HI_MPI_VGS_CancelJob(VgsHandle);
            return s32Ret;
        }
    
        return s32Ret;
}

/*****************************************************************************
 Prototype    : Svp_Convert.Svp_ConverPoint
 Description  : 打点到视频码流
 Input        : VIDEO_FRAME_INFO_S *pstFrmInfo    
                VGS_DRAW_LINE_S*   stVgsDrawLine  
                int  num                          
 Output       : None
 Return Value : HI_S32
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2020/7/23
    Author       : x
    Modification : Created function

*****************************************************************************/
HI_S32  Svp_Convert:: Svp_CoverPoint(VIDEO_FRAME_INFO_S *pstFrmInfo, VGS_DRAW_LINE_S*   stVgsDrawLine,int  num)
{

        HI_S32 s32Ret;
        VGS_TASK_ATTR_S  stDrawLineVgsTask;
        VGS_HANDLE       VgsDrawLineHandle;

        
        s32Ret = HI_MPI_VGS_BeginJob(&VgsDrawLineHandle);
        if(s32Ret!= HI_SUCCESS)
        {
             printf("HI_MPI_VGS_BeginJob DrawLine error %x\r\n",s32Ret);
             return   -1;
        }
        
        memcpy(&stDrawLineVgsTask.stImgIn ,pstFrmInfo,sizeof(VIDEO_FRAME_INFO_S));
        memcpy(&stDrawLineVgsTask.stImgOut,pstFrmInfo,sizeof(VIDEO_FRAME_INFO_S));

        for(int  idraw  = 0 ; idraw < num; idraw++)
        {
        
            stVgsDrawLine[idraw].stStartPoint.s32X = ALIGN2(stVgsDrawLine[idraw].stStartPoint.s32X);
            stVgsDrawLine[idraw].stStartPoint.s32Y = ALIGN2(stVgsDrawLine[idraw].stStartPoint.s32Y);
            stVgsDrawLine[idraw].stEndPoint.s32X  =  ALIGN2(stVgsDrawLine[idraw].stEndPoint.s32X);
            stVgsDrawLine[idraw].stEndPoint.s32Y  =  ALIGN2(stVgsDrawLine[idraw].stEndPoint.s32Y);
#if  0
            if(stVgsDrawLine[idraw].stStartPoint.s32X < 0||   stVgsDrawLine[idraw].stStartPoint.s32Y < 0||
               stVgsDrawLine[idraw].stEndPoint.s32X   < 0||   stVgsDrawLine[idraw].stEndPoint.s32Y < 0)
            {
                printf("Svp_ConverPoint point[%d] param  < 0 {%d, %d}",idraw,stVgsDrawLine[idraw].stStartPoint.s32Y, \
                                                                                     stVgsDrawLine[idraw].stStartPoint.s32Y
                                                                                     );
                return  -1;
            }
 #endif
        }
        
        s32Ret =HI_MPI_VGS_AddDrawLineTaskArray(VgsDrawLineHandle,&stDrawLineVgsTask, stVgsDrawLine, 98);
        if(s32Ret!= HI_SUCCESS)
        {
             printf("HI_MPI_VGS_AddDrawLineTaskArray error %x\r\n",s32Ret);
             goto END_VGS;
        }
        
        
   END_VGS:
    
        s32Ret = HI_MPI_VGS_EndJob(VgsDrawLineHandle);
        if(s32Ret!= HI_SUCCESS)
        {
             printf("HI_MPI_VGS_EndJob   DrawLine error %x\r\n",s32Ret);
             return   -1;
        }
        
        
 
}

/*****************************************************************************
 Prototype    : Svp_Convert.Svp_CscImg_Creat
 Description  : 分配转换数据内存
 Input        : VIDEO_FRAME_INFO_S *   stExtFrmInfo  
                IVE_IMAGE_S  *pImg                   
                IVE_CSC_CTRL_S mode                  
                IVE_IMAGE_TYPE_E  dstype             
 Output       : None
 Return Value : HI_S32
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2020/7/23
    Author       : x
    Modification : Created function

*****************************************************************************/
HI_S32  Svp_Convert:: Svp_CscImg_Creat(VIDEO_FRAME_INFO_S *   stExtFrmInfo,IVE_IMAGE_S  *pImg,IVE_CSC_CTRL_S mode,IVE_IMAGE_TYPE_E  dstype)
{
     HI_S32     s32Ret;
     HI_BOOL    bFinish;
     HI_U32     u32Size;

     u32Size =  stExtFrmInfo->stVFrame.u32Width * stExtFrmInfo->stVFrame.u32Height * 3;

     s32Ret = HI_MPI_SYS_MmzAlloc(&pImg->au64PhyAddr[0], (void**)&pImg->au64VirAddr[0],"OutputImage", HI_NULL, u32Size);

     
     if(s32Ret != HI_SUCCESS)
     {
           printf("can't alloc OutputImage memory for %x\n",s32Ret);
     }


     return  0;

}
/*****************************************************************************
 Prototype    : Svp_Convert.Svp_CscImg_Free
 Description  : 释放转换内存
 Input        : IVE_IMAGE_S  *pImg  
 Output       : None
 Return Value : HI_S32
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2020/7/23
    Author       : x
    Modification : Created function

*****************************************************************************/
HI_S32  Svp_Convert:: Svp_CscImg_Free(IVE_IMAGE_S  *pImg)
{
       if (pImg->au64VirAddr[0] != 0)
       {
            HI_MPI_SYS_MmzFree(pImg->au64PhyAddr[0], (void*)pImg->au64VirAddr[0]);
       }
}
/*****************************************************************************
 Prototype    : Svp_Convert.Svp_CscImg_Convert
 Description  : yuv 转 BGR 
 Input        : VIDEO_FRAME_INFO_S *   stExtFrmInfo  
 Output       : None
 Return Value : HI_S32
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2020/7/23
    Author       : x
    Modification : Created function

*****************************************************************************/
HI_S32  Svp_Convert:: Svp_CscImg_Convert(VIDEO_FRAME_INFO_S *   pstExtFrmInfo,IVE_IMAGE_S  *pImg,IVE_CSC_MODE_E  mode,IVE_IMAGE_TYPE_E  dstype)
{

        HI_S32     s32Ret;
        HI_BOOL    bFinish;
        HI_U32     u32Size;
        HI_U32     uChanSize;
        IVE_CSC_CTRL_S     ive_ctrl;
        IVE_SRC_IMAGE_S    stSrc; 
        IVE_HANDLE         hIveHandle;

        if(pstExtFrmInfo == NULL || pImg == NULL)
        {
            printf("Svp_CscImg_Convert  parameter  is  null\r\n");
            return  -1;
        }

        
        stSrc.u32Width       = pstExtFrmInfo->stVFrame.u32Width;   
        stSrc.u32Height      = pstExtFrmInfo->stVFrame.u32Height; 
        stSrc.au32Stride [0] = pstExtFrmInfo->stVFrame.u32Stride[0];  
        stSrc.au32Stride [1] = pstExtFrmInfo->stVFrame.u32Stride[1];	
        stSrc.au64VirAddr[0] = pstExtFrmInfo->stVFrame.u64VirAddr[0];  
        stSrc.au64VirAddr[1] = pstExtFrmInfo->stVFrame.u64VirAddr[1];				                 
        stSrc.au64PhyAddr[0] = pstExtFrmInfo->stVFrame.u64PhyAddr[0];   
        stSrc.au64PhyAddr[1] = pstExtFrmInfo->stVFrame.u64PhyAddr[1];	
        stSrc.enType = IVE_IMAGE_TYPE_YUV420SP;	  

        uChanSize =   stSrc.u32Width  * stSrc.u32Height;
        u32Size   =   stSrc.u32Width  * stSrc.u32Height  * 3;
        
        s32Ret = HI_MPI_SYS_MmzAlloc(&pImg->au64PhyAddr[0], (void**)&pImg->au64VirAddr[0],"OutputImage", HI_NULL, u32Size);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_SYS_MmzAlloc Svp_CscImg_Convert  error\r\n");
            return  -1;
        }
        
        pImg->enType = dstype;
        pImg->u32Width = stSrc.u32Width;
        pImg->u32Height= stSrc.u32Height;
        
        for(int j =0;j < 3;j++)
        {
            pImg->au32Stride[j] = stSrc.u32Width;
            pImg->au64PhyAddr[j]= pImg->au64PhyAddr[0] + j*uChanSize;
            pImg->au64VirAddr[j]= pImg->au64VirAddr[0] + j*uChanSize;


        }

        ive_ctrl.enMode =  mode;
        
        s32Ret =  HI_MPI_IVE_CSC(&hIveHandle,&stSrc,pImg,&ive_ctrl, HI_TRUE);
        if(s32Ret!= HI_SUCCESS)
        {
           printf("HI_MPI_IVE_CSC error\r\n");
           return   -1;
        }

        s32Ret = HI_MPI_IVE_Query(hIveHandle,&bFinish,HI_FALSE); 
        if(s32Ret!= HI_SUCCESS)
        {
           printf("HI_MPI_IVE_Query error\r\n");
  
        }
	return HI_SUCCESS;
}

/*****************************************************************************
 Prototype    : Svp_Convert.Svp_CropImg_Free
 Description  : 释放转换内存
 Input        : IVE_DATA_S* pstDstData  
 Output       : None
 Return Value : HI_S32
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2020/7/23
    Author       : x
    Modification : Created function

*****************************************************************************/
HI_S32  Svp_Convert:: Svp_CropImg_Free(IVE_IMAGE_S* pstDstData)
{
       if (pstDstData->au64VirAddr[0]!= 0)
       {
            HI_MPI_SYS_MmzFree(pstDstData->au64PhyAddr[0],(void *) pstDstData->au64VirAddr[0]);
       }

}
/*****************************************************************************
 Prototype    : Svp_Convert.Svp_CropImg_Convet
 Description  : Crop 图像
 Input        : POINT_S  point_start  
                int  width            
                int  height           
 Output       : None
 Return Value : HI_S32
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2020/7/23
    Author       : x
    Modification : Created function

*****************************************************************************/
HI_S32  Svp_Convert:: Svp_CropImg_Convet(IVE_IMAGE_S* pstSrcData,IVE_IMAGE_S* pstDstData, POINT_S  point_start,int  width,int  height)
{
        HI_S32   s32Ret;
        HI_BOOL  bFinish;

        IVE_HANDLE      dmaHandle;
        IVE_SRC_DATA_S  stSrcData;
        IVE_DST_DATA_S  stDstData;

        HI_U64          u64AllocPhyAddr;
        HI_U64          u64AllocVirAddr;

        HI_U32 u32picSize  =  width *height;
        HI_U32 u32Size     =  u32picSize *3;

        printf("start  point:{x :%d y:%d}   {%d,%d}",point_start.s32X,point_start.s32Y,width,height);
        
        s32Ret = HI_MPI_SYS_MmzAlloc(&u64AllocPhyAddr, (void**)&u64AllocVirAddr, "CROP", HI_NULL, u32Size);
        if(s32Ret!= HI_SUCCESS)
        {
             printf("HI_MPI_SYS_MmzAlloc Svp_CropImg_Convet error\r\n");
             return   -1;
        }

        pstDstData->enType  =  IVE_IMAGE_TYPE_U8C3_PLANAR;
        pstDstData->u32Width =  width;
        pstDstData->u32Height=  height;
      
        IVE_DMA_CTRL_S  stCtrl =  {IVE_DMA_MODE_DIRECT_COPY,0};

        for(int  j  = 0; j< 3;j++)
        {
             pstDstData->au32Stride[j]  =  width;
             pstDstData->au64PhyAddr[j] =  u64AllocPhyAddr + j*u32picSize;
             pstDstData->au64VirAddr[j] =  u64AllocVirAddr + j*u32picSize;
             
             stSrcData.u32Width   =  width ;
             stSrcData.u32Height  =  height;
             stSrcData.u32Stride  =  pstSrcData->au32Stride[j];
             stSrcData.u64PhyAddr =  pstSrcData->au64PhyAddr[j]  +  point_start.s32Y *stSrcData.u32Stride + point_start.s32X;
             stSrcData.u64VirAddr =  pstSrcData->au64VirAddr[j]  +  point_start.s32Y *stSrcData.u32Stride + point_start.s32X;

             stDstData.u32Height  =  height;
             stDstData.u32Width   =  width;
             stDstData.u32Stride  =  stDstData.u32Width ;
             stDstData.u64PhyAddr =  u64AllocPhyAddr + j*u32picSize;
             stDstData.u64VirAddr =  u64AllocVirAddr + j*u32picSize;
             

             s32Ret = HI_MPI_IVE_DMA(&dmaHandle,&stSrcData,&stDstData,&stCtrl,HI_FALSE);
             if(s32Ret!= HI_SUCCESS)
             {
                 printf("HI_MPI_IVE_DMA error %x\r\n",s32Ret);
                 return   -1;
             }

             s32Ret = HI_MPI_IVE_Query(dmaHandle,&bFinish,HI_FALSE); 
             if(s32Ret!= HI_SUCCESS)
             {
                printf("HI_MPI_IVE_Query error\r\n");
             }

        }

}

/*****************************************************************************
 Prototype    : Svp_Convert.Svp_ResizeImg_Free
 Description  : 释放resize  缓存
 Input        : IVE_IMAGE_S* pstDstData  
 Output       : None
 Return Value : HI_S32
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2020/7/27
    Author       : x
    Modification : Created function

*****************************************************************************/
HI_S32  Svp_Convert:: Svp_ResizeImg_Free(IVE_IMAGE_S* pstDstData)
{

       if (pstDstData->au64VirAddr[0] != 0)
       {
            HI_MPI_SYS_MmzFree(pstDstData->au64PhyAddr[0], (void*)pstDstData->au64VirAddr[0]);
       }

       return  0;

}
/*****************************************************************************
 Prototype    : Svp_Convert.Svp_ResizeImg_Convert
 Description  : resize 内存
 Input        : None
 Output       : None
 Return Value : HI_S32
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2020/7/23
    Author       : x
    Modification : Created function

*****************************************************************************/
HI_S32  Svp_Convert:: Svp_ResizeImg_Convert(IVE_IMAGE_S* pstSrcData,IVE_IMAGE_S* pstDstData,int  width,int  height)
{
       
         HI_S32      s32Ret;
         HI_U32      u32Size;
         HI_U32      u32Pic;
         IVE_HANDLE  IveHandle;
         HI_U64           u64AllocPhyAddr;
         HI_U64           u64AllocVirAddr;
         IVE_RESIZE_CTRL_S     stResizeCtrl;
         
         stResizeCtrl.u16Num = 1;
         stResizeCtrl.enMode = IVE_RESIZE_MODE_LINEAR;
         stResizeCtrl.stMem.u32Size =  49*stResizeCtrl.u16Num;

             
         s32Ret  = HI_MPI_SYS_MmzAlloc(&stResizeCtrl.stMem.u64PhyAddr, (void**)&stResizeCtrl.stMem.u64VirAddr,
                                   "resize1", HI_NULL, 49*stResizeCtrl.u16Num);
         u32Pic  = width * height;
         u32Size = u32Pic*3;
         
         s32Ret  = HI_MPI_SYS_MmzAlloc(&u64AllocPhyAddr, (void**)&u64AllocVirAddr,"resize", HI_NULL, u32Size);
         if(s32Ret!=  HI_SUCCESS)
         {
             printf("Svp_ResizeImg_Convert resize  malloc  error\r\n");
             return   -1;
             
         }

         for(HI_U32   j  =0 ; j <  3;j++)
         {
             pstDstData->u32Width     = width;
             pstDstData->u32Height    = height;
             pstDstData->au32Stride[j]= width;
             pstDstData->au64PhyAddr[j]  = u64AllocPhyAddr  +  (HI_U64)(j*u32Pic);
             pstDstData->au64VirAddr[j]  = u64AllocVirAddr  +  (HI_U64)(j*u32Pic);
         }
         
         pstDstData->enType   = IVE_IMAGE_TYPE_U8C3_PLANAR;
         
         s32Ret = HI_MPI_IVE_Resize(&IveHandle,pstSrcData, pstDstData, &stResizeCtrl, HI_FALSE);
         if(s32Ret!= HI_SUCCESS)
         {
             printf("Svp_ResizeImg_Convert  HI_MPI_IVE_Resize error %x\r\n",s32Ret);
             return   -1;
         }   

         HI_MPI_SYS_MmzFree(stResizeCtrl.stMem.u64PhyAddr, (void*)stResizeCtrl.stMem.u64VirAddr);

         
         return   0;
         
}
/*****************************************************************************
 Prototype    : Svp_Convert.Svp_BgrImg_Dump
 Description  : 存储临时bgr 文件
 Input        : IVE_IMAGE_S* pstSrcData  
 Output       : None
 Return Value : HI_S32
 Calls        : 
 Called By    : 
 
  History        :
  1.Date         : 2020/7/29
    Author       : x
    Modification : Created function

*****************************************************************************/
HI_S32  Svp_Convert ::  Svp_BgrImg_Dump(IVE_IMAGE_S* pstSrcData,char *path)
{
    char file[128] = {0};

    snprintf(file, 128, "%s_%d_%d.bgr",path,pstSrcData->u32Width,pstSrcData->u32Height);

    FILE *pFp  = fopen(file,"wb");
        
    const HI_U16 width  = pstSrcData->u32Width;
    const HI_U16 height = pstSrcData->u32Height * 3;
    
    HI_U8* pU8 = ( HI_U8*)pstSrcData->au64VirAddr[0];
    
    for (HI_U16 idxH = 0; idxH < height;  idxH++)
    {
       fwrite(pU8, width,  1,  pFp);
       
       pU8 +=  pstSrcData->u32Width;
    }

    fflush(pFp);

    fclose(pFp);

    return   0;


}

 HI_S32   Svp_Convert :: Svp_Bgr2Mat(cv::Mat& img, IVE_IMAGE_S* pstSrcData)
 {
	int width ,height,channel;
	unsigned char *img_data = NULL;
	unsigned char *img_data_conv = NULL;
	cv::Mat image;
	width = pstSrcData->u32Width;
	height = pstSrcData->u32Height;
	channel = 3;
	//img_data = (unsigned char*)malloc(sizeof(unsigned char) * width * height * channel);
	img_data = ( HI_U8*)pstSrcData->au64VirAddr[0];
	img_data_conv = (unsigned char*)malloc(sizeof(unsigned char) * width * height * channel);

	for (int k = 0; k < channel; k++)
		for (int i = 0; i < height; i++)
			for (int j = 0; j < width; j++)
				img_data_conv[channel * (i * width + j) + k] = img_data[k * height * width + i * width + j];
	image = cv::Mat(height, width, CV_8UC3, img_data_conv);
	img = image.clone();
	
	free(img_data_conv);
	img_data_conv = NULL;
	return HI_SUCCESS;
 }

