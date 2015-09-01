//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGeoTiffCompressedTable.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//:>-----------------------------------------------------------------------------
//:> Methods for class HRFGeoTiffCompressedTable
//:>-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HRFGeoTiffCompressedTable.h>
#include <Imagepp/all/h/HCDCodecIdentity.h>
#include <Imagepp/all/h/HFCBinStream.h>
#include <Imagepp/all/h/HFCException.h>
#include <Imagepp/all/h/HRFUtility.h>
#include <Imagepp/all/h/HFCURL.h>

//:>-----------------------------------------------------------------------------
//:> public section
//:>-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 The destructor for this class.
-----------------------------------------------------------------------------*/
HRFGeoTiffCompressedTable::~HRFGeoTiffCompressedTable()
    {
    HPRECONDITION(m_RefCount == 0);
    }


/**----------------------------------------------------------------------------
 This method uncompressed the table data and keep it in memory.

 @notes A call to ReleaseTable must be call to free the data pointer.

 @see ReleaseTable()
-----------------------------------------------------------------------------*/
void HRFGeoTiffCompressedTable::LockTable()
    {
    if (m_RefCount == 0)
        {
        if (m_pCompressedTableFile != 0)
            {
            //:> we have a file, use it instead of using the compressed data
            m_pUncompressedTable = ReadDataFile();

            HPRECONDITION(m_RecordSize > 0);
            HPRECONDITION(m_pUncompressedTable != 0);
            HPRECONDITION(m_pUncompressedTable->GetDataSize() % m_RecordSize == 0);

            m_NbRecord = (uint32_t)(m_pUncompressedTable->GetDataSize() / m_RecordSize);
            }
        else
            {
            m_pUncompressedTable = new HCDPacket(new HCDCodecIdentity(),
                                                 new Byte[m_UncompressedDataSize],
                                                 m_UncompressedDataSize,
                                                 0);
            m_pUncompressedTable->SetBufferOwnership(true);

            m_pCompressedTable->Decompress(m_pUncompressedTable);

            if (m_NbRecord == 0)
                {
                HPRECONDITION(m_RecordSize > 0);
                HPRECONDITION(m_pUncompressedTable->GetDataSize() % m_RecordSize == 0);
                m_NbRecord = (uint32_t)(m_pUncompressedTable->GetDataSize() / m_RecordSize);
                }
            }
        }

    m_RefCount++;
    }


/**----------------------------------------------------------------------------
 This method find a value from the specific key into the table.

 @param pi_rKeyColumnName       A const reference to a string that containt the
                                name of the key column
 @param pi_rKeyValue            A const reference to a string that containt the
                                value of the search key
 @param pi_rSearchColumnName    A const reference to a string that containt the
                                name of the column to extract the value
 @param pi_rSearchValue         A pointer to a string to receive the search
                                result

 @return bool  true if the record was found, false otherwise

 @notes If the table was not lock, the method decompress the table, find the
        value and release the pointer.

 @see GetValues()
 @see LockTable()
 @see ReleaseTable()
 ----------------------------------------------------------------------------*/
