/*!
***********************************************************************
*  @mainpage
*     This is the main page for camera snap module. For detailed documentation
*     see the comments in each file.
*
*
*     For bug reporting and known issues see:
*     https://www.cse.fau.edu/mlab#
*
*  @author
*     The main contributors are listed in contributors.h
*
*  @version
*     1.0
*
*  @note
*     tags are used for document system "doxygen"
*     available at http://www.doxygen.org
*/

/*!
*  @file
*     mainHeader.h
*  @brief
*     OTC project main()
*  @author
*   Main contributors (see contributors.h for copyright, address and affiliation details)
*   - Waazim Reza               <wreza@fau.edu>
***********************************************************************
*/

#include "mainHeader.h"


// global camera data
tCamera         GCamera1;
tCamera			GCamera2;

int numCameras = 0;

#if defined(_LINUX) || defined(_QNX) || defined(_OSX)
struct tms      gTMS;
unsigned long   gT00 = times(&gTMS);


void Sleep(unsigned int time)
{
	struct timespec t,r;
	
	t.tv_sec    = time / 1000;
	t.tv_nsec   = (time % 1000) * 1000000;    
	
	while(nanosleep(&t,&r)==-1)
		t = r;
}

unsigned long GetTickCount()
{
	unsigned long lNow = times(&gTMS);

	if(lNow < gT00)
		gT00 = lNow;
	
	return (unsigned long)((float)(lNow - gT00) * 10000000.0 / (float)CLOCKS_PER_SEC);
}

void SetConsoleCtrlHandler(void (*func)(int), int junk)
{
	signal(SIGINT, func);
}
#endif

#ifdef _WINDOWS
unsigned long __stdcall Thread2Func(void *pContext)
#else
void *Thread2Func(void *pContext)
#endif
{
	while(!GCamera1.Abort)
	{
		//printf("Thread 2 \n");
	}
	return 0;
}
#ifdef _WINDOWS
unsigned long __stdcall ThreadFunc(void *pContext)
#else
void *ThreadFunc(void *pContext)
#endif
{
	unsigned long Completed,Dropped,Done;
	unsigned long Before,Now,Total,Elapsed;
	unsigned long Missed,Errs;
	double Fps;
	float Rate;
	tPvErr Err;

	tCamera *tCamInstance = (tCamera*)pContext;

	//printf("Cam Instance%lu \n",tCamInstance->UID);


	Fps = 0;
	Elapsed = 0;
	Total = 0;
	Done = 0;
	Before = GetTickCount();

	while(!tCamInstance->isUnplugged)
	{
		while(!tCamInstance->Abort &&
			!(Err = PvAttrUint32Get(tCamInstance->Handle,"StatFramesCompleted",&Completed)) &&
			!(Err = PvAttrUint32Get(tCamInstance->Handle,"StatFramesDropped",&Dropped)) &&
			!(Err = PvAttrUint32Get(tCamInstance->Handle,"StatPacketsMissed",&Missed)) &&
			!(Err = PvAttrUint32Get(tCamInstance->Handle,"StatPacketsErroneous",&Errs)) &&
			!(Err = PvAttrFloat32Get(tCamInstance->Handle,"StatFrameRate",&Rate)))
		{
			Now = GetTickCount();
			Total += Completed - Done;
			Elapsed += Now-Before;

			if(Elapsed >= 500)
			{
				Fps = (double)Total * 1000.0 / (double)Elapsed;
				Elapsed = 0;
				Total = 0;
			}

			printf("completed : %9lu dropped : %9lu missed : %9lu err. : %9lu rate : %5.2f (%5.2f)\r",
			Completed,Dropped,Missed,Errs,Rate,Fps);
			Before = GetTickCount();
			Done = Completed;

			Sleep(20);
		}
	}
	printf("Came out of the thread \n");
	printf("\n");

	return 0;
}

// spawn a thread
void SpawnThread(tCamera *tCamInstance)
{

	if(tCamInstance->readyToCapture)
	{

		printf("Inside spawn Thread \n");
#ifdef _WINDOWS	
		tCamInstance->ThHandle = CreateThread(NULL,NULL,ThreadFunc,tCamInstance,NULL,&(tCamInstance->ThId));
#else
		pthread_create(tCamInstance->ThHandle,NULL,ThreadFunc,(void *)&GCamera1);
#endif    
		// we wait until there is no more camera
		WaitForEver(tCamInstance);

		//printf("Waiting for thread \n");
		// then wait for the thread to finish
		if(tCamInstance->ThHandle)
			WaitThread(tCamInstance);     

		// stop the camera
		CameraStop(tCamInstance);   
	}
}

// wait for the thread to be over
void WaitThread(tCamera *tCamInstance)
{
	#ifdef _WINDOWS		
	WaitForSingleObject(tCamInstance->ThHandle,INFINITE);
	#else
	pthread_join(tCamInstance->ThHandle,NULL);
	#endif
}

// wait for a camera to be plugged


