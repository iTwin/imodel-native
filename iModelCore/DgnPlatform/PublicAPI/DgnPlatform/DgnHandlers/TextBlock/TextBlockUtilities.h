/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/TextBlock/TextBlockUtilities.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__BENTLEY_INTERNAL_ONLY__*/

#include <DgnPlatform/DgnHandlers/TextBlock/TextBlockAPI.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Many of these methods are here and not on TextBlock to further discourage their use. Internal and low-level APIs want re-usable functionality, but you should NEVER consider these for general use.
// @bsiclass                                                    Jeff.Marker     03/08
//=======================================================================================
class TextBlockUtilities
    {
    public:     DGNPLATFORM_EXPORT  static  DRange3d                ComputeJustificationRange   (DRange3dCR nominalRange, DRange3dCR exactRange);
#if defined (NEEDS_WORK_DGNITEM)
    public:     DGNPLATFORM_EXPORT  static  void                    AppendEdfs                  (TextBlockR, WStringCR fullString, TextEDParamCR);
    public:     DGNPLATFORM_EXPORT  static  void                    AppendEdfs                  (TextBlockR, WStringCR fullString, EDFieldVectorCR);
#endif
    public:                         static  DgnGlyphRunLayoutFlags  ComputeRunLayoutFlags       (TextParamWideCR, DPoint2dCR fontSize);

    }; // TextUtilities

END_BENTLEY_DGNPLATFORM_NAMESPACE
