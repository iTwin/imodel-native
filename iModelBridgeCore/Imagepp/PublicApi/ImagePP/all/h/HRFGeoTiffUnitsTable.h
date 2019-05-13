//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFGeoTiffUnitsTable
//-----------------------------------------------------------------------------
#pragma once

#include "HRFGeoTiffCompressedTable.h"
#include "HFCMacros.h"

/** -----------------------------------------------------------------------------
    @version 1.0
    @author  Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})

    This class is used to keep in memory some information related to the GeoTiff
    units. For the first version, the data is always compressed with Deflate.

    @h3{Notes}

    The file HRFGeoTiffUnitsTable.dat and HRFGeoTiffUnitsTable.tbl must be exist.
    The file HRFGeoTiffUnitsTable.dat can be create with the
    HRFGeoTiffCompressTable.exe application.

    @see HRFGeoTiffCompressedTable
    @see HRFGeoTiffCompressTable application
    -----------------------------------------------------------------------------
*/

BEGIN_IMAGEPP_NAMESPACE
class HRFGeoTiffUnitsTable : public HRFGeoTiffCompressedTable
    {
public:

    HDECLARE_CLASS_ID(HRFGeoTiffId_UnitsTable, HRFGeoTiffCompressedTable);

    /** -----------------------------------------------------------------------------
        HRFGeoTiffUnitsRecord
        -----------------------------------------------------------------------------
    */
    typedef struct
        {
        int16_t Code;
        string      EPSGName;
        string      EPSGAbbr;
        int16_t TargetUnitCode;
        double     FactorB;
        double     FactorC;
        } HRFGeoTiffUnitsRecord;


    //:> Primary methods.
    // this class is a singleton, use Instanciate to create the class
    virtual                 ~HRFGeoTiffUnitsTable();

    IMAGEPP_EXPORT bool                   GetRecord(int16_t pi_Code,
                                             HRFGeoTiffUnitsRecord*    po_pRecord) const;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT , HRFGeoTiffUnitsTable)

    static HRFGeoTiffCompressedTable::ColumnInfo    s_TableDef[];
    static Byte                                    s_NbColumns;
    static Byte                                    s_CompressedTable[];
    static uint32_t                                 s_CompressedTableSize;
    static uint32_t                                 s_UncompressedTableSize;
    static uint16_t                          s_RecordSize;

    // constructor
    HRFGeoTiffUnitsTable(const HFCPtr<HFCURL>& pi_rpUnitsTableFile = 0);

    //:> disable method
    HRFGeoTiffUnitsTable(const HRFGeoTiffUnitsTable&);
    HRFGeoTiffUnitsTable& operator=(const HRFGeoTiffUnitsTable&);
    };
END_IMAGEPP_NAMESPACE

