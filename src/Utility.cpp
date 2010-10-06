/*!
 *  @file
 *     Utility.cpp
 *  @brief
 *     OTC project: This file contains functions and data structures declaration
 *	   for some common utilities for all other files
 *  @author
 *   Main contributors (see contributors.h for copyright, address and affiliation details)
 *   - Waazim Reza               <wreza@fau.edu>
 ***********************************************************************
 */

#include"Utility.h"

/*!
 * @brief 
 *		Converts the enum error code and prints the corresponding error message
 * @param 
 *		the enum value errorCode
 * @author 
 *		Waazim Reza
 * @return 
 *		void
 */
 void convertandPrintErrorCode(tPvErr errorCode)
{
	switch(int(errorCode))
	{
	case 0:	
		break;
	case 1:
		//printf("Success – no error \n");
		break;
	case 2:
		printf("Unexpected camera fault \n");
		break;

	case 3:
		printf("Unexpected fault in PvAPI or driver \n");
		break;

	case 4:
		printf("Camera handle is bad \n");
		break;

	case 5:
		printf("Function parameter is bad \n");
		break;

	case 6:
		printf("Incorrect sequence of API calls \n");
		break;

	case 7:
		printf("queuing a frame before starting image capture \n");
		break;

	case 8:
		printf("camera \
cannot be opened in the requested mode, because\
it is already in use by another application \n");
		break;

	case 9:
		printf("camera has been unexpectedly unplugged \n");
		break;

	case 10:
		printf("user attempts to capture\
images, but the camera setup is incorrect \n");
		break;

	case 11:
		printf("Required system or network resources are\
unavailable \n");
		break;

	case 12:
		printf("The frame queue is full \n");
		break;

	case 13:
		printf("The frame buffer is too small to store the image \n");
		break;

	case 14:
		printf("Frame is cancelled. This is returned when frames\
are aborted using PvCaptureQueueClear \n");
		break;

	case 15:
		printf("The data for this frame was lost. The contents of\
the image buffer are invalid \n");
		break;

	case 16:
		printf("Some of the data in this frame was lost \n");
		break;

	case 17:
		printf("Timeout expired. This is returned only by functions\
with a specified timeout \n");
		break;

	case 18:
		printf("The attribute value is out of range \n");
		break;

	case 19:
		printf("This function cannot access the attribute, because\
the attribute type is different \n");
		break;

	case 20:
		printf("The attribute cannot be written at this time \n");
		break;

	case 21:
		printf("The attribute is not available at this time \n");
		break;

	case 22:
		printf("Windows’ firewall is blocking the streaming port \n");
		break;

	}			   
}			 