/*!
* @brief 
*		 Callback called when a camera is plugged or unplugged
* @param 
*		context
* @param 
*		interface
* @param 
*		Event ID
* @param 
*		Unige device Id
* @author 
*		Waazim Reza
* @return 
*		void
*/
void _STDCALL CameraEventCB(void* Context,
                            tPvInterface Interface,
                            tPvLinkEvent Event,
                            unsigned long UniqueId)
{
	tPvErr errorCode;
	tCamera *tCamInstance = NULL;
	switch(Event)
	{
	case ePvLinkAdd:
		{
			printf("camera %lu plugged\n",UniqueId);
			numCameras++;
			if(UniqueId==112322)
			{
				tCamInstance = &GCamera1;
			}
			else
			{
				tCamInstance = &GCamera2;
			}
			tCamInstance->isUnplugged = false;
			if(!CameraGrab(tCamInstance,UniqueId))
			{
				printf("Could not grab any cameras \n");
			}

		
			tCamInstance->readyToCapture = false;

			errorCode = CameraSetup(tCamInstance);	//Open the camera 
			convertandPrintErrorCode(errorCode);

			tCamInstance->readyToCapture = CameraStart(tCamInstance);
			


			break;
		}
	case ePvLinkRemove:
		{
			printf("camera %lu unplugged\n",UniqueId);															
			if(UniqueId==112322)
			{
				tCamInstance = &GCamera1;
			}
			else
			{
				tCamInstance = &GCamera2;
			}

			tCamInstance->isUnplugged = true;
			CameraUnsetup(tCamInstance);
			numCameras--;

			break;
		}
	default:
		break;
	}
}

/*!
* @brief 
*		This function registers call back events for camera plugging and unplugging and 
*		waits for a camera to be plugged
* @param 
*		void
* @author 
*		Waazim Reza
* @see 
*		CameraEventCB()
* @return 
*		void
*/
unsigned long  __stdcall WaitForCamera(void *pContext)
//void WaitForCamera()
{
	
	tPvErr errorCode = PvLinkCallbackRegister(CameraEventCB,ePvLinkAdd,NULL);//Separate thread
	convertandPrintErrorCode(errorCode);

    errorCode = PvLinkCallbackRegister(CameraEventCB,ePvLinkRemove,NULL);//Separate thread
	convertandPrintErrorCode(errorCode);

	printf("waiting for a camera ...\n");

	while(!PvCameraCount())		//Waiting for atleast one camera to be connected
	{
		//printf(".");
		Sleep(250);
	}
	//printf("\n");

	while(1)
	{
	}
}



/*!
* @brief 
*		This function waits forever (at least until CTRL-C)
* @param 
*		void
* @author 
*		Waazim Reza
* @see 
*		WaitForCamera()
* @return 
*		void
*/
void WaitForEver(tCamera *tCamInstance)
{
  while(!tCamInstance->Abort)
		Sleep(500);
}



/*!
* @brief 
*		callback called when a frame is done
* @param 
*		instance of tPvFrame
* @author 
*		Waazim Reza
* @see 
*		CameraStart()
* @return 
*		void
*/
void _STDCALL FrameDoneCB(tPvFrame* pFrame)
{
	char filename[50];
	sprintf(filename,"%s%d%s", "./Images/Waaz",pFrame->FrameCount,".tiff");
	if(!ImageWriteTiff(filename,(pFrame)))
	{
		printf("Failed to save the grabbed frame!");
	}
	else
	{
		//printf("frame saved\n");
	}

    // if the frame was completed (or if data were missing/lost) we re-enqueue it
    if(pFrame->Status == ePvErrSuccess  || 
       pFrame->Status == ePvErrDataLost ||
       pFrame->Status == ePvErrDataMissing)
        PvCaptureQueueFrame(GCamera1.Handle,pFrame,FrameDoneCB);  
}



/*!
* @brief 
*		grabs a camera of the specified UID
* @param 
*		UID of the camera
* @author 
*		Waazim Reza
* @see 
*		
* @return 
*		bool
*/
bool CameraGrab(tCamera *tCamInstance,unsigned long UID)
{
	if(	tCamInstance)
	{
		tCamInstance->UID = UID;
	}
	else{
		return false;
	}
	return true;
}

/*!
* @brief 
*		Open a camera
* @param 
*		void
* @author 
*		Waazim Reza
* @see 
*		
* @return 
*		tPvErr
*/
tPvErr CameraSetup(tCamera *tCamInstance)
{
	tCamInstance->isUnplugged = false;
	return PvCameraOpen(tCamInstance->UID,ePvAccessMaster,&(tCamInstance->Handle));   
}


