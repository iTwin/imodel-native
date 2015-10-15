//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/IDTMLayerDir.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFileDirectories/IDTMLayerDir.h>

#include <STMInternal/Storage/HTGFFSubDirManager.h>
#include <STMInternal/Storage/HTGFFAttributeManager.h>

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
uint32_t LayerDir::s_GetVersion ()
    {
    return DIRECTORY_VERSION;
    }

LayerDir* LayerDir::Create ()
    {
    return new LayerDir;
    }

LayerDir::~LayerDir ()
    {

    }


LayerDir::LayerDir ()
    :   Directory(DIRECTORY_VERSION),
        m_featureHeaderType(FEATURE_HEADER_TYPE_NONE),
        m_featurePointType(POINT_TYPE_NONE)
    {

    }


PointTypeID LayerDir::GetFeaturePointType () const
    {
    if (POINT_TYPE_NONE != m_featurePointType)
        return m_featurePointType;

    InitTypes();
    return m_featurePointType;
    }

FeatureHeaderTypeID LayerDir::GetFeatureHeaderType () const
    {
    if (FEATURE_HEADER_TYPE_NONE != m_featureHeaderType)
        return m_featureHeaderType;

    InitTypes();
    return m_featureHeaderType;
    }

void LayerDir::InitTypes () const
    {
    const FeatureDir* dirP = GetFirstFeatureDir();
    if (0 == dirP)
        return;

    m_featurePointType = dirP->GetPointType();
    m_featureHeaderType = dirP->GetHeaderType();

    }

const FeatureDir* LayerDir::GetFirstFeatureDir () const
    {
    const FeatureDir* dirP = 0;

    MixedFeatureDirCIter mixedDirIt = MixedFeatureDirsBegin();
    if (mixedDirIt != MixedFeatureDirsEnd())
        dirP = mixedDirIt->Get();
    else
        {
        UniformFeatureDirCIter uniformDirIt = UniformFeatureDirsBegin();

        if (uniformDirIt != UniformFeatureDirsEnd())
            dirP = uniformDirIt->Get();
        }

    return dirP;
    }


Extent3d64f LayerDir::ComputeExtent () const
    {
    struct AccumulateExtent
        {
        Extent3d64f operator () (const Extent3d64f& extent, const FeatureDirEditor& featureDirEditor) const
            {
            const FeatureDir* featureDir = featureDirEditor.Get();
            if (0 == featureDir)
                return extent;
        

            const SpatialIndexDir* spatialIndexDir = featureDir->GetSpatialIndexDir();

            if (0 == spatialIndexDir)
                {
                HASSERT(!"Spatial index does not exist");
                return extent;
                }

            const Extent3d64f& dirExtent = spatialIndexDir->GetContentExtent();


            // Do not accumulate empty or invalid extents

            const bool extentValid = dirExtent.xMin != dirExtent.xMax || dirExtent.yMin != dirExtent.yMax || dirExtent.zMin != dirExtent.zMax;
            if (!extentValid)
                return extent;

            const Extent3d64f newExtent =  {(std::min)(extent.xMin, dirExtent.xMin),
                                            (std::max)(extent.xMax, dirExtent.xMax),
                                            (std::min)(extent.yMin, dirExtent.yMin),
                                            (std::max)(extent.yMax, dirExtent.yMax),
                                            (std::min)(extent.zMin, dirExtent.zMin),
                                            (std::max)(extent.zMax, dirExtent.zMax)
                                           };
            return newExtent;
            }
        };

    // Create an extent that starts at maximal/minimal values for min/max so that any min/max is smaller/greater then values of this extent.
    static const Extent3d64f startingExtent = {(std::numeric_limits<double>::max)(),
                                               -(std::numeric_limits<double>::max)(),
                                               (std::numeric_limits<double>::max)(),
                                               -(std::numeric_limits<double>::max)(),
                                               (std::numeric_limits<double>::max)(),
                                               -(std::numeric_limits<double>::max)()
                                              };

    Extent3d64f extent = std::accumulate(UniformFeatureDirsBegin(), UniformFeatureDirsEnd(), startingExtent, AccumulateExtent());
    extent = std::accumulate(MixedFeatureDirsBegin(), MixedFeatureDirsEnd(), extent, AccumulateExtent());

    // Return empty extent if no extents found
    if (0 == memcmp(&startingExtent, &extent, sizeof(startingExtent)))
        return Extent3d64f();

    return extent;
    }




