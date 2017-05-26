/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECQuantityFormatting.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECObjects/ECInstance.h>
#include <ECObjects/ECObjects.h>
#include <ECObjects/CalculatedProperty.h>
#include <ECObjects/SchemaLocalizedStrings.h>
#include <Bentley/RefCounted.h>
#include <Bentley/bvector.h>
#include <Bentley/bmap.h>
#include <Bentley/BeFileName.h>
#include <Formatting/FormattingApi.h>

namespace BEU = BentleyApi::Units;
BEGIN_BENTLEY_ECOBJECT_NAMESPACE

struct ECQuantityFormatting
    {
    ECOBJECTS_EXPORT  static Utf8String StdFormatQuantity(BEU::QuantityCR qty, KindOfQuantityCP koq, Utf8String defFormatName = "real4");
    };

END_BENTLEY_ECOBJECT_NAMESPACE