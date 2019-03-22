//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/IDTMFeatureDir.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFileDirectories/IDTMFeatureDir.h>
#include "../IDTMFileDefinition.h"

#include <STMInternal/Storage/HTGFFSubDirManager.h>

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
uint32_t FeatureDir::s_GetVersion ()
    {
    return DIRECTORY_VERSION;
    }

struct FeatureDir::Impl
    {

    };


FeatureDir::FeatureDir ()
    :   Directory(DIRECTORY_VERSION),
        m_pHeaderDir(0),
        m_pPointDir(0)
    {

    }

FeatureDir::~FeatureDir ()
    {
    }

bool FeatureDir::IsPointOnly () const
    {
    return !HasHeaderDir();
    }

bool FeatureDir::IsUniform () const
    {
    return _IsUniform();
    }


FeatureHeaderTypeID FeatureDir::GetHeaderType () const
    {
    if (IsPointOnly())
        return FEATURE_HEADER_TYPE_FEATURE; // TDORAY: Maybe this should be stored ?

    return GetHeaderDir().GetHeaderType();
    }

size_t FeatureDir::GetTileMaxFeatureCount () const
    {
    if (IsPointOnly())
        return 1;

    return GetHeaderDir().GetTileMaxHeaderCount();
    }

uint64_t FeatureDir::CountFeatures () const
    {
    if (IsPointOnly())
        return GetPointDir().CountTiles();

    return GetHeaderDir().CountHeaders();
    }

size_t FeatureDir::CountFeatures (TileID pi_ID) const
    {
    if (IsPointOnly())
        return 1;

    return GetHeaderDir().CountHeaders(pi_ID);
    }



bool FeatureDir::HasMetadataDir () const
    {
    return SubDirMgr().Has(IDTM_DIRECTORYID_FEATUREDIR_METADATA_SUBDIR);
    }

