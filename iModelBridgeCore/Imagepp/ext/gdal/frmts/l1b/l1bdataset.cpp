/******************************************************************************
 * $Id: l1bdataset.cpp 20996 2010-10-28 18:38:15Z rouault $
 *
 * Project:  NOAA Polar Orbiter Level 1b Dataset Reader (AVHRR)
 * Purpose:  Can read NOAA-9(F)-NOAA-17(M) AVHRR datasets
 * Author:   Andrey Kiselev, dron@ak4719.spb.edu
 *
 * Some format info at: http://www.sat.dundee.ac.uk/noaa1b.html
 *
 ******************************************************************************
 * Copyright (c) 2002, Andrey Kiselev <dron@ak4719.spb.edu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#include "gdal_pam.h"
#include "cpl_string.h"

CPL_CVSID("$Id: l1bdataset.cpp 20996 2010-10-28 18:38:15Z rouault $");

CPL_C_START
void    GDALRegister_L1B(void);
CPL_C_END

enum {                  // File formats
    L1B_NONE,           // Not a L1B format
    L1B_NOAA9,          // NOAA-9/14
    L1B_NOAA15,         // NOAA-15/METOP-2
    L1B_NOAA15_NOHDR    // NOAA-15/METOP-2 without ARS header
};

enum {          // Spacecrafts:
    TIROSN,     // TIROS-N
    NOAA6,      // NOAA-6(A)
    NOAAB,      // NOAA-B
    NOAA7,      // NOAA-7(C)
    NOAA8,      // NOAA-8(E)
    NOAA9,      // NOAA-9(F)
    NOAA10,     // NOAA-10(G)
    NOAA11,     // NOAA-11(H)
    NOAA12,     // NOAA-12(D)
    NOAA13,     // NOAA-13(I)
    NOAA14,     // NOAA-14(J)
    NOAA15,     // NOAA-15(K)
    NOAA16,     // NOAA-16(L)
    NOAA17,     // NOAA-17(M)
    NOAA18,     // NOAA-18(N)
    METOP2      // METOP-2(A)
};

enum {          // Product types
    HRPT,
    LAC,
    GAC,
    FRAC
};

enum {          // Data format
    PACKED10BIT,
    UNPACKED8BIT,
    UNPACKED16BIT
};

enum {          // Receiving stations names:
    DU,         // Dundee, Scotland, UK
    GC,         // Fairbanks, Alaska, USA (formerly Gilmore Creek)
    HO,         // Honolulu, Hawaii, USA
    MO,         // Monterey, California, USA
    WE,         // Western Europe CDA, Lannion, France
    SO,         // SOCC (Satellite Operations Control Center), Suitland, Maryland, USA
    WI,         // Wallops Island, Virginia, USA
    SV,         // Svalbard, Norway
    UNKNOWN_STATION
};

enum {          // Data processing centers:
    CMS,        // Centre de Meteorologie Spatiale - Lannion, France
    DSS,        // Dundee Satellite Receiving Station - Dundee, Scotland, UK
    NSS,        // NOAA/NESDIS - Suitland, Maryland, USA
    UKM,        // United Kingdom Meteorological Office - Bracknell, England, UK
    UNKNOWN_CENTER
};

enum {          // AVHRR Earth location indication
    ASCEND,
    DESCEND
};

/************************************************************************/
/*                      AVHRR band widths                               */
/************************************************************************/

static const char *apszBandDesc[] =
{
    // NOAA-7 -- METOP-2 channels
    "AVHRR Channel 1:  0.58  micrometers -- 0.68 micrometers",
    "AVHRR Channel 2:  0.725 micrometers -- 1.10 micrometers",
    "AVHRR Channel 3:  3.55  micrometers -- 3.93 micrometers",
    "AVHRR Channel 4:  10.3  micrometers -- 11.3 micrometers",
    "AVHRR Channel 5:  11.5  micrometers -- 12.5 micrometers",  // not in NOAA-6,-8,-10
    // NOAA-13
    "AVHRR Channel 5:  11.4  micrometers -- 12.4 micrometers",
    // NOAA-15 -- METOP-2
    "AVHRR Channel 3A: 1.58  micrometers -- 1.64 micrometers",
    "AVHRR Channel 3B: 3.55  micrometers -- 3.93 micrometers"
    };

/************************************************************************/
/*      L1B file format related constants                               */
/************************************************************************/

#define L1B_DATASET_NAME_SIZE       42  // Length of the string containing
                                        // dataset name
#define L1B_NOAA9_HEADER_SIZE       122 // Terabit memory (TBM) header length
#define L1B_NOAA9_HDR_NAME_OFF      30  // Dataset name offset
#define L1B_NOAA9_HDR_SRC_OFF       70  // Receiving station name offset
#define L1B_NOAA9_HDR_CHAN_OFF      97  // Selected channels map offset
#define L1B_NOAA9_HDR_CHAN_SIZE     20  // Length of selected channels map
#define L1B_NOAA9_HDR_WORD_OFF      117 // Sensor data word size offset

#define L1B_NOAA15_HEADER_SIZE      512 // Archive Retrieval System (ARS)
                                        // header
#define L1B_NOAA15_HDR_CHAN_OFF     97  // Selected channels map offset
#define L1B_NOAA15_HDR_CHAN_SIZE    20  // Length of selected channels map
#define L1B_NOAA15_HDR_WORD_OFF     117 // Sensor data word size offset

#define L1B_NOAA9_HDR_REC_SIZE      146 // Length of header record
                                        // filled with the data
#define L1B_NOAA9_HDR_REC_ID_OFF    0   // Spacecraft ID offset
#define L1B_NOAA9_HDR_REC_PROD_OFF  1   // Data type offset
#define L1B_NOAA9_HDR_REC_DSTAT_OFF 34  // DACS status offset

#define L1B_NOAA15_HDR_REC_SIZE     992 // Length of header record
                                        // filled with the data
#define L1B_NOAA15_HDR_REC_SITE_OFF 0   // Dataset creation site ID offset
#define L1B_NOAA15_HDR_REC_NAME_OFF 22  // Dataset name
#define L1B_NOAA15_HDR_REC_ID_OFF   72  // Spacecraft ID offset
#define L1B_NOAA15_HDR_REC_PROD_OFF 76  // Data type offset
#define L1B_NOAA15_HDR_REC_STAT_OFF 116 // Instrument status offset
#define L1B_NOAA15_HDR_REC_SRC_OFF  154 // Receiving station name offset

#define DESIRED_GCPS_PER_LINE 11
#define DESIRED_LINES_OF_GCPS 20

// Fixed values used to scale GCPs coordinates in AVHRR records
#define L1B_NOAA9_GCP_SCALE     128.0
#define L1B_NOAA15_GCP_SCALE    10000.0

/************************************************************************/
/* ==================================================================== */
/*                      TimeCode (helper class)                         */
/* ==================================================================== */
/************************************************************************/

#define L1B_TIMECODE_LENGTH 100
class TimeCode {
    long        lYear;
    long        lDay;
    long        lMillisecond;
    char        pszString[L1B_TIMECODE_LENGTH];

  public:
    void SetYear(long year)
    {
        lYear = year;
    }
    void SetDay(long day)
    {
        lDay = day;
    }
    void SetMillisecond(long millisecond)
    {
        lMillisecond = millisecond;
    }
    char* PrintTime()
    {
        snprintf(pszString, L1B_TIMECODE_LENGTH,
                 "year: %ld, day: %ld, millisecond: %ld",
                 lYear, lDay, lMillisecond);
        return pszString;
    }
};
#undef L1B_TIMECODE_LENGTH

/************************************************************************/
/* ==================================================================== */
/*                              L1BDataset                              */
/* ==================================================================== */
/************************************************************************/

class L1BDataset : public GDALPamDataset
{
    friend class L1BRasterBand;

    char        pszRevolution[6]; // Five-digit number identifying spacecraft revolution
    int         eSource;        // Source of data (receiving station name)
    int         eProcCenter;    // Data processing center
    TimeCode    sStartTime;
    TimeCode    sStopTime;

