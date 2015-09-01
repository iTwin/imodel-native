//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGeoTiffCoordSysTable.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

//:>-----------------------------------------------------------------------------
//:> Methods for class HRFGeoTiffCoordSysTable
//:>-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFGeoTiffCoordSysTable.h>
#include <Imagepp/all/h/HCDCodecZlib.h>
#include <Imagepp/all/h/HFCURL.h>

/**----------------------------------------------------------------------------
 HRFGeoTiffCoordSysTable.tbl

 This file containts the table definition. The definition describe the column
 of the table data. This file must

 s_NbColumns    The number of column into the table
 s_RecordSize   The length of a record into the table
 s_TableDef[]   An array of HRFGeoTiffCompressedTable::ColumnInfo that describe
                each column of the table.

 @h3{Notes}

 The data use for this class were extracted from the table HORIZ_CS from EPSG
 database. The query is HORIZ_CS_HMRDataExtraction into the Access database
 EPSG.

 @see HRFGeoTiffCompressedTable
 @see HRFGeoTiffCompressedTable::ColumnInfo
 @see epsg.mdb
-----------------------------------------------------------------------------*/
#include "HRFGeoTiffCoordSysTable.tbl"

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
#include "HRFGeoTiffCoordSysTable.dat"


//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
HFC_IMPLEMENT_SINGLETON(HRFGeoTiffCoordSysTable);


//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------


/**----------------------------------------------------------------------------
 The destructor for this class.
-----------------------------------------------------------------------------*/
HRFGeoTiffCoordSysTable::~HRFGeoTiffCoordSysTable()
    {
    }


/**----------------------------------------------------------------------------
 Get a record from the table.

 @param pi_Code     A short value that containt the search key
 @param po_pRecord  An HRFGeoTiffCoordSysRecord pointer that receive the
                    record value.

 @return bool  true if the record found, false otherwise.

 @see HRFGeoTiffCompressedTable::GetValues()
 @see HRFGeoTiffCoordSysRecord
-----------------------------------------------------------------------------*/
bool HRFGeoTiffCoordSysTable::GetRecord(short pi_Code,
                                         HRFGeoTiffCoordSysRecord* po_pRecord) const
    {
    HPRECONDITION(po_pRecord != 0);

    bool Result = false;

    // convert code to char
    char SearchKey[7];     // Short range is -32,768 to 32,767. Plus one for the NUL.
    sprintf(SearchKey, "%d", pi_Code);

    vector<string> RecordValues;
    if (GetValues("Code", SearchKey, &RecordValues))
        {
        HPRECONDITION(RecordValues.size() == 5); // the table was define with 4 columns

        // the first column was the code, don't convert it and use the search code
        // sscanf(RecordValues[0].c_str(), "%d", po_pRecord->Code)
        po_pRecord->CSCode = pi_Code;

        int32_t Value;
        if ((Result = (sscanf(RecordValues[1].c_str(), "%d", &Value)) == 1))
            {
            HPOSTCONDITION(Value >= SHRT_MIN && Value <= SHRT_MAX);
            po_pRecord->ProjectionCode = (short)Value;
            }

        if (Result && (Result = (sscanf(RecordValues[2].c_str(), "%d", &Value)) == 1))
            {
            HPOSTCONDITION(Value >= SHRT_MIN && Value <= SHRT_MAX);
            po_pRecord->UnitsCode = (short)Value;
            }

        if (Result && (Result = (sscanf(RecordValues[3].c_str(), "%d", &Value)) == 1))
            {
            HPOSTCONDITION(Value >= SHRT_MIN && Value <= SHRT_MAX);
            po_pRecord->GeographicSourceCSCode = (short)Value;
            }

        po_pRecord->HorizCS_EPSG_Name = RecordValues[4];
        }
    return Result;
    }

/**----------------------------------------------------------------------------
 Get a record from the table.

 @param pi_Datum    A string value that containt the search datum key
 @param pi_ProjName A string value that containt the search projection name key
 @param pi_Zone     A string value that containt the search zone key
 @param po_pRecord  An HRFGeoTiffCoordSysRecord pointer that receive the
                    record value.

 @return bool  true if the record found, false otherwise.

 @see HRFGeoTiffCoordSysTable::GetFirst()
 @see HRFGeoTiffCoordSysTable::GetNext()
-----------------------------------------------------------------------------*/
bool HRFGeoTiffCoordSysTable::GetRecord(string pi_Datum,
                                         string pi_ProjName,
                                         string pi_Zone,
                                         HRFGeoTiffCoordSysRecord* po_pRecord) const
    {
    HPRECONDITION(po_pRecord != 0);

    bool Result = false;

    string::size_type Pos;

    string Value;
    string Zone     = " " + pi_Zone;
    string Datum    = pi_Datum + " ";
    string ProjName = " " + pi_ProjName + " ";

    if(GetFirst(po_pRecord))
        {
        do
            {
            Value = po_pRecord->HorizCS_EPSG_Name;
            // Search Datum value
            if((Pos = Value.find(Datum)) == 0)
                {
                if((Pos = Value.find("/")))
                    {
                    Value = Value.substr(Pos + 1);
                    // Search Projection Name value
                    if((Pos = Value.find(ProjName)) == 0)
                        {
                        Value = Value.substr(Pos + ProjName.size());
                        // Search Zone value
                        if((Pos = Value.find(Zone)) != string::npos)
                            Result = true;
                        }
                    }
                }
            }
        while(!Result && GetNext(po_pRecord));
        }

    return Result;
    }
