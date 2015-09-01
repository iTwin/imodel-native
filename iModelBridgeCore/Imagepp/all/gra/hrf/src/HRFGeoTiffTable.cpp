//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGeoTiffTable.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class HRFGeoTiffTable
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include "HRFGeoTiffTable.h"

//-----------------------------------------------------------------------------
// public
// Construction
//-----------------------------------------------------------------------------
HRFGeoTiffTable::HRFGeoTiffTable()
    :   m_pGeoTiffCoordSysTable(0),
        m_pGeoTiffProjectionTable(0),
        m_pGeoTiffUnitsTable(0)
    {
    }

//-----------------------------------------------------------------------------
// public
// Destruction
//-----------------------------------------------------------------------------
HRFGeoTiffTable::~HRFGeoTiffTable()
    {
    const bool CoordSysTableLoaded = 0 != m_pGeoTiffCoordSysTable;
    const bool ProjectionTableLoaded = 0 != m_pGeoTiffProjectionTable;
    const bool UnitTableLoaded = 0 != m_pGeoTiffUnitsTable;

    // Tables should have been previously unloaded with a UnloadTables call that match the load call.
    HPRECONDITION(!CoordSysTableLoaded);
    HPRECONDITION(!ProjectionTableLoaded);
    HPRECONDITION(!UnitTableLoaded);

    // Unload tables anyway in order to avoid further problems...
    UnloadTables(CoordSysTableLoaded, ProjectionTableLoaded, UnitTableLoaded);
    }

//-----------------------------------------------------------------------------
// public
// IsClientTxtFileUsed
// Return true is client's customized geocoding data .txt files are used instead of
// library packed compressed .dat files.
//-----------------------------------------------------------------------------
bool HRFGeoTiffTable::IsClientTxtFileUsed(bool pi_CoordSysTable,
                                           bool pi_ProjectionTable,
                                           bool pi_UnitsTable)
    {
    bool IsCustomTxtFileUse = false;

    if (pi_CoordSysTable)
        {
        IsCustomTxtFileUse = (HRFGeoTiffCoordSysTable::GetInstance()->GetTableFileName() != 0);
        }

    if (pi_ProjectionTable && (IsCustomTxtFileUse == false))
        {
        IsCustomTxtFileUse = (HRFGeoTiffProjectionTable::GetInstance()->GetTableFileName() != 0);
        }

    if (pi_UnitsTable && (IsCustomTxtFileUse == false))
        {
        IsCustomTxtFileUse = (HRFGeoTiffUnitsTable::GetInstance()->GetTableFileName() != 0);
        }

    return IsCustomTxtFileUse;
    }

//-----------------------------------------------------------------------------
// public
// FindUnitsRecord
//-----------------------------------------------------------------------------
const bool HRFGeoTiffTable::FindUnits(unsigned short          pi_UnitKey,
                                       HRFGeoTiffUnitsTable::HRFGeoTiffUnitsRecord*
                                       po_pRecord)
    {
    HPRECONDITION(po_pRecord != 0);

    return m_pGeoTiffUnitsTable->GetRecord(pi_UnitKey, po_pRecord);
    }

//-----------------------------------------------------------------------------
// public
// FindGeotifcsRecord
//-----------------------------------------------------------------------------
const bool HRFGeoTiffTable::FindCoordSysUnits(unsigned short  pi_GeoKey,
                                               HRFGeoTiffUnitsTable::HRFGeoTiffUnitsRecord*
                                               po_pRecord)
    {
    HPRECONDITION(po_pRecord != 0);

    bool Found = false;
    HRFGeoTiffCoordSysTable::HRFGeoTiffCoordSysRecord CoordSysRecord;
    if (m_pGeoTiffCoordSysTable->GetRecord(pi_GeoKey, &CoordSysRecord))
        {
        Found = m_pGeoTiffUnitsTable->GetRecord(CoordSysRecord.UnitsCode, po_pRecord);
        }
    return Found;
    }

//-----------------------------------------------------------------------------
// public
// FindGeotifcsProjectionRecord
//-----------------------------------------------------------------------------
const bool HRFGeoTiffTable::FindProjectionUnits(unsigned short        pi_GeoKey,
                                                 HRFGeoTiffUnitsTable::HRFGeoTiffUnitsRecord*
                                                 po_pRecord)
    {
    HPRECONDITION(po_pRecord != 0);

    bool Found = false;

    HRFGeoTiffProjectionTable::HRFGeoTiffProjectionRecord ProjectionRecord;
    if (m_pGeoTiffProjectionTable->GetRecord(pi_GeoKey, &ProjectionRecord))
        {
        Found = m_pGeoTiffUnitsTable->GetRecord(ProjectionRecord.UnitsCode, po_pRecord);
        }

    return Found;
    }

//-----------------------------------------------------------------------------
// public
// LoadTables
//-----------------------------------------------------------------------------
const bool HRFGeoTiffTable::LoadTables(bool   pi_CoordSysTable,
                                        bool   pi_ProjectionTable,
                                        bool   pi_UnitsTable)
    {
    if (pi_CoordSysTable)
        {
        HASSERT(0 == m_pGeoTiffCoordSysTable);
        m_pGeoTiffCoordSysTable = HRFGeoTiffCoordSysTable::GetInstance();
        m_pGeoTiffCoordSysTable->LockTable();
        }

    if (pi_ProjectionTable)
        {
        HASSERT(0 == m_pGeoTiffProjectionTable);
        m_pGeoTiffProjectionTable = HRFGeoTiffProjectionTable::GetInstance();
        m_pGeoTiffProjectionTable->LockTable();
        }

    if (pi_UnitsTable)
        {
        HASSERT(0 == m_pGeoTiffUnitsTable);
        m_pGeoTiffUnitsTable = HRFGeoTiffUnitsTable::GetInstance();
        m_pGeoTiffUnitsTable->LockTable();
        }

    return true;
    }

//-----------------------------------------------------------------------------
// public
// UnloadTables
//-----------------------------------------------------------------------------
const bool HRFGeoTiffTable::UnloadTables(bool   pi_CoordSysTable,
                                          bool   pi_ProjectionTable,
                                          bool   pi_UnitsTable)
    {

    if (pi_CoordSysTable)
        {
        HASSERT(0 != m_pGeoTiffCoordSysTable);
        m_pGeoTiffCoordSysTable->ReleaseTable();
        m_pGeoTiffCoordSysTable = 0;
        }

    if (pi_ProjectionTable)
        {
        HASSERT(0 != m_pGeoTiffProjectionTable);
        m_pGeoTiffProjectionTable->ReleaseTable();
        m_pGeoTiffProjectionTable = 0;
        }

    if (pi_UnitsTable)
        {
        HASSERT(0 != m_pGeoTiffUnitsTable);
        m_pGeoTiffUnitsTable->ReleaseTable();
        m_pGeoTiffUnitsTable = 0;
        }

    return true;
    }