bool HRFGeoTiffCompressedTable::GetValue(const string&   pi_rKeyColumnName,
                                          const string&   pi_rKeyValue,
                                          const string&   pi_rSearchColumnName,
                                          string*         po_pSearchValue) const
    {
    HPRECONDITION(po_pSearchValue != 0);

    //:> Get the columns position
    unsigned short KeyColumnStartPos=0;
    unsigned short KeyColumnSize=0;
    unsigned short SearchColumnStartPos=0;
    unsigned short SearchColumnSize=0;
    bool   KeyColumnFound = false;
    bool   SearchColumnFound = false;
    bool   RecordFound = false;

    ColumnsInfo::const_iterator Itr(m_ColumnsArray.begin());

    while (!KeyColumnFound && !SearchColumnFound && Itr != m_ColumnsArray.end())
        {
        if (!KeyColumnFound)
            {
            if (strcmp(pi_rKeyColumnName.c_str(), Itr->m_ColumnName.c_str()) == 0)
                {
                KeyColumnStartPos = Itr->m_ColumnStartPos;
                KeyColumnSize = Itr->m_ColumnSize;
                }
            }

        if (!SearchColumnFound)
            {
            if (strcmp(pi_rKeyColumnName.c_str(), Itr->m_ColumnName.c_str()) == 0)
                {
                SearchColumnStartPos = Itr->m_ColumnStartPos;
                SearchColumnSize = Itr->m_ColumnSize;
                }
            }
        Itr++;
        }


    po_pSearchValue->erase();
    if (KeyColumnFound && SearchColumnFound)
        {
        (const_cast<HRFGeoTiffCompressedTable*>(this))->LockTable();

        //:> set a pointer to the key column of the first record
        Byte* pKeyPtr = m_pUncompressedTable->GetBufferAddress() + KeyColumnStartPos;

        //:> add space at the end of the key value to make
        //:> the string as the same size of the column
        string KeyValue(pi_rKeyValue);
        size_t i;
        for (i = KeyValue.length(); i < KeyColumnSize; i++)
            KeyValue += " ";

        for (i = 0; i < m_NbRecord && !RecordFound; i++, pKeyPtr += m_RecordSize)
            {
            RecordFound = strncmp(KeyValue.c_str(), (char*)pKeyPtr, KeyValue.size()) == 0;
            }

        if (RecordFound)
            {
            *po_pSearchValue = string((char*)(pKeyPtr - KeyColumnStartPos + SearchColumnStartPos), SearchColumnSize);
            }

        (const_cast<HRFGeoTiffCompressedTable*>(this))->ReleaseTable();
        }

    return RecordFound;
    }

/**----------------------------------------------------------------------------
 This method find values from the specific key into the table.

 @param pi_rKeyColumnName       A const reference to a string that containt the
                                name of the key column
 @param pi_rKeyValue            A const reference to a string that containt the
                                value of the search key
 @param pi_rSearchColumnsName   A const reference to a vector that containt the
                                name of the columns to extract the value
 @param pi_rSearchValues        A pointer to a vector to receive the search
                                result

 @return bool  true if the record was found, false otherwise

 @notes If the table was not lock, the method decompress the table, find the
        value and release the pointer.

 @see GetValue()
 @see LockTable()
 @see ReleaseTable()

 ----------------------------------------------------------------------------*/