bool LayerDir::IsPointTypeCompatible (PointTypeID pi_PointType) const
    {
    const PointTypeID pointType = GetFeaturePointType();

    // Ensure that layer stores same point type for all directories
    if (POINT_TYPE_NONE == pointType)
        return true;

    return pointType == pi_PointType;
    }

size_t LayerDir::CountUniformFeatureDirs () const
    {
    return SubDirMgr().GetCount(IDTM_DIRECTORYID_LAYERDIR_UNIFORM_FEATURE_SUBDIRS);
    }

size_t LayerDir::CountPointOnlyUniformFeatureDirs () const
    {
    struct IsPointOnly
        {
        bool operator () (const UniformFeatureDirEditor& editor) const
            {
            const UniformFeatureDir* dirP = editor.Get();
            if (0 == dirP)
                return false;

            return dirP->IsPointOnly();
            }
        };

    return std::count_if(UniformFeatureDirsBegin(), UniformFeatureDirsEnd(), IsPointOnly());
    }


LayerDir::UniformFeatureDirCIter LayerDir::UniformFeatureDirsBegin () const
    {
    return SubDirIterMgr<UniformFeatureDirCIter::value_type>().begin(IDTM_DIRECTORYID_LAYERDIR_UNIFORM_FEATURE_SUBDIRS,
                                                                     SubDirMgr<UniformFeatureDir>());
    }

LayerDir::UniformFeatureDirCIter LayerDir::UniformFeatureDirsEnd () const
    {
    return SubDirIterMgr<UniformFeatureDirCIter::value_type>().end(IDTM_DIRECTORYID_LAYERDIR_UNIFORM_FEATURE_SUBDIRS,
                                                                   SubDirMgr<UniformFeatureDir>());
    }


LayerDir::UniformFeatureDirIter LayerDir::UniformFeatureDirsBegin ()
    {
    return SubDirIterMgr<UniformFeatureDirIter::value_type>().begin(IDTM_DIRECTORYID_LAYERDIR_UNIFORM_FEATURE_SUBDIRS,
                                                                    SubDirMgr<UniformFeatureDir>());
    }

LayerDir::UniformFeatureDirIter LayerDir::UniformFeatureDirsEnd ()
    {
    return SubDirIterMgr<UniformFeatureDirIter::value_type>().end(IDTM_DIRECTORYID_LAYERDIR_UNIFORM_FEATURE_SUBDIRS,
                                                                  SubDirMgr<UniformFeatureDir>());
    }


bool LayerDir::HasUniformFeatureDir (FeatureType pi_FeatureType) const
{
    return SubDirMgr().Has(IDTM_DIRECTORYID_LAYERDIR_UNIFORM_FEATURE_SUBDIRS, pi_FeatureType);
    }

const UniformFeatureDir* LayerDir::GetUniformFeatureDir (FeatureType pi_FeatureType) const
    {
    return const_cast<LayerDir*>(this)->GetUniformFeatureDir(pi_FeatureType);
    }

UniformFeatureDir* LayerDir::GetUniformFeatureDir (FeatureType pi_FeatureType)
    {
    return SubDirMgr<UniformFeatureDir>().Get(IDTM_DIRECTORYID_LAYERDIR_UNIFORM_FEATURE_SUBDIRS, pi_FeatureType);
    }


UniformFeatureDir* LayerDir::CreateUniformFeatureDir   (FeatureType         pi_FeatureType,
                                                        PointTypeID         pi_PointType,
                                                        const Compression& pi_CompressType)
    {
    if (!IsPointTypeCompatible(pi_PointType))
        return 0;

    static const UniformFeatureDir::Options OPTIONS(false);
    const CreateConfig DirCreateConfig(GetTypeDescriptor(pi_PointType),
                                       pi_CompressType);

    return SubDirMgr<UniformFeatureDir>().Create(IDTM_DIRECTORYID_LAYERDIR_UNIFORM_FEATURE_SUBDIRS, pi_FeatureType, DirCreateConfig, &OPTIONS);
    }

