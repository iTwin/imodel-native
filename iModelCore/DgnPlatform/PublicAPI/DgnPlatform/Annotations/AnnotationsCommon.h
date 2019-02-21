//-------------------------------------------------------------------------------------- 
//     $Source: PublicAPI/DgnPlatform/Annotations/AnnotationsCommon.h $ 
//  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $ 
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Describes how to interpret annotation color values.
//! @ingroup GROUP_Annotation
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
enum class AnnotationColorType
{
    ByCategory = 1, //!< Ignore the stored ColorDef, and use the object's category color instead
    RGBA = 2, //!< Use the stored ColorDef
    ViewBackground = 3 //!< Ignore the stored ColorDef, and use the view's background color instead
};

END_BENTLEY_DGN_NAMESPACE
