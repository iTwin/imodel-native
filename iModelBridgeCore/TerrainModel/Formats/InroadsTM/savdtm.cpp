//---------------------------------------------------------------------------+
// $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* savdtm.c                                            tmi    24-Apr-1990     */
/*----------------------------------------------------------------------------*/
/* Saves a surface from memory to a .DTM file.				      */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"
#include "timestamputils.h"
#include "filfnc.h"


/*----------------------------------------------------------------------------*/
/* Constants and macros                                                       */
/*----------------------------------------------------------------------------*/
#define     CIVSTATUS(a,b,c,d,e,f,g,h,i,j) { double zzz = (double)((a+=b)+(d+=e)+(g+=h))/(double)(c+f+i) * 100.; aecStatus_show ( zzz, j ); }


/*----------------------------------------------------------------------------*/
/* Externals                                                                  */
/*----------------------------------------------------------------------------*/
static long npnt, cpnt, ntin, ctin, nftr, cftr;


/*----------------------------------------------------------------------------*/
/* Private static function prototypes                                         */
/*----------------------------------------------------------------------------*/
static int aecDTM_saveFileHistory( struct CIVdtmsrf *srfP );
static int aecDTM_saveOpen(wchar_t *,unsigned long,unsigned long,FILE **);
static int aecDTM_saveGUID(struct CIVdtmsrf *, FILE *);
static int aecDTM_saveName(struct CIVdtmsrf *,FILE *);
static int aecDTM_savePreferenceName(struct CIVdtmsrf *, FILE *);
static int aecDTM_saveXSectionSymbologyName(struct CIVdtmsrf *, FILE *);
static int aecDTM_saveProfileSymbologyName(struct CIVdtmsrf *, FILE *);
static int aecDTM_saveProfileOffsets(struct CIVdtmsrf *, FILE *);
static int aecDTM_saveRevision(struct CIVdtmsrf *, FILE *);
static int aecDTM_saveParameters(struct CIVdtmsrf *,FILE *);
static int aecDTM_saveFiles(struct CIVdtmsrf *,FILE *);
static int aecDTM_savePoints(struct CIVdtmsrf *,FILE *);
static int aecDTM_saveRangePoints(struct CIVdtmsrf *,FILE *);
static int aecDTM_saveTriangles(struct CIVdtmsrf *,FILE *,struct CIVptrind *,struct CIVptrind *);
static int aecDTM_saveFeatures(struct CIVdtmsrf *,FILE *,struct CIVptrind *,struct CIVptrind *);
static int aecDTM_saveFeatureStyles(struct CIVdtmsrf *,FILE *);
static int aecDTM_saveFeaturePayItems(struct CIVdtmsrf *,FILE *);
static int aecDTM_saveCorridors(struct CIVdtmsrf *,FILE *);
static int aecDTM_saveComponents(struct CIVdtmsrf *,FILE *);
static int aecDTM_saveComponentMembers(struct CIVdtmsrf *,FILE *);




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_save
 DESC: Saves a surface to a .DTM file.
 HIST: Original - tmi 24-Apr-1990
 MISC:
 KEYW: DTM SAVE
-----------------------------------------------------------------------------%*/


