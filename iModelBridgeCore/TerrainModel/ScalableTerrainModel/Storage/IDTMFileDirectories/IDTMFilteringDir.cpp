//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/IDTMFilteringDir.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFileDirectories/IDTMFilteringDir.h>

#include <STMInternal/Storage/HTGFFSubDirManager.h>
#include <STMInternal/Storage/HTGFFAttributeManager.h>

#include "IDTMCommonDirTools.h"
#include "../IDTMFileDefinition.h"

using namespace HTGFF;

namespace {
/* 
 * VERSIONNING 
 * Version 0:
 *      Current.
 */

const uint32_t DIRECTORY_VERSION = 0;
}


namespace IDTMFile {


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t FilteringDir::s_GetVersion ()
    {
    return DIRECTORY_VERSION;
    }

namespace { // BEGIN unnamed namspace

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
class FilteringStatusTypeDef
    {
    enum DimensionRole
        {
        DR_Status,
        DR_QTY,
        };

public:
    static const DataType&      GetType                ()
        {
        static const DataType DATA_TYPE (Dimension::CreateFrom(Dimension::TYPE_UINT8, DR_Status));
        return DATA_TYPE;
        }
    };


} // END unnamed namespace


FilteringDir::Options::Options (FilterType pi_Type)
    :   m_Type(pi_Type)
    {

    }

FilteringDir::Options::~Options ()
    {
    };

FilterType FilteringDir::Options::GetType () const
    {
    return m_Type;
    }



FilteringDir::FilteringDir ()
    :   Directory(DIRECTORY_VERSION),
        m_pNodesFilteringStatusDir(0)
    {

    }

FilteringDir::~FilteringDir ()
    {

    }

FilterType FilteringDir::GetType () const
    {
    uint32_t Type = 0;
    if (!AttributeMgr().Get (IDTM_ATTRIBUTEID_FILTERINGDIR_TYPE, Type))
        {
        HASSERT(!"Filter type was not set");
        }
    else if (Type >= FILTER_TYPE_QTY)
        {
        HASSERT(!"Invalid filter type");
        Type = 0;
        }

    return static_cast<FilterType>(Type);
    }

bool FilteringDir::SetType (FilterType pi_Type)
    {
    HASSERT(pi_Type < FILTER_TYPE_QTY);
    return AttributeMgr().Set (IDTM_ATTRIBUTEID_FILTERINGDIR_TYPE, static_cast<uint32_t>(pi_Type));
    }

bool FilteringDir::IsFiltered (TileID pi_ID) const
    {
    HPRECONDITION(pi_ID != GetNullTileID());
    return m_pNodesFilteringStatusDir->Get(pi_ID) != 0;
    }


void FilteringDir::SetFiltered (TileID     pi_ID,
                                bool           pi_Filtered)
    {
    HPRECONDITION(pi_ID != GetNullTileID());
    m_pNodesFilteringStatusDir->Edit(pi_ID) = pi_Filtered;
    }


bool FilteringDir::_Create (const CreateConfig&     pi_rCreateConfig,
                            const UserOptions*      pi_pUserOptions)
    {
    static NodesFilteringStatusDir::Options NODESFILTERINGSTATUS_DIR_OPTIONS(IDTM_DEFAULT.filtered);

    const CreateConfig NodesFilteringStatusDirConfig(FilteringStatusTypeDef::GetType(), pi_rCreateConfig.m_DataCompressType);

    bool Success = true;

    m_pNodesFilteringStatusDir = SubDirMgr<NodesFilteringStatusDir>().Create(IDTM_DIRECTORYID_FILTERINGDIR_NODES_FILTERED_STATUS_SUBDIR,
                                                                             NodesFilteringStatusDirConfig,
                                                                             &NODESFILTERINGSTATUS_DIR_OPTIONS);
    Success &= 0 != m_pNodesFilteringStatusDir;


    HPRECONDITION(0 != pi_pUserOptions);
    const Options& rOptions = pi_pUserOptions->SafeReinterpretAs<Options>();


    Success &= SetType(rOptions.GetType());
    Success &= rOptions._SaveTo(*this);

    // Set default filtered value for default top tile
    SetFiltered(IDTM_DEFAULT.topNode, IDTM_DEFAULT.filtered);

    return Success;
    }

bool FilteringDir::_Load (const UserOptions*      pi_pUserOptions)
    {
    static NodesFilteringStatusDir::Options NODESFILTERINGSTATUS_DIR_OPTIONS(IDTM_DEFAULT.filtered);

    bool Success = true;

    if (0 == m_pNodesFilteringStatusDir)
        m_pNodesFilteringStatusDir = SubDirMgr<NodesFilteringStatusDir>().Get(IDTM_DIRECTORYID_FILTERINGDIR_NODES_FILTERED_STATUS_SUBDIR,
                                                                              &NODESFILTERINGSTATUS_DIR_OPTIONS);
    Success &= 0 != m_pNodesFilteringStatusDir;


    return Success;
    }

bool FilteringDir::_Save ()
    {
    bool Success = true;
    return Success;
    }



DumbFilteringHandler::Options::Options ()
    :   FilteringDir::Options(FILTER_TYPE_DUMB)

    {
    }

bool DumbFilteringHandler::Options::_SaveTo (FilteringDir& pi_rDir) const
    {
    DumbFilteringHandler Handler(&pi_rDir);
    return Handler.Create(*this);
    }

DumbFilteringHandler::DumbFilteringHandler (FilteringDir* pi_rpDir)
    :   super_class(*pi_rpDir),
        m_pDir(pi_rpDir)
    {

    }

DumbFilteringHandler::~DumbFilteringHandler ()
    {
    const bool Success = Save();
    HASSERT(Success);
    }

bool DumbFilteringHandler::IsCompatibleWith (const FilteringDir& pi_rDir)
    {
    return FILTER_TYPE_DUMB == pi_rDir.GetType();
    }


DumbFilteringHandler::Ptr DumbFilteringHandler::CreateFrom (FilteringDir* pi_pDir)
    {
    return CreateFromImpl(pi_pDir);
    }


bool DumbFilteringHandler::_Save ()
    {
    return true;
    }

bool DumbFilteringHandler::_Load ()
    {
    return true;
    }

bool DumbFilteringHandler::Create (const Options& pi_rOptions)
    {
    return true;
    }

} // END namespace IDTMFile
