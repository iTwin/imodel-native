/*--------------------------------------------------------------------------------------+
|     $Source: PublicAPI/DgnPlatform/DgnCore/TextStyleInterop.h $
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__

#include "DgnPlatform/DgnPlatform.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//=======================================================================================
// @bsiclass                                                    Jeff.Marker     01/2015
//=======================================================================================
struct TextStyleInterop : public NonCopyableClass
{
private:
    TextStyleInterop() {}

public:    
    DGNPLATFORM_EXPORT static BentleyStatus AnnotationToTextString(TextStringStyleR, AnnotationTextStyleCR);
    DGNPLATFORM_EXPORT static BentleyStatus TextStringToAnnotation(DgnDbR, AnnotationTextStyleR, TextStringStyleR);

}; // TextStyleInterop

//! @endGroup

END_BENTLEY_DGNPLATFORM_NAMESPACE
