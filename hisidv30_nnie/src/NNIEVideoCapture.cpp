#include "NNIEVideoCaputre.h"


NNIEVideoCapture::NNIEVideoCapture(dictionary *config)
{
	int ret = -1;
	VideoSourceInfo_.type =   (VIDEO_SOURCE_TYPE_E)iniparser_getint(config, "VideoSource:type",0);
	VideoSourceInfo_.imagesize[SENSOR_0].u32Width  = iniparser_getint(config, "VideoSource:sensor0_imageW", 1280);
	VideoSourceInfo_.imagesize[SENSOR_0].u32Height = iniparser_getint(config, "VideoSource:sensor0_imageH",   720);
	VideoSourceInfo_.imagesize[SENSOR_1].u32Width  = iniparser_getint(config, "VideoSource:sensor1_imageW", 1280);
	VideoSourceInfo_.imagesize[SENSOR_1].u32Height = iniparser_getint(config, "VideoSource:sensor1_imageH",   720);
	if(openflag_ == false)
	{
		ret = video_source_.Init( &VideoSourceInfo_);
		if(HI_SUCCESS == ret)
		{
			openflag_ = true;
			
		}
		else
		{
			openflag_ = false;
			printf("open cam faied\n");
		}
	}
	else
	{
		printf("open cam is already open\n");
	}

	
}
NNIEVideoCapture::~NNIEVideoCapture()
{
	video_source_.Deinit();	
	openflag_ = false;
}
bool NNIEVideoCapture::isOpened()
{
	return openflag_ ;

}

bool NNIEVideoCapture::read(SENSOR_NUM_E num,cv::Mat& frame)
{
	int ret = -1;
	VIDEO_FRAME_INFO_S  *pstVideoFrameBase = &VideoFrameInfo_[num][BASE_STREAM];
	VIDEO_FRAME_INFO_S  *pstVideoFrameExt = &VideoFrameInfo_[num][EXT_STREAM];
	IVE_IMAGE_S        pImg;
	if(openflag_ == false)
	{
		printf("cam is not open!\n");
		return false;
	}
	if(num >= SENSOR_NUM )
	{
		printf("the param is error!\n");
		return false;
	}
	ret = video_source_.GetFrame( num,  EXT_STREAM, pstVideoFrameExt);
	if(HI_SUCCESS != ret )
	{
		printf("get ext frame faied\n");
		return false;
	}

	ret = video_source_.GetFrame( num,  BASE_STREAM, pstVideoFrameBase);
	if(HI_SUCCESS != ret )
	{
		printf("get base frame faied\n");
		return false;
	}
	ret = svp_convert_.Svp_CscImg_Convert( pstVideoFrameExt, &pImg, IVE_CSC_MODE_PIC_BT601_YUV2RGB,IVE_IMAGE_TYPE_U8C3_PLANAR);
	if(HI_SUCCESS != ret )
	{
		printf("yuv convert to bgr faied\n");
		goto failed;
	}
	ret = svp_convert_.Svp_Bgr2Mat( frame, &pImg);
	if(HI_SUCCESS != ret )
	{
		printf("bgr convert to mat faied\n");
		goto failed;
	}
	svp_convert_.Svp_CscImg_Free(&pImg);
	video_source_.ReleaseFrame(num,  EXT_STREAM, pstVideoFrameExt);
	return true;
	
failed:
	svp_convert_.Svp_CscImg_Free(&pImg);
	video_source_.ReleaseFrame(num,  EXT_STREAM, pstVideoFrameExt);
	return false;
	
}


/*modify*/
bool NNIEVideoCapture::read(SENSOR_NUM_E num,STREAM_NUM_E CHAN_EN,VIDEO_FRAME_INFO_S *pstFrame)
{
    int ret = -1;

	if(openflag_ == false)
	{
		printf("cam is not open!\n");
		return false;
	}
	if(num >= SENSOR_NUM )
	{
		printf("the param is error!\n");
		return false;
	}
	ret = video_source_.GetFrame( num,  CHAN_EN, pstFrame);
	if(HI_SUCCESS != ret )
	{
		printf("get ext frame faied\n");
		return false;
	}


}


