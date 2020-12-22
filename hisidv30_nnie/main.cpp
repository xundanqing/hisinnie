#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/prctl.h>
#include <signal.h>
#include <asm/sigcontext.h>
#include <deque>
#include "opencv2/opencv.hpp"
#include "faceDetect.h"
#include "faceAlign.h"
#include "faceRecognition.h"
#include "detector.h"
#include "NNIEVideoCaputre.h"
#include "iniparser.h"
#include "dictionary.h"
#include "util.h"
#include "lane.h"

using namespace std;


static bool g_process_flag = true;

typedef struct
{
    const char *name;
    int signum;
    int flags;        //0:default handler, >0:catch, <0:ignore
}SigTable_ST;
static SigTable_ST sigTable[] =
{
    {"zero", 0, 0},
    {"SIGHUP", SIGHUP, -1},
    {"SIGINT", SIGINT, 1},
    {"SIGQUIT", SIGQUIT, -1},
    {"SIGILL", SIGILL, 1},
    {"SIGTRAP", SIGTRAP, -1},
    {"SIGABRT", SIGABRT, 1},
    {"SIGBUS", SIGBUS, 1},
    {"SIGFPE", SIGFPE, -1},
    {"SIGKILL", SIGKILL, 0},
    {"SIGUSR1", SIGUSR1, 2},
    {"SIGSEGV", SIGSEGV, 1},
    {"SIGUSR2", SIGUSR2, -1},
    {"SIGPIPE", SIGPIPE, -1},
    {"SIGALRM", SIGALRM, -1},
    {"SIGTERM", SIGTERM, 1},
    {"SIGSTKFLT", SIGSTKFLT, -1},
    //{"SIGCHLD", SIGCHLD, -1},
    {"SIGCONT", SIGCONT, -1},
    {"SIGSTOP", SIGSTOP, 0},
    {"SIGTSTP", SIGTSTP, -1},
    {"SIGTTIN", SIGTTIN, -1},
    {"SIGTTOU", SIGTTOU, -1},
    {"SIGURG", SIGURG, -1},
    {"SIGXCPU", SIGXCPU, -1},
    {"SIGXFSZ", SIGXFSZ, -1},
    {"SIGVTALRM", SIGVTALRM, -1},
    {"SIGPROF", SIGPROF, -1},
    {"SIGWINCH", SIGWINCH, -1},
    {"SIGIO", SIGIO, -1},
    {"SIGPWR", SIGPWR, -1},
    {"SIGSYS", SIGSYS, -1},
};
static void HandleSig(int signo)
{
	printf("app  exit!!!! signal num = %d ,signal name is %s\n",signo,sigTable[signo].name);
	g_process_flag = false;
}
static void  Fun_Signal_Handle(void)
{
	struct sigaction sigsetup;
	unsigned int i;

    //set signal handler
    for(i=0; i<sizeof(sigTable)/sizeof(sigTable[0]); i++)
    {
        //exit on system signal
        if(sigTable[i].flags == 1)
        {
            memset(&sigsetup, 0, sizeof(sigsetup));
            sigsetup.sa_handler = HandleSig;
            sigsetup.sa_flags = 0;
            sigemptyset(&sigsetup.sa_mask);
            sigaction(sigTable[i].signum, &sigsetup, NULL);
        }

        //ignore these signal
        if(sigTable[i].flags < 0)
        {
            memset(&sigsetup, 0, sizeof(sigsetup));
            sigsetup.sa_handler = SIG_IGN;
            sigsetup.sa_flags = 0;
            sigemptyset(&sigsetup.sa_mask);
            sigaction(sigTable[i].signum, &sigsetup, NULL);
        }
    }

}


int main()
{

    Fun_Signal_Handle();
	
    dictionary *config;
	if(NULL == (config = iniparser_load("./config.ini"))){
		printf("ERROR: open file \" ./config.ini \"failed!\n");
        return -1;
	}
   
    //GlobalInfo globalInfo;
    //Lane* mLane = new Lane(config);
   // Detector* mDetector = new Detector(config);

   // mLane->build();
   // mDetector->build();
	
    VIDEO_FRAME_INFO_S  stframe;
		    
    NNIEVideoCapture *  video_capture =   new  NNIEVideoCapture(config);

	while(1)
    {
			video_capture->isOpened();

            video_capture->read(SENSOR_0,BASE_STREAM,&stframe);


			




	}
#if  0		
    ifstream  infile("./lane.lst");
    string imgPath;
    while(infile >> imgPath)
    {   
        std::cout<< "processing file: " << imgPath << std::endl;
        globalInfo.reset();
        cv::Mat img = cv::imread(imgPath);

        mLane->doInference(img,globalInfo);
        mDetector->doInference(img,globalInfo);

        globalInfo.putOnImage(img);
        std::string savePath = imgPath.replace(imgPath.find("lane"),4,"lane_result");
        cv::imwrite(savePath,img);

    }
#endif
   
}


// #include <iostream>
// #include <string>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <pthread.h>
// #include <sys/prctl.h>
// #include <signal.h>
// #include <asm/sigcontext.h>
// #include "opencv2/opencv.hpp"
// #include "iniparser.h"
// #include "dictionary.h"
// #include "faceDetect.h"
// #include "faceAlign.h"
// #include "faceRecognition.h"
// #include "NNIEVideoCaputre.h"
// using namespace std;

