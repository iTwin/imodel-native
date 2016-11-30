//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* srfuti.c                                            tmi    10-Apr-1990     */
/*----------------------------------------------------------------------------*/
/* Contains various surface utilities.					      */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"
#include "TimeStampUtils.h"

static int aecDTM_removeChildlessFtrsProc ( void *, struct CIVdtmsrf *, int, struct CIVdtmftr * );
//static int aecDTM_updateCorridorCtrlGuidsProc (void*,struct CIVdtmsrf*,struct CIVdtmcor*);
//static int aecDTM_assignNewComponentGuidsProc (void*,struct CIVdtmsrf*,struct CIVdtmcmp*);
//static int aecDTM_updateMemberParentGuidsProc (void*,struct CIVdtmsrf*,struct CIVdtmcmpmem*);
//static int aecDTM_updateMemberGuidsProc (void*,struct CIVdtmsrf*,struct CIVdtmcmpmem*);
//static int aecDTM_assignNewFeatureGuidsProc (void*,struct CIVdtmsrf*,int,struct CIVdtmftr*);



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getSurfaceFileIndex
 DESC: Given a point file type this function returns the index into the
       surface point file array.
 HIST: Original - tmi 10-Apr-1990
 MISC:
 KEYW: DTM FILE INDEX GET
-----------------------------------------------------------------------------%*/

