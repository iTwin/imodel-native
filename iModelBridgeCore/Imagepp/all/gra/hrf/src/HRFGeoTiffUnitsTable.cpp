//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGeoTiffUnitsTable.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Methods for class HRFGeoTiffUnitsTable
//:>-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFGeoTiffUnitsTable.h>
#include <Imagepp/all/h/HCDCodecZlib.h>
#include <Imagepp/all/h/HFCURL.h>

/**----------------------------------------------------------------------------
 HRFGeoTiffUnitsTable.tbl

 This file containts the table definition. The definition describe the column
 of the table data. This file must

 s_NbColumns    The number of column into the table
 s_RecordSize   The length of a record into the table
 s_TableDef[]   An array of HRFGeoTiffCompressedTable::ColumnInfo that describe
                each column of the table.

 @h3{Notes}

 The data use for this class were extracted from the table UOM_LENGTH from EPSG
 database. The query is UOM_LENGTH_HMRDataExtraction into the Access database
 EPSG.

 @see HRFGeoTiffCompressedTable
 @see HRFGeoTiffCompressedTable::ColumnInfo
 @see epsg.mdb
-----------------------------------------------------------------------------*/
#include "HRFGeoTiffUnitsTable.tbl"

/**----------------------------------------------------------------------------
 HRFGeoTiffCoordSysTable.dat

 This file containt the table data compressed with deflate. The file can be
 create with the HRFGeoTiffCompressTable application.

 s_UncompressedTableSize    The size of the uncompressed table.
 s_CompressedTableSize      The size of the compressed table.
 s_CompressedTable[]        The compressed table data.


 @see HRFGeoTiffCompressedTable
 @see HRFGeoTiffCompressTable application
-----------------------------------------------------------------------------*/
#include "HRFGeoTiffUnitsTable.dat"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
HFC_IMPLEMENT_SINGLETON(HRFGeoTiffUnitsTable);

//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 The destructor for this class.
-----------------------------------------------------------------------------*/
HRFGeoTiffUnitsTable::~HRFGeoTiffUnitsTable()
    {
    }

/**----------------------------------------------------------------------------
 Get a record from the table.

 @param pi_Code     A short value that containt the search key
 @param po_pRecord  An HRFGeoTiffUnitsRecord pointer that receive the
                    record value.

 @return bool  true if the record found, false otherwise.

 @see HRFGeoTiffCompressedTable::GetValues()
 @see HRFGeoTiffUnitsRecord
-----------------------------------------------------------------------------*/
bool HRFGeoTiffUnitsTable::GetRecord(short pi_Code,
                                      HRFGeoTiffUnitsRecord*    po_pRecord) const
    {
    HPRECONDITION(po_pRecord != 0);

    bool Result = false;

    // convert code to char
    char SearchKey[8];
    sprintf(SearchKey, "%d", pi_Code);

    vector<string> RecordValues;
    if (GetValues("Code", SearchKey, &RecordValues))
        {
        HPRECONDITION(RecordValues.size() == 6); //:> the table was define with 6 columns

        // the first column was the code, don't convert it and use the search code
        // sscanf(RecordValues[0].c_str(), "%d", po_pRecord->Code)
        po_pRecord->Code = pi_Code;
        po_pRecord->EPSGName = RecordValues[1];
        po_pRecord->EPSGAbbr = RecordValues[2];
        Result = sscanf(RecordValues[3].c_str(), "%hd",  &(po_pRecord->TargetUnitCode)) == 1;
        Result = sscanf(RecordValues[4].c_str(), "%lf", &(po_pRecord->FactorB)) == 1 && Result;
        Result = sscanf(RecordValues[5].c_str(), "%lf", &(po_pRecord->FactorC)) == 1 && Result;
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
HRFGeoTiffUnitsTable::HRFGeoTiffUnitsTable(const HFCPtr<HFCURL>& pi_rpUnitsTableFile)
    : HRFGeoTiffCompressedTable(pi_rpUnitsTableFile)
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