    GDAL_GCP    *pasGCPList;
    int         nGCPCount;
    int         iGCPOffset;
    int         iGCPCodeOffset;
    int         nGCPsPerLine;
    int         eLocationIndicator, iGCPStart, iGCPStep;

    int         eL1BFormat;
    int         nBufferSize;
    int         eSpacecraftID;
    int         eProductType;   // LAC, GAC, HRPT, FRAC
    int         iDataFormat;    // 10-bit packed or 16-bit unpacked
    int         nRecordDataStart;
    int         nRecordDataEnd;
    int         nDataStartOffset;
    int         nRecordSize;
    GUInt32     iInstrumentStatus;
    GUInt32     iChannelsMask;

    char        *pszGCPProjection;

    VSILFILE   *fp;

    int         bFetchGeolocation;
    int         bGuessDataFormat;

    void        ProcessRecordHeaders();
    void        FetchGCPs( GDAL_GCP *, GByte *, int );
    void        FetchNOAA9TimeCode(TimeCode *, GByte *, int *);
    void        FetchNOAA15TimeCode(TimeCode *, GUInt16 *, int *);
    CPLErr      ProcessDatasetHeader();
    int         ComputeFileOffsets();
    
    static int  DetectFormat( GDALOpenInfo *poOpenInfo );

  public:
                L1BDataset( int );
                ~L1BDataset();
    
    virtual int GetGCPCount();
    virtual const char *GetGCPProjection();
    virtual const GDAL_GCP *GetGCPs();

    static int  Identify( GDALOpenInfo * );
    static GDALDataset *Open( GDALOpenInfo * );

};

/************************************************************************/
/* ==================================================================== */
/*                            L1BRasterBand                             */
/* ==================================================================== */
/************************************************************************/

class L1BRasterBand : public GDALPamRasterBand
{
    friend class L1BDataset;

  public:

                L1BRasterBand( L1BDataset *, int );
    
//    virtual double GetNoDataValue( int *pbSuccess = NULL );
    virtual CPLErr IReadBlock( int, int, void * );
};


/************************************************************************/
/*                           L1BRasterBand()                            */
/************************************************************************/

L1BRasterBand::L1BRasterBand( L1BDataset *poDS, int nBand )

{
    this->poDS = poDS;
    this->nBand = nBand;
    eDataType = GDT_UInt16;

    nBlockXSize = poDS->GetRasterXSize();
    nBlockYSize = 1;
}

/************************************************************************/
/*                             IReadBlock()                             */
/************************************************************************/

CPLErr L1BRasterBand::IReadBlock( int nBlockXOff, int nBlockYOff,
                                  void * pImage )
{
    L1BDataset  *poGDS = (L1BDataset *) poDS;

/* -------------------------------------------------------------------- */
/*      Seek to data.                                                   */
/* -------------------------------------------------------------------- */
    int iDataOffset = (poGDS->eLocationIndicator == DESCEND) ?
        poGDS->nDataStartOffset + nBlockYOff * poGDS->nRecordSize :
        poGDS->nDataStartOffset +
            (nRasterYSize - nBlockYOff - 1) * poGDS->nRecordSize;
    VSIFSeekL( poGDS->fp, iDataOffset, SEEK_SET );

/* -------------------------------------------------------------------- */
/*      Read data into the buffer.                                      */
/* -------------------------------------------------------------------- */
    GUInt16     *iScan = NULL;          // Unpacked 16-bit scanline buffer
    int         i, j;

    switch (poGDS->iDataFormat)
    {
        case PACKED10BIT:
            {
                // Read packed scanline
                GUInt32 *iRawScan = (GUInt32 *)CPLMalloc(poGDS->nRecordSize);
                VSIFReadL( iRawScan, 1, poGDS->nRecordSize, poGDS->fp );

                iScan = (GUInt16 *)CPLMalloc(poGDS->nBufferSize);
                j = 0;
                for(i = poGDS->nRecordDataStart / (int)sizeof(iRawScan[0]);
                    i < poGDS->nRecordDataEnd / (int)sizeof(iRawScan[0]); i++)
                {
                    GUInt32 iWord1 = CPL_MSBWORD32( iRawScan[i] );
                    GUInt32 iWord2 = iWord1 & 0x3FF00000;

                    iScan[j++] = (GUInt16) (iWord2 >> 20);
                    iWord2 = iWord1 & 0x000FFC00;
                    iScan[j++] = (GUInt16) (iWord2 >> 10);
                    iScan[j++] = (GUInt16) (iWord1 & 0x000003FF);
                }
                CPLFree(iRawScan);
            }
            break;
        case UNPACKED16BIT:
            {
                // Read unpacked scanline
                GUInt16 *iRawScan = (GUInt16 *)CPLMalloc(poGDS->nRecordSize);
                VSIFReadL( iRawScan, 1, poGDS->nRecordSize, poGDS->fp );

                iScan = (GUInt16 *)CPLMalloc(poGDS->GetRasterXSize()
                                             * poGDS->nBands * sizeof(GUInt16));
                for (i = 0; i < poGDS->GetRasterXSize() * poGDS->nBands; i++)
                {
                    iScan[i] = CPL_MSBWORD16( iRawScan[poGDS->nRecordDataStart
                        / (int)sizeof(iRawScan[0]) + i] );
                }
                CPLFree(iRawScan);
            }
            break;
        case UNPACKED8BIT:
            {
                // Read 8-bit unpacked scanline
                GByte   *byRawScan = (GByte *)CPLMalloc(poGDS->nRecordSize);
                VSIFReadL( byRawScan, 1, poGDS->nRecordSize, poGDS->fp );
                
                iScan = (GUInt16 *)CPLMalloc(poGDS->GetRasterXSize()
                                             * poGDS->nBands * sizeof(GUInt16));
                for (i = 0; i < poGDS->GetRasterXSize() * poGDS->nBands; i++)
                    iScan[i] = byRawScan[poGDS->nRecordDataStart
                        / (int)sizeof(byRawScan[0]) + i];
                CPLFree(byRawScan);
            }
            break;
        default: // NOTREACHED
            break;
    }
    
    int nBlockSize = nBlockXSize * nBlockYSize;
    if (poGDS->eLocationIndicator == DESCEND)
    {
        for( i = 0, j = 0; i < nBlockSize; i++ )
        {
            ((GUInt16 *) pImage)[i] = iScan[j + nBand - 1];
            j += poGDS->nBands;
        }
    }
    else
    {
        for ( i = nBlockSize - 1, j = 0; i >= 0; i-- )
        {
            ((GUInt16 *) pImage)[i] = iScan[j + nBand - 1];
            j += poGDS->nBands;
        }
    }
    
    CPLFree(iScan);
    return CE_None;
}

/************************************************************************/
/*                           L1BDataset()                               */
/************************************************************************/

L1BDataset::L1BDataset( int eL1BFormat )

{
    this->eL1BFormat = eL1BFormat;
    fp = NULL;
    nGCPCount = 0;
    pasGCPList = NULL;
    pszGCPProjection = CPLStrdup( "GEOGCS[\"WGS 72\",DATUM[\"WGS_1972\",SPHEROID[\"WGS 72\",6378135,298.26,AUTHORITY[\"EPSG\",7043]],TOWGS84[0,0,4.5,0,0,0.554,0.2263],AUTHORITY[\"EPSG\",6322]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",8901]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",9108]],AUTHORITY[\"EPSG\",4322]]" );
    nBands = 0;
    eLocationIndicator = DESCEND; // XXX: should be initialised
    iChannelsMask = 0;
    iInstrumentStatus = 0;
    bFetchGeolocation = FALSE;
    bGuessDataFormat = FALSE;
}

/************************************************************************/
/*                            ~L1BDataset()                             */
/************************************************************************/

L1BDataset::~L1BDataset()

{
    FlushCache();

    if( nGCPCount > 0 )
    {
        GDALDeinitGCPs( nGCPCount, pasGCPList );
        CPLFree( pasGCPList );
    }
    if ( pszGCPProjection )
        CPLFree( pszGCPProjection );
    if( fp != NULL )
        VSIFCloseL( fp );
}