UniformFeatureDir* LayerDir::CreatePointsOnlyUniformFeatureDir     (FeatureType         pi_FeatureType,
                                                                    PointTypeID         pi_PointType,
                                                                    const Compression&  pi_CompressType)
    {
    if (!IsPointTypeCompatible(pi_PointType))
        return 0;

    static const UniformFeatureDir::Options OPTIONS(true);
    const CreateConfig DirCreateConfig(GetTypeDescriptor(pi_PointType),
                                       pi_CompressType);

    return SubDirMgr<UniformFeatureDir>().Create(IDTM_DIRECTORYID_LAYERDIR_UNIFORM_FEATURE_SUBDIRS, pi_FeatureType, DirCreateConfig, &OPTIONS);
    }


size_t LayerDir::CountMixedFeatureDirs () const
    {
    return SubDirMgr().GetCount(IDTM_DIRECTORYID_LAYERDIR_MIXED_FEATURE_SUBDIRS);
    }


LayerDir::MixedFeatureDirCIter LayerDir::MixedFeatureDirsBegin () const
    {
    return SubDirIterMgr<MixedFeatureDirCIter::value_type>().begin(IDTM_DIRECTORYID_LAYERDIR_MIXED_FEATURE_SUBDIRS,
                                                                   SubDirMgr<MixedFeatureDir>());
    }

LayerDir::MixedFeatureDirCIter LayerDir::MixedFeatureDirsEnd () const
    {
    return SubDirIterMgr<MixedFeatureDirCIter::value_type>().end(IDTM_DIRECTORYID_LAYERDIR_MIXED_FEATURE_SUBDIRS,
                                                                 SubDirMgr<MixedFeatureDir>());
    }


LayerDir::MixedFeatureDirIter LayerDir::MixedFeatureDirsBegin ()
    {
    return SubDirIterMgr<MixedFeatureDirIter::value_type>().begin(IDTM_DIRECTORYID_LAYERDIR_MIXED_FEATURE_SUBDIRS,
                                                                  SubDirMgr<MixedFeatureDir>());
    }

LayerDir::MixedFeatureDirIter LayerDir::MixedFeatureDirsEnd ()
    {
    return SubDirIterMgr<MixedFeatureDirIter::value_type>().end(IDTM_DIRECTORYID_LAYERDIR_MIXED_FEATURE_SUBDIRS,
                                                                SubDirMgr<MixedFeatureDir>());
    }

bool LayerDir::HasMixedFeatureDir (size_t pi_Index) const
{
    return SubDirMgr().Has(IDTM_DIRECTORYID_LAYERDIR_MIXED_FEATURE_SUBDIRS, pi_Index);
    }

const MixedFeatureDir* LayerDir::GetMixedFeatureDir (size_t pi_Index) const
    {
    return const_cast<LayerDir*>(this)->GetMixedFeatureDir(pi_Index);
    }

MixedFeatureDir* LayerDir::GetMixedFeatureDir (size_t pi_Index)
    {
    return SubDirMgr<MixedFeatureDir>().Get(IDTM_DIRECTORYID_LAYERDIR_MIXED_FEATURE_SUBDIRS, pi_Index);
}


MixedFeatureDir* LayerDir::CreateMixedFeatureDir   (size_t              pi_Index,
                                                    PointTypeID         pi_PointType,
                                                    const Compression&  pi_CompressType)
{
    if (!IsPointTypeCompatible(pi_PointType))
        return 0;

    const CreateConfig DirCreateConfig(GetTypeDescriptor(pi_PointType),
                                       pi_CompressType);

    return SubDirMgr<MixedFeatureDir>().Create(IDTM_DIRECTORYID_LAYERDIR_MIXED_FEATURE_SUBDIRS, pi_Index, DirCreateConfig);
    }

MixedFeatureDir* LayerDir::AddMixedFeatureDir  (PointTypeID          pi_PointType,
                                                const Compression&   pi_CompressType)
    {
    if (!IsPointTypeCompatible(pi_PointType))
        return 0;

    const CreateConfig DirCreateConfig(GetTypeDescriptor(pi_PointType),
                                       pi_CompressType);

    return SubDirMgr<MixedFeatureDir>().Add(IDTM_DIRECTORYID_LAYERDIR_MIXED_FEATURE_SUBDIRS, DirCreateConfig);
    }

bool LayerDir::HasWkt () const
    {
    return AttributeMgr().IsPresent(IDTM_ATTRIBUTEID_LAYERDIR_WKT_SPATIAL_REFERENCE_SYSTEM);
    }

