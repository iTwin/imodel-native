//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+

#include "stdafx.h"

/*----------------------------------------------------------------------------*/
/* Constants and macros                                                       */
/*----------------------------------------------------------------------------*/

#define PARlock_feature          0x400

/*----------------------------------------------------------------------------*/
/* Data structures                                                            */
/*----------------------------------------------------------------------------*/

struct PARprod { wchar_t name[CIV_MAX_FNAME]; wchar_t appNam[CIV_MAX_FNAME]; long id; };
struct PARfmtpre { long format, precision; };
struct PARcoordinate { long precisionX, precisionY, precisionZ; };
struct PARlocks { long lock; };

static struct PARprod PARproduct;
static struct PARlocks  PARlock;
static struct PARcoordinate PARcoord;
static struct PARfmtpre PARstatus;

union
    {
    byte b;
    struct
        {
#if defined (BITFIELDS_REVERSED)
        unsigned fract:5;
        unsigned dec:3;
#else
        unsigned dec:3;
        unsigned fract:5;
#endif
        } u;
    } adres2;

/*%-----------------------------------------------------------------------------
FUNC: aecParams_getFeatureState
DESC: It gets the Feature lock state.
HIST: Original - mohan 01-May-1993
MISC:
KEYW: PARAMETERS FEATURE STATE GET
-----------------------------------------------------------------------------%*/

int aecParams_getFeatureState /* <= feature state                  */
    (
    int *state                           /* <= state: 0 or 1 (or NULL)          */
    )
    {
    int tmp;
    tmp = ( PARlock.lock & PARlock_feature ) ? TRUE : FALSE;
    if ( state != (int *)0 ) *state = tmp;
    return ( tmp );
    }


/*%-----------------------------------------------------------------------------
FUNC: aecParams_setStatusState
DESC: It sets the status lock state.
This is not set on the GUI anymore.  But we need to disable it internally
for the pad commands (and others for performance) mw
HIST: Original - tmi 05-Jan-1993
MISC:
KEYW: PARAMETERS STATUS STATE SET
-----------------------------------------------------------------------------%*/

void aecParams_setStatusState
    (
    int *state  // => state: 0, 1
    )
    {
    if ( state != (int *)0 )
        PARstatus.format = *state;
    }

/*%-----------------------------------------------------------------------------
FUNC: aecParams_getStatusState
DESC: It gets the status lock state.
HIST: Original - tmi 05-Jan-1993
MISC:
KEYW: PARAMETERS STATUS STATE GET
-----------------------------------------------------------------------------%*/

int aecParams_getStatusState /* <= state: 0 or 1                   */
    (
    int *state  // <= state: 0 or 1
    )
    {
    if ( state != (int *)0 )
        *state = PARstatus.format;

    return( PARstatus.format );
    }

/*%-----------------------------------------------------------------------------
FUNC: aecParams_getFileExtension 
DESC: returns file extensions 
HIST: Original                                       mohan    12/1992
MISC:
KEYW: PARAMETERS FILE EXTENSION GET
----------------------------------------------------------------------------%*/

LPWSTR aecParams_getFileExtension /* <= file extension              */
    (
    int  ext                             /* => file extension to get            */
    )
    {
    static wchar_t extension[CIV_MAX_EXT];
    memset(extension, '\0', sizeof( extension ));
    wcscpy( extension, L"*.dtm" );                                                 /* DO_NOT_TRANSLATE */
    return( extension );
    }


 
/*%-----------------------------------------------------------------------------
FUNC: aecParams_getLinearUnits
DESC: Gets the Linear Units
HIST: Original - tmi 01-Mar-1993
MISC:
KEYW: PARAMETERS LINEAR UNITS GET
-----------------------------------------------------------------------------%*/
void aecParams_getLinearUnits
    (
    int *unitsP,                         /* => unitsP (or NULL)                 */
    int *precisionP                      /* => precision (or NULL)              */
    )
    {
    if ( unitsP != (int *)0 )
        {
#ifdef DHTODO
        //UnitInfo masterUnitInfo;
        //mdlModelRef_getMasterUnit(ACTIVEMODEL, &masterUnitInfo);
        //if (UnitSystem::Metric == mdlUnits_getUnitSystem(&masterUnitInfo))
#endif
        *unitsP = 1;
#ifdef DHTODO
        //else
        //    *unitsP = 0;
#endif
        }
    if ( precisionP != (int *)0 )
        {
        *precisionP = 4;
        //if( tcb ) 
        //{
        //	adres2.b = (UChar)(tcb->ad1.format.adres2);

        //	if (tcb->ad1.format.ref_decfract)
        //	{
        //		/* Make a valid denominator for the fractional accuracy. 0 means a whole number. */
        //		if (ACCURACYFLAGS_FractionalZero == tcb->ad1.format.accuracyFlags)
        //			*precisionP =  1;
        //		else
        //			*precisionP = (adres2.u.fract+1)*2;
        //	}
        //	else if (ACCURACYFLAGS_Scientific == tcb->ad1.format.accuracyFlags)
        //	{
        //		*precisionP = tcb->ad1.format.adres2;
        //	}
        //	else
        //	{
        //		if (adres2.u.dec)
        //			*precisionP = adres2.u.dec -1;
        //		else
        //			*precisionP = 4;
        //	}
        //}
        }
    }



/*%-----------------------------------------------------------------------------
FUNC: aecParams_getX
DESC: It returns the X precision.
HIST: Original - tmi 05-Jan-1993
MISC:
KEYW: PARAMETERS COORDINATE X EASTING PRECISION GET
-----------------------------------------------------------------------------%*/

int aecParams_getX          /* <= precision                        */
    (
    int *precisionX                      /* <= precision                        */
    )
    {
    if ( precisionX != (int *)0 ) *precisionX = PARcoord.precisionX;
    return ( PARcoord.precisionX );
    }



/*%-----------------------------------------------------------------------------
FUNC: aecParams_getZ
DESC: It returns the Z precision.
HIST: Original - tmi 05-Jan-1993
MISC:
KEYW: PARAMETERS COORDINATE Z ELEVATION PRECISION GET
-----------------------------------------------------------------------------%*/

int aecParams_getZ          /* <= precision                        */
    (
    int *precisionZ                      /* <= precision                        */
    )
    {
    if ( precisionZ != (int *)0 ) *precisionZ = PARcoord.precisionZ;
    return ( PARcoord.precisionZ );
    }


/*%-----------------------------------------------------------------------------
FUNC: aecParams_getProductName
DESC: Use this one to get the string and id identifying a product.
HIST: Original - tmi 09-Jan-1993
HIST: Added suite name - dkh 10-7-99
MISC:
KEYW: PARAMETERS PRODUCT NAME GET
-----------------------------------------------------------------------------%*/

void aecParams_getProductNameAndID
    (
    LPWSTR nameP,                         /* => product name                     */
    LPWSTR appNameP,                      /* => application name                 */
    int  *idP                            /* => product id                       */
    )
    {
    if ( appNameP != (LPWSTR )0 )
        wcscpy ( appNameP, PARproduct.appNam );

    if ( nameP != (LPWSTR )0 )
        wcscpy ( nameP, PARproduct.name );

    if ( idP != (int *)0 )        // NO LONGER APPLIES TO THIS PRODUCT
        *idP = PARproduct.id;

    }

