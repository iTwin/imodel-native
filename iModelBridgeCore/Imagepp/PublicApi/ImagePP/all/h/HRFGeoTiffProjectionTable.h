//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HRFGeoTiffProjectionTable.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HRFGeoTiffProjectionTable
//-----------------------------------------------------------------------------
#pragma once

#include "HRFGeoTiffCompressedTable.h"
#include "HFCMacros.h"

/** -----------------------------------------------------------------------------
    @version 1.0
    @author  Ghislain Tardif (${mailto:Ghislain.Tardif@Bentley.com})

    This class is used to keep in memory some information related to the GeoTiff
    transfo model. For the first version, the data is always compressed with Deflate.

    @h3{Notes}

    The file HRFGeoTiffProjectionTable.dat and HRFGeoTiffTransModel.tbl must be exist.
    The file HRFGeoTiffTrasnfoModelTable.dat can be create with the
    HRFGeoTiffCompressTable.exe application.

    @see HRFGeoTiffCompressedTable
    @see HRFGeoTiffCompressTable application
    -----------------------------------------------------------------------------
*/

BEGIN_IMAGEPP_NAMESPACE
class HRFGeoTiffProjectionTable : public HRFGeoTiffCompressedTable
    {
public:

    HDECLARE_CLASS_ID(HRFGeoTiffId_ProjectionTable, HRFGeoTiffCompressedTable);
    /** -----------------------------------------------------------------------------
        HRFGeoTiffProjectionRecord
        -----------------------------------------------------------------------------
    */

    typedef struct
        {
        short Code;
        short UnitsCode;
        } HRFGeoTiffProjectionRecord;

    //:> Primary methods.
    // this class is a singleton, use method Instanciate to create the class
    virtual                 ~HRFGeoTiffProjectionTable();

    bool                   GetRecord(short pi_Code,
                                      HRFGeoTiffProjectionRecord*   po_pRecord) const;

private:
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT , HRFGeoTiffProjectionTable)

    static HRFGeoTiffCompressedTable::ColumnInfo    s_TableDef[];
    static Byte                                    s_NbColumns;
    static Byte                                    s_CompressedTable[];
    static uint32_t                                 s_CompressedTableSize;
    static uint32_t                                 s_UncompressedTableSize;
    static unsigned short                          s_RecordSize;

    // constructor
    HRFGeoTiffProjectionTable(const HFCPtr<HFCURL>& pi_rpProjectionTableFile = 0);

    //:> disable method
    HRFGeoTiffProjectionTable(const HRFGeoTiffProjectionTable&);
    HRFGeoTiffProjectionTable& operator=(const HRFGeoTiffProjectionTable&);
    };
END_IMAGEPP_NAMESPACE