/*!
* @brief 
*		setup and start streaming
* @param 
*		void
* @author 
*		Waazim Reza
* @see 
*		
* @return 
*		bool
*/
bool CameraStart(tCamera *tCamInstance)
{
	unsigned long FrameSize = 0;

	// Auto adjust the packet size to max supported by the network, up to a max of 8228.
	// NOTE: In Vista, if the packet size on the network card is set lower than 8228,
	//       this call may break the network card's driver. See release notes.
	//
	//PvCaptureAdjustPacketSize(tCamInstance->Handle,8228);

	// how big should the frame buffers be?
	if(!PvAttrUint32Get(tCamInstance->Handle,"TotalBytesPerFrame",&FrameSize))
	{
		bool failed = false;

		// allocate the buffer for each frames
		for(int i=0;i<FRAMESCOUNT && !failed;i++)
		{
			tCamInstance->Frames[i].ImageBuffer = new char[FrameSize];
			if(tCamInstance->Frames[i].ImageBuffer)
				tCamInstance->Frames[i].ImageBufferSize = FrameSize;
			else
				failed = true;
		}

		if(!failed)

		{
			// set the camera is acquisition mode
			if(!PvCaptureStart(tCamInstance->Handle))
			{
				// start the acquisition and make sure the trigger mode is "freerun"
				if(PvCommandRun(tCamInstance->Handle,"AcquisitionStart") ||
				   PvAttrEnumSet(tCamInstance->Handle,"FrameStartTriggerMode","Freerun"))
				{
					// if that fail, we reset the camera to non capture mode
					PvCaptureEnd(tCamInstance->Handle) ;
					return false;
				}
				else                
				{
					// then enqueue all the frames
				
					for(int i=0;i<FRAMESCOUNT;i++)
						PvCaptureQueueFrame(tCamInstance->Handle,&(tCamInstance->Frames[i]),FrameDoneCB);

					printf("frames queued ...\n");

					return true;
				}
			}
			else
				return false;
		}
		else
			return false;
	}
	else
		return false;
}

// stop streaming
void CameraStop(tCamera *tCamInstance)
{
    PvLinkCallbackUnRegister(CameraEventCB,ePvLinkAdd);
    PvLinkCallbackUnRegister(CameraEventCB,ePvLinkRemove);

	printf("stopping streaming\n");
	PvCommandRun(tCamInstance->Handle,"AcquisitionStop");
	PvCaptureEnd(tCamInstance->Handle);  
}

// unsetup the camera
void CameraUnsetup(tCamera *tCamInstance)
{
	// dequeue all the frame still queued (this will block until they all have been dequeued)
	PvCaptureQueueClear(tCamInstance->Handle);
	// then close the camera
	PvCameraClose(tCamInstance->Handle);

	// delete all the allocated buffers
	for(int i=0;i<FRAMESCOUNT;i++)
		delete [] (char*)tCamInstance->Frames[i].ImageBuffer;
}

// CTRL-C handler
#ifdef _WINDOWS
BOOL WINAPI CtrlCHandler(DWORD dwCtrlType)
#else
void CtrlCHandler(int Signo)
#endif	
{  
	GCamera1.Abort = true;    
		
	#ifndef _WINDOWS
	signal(SIGINT, CtrlCHandler);
	#else
	return true;
	#endif
}
/*
//void CameraThread(tCamera *tCamInstance)
unsigned long  __stdcall CameraThread(void *pContext)
{
	tCamera *tCamInstance =(tCamera*)pContext;

	if(tCamInstance->readyToCapture)
	{
		//	printf("camera is streaming now. Press CTRL-C to terminate\n");

		// spawn a thread
		SpawnThread(tCamInstance);

		// we wait until there is no more camera
		WaitForEver(tCamInstance);

		//printf("Waiting for thread \n");
		// then wait for the thread to finish
		if(tCamInstance->ThHandle)
			WaitThread(tCamInstance);     

		// stop the camera
		CameraStop(tCamInstance);   
	}
	return 1;
}
*/
int main(int argc, char* argv[])
{
	//Delete the Images captured for the previous run
	system("deleteIMGs.bat");		
	tCamera *tCamInstance1 = NULL;
	tCamera *tCamInstance2 = NULL;

	DWORD           cameraThreadID1;
	DWORD           cameraThreadID2;
	DWORD           waitForCameraID;
	HANDLE          ThWaitForCamera;
	HANDLE          ThHandle1;
	HANDLE          ThHandle2;

	memset(&GCamera1,0,sizeof(tCamera));
	memset(&GCamera2,0,sizeof(tCamera));

	// initialise the Prosilica API
	if(!PvInitialize())
	{ 

		SetConsoleCtrlHandler(&CtrlCHandler, TRUE);

		// wait for a camera to be plugged
		//WaitForCamera();
		ThWaitForCamera = CreateThread(NULL,NULL,WaitForCamera,tCamInstance1,NULL,&waitForCameraID);
#if 1
		Sleep(1000);
		if(GCamera1.UID)
		{
			printf("Before spawn Thread \n");
			//tCamInstance1 = &GCamera1;
			//ThHandle1 = CreateThread(NULL,NULL,CameraThread,tCamInstance1,NULL,&cameraThreadID1);
			SpawnThread(&GCamera1);
		}

		if(ThWaitForCamera)
		{
			WaitForSingleObject(ThWaitForCamera,INFINITE);
			//WaitForSingleObject(ThHandle1,INFINITE);
		}

/*		if(GCamera2.UID)
		{
			tCamInstance2 = &GCamera2;
			ThHandle2 = CreateThread(NULL,NULL,CameraThread,tCamInstance2,NULL,&cameraThreadID2);
		}
*/
		//CameraThread(tCamInstance1);
#endif
		// uninitialise the API
		PvUnInitialize();
	}


	return 0;
}
