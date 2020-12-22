#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#include "mpp_video.h"
#include "sample_comm_ive.h"

SAMPLE_VI_CONFIG_S     s_stViConfig   = {0};
SAMPLE_IVE_SWITCH_S    s_stSwitch = {HI_FALSE,HI_TRUE};

SAMPLE_VO_CONFIG_S stVoConfig;
VI_DEV             ViDev[2]          = {0, 1};
VI_PIPE            ViPipe[4]         = {0, 1, 2, 3};
VI_CHN             ViChn          = 0;
VPSS_GRP           VpssGrp[2]        = {0, 1};
HI_BOOL            abChnEnable[VPSS_MAX_PHY_CHN_NUM] = {1, 1};
SAMPLE_VI_CONFIG_S stViConfig;

HI_S32 Mpp_Video_Start_Single_ViVpssVo(SIZE_S extsize)
{
    SIZE_S astSize[VPSS_CHN_NUM];
    PIC_SIZE_E aenSize[VPSS_CHN_NUM];
    VI_CHN_ATTR_S stViChnAttr;
    SAMPLE_RC_E enRcMode = SAMPLE_RC_CBR;
    PAYLOAD_TYPE_E enStreamType = PT_H264;
    HI_BOOL bRcnRefShareBuf=HI_FALSE;
    VENC_GOP_ATTR_S stGopAttr;
    VI_DEV ViDev0 = 0;
    VI_PIPE ViPipe0 = 0;
    VI_CHN ViChn = 0;
    HI_S32 s32ViCnt = 1;
    HI_S32 s32WorkSnsId  = 0;
    VPSS_GRP VpssGrp = 0;
    HI_S32 s32Ret = HI_SUCCESS;
    VENC_CHN VeH264Chn = 0;
    WDR_MODE_E enWDRMode = WDR_MODE_NONE;
    DYNAMIC_RANGE_E enDynamicRange = DYNAMIC_RANGE_SDR8;
    PIXEL_FORMAT_E enPixFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    VIDEO_FORMAT_E enVideoFormat = VIDEO_FORMAT_LINEAR;
    COMPRESS_MODE_E enCompressMode = COMPRESS_MODE_NONE;
    VI_VPSS_MODE_E enMastPipeMode = VI_ONLINE_VPSS_OFFLINE;

    memset(&s_stViConfig,0,sizeof(s_stViConfig));

    SAMPLE_COMM_VI_GetSensorInfo(&s_stViConfig);
    s_stViConfig.s32WorkingViNum                           = s32ViCnt;

    s_stViConfig.as32WorkingViId[0]                        = 0;
    s_stViConfig.astViInfo[0].stSnsInfo.MipiDev            = SAMPLE_COMM_VI_GetComboDevBySensor(s_stViConfig.astViInfo[0].stSnsInfo.enSnsType, 0);
    s_stViConfig.astViInfo[0].stSnsInfo.s32BusId           = 0;

    s_stViConfig.astViInfo[0].stDevInfo.ViDev              = ViDev0;
    s_stViConfig.astViInfo[0].stDevInfo.enWDRMode          = enWDRMode;

    s_stViConfig.astViInfo[0].stPipeInfo.enMastPipeMode    = enMastPipeMode;
    s_stViConfig.astViInfo[0].stPipeInfo.aPipe[0]          = ViPipe0;
    s_stViConfig.astViInfo[0].stPipeInfo.aPipe[1]          = -1;
   s_stViConfig.astViInfo[0].stPipeInfo.aPipe[2]          = -1;
   s_stViConfig.astViInfo[0].stPipeInfo.aPipe[3]          = -1;

    s_stViConfig.astViInfo[0].stChnInfo.ViChn              = ViChn;
    s_stViConfig.astViInfo[0].stChnInfo.enPixFormat        = enPixFormat;
    s_stViConfig.astViInfo[0].stChnInfo.enDynamicRange     = enDynamicRange;
    s_stViConfig.astViInfo[0].stChnInfo.enVideoFormat      = enVideoFormat;
    s_stViConfig.astViInfo[0].stChnInfo.enCompressMode     = enCompressMode;

    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(s_stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.enSnsType, &aenSize[0]);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_0,
        "Error(%#x),SAMPLE_COMM_VI_GetSizeBySensor failed!\n",s32Ret);
    aenSize[1] = aenSize[0];

    /******************************************
     step  1: Init vb
    ******************************************/
    s32Ret = SAMPLE_COMM_IVE_VbInit(aenSize,astSize,VPSS_CHN_NUM);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_0,
        "Error(%#x),SAMPLE_COMM_IVE_VbInit failed!\n",s32Ret);
    /******************************************
     step 2: Start vi
    ******************************************/
    s32Ret = SAMPLE_COMM_VI_SetParam(&s_stViConfig);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_1,
        "Error(%#x),SAMPLE_COMM_VI_SetParam failed!\n",s32Ret);

    s32Ret = SAMPLE_COMM_VI_StartVi(&s_stViConfig);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_1,
        "Error(%#x),SAMPLE_COMM_VI_StartVi failed!\n",s32Ret);
    /******************************************
     step 3: Start vpss
    ******************************************/
    astSize[1].u32Height = extsize.u32Height;
    astSize[1].u32Width = extsize.u32Width;
    s32Ret = SAMPLE_COMM_IVE_StartVpss(astSize,VPSS_CHN_NUM);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_2,
        "Error(%#x),SAMPLE_IVS_StartVpss failed!\n",s32Ret);
    /******************************************
      step 4: Bind vpss to vi
     ******************************************/
    s32Ret = SAMPLE_COMM_VI_Bind_VPSS(ViPipe0, ViChn, VpssGrp);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_3,
        "Error(%#x),SAMPLE_COMM_VI_BindVpss failed!\n",s32Ret);
    //Set vi frame
    s32Ret = HI_MPI_VI_GetChnAttr(ViPipe0, ViChn,&stViChnAttr);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_4,
        "Error(%#x),HI_MPI_VI_GetChnAttr failed!\n",s32Ret);

    s32Ret = HI_MPI_VI_SetChnAttr(ViPipe0, ViChn,&stViChnAttr);
    SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_4,
        "Error(%#x),HI_MPI_VI_SetChnAttr failed!\n",s32Ret);
    /******************************************
     step 5: Start Vo
     ******************************************/
    if (HI_TRUE == s_stSwitch.bVo)
    {
        s32Ret = SAMPLE_COMM_IVE_StartVo();
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_4,
            "Error(%#x),SAMPLE_COMM_IVE_StartVo failed!\n", s32Ret);
    }
