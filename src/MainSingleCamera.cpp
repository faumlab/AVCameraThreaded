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
*     Snap.cpp
*  @brief
*     OTC project main()
*  @author
*   Main contributors (see contributors.h for copyright, address and affiliation details)
*   - Waazim Reza               <wreza@fau.edu>
***********************************************************************
*/

#include "mainHeader.h"


#define FRAMESCOUNT 10


// global camera data
tCamera         GCamera;
tCamera GCamera1;
tCamera GCamera2;

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

	Fps = 0;
	Elapsed = 0;
	Total = 0;
	Done = 0;
	Before = GetTickCount();

	while(!GCamera1.Abort &&
		  !(Err = PvAttrUint32Get(GCamera1.Handle,"StatFramesCompleted",&Completed)) &&
		  !(Err = PvAttrUint32Get(GCamera1.Handle,"StatFramesDropped",&Dropped)) &&
		  !(Err = PvAttrUint32Get(GCamera1.Handle,"StatPacketsMissed",&Missed)) &&
		  !(Err = PvAttrUint32Get(GCamera1.Handle,"StatPacketsErroneous",&Errs)) &&
		  !(Err = PvAttrFloat32Get(GCamera1.Handle,"StatFrameRate",&Rate)))
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
	
	printf("\n");

	return 0;
}

// spawn a thread
void SpawnThread()
{
#ifdef _WINDOWS	
	GCamera1.ThHandle = CreateThread(NULL,NULL,ThreadFunc,&GCamera1,NULL,&(GCamera1.ThId));
	GCamera2.ThHandle = CreateThread(NULL,NULL,Thread2Func,&GCamera2,NULL,&(GCamera2.ThId));

#else
	pthread_create(&GCamera1.ThHandle,NULL,ThreadFunc,(void *)&GCamera1);
	pthread_create(&GCamera2.ThHandle,NULL,ThreadFunc,(void *)&GCamera2);

#endif    

}