bool HRFGeoTiffCompressedTable::GetValues(const string&          pi_rKeyColumnName,
                                           const string&          pi_rKeyValue,
                                           const vector<string>&  pi_rSelectColumnsName,
                                           vector<string>*        po_pSelectValues) const
    {
    HPRECONDITION(pi_rSelectColumnsName.size() > 1);
    HPRECONDITION(po_pSelectValues != 0);

    po_pSelectValues->erase(po_pSelectValues->begin(), po_pSelectValues->end());

    //:> find the column index
    ColumnsInfo::const_iterator KeyColumn(m_ColumnsArray.begin());
    bool   KeyColumnFound = false;

    while (!KeyColumnFound && KeyColumn != m_ColumnsArray.end())
        {
        if (strcmp(pi_rKeyColumnName.c_str(), KeyColumn->m_ColumnName.c_str()) == 0)
            {
            KeyColumnFound = true;
            }
        KeyColumn++;
        }

    vector<ColumnsInfo::const_iterator> SelectColumns;
    bool                               SelectColumnsFound=false;

    if (KeyColumnFound)
        {
        //:> find the search column
        ColumnsInfo::const_iterator TableColumnsItr;
        vector<string>::const_iterator SelectColumnsNameItr(pi_rSelectColumnsName.begin());
        SelectColumnsFound = true;
        while (SelectColumnsNameItr != pi_rSelectColumnsName.end() && SelectColumnsFound)
            {
            TableColumnsItr = m_ColumnsArray.begin();
            SelectColumnsFound = false;
            while (!SelectColumnsFound && TableColumnsItr != m_ColumnsArray.end())
                {
                if (strcmp(SelectColumnsNameItr->c_str(), TableColumnsItr->m_ColumnName.c_str()) == 0)
                    {
                    SelectColumns.push_back(TableColumnsItr);
                    SelectColumnsFound = true;
                    }
                TableColumnsItr++;
                }

            SelectColumnsNameItr++;
            }
        }

    if (KeyColumnFound && SelectColumnsFound)
        {
        (const_cast<HRFGeoTiffCompressedTable*>(this))->LockTable();

        //:> set a pointer to the key column of the first record
        Byte* pKeyPtr = m_pUncompressedTable->GetBufferAddress() + KeyColumn->m_ColumnStartPos;
        bool RecordFound = false;

        //:> add space at the end of the key value to make
        //:> the string as the same size of the column
        string KeyValue(pi_rKeyValue);
        size_t i;
        for (i = KeyValue.length(); i < KeyColumn->m_ColumnSize; i++)
            KeyValue += " ";

        for (i = 0; i < m_NbRecord && !RecordFound; i++)
            {
            RecordFound = strncmp(KeyValue.c_str(), (char*)pKeyPtr, KeyValue.size()) == 0;
            if (!RecordFound)
                pKeyPtr += m_RecordSize;
            }

        if (RecordFound)
            {
            vector<ColumnsInfo::const_iterator>::const_iterator SelectColumnsItr(SelectColumns.begin());
            while (SelectColumnsItr != SelectColumns.end())
                {
                po_pSelectValues->push_back(string((char*)(pKeyPtr - KeyColumn->m_ColumnStartPos + (*SelectColumnsItr)->m_ColumnStartPos),
                                                   (*SelectColumnsItr)->m_ColumnSize));

                SelectColumnsItr++;
                }
            }
        (const_cast<HRFGeoTiffCompressedTable*>(this))->ReleaseTable();
        }

    return KeyColumnFound && SelectColumnsFound;
    }

/**----------------------------------------------------------------------------
 This method find values from the specific key into the table.

 @param pi_rKeyColumnName       A const reference to a string that containt the
                                name of the key column
 @param pi_rKeyValue            A const reference to a string that containt the
                                value of the search key
 @param pi_rSearchValues        A pointer to a vector to receive the search
                                result

 @return bool  true if the record was found, false otherwise

 @notes If the table was not lock, the method decompress the table, find the
        value and release the pointer.

 @see GetValue()
 @see LockTable()
 @see ReleaseTable()

 ----------------------------------------------------------------------------*/
bool HRFGeoTiffCompressedTable::GetValues(const string&          pi_rKeyColumnName,
                                           const string&          pi_rKeyValue,
                                           vector<string>*        po_pSelectValues) const
    {
    HPRECONDITION(po_pSelectValues != 0);

    po_pSelectValues->erase(po_pSelectValues->begin(), po_pSelectValues->end());

    //:> find the column index
    ColumnsInfo::const_iterator KeyColumn(m_ColumnsArray.begin());
    bool   KeyColumnFound = false;

    while (!KeyColumnFound && KeyColumn != m_ColumnsArray.end())
        {
        if (strcmp(pi_rKeyColumnName.c_str(), KeyColumn->m_ColumnName.c_str()) == 0)
            {
            KeyColumnFound = true;
            }
        else
            KeyColumn++;
        }


    vector<ColumnsInfo::const_iterator> SelectColumns;

    if (KeyColumnFound)
        {
        (const_cast<HRFGeoTiffCompressedTable*>(this))->LockTable();

        //:> set a pointer to the key column of the first record
        Byte* pKeyPtr = m_pUncompressedTable->GetBufferAddress() + KeyColumn->m_ColumnStartPos;

        //:> add space at the end of the key value to make
        //:> the string as the same size of the column
        string KeyValue(pi_rKeyValue);
        size_t i;
        for (i = KeyValue.length(); i < KeyColumn->m_ColumnSize; i++)
            KeyValue += " ";

        bool RecordFound = false;
        for (i = 0; i < m_NbRecord && !RecordFound; i++)
            {
            RecordFound = strncmp(KeyValue.c_str(), (char*)pKeyPtr, KeyValue.size()) == 0;
            if (!RecordFound)   //:> it's not the record we found, skip to the next record
                pKeyPtr += m_RecordSize;
            }

        string Value;
        if (RecordFound)
            {
            vector<ColumnInfo>::const_iterator TableColumnsItr(m_ColumnsArray.begin());
            while (TableColumnsItr != m_ColumnsArray.end())
                {
                po_pSelectValues->push_back(string((char*)(pKeyPtr - KeyColumn->m_ColumnStartPos + TableColumnsItr->m_ColumnStartPos),
                                                   TableColumnsItr->m_ColumnSize));

                TableColumnsItr++;
                }
            }
        (const_cast<HRFGeoTiffCompressedTable*>(this))->ReleaseTable();
        }

    return po_pSelectValues->size() != 0;
    }

