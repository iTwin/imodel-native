//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGeoTiffTable.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class GeoTiffRecord
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HRFGeoTiffCoordSysTable.h>
#include <Imagepp/all/h/HRFGeoTiffProjectionTable.h>
#include <Imagepp/all/h/HRFGeoTiffUnitsTable.h>

class HRFGeoTiffTable
    {
public:

    enum UnitsSource
        {
        ProjectionUnits = 0,
        CoordSysUnits,
        UnitsKey
        };

    HRFGeoTiffTable();
    ~HRFGeoTiffTable();

    const bool     FindUnits           (unsigned short pi_UnitKey,
                                         HRFGeoTiffUnitsTable::HRFGeoTiffUnitsRecord*
                                         po_pRecord);

    const bool     FindCoordSysUnits   (unsigned short pi_GeoKey,
                                         HRFGeoTiffUnitsTable::HRFGeoTiffUnitsRecord*
                                         po_pRecord);

    const bool     FindProjectionUnits (unsigned short pi_GeoKey,
                                         HRFGeoTiffUnitsTable::HRFGeoTiffUnitsRecord*
                                         po_pRecord);

    const bool     LoadTables(bool    pi_CoordSysTable = true,
                               bool    pi_ProjectionTable = true,
                               bool    pi_UnitsTable = true);

    const bool     UnloadTables(bool  pi_CoordSysTable = true,
                                 bool  pi_ProjectionTable = true,
                                 bool  pi_UnitsTable = true);

    static bool    IsClientTxtFileUsed(bool pi_CoordSysTable = true,
                                        bool pi_ProjectionTable = true,
                                        bool pi_UnitsTable = true);

private:

    HRFGeoTiffCoordSysTable*    m_pGeoTiffCoordSysTable;
    HRFGeoTiffProjectionTable*  m_pGeoTiffProjectionTable;
    HRFGeoTiffUnitsTable*       m_pGeoTiffUnitsTable;
    };