/*---------------------------------------------------------------------------------**//**
* @description  Returns the primitive metadata dir for the current tile dir.
* @return       A valid metadata dir on success, NULL pointer otherwise.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const PrimitiveMetadataDir* FeatureDir::GetMetadataDir () const
    {
    return const_cast<FeatureDir*>(this)->GetMetadataDir();
    }

PrimitiveMetadataDir* FeatureDir::GetMetadataDir ()
    {
    return SubDirMgr<PrimitiveMetadataDir>().Get(IDTM_DIRECTORYID_FEATUREDIR_METADATA_SUBDIR);
    }


/*---------------------------------------------------------------------------------**//**
* @description  Create the primitive metadata dir for the current tile dir if it does
*               not already exists.
* @return       A valid metadata dir on success, NULL pointer otherwise.
* @bsimethod                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveMetadataDir* FeatureDir::CreateMetadataDir ()
    {
    const CreateConfig DirCreateConfig(DataType(Dimension::CreateFrom(Dimension::TYPE_UINT8)), Compression::None::Create());

    return SubDirMgr<PrimitiveMetadataDir>().Create(IDTM_DIRECTORYID_FEATUREDIR_METADATA_SUBDIR,
                                                    DirCreateConfig);
    }

bool FeatureDir::HasSpatialIndexDir () const
    {
    return SubDirMgr().Has(IDTM_DIRECTORYID_FEATUREDIR_SPATIAL_INDEX_SUBDIR);
    }

const SpatialIndexDir* FeatureDir::GetSpatialIndexDir  () const
    {
    return const_cast<FeatureDir*>(this)->GetSpatialIndexDir();
    }

SpatialIndexDir* FeatureDir::GetSpatialIndexDir ()
    {
    return SubDirMgr<SpatialIndexDir>().Get(IDTM_DIRECTORYID_FEATUREDIR_SPATIAL_INDEX_SUBDIR);
    }

SpatialIndexDir* FeatureDir::CreateSpatialIndexDir   (const SpatialIndexDir::Options& pi_rOptions)
    {
    const CreateConfig DirCreateConfig(DataType::CreateVoid(),
                                       Compression::None::Create());

    return SubDirMgr<SpatialIndexDir>().Create(IDTM_DIRECTORYID_FEATUREDIR_SPATIAL_INDEX_SUBDIR, DirCreateConfig, &pi_rOptions);
    }


bool FeatureDir::HasFilteringDir () const
    {
    return SubDirMgr().Has(IDTM_DIRECTORYID_FEATUREDIR_FILTERING_SUBDIR);
    }

const FilteringDir* FeatureDir::GetFilteringDir () const
    {
    return const_cast<FeatureDir*>(this)->GetFilteringDir();
    }

FilteringDir* FeatureDir::GetFilteringDir ()
    {
    return SubDirMgr<FilteringDir>().Get(IDTM_DIRECTORYID_FEATUREDIR_FILTERING_SUBDIR);
    }

FilteringDir* FeatureDir::CreateFilteringDir (const FilteringDir::Options& pi_rOptions)
    {
    const CreateConfig DirCreateConfig(DataType::CreateVoid(),
                                       Compression::None::Create());

    return SubDirMgr<FilteringDir>().Create(IDTM_DIRECTORYID_FEATUREDIR_FILTERING_SUBDIR, DirCreateConfig, &pi_rOptions);
    }


bool FeatureDir::_Create   (const CreateConfig&     pi_rCreateConfig,
                            const UserOptions*      pi_pUserOptions)
    {
    bool Success = true;

    // Ensure that our sub header/data dirs are created
    const CreateConfig PointDirConfig(pi_rCreateConfig.m_DataType, pi_rCreateConfig.m_DataCompressType);
    m_pPointDir = SubDirMgr<PointDir>().Create(IDTM_DIRECTORYID_FEATUREDIR_POINT_SUBDIR, PointDirConfig);
    Success &= (0 != m_pPointDir);

    Success &= _CreateFeatureHeaderDir(m_pHeaderDir, pi_rCreateConfig, pi_pUserOptions);

    return Success;
    }

bool FeatureDir::_Load (const UserOptions*      pi_pUserOptions)
    {
    bool Success = true;

    // Ensure that our sub header/data dirs are loaded.
    if (0 == m_pPointDir)
        {
        m_pPointDir = SubDirMgr<PointDir>().Get(IDTM_DIRECTORYID_FEATUREDIR_POINT_SUBDIR);
        Success &= (0 != m_pPointDir);
        }

    if (0 == m_pHeaderDir)
        {
        Success &= _LoadFeatureHeaderDir(m_pHeaderDir, pi_pUserOptions);
        }

    return Success;
    }


bool FeatureDir::CreatePointOnlyFeatureHeaderDir   (HeaderDir*&         po_rpHeaderDir,
                                                    FeatureType         pi_FeatureType,
                                                    const CreateConfig& pi_rCreateConfig)
    {
    po_rpHeaderDir = 0;
    return true;
    }

bool FeatureDir::LoadPointOnlyFeatureHeaderDir     (HeaderDir*&     po_rpHeaderDir,
                                                    FeatureType     pi_FeatureType)
    {
    po_rpHeaderDir = 0;
    return true;
    }

bool FeatureDir::CreateLinearFeatureHeaderDir  (HeaderDir*&         po_rpHeaderDir,
                                                const CreateConfig& pi_rCreateConfig)
    {
    const CreateConfig HeaderDirConfig(GetTypeDescriptor(FEATURE_HEADER_TYPE_FEATURE),
                                       pi_rCreateConfig.m_DataCompressType);
    po_rpHeaderDir = SubDirMgr<FeatureHeaderDir>().Create(IDTM_DIRECTORYID_FEATUREDIR_HEADER_SUBDIR, HeaderDirConfig);
    return 0 != po_rpHeaderDir;
    }

bool FeatureDir::LoadLinearFeatureHeaderDir (HeaderDir*&    po_rpHeaderDir)
    {
    po_rpHeaderDir = SubDirMgr<FeatureHeaderDir>().Get(IDTM_DIRECTORYID_FEATUREDIR_HEADER_SUBDIR);
    return 0 != po_rpHeaderDir;
    }


bool FeatureDir::DefaultCreateFeatureHeaderDir (HeaderDir*&         po_rpHeaderDir,
                                                const CreateConfig& pi_rCreateConfig)
    {
    return CreateLinearFeatureHeaderDir(po_rpHeaderDir, pi_rCreateConfig);
    }

bool FeatureDir::DefaultLoadFeatureHeaderDir (HeaderDir*& po_rpHeaderDir)
    {
    return LoadLinearFeatureHeaderDir(po_rpHeaderDir);
    }



PointDir* FeatureDir::GetPointDirP () const
    {
    return m_pPointDir;
    }

FeatureDir::HeaderDir* FeatureDir::GetHeaderDirP () const
    {
    return m_pHeaderDir;
    }

bool FeatureDir::HasHeaderDir () const
    {
    return 0 != m_pHeaderDir;
    }

FeatureDir::HeaderDir& FeatureDir::GetHeaderDir () const
    {
    HPRECONDITION(0 != m_pHeaderDir);
    return *m_pHeaderDir;
    }


PointDir& FeatureDir::GetPointDir () const
    {
    HPRECONDITION(0 != m_pPointDir);
    return *m_pPointDir;
    }

} // End namespace IDTMFile