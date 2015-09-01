//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGeoTiffProjectionTable.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Methods for class HRFGeoTiffProjectionTable
//:>-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFGeoTiffProjectionTable.h>
#include <Imagepp/all/h/HCDCodecZlib.h>
#include <Imagepp/all/h/HFCURL.h>

/**----------------------------------------------------------------------------
 HRFGeoTiffProjectionTable.tbl

 This file containts the table definition. The definition describe the column
 of the table data. This file must

 s_NbColumns    The number of column into the table
 s_RecordSize   The length of a record into the table
 s_TableDef[]   An array of HRFGeoTiffCompressedTable::ColumnInfo that describe
                each column of the table.

 @h3{Notes}

 The data use for this class were extracted from the table TRF_NONPOLYNOMIAL
 from EPSG database. The query is TRF_NONPOLYNOMIAL_HMRDataExtraction into the
 Access database EPSG.

 @see HRFGeoTiffCompressedTable
 @see HRFGeoTiffCompressedTable::ColumnInfo
 @see epsg.mdb
-----------------------------------------------------------------------------*/
#include "HRFGeoTiffProjectionTable.tbl"

/**----------------------------------------------------------------------------
 HRFGeoTiffProjectionTable.dat

 This file containt the table data compressed with deflate. The file can be
 create with the HRFGeoTiffCompressTable application.

 s_UncompressedTableSize    The size of the uncompressed table.
 s_CompressedTableSize      The size of the compressed table.
 s_CompressedTable[]        The compressed table data.


 @see HRFGeoTiffCompressedTable
 @see HRFGeoTiffCompressTable application
-----------------------------------------------------------------------------*/
#include "HRFGeoTiffProjectionTable.dat"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
HFC_IMPLEMENT_SINGLETON(HRFGeoTiffProjectionTable);


//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 The destructor for this class.
-----------------------------------------------------------------------------*/
HRFGeoTiffProjectionTable::~HRFGeoTiffProjectionTable()
    {
    }

/**----------------------------------------------------------------------------
 Get a record from the table.

 @param pi_Code     A short value that containt the search key
 @param po_pRecord  An HRFGeoTiffProjectionRecord pointer that receive the
                    record value.

 @return bool  true if the record found, false otherwise.

 @see HRFGeoTiffCompressedTable::GetValues()
 @see HRFGeoTiffProjectionRecord
-----------------------------------------------------------------------------*/
bool HRFGeoTiffProjectionTable::GetRecord(short pi_Code,
                                           HRFGeoTiffProjectionRecord*  po_pRecord) const
    {
    HPRECONDITION(po_pRecord != 0);

    bool Result = false;

    // convert code to char
    char SearchKey[6];
    sprintf(SearchKey, "%d", pi_Code);

    vector<string> RecordValues;
    if (GetValues("Code", SearchKey, &RecordValues))
        {
        HPRECONDITION(RecordValues.size() == 2); // the table was define with 3 columns

        // the first column was the code, don't convert it and use the search code
        // sscanf(RecordValues[0].c_str(), "%d", po_pRecord->Code)
        po_pRecord->Code = pi_Code;

        int32_t Value;
        if ((Result = (sscanf(RecordValues[1].c_str(), "%d", &Value)) == 1))
            {
            HPOSTCONDITION(Value >= SHRT_MIN && Value <= SHRT_MAX);
            po_pRecord->UnitsCode = (short)Value;
            }
        }
    return Result;
    }

//:>-----------------------------------------------------------------------------
//:> protected section
//:>-----------------------------------------------------------------------------


//:>-----------------------------------------------------------------------------
//:> private section
//:>-----------------------------------------------------------------------------
/**----------------------------------------------------------------------------
 Default constructor for this class.

 This class is a singleton...
-----------------------------------------------------------------------------*/
HRFGeoTiffProjectionTable::HRFGeoTiffProjectionTable(const HFCPtr<HFCURL>& pi_rpProjectionTableFile)
    : HRFGeoTiffCompressedTable(pi_rpProjectionTableFile)
    {
    //:> Initialize the packet with the static variable,
    //:> The packet was not ownership of the buffer, the buffer is static variable
    m_pCompressedTable = new HCDPacket(new HCDCodecZlib(),
                                       s_CompressedTable,
                                       s_CompressedTableSize,
                                       s_CompressedTableSize);

    //:> initialize the table definition
    for (unsigned short i = 0; i < s_NbColumns; i++)
        m_ColumnsArray.push_back(s_TableDef[i]);

    //:> set the size of the uncompressed data
    m_UncompressedDataSize = s_UncompressedTableSize;

    //:> set the number of record
    m_RecordSize = s_RecordSize;
    }