// static bool g_process_flag = true;

// typedef struct
// {
//     const char *name;
//     int signum;
//     int flags;        //0:default handler, >0:catch, <0:ignore
// }SigTable_ST;
// static SigTable_ST sigTable[] =
// {
//     {"zero", 0, 0},
//     {"SIGHUP", SIGHUP, -1},
//     {"SIGINT", SIGINT, 1},
//     {"SIGQUIT", SIGQUIT, -1},
//     {"SIGILL", SIGILL, 1},
//     {"SIGTRAP", SIGTRAP, -1},
//     {"SIGABRT", SIGABRT, 1},
//     {"SIGBUS", SIGBUS, 1},
//     {"SIGFPE", SIGFPE, -1},
//     {"SIGKILL", SIGKILL, 0},
//     {"SIGUSR1", SIGUSR1, 2},
//     {"SIGSEGV", SIGSEGV, 1},
//     {"SIGUSR2", SIGUSR2, -1},
//     {"SIGPIPE", SIGPIPE, -1},
//     {"SIGALRM", SIGALRM, -1},
//     {"SIGTERM", SIGTERM, 1},
//     {"SIGSTKFLT", SIGSTKFLT, -1},
//     //{"SIGCHLD", SIGCHLD, -1},
//     {"SIGCONT", SIGCONT, -1},
//     {"SIGSTOP", SIGSTOP, 0},
//     {"SIGTSTP", SIGTSTP, -1},
//     {"SIGTTIN", SIGTTIN, -1},
//     {"SIGTTOU", SIGTTOU, -1},
//     {"SIGURG", SIGURG, -1},
//     {"SIGXCPU", SIGXCPU, -1},
//     {"SIGXFSZ", SIGXFSZ, -1},
//     {"SIGVTALRM", SIGVTALRM, -1},
//     {"SIGPROF", SIGPROF, -1},
//     {"SIGWINCH", SIGWINCH, -1},
//     {"SIGIO", SIGIO, -1},
//     {"SIGPWR", SIGPWR, -1},
//     {"SIGSYS", SIGSYS, -1},
// };
// static void HandleSig(int signo)
// {
// 	printf("app  exit!!!! signal num = %d ,signal name is %s\n",signo,sigTable[signo].name);
// 	g_process_flag = false;
// }
// static void  Fun_Signal_Handle(void)
// {
// 	struct sigaction sigsetup;
// 	unsigned int i;

//     //set signal handler
//     for(i=0; i<sizeof(sigTable)/sizeof(sigTable[0]); i++)
//     {
//         //exit on system signal
//         if(sigTable[i].flags == 1)
//         {
//             memset(&sigsetup, 0, sizeof(sigsetup));
//             sigsetup.sa_handler = HandleSig;
//             sigsetup.sa_flags = 0;
//             sigemptyset(&sigsetup.sa_mask);
//             sigaction(sigTable[i].signum, &sigsetup, NULL);
//         }

//         //ignore these signal
//         if(sigTable[i].flags < 0)
//         {
//             memset(&sigsetup, 0, sizeof(sigsetup));
//             sigsetup.sa_handler = SIG_IGN;
//             sigsetup.sa_flags = 0;
//             sigemptyset(&sigsetup.sa_mask);
//             sigaction(sigTable[i].signum, &sigsetup, NULL);
//         }
//     }

// }


// int main()
// {	
//     Fun_Signal_Handle();
// 	dictionary *config;
// 	if(NULL == (config = iniparser_load("./config.ini")))
// 	{
// 		printf("ERROR: open file \" ./config.ini \"failed!\n");
//               return -1;
// 	}

// 	FaceDetector* mFaceDet = new FaceDetector(config);
//     FaceAlign* mFaceAlign = new FaceAlign(config);
// 	mFaceDet->build();
//     mFaceAlign->build();

// 	NNIEVideoCapture *videocap = new NNIEVideoCapture(config);
// 	if(!videocap->isOpened())
// 	{
// 		std::cerr << "can not open the corresponding camera!" << std::endl;
// 		exit(-1);
// 	}

//     nnie::Mat landmarks;
//     nnie::Mat pose;
//     cv::Mat frame;

// 	while(g_process_flag)
// 	{
// 		if(videocap->read(SENSOR_0, frame))
// 		{
//             // cv::Rect box;
//             // mFaceDet->doInference(frame,box);
//             // if( box.area() > 0)
//             //     mFaceAlign->doInference(frame,box,landmarks,pose);
//             cv::imwrite("det0.jpg",frame);
// 		}

// 		if(videocap->read(SENSOR_1, frame))
// 		{
//             // cv::Rect box;
//             // mFaceDet->doInference(frame,box);
//             // if( box.area() > 0)
//             //     mFaceAlign->doInference(frame,box,landmarks,pose);
//             cv::imwrite("det1.jpg",frame);
// 		}


// 	}
// EXIT: 
// 	delete mFaceDet;
// 	delete mFaceAlign;
// 	delete videocap;
// 	 exit(-1);

// }
    
 