/************************************************************************/
/*                            GetGCPCount()                             */
/************************************************************************/

int L1BDataset::GetGCPCount()

{
    return nGCPCount;
}

/************************************************************************/
/*                          GetGCPProjection()                          */
/************************************************************************/

const char *L1BDataset::GetGCPProjection()

{
    if( nGCPCount > 0 )
        return pszGCPProjection;
    else
        return "";
}

/************************************************************************/
/*                               GetGCPs()                              */
/************************************************************************/

const GDAL_GCP *L1BDataset::GetGCPs()
{
    return pasGCPList;
}

/************************************************************************/
/*      Fetch timecode from the record header (NOAA9-NOAA14 version)    */
/************************************************************************/

void L1BDataset::FetchNOAA9TimeCode( TimeCode *psTime, GByte *piRecordHeader,
                                     int *peLocationIndicator )
{
    GUInt32 lTemp;

    lTemp = ((piRecordHeader[2] >> 1) & 0x7F);
    psTime->SetYear((lTemp > 77) ? 
        (lTemp + 1900) : (lTemp + 2000)); // Avoid `Year 2000' problem
    psTime->SetDay((GUInt32)(piRecordHeader[2] & 0x01) << 8
                   | (GUInt32)piRecordHeader[3]);
    psTime->SetMillisecond( ((GUInt32)(piRecordHeader[4] & 0x07) << 24)
        | ((GUInt32)piRecordHeader[5] << 16)
        | ((GUInt32)piRecordHeader[6] << 8)
        | (GUInt32)piRecordHeader[7] );
    if ( peLocationIndicator )
    {
        *peLocationIndicator =
            ((piRecordHeader[8] & 0x02) == 0) ? ASCEND : DESCEND;
    }
}

/************************************************************************/
/*      Fetch timecode from the record header (NOAA15-METOP2 version)   */
/************************************************************************/

void L1BDataset::FetchNOAA15TimeCode( TimeCode *psTime,
                                      GUInt16 *piRecordHeader,
                                      int *peLocationIndicator )
{
#ifdef CPL_LSB
    GUInt16 iTemp;
    GUInt32 lTemp;

    iTemp = piRecordHeader[1];
    psTime->SetYear(CPL_SWAP16(iTemp));
    iTemp = piRecordHeader[2];
    psTime->SetDay(CPL_SWAP16(iTemp));
    lTemp = (GUInt32)CPL_SWAP16(piRecordHeader[4]) << 16 |
        (GUInt32)CPL_SWAP16(piRecordHeader[5]);
    psTime->SetMillisecond(lTemp);
    if ( peLocationIndicator )
    {
        // FIXME: hemisphere
        *peLocationIndicator =
            ((CPL_SWAP16(piRecordHeader[6]) & 0x8000) == 0) ? ASCEND : DESCEND;
    }
#else
    psTime->SetYear(piRecordHeader[1]);
    psTime->SetDay(piRecordHeader[2]);
    psTime->SetMillisecond( (GUInt32)piRecordHeader[4] << 16
                            | (GUInt32)piRecordHeader[5] );
    if ( peLocationIndicator )
    {
        *peLocationIndicator =
            ((piRecordHeader[6] & 0x8000) == 0) ? ASCEND : DESCEND;
    }
#endif
}

/************************************************************************/
/*      Fetch GCPs from the individual scanlines                        */
/************************************************************************/

void L1BDataset::FetchGCPs( GDAL_GCP *pasGCPList,
                            GByte *pabyRecordHeader, int iLine )
{
    // LAC and HRPT GCPs are tied to the center of pixel,
    // GAC ones are slightly displaced.
    double  dfDelta = (eProductType == GAC) ? 0.9 : 0.5;
    double  dfPixel = (eLocationIndicator == DESCEND) ?
        iGCPStart + dfDelta : (nRasterXSize - (iGCPStart + dfDelta));

    int     nGCPs;
    if ( eSpacecraftID <= NOAA14 )
    {
        // NOAA9-NOAA14 records have an indicator of number of working GCPs.
        // Number of good GCPs may be smaller than the total amount of points.
        nGCPs = (*(pabyRecordHeader + iGCPCodeOffset) < nGCPsPerLine) ?
            *(pabyRecordHeader + iGCPCodeOffset) : nGCPsPerLine;
#ifdef DEBUG
        CPLDebug( "L1B", "iGCPCodeOffset=%d, nGCPsPerLine=%d, nGoodGCPs=%d",
                  iGCPCodeOffset, nGCPsPerLine, nGCPs );
#endif
    }
    else
        nGCPs = nGCPsPerLine;

    pabyRecordHeader += iGCPOffset;

    while ( nGCPs-- )
    {
        if ( eSpacecraftID <= NOAA14 )
        {
            GInt16  nRawY = CPL_MSBWORD16( *(GInt16*)pabyRecordHeader );
            pabyRecordHeader += sizeof(GInt16);
            GInt16  nRawX = CPL_MSBWORD16( *(GInt16*)pabyRecordHeader );
            pabyRecordHeader += sizeof(GInt16);

            pasGCPList[nGCPCount].dfGCPY = nRawY / L1B_NOAA9_GCP_SCALE;
            pasGCPList[nGCPCount].dfGCPX = nRawX / L1B_NOAA9_GCP_SCALE;
        }
        else
        {
            GInt32  nRawY = CPL_MSBWORD32( *(GInt32*)pabyRecordHeader );
            pabyRecordHeader += sizeof(GInt32);
            GInt32  nRawX = CPL_MSBWORD32( *(GInt32*)pabyRecordHeader );
            pabyRecordHeader += sizeof(GInt32);

            pasGCPList[nGCPCount].dfGCPY = nRawY / L1B_NOAA15_GCP_SCALE;
            pasGCPList[nGCPCount].dfGCPX = nRawX / L1B_NOAA15_GCP_SCALE;
        }

        if ( pasGCPList[nGCPCount].dfGCPX < -180
             || pasGCPList[nGCPCount].dfGCPX > 180
             || pasGCPList[nGCPCount].dfGCPY < -90
             || pasGCPList[nGCPCount].dfGCPY > 90 )
            continue;

        pasGCPList[nGCPCount].dfGCPZ = 0.0;
        pasGCPList[nGCPCount].dfGCPPixel = dfPixel;
        dfPixel += (eLocationIndicator == DESCEND) ? iGCPStep : -iGCPStep;
        pasGCPList[nGCPCount].dfGCPLine =
            (double)( (eLocationIndicator == DESCEND) ?
                iLine : nRasterYSize - iLine - 1 ) + 0.5;
        nGCPCount++;
    }
}

/************************************************************************/
/*                      ProcessRecordHeaders()                          */
/************************************************************************/

