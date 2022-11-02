/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once


#include "DgnPlatform/DgnPlatform.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//! @addtogroup TextModule
//! @beginGroup

//=======================================================================================
// @bsiclass
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

END_BENTLEY_DGN_NAMESPACE
