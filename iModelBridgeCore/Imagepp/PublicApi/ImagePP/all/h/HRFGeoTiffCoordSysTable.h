//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFGeoTiffCoordSysTable.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFGeoTiffCoordSysTable
//-----------------------------------------------------------------------------
#pragma once

#include "HRFGeoTiffCompressedTable.h"
#include "HFCMacros.h"

/** -----------------------------------------------------------------------------
    @version 1.0
    @author  Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})

    This class is used to keep in memory some information related to the GeoTiff
    coord sys. For the first version, the data is always compressed with Deflate.

    @h3{Notes}

    The file HRFGeoTiffCoordSysTable.dat and HRFGeoTiffCoordSysTable.tbl must be
    exist. The file HRFGeoTiffCoordSysTable.dat can be create with the
    HRFGeoTiffCompressTable.exe application.

    @see HRFGeoTiffCompressedTable
    @see HRFGeoTiffCompressTable application
    -----------------------------------------------------------------------------
*/

class HRFGeoTiffCoordSysTable : public HRFGeoTiffCompressedTable
    {
public:

    HDECLARE_CLASS_ID(1491, HRFGeoTiffCompressedTable);

    /** -----------------------------------------------------------------------------
        HRFGeoTiffCoordSysRecord
        -----------------------------------------------------------------------------
    */
    typedef struct
        {
        short CSCode;
        short ProjectionCode;
        short UnitsCode;
        short GeographicSourceCSCode;
        string      HorizCS_EPSG_Name;

        } HRFGeoTiffCoordSysRecord;

    //:> Primary methods.
    // this class is a singleton, use the method Instanciate to create this class
    virtual                 ~HRFGeoTiffCoordSysTable();

    _HDLLg bool                   GetRecord(short pi_Code,
                                             HRFGeoTiffCoordSysRecord* po_pRecord) const;
    _HDLLg bool                   GetRecord(string pi_Datum,
                                             string pi_ProjName,
                                             string pi_Zone,
                                             HRFGeoTiffCoordSysRecord* po_pRecord) const;
    _HDLLg bool                   GetFirst(HRFGeoTiffCoordSysRecord* po_pRecord) const;
    _HDLLg bool                   GetNext (HRFGeoTiffCoordSysRecord* po_pRecord) const;

private:
    HFC_DECLARE_SINGLETON_DLL(_HDLLg, HRFGeoTiffCoordSysTable)

    static HRFGeoTiffCompressedTable::ColumnInfo    s_TableDef[];
    static Byte                                    s_NbColumns;
    static Byte                                    s_CompressedTable[];
    static uint32_t                                 s_CompressedTableSize;
    static uint32_t                                 s_UncompressedTableSize;
    static unsigned short                          s_RecordSize;

    mutable size_t                                  m_CurrentIndex;
    size_t                                          m_NbRecord;

    HArrayAutoPtr<Byte>                            m_pWorkingBuffer;

    void FillRecord         (size_t                    pi_IndexRecord,
                             HRFGeoTiffCoordSysRecord* po_pRecord) const;

    // constructor
    HRFGeoTiffCoordSysTable(const HFCPtr<HFCURL>& pi_rpCoordSysTableFile = 0);

    //:> disable method
    HRFGeoTiffCoordSysTable(const HRFGeoTiffCoordSysTable&);
    HRFGeoTiffCoordSysTable& operator=(const HRFGeoTiffCoordSysTable&);
    };