bool HRFGeoTiffCoordSysTable::GetFirst(HRFGeoTiffCoordSysRecord* po_pRecord) const
    {
    bool Ret = true;

    if (m_NbRecord == 0)
        Ret = false;
    else
        {
        m_CurrentIndex = 0;
        FillRecord(m_CurrentIndex, po_pRecord);
        }

    return Ret;
    }
bool HRFGeoTiffCoordSysTable::GetNext (HRFGeoTiffCoordSysRecord* po_pRecord) const
    {
    bool Ret = true;

    m_CurrentIndex++;
    if (m_CurrentIndex >= m_NbRecord)
        Ret = false;
    else
        {
        FillRecord(m_CurrentIndex, po_pRecord);
        }

    return Ret;
    }

//:>-----------------------------------------------------------------------------
//:> protected section
//:>-----------------------------------------------------------------------------


//:>-----------------------------------------------------------------------------
//:> private section
//:>-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Constructor.

 This class is a singleton...
-----------------------------------------------------------------------------*/
HRFGeoTiffCoordSysTable::HRFGeoTiffCoordSysTable(const HFCPtr<HFCURL>& pi_rpCoordSysTableFile)
    : HRFGeoTiffCompressedTable(pi_rpCoordSysTableFile)
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

    // Index use by the methods GetFirst and Next
    m_CurrentIndex = 0;
    m_NbRecord = m_UncompressedDataSize / m_RecordSize;

    m_pWorkingBuffer = new Byte[m_RecordSize + 1];
    }

void HRFGeoTiffCoordSysTable::FillRecord(size_t                    pi_IndexRecord,
                                         HRFGeoTiffCoordSysRecord* po_pRecord) const
    {
    HPRECONDITION(pi_IndexRecord < m_NbRecord);
    const Byte* pTable = GetTableRawData() + (pi_IndexRecord * m_RecordSize);

    vector<ColumnInfo>::const_iterator TableColumnsItr(m_ColumnsArray.begin());
    int32_t Value;

    // Field Code
    memcpy(m_pWorkingBuffer, pTable+TableColumnsItr->m_ColumnStartPos, TableColumnsItr->m_ColumnSize);
    m_pWorkingBuffer[TableColumnsItr->m_ColumnSize] = 0;
    if (sscanf((const char*)m_pWorkingBuffer.get(), "%d", &Value) == 1)
        po_pRecord->CSCode = (short)Value;

    // Field TransfoModel Code
    TableColumnsItr++;
    memcpy(m_pWorkingBuffer, pTable+TableColumnsItr->m_ColumnStartPos, TableColumnsItr->m_ColumnSize);
    m_pWorkingBuffer[TableColumnsItr->m_ColumnSize] = 0;
    if (sscanf((const char*)m_pWorkingBuffer.get(), "%d", &Value) == 1)
        po_pRecord->ProjectionCode = (short)Value;

    // Field Units Code
    TableColumnsItr++;
    memcpy(m_pWorkingBuffer, pTable+TableColumnsItr->m_ColumnStartPos, TableColumnsItr->m_ColumnSize);
    m_pWorkingBuffer[TableColumnsItr->m_ColumnSize] = 0;
    if (sscanf((const char*)m_pWorkingBuffer.get(), "%d", &Value) == 1)
        po_pRecord->UnitsCode = (short)Value;

    // Field Source CS Code
    TableColumnsItr++;
    memcpy(m_pWorkingBuffer, pTable+TableColumnsItr->m_ColumnStartPos, TableColumnsItr->m_ColumnSize);
    m_pWorkingBuffer[TableColumnsItr->m_ColumnSize] = 0;
    if (sscanf((const char*)m_pWorkingBuffer.get(), "%d", &Value) == 1)
        po_pRecord->GeographicSourceCSCode = (short)Value;

    // Field HorizCS - EPSG Name
    TableColumnsItr++;
    memcpy(m_pWorkingBuffer, pTable+TableColumnsItr->m_ColumnStartPos, TableColumnsItr->m_ColumnSize);
    m_pWorkingBuffer[TableColumnsItr->m_ColumnSize] = 0;
    po_pRecord->HorizCS_EPSG_Name = (char*)m_pWorkingBuffer.get();
    }
