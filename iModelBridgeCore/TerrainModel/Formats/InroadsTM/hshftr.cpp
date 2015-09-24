//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* hshftr.c                                            twl    11-Dec-1998     */
/*----------------------------------------------------------------------------*/
/* Contains feature hash functions.				                 	                  */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"

static int aecDTM_hashFeatureGUID(const void *);
static int aecDTM_hashFeatureName(const void *);
static int aecDTM_compareFeatureNames(const void *, const void *);
static int aecDTM_compareFeatureGUIDs(const void *, const void *);
static int aecDTM_hashAllFeaturesProc(void *, struct CIVdtmsrf *, int, struct CIVdtmftr *);



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_hashFeatureCreate
 DESC: Creates BeGuid and name hash tables for surface features.
 HIST: Original - twl 11-Dec-1998
 MISC:
 KEYW: DTM CREATE FEATURE HASH TABLE
-----------------------------------------------------------------------------%*/

int aecDTM_hashFeatureCreate
(
  struct CIVdtmsrf *srfP
)
{
  int sts = SUCCESS;

  aecDTM_hashFeatureDestroy ( srfP );

  srfP->ftrGUIDMapP = new CMapStringToPtr;  
  srfP->ftrGUIDMapP->InitHashTable ( DTM_C_FTRHSHSIZ );

  srfP->ftrNameMapP = new CMapStringToPtr;  
  srfP->ftrNameMapP->InitHashTable ( DTM_C_FTRHSHSIZ );

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_hashAllFeatures
 DESC: Creates empty BeGuid and name hash tables for surface features and inserts
       surface features into the tables.
 HIST: Original - twl 11-Dec-1998
 MISC:
 KEYW: DTM ALL FEATURE HASH TABLE
-----------------------------------------------------------------------------%*/

int aecDTM_hashAllFeatures
(
  struct CIVdtmsrf *srfP
)
{
  int sts = SUCCESS;

  aecDTM_hashFeatureCreate ( srfP );
  aecDTM_sendAllFeatures ( (void *)NULL, srfP, 0, 0, aecDTM_hashAllFeaturesProc, (void *)srfP );

  return ( sts ); 
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_hashAllFeaturesProc
 DESC: Proc function for aecDTM_hashAllFeatures.
 HIST: Original - twl 11-Dec-1998
 MISC: static
 KEYW: DTM CREATE FEATURE HASH TABLE
-----------------------------------------------------------------------------%*/

static int aecDTM_hashAllFeaturesProc
(
  void *dat,
  struct CIVdtmsrf *,
  int,
  struct CIVdtmftr *ftrP
)
{
  struct CIVdtmsrf *srfP = ( struct CIVdtmsrf * ) dat;

  return ( aecDTM_hashInsertFeature ( srfP, ftrP ) );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_hashInsertFeature
 DESC: Hashes a single feature and inserts it in the BeGuid and name tables.
 HIST: Original - twl 11-Dec-1998
 MISC:
 KEYW: DTM INSERT FEATURE HASH TABLE
-----------------------------------------------------------------------------%*/

int aecDTM_hashInsertFeature
(
  struct CIVdtmsrf *srfP,
  struct CIVdtmftr *ftrP
)
{
  int sts = SUCCESS;

  wchar_t guidString[GUID_STRING_MAX_SIZE];
  aecGuid_toString ( guidString, &ftrP->guid );
  srfP->ftrGUIDMapP->SetAt ( guidString, ftrP );
  srfP->ftrNameMapP->SetAt ( ftrP->nam, ftrP );

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_hashDeleteFeature
 DESC: Hashes a single feature and deletes it from the BeGuid and name tables.
 HIST: Original - twl 8-Jan-1999
 MISC:
 KEYW: DTM DELETE FEATURE HASH TABLE
-----------------------------------------------------------------------------%*/

int aecDTM_hashDeleteFeature
(
  struct CIVdtmsrf *srfP,
  struct CIVdtmftr *ftrP
)
{
  int sts = SUCCESS;

  wchar_t guidString[GUID_STRING_MAX_SIZE];
  aecGuid_toString ( guidString, &ftrP->guid );
  srfP->ftrGUIDMapP->RemoveKey ( guidString );
  srfP->ftrNameMapP->RemoveKey ( ftrP->nam );

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_hashFindFeatureByGUID
 DESC: Finds a feature via the BeGuid hash table.
 HIST: Original - twl 11-Dec-1998
 MISC:
 KEYW: DTM FIND FEATURE BeGuid HASH TABLE
-----------------------------------------------------------------------------%*/

int aecDTM_hashFindFeatureByGUID
(
  CIVdtmftr **ftrPP,
  struct CIVdtmsrf *srfP,
  BeGuid *guidP
)
{
  int sts = SUCCESS;

  wchar_t guidString[GUID_STRING_MAX_SIZE];
  aecGuid_toString ( guidString, guidP );

  if ( !srfP->ftrGUIDMapP->Lookup ( guidString, (void *&)*ftrPP ) )
  {
        *ftrPP = NULL;
        sts = DTM_M_NOFTRF;
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_hashFindFeatureByName
 DESC: Finds a feature via the name hash table.
 HIST: Original - twl 11-Dec-1998
 MISC:
 KEYW: DTM FIND FEATURE NAME HASH TABLE
-----------------------------------------------------------------------------%*/

int aecDTM_hashFindFeatureByName
(
  CIVdtmftr **ftrPP,
  struct CIVdtmsrf *srfP,
  wchar_t *name
)
{
  int sts = DTM_M_NOFTRF;

  *ftrPP = NULL;

  if ( srfP && srfP->ftrNameMapP && srfP->ftrNameMapP->Lookup ( name, (void *&)*ftrPP ) )
  {
        sts = SUCCESS;
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_hashFeatureDestroy
 DESC: Removes all entries and destroys memory allocated in a surfaces BeGuid
       and name feature hash tables.
 HIST: Original - twl 11-Dec-1998
 MISC:
 KEYW: DTM DESTROY FEATURE HASH TABLE
-----------------------------------------------------------------------------%*/

void aecDTM_hashFeatureDestroy
(
  struct CIVdtmsrf *srfP
)
{
  if ( srfP->ftrGUIDMapP )
  {
    delete srfP->ftrGUIDMapP ;
    srfP->ftrGUIDMapP = NULL;
  }

  if ( srfP->ftrNameMapP )
  {
    delete srfP->ftrNameMapP;
    srfP->ftrNameMapP = NULL;
  }
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_hashFeatureGUID
 DESC: Hashes a BeGuid.
 HIST: Original - twl 11-Dec-1998
 MISC: static
 KEYW: DTM FEATURE HASH TABLE
-----------------------------------------------------------------------------%*/

static int aecDTM_hashFeatureGUID
(
  const void *p
)
{
  //return( ( (CIVdtmftr *)p )->guid.Data1 % DTM_C_FTRHSHSIZ );
    return(((CIVdtmftr *)p)->guid.m_guid.u[0] % DTM_C_FTRHSHSIZ);
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_hashFeatureName
 DESC: Hashes a feature name.
 HIST: Original - twl 11-Dec-1998
 MISC: static
 KEYW: DTM FEATURE HASH TABLE
-----------------------------------------------------------------------------%*/

static int aecDTM_hashFeatureName
(
   const void *p                   /* (i) */
)
{
  wchar_t wcs[ AEC_C_BUFLEN ];
  wchar_t *name = ((CIVdtmftr *)p)->nam;
  int sum = 0;

  memset( wcs, 0, sizeof( wcs ) );
  wcscpy ( wcs, name );

  size_t len = wcslen( wcs );

  for( size_t i = 0; i < len; i++ )
  {
    int factor = 1;

    switch( i % 3 )
    {
      case 0:
      factor = 1;
      break;

      case 1:
      factor = 10;
      break;

      case 2:
      factor = 100;
      break;
    }

    if( iswdigit( wcs[i] ) )
      sum += ( wcs[i] - TEXT('0') ) * factor;
    else
      sum += wcs[i];
  }

  return( sum % DTM_C_FTRHSHSIZ );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_compareFeatureGUIDs
 DESC: Compares the guids of two features.
 HIST: Original - twl 11-Dec-1998
 MISC: static
 KEYW: DTM COMPARE FEATURE HASH TABLE
-----------------------------------------------------------------------------%*/

static int aecDTM_compareFeatureGUIDs
(
  const void *p1,
  const void *p2
)
{
  CIVdtmftr *e1P = ( CIVdtmftr * )p1;
  CIVdtmftr *e2P = ( CIVdtmftr * )p2;

  return ( aecGuid_compare ( &e1P->guid, &e2P->guid ) );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_compareFeatureNames
 DESC: Compares the names of two features
 HIST: Original - twl 11-Dec-1998
 MISC: static
 KEYW: DTM COMPARE FEATURE HASH TABLE
-----------------------------------------------------------------------------%*/

static int aecDTM_compareFeatureNames
(
  const void *p1,
  const void *p2
)
{
  CIVdtmftr *e1P = ( CIVdtmftr * )p1;
  CIVdtmftr *e2P = ( CIVdtmftr * )p2;

  return ( wcscmp ( e1P->nam, e2P->nam ) );
}
