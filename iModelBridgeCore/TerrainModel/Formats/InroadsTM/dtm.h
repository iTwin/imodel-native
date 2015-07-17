//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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
#include "dtmdis.h"
#include "dtmedt.h"
#include "dtmflg.h"
#include "dtmfnd.h"
#include "dtmftr.h"
#include "dtmio.h"
#include "dtmmac.h"
#include "dtmmem.h"
#include "dtmuti.h"
#include "dtmvol.h"

int aecDTM_projectSurfaceInitialize (void);