void L1BDataset::ProcessRecordHeaders()
{
    void    *pRecordHeader = CPLMalloc( nRecordDataStart );

    VSIFSeekL(fp, nDataStartOffset, SEEK_SET);
    VSIFReadL(pRecordHeader, 1, nRecordDataStart, fp);

    if (eSpacecraftID <= NOAA14)
    {
        FetchNOAA9TimeCode( &sStartTime, (GByte *) pRecordHeader,
                            &eLocationIndicator );
    }
    else
    {
        FetchNOAA15TimeCode( &sStartTime, (GUInt16 *) pRecordHeader,
                             &eLocationIndicator );
    }

    VSIFSeekL( fp, nDataStartOffset + (nRasterYSize - 1) * nRecordSize,
              SEEK_SET);
    VSIFReadL( pRecordHeader, 1, nRecordDataStart, fp );

    if (eSpacecraftID <= NOAA14)
        FetchNOAA9TimeCode( &sStopTime, (GByte *) pRecordHeader, NULL );
    else
        FetchNOAA15TimeCode( &sStopTime, (GUInt16 *) pRecordHeader, NULL );

/* -------------------------------------------------------------------- */
/*      Pick a skip factor so that we will get roughly 20 lines         */
/*      worth of GCPs.  That should give respectible coverage on all    */
/*      but the longest swaths.                                         */
/* -------------------------------------------------------------------- */
    int nTargetLines = DESIRED_LINES_OF_GCPS;
    int nLineSkip = nRasterYSize / ( nTargetLines - 1 );
    
/* -------------------------------------------------------------------- */
/*      Initialize the GCP list.                                        */
/* -------------------------------------------------------------------- */
    pasGCPList = (GDAL_GCP *)CPLCalloc( nTargetLines * nGCPsPerLine,
                                        sizeof(GDAL_GCP) );
    GDALInitGCPs( nTargetLines * nGCPsPerLine, pasGCPList );

/* -------------------------------------------------------------------- */
/*      Fetch the GCPs for each selected line.  We force the last       */
/*      line sampled to be the last line in the dataset even if that    */
/*      leaves a bigger than expected gap.                              */
/* -------------------------------------------------------------------- */
    int iStep;

    for( iStep = 0; iStep < nTargetLines; iStep++ )
    {
        int nOrigGCPs = nGCPCount;
        int iLine;

        if( iStep == nTargetLines - 1 )
            iLine = nRasterXSize - 1;
        else
            iLine = nLineSkip * iStep;

        VSIFSeekL( fp, nDataStartOffset + iLine * nRecordSize, SEEK_SET );
        VSIFReadL( pRecordHeader, 1, nRecordDataStart, fp );

        FetchGCPs( pasGCPList, (GByte *)pRecordHeader, iLine );

/* -------------------------------------------------------------------- */
/*      We don't really want too many GCPs per line.  Downsample to     */
/*      11 per line.                                                    */
/* -------------------------------------------------------------------- */
        int iGCP;
        int nGCPsOnThisLine = nGCPCount - nOrigGCPs;
        int nDesiredGCPsPerLine = MIN(DESIRED_GCPS_PER_LINE,nGCPsOnThisLine);
        int nGCPStep = ( nDesiredGCPsPerLine > 1 ) ?
            ( nGCPsOnThisLine - 1 ) / ( nDesiredGCPsPerLine-1 ) : 1;
        int iSrcGCP = nOrigGCPs;
        int iDstGCP = nOrigGCPs;

        if( nGCPStep == 0 )
            nGCPStep = 1;

        for( iGCP = 0; iGCP < nDesiredGCPsPerLine; iGCP++ )
        {
            iSrcGCP += iGCP * nGCPStep;
            iDstGCP += iGCP;

            pasGCPList[iDstGCP].dfGCPX = pasGCPList[iSrcGCP].dfGCPX;
            pasGCPList[iDstGCP].dfGCPY = pasGCPList[iSrcGCP].dfGCPY;
            pasGCPList[iDstGCP].dfGCPPixel = pasGCPList[iSrcGCP].dfGCPPixel;
            pasGCPList[iDstGCP].dfGCPLine = pasGCPList[iSrcGCP].dfGCPLine;
        }

        nGCPCount = nOrigGCPs + nDesiredGCPsPerLine;
    }

    if( nGCPCount < nTargetLines * nGCPsPerLine )
    {
        GDALDeinitGCPs( nTargetLines * nGCPsPerLine - nGCPCount, 
                        pasGCPList + nGCPCount );
    }

    CPLFree( pRecordHeader );

/* -------------------------------------------------------------------- */
/*      Set fetched information as metadata records                     */
/* -------------------------------------------------------------------- */
    // Time of first scanline
    SetMetadataItem( "START",  sStartTime.PrintTime() );
    // Time of last scanline
    SetMetadataItem( "STOP",  sStopTime.PrintTime() );
    // AVHRR Earth location indication

    switch( eLocationIndicator )
    {
        case ASCEND:
            SetMetadataItem( "LOCATION", "Ascending" );
            break;
        case DESCEND:
        default:
            SetMetadataItem( "LOCATION", "Descending" );
            break;
    }

}

/************************************************************************/
/*                      ProcessDatasetHeader()                          */
/************************************************************************/