//:>-----------------------------------------------------------------------------
//:> protected section
//:>-----------------------------------------------------------------------------

/**----------------------------------------------------------------------------
 Default constructor for descendant class
-----------------------------------------------------------------------------*/
HRFGeoTiffCompressedTable::HRFGeoTiffCompressedTable(const HFCPtr<HFCURL>& pi_rpCompressedTableFile)
    {
    m_pCompressedTableFile = pi_rpCompressedTableFile;
    m_RefCount = 0;
    m_pCompressedTable = 0;
    m_RecordSize = 0;
    m_UncompressedDataSize = 0;
    m_NbRecord = 0;

    }


//:>-----------------------------------------------------------------------------
//:> private section
//:>-----------------------------------------------------------------------------

#define PRINTABLE_CHARACTER(c) (c != EOF && c != '\n' && c != '\r' && c != 0x0D && c != 0x0A)
//-----------------------------------------------------------------------------
// ReadTextLine
//-----------------------------------------------------------------------------
size_t HRFGeoTiffCompressedTable::ReadTextLine(HFCBinStream* pi_pFile, unsigned char* pio_pTextLine) const
    {
    HPRECONDITION(pi_pFile != 0);
    HPRECONDITION(pio_pTextLine != 0);

    unsigned char  ReadChar=0;
    bool   EndOfRead = false;

    //:> skip all not printable character
    while (!pi_pFile->EndOfFile() && !EndOfRead)
        {
        pi_pFile->Read(&ReadChar, 1);
        EndOfRead = PRINTABLE_CHARACTER(ReadChar);
        }

    unsigned char* pOutputPtr = pio_pTextLine;
    if (!pi_pFile->EndOfFile())
        {
        EndOfRead = false;
        *pOutputPtr++ = ReadChar; // add the last char read

        while (!pi_pFile->EndOfFile() && !EndOfRead)
            {
            pi_pFile->Read(&ReadChar, 1);
            if (PRINTABLE_CHARACTER(ReadChar))
                *pOutputPtr++ = ReadChar;
            else
                EndOfRead = true;
            }
        }

    return pOutputPtr - pio_pTextLine;
    }


//-----------------------------------------------------------------------------
// ReadDataFile
//-----------------------------------------------------------------------------
HFCPtr<HCDPacket> HRFGeoTiffCompressedTable::ReadDataFile()
    {
    HPRECONDITION(m_RecordSize != 0);

    HAutoPtr<HFCBinStream> pFile(HFCBinStream::Instanciate(m_pCompressedTableFile, HFC_READ_ONLY, 0, true));

    uint64_t FileSize = pFile->GetSize();

    unsigned char* pTableData = new unsigned char[(uint32_t)FileSize];

    HFCPtr<HCDPacket> pPacket = new HCDPacket(pTableData,
                                              (size_t)FileSize);
    pPacket->SetBufferOwnership(true);

    // read the first line to keep the line size
    size_t ReadSize;
    ReadSize = ReadTextLine(pFile, pTableData);
    if (ReadSize != m_RecordSize)
        throw HFCCorruptedFileException(m_pCompressedTableFile->GetURL());

    while (ReadSize > 0)
        {
        pTableData += ReadSize;

        ReadSize = ReadTextLine(pFile, pTableData);

        if (ReadSize > 0 && ReadSize != m_RecordSize)
            throw HFCCorruptedFileException(m_pCompressedTableFile->GetURL());
        }

    pPacket->SetDataSize(pTableData - pPacket->GetBufferAddress());

    return pPacket;
    }