bool NNIEVideoCapture::display(SENSOR_NUM_E num,cv::Rect& box,cv::Point aline_point[])
{
	int ret = -1;
	float  widthexpend  ; 
       float  heightexpend ;
	int baseimageW ;
	int baseimageH ;
	st_SVP_NNIE_RECT_ARRAY_S stRect;
	VGS_DRAW_LINE_S  stVgsDrawLine[98] ;
	VIDEO_FRAME_INFO_S  *pstVideoFrameBase = &VideoFrameInfo_[num][BASE_STREAM];
	baseimageW = pstVideoFrameBase->stVFrame.u32Width;
	baseimageH = pstVideoFrameBase->stVFrame.u32Height;
	
	widthexpend = (float)baseimageW / (float)VideoSourceInfo_.imagesize[num].u32Width;
	heightexpend = (float)baseimageH / (float)VideoSourceInfo_.imagesize[num].u32Height;
	if(!box.empty())
	{
	
	       float  point_x1  =  box.x  * widthexpend;
      		float  point_y1  =  box.y  * heightexpend;
      		float  point_x2  =  point_x1 + box.width * widthexpend;
		float  point_y2  =  point_y1;
		float  point_x3  =  point_x1;
		float  point_y3  =  point_y1 + box.height* heightexpend;
		float  point_x4  =  point_x1 + box.width * widthexpend;
		float  point_y4  =  point_y1 + box.height* heightexpend;
		stRect.astRect[0].astPoint[0].s32X = ALIGN2((int)point_x1);
		stRect.astRect[0].astPoint[0].s32Y = ALIGN2((int)point_y1);
		stRect.astRect[0].astPoint[1].s32X = ALIGN2((int)point_x2);
		stRect.astRect[0].astPoint[1].s32Y = ALIGN2((int)point_y2);
		stRect.astRect[0].astPoint[2].s32X = ALIGN2((int)point_x3);
		stRect.astRect[0].astPoint[2].s32Y = ALIGN2((int)point_y3);
		stRect.astRect[0].astPoint[3].s32X = ALIGN2((int)point_x4);
		stRect.astRect[0].astPoint[3].s32Y = ALIGN2((int)point_y4);
		stRect.u32TotalNum = 1;
		ret = svp_convert_.Svp_CoverRect(pstVideoFrameBase, &stRect, 0x0000FF00);
		if(HI_SUCCESS != ret )
		{
			printf("rect cover failed\n");
			//return false;
		}
		for(int  i = 0 ; i<  98; i++)
		{
			stVgsDrawLine[i].stStartPoint.s32X =ALIGN2((int)(aline_point[i].x * widthexpend));
			stVgsDrawLine[i].stStartPoint.s32Y = ALIGN2((int)(aline_point[i].y * heightexpend)) ;

			stVgsDrawLine[i].stEndPoint.s32X = stVgsDrawLine[i].stStartPoint.s32X + 6;
			stVgsDrawLine[i].stEndPoint.s32Y = stVgsDrawLine[i].stStartPoint.s32Y ;

			stVgsDrawLine[i].u32Thick=6;
			stVgsDrawLine[i].u32Color=0x0000FF00;
		
		}

		ret = svp_convert_.Svp_CoverPoint(pstVideoFrameBase, stVgsDrawLine, 98);
		if(HI_SUCCESS != ret )
		{
			printf("point line cover failed\n");
			//return false;
		}
	}

	ret = video_source_.SendToDisplay(num,pstVideoFrameBase);
	if(HI_SUCCESS != ret )
	{
		printf("SendToDisplay failed\n");
	}

	video_source_.ReleaseFrame(num,  BASE_STREAM, pstVideoFrameBase);
	if(HI_SUCCESS != ret )
	{
		printf("ReleaseFrame failed\n");
		return false;
	}
	return true;

}

void NNIEVideoCapture::release()
{
	video_source_.Deinit();
	openflag_ = false;
}