CPLErr L1BDataset::ProcessDatasetHeader()
{
    char    szDatasetName[L1B_DATASET_NAME_SIZE + 1];

    if ( eL1BFormat == L1B_NOAA9 )
    {
        GByte   abyTBMHeader[L1B_NOAA9_HEADER_SIZE];

        if ( VSIFSeekL( fp, 0, SEEK_SET ) < 0
             || VSIFReadL( abyTBMHeader, 1, L1B_NOAA9_HEADER_SIZE,
                           fp ) < L1B_NOAA9_HEADER_SIZE )
        {
            CPLDebug( "L1B", "Can't read NOAA-9/14 TBM header." );
            return CE_Failure;
        }

        // Fetch dataset name. NOAA-9/14 datasets contain the names in TBM
        // header only, so read it there.
        memcpy( szDatasetName, abyTBMHeader + L1B_NOAA9_HDR_NAME_OFF,
                L1B_DATASET_NAME_SIZE );
        szDatasetName[L1B_DATASET_NAME_SIZE] = '\0';

        // Determine processing center where the dataset was created
        if ( EQUALN((const char *)abyTBMHeader
                    + L1B_NOAA9_HDR_NAME_OFF, "CMS", 3) )
             eProcCenter = CMS;
        else if ( EQUALN((const char *)abyTBMHeader
                         + L1B_NOAA9_HDR_NAME_OFF, "DSS", 3) )
             eProcCenter = DSS;
        else if ( EQUALN((const char *)abyTBMHeader
                         + L1B_NOAA9_HDR_NAME_OFF, "NSS", 3) )
             eProcCenter = NSS;
        else if ( EQUALN((const char *)abyTBMHeader
                         + L1B_NOAA9_HDR_NAME_OFF, "UKM", 3) )
             eProcCenter = UKM;
        else
             eProcCenter = UNKNOWN_CENTER;

        // Determine number of bands
        int     i;
        for ( i = 0; i < L1B_NOAA9_HDR_CHAN_SIZE; i++ )
        {
            if ( abyTBMHeader[L1B_NOAA9_HDR_CHAN_OFF + i] == 1
                 || abyTBMHeader[L1B_NOAA9_HDR_CHAN_OFF + i] == 'Y' )
            {
                nBands++;
                iChannelsMask |= (1 << i);
            }
        }
        if ( nBands == 0 || nBands > 5 )
        {
            nBands = 5;
            iChannelsMask = 0x1F;
        }

        // Determine data format (10-bit packed or 8/16-bit unpacked)
        if ( EQUALN((const char *)abyTBMHeader + L1B_NOAA9_HDR_WORD_OFF,
                    "10", 2) )
            iDataFormat = PACKED10BIT;
        else if ( EQUALN((const char *)abyTBMHeader + L1B_NOAA9_HDR_WORD_OFF,
                         "16", 2) )
            iDataFormat = UNPACKED16BIT;
        else if ( EQUALN((const char *)abyTBMHeader + L1B_NOAA9_HDR_WORD_OFF,
                         "08", 2) )
            iDataFormat = UNPACKED8BIT;
        else if ( EQUALN((const char *)abyTBMHeader + L1B_NOAA9_HDR_WORD_OFF,
                         "  ", 2)
                  || abyTBMHeader[L1B_NOAA9_HDR_WORD_OFF] == '\0' )
            /* Empty string can be found in the following samples : 
                http://www2.ncdc.noaa.gov/docs/podug/data/avhrr/franh.1b (10 bit)
                http://www2.ncdc.noaa.gov/docs/podug/data/avhrr/frang.1b (10 bit)
                http://www2.ncdc.noaa.gov/docs/podug/data/avhrr/calfilel.1b (16 bit)
                http://www2.ncdc.noaa.gov/docs/podug/data/avhrr/rapnzg.1b (16 bit)
                ftp://ftp.sat.dundee.ac.uk/misc/testdata/noaa12/hrptnoaa1b.dat (10 bit)
            */
            bGuessDataFormat = TRUE;
        else
        {
#ifdef DEBUG
            CPLDebug( "L1B", "Unknown data format \"%.2s\".",
                      abyTBMHeader + L1B_NOAA9_HDR_WORD_OFF );
#endif
            return CE_Failure;
        }

        // Now read the dataset header record
        GByte   abyRecHeader[L1B_NOAA9_HDR_REC_SIZE];
        if ( VSIFSeekL( fp, L1B_NOAA9_HEADER_SIZE, SEEK_SET ) < 0
             || VSIFReadL( abyRecHeader, 1, L1B_NOAA9_HDR_REC_SIZE,
                           fp ) < L1B_NOAA9_HDR_REC_SIZE )
        {
            CPLDebug( "L1B", "Can't read NOAA-9/14 record header." );
            return CE_Failure;
        }

        // Determine the spacecraft name
        switch ( abyRecHeader[L1B_NOAA9_HDR_REC_ID_OFF] )
        {
            /* FIXME: use time code to determine TIROS-N, because the SatID
             * identical to NOAA-11
             * case 1:
                eSpacecraftID = TIROSN;
                break;
            case 2:
                eSpacecraftID = NOAA6;
                break;*/
            case 4:
                eSpacecraftID = NOAA7;
                break;
            case 6:
                eSpacecraftID = NOAA8;
                break;
            case 7:
                eSpacecraftID = NOAA9;
                break;
            case 8:
                eSpacecraftID = NOAA10;
                break;
            case 1:
                eSpacecraftID = NOAA11;
                break;
            case 5:
                eSpacecraftID = NOAA12;
                break;
            case 2:
                eSpacecraftID = NOAA13;
                break;
            case 3:
                eSpacecraftID = NOAA14;
                break;
            default:
#ifdef DEBUG
                CPLDebug( "L1B", "Unknown spacecraft ID \"%d\".",
                          abyRecHeader[L1B_NOAA9_HDR_REC_ID_OFF] );
#endif
                return CE_Failure;
        }

        // Determine the product data type
        int iWord = abyRecHeader[L1B_NOAA9_HDR_REC_PROD_OFF] >> 4;
        switch ( iWord )
        {
            case 1:
                eProductType = LAC;
                break;
            case 2:
                eProductType = GAC;
                break;
            case 3:
                eProductType = HRPT;
                break;
            default:
#ifdef DEBUG
                CPLDebug( "L1B", "Unknown product type \"%d\".", iWord );
#endif
                return CE_Failure;
        }

        // Determine receiving station name
        iWord = ( abyRecHeader[L1B_NOAA9_HDR_REC_DSTAT_OFF] & 0x60 ) >> 5;
        switch( iWord )
        {
            case 1:
                eSource = GC;
                break;
            case 2:
                eSource = WI;
                break;
            case 3:
                eSource = SO;
                break;
            default:
                eSource = UNKNOWN_STATION;
                break;
        }
    }

    else if ( eL1BFormat == L1B_NOAA15 || eL1BFormat == L1B_NOAA15_NOHDR )
    {
        if ( eL1BFormat == L1B_NOAA15 )
        {
            GByte   abyARSHeader[L1B_NOAA15_HEADER_SIZE];

            if ( VSIFSeekL( fp, 0, SEEK_SET ) < 0
                 || VSIFReadL( abyARSHeader, 1, L1B_NOAA15_HEADER_SIZE,
                               fp ) < L1B_NOAA15_HEADER_SIZE )
            {
                CPLDebug( "L1B", "Can't read NOAA-15 ARS header." );
                return CE_Failure;
            }

            // Determine number of bands
            int     i;
            for ( i = 0; i < L1B_NOAA15_HDR_CHAN_SIZE; i++ )
            {
                if ( abyARSHeader[L1B_NOAA15_HDR_CHAN_OFF + i] == 1
                     || abyARSHeader[L1B_NOAA15_HDR_CHAN_OFF + i] == 'Y' )
                {
                    nBands++;
                    iChannelsMask |= (1 << i);
                }
            }
            if ( nBands == 0 || nBands > 5 )
            {
                nBands = 5;
                iChannelsMask = 0x1F;
            }

            // Determine data format (10-bit packed or 8/16-bit unpacked)
            if ( EQUALN((const char *)abyARSHeader + L1B_NOAA15_HDR_WORD_OFF,
                        "10", 2) )
                iDataFormat = PACKED10BIT;
            else if ( EQUALN((const char *)abyARSHeader + L1B_NOAA15_HDR_WORD_OFF,
                             "16", 2) )
                iDataFormat = UNPACKED16BIT;
            else if ( EQUALN((const char *)abyARSHeader + L1B_NOAA15_HDR_WORD_OFF,
                             "08", 2) )
                iDataFormat = UNPACKED8BIT;
            else
            {
#ifdef DEBUG
                CPLDebug( "L1B", "Unknown data format \"%.2s\".",
                          abyARSHeader + L1B_NOAA9_HDR_WORD_OFF );
#endif
                return CE_Failure;
            }
        }
        else
        {
            nBands = 5;
            iChannelsMask = 0x1F;
            iDataFormat = PACKED10BIT;
        }

        // Now read the dataset header record
        GByte   abyRecHeader[L1B_NOAA15_HDR_REC_SIZE];
        if ( VSIFSeekL( fp,
                        (eL1BFormat == L1B_NOAA15) ? L1B_NOAA15_HEADER_SIZE : 0,
                        SEEK_SET ) < 0
             || VSIFReadL( abyRecHeader, 1, L1B_NOAA15_HDR_REC_SIZE,
                           fp ) < L1B_NOAA15_HDR_REC_SIZE )
        {
            CPLDebug( "L1B", "Can't read NOAA-9/14 record header." );
            return CE_Failure;
        }

        // Fetch dataset name
        memcpy( szDatasetName, abyRecHeader + L1B_NOAA15_HDR_REC_NAME_OFF,
                L1B_DATASET_NAME_SIZE );
        szDatasetName[L1B_DATASET_NAME_SIZE] = '\0';

        // Determine processing center where the dataset was created
        if ( EQUALN((const char *)abyRecHeader
                    + L1B_NOAA15_HDR_REC_SITE_OFF, "CMS", 3) )
             eProcCenter = CMS;
        else if ( EQUALN((const char *)abyRecHeader
                         + L1B_NOAA15_HDR_REC_SITE_OFF, "DSS", 3) )
             eProcCenter = DSS;
        else if ( EQUALN((const char *)abyRecHeader
                         + L1B_NOAA15_HDR_REC_SITE_OFF, "NSS", 3) )
             eProcCenter = NSS;
        else if ( EQUALN((const char *)abyRecHeader
                         + L1B_NOAA15_HDR_REC_SITE_OFF, "UKM", 3) )
             eProcCenter = UKM;
        else
             eProcCenter = UNKNOWN_CENTER;

        // Determine the spacecraft name
        int iWord = CPL_MSBWORD16( *(GUInt16 *)
            (abyRecHeader + L1B_NOAA15_HDR_REC_ID_OFF) );
        switch ( iWord )
        {
            case 2:
                eSpacecraftID = NOAA16;
                break;
            case 4:
                eSpacecraftID = NOAA15;
                break;
            case 6:
                eSpacecraftID = NOAA17;
                break;
            case 7:
                eSpacecraftID = NOAA18;
                break;
            /* FIXME: find appropriate samples and test these two cases:
             * case 8:
                eSpacecraftID = NOAA-N';
                break;
            case 11:
                eSpacecraftID = METOP-1;
                break;*/
            case 12:
            case 14:    // METOP simulator (code used in AAPP format)
                eSpacecraftID = METOP2;
                break;
            default:
#ifdef DEBUG
                CPLDebug( "L1B", "Unknown spacecraft ID \"%d\".", iWord );
#endif
                return CE_Failure;
        }

        // Determine the product data type
        iWord = CPL_MSBWORD16( *(GUInt16 *)
            (abyRecHeader + L1B_NOAA15_HDR_REC_PROD_OFF) );
        switch ( iWord )
        {
            case 1:
                eProductType = LAC;
                break;
            case 2:
                eProductType = GAC;
                break;
            case 3:
                eProductType = HRPT;
                break;
            case 4:     // XXX: documentation specifies the code '4'
            case 13:    // for FRAC but real datasets contain '13 here.'
                eProductType = FRAC;
                break;
            default:
#ifdef DEBUG
                CPLDebug( "L1B", "Unknown product type \"%d\".", iWord );
#endif
                return CE_Failure;
        }

        // Fetch hinstrument status. Helps to determine whether we have
        // 3A or 3B channel in the dataset.
        iInstrumentStatus = CPL_MSBWORD32( *(GUInt32 *)
            (abyRecHeader + L1B_NOAA15_HDR_REC_STAT_OFF) );

        // Determine receiving station name
        iWord = CPL_MSBWORD16( *(GUInt16 *)
            (abyRecHeader + L1B_NOAA15_HDR_REC_SRC_OFF) );
        switch( iWord )
        {
            case 1:
                eSource = GC;
                break;
            case 2:
                eSource = WI;
                break;
            case 3:
                eSource = SO;
                break;
            case 4:
                eSource = SV;
                break;
            case 5:
                eSource = MO;
                break;
            default:
                eSource = UNKNOWN_STATION;
                break;
        }
    }
    else
        return CE_Failure;

/* -------------------------------------------------------------------- */
/*      Set fetched information as metadata records                     */
/* -------------------------------------------------------------------- */
    const char *pszText;

    SetMetadataItem( "DATASET_NAME",  szDatasetName );

    switch( eSpacecraftID )
    {
        case TIROSN:
            pszText = "TIROS-N";
            break;
        case NOAA6:
            pszText = "NOAA-6(A)";
            break;
        case NOAAB:
            pszText = "NOAA-B";
            break;
        case NOAA7:
            pszText = "NOAA-7(C)";
            break;
        case NOAA8:
            pszText = "NOAA-8(E)";
            break;
        case NOAA9:
            pszText = "NOAA-9(F)";
            break;
        case NOAA10:
            pszText = "NOAA-10(G)";
            break;
        case NOAA11:
            pszText = "NOAA-11(H)";
            break;
        case NOAA12:
            pszText = "NOAA-12(D)";
            break;
        case NOAA13:
            pszText = "NOAA-13(I)";
            break;
        case NOAA14:
            pszText = "NOAA-14(J)";
            break;
        case NOAA15:
            pszText = "NOAA-15(K)";
            break;
        case NOAA16:
            pszText = "NOAA-16(L)";
            break;
        case NOAA17:
            pszText = "NOAA-17(M)";
            break;
        case NOAA18:
            pszText = "NOAA-18(N)";
            break;
        case METOP2:
            pszText = "METOP-2(A)";
            break;
        default:
            pszText = "Unknown";
            break;
    }
    SetMetadataItem( "SATELLITE",  pszText );

    switch( eProductType )
    {
        case LAC:
            pszText = "AVHRR LAC";
            break;
        case HRPT:
            pszText = "AVHRR HRPT";
            break;
        case GAC:
            pszText = "AVHRR GAC";
            break;
        case FRAC:
            pszText = "AVHRR FRAC";
            break;
        default:
            pszText = "Unknown";
            break;
    }
    SetMetadataItem( "DATA_TYPE",  pszText );

    // Get revolution number as string, we don't need this value for processing
    char    szRevolution[6];
    memcpy( szRevolution, szDatasetName + 32, 5 );
    szRevolution[5] = '\0';
    SetMetadataItem( "REVOLUTION",  szRevolution );

    switch( eSource )
    {
        case DU:
            pszText = "Dundee, Scotland, UK";
            break;
        case GC:
            pszText = "Fairbanks, Alaska, USA (formerly Gilmore Creek)";
            break;
        case HO:
            pszText = "Honolulu, Hawaii, USA";
            break;
        case MO:
            pszText = "Monterey, California, USA";
            break;
        case WE:
            pszText = "Western Europe CDA, Lannion, France";
            break;
        case SO:
            pszText = "SOCC (Satellite Operations Control Center), Suitland, Maryland, USA";
            break;
        case WI:
            pszText = "Wallops Island, Virginia, USA";
            break;
        default:
            pszText = "Unknown receiving station";
            break;
    }
    SetMetadataItem( "SOURCE",  pszText );

    switch( eProcCenter )
    {
        case CMS:
            pszText = "Centre de Meteorologie Spatiale - Lannion, France";
            break;
        case DSS:
            pszText = "Dundee Satellite Receiving Station - Dundee, Scotland, UK";
            break;
        case NSS:
            pszText = "NOAA/NESDIS - Suitland, Maryland, USA";
            break;
        case UKM:
            pszText = "United Kingdom Meteorological Office - Bracknell, England, UK";
            break;
        default:
            pszText = "Unknown processing center";
            break;
    }
    SetMetadataItem( "PROCESSING_CENTER",  pszText );
    
    return CE_None;
}

