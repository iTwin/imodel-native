//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/IDTMSpatialIndexDir.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFileDirectories/IDTMSpatialIndexDir.h>
#include <STMInternal/Storage/HTGFFAttributeManager.h>
#include <STMInternal/Storage/HTGFFSubDirManager.h>

#include "IDTMCommonDirTools.h"
#include "../IDTMFileDefinition.h"
#include <STMInternal/Storage/IDTMTypes.h>

#include <STMInternal/Storage/HTGFFSubDirHelpers.h>

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
uint32_t SpatialIndexDir::s_GetVersion ()
    {
    return DIRECTORY_VERSION;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   5/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SpatialIndexDir::Impl
    {
private:

    virtual const Extent3d64f&          _GetContentExtent                  () const = 0;

public:
    virtual                             ~Impl                              () = 0 {}

    const Extent3d64f&                  GetContentExtent                   () const {
        return _GetContentExtent();
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   5/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SpatialIndexDir::DefaultImpl : Impl
    {
private:
    virtual const Extent3d64f&          _GetContentExtent                  () const
        {
        return IDTM_DEFAULT.extent3d;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   5/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct SpatialIndexDir::TreeImpl : Impl
    {
private:
    typedef HTGFF::AttributeSubDir<Extent3d64f>
    ExtentsDir;

    const SpatialIndexDir&              m_rDir;
    ExtentsDir*                         m_pContentExtentsDir;


    virtual const Extent3d64f&          _GetContentExtent                  () const override
    {
        NodeID TopNodeID = GetNullNodeID();
        if (!m_rDir.AttributeMgr().Get(IDTM_ATTRIBUTEID_SPATIALINDEXDIR_TOPNODEID, TopNodeID))
    {
    HPOSTCONDITION(!"Attribute not found!");
        return IDTM_DEFAULT.extent3d;
        }

    if (TopNodeID == GetNullNodeID())
    return IDTM_DEFAULT.extent3d;

    return m_pContentExtentsDir->Get(TopNodeID);
    }

    explicit                            TreeImpl                           (const SpatialIndexDir&              pi_rDir,
                                                                            ExtentsDir*                         pi_pContentExtentDir)
            :   m_rDir(pi_rDir),
                m_pContentExtentsDir(pi_pContentExtentDir)
        {

        }

public:
    static TreeImpl*                    Create                             (const SpatialIndexDir&              pi_rDir)
        {
        static ExtentsDir::Options EXTENTS_DIR_OPTIONS(IDTM_DEFAULT.extent3d);
        ExtentsDir* pExtentsDir = pi_rDir.SubDirMgr<ExtentsDir>().Get(IDTM_DIRECTORYID_SPATIALINDEXDIR_NODES_CONTENT_3D_EXTENT_SUBDIR, 
                                                                      &EXTENTS_DIR_OPTIONS);

        if (0 == pExtentsDir)
            return 0;

        return new TreeImpl(pi_rDir, pExtentsDir);
        }

    };


const SpatialIndexDir::Impl& SpatialIndexDir::GetImpl () const
    {
    if (0 != m_pImpl.get())
        return *m_pImpl;

    switch (GetType())
        {
        case SPATIAL_INDEX_TYPE_QUAD_TREE:
        case SPATIAL_INDEX_TYPE_OCT_TREE:
            const_cast<SpatialIndexDir&>(*this).m_pImpl.reset(TreeImpl::Create(*this));
        }

    if (0 == m_pImpl.get())
        {
        static const DefaultImpl DEFAULT_IMPL;
        return DEFAULT_IMPL;
        }

    return *m_pImpl;
    }

SpatialIndexDir::Options::Options (SpatialIndexType pi_Type)
    :   m_Type(pi_Type)
    {

    }

SpatialIndexType SpatialIndexDir::Options::GetType () const
    {
    return m_Type;
    }



SpatialIndexDir::~SpatialIndexDir  ()
    {

    }


SpatialIndexDir::SpatialIndexDir ()
    :   Directory(DIRECTORY_VERSION)
    {

    }


bool SpatialIndexDir::_Create  (const CreateConfig&     pi_rCreateConfig,
                                const UserOptions*      pi_pUserOptions)
    {
    bool Success = true;

    HPRECONDITION(0 != pi_pUserOptions);

    const Options& rOptions = pi_pUserOptions->SafeReinterpretAs<Options>();

    Success &= SetType(rOptions.GetType());
    Success &= rOptions._SaveTo(*this);

    return Success;
    }

bool SpatialIndexDir::_Load   (const UserOptions*      pi_pUserOptions)
    {
    return true;
    }


bool SpatialIndexDir::_Save ()
    {
    return true;
    }


SpatialIndexType SpatialIndexDir::GetType () const
    {
    uint32_t Type = 0;
    if (!AttributeMgr().Get (IDTM_ATTRIBUTEID_SPATIALINDEXDIR_TYPE, Type))
        {
        HASSERT(!"Spatial index type was not set");
        }
    else if (Type >= SPATIAL_INDEX_TYPE_QTY)
        {
        HASSERT(!"Invalid spatial index type");
        Type = 0;
        }

    return static_cast<SpatialIndexType>(Type);
    }


const Extent3d64f& SpatialIndexDir::GetContentExtent () const
    {
    return GetImpl().GetContentExtent();
    }


bool SpatialIndexDir::SetType (SpatialIndexType pi_Type)
    {
    HASSERT(pi_Type < SPATIAL_INDEX_TYPE_QTY);
    return AttributeMgr().Set (IDTM_ATTRIBUTEID_SPATIALINDEXDIR_TYPE, static_cast<uint32_t>(pi_Type));
    }


} //End namespace IDTMFile