#if 0
     //step 6: Bind vo to vpss
     /******************************************/
     SAMPLE_COMM_VPSS_Bind_VO(VpssGrp,  0, 0,  0);
    /******************************************/
#endif
    
    /******************************************
     step 7: Start Venc
    ******************************************/
    if (HI_TRUE == s_stSwitch.bVenc)
    {
        s32Ret = SAMPLE_COMM_VENC_GetGopAttr(VENC_GOPMODE_NORMALP,&stGopAttr);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_5,
            "Error(%#x),SAMPLE_COMM_VENC_GetGopAttr failed!\n",s32Ret);
        s32Ret = SAMPLE_COMM_VENC_Start(VeH264Chn, enStreamType,aenSize[0],enRcMode,0,bRcnRefShareBuf,&stGopAttr);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_5,
            "Error(%#x),SAMPLE_COMM_VENC_Start failed!\n",s32Ret);
        s32Ret = SAMPLE_COMM_VENC_StartGetStream(&VeH264Chn, 1);
        SAMPLE_CHECK_EXPR_GOTO(HI_SUCCESS != s32Ret, END_INIT_6,
            "Error(%#x),SAMPLE_COMM_VENC_StartGetStream failed!\n",s32Ret);
    }

    return HI_SUCCESS;

END_INIT_6:
    if (HI_TRUE == s_stSwitch.bVenc)
    {
        SAMPLE_COMM_VENC_Stop(VeH264Chn);
    }
END_INIT_5:
    if (HI_TRUE == s_stSwitch.bVo)
    {
        SAMPLE_COMM_IVE_StopVo();
    }
END_INIT_4:
    SAMPLE_COMM_VI_UnBind_VPSS(ViPipe0, ViChn, VpssGrp);
END_INIT_3:
    SAMPLE_COMM_IVE_StopVpss(VPSS_CHN_NUM);
END_INIT_2:
    SAMPLE_COMM_VI_StopVi(&s_stViConfig);
END_INIT_1:    //system exit
    SAMPLE_COMM_SYS_Exit();
    memset(&s_stViConfig,0,sizeof(s_stViConfig));
END_INIT_0:

    return s32Ret;
}

HI_VOID Mpp_Video_Stop_Single_ViVpssVo(void)
{
    if (HI_TRUE == s_stSwitch.bVenc)
    {
        SAMPLE_COMM_VENC_StopGetStream();
        SAMPLE_COMM_VENC_Stop(0);
    }
    if (HI_TRUE == s_stSwitch.bVo)
    {
        SAMPLE_COMM_IVE_StopVo();
    }

    SAMPLE_COMM_VI_UnBind_VPSS(s_stViConfig.astViInfo[0].stPipeInfo.aPipe[0],s_stViConfig.astViInfo[0].stChnInfo.ViChn, 0);
    SAMPLE_COMM_IVE_StopVpss(VPSS_CHN_NUM);
    SAMPLE_COMM_VI_StopVi(&s_stViConfig);
    SAMPLE_COMM_SYS_Exit();
    memset(&s_stViConfig,0,sizeof(s_stViConfig));
}