/************************************************************************/
/*                        ComputeFileOffsets()                          */
/************************************************************************/

int L1BDataset::ComputeFileOffsets()
{
    switch( eProductType )
    {
        case HRPT:
        case LAC:
        case FRAC:
            nRasterXSize = 2048;
            nBufferSize = 20484;
            iGCPStart = 25;
            iGCPStep = 40;
            nGCPsPerLine = 51;
            if ( eL1BFormat == L1B_NOAA9 )
            {
                if (iDataFormat == PACKED10BIT)
                {
                    nRecordSize = 14800;
                    nRecordDataEnd = 14104;
                }
                else if (iDataFormat == UNPACKED16BIT)
                {
                    switch(nBands)
                    {
                        case 1:
                        nRecordSize = 4544;
                        nRecordDataEnd = 4544;
                        break;
                        case 2:
                        nRecordSize = 8640;
                        nRecordDataEnd = 8640;
                        break;
                        case 3:
                        nRecordSize = 12736;
                        nRecordDataEnd = 12736;
                        break;
                        case 4:
                        nRecordSize = 16832;
                        nRecordDataEnd = 16832;
                        break;
                        case 5:
                        nRecordSize = 20928;
                        nRecordDataEnd = 20928;
                        break;
                    }
                }
                else // UNPACKED8BIT
                {
                    switch(nBands)
                    {
                        case 1:
                        nRecordSize = 2496;
                        nRecordDataEnd = 2496;
                        break;
                        case 2:
                        nRecordSize = 4544;
                        nRecordDataEnd = 4544;
                        break;
                        case 3:
                        nRecordSize = 6592;
                        nRecordDataEnd = 6592;
                        break;
                        case 4:
                        nRecordSize = 8640;
                        nRecordDataEnd = 8640;
                        break;
                        case 5:
                        nRecordSize = 10688;
                        nRecordDataEnd = 10688;
                        break;
                    }
                }
                nDataStartOffset = nRecordSize + L1B_NOAA9_HEADER_SIZE;
                nRecordDataStart = 448;
                iGCPCodeOffset = 52;
                iGCPOffset = 104;
            }

            else if ( eL1BFormat == L1B_NOAA15
                      || eL1BFormat == L1B_NOAA15_NOHDR )
            {
                if (iDataFormat == PACKED10BIT)
                {
                    nRecordSize = 15872;
                    nRecordDataEnd = 14920;
                }
                else if (iDataFormat == UNPACKED16BIT)
                {
                    switch(nBands)
                    {
                        case 1:
                        nRecordSize = 6144;
                        nRecordDataEnd = 5360;
                        break;
                        case 2:
                        nRecordSize = 10240;
                        nRecordDataEnd = 9456;
                        break;
                        case 3:
                        nRecordSize = 14336;
                        nRecordDataEnd = 13552;
                        break;
                        case 4:
                        nRecordSize = 18432;
                        nRecordDataEnd = 17648;
                        break;
                        case 5:
                        nRecordSize = 22528;
                        nRecordDataEnd = 21744;
                        break;
                    }
                }
                else // UNPACKED8BIT
                {
                    switch(nBands)
                    {
                        case 1:
                        nRecordSize = 4096;
                        nRecordDataEnd = 3312;
                        break;
                        case 2:
                        nRecordSize = 6144;
                        nRecordDataEnd = 5360;
                        break;
                        case 3:
                        nRecordSize = 8192;
                        nRecordDataEnd = 7408;
                        break;
                        case 4:
                        nRecordSize = 10240;
                        nRecordDataEnd = 9456;
                        break;
                        case 5:
                        nRecordSize = 12288;
                        nRecordDataEnd = 11504;
                        break;
                    }
                }
                nDataStartOffset = ( eL1BFormat == L1B_NOAA15_NOHDR ) ?
                    nRecordDataEnd : nRecordSize + L1B_NOAA15_HEADER_SIZE;
                nRecordDataStart = 1264;
                iGCPCodeOffset = 0; // XXX: not exist for NOAA15?
                iGCPOffset = 640;
            }
            else
                return 0;
            break;

        case GAC:
            nRasterXSize = 409;
            nBufferSize = 4092;
            iGCPStart = 5; // FIXME: depends of scan direction
            iGCPStep = 8;
            nGCPsPerLine = 51;
            if (  eL1BFormat == L1B_NOAA9 )
            {
                if (iDataFormat == PACKED10BIT)
                {
                    nRecordSize = 3220;
                    nRecordDataEnd = 3176;
                }
                else if (iDataFormat == UNPACKED16BIT)
                    switch(nBands)
                    {
                        case 1:
                        nRecordSize = 1268;
                        nRecordDataEnd = 1266;
                        break;
                        case 2:
                        nRecordSize = 2084;
                        nRecordDataEnd = 2084;
                        break;
                        case 3:
                        nRecordSize = 2904;
                        nRecordDataEnd = 2902;
                        break;
                        case 4:
                        nRecordSize = 3720;
                        nRecordDataEnd = 3720;
                        break;
                        case 5:
                        nRecordSize = 4540;
                        nRecordDataEnd = 4538;
                        break;
                    }
                else // UNPACKED8BIT
                {
                    switch(nBands)
                    {
                        case 1:
                        nRecordSize = 860;
                        nRecordDataEnd = 858;
                        break;
                        case 2:
                        nRecordSize = 1268;
                        nRecordDataEnd = 1266;
                        break;
                        case 3:
                        nRecordSize = 1676;
                        nRecordDataEnd = 1676;
                        break;
                        case 4:
                        nRecordSize = 2084;
                        nRecordDataEnd = 2084;
                        break;
                        case 5:
                        nRecordSize = 2496;
                        nRecordDataEnd = 2494;
                        break;
                    }
                }
                nDataStartOffset = nRecordSize * 2 + L1B_NOAA9_HEADER_SIZE;
                nRecordDataStart = 448;
                iGCPCodeOffset = 52;
                iGCPOffset = 104;
            }

            else if ( eL1BFormat == L1B_NOAA15
                      || eL1BFormat == L1B_NOAA15_NOHDR )
            {
                if (iDataFormat == PACKED10BIT)
                {
                    nRecordSize = 4608;
                    nRecordDataEnd = 3992;
                }
                else if (iDataFormat == UNPACKED16BIT)
                {
                    switch(nBands)
                    {
                        case 1:
                        nRecordSize = 2360;
                        nRecordDataEnd = 2082;
                        break;
                        case 2:
                        nRecordSize = 3176;
                        nRecordDataEnd = 2900;
                        break;
                        case 3:
                        nRecordSize = 3992;
                        nRecordDataEnd = 3718;
                        break;
                        case 4:
                        nRecordSize = 4816;
                        nRecordDataEnd = 4536;
                        break;
                        case 5:
                        nRecordSize = 5632;
                        nRecordDataEnd = 5354;
                        break;
                    }
                }
                else // UNPACKED8BIT
                {
                    switch(nBands)
                    {
                        case 1:
                        nRecordSize = 1952;
                        nRecordDataEnd = 1673;
                        break;
                        case 2:
                        nRecordSize = 2360;
                        nRecordDataEnd = 2082;
                        break;
                        case 3:
                        nRecordSize = 2768;
                        nRecordDataEnd = 2491;
                        break;
                        case 4:
                        nRecordSize = 3176;
                        nRecordDataEnd = 2900;
                        break;
                        case 5:
                        nRecordSize = 3584;
                        nRecordDataEnd = 3309;
                        break;
                    }
                }
                nDataStartOffset = ( eL1BFormat == L1B_NOAA15_NOHDR ) ?
                    nRecordDataEnd : nRecordSize + L1B_NOAA15_HEADER_SIZE;
                nRecordDataStart = 1264;
                iGCPCodeOffset = 0; // XXX: not exist for NOAA15?
                iGCPOffset = 640;
            }
            else
                return 0;
        break;
        default:
            return 0;
    }

    return 1;
}

