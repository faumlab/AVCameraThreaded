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
*   - Waazim Reza, Sagar Aghera , Rafael Giusti               <wreza@fau.edu,saghera@fau.edu,rgiusti@fau.edu>
***********************************************************************
*/

#include "mainHeader.h"
#include "stdio.h"


// global camera data
tCamera         GCamera1;	//Camera Instance for the camera with UniqueId 112322
tCamera			GCamera2;	//Camera Instance for the camera with UniqueId 112321

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
unsigned long __stdcall CameraCaptureThread(void *pContext)
#else
void *CameraCaptureThread(void *pContext)
#endif
{
	/*
	Statistics of completed,done, lost and dropped frames
	*/
	unsigned long Completed,Dropped,Done;
	unsigned long Missed,Errs;
	
	/*
	Used for calculating the frame rate
	*/
	unsigned long Before,Now,Total,Elapsed; 
	double Fps;
	float Rate;
	
	/*
	Error returned by the camera snapping process
	*/
	tPvErr Err;

	tCamera *tCamInstance = (tCamera*)pContext;
	/*
	Initializatio of the variables
	*/
	Fps = 0;
	Elapsed = 0;
	Total = 0;
	Done = 0;

	/*
	Get the tick count before the start of camera snap process
	*/
	Before = GetTickCount();

	/*
	Start the capture process if the camera is not unplugged
	*/
	while(!tCamInstance->isUnplugged)
	{
		while(!tCamInstance->Abort) //TODO remove the abort
		{
			Err = PvAttrUint32Get(tCamInstance->Handle,"StatFramesCompleted",&Completed); 
			if(Err)
			{
				printf("Error in %s:%d at CameraCaptureThread() ----> ", __FILE__, __LINE__); 
				convertandPrintErrorCode(Err);
				break;
			}

			Err = PvAttrUint32Get(tCamInstance->Handle,"StatFramesCompleted",&Completed);
			if(Err)
			{
				printf("Error in %s:%d at CameraCaptureThread() ----> ", __FILE__, __LINE__); 
				convertandPrintErrorCode(Err);
				break;
			}

			Err = PvAttrUint32Get(tCamInstance->Handle,"StatFramesDropped",&Dropped);
			if(Err)
			{
				printf("Error in %s:%d at CameraCaptureThread() ----> ", __FILE__, __LINE__); 
				convertandPrintErrorCode(Err);
				break;
			}

			Err = PvAttrUint32Get(tCamInstance->Handle,"StatPacketsMissed",&Missed);
			if(Err)
			{
				printf("Error in %s:%d at CameraCaptureThread() ----> ", __FILE__, __LINE__); 
				convertandPrintErrorCode(Err);
				break;
			}

			Err = PvAttrUint32Get(tCamInstance->Handle,"StatPacketsErroneous",&Errs);
			if(Err)
			{
				printf("Error in %s:%d at CameraCaptureThread() ----> ", __FILE__, __LINE__); 
				convertandPrintErrorCode(Err);
				break;
			}

			Err = PvAttrFloat32Get(tCamInstance->Handle,"StatFrameRate",&Rate);
			if(Err)
			{
				printf("Error in %s:%d at CameraCaptureThread() ----> ", __FILE__, __LINE__); 
				convertandPrintErrorCode(Err);
				break;
			}
			Now = GetTickCount();
			Total += Completed - Done;
			Elapsed += Now-Before;

			if(Elapsed >= 500)
			{
				Fps = (double)Total * 1000.0 / (double)Elapsed;
				Elapsed = 0;
				Total = 0;
			}

			printf("Completed : %9lu dropped : %9lu missed : %9lu err. : %9lu rate : %5.2f (%5.2f)\r",
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
	/*
	Check if the readytoCapture flag is set
	If it is set go forward and complete start camera thread
	*/
	if(tCamInstance->readyToCapture)		
	{
		printf("Inside spawn Thread \n");
#ifdef _WINDOWS	
		tCamInstance->ThHandle = CreateThread(NULL,NULL,CameraCaptureThread,tCamInstance,NULL,&(tCamInstance->ThId));
#else
		pthread_create(tCamInstance->ThHandle,NULL,CameraCaptureThread,(void *)&GCamera1);
#endif    
		// We wait until there is no more camera
		WaitForEver(tCamInstance);

		// Then wait for the thread to finish
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
*		Waazim Reza, Sagar Aghera , Rafael Giusti
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

			/*
			Increment the number of cameras count
			*/
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
			printf("Num of cameras %d \n", numCameras);


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
			printf("Num of cameras %d \n", numCameras);
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
*		Waazim Reza, Sagar Aghera , Rafael Giusti
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
*		Waazim Reza, Sagar Aghera , Rafael Giusti
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
*		Waazim Reza, Sagar Aghera , Rafael Giusti
* @see 
*		CameraStart()
* @return 
*		void
*/
void _STDCALL FrameDoneCB(tPvFrame* pFrame)
{
	char filename[50];
	char filename1[50];
	char filevalue[50];
	unsigned long exp = 0;
	unsigned long  * stringsize = 0;
	tCamera *tCamInstance = NULL;

	/*
	TimestampHi is the higher 32 bits of the TimeStamp
	TimestampLo is the lower 32 bits of the TimeStamp
	
	Timestamps computation - 
	Shift the TimestampHi 32 bits to the left and add the TimestampLo
	*/
	unsigned long long timeStampMerged  = pFrame->TimestampLo + 
		pFrame->TimestampHi*4294967296.;


	//if(pFrame->Context[0])
	unsigned long  *pCamInstance = (unsigned long *)(pFrame->Context[0]);
	if(*pCamInstance == 112322) //TODO
	{
		tCamInstance = &GCamera1;
		
		if(!PvAttrEnumGet(tCamInstance->Handle,"ExposureValue",filevalue,128,NULL))
		{
		/*
		Concateneate the Merged timestamp with the filename
		*/
		
		sprintf(filename1,"%s%I64u%s", "./ImagesCamera1/snap",timeStampMerged,".txt");
		FILE *fp;
	fp = fopen(filename1,"w");
	fwrite(filevalue,1,sizeof(filevalue),fp);
		}
		
		sprintf(filename,"%s%I64u%s", "./ImagesCamera1/snap",timeStampMerged,".tiff");
	}
	else
	{
		tCamInstance = &GCamera2;
		if(!PvAttrEnumGet(tCamInstance->Handle,"ExposureValue",filevalue,128,NULL))
		{
		/*
		Concateneate the Merged timestamp with the filename
		*/
		
		sprintf(filename1,"%s%I64u%s", "./ImagesCamera1/snap",timeStampMerged,".txt");
		FILE *fp;
	fp = fopen(filename1,"w");
	fwrite(filevalue,1,sizeof(filevalue),fp);
		}
		//PvAttrEnumGet(tCamInstance,"ExposureExposureValue",filevalue,sizeof(filevalue),stringsize);
		//sprintf(filevalue,"%s%I64u", "Exposure time:",exp);
		sprintf(filename,"%s%I64u%s", "./ImagesCamera2/snap",timeStampMerged,".tiff");
		
	}
	
	

	/*
	Save the recieved frame to the disk
	*/
	if(!ImageWriteTiff(filename,(pFrame)))
	{
		printf("Failed to save the grabbed frame! \n ");
	}
	else
	{
		//printf("frame saved\n");
	}

	/*
		If the frame was completed (or if data were missing/lost) we re-enqueue it
	*/

	if(pFrame->Status == ePvErrSuccess  || 
		pFrame->Status == ePvErrDataLost ||
		pFrame->Status == ePvErrDataMissing)
		PvCaptureQueueFrame(tCamInstance->Handle,pFrame,FrameDoneCB);  
}



/*!
* @brief 
*		grabs a camera of the specified UID
* @param 
*		Camera Instance
* @param 
*		UID of the camera
* @author 
*		Waazim Reza, Sagar Aghera , Rafael Giusti
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
*		camera instance
* @author 
*		Waazim Reza, Sagar Aghera , Rafael Giusti
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
*		Camera Instance
* @author 
*		Waazim Reza, Sagar Aghera , Rafael Giusti
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
	tPvErr errorCode = PvAttrUint32Get(tCamInstance->Handle,"TotalBytesPerFrame",&FrameSize);
	if(errorCode)
	{
		printf("Error in %s:%d at CameraStart() ----> ", __FILE__, __LINE__); 
		convertandPrintErrorCode(errorCode);
		return false;
	}


	bool failed = false;

	// allocate the buffer for each frames
	for(int i=0;i<FRAMESCOUNT && !failed;i++)
	{
		tCamInstance->Frames[i].ImageBuffer = new char[FrameSize];
		if(tCamInstance->Frames[i].ImageBuffer)
			tCamInstance->Frames[i].ImageBufferSize = FrameSize;
		else
			return false;
	}

	// set the camera is acquisition mode
	errorCode =  PvCaptureStart(tCamInstance->Handle);
	if(errorCode)
	{
		printf("Error in %s:%d at CameraStart() ----> ", __FILE__, __LINE__); 
		convertandPrintErrorCode(errorCode);
		return false;
	}
	PvCommandRun(tCamInstance->Handle,"TimeStampReset");
	PvAttrEnumSet(tCamInstance->Handle,"ExposureMode","Auto");
	PvAttrEnumSet(tCamInstance->Handle,"GainMode","Auto");
	PvAttrEnumSet(tCamInstance->Handle,"WhitebalMode","Auto");
	
	if(PvCommandRun(tCamInstance->Handle,"AcquisitionStart") || PvAttrFloat32Set(tCamInstance->Handle,"FrameRate",3.0) ||
		PvAttrEnumSet(tCamInstance->Handle,"FrameStartTriggerMode","FixedRate"))
	{
		// if that fail, we reset the camera to non capture mode
		PvCaptureEnd(tCamInstance->Handle) ;
		return false;
	}
	else                
	{
		// then enqueue all the frames
		for(int i=0;i<FRAMESCOUNT;i++)
		{
			(tCamInstance->Frames[i].Context[0]) = (unsigned long *)&(tCamInstance->UID);
			//unsigned long * pCamInstance = (unsigned long  *)(tCamInstance->Frames->Context[0]);

			PvCaptureQueueFrame(tCamInstance->Handle,&(tCamInstance->Frames[i]),FrameDoneCB);
		}
		printf("frames queued ...\n");

		return true;
	}

}


/*!
* @brief 
*		stop streaming, unregister the events
* @param 
*		Camera Instance
* @author 
*		Waazim Reza, Sagar Aghera , Rafael Giusti
* @see 
*		
* @return 
*		void
*/
void CameraStop(tCamera *tCamInstance)
{
	PvLinkCallbackUnRegister(CameraEventCB,ePvLinkAdd);
	PvLinkCallbackUnRegister(CameraEventCB,ePvLinkRemove);

	printf("stopping streaming\n");
	PvCommandRun(tCamInstance->Handle,"AcquisitionStop");
	PvCaptureEnd(tCamInstance->Handle);  
}


/*!
* @brief 
*		Unsetup the camera
*		Close the camera
*		Clear the Queue
* @param 
*		Camera Instance
* @author 
*		Waazim Reza, Sagar Aghera , Rafael Giusti
* @see 
*		
* @return 
*		void
*/
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
//TODO Remove the control C handler
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


int main(int argc, char* argv[])
{
	//Delete the Images captured for the previous run
	//system("deleteIMGs.bat");		

	tCamera *tCamInstance1 = NULL;		//Camera Instances for camera 1 //TODO
	tCamera *tCamInstance2 = NULL;		//Camera Instances for camera 2 //TODO

	/*
	Three threads are part of the this implementation
	Main Thread  - Wait for cameras which would register CameraPlugged and CameraUnPluggedEvents				  	
	CameraThread1 - Camera Setup, camera open, camera capture start,capture image save for a camera 1
	CameraThread2 - Camera Setup, camera open, camera capture start,capture image save for a camera 2
	*/

	DWORD           cameraThreadID1;
	DWORD           cameraThreadID2;
	DWORD           waitForCameraID;

	HANDLE          ThWaitForCamera;
	HANDLE          ThHandle1;
	HANDLE          ThHandle2;

	/*
	Initialize the two global camera instances here
	*/
	memset(&GCamera1,0,sizeof(tCamera));
	memset(&GCamera2,0,sizeof(tCamera));

	// initialise the Prosilica API
	if(!PvInitialize())
	{ 
		/*
		TODO Remove control C Handler not required for our scenario
		*/
		SetConsoleCtrlHandler(&CtrlCHandler, TRUE);

		/*
		WaitForCamera Thread - Wait for a camera to be plugged using event registering
		*/
		ThWaitForCamera = CreateThread(NULL,NULL,WaitForCamera,tCamInstance1,NULL,&waitForCameraID);
		//WaitForSingleObject(ThWaitForCamera,INFINITE);

		/*
		Wait Untill either of the cameras are setup
		*/
		while(!GCamera1.readyToCapture && !GCamera2.readyToCapture);		

		if(GCamera1.UID) //TODO
		{
			SpawnThread(&GCamera1); //TODO
		}
		if(GCamera2.UID) //TODO
		{
			SpawnThread(&GCamera2); //TODO
		}

		if(ThWaitForCamera)
		{
			WaitForSingleObject(ThWaitForCamera,INFINITE);
			//WaitForSingleObject(ThHandle1,INFINITE);
		}

		PvUnInitialize();
	}


	return 0;
}
