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
private:
    static bool GetImageId(Utf8StringR imageId, ECN::IECCustomAttributeContainerCR container, bool expanded);
public:
    static Utf8String GetImageId(ECN::ECClassCR ecClass, bool isInstanceImage, bool expanded);
    static Utf8String GetImageId(ECN::ECPropertyCR ecProperty, bool expanded);
    static Utf8String GetLabelGroupingNodeImageId(bool expanded);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
