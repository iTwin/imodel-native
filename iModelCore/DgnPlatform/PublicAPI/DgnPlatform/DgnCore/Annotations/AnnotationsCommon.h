//-------------------------------------------------------------------------------------- 
//     $Source:  
//  $Copyright:  
//-------------------------------------------------------------------------------------- 
#pragma once

//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup Annotations
//! @beginGroup

//=======================================================================================
//! Describes how to interpret annotation color values.
// @bsiclass                                                    Jeff.Marker     05/2014
//=======================================================================================
enum class AnnotationColorType
{
    ByCategory = 1, //!< Ignore the stored ColorDef, and use the object's category color instead
    RGBA = 2, //!< Use the stored ColorDef
    ViewBackground = 3 //!< Ignore the stored ColorDef, and use the view's background color instead
};

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
