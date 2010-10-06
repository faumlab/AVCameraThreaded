/*!
 *  @file
 *     Snap.h
 *  @brief
 *     OTC project: This file contains functions and data structures declaration
 *	   for the snap module of the camera
 *  @author
 *   Main contributors (see contributors.h for copyright, address and affiliation details)
 *   - Waazim Reza               <wreza@fau.edu>
 ***********************************************************************
 */

/*
	Include all the required header files here :
	Check for windows or Linux compatibility also
*/
#ifdef _WINDOWS
#include "StdAfx.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma warning (disable : 4996)
#ifdef _WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif
#if defined(_LINUX) || defined(_QNX) || defined(_OSX)
#include <unistd.h>
#include <time.h>
#endif

//Include headers from for PvApi and Image lib 
#include <PvApi.h>



//Include the File Parser functionalities for parsing the input config file 
//#include "ParseFile.h"

//#include "Utility.h"
#include "snapCallback.h"
#include "Utility.h"

#define FRAMESCOUNT 10


/*!
 * @brief 
 *		Camera Data structure containing info like Unique 
 *		camera ID, Handle and the frame
 *
 * @author Waazim Reza
 * @version 1.0
 */
typedef struct 
{
	unsigned long   UID;
	tPvHandle       Handle;
	tPvFrame        Frames[FRAMESCOUNT];
#ifdef _WINDOWS
	HANDLE          ThHandle;
	DWORD           ThId;
#else
	pthread_t       ThHandle;
#endif    
	bool            Abort;
	bool            readyToCapture;
	bool			isUnplugged;

} tCamera;


bool CameraGrab(tCamera *tCamInstance,unsigned long UID);
tPvErr CameraSetup(tCamera *tCamInstance);
bool CameraStart(tCamera *tCamInstance);
void CameraUnsetup(tCamera *tCamInstance);
void WaitThread(tCamera *tCamInstance);
void CameraStop(tCamera *tCamInstance);
void WaitForEver(tCamera *tCamInstance);