HI_S32 Mpp_Video_Start_Double_ViVpssVo(SIZE_S extsize0,SIZE_S extsize1)
{
    HI_S32             s32Ret = HI_SUCCESS;

    HI_S32             s32ViCnt       = 2;
    HI_S32             s32WorkSnsId   = 0;


    SIZE_S             stSize;
    VB_CONFIG_S        stVbConf;
    PIC_SIZE_E         enPicSize;
    HI_U32             u32BlkSize;

    VO_CHN             VoChn          = 0;
 

    WDR_MODE_E         enWDRMode      = WDR_MODE_NONE;
    DYNAMIC_RANGE_E    enDynamicRange = DYNAMIC_RANGE_SDR8;
    PIXEL_FORMAT_E     enPixFormat    = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    VIDEO_FORMAT_E     enVideoFormat  = VIDEO_FORMAT_LINEAR;
    COMPRESS_MODE_E    enCompressMode = COMPRESS_MODE_NONE;
    VI_VPSS_MODE_E     enMastPipeMode = VI_OFFLINE_VPSS_OFFLINE;//VI_ONLINE_VPSS_OFFLINE;

    VPSS_GRP_ATTR_S    stVpssGrpAttr;
    VPSS_CHN           VpssChn        = VPSS_CHN0;

    VPSS_CHN_ATTR_S    astVpssChnAttr[VPSS_MAX_PHY_CHN_NUM];

    /*config vi*/
    SAMPLE_COMM_VI_GetSensorInfo(&stViConfig);
    SAMPLE_PRT("-----------sensor type: sensor0 = %d,  sensor1= %d\n",stViConfig.astViInfo[0].stSnsInfo.enSnsType,stViConfig.astViInfo[1].stSnsInfo.enSnsType);
    stViConfig.s32WorkingViNum                                   = s32ViCnt;
    stViConfig.as32WorkingViId[0]                                = 0;
    stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.MipiDev         = ViDev[0];
    stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.s32BusId        = 0;
    stViConfig.astViInfo[s32WorkSnsId].stDevInfo.ViDev           = ViDev[0];
    stViConfig.astViInfo[s32WorkSnsId].stDevInfo.enWDRMode       = enWDRMode;
    stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.enMastPipeMode = enMastPipeMode;
    stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[0]       = ViPipe[0];
    stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[1]       = -1;
    stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[2]       = -1;
    stViConfig.astViInfo[s32WorkSnsId].stPipeInfo.aPipe[3]       = -1;
    stViConfig.astViInfo[s32WorkSnsId].stChnInfo.ViChn           = ViChn;
    stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enPixFormat     = enPixFormat;
    stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enDynamicRange  = enDynamicRange;
    stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enVideoFormat   = enVideoFormat;
    stViConfig.astViInfo[s32WorkSnsId].stChnInfo.enCompressMode  = enCompressMode;

    stViConfig.as32WorkingViId[1]                                = 1;
    stViConfig.astViInfo[1].stSnsInfo.MipiDev         = ViDev[1];
    stViConfig.astViInfo[1].stSnsInfo.s32BusId        = 1;
    stViConfig.astViInfo[1].stDevInfo.ViDev           = ViDev[1];
    stViConfig.astViInfo[1].stDevInfo.enWDRMode       = enWDRMode;
    stViConfig.astViInfo[1].stPipeInfo.enMastPipeMode = enMastPipeMode;
    stViConfig.astViInfo[1].stPipeInfo.aPipe[0]       = ViPipe[2];
    stViConfig.astViInfo[1].stPipeInfo.aPipe[1]       = -1;
    stViConfig.astViInfo[1].stPipeInfo.aPipe[2]       = -1;
    stViConfig.astViInfo[1].stPipeInfo.aPipe[3]       = -1;
    stViConfig.astViInfo[1].stChnInfo.ViChn           = ViChn;
    stViConfig.astViInfo[1].stChnInfo.enPixFormat     = enPixFormat;
    stViConfig.astViInfo[1].stChnInfo.enDynamicRange  = enDynamicRange;
    stViConfig.astViInfo[1].stChnInfo.enVideoFormat   = enVideoFormat;
    stViConfig.astViInfo[1].stChnInfo.enCompressMode  = enCompressMode;

    /*get picture size*/
    s32Ret = SAMPLE_COMM_VI_GetSizeBySensor(stViConfig.astViInfo[s32WorkSnsId].stSnsInfo.enSnsType, &enPicSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get picture size by sensor failed!\n");
        return s32Ret;
    }

    s32Ret = SAMPLE_COMM_SYS_GetPicSize(enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get picture size failed!\n");
        return s32Ret;
    }

    /*config vb*/
    hi_memset(&stVbConf, sizeof(VB_CONFIG_S), 0, sizeof(VB_CONFIG_S));
    stVbConf.u32MaxPoolCnt              = 2;

    u32BlkSize = COMMON_GetPicBufferSize(stSize.u32Width, stSize.u32Height, SAMPLE_PIXEL_FORMAT, DATA_BITWIDTH_8, COMPRESS_MODE_SEG, DEFAULT_ALIGN);
    stVbConf.astCommPool[0].u64BlkSize  = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt   = 20;

    u32BlkSize = VI_GetRawBufferSize(stSize.u32Width, stSize.u32Height, PIXEL_FORMAT_RGB_BAYER_16BPP, COMPRESS_MODE_NONE, DEFAULT_ALIGN);
    stVbConf.astCommPool[1].u64BlkSize  = u32BlkSize;
    stVbConf.astCommPool[1].u32BlkCnt   = 4;

    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("system init failed with %d!\n", s32Ret);
        return s32Ret;
    }

    /*start vi*/
    s32Ret = SAMPLE_COMM_VI_StartVi(&stViConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vi failed.s32Ret:0x%x !\n", s32Ret);
        goto EXIT;
    }

    /*config vpss*/
    hi_memset(&stVpssGrpAttr, sizeof(VPSS_GRP_ATTR_S), 0, sizeof(VPSS_GRP_ATTR_S));
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate    = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate    = -1;
    stVpssGrpAttr.enDynamicRange                 = DYNAMIC_RANGE_SDR8;
    stVpssGrpAttr.enPixelFormat                  = enPixFormat;
    stVpssGrpAttr.u32MaxW                        = stSize.u32Width;
    stVpssGrpAttr.u32MaxH                        = stSize.u32Height;
    stVpssGrpAttr.bNrEn                          = HI_TRUE;
    stVpssGrpAttr.stNrAttr.enCompressMode        = COMPRESS_MODE_FRAME;
    stVpssGrpAttr.stNrAttr.enNrMotionMode        = NR_MOTION_MODE_NORMAL;

    astVpssChnAttr[0].u32Width                    = stSize.u32Width;
    astVpssChnAttr[0].u32Height                   = stSize.u32Height;
    astVpssChnAttr[0].enChnMode                   = VPSS_CHN_MODE_USER;
    astVpssChnAttr[0].enCompressMode              = enCompressMode;
    astVpssChnAttr[0].enDynamicRange              = enDynamicRange;
    astVpssChnAttr[0].enVideoFormat               = enVideoFormat;
    astVpssChnAttr[0].enPixelFormat               = enPixFormat;
    astVpssChnAttr[0].stFrameRate.s32SrcFrameRate = 30;
    astVpssChnAttr[0].stFrameRate.s32DstFrameRate = 30;
    astVpssChnAttr[0].u32Depth                    = 1;
    astVpssChnAttr[0].bMirror                     = HI_FALSE;
    astVpssChnAttr[0].bFlip                       = HI_FALSE;
    astVpssChnAttr[0].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
	
    astVpssChnAttr[1].u32Width                    = extsize0.u32Width;
    astVpssChnAttr[1].u32Height                   = extsize0.u32Height;
    astVpssChnAttr[1].enChnMode                   = VPSS_CHN_MODE_USER;
    astVpssChnAttr[1].enCompressMode              = enCompressMode;
    astVpssChnAttr[1].enDynamicRange              = enDynamicRange;
    astVpssChnAttr[1].enVideoFormat               = enVideoFormat;
    astVpssChnAttr[1].enPixelFormat               = enPixFormat;
    astVpssChnAttr[1].stFrameRate.s32SrcFrameRate = 30;
    astVpssChnAttr[1].stFrameRate.s32DstFrameRate = 30;
    astVpssChnAttr[1].u32Depth                    = 1;
    astVpssChnAttr[1].bMirror                     = HI_FALSE;
    astVpssChnAttr[1].bFlip                       = HI_FALSE;
    astVpssChnAttr[1].stAspectRatio.enMode        = ASPECT_RATIO_NONE;


    /*start vpss*/
    abChnEnable[0] = HI_TRUE;
    abChnEnable[1] = HI_TRUE;
    s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp[0], abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
        goto EXIT1;
    }

    astVpssChnAttr[1].u32Width                    = extsize1.u32Width;
    astVpssChnAttr[1].u32Height                   = extsize1.u32Height;
