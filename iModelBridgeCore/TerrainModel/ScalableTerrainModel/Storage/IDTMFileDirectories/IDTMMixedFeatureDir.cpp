//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/IDTMMixedFeatureDir.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFileDirectories/IDTMMixedFeatureDir.h>
#include "../IDTMFileDefinition.h"


using namespace IDTMFile;



MixedFeatureDir*  MixedFeatureDir::Create ()
    {
    return new MixedFeatureDir;
    }

MixedFeatureDir::MixedFeatureDir ()
    {

    }


bool MixedFeatureDir::_CreateFeatureHeaderDir  (HeaderDir*&             po_rpHeaderDir,
                                                const CreateConfig&     pi_rCreateConfig,
                                                const UserOptions*      pi_pUserOptions)
    {
    return DefaultCreateFeatureHeaderDir(po_rpHeaderDir, pi_rCreateConfig);
    }

bool MixedFeatureDir::_LoadFeatureHeaderDir    (HeaderDir*&             po_rpHeaderDir,
                                                const UserOptions*      pi_pUserOptions)
    {
    return DefaultLoadFeatureHeaderDir(po_rpHeaderDir);
    }

