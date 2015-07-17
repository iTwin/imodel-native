//---------------------------------------------------------------------------+
// $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//---------------------------------------------------------------------------+
/*----------------------------------------------------------------------------*/
/* ftruti.c                                            twl    27-Oct-1998     */
/*----------------------------------------------------------------------------*/
/* Contains various feature utilities.				                 	              */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* Include files                                                              */
/*----------------------------------------------------------------------------*/
#include "stdafx.h"

#define     PNT_SAME    0
#define     PNT_NEW     1
#define     PNT_MOVED   2

struct CIVdtmdat
{
    struct CIVdtmblk *blk;
    struct CIVdtmsrf *srf;
};

struct styleProcData
{
    struct CIVdtmsrf *srf;
    long   index;
};

typedef struct
{
    struct CIVdtmpnt *newAddr;
    struct CIVdtmpnt *oldAddr;
    long pntCode;
} MatchInfo;


static int  aecDTM_countFeaturesProcess(void *, int, long, DPoint3d *, struct CIVdtmpnt * );
static int  aecDTM_featuresFromPointsProcess(void *, int, long, DPoint3d *, struct CIVdtmpnt * );
static int  aecDTM_assignFeatureStylesProc(void *, struct CIVdtmsrf *unused, int, struct CIVdtmftr * );
static int  aecDTM_getFeaturePoints(struct CIVdtmftr *, CIVdtmpnt **, long *);
static int  aecDTM_getFeatureStyles(struct CIVdtmftr *, CIVdtmstynam **, long *);
static int  aecDTM_getFeaturePayItems(struct CIVdtmftr *, CIVdtmpaynam **, long *);
static int  aecDTM_setFeaturePoints(long, struct CIVdtmftr *, struct CIVdtmsrf *, CIVdtmpnt *, long, double*, byte *);
static int  aecDTM_setFeatureStyles(struct CIVdtmftr *, struct CIVdtmsrf *, CIVdtmstynam *, long );
static int  aecDTM_transferFeaturePoints(long, struct CIVdtmfil *, struct CIVdtmpnt *, CIVdtmpnt *, long, int);
static int  aecDTM_preProcessMatchPnts(MatchInfo **, long, struct CIVdtmsrf *, struct CIVdtmftr *, struct CIVdtmpnt *, long, int);
static int  aecDTM_postProcessMatchPnts(MatchInfo *, long, struct CIVdtmsrf *, struct CIVdtmfil *, struct CIVdtmftr *);
static void aecDTM_featurePointsHorizontalLength(double *, long, CIVdtmpnt *);
static void segElevation(CIVdtmpnt *, CIVdtmpnt *, CIVdtmpnt *);
static int  aecDTM_compareMatches(const void *, const void *);
static int  aecDTM_compareXYZmatches(const void *, const void *);
static int  aecDTM_tinMatchProcess(void *, long, DPoint3d *, struct CIVdtmtin *, unsigned long);


static int cnt = 0;
static boolean FirstRegPnt;



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_generateFeaturesFromPoints
 DESC: Created featurs from breakline, exterior boundary, etc points when loading
       a type 5 or earlier.DTM file.
 HIST: Original - twl 23-Oct-1998
 MISC: static
 KEYW: DTM LOAD FEATURE FROM POINTS
-----------------------------------------------------------------------------%*/