// start vpssgrp 1
    s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp[1], abChnEnable, &stVpssGrpAttr, astVpssChnAttr);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vpss group failed. s32Ret: 0x%x !\n", s32Ret);
        goto EXIT1;
    }

    /*vpss bind vo*/
    s32Ret = SAMPLE_COMM_VI_Bind_VPSS(ViPipe[0], ViChn, VpssGrp[0]);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("vpss bind vi failed. s32Ret: 0x%x !\n", s32Ret);
        goto EXIT2;
    }

    s32Ret = SAMPLE_COMM_VI_Bind_VPSS(ViPipe[2], ViChn, VpssGrp[1]);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("vpss bind vi failed. s32Ret: 0x%x !\n", s32Ret);
        goto EXIT3;
    }

    /*config vo*/
    SAMPLE_COMM_VO_GetDefConfig(&stVoConfig);
    stVoConfig.enDstDynamicRange = enDynamicRange;

    stVoConfig.enVoIntfType = VO_INTF_HDMI;
    stVoConfig.enIntfSync = VO_OUTPUT_1080P60;//VO_OUTPUT_1080x1920_60;

    stVoConfig.enPicSize = enPicSize;
//	stVoConfig.enVoPartMode      = 1;
	stVoConfig.enVoMode		 =VO_MODE_2MUX;

    /*start vo*/
    s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConfig);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("start vo failed. s32Ret: 0x%x !\n", s32Ret);
        goto EXIT4;
    }
