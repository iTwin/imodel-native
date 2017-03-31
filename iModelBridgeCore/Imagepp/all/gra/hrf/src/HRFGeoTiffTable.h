//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGeoTiffTable.h $
//:>
//:>  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class GeoTiffRecord
//-----------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HRFGeoTiffCoordSysTable.h>
#include <ImagePP/all/h/HRFGeoTiffProjectionTable.h>
#include <ImagePP/all/h/HRFGeoTiffUnitsTable.h>

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

    const bool     FindUnits           (uint16_t pi_UnitKey,
                                         HRFGeoTiffUnitsTable::HRFGeoTiffUnitsRecord*
                                         po_pRecord);

    const bool     FindCoordSysUnits   (uint16_t pi_GeoKey,
                                         HRFGeoTiffUnitsTable::HRFGeoTiffUnitsRecord*
                                         po_pRecord);

    const bool     FindProjectionUnits (uint16_t pi_GeoKey,
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