bool LayerDir::SetWkt (const HCPWKT& pi_wkt)
        {
    if (!pi_wkt.IsEmpty())
        return AttributeMgr().Set(IDTM_ATTRIBUTEID_LAYERDIR_WKT_SPATIAL_REFERENCE_SYSTEM, pi_wkt.GetCStr());

    if (AttributeMgr().IsPresent(IDTM_ATTRIBUTEID_LAYERDIR_WKT_SPATIAL_REFERENCE_SYSTEM))
        return AttributeMgr().Remove(IDTM_ATTRIBUTEID_LAYERDIR_WKT_SPATIAL_REFERENCE_SYSTEM);
    return true;
    }

HCPWKT LayerDir::GetWkt () const
    {
    WCharP pUnicodeWkt = NULL;
    if(AttributeMgr().Get(IDTM_ATTRIBUTEID_LAYERDIR_WKT_SPATIAL_REFERENCE_SYSTEM, pUnicodeWkt))
        return HCPWKT(pUnicodeWkt);

    return HCPWKT();
    }

const DTMMetadataDir* LayerDir::GetMetadataDir () const
    {
    return const_cast<LayerDir*>(this)->GetMetadataDir();
    }

DTMMetadataDir* LayerDir::GetMetadataDir ()
    {
    return SubDirMgr<DTMMetadataDir>().Get(IDTM_DIRECTORYID_LAYERDIR_METADATA_SUBDIR);
    }


DTMMetadataDir* LayerDir::CreateMetadataDir ()
    {
    const CreateConfig DirCreateConfig;
    return SubDirMgr<DTMMetadataDir>().Create(IDTM_DIRECTORYID_LAYERDIR_METADATA_SUBDIR, DirCreateConfig);
    }


inline LayerDir::FeatureDirEditor::FeatureDirEditor () 
    {
    }

inline LayerDir::FeatureDirEditor::FeatureDirEditor (const FeatureDirEditor& rhs) 
    :   super_class(rhs) 
    {
    }


LayerDir::UniformFeatureDirEditor::UniformFeatureDirEditor ()
    {    
    }

LayerDir::UniformFeatureDirEditor::UniformFeatureDirEditor (const UniformFeatureDirEditor& rhs)
    :   FeatureDirEditor(rhs)
    {
    }

inline const UniformFeatureDir* LayerDir::UniformFeatureDirEditor::_Get () const
    {
    return AsTypedMgr<UniformFeatureDir>(GetBase()).Get(GetSubDirTagID(), GetSubDirIDIter());
    }

inline UniformFeatureDir* LayerDir::UniformFeatureDirEditor::_Get ()
    {
    return AsTypedMgr<UniformFeatureDir>(GetBase()).Get(GetSubDirTagID(), GetSubDirIDIter());
    }

const UniformFeatureDir* LayerDir::UniformFeatureDirEditor::Get () const
    {
    return _Get();
    }

UniformFeatureDir* LayerDir::UniformFeatureDirEditor::Get ()
    {
    return _Get();
    }



LayerDir::MixedFeatureDirEditor::MixedFeatureDirEditor ()
    {    
    }

LayerDir::MixedFeatureDirEditor::MixedFeatureDirEditor (const MixedFeatureDirEditor& rhs)
    :   FeatureDirEditor(rhs)
    {
    }

inline const MixedFeatureDir* LayerDir::MixedFeatureDirEditor::_Get () const
    {
    return AsTypedMgr<MixedFeatureDir>(GetBase()).Get(GetSubDirTagID(), GetSubDirIDIter());
    }

inline MixedFeatureDir* LayerDir::MixedFeatureDirEditor::_Get ()
    {
    return AsTypedMgr<MixedFeatureDir>(GetBase()).Get(GetSubDirTagID(), GetSubDirIDIter());
    }

const MixedFeatureDir* LayerDir::MixedFeatureDirEditor::Get () const
    {
    return _Get();
    }

MixedFeatureDir* LayerDir::MixedFeatureDirEditor::Get ()
    {
    return _Get();
    }

const FeatureDir* LayerDir::FeatureDirEditor::Get () const
    {
    return _Get();
    }

FeatureDir* LayerDir::FeatureDirEditor::Get ()
    {
    return _Get();
    }

} // End namespace IDTMFile