/************************************************************************/
/*                           DetectFormat()                             */
/************************************************************************/

int L1BDataset::DetectFormat( GDALOpenInfo *poOpenInfo )

{
    GByte* pabyHeader = poOpenInfo->pabyHeader;
    if (pabyHeader == NULL || poOpenInfo->nHeaderBytes < L1B_NOAA9_HEADER_SIZE)
        return L1B_NONE;

    // We will try the NOAA-15 and later formats first
    if ( poOpenInfo->nHeaderBytes > L1B_NOAA15_HEADER_SIZE + 61
         && *(pabyHeader + L1B_NOAA15_HEADER_SIZE + 25) == '.'
         && *(pabyHeader + L1B_NOAA15_HEADER_SIZE + 30) == '.'
         && *(pabyHeader + L1B_NOAA15_HEADER_SIZE + 33) == '.'
         && *(pabyHeader + L1B_NOAA15_HEADER_SIZE + 40) == '.'
         && *(pabyHeader + L1B_NOAA15_HEADER_SIZE + 46) == '.'
         && *(pabyHeader + L1B_NOAA15_HEADER_SIZE + 52) == '.'
         && *(pabyHeader + L1B_NOAA15_HEADER_SIZE + 61) == '.' )
        return L1B_NOAA15;

    // Next try the NOAA-9/14 formats
    if ( *(pabyHeader + 8 + 25) == '.'
         && *(pabyHeader + 8 + 30) == '.'
         && *(pabyHeader + 8 + 33) == '.'
         && *(pabyHeader + 8 + 40) == '.'
         && *(pabyHeader + 8 + 46) == '.'
         && *(pabyHeader + 8 + 52) == '.'
         && *(pabyHeader + 8 + 61) == '.' )
        return L1B_NOAA9;

    // Finally try the AAPP formats 
    if ( *(pabyHeader + 25) == '.'
         && *(pabyHeader + 30) == '.'
         && *(pabyHeader + 33) == '.'
         && *(pabyHeader + 40) == '.'
         && *(pabyHeader + 46) == '.'
         && *(pabyHeader + 52) == '.'
         && *(pabyHeader + 61) == '.' )
        return L1B_NOAA15_NOHDR;

    return L1B_NONE;
}

/************************************************************************/
/*                              Identify()                              */
/************************************************************************/

int L1BDataset::Identify( GDALOpenInfo *poOpenInfo )

