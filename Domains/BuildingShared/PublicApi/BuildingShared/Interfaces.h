/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/BuildingShared/Interfaces.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatformApi.h>
#include "BuildingSharedMacros.h"

BEGIN_BUILDING_SHARED_NAMESPACE


#define BCSSERIALIZABLE_ELEMENT_ClassName                   "ClassName"
#define BCSSERIALIZABLE_ELEMENT_ElementId                   "AllocationId"
#define BCSSERIALIZABLE_ELEMENT_Name                        "Name"
#define BCSSERIALIZABLE_ELEMENT_CodeValue                   "CodeValue"


//=======================================================================================
// @bsiclass                                     Jonas.Valiunas                 04/2018
//=======================================================================================
struct IBCSSerializable
    {
    public:
        //! Serializes element data to a JSON object
        //! @param[in,out]  elementData JSON object that will hold serialized data
        virtual void SerializeProperties(Json::Value& elementData) const = 0;
        
        //! Updates element from Json properties
        //! @param[in]  elementData JSON object that holds serialized data
        virtual void UpdateFromJson(Json::Value const& elementData) = 0;

        //! Formats serialized element data in a JSON object
        //! @param[in, out]  elementData JSON object that holds serialized data
        virtual void FormatSerializedProperties(Json::Value& elementData) const = 0;
    };

END_BUILDING_SHARED_NAMESPACE