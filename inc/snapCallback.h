/*!
 *  @file
 *     snapCallback.h
 *  @brief
 *     OTC project: This file contains callback function declarations
 *	   for the snap module of the camera
 *  @author
 *   Main contributors (see contributors.h for copyright, address and affiliation details)
 *   - Waazim Reza               <wreza@fau.edu>
 ***********************************************************************
 */


#include "stdio.h"
#include <PvApi.h>
#ifdef _WINDOWS
#define _STDCALL __stdcall
#else
#define _STDCALL
#define TRUE     0
#endif

#include <ImageLib.h>

void _STDCALL FrameDoneCB(tPvFrame* pFrame);
void _STDCALL CameraEventCB(void* Context,
                            tPvInterface Interface,
                            tPvLinkEvent Event,
                            unsigned long UniqueId);
void _STDCALL F_CameraEventCallback(void*                   Context,
                                    tPvHandle               Camera,
                                    const tPvCameraEvent*	EventList,
                                    unsigned long			EventListLength);