#if 0
    /*vpss bind vo*/
    s32Ret = SAMPLE_COMM_VPSS_Bind_VO(VpssGrp[0], VpssChn, stVoConfig.VoDev, VoChn);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("vo bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
        goto EXIT5;
    }

    s32Ret = SAMPLE_COMM_VPSS_Bind_VO(VpssGrp[1], 1, stVoConfig.VoDev, 1);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("vo bind vpss failed. s32Ret: 0x%x !\n", s32Ret);
        goto EXIT6;
    }
#endif


return HI_SUCCESS;

EXIT6:
    SAMPLE_COMM_VO_StopVO(&stVoConfig);
EXIT5:
    SAMPLE_COMM_VI_UnBind_VPSS(ViPipe[2], ViChn, VpssGrp[1]);
EXIT4:
    SAMPLE_COMM_VI_UnBind_VPSS(ViPipe[0], ViChn, VpssGrp[0]);
EXIT3:
    SAMPLE_COMM_VPSS_Stop(VpssGrp[1], abChnEnable);
EXIT2:
    SAMPLE_COMM_VPSS_Stop(VpssGrp[0], abChnEnable);
EXIT1:
    SAMPLE_COMM_VI_StopVi(&stViConfig);
EXIT:
    SAMPLE_COMM_SYS_Exit();
    return s32Ret;
}
HI_VOID Mpp_Video_Stop_Double_ViVpssVo(void)
{
    SAMPLE_COMM_VO_StopVO(&stVoConfig);
    SAMPLE_COMM_VI_UnBind_VPSS(ViPipe[2], ViChn, VpssGrp[1]);
    SAMPLE_COMM_VI_UnBind_VPSS(ViPipe[0], ViChn, VpssGrp[0]);
    SAMPLE_COMM_VPSS_Stop(VpssGrp[1], abChnEnable);
    SAMPLE_COMM_VPSS_Stop(VpssGrp[0], abChnEnable);
    SAMPLE_COMM_VI_StopVi(&stViConfig);
    SAMPLE_COMM_SYS_Exit();
}

#define VIDEO_FILE_NAME  "facedetect720P.avi"

PAYLOAD_TYPE_E enType = PT_MJPEG;
PIC_SIZE_E picsize    = PIC_720P;

VDEC_THREAD_PARAM_S stVdecSend[VDEC_MAX_CHN_NUM];
pthread_t   VdecThread[2*VDEC_MAX_CHN_NUM];
SAMPLE_VO_CONFIG_S stVoConfig;


