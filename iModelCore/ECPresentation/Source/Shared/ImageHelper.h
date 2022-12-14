/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct ImageHelper
{
public:
    static Utf8String GetImageId(ECClassCR ecClass, bool isInstanceImage, bool expanded);
    static Utf8String GetImageId(ECPropertyCR ecProperty, bool expanded);
    static Utf8String GetLabelGroupingNodeImageId(bool expanded);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