int aecDTM_generateFeaturesFromPoints
(
    struct CIVdtmsrf *srf
)
{
    struct CIVdtmblk *blk;
    long typmsk = DTM_C_REGMSK | DTM_C_BRKMSK | DTM_C_INTMSK | DTM_C_EXTMSK | DTM_C_CTRMSK;
    int  opt = DTM_C_NOBREK | DTM_C_DELETE | DTM_C_SNDONE;
    int i;
    int sts = SUCCESS;

    cnt = 0;

    if ( ( sts = aecDTM_sendAllPoints ( (void *)0, srf, opt, typmsk, aecDTM_countFeaturesProcess, (void *)srf ) ) == SUCCESS )
        for ( i = 0; i < DTM_C_NMFTRF && sts == SUCCESS; i++ )
        {
            if ( srf->ftrf[i]->nrec )
                sts = aecDTM_allocateBlock ( &blk, srf->ftrf[i], srf->ftrf[i]->nrec, 0 );
        }

    FirstRegPnt = TRUE;

    if ( sts == SUCCESS )
        sts = aecDTM_sendAllPoints ( (void *)0, srf, opt, typmsk, aecDTM_featuresFromPointsProcess, (void *)srf );

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_countFeaturesProcess
 DESC: 
 HIST: Original - twl 23-Oct-1998
 MISC: static
 KEYW: DTM LOAD FEATURE FROM POINTS
-----------------------------------------------------------------------------%*/

static int aecDTM_countFeaturesProcess
(
    void *tmp,
    int typ,
    long,
    DPoint3d *,
    struct CIVdtmpnt *p
)
{
    CIVdtmsrf *srf = (CIVdtmsrf *)tmp;
    int sts = SUCCESS;

    if ( typ == DTM_C_DTMREG )
        srf->regFtrf->nrec = 1;
    else if ( !(p->flg & DTM_C_PNTPUD) )
    {
        switch ( typ )
        {
            case ( DTM_C_DTMBRK ) :  srf->brkFtrf->nrec++;  break;
            case ( DTM_C_DTMINT ) :  srf->intFtrf->nrec++;  break;
            case ( DTM_C_DTMEXT ) :  srf->extFtrf->nrec++;  break;
            case ( DTM_C_DTMCTR ) :  srf->ctrFtrf->nrec++;  break;
        }

        if ( cnt++ == 100 ) 
        {
            aecTicker_show();
            cnt = 0;
        }
    }

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_featuresFromPointsProcess
 DESC:
 HIST: Original - twl 23-Oct-1998
 MISC: static
 KEYW: DTM LOAD FEATURE FROM POINTS
-----------------------------------------------------------------------------%*/

static int aecDTM_featuresFromPointsProcess
(
    void *tmp,
    int typ,
    long,
    DPoint3d *,
    struct CIVdtmpnt *p
)
{
    CIVdtmsrf *srf = (CIVdtmsrf *)tmp;
    struct CIVdtmblk *blkP = NULL;
    wchar_t baseName[DTM_C_NAMSIZ];
    wchar_t ftrName[DTM_C_NAMSIZ];
    long genMsg = 0;
    long index;
    int sts = SUCCESS;

    switch ( typ )
    {
        case ( DTM_C_DTMREG ) :  blkP = srf->regFtrf->blk;  genMsg = DTM_M_PNTFTRNAME; break;
        case ( DTM_C_DTMBRK ) :  blkP = srf->brkFtrf->blk;  genMsg = DTM_M_BRKFTRNAME; break;
        case ( DTM_C_DTMINT ) :  blkP = srf->intFtrf->blk;  genMsg = DTM_M_INTFTRNAME; break;
        case ( DTM_C_DTMEXT ) :  blkP = srf->extFtrf->blk;  genMsg = DTM_M_EXTFTRNAME; break;
        case ( DTM_C_DTMCTR ) :  blkP = srf->ctrFtrf->blk;  genMsg = DTM_M_CTRFTRNAME; break;
    }

    if ( ( typ == DTM_C_DTMREG && FirstRegPnt ) || ( typ != DTM_C_DTMREG && !(p->flg & DTM_C_PNTPUD) ) )
    {
        blkP->use++;
        index = blkP->use - 1;
        swprintf ( baseName, L"%s%d", aecOutput_getMessageString(genMsg), (index + 1) );
        aecGuid_generate ( &blkP->rec.ftr[index].guid );
        wcscpy ( ftrName, baseName );
        wcscpy ( blkP->rec.ftr[index].nam, ftrName );
        blkP->rec.ftr[index].p1 = p;
        blkP->rec.ftr[index].numPnts = 1;

        if ( typ == DTM_C_DTMREG )
            FirstRegPnt = FALSE;
    }
    else
    {
        index = blkP->use - 1;
        blkP->rec.ftr[index].numPnts++;
    }

    if ( cnt++ == 100 ) 
    {
        aecTicker_show();
        cnt = 0;
    }

    return ( sts );
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_generateUniqueFeatureName
 DESC: Generates a feature name which is unique within a surface.
 HIST: Original - twl 27-Mar-1998
 MISC:
 KEYW: DTM FEATURE NAME GENERATE UNIQUE
-----------------------------------------------------------------------------%*/

void aecDTM_generateUniqueFeatureName
(
    wchar_t *newNameP,                 /* <= unique name                        */
    wchar_t *oldNameP,                 /* => name to check                      */
    struct CIVdtmsrf *srfP             /* => DTM surface (or NULL)              */
)
{
    CIVdtmftr *ftrP;

    if ( newNameP != (wchar_t *)0 )
    {
        newNameP[0] = '\0';

        if ( oldNameP != (wchar_t *)0 )
        {
            wchar_t tmpOldName[DTM_C_NAMSIZ];
            size_t idx;
            int cnt = 1;

            wcscpy ( newNameP, oldNameP );
            wcscpy ( tmpOldName, oldNameP );
            tmpOldName[DTM_C_NAMSIZ-1] = '\0';
            idx = wcslen ( tmpOldName ) - 1;

            while ( idx > 0 && iswdigit ( tmpOldName[idx-1] ) )
                idx--;

            if ( iswdigit( tmpOldName[idx] ) )
            {
                wchar_t wcDigits[DTM_C_NAMSIZ];
                wcscpy ( wcDigits, &tmpOldName[idx] );
                swscanf ( wcDigits, L"%ld", &cnt );
                cnt++;
                tmpOldName[idx] = '\0';                    
            }

            do
            {
                if ( aecDTM_findFeatureByName ( &ftrP, srfP, newNameP ) == SUCCESS )
                    swprintf ( newNameP, L"%.*s%d", DTM_C_NAMSIZ-3, tmpOldName, cnt++ );     /* DO_NOT_TRANSLATE */
                else
                    cnt = 0;
            } while ( cnt );
        }
    }
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_generateFeatureStyles
 DESC: Generates a default style for each feature in the feature table.
 HIST: Original - twl 23-Oct-1998
 MISC: static
 KEYW: DTM GENERATE FEATURE STYLES
-----------------------------------------------------------------------------%*/

int aecDTM_generateFeatureStyles
(
    struct CIVdtmsrf *srf
)
{
    struct styleProcData dat;
    struct CIVdtmblk *blk;
    struct CIVdtmsty *s;
    long typmsk = DTM_C_REGMSK | DTM_C_BRKMSK | DTM_C_INTMSK | DTM_C_EXTMSK | DTM_C_CTRMSK;
    int  opt = DTM_C_NOBREK | DTM_C_DELETE | DTM_C_SNDONE;
    int sts = SUCCESS;
    int j = 0;
    int i;

    srf->styf->nrec = 0;

    for ( i = 0; i < DTM_C_NMFTRF; i++ )
        srf->styf->nrec += srf->ftrf[i]->nrec;

    if ( srf->styf->nrec && ( sts = aecDTM_allocateBlock ( &blk, srf->styf, srf->styf->nrec, 0 ) ) == SUCCESS )
    {
        for ( s = blk->rec.sty; blk->use < srf->styf->nrec  &&  sts == SUCCESS; blk->use++ )
        {
            if ( j++ == 100 ) 
            {
                aecTicker_show();
                j = 0;
            }

            wcsncpy ( s[blk->use].nam, DEFAULT_PREFERENCE_NAME, CIV_C_NAMSIZ-1 );
        }

        memset ( &dat, 0, sizeof ( dat ) );
        dat.srf = srf;
        sts = aecDTM_sendAllFeatures ( (void *)NULL, srf, opt, typmsk, aecDTM_assignFeatureStylesProc, (void *)&dat );
    }

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_assignFeatureStylesProc
 DESC:
 HIST: Original - twl 4-Nov-1998
 MISC: static
 KEYW: DTM LOAD ASSIGN FEATURE STYLES
-----------------------------------------------------------------------------%*/

static int aecDTM_assignFeatureStylesProc
(
    void *tmp,
    struct CIVdtmsrf *,
    int ftrTyp,
    struct CIVdtmftr *ftrP
)
{
    struct styleProcData *dat = (struct styleProcData *)tmp;
    int sts = SUCCESS;


    if ( dat->index < dat->srf->styf->blk->use )
    {
        ftrP->s1 = &dat->srf->styf->blk->rec.sty[dat->index];

        if ( ftrTyp == DTM_C_DTMREGFTR )
          wcsncpy ( ftrP->s1[0].nam, aecOutput_getMessageString ( DTM_M_RNDFTRSTY ), CIV_C_NAMSIZ-1 );
        else
          wcsncpy ( ftrP->s1[0].nam, DEFAULT_PREFERENCE_NAME, CIV_C_NAMSIZ-1 );

        ftrP->numStyles = 1;
        dat->index++;
    }

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getFeatureInfo
 DESC: Extracts feature information from a feature stored in a surface.
       pntsPP and stylesPP must be freed by the caller.
 HIST: Original - twl 4-Jan-1999
 MISC: static
 KEYW: DTM FEATURE GET INFO
-----------------------------------------------------------------------------%*/

int aecDTM_getFeatureInfo
(
    CIVdtmftr    *ftrP,                   /*  => feature                      */
    CIVdtmsrf    *srfP,                   /*  => surface containing feature   */
    GUID         *guidP,                  /* <=  feature's GUID (or NULL)     */
    long         *typeP,                  /* <=  feature type (or NULL)       */
    wchar_t      nameP[DTM_C_NAMSIZ],     /* <=  feature name (or NULL)       */
    wchar_t      descP[DTM_C_NAMSIZ],     /* <=  feature description (or NULL)*/
    wchar_t      parentP[DTM_C_NAMSIZ],   /* <=  parent feature name (or NULL)*/
    CIVdtmpnt    **pntsPP,                /* <=  feature's points (or NULL)   */
    long         *numPntsP,               /* <=  number of points (or NULL)   */
    double       *pntDensityP,            /* <=  point density (or NULL)      */
    CIVdtmstynam **stylesPP,              /* <=  feature's styles (or NULL)   */
    long         *numStylesP,             /* <=  number of styles (or NULL)   */
    CIVdtmpaynam **payItemsPP,            /* <=  feature's pay items (or NULL)*/
    long         *numPayItemsP,           /* <=  number of pay items (or NULL)*/
    byte      *flagP                   /* <=  feature's flag (or NULL)     */
)
{
    int sts = SUCCESS;

    if ( sts == SUCCESS && pntsPP )
        *pntsPP = NULL;

    if ( sts == SUCCESS && stylesPP )
        *stylesPP = NULL;

    if ( sts == SUCCESS && payItemsPP )
        *payItemsPP = NULL;

    if ( sts == SUCCESS && guidP )
        aecGuid_copy ( guidP, &ftrP->guid );

    if ( sts == SUCCESS && typeP )
        aecDTM_findFeatureType ( typeP, NULL, srfP, ftrP );

    if ( sts == SUCCESS && nameP )
        wcscpy ( nameP, ftrP->nam );

    if ( sts == SUCCESS && descP )
        wcscpy ( descP, ftrP->des );

    if ( sts == SUCCESS && parentP )
        wcscpy ( parentP, ftrP->par );

    if ( sts == SUCCESS && ( pntsPP || numPntsP ) )
        sts = aecDTM_getFeaturePoints ( ftrP, pntsPP, numPntsP );

    if ( sts == SUCCESS && stylesPP && numStylesP )
        sts = aecDTM_getFeatureStyles ( ftrP, stylesPP, numStylesP );

    if ( sts == SUCCESS && payItemsPP && numPayItemsP )
        sts = aecDTM_getFeaturePayItems ( ftrP, payItemsPP, numPayItemsP );

    if ( sts == SUCCESS && pntDensityP )
        *pntDensityP = ftrP->pntDensity;

    if ( sts == SUCCESS && flagP )
        *flagP = ftrP->flg;

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getFeaturePoints
 DESC: Extracts feature points from a feature stored in a surface.
       pntsPP must be freed by the caller.
 HIST: Original - twl 4-Jan-1999
 MISC: static
 KEYW: DTM FEATURE GET POINTS
-----------------------------------------------------------------------------%*/

static int aecDTM_getFeaturePoints
(
    struct CIVdtmftr *ftrP,
    CIVdtmpnt **pntsPP,
    long *numPntsP
)
{
    int i;
    int sts = SUCCESS;

    *numPntsP = 0;

    for ( i = 0; i < ftrP->numPnts; i++ )
        if ( !aecDTM_isPointDeletedFlagSet ( &ftrP->p1[i] ) && !( ftrP->p1[i].flg & DTM_C_PNTDFY ) )
            (*numPntsP)++;

    if ( pntsPP )
    {
        if ( !(*numPntsP) ) *numPntsP = 1;

        if ( ( *pntsPP = (CIVdtmpnt * ) calloc ( *numPntsP, sizeof ( CIVdtmpnt ) ) ) != NULL )
        {
            *numPntsP = 0;

            for ( i = 0; i < ftrP->numPnts; i++ )
            {
                if ( !aecDTM_isPointDeletedFlagSet ( &ftrP->p1[i] ) && !( ftrP->p1[i].flg & DTM_C_PNTDFY ) )
                {
                    DTMPOINTTODPOINT ( srfP, &ftrP->p1[i], (*pntsPP)[*numPntsP].cor );
                    (*pntsPP)[*numPntsP].flg = 0;

                    if ( ftrP->p1[i].flg & DTM_C_PNTPUD)
                        (*pntsPP)[*numPntsP].flg |= DTM_C_PNTPUD;

                    (*numPntsP)++;
                }
            }
        }
        else
            sts = DTM_M_MEMALF;
    }
    
    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getFeatureStyles
 DESC: Extracts feature styles from a feature stored in a surface.
       stylesPP must be freed by the caller.
 HIST: Original - twl 4-Jan-1999
 MISC: static
 KEYW: DTM FEATURE GET POINTS
-----------------------------------------------------------------------------%*/

static int aecDTM_getFeatureStyles
(
    struct CIVdtmftr *ftrP,
    CIVdtmstynam **stylesPP,
    long *numStylesP
)
{
    int i;
    int sts = SUCCESS;

    if ( ( *stylesPP = (CIVdtmstynam *) calloc ( ftrP->numStyles * CIV_C_NAMSIZ, sizeof ( wchar_t ) ) ) != NULL )
    {
        *numStylesP = 0;

        for ( i = 0; i < ftrP->numStyles; i++ )
        {
            if ( !aecDTM_isStyleDeletedFlagSet ( &ftrP->s1[i] ) )
            {
                wcscpy ( (*stylesPP)[i], ftrP->s1[i].nam );
                (*numStylesP)++;
            }
        }
    }
    else
        sts = DTM_M_MEMALF;

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_getFeaturePayItems
 DESC: Extracts feature pay items from a feature stored in a surface.
       payItemsPP must be freed by the caller.
 HIST: Original - twl 10-Oct-2003
 MISC: static
 KEYW: DTM FEATURE GET PAY ITEMS
-----------------------------------------------------------------------------%*/

static int aecDTM_getFeaturePayItems
(
    struct CIVdtmftr *ftrP,
    CIVdtmpaynam **payItemsPP,
    long *numPayItemsP
)
{
    int i;
    long alloc = ftrP->numPayItems ? ftrP->numPayItems : 1;
    int sts = SUCCESS;

    if ( ( *payItemsPP = (CIVdtmpaynam *) calloc ( alloc * CIV_C_NAMSIZ, sizeof ( wchar_t ) ) ) != NULL )
    {
        *numPayItemsP = 0;

        for ( i = 0; i < ftrP->numPayItems; i++ )
        {
            if ( !aecDTM_isPayItemDeletedFlagSet ( &ftrP->pay[i] ) )
            {
                wcscpy ( (*payItemsPP)[i], ftrP->pay[i].nam );
                (*numPayItemsP)++;
            }
        }
    }
    else
        sts = DTM_M_MEMALF;

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_setFeatureInfo
 DESC: Sets a feature's guid, name, description, parent, points, point density,
       styles, and properties.  Each is optional if a null is passed for any
       one of these it will be left unchanged in the feature.  When an array of
       points are passed, the feature's existing points will be replaced with
       the points passed in.  The 'opt' parameter allows the caller to specify
       additional information about the points that can speed up processing
       of existing triangles.

       The following values can be passed for opt:
       DTM_C_DELONLY - If points have only been deleted.
       DTM_C_ADDONLY - If points have only been inserted or added.
       DTM_C_MOVONLY - If points have only been moved.
       DTM_C_ELVONLY - If the elevations of points have only been changed.
    
 HIST: Original - twl 4-Jan-1999
 MISC: 
 KEYW: DTM FEATURE SET INFO
-----------------------------------------------------------------------------%*/

int aecDTM_setFeatureInfo
(
    CIVdtmftr    *ftrP,                   /* <=> feature                      */
    CIVdtmsrf    *srfP,                   /*  => surface containing feature   */
    long         opt,                     /*  => operational information or 0 */
    GUID         *guidP,                  /*  => feature's GUID (or NULL)     */
    long         *typeP,                  /*  => feature type (or NULL)       */
    wchar_t      nameP[DTM_C_NAMSIZ],     /*  => feature name (or NULL)       */
    wchar_t      descP[DTM_C_NAMSIZ],     /*  => feature description (or NULL)*/
    wchar_t      parentP[DTM_C_NAMSIZ],   /*  => parent feature name (or NULL)*/
    CIVdtmpnt    *pntsP,                  /*  => feature's points (or NULL)   */
    long         numPnts,                 /*  => number of points (or NULL)   */
    double       *pntDensityP,            /*  => point density (or NULL)      */
    CIVdtmstynam *stylesP,                /*  => feature's styles (or NULL)   */
    long         numStyles,               /*  => number of styles (or NULL)   */
    CIVdtmpaynam *payItemsP,              /*  => pay items (or NULL)          */
    long         numPayItems,             /*  => # of pay items               */
    byte      *flagP,                  /*  => feature's flag (or NULL)     */
    BOOL                                  /*  => retriangulate (usually TRUE) */
)
{
    long type;
    int sts = SUCCESS;

    aecDTM_findFeatureType ( &type, NULL, srfP, ftrP );

    if ( typeP && *typeP != type )
    {
        GUID guid;
        GUID newGuid;
        wchar_t name[DTM_C_NAMSIZ];
        wchar_t desc[DTM_C_NAMSIZ];
        wchar_t parent[DTM_C_NAMSIZ];
        CIVdtmpnt *pnts = NULL;
        long nPnts;
        double density;
        CIVdtmstynam *styles = NULL;
        long nStyles;
        CIVdtmpaynam *payItems = NULL;
        long nPayItems;
        byte flag;

        if ( ( sts = aecDTM_getFeatureInfo ( ftrP, srfP, 
                                      &guid, NULL, name, desc, parent, &pnts, &nPnts, &density,
                                      &styles, &nStyles, &payItems, &nPayItems, &flag ) ) == SUCCESS )
        {
            aecDTM_deleteFeatureByGUID ( srfP, &guid );

            if ( guidP )
                aecGuid_copy ( &guid, guidP );

            if ( nameP )
                memcpy ( name, nameP, sizeof ( name ) );

            if ( descP )
                memcpy ( desc, descP, sizeof ( desc ) );

            if ( parentP )
                memcpy ( parent, parentP, sizeof ( parent ) );

            if ( pntsP )
            {
                if ( pnts )
                    free ( pnts );

                pnts = pntsP;
                nPnts = numPnts;
            }

            if ( type == DTM_C_DTMREGFTR && pntsP == NULL )
            {
                int i;
                pnts[0].flg &= ~DTM_C_PNTPUD;

                for ( i = 1; i < nPnts; i++ )
                    pnts[i].flg |= DTM_C_PNTPUD;
            }
    
            if ( pntDensityP )
                density = *pntDensityP;

            if ( stylesP )
            {
                if ( styles )
                    free ( styles );

                styles = stylesP;
                nStyles = numStyles;
            }

            if ( payItemsP )
            {
                if ( payItems )
                    free ( payItems );

                payItems = payItemsP;
                nPayItems = numPayItems;
            }

            if ( flagP )
                memcpy ( &flag, flagP, sizeof ( flag ) );

            sts = aecDTM_addFeature ( &newGuid, srfP, 0,
                                      name, desc, parent, *typeP, nPnts, pnts, NULL, NULL, density,
                                      styles, nStyles, payItems, nPayItems, flag );

            if ( sts == SUCCESS && ( sts = aecDTM_findFeatureByGUID ( &ftrP, srfP, &newGuid ) ) == SUCCESS )
                sts = aecDTM_setFeatureInfo ( ftrP, srfP, 0, &guid, NULL, NULL, NULL, NULL,
                                              NULL, 0, NULL, NULL, 0, NULL, 0, NULL, TRUE );

            if ( pnts && pnts != pntsP )
                free ( pnts );

            if ( styles && styles != stylesP )
                free ( styles );

            if ( payItems && payItems != payItemsP )
                free ( payItems );
        }
    }
    else
    {
        if ( sts == SUCCESS && !aecDTM_isFeatureDeletedFlagSet ( ftrP ) )
            aecDTM_hashDeleteFeature ( srfP, ftrP  );

        if ( sts == SUCCESS && guidP )
            aecGuid_copy ( &ftrP->guid, guidP );

        if ( sts == SUCCESS && nameP )
        {
            if ( wcscmp ( nameP, L"" ) )
                wcscpy ( ftrP->nam, nameP );
            else
                wcscpy ( ftrP->nam, aecOutput_getMessageString ( DTM_M_NOFTRNAME ) );
        }

        if ( sts == SUCCESS && descP )
            wcscpy ( ftrP->des, descP );

        if ( sts == SUCCESS && parentP )
            wcscpy ( ftrP->par, parentP );

        if ( sts == SUCCESS )
            sts = aecDTM_setFeaturePoints ( opt, ftrP, srfP, pntsP, numPnts, pntDensityP, flagP );

        if ( sts == SUCCESS && flagP )
            aecDTM_setFeatureFlag ( ftrP, srfP, flagP );

        if ( sts == SUCCESS && stylesP && numStyles )
            sts = aecDTM_setFeatureStyles ( ftrP, srfP, stylesP, numStyles );

        if ( !aecDTM_isFeatureDeletedFlagSet ( ftrP ) )
            sts = aecDTM_hashInsertFeature ( srfP, ftrP );

        aecDTM_setSurfaceModifiedFlag ( srfP );
    }

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_setFeaturePoints
 DESC: Replaces a feature existing set of points with a new set.
       See aecDTM_setFeatureInfo for 'opt' values.
 HIST: Original - twl 4-Jan-1999
 MISC: static
 KEYW: DTM FEATURE SET POINTS
-----------------------------------------------------------------------------%*/

static int aecDTM_setFeaturePoints
(
    long opt,                   //  => Operational information.
    struct CIVdtmftr *ftrP,     // <=> Feature .
    struct CIVdtmsrf *srfP,     //  => Surface.
    CIVdtmpnt *inputPntsP,      //  => Points to be assigned.
    long numInputPnts,          //  => Number of points.
    double *pntDensityP,        //  => Point density factor.
    byte *flagP              //  => Feature flag settings.
)
{
    MatchInfo *matchListP = NULL;
    struct CIVdtmblk *pntBlkP;
    struct CIVdtmfil *pntFilP;
    struct CIVdtmpnt *p1P;
    boolean bFree_inputPntsP = FALSE;
    boolean bPntsChanged = FALSE;
    long pntType, ftrType, alcSize;
    int closeString;
    int sts = SUCCESS;
    int i;

    aecDTM_findFeatureType ( &ftrType, NULL, srfP, ftrP );


    // If the point density has changed, we will have to replace the features
    // points even if the caller did not pass any new points.

    if ( pntDensityP )
    {

        // If the caller did not pass any new points he passed in 
        // a new point density, get the existing points so we can
        // re-densify them.

        if ( *pntDensityP != ftrP->pntDensity && !inputPntsP )
        {
            aecDTM_getFeaturePoints ( ftrP, &inputPntsP, &numInputPnts );
            bFree_inputPntsP = TRUE;
        }

        ftrP->pntDensity = *pntDensityP;
    }

    if ( inputPntsP && numInputPnts )
    {
        aecDTM_findPointType ( &pntType, NULL, &pntBlkP, srfP, ftrP->p1 );
        aecDTM_findPointFile ( &pntFilP, srfP, ftrP->p1 );

        // Check the feature type and validate the points.

        if ( ( sts = aecDTM_addFeatureCheck ( srfP, ftrType, &numInputPnts, inputPntsP, &closeString, FALSE ) ) == SUCCESS )
        {
            // If we have a point density greater than zero, generate densified points.

            if ( ftrP->pntDensity > AEC_C_TOL3 )
            {
                CIVdtmpnt *newPntsP = NULL;
                long newNumPoints = 0;

                aecDTM_densifyFeaturePoints ( &newNumPoints, &newPntsP, numInputPnts, inputPntsP, ftrP->pntDensity );

                if ( bFree_inputPntsP && inputPntsP )
                    free ( inputPntsP );

                inputPntsP = newPntsP;
                numInputPnts = newNumPoints;  
                bFree_inputPntsP = TRUE;                  
            }


            // Replace the feature's existing points.

            if ( sts == SUCCESS )
            {
                if ( numInputPnts + closeString == ftrP->numPnts )
                {
                    for ( i = 0; i < ftrP->numPnts && !bPntsChanged; i++ )
                        if ( !VEQUAL ( ftrP->p1[i].cor, inputPntsP[i].cor, AEC_C_TOL ) )
                            bPntsChanged = TRUE;
                }
                else
                    bPntsChanged = TRUE;                    

                if ( bPntsChanged )
                    aecDTM_preProcessMatchPnts ( &matchListP, opt, srfP, ftrP, inputPntsP, numInputPnts, closeString );

                if ( numInputPnts + closeString <= ftrP->numPnts )
                {
                    aecDTM_transferFeaturePoints ( opt, pntFilP, ftrP->p1, inputPntsP, numInputPnts, closeString );

                    for ( i = numInputPnts + closeString; i < ftrP->numPnts; i++ )
                        aecDTM_deletePoint ( srfP, pntFilP, &ftrP->p1[i] );

                    ftrP->numPnts = numInputPnts;
                }
                else if ( ftrP->p1 == &pntBlkP->rec.pnt[0] && ftrP->numPnts == pntBlkP->use && 
                     ( numInputPnts + closeString ) <= pntBlkP->alc )
                {
                    aecDTM_transferFeaturePoints ( opt, pntFilP, &pntBlkP->rec.pnt[0], inputPntsP, numInputPnts, closeString );

                    for ( i = numInputPnts + closeString; i < pntBlkP->alc; i++ )
                        memset ( &pntBlkP->rec.pnt[i], 0, sizeof ( struct CIVdtmpnt ) );

                    pntFilP->nrec += ( numInputPnts + closeString - pntBlkP->use );
                    ftrP->numPnts = pntBlkP->use = numInputPnts + closeString;
                }
                else
                {
                    alcSize = aecDTM_calcFeaturePntAllocSize( numInputPnts + closeString );

                    if ( ( sts = aecDTM_addFeaturePointsToFile ( &p1P, srfP, pntFilP, pntType, closeString, numInputPnts, inputPntsP, alcSize ) ) == SUCCESS )
                    {
                        for ( i = 0; i < ftrP->numPnts; i++ )
                            aecDTM_deletePoint ( srfP, pntFilP, &ftrP->p1[i] );

                        ftrP->p1 = p1P;      
                        ftrP->numPnts = numInputPnts + closeString; 
                    }                
                }

                if ( flagP )
                    aecDTM_setFeatureFlag ( ftrP, srfP, flagP );
                else
                {
                    byte tmpFlag;
                    tmpFlag = ftrP->flg;
                    aecDTM_setFeatureFlag ( ftrP, srfP, &tmpFlag );
                }


                // If the entire feature must be retriangulated, triangulate each and
                // every new point assigned to the feature.  Adjust triangle's point
                // addresses that point to the feature's existing points that have 
                // not been changed but have been moved in memory.
                // Then triangulate any additional points added to the feature.

                if ( bPntsChanged )
                    aecDTM_postProcessMatchPnts ( matchListP, opt, srfP, pntFilP, ftrP );
            }
        }
    }
    // If no points are being changed, the we only have to retriangulate the entire
    // feature if the exclude from triangulation bit has changed.
    else if ( flagP && ( ( *flagP & DTM_C_FTRTIN ) != ( ftrP->flg & DTM_C_FTRTIN ) ) ) 
    {
        aecDTM_setFeatureFlag ( ftrP, srfP, flagP );
    }

    if ( bFree_inputPntsP && inputPntsP )
        free ( inputPntsP );

    if ( bPntsChanged )
        aecDTM_setSurfaceTinOutOfDateFlag ( srfP );

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_transferFeaturePoints
 DESC:
 HIST: Original - twl 4-Jan-1999
 MISC: static
 KEYW: DTM FEATURE TRANSFER POINTS
-----------------------------------------------------------------------------%*/

static int aecDTM_transferFeaturePoints
(
    long,
    struct CIVdtmfil *pntFilP,
    struct CIVdtmpnt *pntBuf,
    struct CIVdtmpnt *inputPntsP,
    long numInputPnts,
    int closeString
)
{
    int sts = SUCCESS;
    int i;

    for ( i = 0; i < numInputPnts; i++ )
    {
        if ( aecDTM_isPointDeletedFlagSet ( &pntBuf[i] ) )
            pntFilP->ndel--;

        memcpy ( &pntBuf[i], &inputPntsP[i], sizeof ( struct CIVdtmpnt ) );
    }

    if ( closeString )
    {
        if ( aecDTM_isPointDeletedFlagSet ( &pntBuf[numInputPnts + closeString] ) )
            pntFilP->ndel--;

        memcpy ( &pntBuf[numInputPnts], &inputPntsP[0], sizeof ( struct CIVdtmpnt ) );
        pntBuf[numInputPnts].flg |= DTM_C_PNTPUD;
    }

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_preProcessMatchPnts
 DESC: Compares a feature's existing list of points with an array of new points.
       Returns a list of the memory address of existing points that match the
       new points.  If points are being deleted only or moved only, non-matching
       points will be deleted and untriangulated.
 HIST: Original - twl 03-Jul-1999
 MISC:
 KEYW: DTM PRE PROCESS MATCH POINTS
-----------------------------------------------------------------------------%*/

static int aecDTM_preProcessMatchPnts
(
    MatchInfo **matchListPP,    /* <=  List of match information (caller should free)*/
    long opt,                   /*  => See set feature info */
    struct CIVdtmsrf *srfP,     /*  => Surface to use */
    struct CIVdtmftr *ftrP,     /*  => Feature to use */
    struct CIVdtmpnt *pntsP,    /*  => Points being added to feature */
    long numPnts,               /*  => Number of new points being added */
    int closeString
)
{
    struct CIVdtmpnt *xyzMatchHshTblP = NULL;
    struct CIVpntedt pntdel;
    long nTri;
    long i, k;
    int sts = SUCCESS;


    // If surface has not been triangulated skip this process.

    aecDTM_countSurfaceTriangle( &nTri, srfP );

    if ( nTri == 0 )
        return ( sts );

    memset ( &pntdel, 0, sizeof ( pntdel ) );
    *matchListPP = NULL;

    if ( ftrP->numPnts > (numPnts + closeString) )
        *matchListPP = (MatchInfo *) calloc ( ftrP->numPnts, sizeof ( MatchInfo ) );
    else
        *matchListPP = (MatchInfo *) calloc ( numPnts + closeString, sizeof ( MatchInfo ) );


    if ( *matchListPP == NULL )
        sts = DTM_M_MEMALF;
    else
    {
        switch ( opt )
        {
        case DTM_C_DELONLY:
            pntdel.state = PNTEDT_INIT;
            pntdel.opt = PNTEDT_NOPATCH;
            pntdel.srf = srfP;

            // Initialize the point delete process.

            if ( ( sts = aecDTM_pointDelete ( &pntdel ) ) == SUCCESS )
            {
                pntdel.state = PNTEDT_PRCD;

                for ( i = 0, k = 0; i < ftrP->numPnts && sts == SUCCESS; i++ )
                {
                    // For each old point that matches a new point, save
                    // the address of the old point.  If they don't match,
                    // delete and untriangulate the old point.
                    if( ( k < numPnts ) && (VEQUAL( ftrP->p1[i].cor, pntsP[k].cor, AEC_C_TOL )))
                    {
                        (*matchListPP)[k].oldAddr = &ftrP->p1[i];
                        k++;
                    }
                    else
                    {
                        pntdel.pnt = &ftrP->p1[i];
                        aecDTM_pointDelete ( &pntdel );
                    }
                }
            }
            break;

        case DTM_C_ADDONLY:
            for ( i = 0, k = 0; i < numPnts && sts == SUCCESS; i++ )
            {
                // For each old point that matches a new point, save
                // the address of the old point.  If they don't match,
                // mark the new point as PNT_NEW.
                if((VEQUAL( pntsP[i].cor, ftrP->p1[k].cor, AEC_C_TOL )))
                {
                   (*matchListPP)[i].oldAddr = &ftrP->p1[k];
                    k++;
                }
                else
                    (*matchListPP)[i].pntCode = PNT_NEW;
            }
            break;

        case DTM_C_MOVONLY:
            pntdel.state = PNTEDT_INIT;
            pntdel.opt = PNTEDT_NOPATCH;
            pntdel.srf = srfP;

            if ( ( sts = aecDTM_pointDelete ( &pntdel ) ) == SUCCESS )
            {
                pntdel.state = PNTEDT_PRCD;

                for ( i = 0; i < ftrP->numPnts && sts == SUCCESS; i++ )
                {
                    // For each old point that matches a new point, save
                    // the address of the old point.  If they don't match,
                    // delete and untriangulate the old point and mark the new
                    // point as PNT_MOVED.
                    if( i < numPnts && (VEQUALXY( ftrP->p1[i].cor, pntsP[i].cor, AEC_C_TOL )) )
                    {
                        (*matchListPP)[i].oldAddr = &ftrP->p1[i];
                    }
                    else
                    {
                        if ( !aecDTM_isPointDeletedFlagSet ( &ftrP->p1[i] ) )
                        {
                            pntdel.pnt = &ftrP->p1[i];
                            aecDTM_pointDelete ( &pntdel );
                        }

                        (*matchListPP)[i].pntCode = PNT_MOVED;
                    }
                }

            }
            break; 

        case DTM_C_ELVONLY:
            // Nothing has to be done.  If elevations change only, triangles will
            // not be affected.
            break;   

        default: 
            pntdel.state = PNTEDT_INIT;
            pntdel.opt = PNTEDT_NOPATCH;
            pntdel.srf = srfP;

            if ( ( sts = aecHash_create ( (void **)&xyzMatchHshTblP, aecDTM_compareXYZmatches ) ) == SUCCESS )
            {
                for ( i = 0; i < numPnts && sts == SUCCESS; i++ )
                    sts = aecHash_insert ( xyzMatchHshTblP, &pntsP[i] );
            }
            
            pntdel.state = PNTEDT_INIT;
            pntdel.srf = srfP;

            // Initialize the point delete process.

            if ( ( sts = aecDTM_pointDelete ( &pntdel ) ) == SUCCESS )
            {
                pntdel.state = PNTEDT_PRCD;

                for ( i = 0, k = 0; i < ftrP->numPnts && sts == SUCCESS; i++ )
                {
                    // For each old point that matches a new point, save
                    // the address of the old point.  If they don't match,
                    // delete and untriangulate the old point.
                    if( VEQUAL( ftrP->p1[i].cor, pntsP[k].cor, AEC_C_TOL ) || 
                        aecHash_find ( xyzMatchHshTblP, &ftrP->p1[i] ) )
                    {
                        if ( k < numPnts - 1 )
                            k++;
                    }
                    else
                    {
                        pntdel.pnt = &ftrP->p1[i];
                        aecDTM_pointDelete ( &pntdel );
                    }
                }
            }

            aecHash_destroy ( xyzMatchHshTblP );

            if ( ( sts = aecHash_create ( (void **)&xyzMatchHshTblP, aecDTM_compareXYZmatches ) ) == SUCCESS )
            {
                for ( i = 0; i < ftrP->numPnts && sts == SUCCESS; i++ )
                    sts = aecHash_insert ( xyzMatchHshTblP, &ftrP->p1[i] );
            }

            for ( i = 0; i < numPnts && sts == SUCCESS; i++ )
            {
                struct CIVdtmpnt *matchPntP = NULL;

                // For each old point that matches a new point, save
                // the address of the old point.  If they don't match,
                // mark the new point as PNT_NEW.
                if ( ( matchPntP = (struct CIVdtmpnt *)aecHash_find ( xyzMatchHshTblP, &pntsP[i] ) ) != NULL ) 
                {
                   (*matchListPP)[i].oldAddr = matchPntP;
                }
                else
                {
                   (*matchListPP)[i].pntCode = PNT_NEW;
                }
            }

            aecHash_destroy ( xyzMatchHshTblP );
            break;
        }
    }
   
    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_buildMatchTinList
 DESC: Traverses the list of point addressed created by
       aecDTM_preProcessMatchPnts and assigns a new point address for each
       old one.  All triangles that contain point addresses that match the old
       point addresses are replaced with the correspoinding new point addresses.
 HIST: Original - twl 03-Jul-1999
 MISC:
 KEYW: DTM POST PROCESS MATCH POINTS
-----------------------------------------------------------------------------%*/

static int aecDTM_postProcessMatchPnts
(
    MatchInfo *matchListP,      /* <=  List of match information */
    long opt,                   /*  => Set aecDTM_setFeatureInfo */
    struct CIVdtmsrf *srfP,     /*  => Surface to use */
    struct CIVdtmfil *,         /*  => Point file */
    struct CIVdtmftr *ftrP      /*  => Feature to use */
)
{
    struct MatchInfo *matchHashTblP = NULL;
    long nTri = 0;
    int i;
    int sts = SUCCESS;


    // If no triangles exist, there is no need to maintain triangles.

    aecDTM_countSurfaceTriangle( &nTri, srfP );

    if ( nTri == 0 )
        return ( sts );

    switch ( opt )
    {
    case DTM_C_DELONLY:
    case DTM_C_ADDONLY:
    case DTM_C_MOVONLY:
    default:

        // Create a hash table
        if ( ( sts = aecHash_create ( (void **)&matchHashTblP, aecDTM_compareMatches ) ) == SUCCESS )
        {
            for ( i = 0; i < ftrP->numPnts && sts == SUCCESS; i++ )
            {
                // Compare each old memory address of each matching point with the new
                // address.  If they are different add them to the hash table.
                if ( matchListP[i].pntCode == PNT_SAME )
                {
                    if ( &ftrP->p1[i] != matchListP[i].oldAddr )
                    {
                        matchListP[i].newAddr = &ftrP->p1[i];
                        sts = aecHash_insert ( matchHashTblP, &matchListP[i] );
                    }
                }
            }

            // Compare each triangle's point addresses with the old address pointers 
            // stored in the hash table.  If a match is found, replace the trinangle's
            // point address with the corresponding new address found in the hash
            // table.
            aecDTM_sendAllTriangles ( NULL, srfP, DTM_C_DELETE, aecDTM_tinMatchProcess, matchHashTblP );
            aecHash_destroy ( matchHashTblP );
        }
        break;

    case DTM_C_ELVONLY:
        // Don't need to do anything.
        break;
    }

    if ( matchListP )
        free ( matchListP );

    return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_setFeatureStyles
 DESC:
 HIST: Original - twl 4-Jan-1999
 MISC: static
 KEYW: DTM FEATURE SET STYLES
-----------------------------------------------------------------------------%*/

static int aecDTM_setFeatureStyles
(
    struct CIVdtmftr *ftrP,
    struct CIVdtmsrf *srfP,
    CIVdtmstynam *stylesP,                /*  => feature's styles (or NULL)   */
    long         numStyles               /*  => number of styles (or NULL)   */
)
{        
    struct CIVdtmblk *styleBlkP;
    long allocSize;
    int i;
    int sts = SUCCESS;

    if ( numStyles <= ftrP->numStyles )
    {
        for ( i = 0; i < numStyles; i++ )
        {
            if ( aecDTM_isStyleDeletedFlagSet ( &ftrP->s1[i] ) )
                srfP->styf->ndel--;

            memset ( &ftrP->s1[i], 0, sizeof ( CIVdtmsty ) );
            wcscpy ( ftrP->s1[i].nam, stylesP[i] );
        }      

        for ( i = numStyles; i < ftrP->numStyles; i++ )
            aecDTM_deleteStyle ( srfP, &ftrP->s1[i] );
    
        ftrP->numStyles = numStyles;         
    }
    else
    {
        aecDTM_findStyleBlock ( &styleBlkP, srfP, ftrP->s1 );

        if ( ftrP->s1 == &styleBlkP->rec.sty[0] && ftrP->numStyles == styleBlkP->use && 
             numStyles <= styleBlkP->alc )
        {
            for ( i = 0; i < numStyles; i++ )
            {
                if ( aecDTM_isStyleDeletedFlagSet ( &ftrP->s1[i] ) )
                    srfP->styf->ndel--;

                memset ( &styleBlkP->rec.sty[i], 0, sizeof ( CIVdtmsty ) );
                wcscpy ( styleBlkP->rec.sty[i].nam, stylesP[i] );
            }

            srfP->styf->nrec += ( numStyles - styleBlkP->use );
            styleBlkP->use = numStyles;
            ftrP->numStyles = numStyles;
        }
        else
        {
            for ( i = 0; i < ftrP->numStyles; i++ )
                aecDTM_deleteStyle ( srfP, &ftrP->s1[i] );

            allocSize = ( numStyles > DTM_C_BLKSIZ ) ? numStyles : DTM_C_BLKSIZ;
            sts = aecDTM_allocateBlock ( &styleBlkP, srfP->styf, allocSize, 1 );

            for ( i = 0; i < numStyles; i++ )
                wcscpy ( styleBlkP->rec.sty[styleBlkP->use++].nam, stylesP[i] );

            srfP->styf->nrec += numStyles;
            ftrP->s1 = &styleBlkP->rec.sty[0];
            ftrP->numStyles = numStyles;
        }
    }

    return ( sts );
}


/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_setFeatureFlag
 DESC:
 HIST: Original - twl 4-Jan-1999
 MISC: static
 KEYW: DTM FEATURE SET FLAG
-----------------------------------------------------------------------------%*/

void aecDTM_setFeatureFlag
(
    struct CIVdtmftr *ftrP,
    struct CIVdtmsrf *srfP,
    byte *flagP
)
{
    byte tmpFlag;
    int i;

    if ( ( ftrP->flg & DTM_C_FTRTIN ) != ( *flagP & DTM_C_FTRTIN ) )
        aecDTM_setSurfaceTinOutOfDateFlag ( srfP );

    tmpFlag = ftrP->flg;   
    ftrP->flg = 0;

    if ( *flagP & DTM_C_FTRTIN )    // Do not contour
    {
        ftrP->flg |= DTM_C_FTRTIN;

        for ( i = 0; i < ftrP->numPnts; i++ )
            ftrP->p1[i].flg |= DTM_C_PNTTIN;
    }
    else
    {
        for ( i = 0; i < ftrP->numPnts; i++ )
            ftrP->p1[i].flg &= ~DTM_C_PNTTIN;
    }

    if ( tmpFlag & DTM_C_FTRDEL )
        ftrP->flg |= DTM_C_FTRDEL;
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_calcFeaturePntAllocSize
 DESC:
 HIST: Original - twl 5-June-1999
 MISC: static
 KEYW: DTM FEATURE CALCULATE POINTS ALLOCATION
-----------------------------------------------------------------------------%*/

long aecDTM_calcFeaturePntAllocSize  /* <=  Number to allocate     */
(
    long numPnts                                /* <=  Number of points       */
)
{
    long nAlloc;

    if ( numPnts <= 0 )
        nAlloc = 10;

    else if ( numPnts >= 1 && numPnts <= 10 )
        nAlloc = ( numPnts * 2 > 10 ) ? numPnts * 2 : 10;

    else if ( numPnts >= 11 && numPnts <= 40 )
        nAlloc = numPnts + 20;

    else if ( numPnts >= 41 && numPnts <= 100 )
        nAlloc = (long)((double)numPnts * 1.5);

    else
        nAlloc = numPnts + 50;

    return ( nAlloc );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_densifyFeaturePoints
 DESC: Adds density points to a set of dtm feature points.
 HIST: Original - twl 18-Jun-1999
 MISC:
 KEYW: DTM DENISFY FEATURE POINTS
-----------------------------------------------------------------------------%*/

int aecDTM_densifyFeaturePoints
(
  long *numOutputPntsP,                /* <= # of points in list              */
  CIVdtmpnt **outputPntsPP,            /* <= list of computed points          */
  long numInputPnts,                   /* => # vertices in polyline           */
  CIVdtmpnt *inputPntsP,               /* => polyline vertices                */
  double interval                      /* => interval to use                  */
)
{
    int sts = SUCCESS;

    if ( !outputPntsPP || !numOutputPntsP )
        return sts;

    if ( numInputPnts < 2 || interval <= AEC_C_TOL3 )
    {
        *outputPntsPP = (CIVdtmpnt *) calloc ( numInputPnts, sizeof ( CIVdtmpnt ) );

        if ( *outputPntsPP )
        {
            memcpy ( *outputPntsPP, inputPntsP, sizeof ( CIVdtmpnt ) *  numInputPnts );
            *numOutputPntsP = numInputPnts;
        }
        else
        {
            sts = DTM_M_MEMALF;
        }
    }
    else
    {        
        std::vector<int> divisors;
        *numOutputPntsP = 1;

        for ( int i = 0; i < numInputPnts-1; i++ )
        {
            double dis = mdlVec_distanceXY ( &inputPntsP[i].cor, &inputPntsP[i+1].cor );
            int divisor = 1;

            if ( i == 0 || (inputPntsP[i+1].flg & DTM_C_PNTPUD) )
            {
                while ( dis > ( interval + AEC_C_TOL3 ) )
                {
                    dis /= 2.0;
                    divisor *= 2;
                };
            }

            (*numOutputPntsP) += divisor;
            divisors.push_back ( divisor );
        }
        
        *outputPntsPP = (CIVdtmpnt *) calloc ( *numOutputPntsP + 10, sizeof ( CIVdtmpnt ) );

        if ( *outputPntsPP )
        {
            memcpy ( &(*outputPntsPP)[0], &inputPntsP[0], sizeof ( CIVdtmpnt ) );
            *numOutputPntsP = 1;

            for ( int i = 0; i < numInputPnts-1; i++ )
            {
                if ( i == 0 || (inputPntsP[i+1].flg & DTM_C_PNTPUD) )
                {
                    int divisor = divisors[i];

                    if ( divisor > 1 )
                    {
                        CIVdtmpnt newPnt;
                        double xDis = inputPntsP[i+1].cor.x - inputPntsP[i].cor.x;
                        double yDis = inputPntsP[i+1].cor.y - inputPntsP[i].cor.y;
                        double zDis = inputPntsP[i+1].cor.z - inputPntsP[i].cor.z;

                        xDis /= (double)divisor;
                        yDis /= (double)divisor;
                        zDis /= (double)divisor;

                        memset ( &newPnt, 0, sizeof ( newPnt ) );
                        memcpy ( &newPnt.cor, &inputPntsP[i].cor, sizeof ( CIVdtmpnt ) );
                        newPnt.flg |= (DTM_C_PNTPUD|DTM_C_PNTDFY);

                        for ( int j = 0; j < divisor - 1; j++ )
                        {
                            newPnt.cor.x += xDis;
                            newPnt.cor.y += yDis;
                            newPnt.cor.z += zDis;

                            memcpy ( &(*outputPntsPP)[(*numOutputPntsP)++], &newPnt, sizeof ( CIVdtmpnt ) );
                        }
                    }
                }

                memcpy ( &(*outputPntsPP)[(*numOutputPntsP)++], &inputPntsP[i+1], sizeof ( CIVdtmpnt ) );
            }        
        }
        else
        {
            sts = DTM_M_MEMALF;
        }
    }

    return sts;
}



/*%-----------------------------------------------------------------------------
 FUNC: aecPolygon_horizontalLength
 DESC: Computes the length of the segments in an array (XY only).
 HIST: Original - tmi 11-Jan-1992
 MISC:
 KEYW: POLYGON POLYLINE LENGTH HORIZONTAL
-----------------------------------------------------------------------------%*/

static void aecDTM_featurePointsHorizontalLength
(
  double *lenP,                        /* <= computed hor. length             */
  long nvrt,                           /* => # vertices in poly.              */
  CIVdtmpnt *vrtP                       /* => polygon coordinates              */
)
{
  CIVdtmpnt tmp;
  long i;

  for ( *lenP = 0., i = 1; i < nvrt; i++ )
  {
    VSUBXY ( vrtP[i].cor, vrtP[i-1].cor, tmp.cor );
    *lenP += VLENXY ( tmp.cor );
  }
}



/*%-----------------------------------------------------------------------------
 FUNC: segElevation
 DESC:
 HIST: Original - dgc
 MISC: static
 KEYW:
-----------------------------------------------------------------------------%*/

static void segElevation
(
  CIVdtmpnt *pntP,                     /* <>                                  */
  CIVdtmpnt *v0,                       /* =>                                  */
  CIVdtmpnt *v1                        /* =>                                  */
)
{
  double z;
  if( fabs( v1->cor.x - v0->cor.x ) > AEC_C_TOL )
    z = ( pntP->cor.x - v0->cor.x ) / ( v1->cor.x - v0->cor.x ) * ( v1->cor.z - v0->cor.z ) + v0->cor.z;
  else if( fabs( v1->cor.y - v0->cor.y ) > AEC_C_TOL )
    z = ( pntP->cor.y - v0->cor.y ) / ( v1->cor.y - v0->cor.y ) * ( v1->cor.z - v0->cor.z ) + v0->cor.z;
  else
    z = v0->cor.z;
  pntP->cor.z = z;
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_compareMatches
 DESC: Match items are sorted in the hash table according to old address.
 HIST: Original - twl 8-Jul-1999
 MISC:
 KEYW: DTM HASH MATCH ITEM
-----------------------------------------------------------------------------%*/

static int aecDTM_compareMatches
(
  const void *p1,
  const void *p2
)
{
  MatchInfo *match1 = ( MatchInfo *)p1;
  MatchInfo *match2 = ( MatchInfo *)p2;
  int sts = 0;

  if ( match1->oldAddr < match2->oldAddr )
    sts = -1;
  else if (  match1->oldAddr >  match2->oldAddr )
    sts = 1;
  else
    sts = 0;

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_comparePntMatches
 DESC: Match items are sorted in the hash table according to old address.
 HIST: Original - twl 8-Jul-1999
 MISC:
 KEYW: DTM HASH MATCH ITEM
-----------------------------------------------------------------------------%*/

static int aecDTM_compareXYZmatches
(
  const void *p1,
  const void *p2
)
{
  struct CIVdtmpnt *pnt1P = ( struct CIVdtmpnt *)p1;
  struct CIVdtmpnt *pnt2P = ( struct CIVdtmpnt *)p2;
  int sts = 0;

  if ( VEQUAL( pnt1P->cor, pnt2P->cor, AEC_C_TOL ) )
    sts = 0;
  else if ( pnt1P->cor.x < pnt2P->cor.x )
    sts = -1;
  else if ( pnt1P->cor.x > pnt2P->cor.x )
    sts = 1;
  else if ( pnt1P->cor.y < pnt2P->cor.y )
    sts = -1;
  else if ( pnt1P->cor.y > pnt2P->cor.y )
    sts = 1;
  else if ( pnt1P->cor.z < pnt2P->cor.z )
    sts = -1;
  else if ( pnt1P->cor.z > pnt2P->cor.z )
    sts = 1;

  return ( sts );
}



/*%-----------------------------------------------------------------------------
 FUNC: aecDTM_tinMatchProcess
 DESC: Searches the hash table each point address contained by the triangle.  If
       found, the triangles point address is replaced with the new address
       returned from the hash table.
 HIST: Original - twl 8-Jul-1999
 MISC:
 KEYW: DTM HASH MATCH ITEM
-----------------------------------------------------------------------------%*/

static int aecDTM_tinMatchProcess
(
    void *dat,
    long,
    DPoint3d *,
    struct CIVdtmtin *tin,
    unsigned long
)
{
    MatchInfo matchItem;
    MatchInfo *matchItemP;
    struct MatchInfo *matchHashTblP = (struct MatchInfo *)dat;


    matchItem.oldAddr = tin->p1;
    if ( ( matchItemP = (::MatchInfo *) aecHash_find ( matchHashTblP, &matchItem ) ) != NULL )
        tin->p1 = matchItemP->newAddr;

    matchItem.oldAddr = tin->p2;
    if ( ( matchItemP = (::MatchInfo *) aecHash_find ( matchHashTblP, &matchItem ) ) != NULL )
        tin->p2 = matchItemP->newAddr;

    matchItem.oldAddr = tin->p3;
    if ( ( matchItemP = (::MatchInfo *) aecHash_find ( matchHashTblP, &matchItem ) ) != NULL )
        tin->p3 = matchItemP->newAddr;

    return ( SUCCESS );
}