HI_S32 Mpp_Video_Start_File_Vdec_Vpss(SIZE_S extsize)
{
    VB_CONFIG_S stVbConfig;
    HI_S32 i, s32Ret = HI_SUCCESS;
    SIZE_S stDispSize;
    VO_LAYER VoLayer;
    HI_U32 u32VdecChnNum, VpssGrpNum;
    VPSS_GRP VpssGrp;
    PIC_SIZE_E enDispPicSize;
    SAMPLE_VDEC_ATTR astSampleVdec[VDEC_MAX_CHN_NUM];
    VPSS_CHN_ATTR_S astVpssChnAttr[VPSS_MAX_CHN_NUM];
 
    VPSS_GRP_ATTR_S stVpssGrpAttr;

    VO_INTF_SYNC_E enIntfSync;
    VO_INTF_TYPE_E enVoIntfType;

    u32VdecChnNum = 1;
    VpssGrpNum    = u32VdecChnNum;
    /************************************************
    step1:  init SYS, init common VB(for VPSS and VO)
    *************************************************/

    enDispPicSize = picsize;
    enIntfSync    = VO_OUTPUT_720P60;
    enVoIntfType  = VO_INTF_HDMI;

    s32Ret =  SAMPLE_COMM_SYS_GetPicSize(enDispPicSize, &stDispSize);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("sys get pic size fail for %#x!\n", s32Ret);
        goto END1;
    }

    memset(&stVbConfig, 0, sizeof(VB_CONFIG_S));
    stVbConfig.u32MaxPoolCnt             = 1;
    stVbConfig.astCommPool[0].u32BlkCnt  = 10*u32VdecChnNum;
    stVbConfig.astCommPool[0].u64BlkSize = COMMON_GetPicBufferSize(stDispSize.u32Width, stDispSize.u32Height,
                                                PIXEL_FORMAT_YVU_SEMIPLANAR_420, DATA_BITWIDTH_8, COMPRESS_MODE_SEG, 0);
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConfig);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("init sys fail for %#x!\n", s32Ret);
        goto END1;
    }

    /************************************************
    step2:  init module VB or user VB(for VDEC)
    *************************************************/
    if(enType == PT_JPEG || enType == PT_MJPEG  )  
    {
	 for(i=0; i<u32VdecChnNum; i++)
       {
           astSampleVdec[i].enType                            = PT_JPEG;
           astSampleVdec[i].u32Width                          = stDispSize.u32Width;
           astSampleVdec[i].u32Height                         = stDispSize.u32Height;
           astSampleVdec[i].enMode                            = VIDEO_MODE_FRAME;
           astSampleVdec[i].stSapmleVdecPicture.enPixelFormat = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
           astSampleVdec[i].stSapmleVdecPicture.u32Alpha      = 255;
           astSampleVdec[i].u32DisplayFrameNum                = 2;
           astSampleVdec[i].u32FrameBufCnt                    = astSampleVdec[i].u32DisplayFrameNum + 1;
       }
    }
    else
   {
	for(i=0; i<u32VdecChnNum; i++)
	{
	      astSampleVdec[i].enType                           = enType;
	      astSampleVdec[i].u32Width                         = stDispSize.u32Width;
	      astSampleVdec[i].u32Height                        = stDispSize.u32Height;
	      astSampleVdec[i].enMode                           = VIDEO_MODE_FRAME;
	      astSampleVdec[i].stSapmleVdecVideo.enDecMode      = VIDEO_DEC_MODE_IP;
	      astSampleVdec[i].stSapmleVdecVideo.enBitWidth     = DATA_BITWIDTH_8;
	      astSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum = 2;
	      astSampleVdec[i].u32DisplayFrameNum               = 2;
	      astSampleVdec[i].u32FrameBufCnt = astSampleVdec[i].stSapmleVdecVideo.u32RefFrameNum + astSampleVdec[i].u32DisplayFrameNum + 1;
	}
    }
   
    s32Ret = SAMPLE_COMM_VDEC_InitVBPool(u32VdecChnNum, &astSampleVdec[0]);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("init mod common vb fail for %#x!\n", s32Ret);
        goto END2;
    }

    /************************************************
    step3:  start VDEC
    *************************************************/
    s32Ret = SAMPLE_COMM_VDEC_Start(u32VdecChnNum, &astSampleVdec[0]);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("start VDEC fail for %#x!\n", s32Ret);
        goto END3;
    }

    /************************************************
    step4:  start VPSS
    *************************************************/
    stVpssGrpAttr.u32MaxW = 1920;
    stVpssGrpAttr.u32MaxH = 1080;
    stVpssGrpAttr.stFrameRate.s32SrcFrameRate = -1;
    stVpssGrpAttr.stFrameRate.s32DstFrameRate = -1;
    stVpssGrpAttr.enDynamicRange = DYNAMIC_RANGE_SDR8;
    stVpssGrpAttr.enPixelFormat  = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVpssGrpAttr.bNrEn   = HI_FALSE;
    memset(abChnEnable, 0, sizeof(abChnEnable));

    abChnEnable[0] = HI_TRUE;
    astVpssChnAttr[0].u32Width                    = stDispSize.u32Width;
    astVpssChnAttr[0].u32Height                   = stDispSize.u32Height;
    astVpssChnAttr[0].enChnMode                   = VPSS_CHN_MODE_USER;
    astVpssChnAttr[0].enCompressMode              = COMPRESS_MODE_NONE;
    astVpssChnAttr[0].enDynamicRange              = DYNAMIC_RANGE_SDR8;
    astVpssChnAttr[0].enPixelFormat               = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    astVpssChnAttr[0].stFrameRate.s32SrcFrameRate = -1;
    astVpssChnAttr[0].stFrameRate.s32DstFrameRate = -1;
    astVpssChnAttr[0].u32Depth                    = 1;
    astVpssChnAttr[0].bMirror                     = HI_FALSE;
    astVpssChnAttr[0].bFlip                       = HI_FALSE;
    astVpssChnAttr[0].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    astVpssChnAttr[0].enVideoFormat               = VIDEO_FORMAT_LINEAR;
	
    abChnEnable[1] = HI_TRUE;
    astVpssChnAttr[1].u32Width                    = extsize.u32Width;
    astVpssChnAttr[1].u32Height                   = extsize.u32Height;
    astVpssChnAttr[1].enChnMode                   = VPSS_CHN_MODE_USER;
    astVpssChnAttr[1].enCompressMode              = COMPRESS_MODE_NONE;
    astVpssChnAttr[1].enDynamicRange              = DYNAMIC_RANGE_SDR8;
    astVpssChnAttr[1].enPixelFormat               = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    astVpssChnAttr[1].stFrameRate.s32SrcFrameRate = -1;
    astVpssChnAttr[1].stFrameRate.s32DstFrameRate = -1;
    astVpssChnAttr[1].u32Depth                    = 1;
    astVpssChnAttr[1].bMirror                     = HI_FALSE;
    astVpssChnAttr[1].bFlip                       = HI_FALSE;
    astVpssChnAttr[1].stAspectRatio.enMode        = ASPECT_RATIO_NONE;
    astVpssChnAttr[1].enVideoFormat               = VIDEO_FORMAT_LINEAR;
    for(i=0; i<u32VdecChnNum; i++)
    {
        VpssGrp = i;
    	 s32Ret = SAMPLE_COMM_VPSS_Start(VpssGrp, &abChnEnable[0], &stVpssGrpAttr, &astVpssChnAttr[0]);
    	 if(s32Ret != HI_SUCCESS)
   	 {
        	SAMPLE_PRT("start VPSS fail for %#x!\n", s32Ret);
        	goto END4;
        }
    }

    /************************************************
    step5:  start VO
    *************************************************/
    stVoConfig.VoDev                 = SAMPLE_VO_DEV_UHD;
    stVoConfig.enVoIntfType          = enVoIntfType;
    stVoConfig.enIntfSync            = enIntfSync;
    stVoConfig.enPicSize             =  picsize;
    stVoConfig.u32BgColor            = COLOR_RGB_BLUE;
    stVoConfig.u32DisBufLen          = 3;
    stVoConfig.enDstDynamicRange     = DYNAMIC_RANGE_SDR8;
    stVoConfig.enVoMode              = VO_MODE_1MUX;
    stVoConfig.enPixFormat           = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    stVoConfig.stDispRect.s32X       = 0;
    stVoConfig.stDispRect.s32Y       = 0;
    stVoConfig.stDispRect.u32Width   = stDispSize.u32Width;
    stVoConfig.stDispRect.u32Height  = stDispSize.u32Height;
    stVoConfig.stImageSize.u32Width  = stDispSize.u32Width;
    stVoConfig.stImageSize.u32Height = stDispSize.u32Height;
    stVoConfig.enVoPartMode          = VO_PART_MODE_SINGLE;

    s32Ret = SAMPLE_COMM_VO_StartVO(&stVoConfig);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("start VO fail for %#x!\n", s32Ret);
        goto END5;
    }

    /************************************************
    step6:  VDEC bind VPSS
    *************************************************/
    for(i=0; i<u32VdecChnNum; i++)
    {
        s32Ret = SAMPLE_COMM_VDEC_Bind_VPSS(i, i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("vdec bind vpss fail for %#x!\n", s32Ret);
            goto END6;
        }
    }

    /************************************************
    step7:  VPSS bind VO
    *************************************************/
 #if 0 
    VoLayer = stVoConfig.VoDev;
    for(i=0; i<VpssGrpNum; i++)
    {
        s32Ret = SAMPLE_COMM_VPSS_Bind_VO(i, 0, VoLayer, i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("vpss bind vo fail for %#x!\n", s32Ret);
            goto END7;
        }
    }
 #endif
    /************************************************
    step8:  send stream to VDEC
    *************************************************/
    for(i=0; i<u32VdecChnNum; i++)
    {
        snprintf(stVdecSend[i].cFileName, sizeof(stVdecSend[i].cFileName), VIDEO_FILE_NAME);
        snprintf(stVdecSend[i].cFilePath, sizeof(stVdecSend[i].cFilePath), "%s", "./");
        stVdecSend[i].enType          = astSampleVdec[i].enType;
        stVdecSend[i].s32StreamMode   = astSampleVdec[i].enMode;
        stVdecSend[i].s32ChnId        = i;
        stVdecSend[i].s32IntervalTime = 1000;
        stVdecSend[i].u64PtsInit      = 0;
        stVdecSend[i].u64PtsIncrease  = 0;
        stVdecSend[i].eThreadCtrl     = THREAD_CTRL_START;
        stVdecSend[i].bCircleSend     = HI_TRUE;
        stVdecSend[i].s32MilliSec     = 0;
        stVdecSend[i].s32MinBufSize   = (astSampleVdec[i].u32Width * astSampleVdec[i].u32Height * 3)>>1;
    }
    SAMPLE_COMM_VDEC_StartSendStream(u32VdecChnNum, &stVdecSend[0], &VdecThread[0]);

    return s32Ret;

