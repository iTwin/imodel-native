//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/Storage/IDTMFileDirectories/FeatureHeaderTypes.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ScalableTerrainModelPCH.h>


#include <STMInternal/Storage/IDTMFileDirectories/FeatureHeaderTypes.h>
#include <STMInternal/Storage/IDTMTypes.h>


namespace IDTMFile {

/*---------------------------------------------------------------------------------**//**
* Stored types integrity checks
+---------------+---------------+---------------+---------------+---------------+------*/
HSTATICASSERT(4*sizeof(uint32_t) == sizeof(FeatureHeader));


FeatureHeader::group_id_type FeatureHeader::GetNullID ()
    {
    return IDTrait<group_id_type>::GetNullID ();
    }


} //End namespace IDTMFile