// wait for the thread to be over
void WaitThread()
{
	#ifdef _WINDOWS		
	WaitForSingleObject(GCamera1.ThHandle,INFINITE);
	WaitForSingleObject(GCamera2.ThHandle,INFINITE);
	#else
	pthread_join(GCamera1.ThHandle,NULL);
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
    switch(Event)
    {
        case ePvLinkAdd:
        {
            printf("camera %lu plugged\n",UniqueId);
            break;
        }
        case ePvLinkRemove:
        {
            printf("camera %lu unplugged\n",UniqueId);
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
void WaitForCamera()
{
    PvLinkCallbackRegister(CameraEventCB,ePvLinkAdd,NULL);
    PvLinkCallbackRegister(CameraEventCB,ePvLinkRemove,NULL);


	printf("waiting for a camera ...\n");

	while(!PvCameraCount() && !GCamera1.Abort && !GCamera2.Abort)
	{
		printf(".");
		Sleep(250);
	}
	printf("\n");
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
void WaitForEver()
{
  while(!GCamera1.Abort && !GCamera2.Abort)
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
//void Timestamp(tPvFrame* pFrame)
//{
//	FILE *fp
//	char Time_txt[50];
//	sprintf(Time_txt,"%s%d%s", "./TimeStamps/snap",pFrame->FrameCount,".txt");
//	fp = fopen(Time_txt,"w")
//	fprintf(fp,
//	
//
//}
void _STDCALL FrameDoneCB(tPvFrame* pFrame)
{
	char filename[50];
	//printf(filename,"%s%d%d%s", "./Images/snap",pFrame->TimestampHi,pFrame->TimestampLo,".tiff");
	  
	unsigned long long timeStampMerged  = pFrame->TimestampLo + 
                             pFrame->TimestampHi*4294967296.;
	
	sprintf(filename,"%s%I64u%s", "./Images/snap",timeStampMerged,".tiff");
	
	if(!ImageWriteTiff(filename,(pFrame)))
	{
		printf("Failed to save the grabbed frame!");
	}
	else
	{
		printf("frame saved\n");
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
bool CameraGrab(unsigned long UID)
{
		GCamera1.UID = UID;
		GCamera2.UID = UID;
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
*		bool
*/
bool CameraSetup()
{
	return !PvCameraOpen(GCamera1.UID,ePvAccessMaster,&(GCamera1.Handle));   
}


/*!
* @brief 
*		setup and start streamings
* @param 
*		void
* @author 
*		Waazim Reza
* @see 
*		
* @return 
*		bool
*/
bool CameraStart()
{
	unsigned long FrameSize = 0;

	// Auto adjust the packet size to max supported by the network, up to a max of 8228.
	// NOTE: In Vista, if the packet size on the network card is set lower than 8228,
	//       this call may break the network card's driver. See release notes.
	//
	PvCaptureAdjustPacketSize(GCamera1.Handle,9014);

	// how big should the frame buffers be?
	if(!PvAttrUint32Get(GCamera1.Handle,"TotalBytesPerFrame",&FrameSize))
	{
		bool failed = false;

		// allocate the buffer for each frames
		for(int i=0;i<FRAMESCOUNT && !failed;i++)
		{
			GCamera1.Frames[i].ImageBuffer = new char[FrameSize];
			if(GCamera1.Frames[i].ImageBuffer)
				GCamera1.Frames[i].ImageBufferSize = FrameSize;
			else
				failed = true;
		}

		if(!failed)

		{
			// set the camera is acquisition mode
			if(!PvCaptureStart(GCamera1.Handle))
			{
				// start the acquisition and make sure the trigger mode is "freerun"
				if(PvCommandRun(GCamera1.Handle,"AcquisitionStart") || PvAttrFloat32Set(GCamera1.Handle,"FrameRate",15.0) ||
				   PvAttrEnumSet(GCamera1.Handle,"FrameStartTriggerMode","FixedRate"))
				{
					// if that fail, we reset the camera to non capture mode
					PvCaptureEnd(GCamera1.Handle) ;
					return false;
				}
				else                
				{
					// then enqueue all the frames
				
					for(int i=0;i<FRAMESCOUNT;i++)
						PvCaptureQueueFrame(GCamera1.Handle,&(GCamera1.Frames[i]),FrameDoneCB);

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
void CameraStop()
{
    PvLinkCallbackUnRegister(CameraEventCB,ePvLinkAdd);
    PvLinkCallbackUnRegister(CameraEventCB,ePvLinkRemove);

	printf("stopping streaming\n");
	PvCommandRun(GCamera1.Handle,"AcquisitionStop");
	PvCaptureEnd(GCamera1.Handle);  
}

// unsetup the camera
void CameraUnsetup()
{
	// dequeue all the frame still queued (this will block until they all have been dequeued)
	PvCaptureQueueClear(GCamera1.Handle);
	// then close the camera
	PvCameraClose(GCamera1.Handle);

	// delete all the allocated buffers
	for(int i=0;i<FRAMESCOUNT;i++)
		delete [] (char*)GCamera1.Frames[i].ImageBuffer;
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

int main(int argc, char* argv[])
{
	//Delete the Images captured for the previous run
	system("deleteIMGs.bat");		

	
	// initialise the Prosilica API
	if(!PvInitialize())
	{ 
		memset(&GCamera1,0,sizeof(tCamera));
		//memset(&GCamera2,0,sizeof(tCamera));

		SetConsoleCtrlHandler(&CtrlCHandler, TRUE);

		// wait for a camera to be plugged
		WaitForCamera();

		// grab a camera from the list
		if(!GCamera1.Abort && CameraGrab(112322))
		{
			// setup the camera
			if(CameraSetup())
			{
				// start streaming from the camera
				if(CameraStart())
				{
					printf("camera is streaming now. Press CTRL-C to terminate\n");

					// spawn a thread
					SpawnThread();

					// we wait until there is no more camera
					WaitForEver();

					// then wait for the thread to finish
					if(GCamera1.ThHandle)
						WaitThread();     

					// stop the camera
					CameraStop();                            
				}
				else
					printf("failed to start streaming\n");

				// unsetup the camera
				CameraUnsetup();
			}
			else
				printf("failed to setup the camera\n");
		}
		else
			printf("failed to find a camera\n");


		// uninitialise the API
		PvUnInitialize();
	}
	else
		printf("failed to initialise the API\n");


	return 0;
}
