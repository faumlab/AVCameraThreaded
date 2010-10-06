/*!
 *  @file
 *     Utility.h
 *  @brief
 *     OTC project: This file contains functions and data structures declaration
 *	   for some common utilities for all other files
 *  @author
 *   Main contributors (see contributors.h for copyright, address and affiliation details)
 *   - Waazim Reza               <wreza@fau.edu>
 ***********************************************************************
 */

/*
	Include all the required header files here :
	Check for windows or Linux compatibility also
*/
#include "stdio.h"
#include "PvApi.h"

void convertandPrintErrorCode(tPvErr errorCode);