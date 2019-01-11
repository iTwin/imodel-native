//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* dtm.h                                                aec    08-Feb-1994    */
/*----------------------------------------------------------------------------*/
/* All data structures and constants used by DTM code.                        */
/*----------------------------------------------------------------------------*/

#pragma once

#if defined (_CIVDTM)
#define CIVDTM_EXPORT extern "C"__declspec(dllexport)
#else
#define CIVDTM_EXPORT extern "C"__declspec(dllimport)
#endif

#include "dtmstr.h"
#include "dtmcon.h"
#include "dtmedt.h"
#include "dtmflg.h"
#include "dtmio.h"
#include "dtmmac.h"
#include "dtmmem.h"
#include "msgfnc.h"

int aecDTM_projectSurfaceInitialize (void);