END7:
    for(i=0; i<VpssGrpNum; i++)
    {
        s32Ret = SAMPLE_COMM_VPSS_UnBind_VO(i, 1, VoLayer, i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("vpss unbind vo fail for %#x!\n", s32Ret);
        }
    }

END6:
    for(i=0; i<u32VdecChnNum; i++)
    {
        s32Ret = SAMPLE_COMM_VDEC_UnBind_VPSS(i, i);
        if(s32Ret != HI_SUCCESS)
        {
            SAMPLE_PRT("vdec unbind vpss fail for %#x!\n", s32Ret);
        }
    }

END5:
    SAMPLE_COMM_VO_StopVO(&stVoConfig);

END4:
    for(i = VpssGrp; i >= 0; i--)
    {
        VpssGrp = i;
        SAMPLE_COMM_VPSS_Stop(VpssGrp, &abChnEnable[0]);
    }
END3:
    SAMPLE_COMM_VDEC_Stop(u32VdecChnNum);

END2:
    SAMPLE_COMM_VDEC_ExitVBPool();

END1:
    SAMPLE_COMM_SYS_Exit();

    return s32Ret;
}
HI_VOID  Mpp_Video_Stop_File_Vdec_Vpss(void)
{
	SAMPLE_COMM_VDEC_CmdCtrl(0, &stVdecSend[0], &VdecThread[0]);

	SAMPLE_COMM_VDEC_StopSendStream(0, &stVdecSend[0], &VdecThread[0]);
	SAMPLE_COMM_VDEC_UnBind_VPSS(0, 0);
	SAMPLE_COMM_VO_StopVO(&stVoConfig);
	SAMPLE_COMM_VPSS_Stop(0, &abChnEnable[0]);
       SAMPLE_COMM_VDEC_Stop(0);	
	SAMPLE_COMM_VDEC_ExitVBPool();   
	SAMPLE_COMM_SYS_Exit();
}

