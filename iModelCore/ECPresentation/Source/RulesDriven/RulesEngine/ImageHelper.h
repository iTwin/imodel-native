/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/ImageHelper.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <ECPresentation/ECPresentationTypes.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                01/2016
+===============+===============+===============+===============+===============+======*/
struct ImageHelper
{
public:
    static Utf8String GetImageId(ECClassCR ecClass, bool isInstanceImage, bool expanded);
    static Utf8String GetImageId(ECPropertyCR ecProperty, bool expanded);
    static Utf8String GetLabelGroupingNodeImageId(bool expanded);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
