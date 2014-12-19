//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/HTiff/src/IDTMFileDirectories/FeatureHeaderTypes.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>

#include <Imagepp/all/h/IDTMFileDirectories/FeatureHeaderTypes.h>
#include <Imagepp/all/h/IDTMTypes.h>


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