int aecDTM_save            /* <= TRUE if error                     */
(
  struct CIVdtmsrf *srfP,             /* => surface to save                   */
  wchar_t *fileNameP,                 /* => file name to save to              */
  unsigned long version               /* => version to save (0 = latest)      */
)
{
  int sts = SUCCESS;

  if ( srfP != (struct CIVdtmsrf *)0 )
  {
    FILE *handleP;
    long srfVersion = srfP->version;

    if ( version > 0  &&  version < DTM_C_DTMVER+1 )
      srfP->version = version;

    if ( fileNameP[0] == '\0' )
      sts = AEC_E_NOFILE_NAME;

    if (sts == SUCCESS)
    {
      aecTicker_initialize();

      if ( ( sts = aecDTM_saveOpen ( fileNameP, srfP->version, srfP->codePage, &handleP ) ) == SUCCESS )
      {
        aecDTM_countSurfacePoints ( &npnt, srfP );
        aecDTM_countSurfaceFeatures ( &nftr, srfP );
        ntin = srfP->tinf->nrec;
        cpnt = ctin = cftr = 0L;

        wcscpy ( srfP->pth, fileNameP );

        if ( npnt > 0L ) aecStatus_initialize(TRUE);

        if ( ( sts = aecDTM_saveGUID ( srfP, handleP ) ) == SUCCESS )
          if ( ( sts = aecDTM_saveName ( srfP, handleP ) ) == SUCCESS )
            if ( ( sts = aecDTM_savePreferenceName ( srfP, handleP ) ) == SUCCESS )
              if ( ( sts = aecDTM_saveXSectionSymbologyName ( srfP, handleP ) ) == SUCCESS )
                if ( ( sts = aecDTM_saveProfileSymbologyName ( srfP, handleP ) ) == SUCCESS )
                  if ( ( sts = aecDTM_saveRevision ( srfP, handleP ) ) == SUCCESS )
                    if ( ( sts = aecDTM_saveProfileOffsets ( srfP, handleP ) ) == SUCCESS )
                      if ( ( sts = aecDTM_saveParameters ( srfP, handleP ) ) == SUCCESS )
                        if ( ( sts = aecDTM_saveFiles ( srfP, handleP ) ) == SUCCESS )
                          if ( ( sts = aecDTM_savePoints ( srfP, handleP ) ) == SUCCESS )
                            if ( ( sts = aecDTM_saveRangePoints ( srfP, handleP ) ) == SUCCESS )
                            {
                              struct CIVptrind *piP, *epiP;
                              int nblk;

                              aecDTM_countSurfaceBlocks ( &nblk, srfP );
                              if ( nblk > 0 )
                              {
                                if ( ( piP = (struct CIVptrind *) calloc ( (unsigned int)nblk, sizeof(struct CIVptrind) ) ) == 0L )
                                  sts = DTM_M_MEMALF;
                                else
                                {
                                  epiP = piP + nblk - 1;
                                  aecDTM_buildPtrIndexTable ( srfP, piP, NULL );
                                  aecDTM_sortPtrIndexTableByPtr ( piP, epiP );

                                  if ( srfP->tinf->blk )
                                    sts = aecDTM_saveTriangles ( srfP, handleP, piP, epiP );

                                  if ( sts == SUCCESS )
                                    if ( ( sts = aecDTM_saveFeatures ( srfP, handleP, piP, epiP ) ) == SUCCESS )
                                      if ( (sts = aecDTM_saveFeatureStyles ( srfP, handleP ) ) == SUCCESS )
                                        sts = aecDTM_saveFeaturePayItems ( srfP, handleP );

                                  if ( sts == SUCCESS )
                                    if ( ( sts = aecDTM_saveCorridors ( srfP, handleP ) ) == SUCCESS )
                                      if ( ( sts = aecDTM_saveComponents ( srfP, handleP ) ) == SUCCESS )
                                        sts = aecDTM_saveComponentMembers ( srfP, handleP );

                                  free ( (void *)piP );
                                }
                              }
                            }
        aecStatus_close();
      }

      if ( handleP ) fclose ( handleP );

      if ( sts != SUCCESS ) aecOutput_setMessage ( sts, fileNameP );

      if ( sts == SUCCESS )
        aecDTM_saveFileHistory ( srfP );

      aecDTM_clearSurfaceModifiedFlag ( srfP );

      aecTicker_stop();

      srfP->version = srfVersion;
    }
  }

  return ( sts );
}