void aecDTM_getSurfaceFileIndex
(
  long *indexP,                       /* <= file index                        */
  long pointType                      /* => point type                        */
)
{
  if ( indexP != (long *)0 )
    switch ( pointType )
    {
      case ( DTM_C_DTMBRK )    :  *indexP = 1;   break;
      case ( DTM_C_DTMINT )    :  *indexP = 2;   break;
      case ( DTM_C_DTMEXT )    :  *indexP = 3;   break;
      case ( DTM_C_DTMCTR )    :  *indexP = 4;   break;
      case ( DTM_C_DTMINF )    :  *indexP = 5;   break;
      case ( DTM_C_DTMRNG )    :  *indexP = 6;   break;
      default :                *indexP = 0;  break;
    }
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getSurfaceFeatureFileIndex
 DESC: Given a point feature file type this function returns the index into the
       surface feature file array.
 HIST: Original - twl 10-Nov-1998/
 MISC:
 KEYW: DTM FEATURE FILE INDEX GET
-----------------------------------------------------------------------------%*/

void aecDTM_getSurfaceFeatureFileIndex
(
  long *indexP,                       /* <= file index                        */
  long featureType                    /* => feature type                      */
)
{
  if ( indexP != (long *)0 )
    switch ( featureType )
    {
      case ( DTM_C_DTMBRKFTR ) :  *indexP = 1;  break;
      case ( DTM_C_DTMINTFTR ) :  *indexP = 2;  break;
      case ( DTM_C_DTMEXTFTR ) :  *indexP = 3;  break;
      case ( DTM_C_DTMCTRFTR ) :  *indexP = 4;  break;
      default :                *indexP = 0;  break;
    }
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_featureFileFromPointFile
 DESC: Given a surface point file type that contains feature points, returns the
       corresponding feature file type
 HIST: Original - twl 10-Nov-1998
 MISC:
 KEYW: DTM FEATURE POINT FILE TYPE
-----------------------------------------------------------------------------%*/

int aecDTM_featureFileFromPointFile
(
  long fileType
)
{
    int ftrFileType;

    switch ( fileType )
    {
      case ( DTM_C_DTMREG )    :
        ftrFileType = DTM_C_DTMREGFTR;
        break;
      case ( DTM_C_DTMBRK )    :
        ftrFileType = DTM_C_DTMBRKFTR;
        break;
      case ( DTM_C_DTMINT )    :
        ftrFileType = DTM_C_DTMINTFTR;
        break;

      case ( DTM_C_DTMEXT )    :
        ftrFileType = DTM_C_DTMEXTFTR;
        break;

      case ( DTM_C_DTMCTR )    :
        ftrFileType = DTM_C_DTMCTRFTR;
        break;

      default:
        ftrFileType = -1;
        break;
    }

    return ( ftrFileType );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_pointFileFromFeatureFile
 DESC: Given a surface feature file type, returns the corresponding point file
       type.
 HIST: Original - twl 10-Nov-1998
 MISC:
 KEYW: DTM FEATURE POINT FILE TYPE
-----------------------------------------------------------------------------%*/

int aecDTM_pointFileFromFeatureFile
(
  long fileType
)
{
    int pntFileType;

    switch ( fileType )
    {
      case ( DTM_C_DTMREGFTR )    :
        pntFileType = DTM_C_DTMREG;
        break;
      case ( DTM_C_DTMBRKFTR )    :
        pntFileType = DTM_C_DTMBRK;
        break;
      case ( DTM_C_DTMINTFTR )    :
        pntFileType = DTM_C_DTMINT;
        break;

      case ( DTM_C_DTMEXTFTR )    :
        pntFileType = DTM_C_DTMEXT;
        break;

      case ( DTM_C_DTMCTRFTR )    :
        pntFileType = DTM_C_DTMCTR;
        break;

      default:
        pntFileType = -1;
        break;
    }

    return ( pntFileType );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_generateUniqueSurfaceName
 DESC: Generates a name which does not conflict with any existing surface name.
 HIST: Original - tmi 05-Mar-1991
 MISC:
 KEYW: DTM NAME GENERATE UNIQUE
-----------------------------------------------------------------------------%*/

void aecDTM_generateUniqueSurfaceName
(
  wchar_t *newNameP,                 /* <= unique name                        */
  wchar_t *oldNameP,                 /* => name to check                      */
  struct CIVdtmprj *prjP             /* => DTM project (or NULL)              */
)
{
  if ( newNameP != (wchar_t *)0 )
  {
    newNameP[0] = '\0';

    if ( oldNameP != (wchar_t *)0 )
    {
      int cnt = 1;

      wcscpy ( newNameP, oldNameP );

      do
      {
        if ( aecDTM_findSurfaceByName ( (struct CIVdtmsrf **)0, prjP, newNameP ) == SUCCESS )
          swprintf ( newNameP, L"%.*s%d", DTM_C_NAMSIZ-3, oldNameP, cnt++ );     /* DO_NOT_TRANSLATE */
        else
          cnt = 0;
      } while ( cnt );
    }
  }
}


/*----------------------------------------------------------------------*/
/* aecDTM_removeChildlessFtrs     twl    09-Aug-2001                    */
/*----------------------------------------------------------------------*/
/* Gets rid of features whose points have all been deleted              */
/*----------------------------------------------------------------------*/

int aecDTM_removeChildlessFtrs
(
  struct CIVdtmsrf *srfP
)
{
    aecDTM_sendAllFeatures ( (void *)0, srfP, 0, 0, aecDTM_removeChildlessFtrsProc, srfP );
    return ( SUCCESS );
}


static int aecDTM_removeChildlessFtrsProc
(
  void *dat,
  struct CIVdtmsrf *,
  int,
  struct CIVdtmftr *ftrP
)
{
  struct CIVdtmsrf *srfP = ( struct CIVdtmsrf * ) dat;
  unsigned long numPnts = 0;
  int i;

  if ( !aecDTM_isFeatureDeletedFlagSet ( ftrP ) && ftrP->numPnts > 0 )
  {
    for ( i = 0; i < ftrP->numPnts; i++ )
    {
      if ( !aecDTM_isPointDeletedFlagSet ( &ftrP->p1[i] ) )
        numPnts++;
    }
  }

  if ( !numPnts )
    aecDTM_deleteFeature ( srfP, NULL, NULL, ftrP );

  return ( SUCCESS );
}

  #ifdef NOTUSED
  static int aecDTM_updateCorridorCtrlGuidsProc
(
  void *dat,
  struct CIVdtmsrf *srfP,
  struct CIVdtmcor *corP
)
{
    CMapStringToString *ftrGuidMapP = (CMapStringToString *)dat;
    wchar_t oldGuid[GUID_STRING_MAX_SIZE];

    if ( corP->ctrlGuidType == DTM_C_CORCTRLFTR )
    {
        CString strNewGuid;

        aecGuid_toString ( oldGuid, &corP->ctrlGuid2 );

        if ( ftrGuidMapP->Lookup ( oldGuid, strNewGuid ) && !strNewGuid.IsEmpty ( ) )
        {
            aecGuid_fromString ( &corP->ctrlGuid2, (LPWSTR)(LPCWSTR)strNewGuid );
            aecGuid_copy ( &corP->ctrlGuid1, &srfP->guid );
        }
    }

    return SUCCESS;
}

static int aecDTM_assignNewComponentGuidsProc
(
  void *dat,
  struct CIVdtmsrf *srP,
  struct CIVdtmcmp *cmpP
)
{
    CMapStringToString **maps = (CMapStringToString **)dat;
    CMapStringToString *corGuidMapP = maps[0];
    CMapStringToString *cmpGuidMapP = maps[1];
    wchar_t oldGuid[GUID_STRING_MAX_SIZE];
    wchar_t newGuid[GUID_STRING_MAX_SIZE];
    CString strNewGuid;

    aecGuid_toString ( oldGuid, &cmpP->guid );
    aecGuid_generate ( &cmpP->guid );
    aecGuid_toString ( newGuid, &cmpP->guid );  

    cmpGuidMapP->SetAt ( oldGuid, newGuid );

    aecGuid_toString ( oldGuid, &cmpP->corGuid );

    if ( corGuidMapP->Lookup ( oldGuid, strNewGuid ) )
        aecGuid_fromString ( &cmpP->corGuid, (LPWSTR)(LPCWSTR)strNewGuid );

    return SUCCESS;
}

static int aecDTM_updateMemberParentGuidsProc
(
  void *dat,
  struct CIVdtmsrf *srP,
  struct CIVdtmcmpmem *cmpMemP
)
{
    CMapStringToString **maps = (CMapStringToString **)dat;
    CMapStringToString *cmpGuidMapP = maps[0];
    CMapStringToString *ftrGuidMapP = maps[1];
    wchar_t oldGuid[GUID_STRING_MAX_SIZE];
    CString strNewGuid;

    aecGuid_toString ( oldGuid, &cmpMemP->cmpGuid );

    if ( cmpGuidMapP->Lookup ( oldGuid, strNewGuid ) )
        aecGuid_fromString ( &cmpMemP->cmpGuid, (LPWSTR)(LPCWSTR)strNewGuid );

    aecGuid_toString ( oldGuid, &cmpMemP->cmpMemGuid );
    ftrGuidMapP->SetAt ( oldGuid, L"" );

    return SUCCESS;
}

static int aecDTM_updateMemberGuidsProc
(
  void *dat,
  struct CIVdtmsrf *srP,
  struct CIVdtmcmpmem *cmpMemP
)
{
    CMapStringToString *ftrGuidMapP = (CMapStringToString *)dat;
    wchar_t oldGuid[GUID_STRING_MAX_SIZE];
    CString strNewGuid;

    aecGuid_toString ( oldGuid, &cmpMemP->cmpMemGuid );

    if ( ftrGuidMapP->Lookup ( oldGuid, strNewGuid ) )
        aecGuid_fromString ( &cmpMemP->cmpMemGuid, (LPWSTR)(LPCWSTR)strNewGuid );

    return SUCCESS;
}

static int aecDTM_assignNewFeatureGuidsProc
(
  void *dat,
  struct CIVdtmsrf *,
  int ftrTyp,
  struct CIVdtmftr *ftrP
)
{
    CMapStringToString *guidMapP = (CMapStringToString *)dat;
    wchar_t oldGuid[GUID_STRING_MAX_SIZE];
    CString strValue;

    aecGuid_toString ( oldGuid, &ftrP->guid );
    aecGuid_generate ( &ftrP->guid );
    
    if ( guidMapP->Lookup ( oldGuid, strValue ) )
    {
        wchar_t newGuid[GUID_STRING_MAX_SIZE];
        aecGuid_toString ( newGuid, &ftrP->guid );
        guidMapP->SetAt ( oldGuid, newGuid );
    }

    return SUCCESS;
}

#endif