{
    if( poOpenInfo->fp == NULL )
        return FALSE;

    if ( DetectFormat(poOpenInfo) == L1B_NONE )
        return FALSE;

    return TRUE;
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

GDALDataset *L1BDataset::Open( GDALOpenInfo * poOpenInfo )

{
    int     eL1BFormat = DetectFormat( poOpenInfo );
    if ( eL1BFormat == L1B_NONE )
        return NULL;
        
/* -------------------------------------------------------------------- */
/*      Confirm the requested access is supported.                      */
/* -------------------------------------------------------------------- */
    if( poOpenInfo->eAccess == GA_Update )
    {
        CPLError( CE_Failure, CPLE_NotSupported, 
                  "The L1B driver does not support update access to existing"
                  " datasets.\n" );
        return NULL;
    }
    
#if 0
    Geolocation
    if ( EQUAL( poOpenInfo->pszFilename, "L1BGCPS:" ) )
        bFetchGeolocation = TRUE;
#endif

/* -------------------------------------------------------------------- */
/*      Create a corresponding GDALDataset.                             */
/* -------------------------------------------------------------------- */
    L1BDataset  *poDS;
    VSIStatBuf  sStat;
    const char  *pszFilename = poOpenInfo->pszFilename;

    poDS = new L1BDataset( eL1BFormat );

    poDS->fp = VSIFOpenL( poOpenInfo->pszFilename, "rb" );
    if ( !poDS->fp )
    {
        CPLDebug( "L1B", "Can't open file \"%s\".", poOpenInfo->pszFilename );
        goto bad;
    }
    
/* -------------------------------------------------------------------- */
/*      Read the header.                                                */
/* -------------------------------------------------------------------- */
    if ( poDS->ProcessDatasetHeader() != CE_None )
    {
        CPLDebug( "L1B", "Error reading L1B record header." );
        goto bad;
    }

    CPLStat(pszFilename, &sStat);

    if ( poDS->bGuessDataFormat )
    {
        int nTempYSize;
        GUInt16 nScanlineNumber;
        int j;

        /* If the data format is unspecified, try each one of the 3 known data formats */
        /* It is considered valid when the spacing between the first 5 scanline numbers */
        /* is a constant */

        for(j=0;j<3;j++)
        {
            poDS->iDataFormat = PACKED10BIT + j;
            if (!poDS->ComputeFileOffsets())
                goto bad;

            nTempYSize = (sStat.st_size - poDS->nDataStartOffset) / poDS->nRecordSize;
            if (nTempYSize < 5)
                continue;

            int nLastScanlineNumber = 0;
            int nDiffLine = 0;
            int i;
            for (i=0;i<5;i++)
            {
                nScanlineNumber = 0;

                VSIFSeekL(poDS->fp, poDS->nDataStartOffset + i * poDS->nRecordSize, SEEK_SET);
                VSIFReadL(&nScanlineNumber, 1, 2, poDS->fp);
#ifdef CPL_LSB
                CPL_SWAP16PTR( &nScanlineNumber );
#endif
                if (i == 1)
                {
                    nDiffLine = nScanlineNumber - nLastScanlineNumber;
                    if (nDiffLine == 0)
                        break;
                }
                else if (i > 1)
                {
                    if (nDiffLine != nScanlineNumber - nLastScanlineNumber)
                        break;
                }

                nLastScanlineNumber = nScanlineNumber;
            }

            if (i == 5)
            {
                CPLDebug("L1B", "Guessed data format : %s",
                         (poDS->iDataFormat == PACKED10BIT) ? "10" :
                         (poDS->iDataFormat == UNPACKED8BIT) ? "08" : "16");
                break;
            }
        }

        if (j == 3)
        {
            CPLError(CE_Failure, CPLE_AppDefined, "Could not guess data format of L1B product");
            goto bad;
        }
    }
    else
    {
        if (!poDS->ComputeFileOffsets())
            goto bad;
    }

    // Compute number of lines dinamycally, so we can read partially
    // downloaded files
    poDS->nRasterYSize =
        (sStat.st_size - poDS->nDataStartOffset) / poDS->nRecordSize;

/* -------------------------------------------------------------------- */
/*      Create band information objects.                                */
/* -------------------------------------------------------------------- */
    int iBand, i;
    
    for( iBand = 1, i = 0; iBand <= poDS->nBands; iBand++ )
    {
        poDS->SetBand( iBand, new L1BRasterBand( poDS, iBand ));
        
        // Channels descriptions
        if ( poDS->eSpacecraftID >= NOAA6 && poDS->eSpacecraftID <= METOP2 )
        {
            if ( !(i & 0x01) && poDS->iChannelsMask & 0x01 )
            {
                poDS->GetRasterBand(iBand)->SetDescription( apszBandDesc[0] );
                i |= 0x01;
                continue;
            }
            if ( !(i & 0x02) && poDS->iChannelsMask & 0x02 )
            {
                poDS->GetRasterBand(iBand)->SetDescription( apszBandDesc[1] );
                i |= 0x02;
                continue;
            }
            if ( !(i & 0x04) && poDS->iChannelsMask & 0x04 )
            {
                if ( poDS->eSpacecraftID >= NOAA15
                     && poDS->eSpacecraftID <= METOP2 )
                    if ( poDS->iInstrumentStatus & 0x0400 )
                        poDS->GetRasterBand(iBand)->SetDescription( apszBandDesc[7] );
                    else
                        poDS->GetRasterBand(iBand)->SetDescription( apszBandDesc[6] );
                else    
                    poDS->GetRasterBand(iBand)->SetDescription( apszBandDesc[2] );
                i |= 0x04;
                continue;
            }
            if ( !(i & 0x08) && poDS->iChannelsMask & 0x08 )
            {
                poDS->GetRasterBand(iBand)->SetDescription( apszBandDesc[3] );
                i |= 0x08;
                continue;
            }
            if ( !(i & 0x10) && poDS->iChannelsMask & 0x10 )
            {
                if (poDS->eSpacecraftID == NOAA13)              // 5 NOAA-13
                    poDS->GetRasterBand(iBand)->SetDescription( apszBandDesc[5] );
                else if (poDS->eSpacecraftID == NOAA6 ||
                         poDS->eSpacecraftID == NOAA8 ||
                         poDS->eSpacecraftID == NOAA10)         // 4 NOAA-6,-8,-10
                    poDS->GetRasterBand(iBand)->SetDescription( apszBandDesc[3] );
                else
                    poDS->GetRasterBand(iBand)->SetDescription( apszBandDesc[4] );
                i |= 0x10;
                continue;
            }
        }
    }

/* -------------------------------------------------------------------- */
/*      Do we have GCPs?                                                */
/* -------------------------------------------------------------------- */
    if ( 1/*EQUALN((const char *)pabyTBMHeader + 96, "Y", 1)*/ )
    {
        poDS->ProcessRecordHeaders();

#if 0
    Geolocation
        CPLString  osTMP;

        poDS->SetMetadataItem( "SRS", poDS->pszGCPProjection, "GEOLOCATION" );
        
        osTMP.Printf( "L1BGCPS:\"%s\"", pszFilename );
        poDS->SetMetadataItem( "X_DATASET", osTMP, "GEOLOCATION" );
        poDS->SetMetadataItem( "X_BAND", "1" , "GEOLOCATION" );
        poDS->SetMetadataItem( "Y_DATASET", osTMP, "GEOLOCATION" );
        poDS->SetMetadataItem( "Y_BAND", "2" , "GEOLOCATION" );

        osTMP.Printf( "%d", (poDS->eLocationIndicator == DESCEND) ?
            poDS->iGCPStart : (poDS->nRasterXSize - poDS->iGCPStart) );
        poDS->SetMetadataItem( "PIXEL_OFFSET", osTMP, "GEOLOCATION" );
        osTMP.Printf( "%d", (poDS->eLocationIndicator == DESCEND) ?
                      poDS->iGCPStep : -poDS->iGCPStep );
        poDS->SetMetadataItem( "PIXEL_STEP", osTMP, "GEOLOCATION" );

        poDS->SetMetadataItem( "LINE_OFFSET", "0", "GEOLOCATION" );
        poDS->SetMetadataItem( "LINE_STEP", "1", "GEOLOCATION" );
#endif
    }

/* -------------------------------------------------------------------- */
/*      Initialize any PAM information.                                 */
/* -------------------------------------------------------------------- */
    poDS->SetDescription( poOpenInfo->pszFilename );
    poDS->TryLoadXML();

    return( poDS );

bad:
    delete poDS;
    return NULL;
}

/************************************************************************/
/*                        GDALRegister_L1B()                            */
/************************************************************************/

void GDALRegister_L1B()

{
    GDALDriver  *poDriver;

    if( GDALGetDriverByName( "L1B" ) == NULL )
    {
        poDriver = new GDALDriver();
        
        poDriver->SetDescription( "L1B" );
        poDriver->SetMetadataItem( GDAL_DMD_LONGNAME, 
                                   "NOAA Polar Orbiter Level 1b Data Set" );
        poDriver->SetMetadataItem( GDAL_DMD_HELPTOPIC, 
                                   "frmt_l1b.html" );

        poDriver->pfnOpen = L1BDataset::Open;

        GetGDALDriverManager()->RegisterDriver( poDriver );
    }
}