static int aecDTM_saveFileHistory( struct CIVdtmsrf *srfP )
{
    return( SUCCESS );
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveOpen
 DESC: Opens up the .DTM file for binary write access.
 HIST: Original - tmi 24-Apr-1990
 MISC: static
 KEYW: DTM SAVE OPEN
-----------------------------------------------------------------------------%*/

static int aecDTM_saveOpen
(
  wchar_t *fil,
  unsigned long version,
  unsigned long codePage,
  FILE **handlePP
)
{
  unsigned char buf[3]; // Do not convert to unicode.
  int sts = SUCCESS;

  aecFile_checkExtension ( fil, aecParams_getFileExtension(DTMEXT) );
  _mbsncpy ( buf, (unsigned char *)DTM_C_DTMCOD, 3 ); // Do not convert to unicode.

  if ( ( *handlePP = _wfopen ( fil, L"w+b" ) ) == (FILE *)0 )                     /* DO_NOT_TRANSLATE */
  {
        DWORD props = GetFileAttributes(fil);
	    DWORD chk = (FILE_ATTRIBUTE_READONLY & props);
	    if(chk == FILE_ATTRIBUTE_READONLY) // That means it is read-only.
            return (sts = DTM_M_FILREADONLY);
        else
            return ( sts = DTM_M_OPFILF );
  }
  else if ( fwrite ( buf, sizeof(buf), 1, *handlePP ) != 1 )
    sts = DTM_M_WRFILF;
  else if ( fwrite ( &version, sizeof(unsigned long), 1, *handlePP ) != 1 )
    sts = DTM_M_WRFILF;
  else
  {
    if ( version > 4 )
      if ( fwrite ( &codePage, sizeof(unsigned long), 1, *handlePP ) != 1 )
        sts = DTM_M_WRFILF;
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveGUID
 DESC: Saves the surface BeGuid to the .DTM file.
 HIST: Original - twl 23-Oct-1998
 MISC: static
 KEYW: DTM SAVE BeGuid
-----------------------------------------------------------------------------%*/

static int aecDTM_saveGUID
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  int sts = SUCCESS;

  if ( srf->version > 5 )
  {
    if ( fwrite ( &srf->guid, sizeof(srf->guid), 1, handleP ) != 1 )
      sts = DTM_M_WRFILF;
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveName
 DESC: Saves the name and description section of the .DTM file.
 HIST: Original - tmi 24-Apr-1990
 MISC: static
 KEYW: DTM SAVE NAME
-----------------------------------------------------------------------------%*/

static int aecDTM_saveName
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  int sts = SUCCESS;

  if ( srf->version > 8 )
  {
    if ( fwrite ( srf->des, sizeof(wchar_t), DTM_C_NAMSIZ, handleP ) != DTM_C_NAMSIZ )
      sts = DTM_M_WRFILF;
    else if ( fwrite ( srf->nam, sizeof(wchar_t), DTM_C_NAMSIZ, handleP ) != DTM_C_NAMSIZ )
      sts = DTM_M_WRFILF;
    else if ( srf->version > 2 )
      if ( fwrite ( srf->mat, sizeof(wchar_t), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
        sts = DTM_M_WRFILF;
  }
  else
  {
    char nam[CIV_C_NAMSIZ]; // Do not convert to wide.
    char des[CIV_C_DESSIZ]; // Do not convert to wide.
    char mat[CIV_C_NAMSIZ]; // Do not convert to wide.

    memset ( nam, 0, sizeof ( nam ) );
    memset ( des, 0, sizeof ( des ) );
    memset ( mat, 0, sizeof ( mat ) );

    wcstombs ( nam, srf->nam, CIV_C_NAMSIZ-1 );
    wcstombs ( des, srf->des, CIV_C_DESSIZ-1 );
    wcstombs ( mat, srf->mat, CIV_C_NAMSIZ-1 );

    if ( fwrite ( des, sizeof(char), CIV_C_DESSIZ, handleP ) != CIV_C_DESSIZ )
      sts = DTM_M_WRFILF;
    else if ( fwrite ( nam, sizeof(char), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
      sts = DTM_M_WRFILF;
    else if ( srf->version > 2 )
      if ( fwrite ( mat, sizeof(char), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
        sts = DTM_M_WRFILF;
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_savePreferenceName
 DESC: Saves the surface preference name to the .DTM file.
 HIST: Original - twl 23-Oct-1998
 MISC: static
 KEYW: DTM SAVE PREFERENCE NAME
-----------------------------------------------------------------------------%*/

static int aecDTM_savePreferenceName
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  int sts = SUCCESS;

  if ( srf->version > 5 )
  {
    if ( srf->version > 8 )
    {
      if ( fwrite ( srf->pref, sizeof(wchar_t), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
        sts = DTM_M_WRFILF;
    }
    else
    {
      char pref[CIV_C_NAMSIZ];  // Do not convert to wide.

      memset ( pref, 0, sizeof ( pref ) );
      wcstombs ( pref, srf->pref, CIV_C_NAMSIZ );

      if ( fwrite ( pref, sizeof(char), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
        sts = DTM_M_WRFILF;
    }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: XSectionSymbologyName
 DESC: Saves the surface symbology name to be used in cross sections
 HIST: Original - twl 13-Jul-1999
 MISC: static
 KEYW: DTM SAVE SYMBOLOGY NAME
-----------------------------------------------------------------------------%*/

static int aecDTM_saveXSectionSymbologyName
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  int sts = SUCCESS;

  if ( srf->version > 5 )
  {
    if ( srf->version > 8 )
    {
      if ( fwrite ( srf->secsym, sizeof(wchar_t), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
        sts = DTM_M_WRFILF;
    }
    else
    {
      char secsym[CIV_C_NAMSIZ];

      memset ( secsym, 0, sizeof ( secsym ) );
      wcstombs ( secsym, srf->secsym, CIV_C_NAMSIZ );

      if ( fwrite ( secsym, sizeof(char), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
        sts = DTM_M_WRFILF;
    }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveProfileSymbologyName
 DESC: Saves the profile symbology name to the .DTM file.
 HIST: Original - twl 27-Jun-2000
 MISC: static
 KEYW: DTM SAVE PROFILE SYMBOLOGY NAME
-----------------------------------------------------------------------------%*/

static int aecDTM_saveProfileSymbologyName
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  int sts = SUCCESS;

  if ( srf->version > 6 )
  {
    if ( srf->version > 8 )
    {
      if ( fwrite ( srf->prfsym, sizeof(wchar_t), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
        sts = DTM_M_WRFILF;
    }
    else
    {
      char prfsym[CIV_C_NAMSIZ];

      memset ( prfsym, 0, sizeof ( prfsym ) );
      wcstombs ( prfsym, srf->prfsym, CIV_C_NAMSIZ );

      if ( fwrite (prfsym, sizeof(char), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
        sts = DTM_M_WRFILF;
    }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveProfileOffsets
 DESC: Saves the profile offset properites.
 HIST: Original - twl 27-Jun-2000
 MISC: static
 KEYW: DTM SAVE PROFILE OFFSETS
-----------------------------------------------------------------------------%*/

static int aecDTM_saveProfileOffsets
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  int sts = SUCCESS;

  if ( srf->version > 6 )
  {
    if ( srf->version > 8 )
    {
      if ( fwrite ( srf->prfoff, sizeof(struct CIVdtmoff), DTM_C_NUMPRFOFF, handleP ) != DTM_C_NUMPRFOFF )
        sts = DTM_M_WRFILF;
    }
    else
    {
      struct CIVdtmoffV8 prfoffV8[DTM_C_NUMPRFOFF];

      memset ( prfoffV8, 0, sizeof ( prfoffV8 ) );

      for ( int i = 0; i < DTM_C_NUMPRFOFF; i++ )
      {
        prfoffV8[i].dis = srf->prfoff[i].dis;
        wcstombs ( prfoffV8[i].sym, srf->prfoff[i].sym, CIV_C_NAMSIZ );
      }

      if ( fwrite ( prfoffV8, sizeof(struct CIVdtmoffV8), DTM_C_NUMPRFOFF, handleP ) != DTM_C_NUMPRFOFF )
        sts = DTM_M_WRFILF;
    }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveRevision
 DESC: Saves the surface last revision aurthor and date to the .DTM file.
 HIST: Original - twl 23-Oct-1998
 MISC: static
 KEYW: DTM SAVE LAST REVISION
-----------------------------------------------------------------------------%*/

static int aecDTM_saveRevision
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  int sts = SUCCESS;

  if ( srf->version > 5 )
  {
    if ( srf->version > 8 )
    {
      if ( fwrite ( srf->revby, sizeof(wchar_t), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
        sts = DTM_M_WRFILF;
    }
    else
    {
      char revby[CIV_C_NAMSIZ]; // Do not convert to wide.

      memset ( revby, 0, sizeof ( revby ) );
      wcstombs ( revby, srf->revby, CIV_C_NAMSIZ );

      if ( fwrite ( revby, sizeof(char), CIV_C_NAMSIZ, handleP ) != CIV_C_NAMSIZ )
        sts = DTM_M_WRFILF;
    }

    if ( sts == SUCCESS )
    {
      if ( fwrite ( &srf->revdate, sizeof(SYSTEMTIME), 1, handleP ) != 1 )
        sts = DTM_M_WRFILF;
    }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveParameters
 DESC: Saves the parameters section of the .DTM file.
 HIST: Original - tmi 24-Apr-1990
 MISC: static
 KEYW: DTM SAVE PARAMETERS
-----------------------------------------------------------------------------%*/

static int aecDTM_saveParameters
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  struct CIVdtmpar p = srf->par;
  int sts = SUCCESS;

  if ( srf->version > 6 )
  {
    if ( fwrite ( &p, sizeof(struct CIVdtmpar), 1, handleP ) != 1 )
      sts = DTM_M_WRFILF;
  }
  else if ( srf->version > 5 )
  {
    struct CIVdtmparV6 parV6;

    parV6.scl      = srf->par.scl;
    parV6.maxlin   = srf->par.maxlin;
    parV6.maxsid   = srf->par.maxsid;
    parV6.ftrsOnly = srf->par.secFtrsOnly;

    if ( fwrite ( &parV6, sizeof(struct CIVdtmparV6), 1, handleP ) != 1 )
      sts = DTM_M_WRFILF;
  }
  else
  {
    struct CIVdtmparV5 parV5;

    parV5.scl     = srf->par.scl;
    parV5.maxlin  = srf->par.maxlin;
    parV5.maxsid  = srf->par.maxsid;

    if ( fwrite ( &parV5, sizeof(struct CIVdtmparV5), 1, handleP ) != 1 )
      sts = DTM_M_WRFILF;
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveFiles
 DESC: Saves the data file headers in from the .DTM file.
 HIST: Original - tmi 24-Apr-1990
 MISC: static
 KEYW: DTM SAVE FILE
-----------------------------------------------------------------------------%*/

static int aecDTM_saveFiles
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  struct CIVdtmfil fil;
  int sts = SUCCESS, i;

  for ( i = 0; i < DTM_C_NMPNTF  &&  sts == SUCCESS; i++ )
  {
    fil = *srf->pntf[i];
    if ( fwrite ( &fil, CIVdtmfilSize, 1, handleP ) != 1 )
      sts = DTM_M_WRFILF;
  }

  fil = *srf->rngf;
  if ( sts == SUCCESS )
    if ( fwrite ( &fil, CIVdtmfilSize, 1, handleP ) != 1 )
      sts = DTM_M_WRFILF;

  fil = *srf->tinf;
  if ( sts == SUCCESS )
    if ( fwrite ( &fil, CIVdtmfilSize, 1, handleP ) != 1 )
      sts = DTM_M_WRFILF;

  if ( srf->version > 5 )
  {
    for ( i = 0; i < DTM_C_NMFTRF  &&  sts == SUCCESS; i++ )
    {
      fil = *srf->ftrf[i];
      if ( fwrite ( &fil, CIVdtmfilSize, 1, handleP ) != 1 )
        sts = DTM_M_WRFILF;
    }

    fil = *srf->styf;
    if ( sts == SUCCESS )
      if ( fwrite ( &fil, CIVdtmfilSize, 1, handleP ) != 1 )
        sts = DTM_M_WRFILF;

    if ( srf->version > 7 )
    {
      fil = *srf->payf;
      if ( sts == SUCCESS )
        if ( fwrite ( &fil, CIVdtmfilSize, 1, handleP ) != 1 )
          sts = DTM_M_WRFILF;
    }
  }

  if ( srf->version > 8 )
  {
    fil = *srf->corf;
    if ( fwrite ( &fil, CIVdtmfilSize, 1, handleP ) != 1 )
      sts = DTM_M_WRFILF;

    fil = *srf->cmpf;
    if ( fwrite ( &fil, CIVdtmfilSize, 1, handleP ) != 1 )
      sts = DTM_M_WRFILF;

    fil = *srf->cmpMemf;
    if ( sts == SUCCESS )
      if ( fwrite ( &fil, CIVdtmfilSize, 1, handleP ) != 1 )
        sts = DTM_M_WRFILF;
  }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_savePoints
 DESC: Saves the points in from the .DTM file.
 HIST: Original - tmi 24-Apr-1990
 MISC: static
 KEYW: DTM SAVE POINTS
-----------------------------------------------------------------------------%*/

static int aecDTM_savePoints
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  struct CIVdtmblk *blk;
  struct CIVdtmpnt *p;
  int sts = SUCCESS, i, j;

  if ( npnt + ntin + nftr )
    CIVSTATUS ( cpnt, 0, npnt, ctin, 0, ntin, cftr, 0, nftr, DTM_M_SVPNTS );

  for ( j = i = 0; i < DTM_C_NMPNTF  &&  sts == SUCCESS; i++ )
    for ( blk = srf->pntf[i]->blk; blk	&&  sts == SUCCESS; blk = blk->nxt )
      for ( p = blk->rec.pnt; p < blk->rec.pnt + blk->use  &&  sts == SUCCESS; p++ )
      {
        struct CIVdtmpnt pnt = *p;

        if ( srf->version <= 5 )
        {
            pnt.flg &= ~DTM_C_PNTTIN;

            if ( pnt.flg & DTM_C_PNTDFY )
            {
                pnt.flg &= ~DTM_C_PNTDFY;
            }
        }

        if ( !(j++ % 100) ) aecTicker_show(), j = 0;


        if ( fwrite ( &pnt, CIVdtmpntSize, 1, handleP ) != 1 )
          sts = DTM_M_WRFILF;

        if ( npnt + ntin + nftr)
          CIVSTATUS ( cpnt, 1, npnt, ctin, 0, ntin, cftr, 0, nftr, DTM_M_SVPNTS );
      }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveRangePoints
 DESC: Saves the range points in from the .DTM file.
 HIST: Original - tmi 24-Apr-1990
 MISC: static
 KEYW: DTM SAVE POINTS RANGE
-----------------------------------------------------------------------------%*/

static int aecDTM_saveRangePoints
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  struct CIVdtmblk *blk;
  struct CIVdtmpnt *p, pnt;
  int sts = SUCCESS;

  for ( blk = srf->rngf->blk; blk  &&  sts == SUCCESS; blk = blk->nxt )
    for ( p = blk->rec.pnt; p < blk->rec.pnt + blk->use  &&  sts == SUCCESS; p++ )
    {
      pnt = *p;

      if ( fwrite ( &pnt, CIVdtmpntSize, 1, handleP ) != 1 )
	sts = DTM_M_WRFILF;
    }

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveTriangles
 DESC: Saves the triangles in from the .DTM file.
 HIST: Original - tmi 24-Apr-1990
 MISC: static
 KEYW: DTM SAVE TRIANGLES
-----------------------------------------------------------------------------%*/

static int aecDTM_saveTriangles
(
  struct CIVdtmsrf *srf,
  FILE *handleP,
  struct CIVptrind *pi,
  struct CIVptrind *epi
)
{
  struct CIVdtmblk *blk;
  struct CIVdtmtin tin, *t;
  int sts = SUCCESS, j = 0;
  size_t sizp, sizt;

  sizp = sizeof(struct CIVdtmpnt);
  sizt = sizeof(struct CIVdtmtin);

  for ( blk = srf->tinf->blk; blk  &&  sts == SUCCESS; blk = blk->nxt )
    for ( t = blk->rec.tin; t < blk->rec.tin + blk->use  &&  sts == SUCCESS; t++ )
    {
      if ( !(j++ % 100) ) aecTicker_show(), j = 0;

      tin = *t;

      aecDTM_convertPtrToIndex ( (size_t *)&tin.p1,  pi, epi, &sizp );
      aecDTM_convertPtrToIndex ( (size_t *)&tin.p2,  pi, epi, &sizp );
      aecDTM_convertPtrToIndex ( (size_t *)&tin.p3,  pi, epi, &sizp );
      aecDTM_convertPtrToIndex ( (size_t *)&tin.n12, pi, epi, &sizt );
      aecDTM_convertPtrToIndex ( (size_t *)&tin.n23, pi, epi, &sizt );
      aecDTM_convertPtrToIndex ( (size_t *)&tin.n31, pi, epi, &sizt );

      tin.op1 = (long)tin.p1;
      tin.op2 = (long)tin.p2;
      tin.op3 = (long)tin.p3;
      tin.on12 = (long)tin.n12;
      tin.on23 = (long)tin.n23;
      tin.on31 = (long)tin.n31;
      if ( fwrite ( &tin, CIVdtmtinSize, 1, handleP ) != 1 )
        sts = DTM_M_WRFILF;
      else if ( npnt + ntin + nftr )
        CIVSTATUS ( cpnt, 0, npnt, ctin, 1, ntin, cftr, 0, nftr, DTM_M_SVTINS );
    }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveFeatures
 DESC: Saves the features to the .DTM file.
 HIST: Original - twl 26-Oct-1998
 MISC: static
 KEYW: DTM SAVE FEATURES
-----------------------------------------------------------------------------%*/

static int aecDTM_saveFeatures
(
  struct CIVdtmsrf *srf,
  FILE *handleP,
  struct CIVptrind *pi,
  struct CIVptrind *epi
)
{
  struct CIVdtmblk *blk;
  struct CIVdtmftr ftr, *f;
  int sts = SUCCESS, j = 0;
  size_t sizp, sizs, sizpay;
  int i;

  if ( srf->version > 5 )
  {
    sizp = sizeof(struct CIVdtmpnt);
    sizs = sizeof(struct CIVdtmsty);
    sizpay = sizeof(struct CIVdtmpay);

    for ( i = 0; i < DTM_C_NMFTRF; i++ )
      for ( blk = srf->ftrf[i]->blk; blk  &&  sts == SUCCESS; blk = blk->nxt )
        for ( f = blk->rec.ftr; f < blk->rec.ftr + blk->use  &&  sts == SUCCESS; f++ )
        {
          if ( !(j++ % 100) ) aecTicker_show(), j = 0;

          ftr = *f;

          aecDTM_convertPtrToIndex ( (size_t *)&ftr.p1,  pi, epi, &sizp );
          aecDTM_convertPtrToIndex ( (size_t *)&ftr.s1,  pi, epi, &sizs );
          aecDTM_convertPtrToIndex ( (size_t *)&ftr.pay,  pi, epi, &sizpay );

          if ( srf->version > 8 )
          {
          ftr.p[0] = (long)ftr.p1;
          ftr.p[1] = (long)ftr.s1;
          ftr.p[2] = (long)ftr.pay;
            if ( fwrite ( &ftr, CIVdtmftrSize, 1, handleP ) != 1 )
              sts = DTM_M_WRFILF;
          }
          else if ( srf->version > 7 )
          {
            struct CIVdtmftrV8 ftrV8;

            memset ( &ftrV8, 0, sizeof ( ftrV8 ) );

            aecGuid_copy ( &ftrV8.guid, &ftr.guid );
            wcstombs ( ftrV8.nam, ftr.nam, CIV_C_NAMSIZ-1 );
            wcstombs ( ftrV8.des, ftr.des, CIV_C_DESSIZ-1 );
            wcstombs ( ftrV8.par, ftr.par, CIV_C_NAMSIZ-1 );
            ftrV8.p[0] = (long)ftr.p1;
            ftrV8.p[1] = (long)ftr.s1;
            ftrV8.p[2] = (long)ftr.pay;
            ftrV8.numPnts = ftr.numPnts;
            ftrV8.numStyles = ftr.numStyles;
            ftrV8.numPayItems = ftr.numPayItems;
            ftrV8.pntDensity = ftr.pntDensity;
            ftrV8.flg = ftr.flg;

            if ( fwrite ( &ftrV8, sizeof ( ftrV8 ), 1, handleP ) != 1 )
              sts = DTM_M_WRFILF;
          }
          else
          {
            struct CIVdtmftrV7 ftrV7;

            memset ( &ftrV7, 0, sizeof ( ftrV7 ) );

            aecGuid_copy ( &ftrV7.guid, &ftr.guid );
            wcstombs ( ftrV7.nam, ftr.nam, CIV_C_NAMSIZ-1 );
            wcstombs ( ftrV7.des, ftr.des, CIV_C_DESSIZ-1 );
            wcstombs ( ftrV7.par, ftr.par, CIV_C_NAMSIZ-1 );
            ftrV7.p[0] = (long)ftr.p1;
            ftrV7.p[1] = (long)ftr.s1;
            ftrV7.numPnts = ftr.numPnts;
            ftrV7.numStyles = ftr.numStyles;
            ftrV7.pntDensity = ftr.pntDensity;
            ftrV7.flg = ftr.flg;

            if ( fwrite ( &ftrV7, sizeof ( ftrV7 ), 1, handleP ) != 1 )
              sts = DTM_M_WRFILF;
          }


          if ( sts == SUCCESS && ( npnt + ntin + nftr ) )
            CIVSTATUS ( cpnt, 0, npnt, ctin, 0, ntin, cftr, 1, nftr, DTM_M_SVFTRS );
        }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveFeatureStyles
 DESC: Saves the feature styles to the .DTM file.
 HIST: Original - twl 26-Oct-1998
 MISC: static
 KEYW: DTM SAVE FEATURE STYLES
-----------------------------------------------------------------------------%*/

static int aecDTM_saveFeatureStyles
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  struct CIVdtmblk *blk;
  struct CIVdtmsty sty, *s;
  int sts = SUCCESS, j = 0;

  if ( srf->version > 5 )
  {
    for ( blk = srf->styf->blk; blk  &&  sts == SUCCESS; blk = blk->nxt )
      for ( s = blk->rec.sty; s < blk->rec.sty + blk->use  &&  sts == SUCCESS; s++ )
      {
        if ( !(j++ % 100) ) aecTicker_show(), j = 0;

        sty = *s;

        if ( srf->version > 8 )
        {
          if ( fwrite ( &sty, CIVdtmstySize, 1, handleP ) != 1 )
            sts = DTM_M_WRFILF;
        }
        else
        {
          CIVdtmstyV8 styV8;

          memset ( &styV8 , 0, sizeof ( styV8 ) );

          wcstombs ( styV8.nam, sty.nam, CIV_C_NAMSIZ );
          styV8.flg = sty.flg;

          if ( fwrite ( &styV8, sizeof ( styV8 ), 1, handleP ) != 1 )
            sts = DTM_M_WRFILF;
        }
      }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveFeaturePayItems
 DESC: Saves the feature styles to the .DTM file.
 HIST: Original - twl 26-Oct-1998
 MISC: static
 KEYW: DTM SAVE FEATURE STYLES
-----------------------------------------------------------------------------%*/

static int aecDTM_saveFeaturePayItems
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  struct CIVdtmblk *blk;
  struct CIVdtmpay pay, *p;
  int sts = SUCCESS, j = 0;

  if ( srf->version > 7 )
  {
    for ( blk = srf->payf->blk; blk  &&  sts == SUCCESS; blk = blk->nxt )
      for ( p = blk->rec.pay; p < blk->rec.pay + blk->use  &&  sts == SUCCESS; p++ )
      {
        if ( !(j++ % 100) ) aecTicker_show(), j = 0;

        pay = *p;

        if ( srf->version > 8 )
        {
          if ( fwrite ( &pay, CIVdtmpaySize, 1, handleP ) != 1 )
          sts = DTM_M_WRFILF;
        }
        else
        {
          CIVdtmpayV8 payV8;

          memset ( &payV8 , 0, sizeof ( payV8 ) );

          wcstombs ( payV8.nam, pay.nam, CIV_C_NAMSIZ );
          payV8.flg = pay.flg;

          if ( fwrite ( &payV8, sizeof ( payV8 ), 1, handleP ) != 1 )
            sts = DTM_M_WRFILF;
        }
      }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveCorridors
 DESC: Saves the corridors to the .DTM file.
 HIST: Original - twl 29-Oct-2004
 MISC: static
 KEYW: DTM SAVE CORRIDORS
-----------------------------------------------------------------------------%*/

static int aecDTM_saveCorridors
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  struct CIVdtmblk *blk;
  struct CIVdtmcor cor, *c;
  int sts = SUCCESS, j = 0;

  if ( srf->version > 8 )
  {
    for ( blk = srf->corf->blk; blk  &&  sts == SUCCESS; blk = blk->nxt )
      for ( c = blk->rec.cor; c < blk->rec.cor + blk->use  &&  sts == SUCCESS; c++ )
      {
        if ( !(j++ % 100) ) aecTicker_show(), j = 0;

        cor = *c;

        if ( fwrite ( &cor, CIVdtmcorSize, 1, handleP ) != 1 )
          sts = DTM_M_WRFILF;
      }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveComponents
 DESC: Saves the components to the .DTM file.
 HIST: Original - twl 24-Apr-2002
 MISC: static
 KEYW: DTM SAVE COMPONENT
-----------------------------------------------------------------------------%*/

static int aecDTM_saveComponents
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  struct CIVdtmblk *blk;
  struct CIVdtmcmp cmp, *c;
  int sts = SUCCESS, j = 0;

  if ( srf->version > 8 )
  {
    for ( blk = srf->cmpf->blk; blk  &&  sts == SUCCESS; blk = blk->nxt )
      for ( c = blk->rec.cmp; c < blk->rec.cmp + blk->use  &&  sts == SUCCESS; c++ )
      {
        if ( !(j++ % 100) ) aecTicker_show(), j = 0;

        cmp = *c;

        if ( srf->version > 10 )
        {
            if ( fwrite ( &cmp, CIVdtmcmpSize, 1, handleP ) != 1 )
              sts = DTM_M_WRFILF;
        }
        else
        {
            CIVdtmcmpV10 v10cmp;

            memset ( &v10cmp, 0, sizeof ( v10cmp ) );

            aecGuid_copy ( &v10cmp.guid, &cmp.guid );
            aecGuid_copy ( &v10cmp.corGuid, &cmp.corGuid );
            memcpy ( v10cmp.nam, cmp.nam, sizeof ( v10cmp.nam ) );
            memcpy ( v10cmp.des, cmp.des, sizeof ( v10cmp.des ) );
            memcpy ( v10cmp.stynam, cmp.stynam, sizeof ( v10cmp.stynam ) );
            v10cmp.startXY = cmp.startXY;
            v10cmp.stopXY = cmp.stopXY;
            v10cmp.startStn = cmp.startStn;
            v10cmp.stopStn = cmp.stopStn;
            v10cmp.flg = cmp.flg;
            v10cmp.flg &= ~DTM_C_CMPOVR;

            if ( fwrite ( &v10cmp, sizeof ( v10cmp ), 1, handleP ) != 1 )
              sts = DTM_M_WRFILF;
        }
      }
  }

  return ( sts );
}




/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_saveComponentMembers
 DESC: Saves the component members to the .DTM file.
 HIST: Original - twl 24-Apr-2002
 MISC: static
 KEYW: DTM SAVE COMPONENT MEMBERS
-----------------------------------------------------------------------------%*/

static int aecDTM_saveComponentMembers
(
  struct CIVdtmsrf *srf,
  FILE *handleP
)
{
  struct CIVdtmblk *blk;
  struct CIVdtmcmpmem cmpmem, *c;
  int sts = SUCCESS, j = 0;

  if ( srf->version > 8 )
  {
    for ( blk = srf->cmpMemf->blk; blk  &&  sts == SUCCESS; blk = blk->nxt )
      for ( c = blk->rec.cmpMem; c < blk->rec.cmpMem + blk->use  &&  sts == SUCCESS; c++ )
      {
        if ( !(j++ % 100) ) aecTicker_show(), j = 0;

        cmpmem = *c;

        if ( srf->version > 10 )
        {
            if ( fwrite ( &cmpmem, CIVdtmcmpmemSize, 1, handleP ) != 1 )
              sts = DTM_M_WRFILF;
        }
        else
        {
            CIVdtmcmpmemV10 v10cmpMem;

            memset ( &v10cmpMem, 0, sizeof ( v10cmpMem ) );

            aecGuid_copy ( &v10cmpMem.cmpGuid, &cmpmem.cmpGuid );
            aecGuid_copy ( &v10cmpMem.cmpMemGuid, &cmpmem.cmpMemGuid );
            v10cmpMem.type = cmpmem.type;
            v10cmpMem.index = cmpmem.index;
            v10cmpMem.flg = cmpmem.flg;

            if ( v10cmpMem.type == DTM_C_CMPMEMOVR )
                v10cmpMem.flg = DTM_C_CMPMEMDEL;

            if ( fwrite ( &v10cmpMem, sizeof ( v10cmpMem ), 1, handleP ) != 1 )
              sts = DTM_M_WRFILF;
        }
      }
  }

  return ( sts );
}


