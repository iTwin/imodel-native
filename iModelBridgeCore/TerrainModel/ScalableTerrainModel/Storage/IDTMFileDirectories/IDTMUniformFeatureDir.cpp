//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/IDTMUniformFeatureDir.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFileDirectories/IDTMUniformFeatureDir.h>
#include <STMInternal/Storage/HTGFFAttributeManager.h>

#include "../IDTMFileDefinition.h"


using namespace IDTMFile;


UniformFeatureDir::Options::Options (bool pi_PointOnly)
    :   m_PointOnly(pi_PointOnly)
    {

    }

bool UniformFeatureDir::Options::IsPointOnly () const
    {
    return m_PointOnly;
    }

UniformFeatureDir* UniformFeatureDir::Create ()
    {
    return new UniformFeatureDir;
    }

UniformFeatureDir::UniformFeatureDir ()
    {

    }

PointDir* UniformFeatureDir::GetPointDir () const
    {
    if (AttributeMgr().IsPresent(IDTM_DIRECTORYID_FEATUREDIR_HEADER_SUBDIR))
        {
        HASSERT(!"Will not give direct access to point dir when not a point only feature dir.");
        return 0;
        }

    return FeatureDir::GetPointDirP();
    }

bool UniformFeatureDir::_CreateFeatureHeaderDir    (HeaderDir*&         po_rpHeaderDir,
                                                    const CreateConfig& pi_rCreateConfig,
                                                    const UserOptions*  pi_pUserOptions)
    {
    HPRECONDITION(0 != pi_pUserOptions);

    const bool DirContainOnlyPoints = pi_pUserOptions->SafeReinterpretAs<Options>().IsPointOnly();

    if (DirContainOnlyPoints)
        return CreatePointOnlyFeatureHeaderDir(po_rpHeaderDir, GetFeatureType(), pi_rCreateConfig);

    return DefaultCreateFeatureHeaderDir(po_rpHeaderDir, pi_rCreateConfig);
    }

bool UniformFeatureDir::_LoadFeatureHeaderDir      (HeaderDir*&         po_rpHeaderDir,
                                                    const UserOptions*  pi_pUserOptions)
    {
    const bool DirContainOnlyPoints = !AttributeMgr().IsPresent(IDTM_DIRECTORYID_FEATUREDIR_HEADER_SUBDIR);

    if (DirContainOnlyPoints)
        {
        return LoadPointOnlyFeatureHeaderDir(po_rpHeaderDir, GetFeatureType());
        }

    return DefaultLoadFeatureHeaderDir(po_rpHeaderDir);
    }
