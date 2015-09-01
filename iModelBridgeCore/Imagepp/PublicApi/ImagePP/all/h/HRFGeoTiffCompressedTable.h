//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFGeoTiffCompressedTable.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HRFGeoTiffCompressedTable
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"
#include "HCDPacket.h"
#include "HRFException.h"

/** -----------------------------------------------------------------------------
    @version 1.0
    @author  Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})

    This class is designed to keep in memory the data of a database table.

    -----------------------------------------------------------------------------
*/

BEGIN_IMAGEPP_NAMESPACE
class HRFGeoTiffCompressedTable : public HFCShareableObject<HRFGeoTiffCompressedTable>
    {
public:

    HDECLARE_BASECLASS_ID(HRFGeoTiffCompressedTableId_Base);


    /** -----------------------------------------------------------------------------
        ColumnInfo struct

        This struct was use to describe a column into the table.

        m_ColumnStartPos    The position of the first character into the table. The
                            column of the table start at 0.

        m_ColumnSize        The size of the column.

        m_ColumnName        The name of the column.
        -----------------------------------------------------------------------------
    */
    struct ColumnInfo
        {
        unsigned short m_ColumnStartPos;
        unsigned short m_ColumnSize;
        string      m_ColumnName;

        ColumnInfo(unsigned short pi_StartPos, unsigned short pi_ColumnSize, const string& pi_ColumnName)
            {
            m_ColumnStartPos    = pi_StartPos;
            m_ColumnSize        = pi_ColumnSize;
            m_ColumnName        = pi_ColumnName;
            };
        };

    /** -----------------------------------------------------------------------------
        ColumnsInfo datatype

        This datatype describe all column into a table.

        -----------------------------------------------------------------------------
    */
    typedef vector<ColumnInfo> ColumnsInfo;

    //:> Primary methods.
    virtual                 ~HRFGeoTiffCompressedTable();

    //:> Column information
    virtual size_t          GetColumnCount() const;
    virtual const HRFGeoTiffCompressedTable::ColumnInfo&
    GetColumnInfo(unsigned short pi_ColumnIdx) const;
    virtual const HRFGeoTiffCompressedTable::ColumnsInfo&
    GetColumnsInfo() const;


    IMAGEPP_EXPORT void                    LockTable();
    IMAGEPP_EXPORT void                    ReleaseTable();

    IMAGEPP_EXPORT void             SetTableFile(const HFCPtr<HFCURL>& pi_rpCompressedTableFile);
    const HFCPtr<HFCURL>&   GetTableFileName() const;

    virtual const Byte*    GetTableRawData() const;
    virtual bool           GetValue(const string&              pi_rKeyColumnName,
                                     const string&              pi_rKeyValue,
                                     const string&              pi_rSearchColumnName,
                                     string*                    po_pSearchValue) const;

    virtual bool           GetValues(const string&             pi_rKeyColumnName,
                                      const string&             pi_rKeyValue,
                                      const vector<string>&     pi_rSearchColumnsName,
                                      vector<string>*           po_pSearchValues) const;

    virtual bool           GetValues(const string&             pi_rKeyColumnName,
                                      const string&             pi_rKeyValue,
                                      vector<string>*           po_pSearchValues) const;


protected:

    //:> method for descendant
    HRFGeoTiffCompressedTable(const HFCPtr<HFCURL>& pi_rpCompressedTableFile = 0);

    /** -----------------------------------------------------------------------------
        This members must be initialize by the descendant


        m_ColumnsArray          Columns definition into the table.

        m_pCompressedTable      A packet that containt the table compressed data.

        m_UncompressedDataSize  The size of the data before the table was compressed.
                                This value is need because we need to create the
                                uncompressed buffer before decompress the data.

        m_RecordSize            The size of one record into the table.

        @see ColumnInfo struct
        @see HCDPacket
        -----------------------------------------------------------------------------
    */
    ColumnsInfo             m_ColumnsArray;
    HFCPtr<HCDPacket>       m_pCompressedTable;
    uint32_t                m_UncompressedDataSize;
    unsigned short         m_RecordSize;


private:

    HFCPtr<HFCURL>          m_pCompressedTableFile;
    uint32_t                m_RefCount;
    uint32_t                m_NbRecord;
    HFCPtr<HCDPacket>       m_pUncompressedTable;

    size_t                  ReadTextLine(HFCBinStream*  pi_pFile,
                                         unsigned char*        pio_pTextLine) const;

    HFCPtr<HCDPacket>       ReadDataFile();


    //:> disable method
    HRFGeoTiffCompressedTable(const HRFGeoTiffCompressedTable&);
    HRFGeoTiffCompressedTable& operator=(const HRFGeoTiffCompressedTable&);
    };
END_IMAGEPP_NAMESPACE