HI_S32 Mpp_Video_Get_Frame_From_Vpss(SENSOR_NUM_E sensor_num,STREAM_NUM_E stream_num,VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	HI_S32 Ret = -1;
	HI_S32 s32MilliSec = 20000;
	VPSS_GRP VpssGrp = sensor_num;
	VPSS_CHN VpssChn = stream_num;

	CHECK_NULL_PTR(pstVideoFrame);
	Ret = HI_MPI_VPSS_GetChnFrame( VpssGrp, VpssChn, pstVideoFrame, s32MilliSec);
	if(HI_SUCCESS != Ret)
	{
 		SAMPLE_PRT("HI_MPI_VPSS_GetChnFrame fail for %#x!\n", Ret);
	}

	return HI_SUCCESS;
}

HI_S32 Mpp_Video_Send_Frame_To_Vo(SENSOR_NUM_E sensor_num,VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	HI_S32 Ret = -1;
	HI_S32 s32MilliSec = 20000;
	VO_LAYER VoLayer = 0;
	VO_CHN VoChn= sensor_num;

	CHECK_NULL_PTR(pstVideoFrame);
	Ret = HI_MPI_VO_SendFrame( VoLayer, VoChn, pstVideoFrame, s32MilliSec);
	if(HI_SUCCESS != Ret)
	{
 		SAMPLE_PRT("HI_MPI_VO_SendFrame fail for %#x!\n", Ret);
	}
	return HI_SUCCESS;
}

HI_S32 Mpp_Video_Release_VpssFrame(SENSOR_NUM_E sensor_num,STREAM_NUM_E stream_num,VIDEO_FRAME_INFO_S *pstVideoFrame)
{
	HI_S32 Ret = -1;
	VPSS_GRP VpssGrp = sensor_num;
	VPSS_CHN VpssChn = stream_num;

	CHECK_NULL_PTR(pstVideoFrame);
	Ret = HI_MPI_VPSS_ReleaseChnFrame( VpssGrp, VpssChn, pstVideoFrame);
	if(HI_SUCCESS != Ret)
	{
 		SAMPLE_PRT("HI_MPI_VPSS_ReleaseChnFrame fail for %#x!\n", Ret);
	}

	return HI_SUCCESS;
}