/**----------------------------------------------------------------------------
 Return the number of the column into the table definition

 @return size_t The number of column into the table definition
-----------------------------------------------------------------------------*/
size_t HRFGeoTiffCompressedTable::GetColumnCount() const
    {
    return m_ColumnsArray.size();
    }

/**----------------------------------------------------------------------------
 Return the information for a specific column.

 @param pi_ColumnIdx    The index of the column

 @return HRFGeoTiffCompressedTable::ColumnInfo The column information

 @see HFCCompressedTable::ColumnInfo
-----------------------------------------------------------------------------*/
const HRFGeoTiffCompressedTable::ColumnInfo& HRFGeoTiffCompressedTable::GetColumnInfo(unsigned short pi_ColumnIdx) const
    {
    HPRECONDITION(pi_ColumnIdx < m_ColumnsArray.size());

    return m_ColumnsArray[pi_ColumnIdx];
    }


/**----------------------------------------------------------------------------
 Return the information for a all columns into the table.

 @return HRFGeoTiffCompressedTable::ColumnsInfo The columns information

 @see HFCCompressedTable::ColumnsInfo
-----------------------------------------------------------------------------*/
const HRFGeoTiffCompressedTable::ColumnsInfo& HRFGeoTiffCompressedTable::GetColumnsInfo() const
    {
    return m_ColumnsArray;
    }

/**----------------------------------------------------------------------------
 Return the table raw data uncompressed.

 @return Byte*         A pointer to the raw data. A call to the method
                        LockTable() must be call before.

 @see LockTable()
 @see ReleaseTable()
-----------------------------------------------------------------------------*/
const Byte* HRFGeoTiffCompressedTable::GetTableRawData() const
    {
    HPRECONDITION(m_pUncompressedTable != 0);
    HPRECONDITION(m_pCompressedTable != 0 && m_pCompressedTable->GetBufferAddress() != 0);
    HPRECONDITION(m_ColumnsArray.size() > 0);

    Byte* pRawData = 0;

    if (m_pUncompressedTable != 0)
        pRawData = m_pUncompressedTable->GetBufferAddress();

    return pRawData;
    }

/**----------------------------------------------------------------------------
 This method release the uncompress data pointer.

 @notes This method must be call for each call to LockTable.


 @see LockTable()
 ----------------------------------------------------------------------------*/
void HRFGeoTiffCompressedTable::ReleaseTable()
    {
    HPRECONDITION(m_RefCount > 0);

    if (m_RefCount > 0)
        m_RefCount--;

    if (m_RefCount == 0)
        m_pUncompressedTable = 0;
    }

/**----------------------------------------------------------------------------
 This method set the name of the file that contain the table data. If a file
 was setted, his data will be use instead of the class data.

 @param pi_rpURL        A smart pointer to the file URL


 @notes The compressed table must be not locked when this method is called

 @exception HRFGeoTiffCompressedTableLockedException

 @see GetTableFileName()
 @see LockTable()
 @see ReleaseTable()
 ----------------------------------------------------------------------------*/
void HRFGeoTiffCompressedTable::SetTableFile(const HFCPtr<HFCURL>& pi_rpCompressedTableFile)
    {
    if (m_RefCount != 0)
        throw HRFGeotiffCompressedTableLockException(pi_rpCompressedTableFile->GetURL());

    m_pCompressedTableFile = pi_rpCompressedTableFile;
    }

/**----------------------------------------------------------------------------
 This method return the file name URL.

 @return const HFCPtr<HFCURL>&


 @see SetTableFile()
 ----------------------------------------------------------------------------*/
const HFCPtr<HFCURL>& HRFGeoTiffCompressedTable::GetTableFileName() const
    {
    return m_pCompressedTableFile;
    }