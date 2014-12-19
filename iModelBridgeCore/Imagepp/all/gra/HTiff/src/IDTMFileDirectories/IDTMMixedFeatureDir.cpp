//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/HTiff/src/IDTMFileDirectories/IDTMMixedFeatureDir.cpp $
//:>
//:>  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------


#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/IDTMFileDirectories/IDTMMixedFeatureDir.h>
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

