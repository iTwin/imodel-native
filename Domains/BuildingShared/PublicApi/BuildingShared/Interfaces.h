/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatformApi.h>
#include "BuildingSharedMacros.h"

BEGIN_BUILDING_SHARED_NAMESPACE


#define BCSSERIALIZABLE_ELEMENT_ClassName                   "ClassName"
#define BCSSERIALIZABLE_ELEMENT_SchemaName                  "SchemaName"
#define BCSSERIALIZABLE_ELEMENT_ElementId                   "ElementId"
#define BCSSERIALIZABLE_ELEMENT_Name                        "Name"
#define BCSSERIALIZABLE_ELEMENT_CodeValue                   "CodeValue"


#define BCSJSONACTIONPERFORMER_ACTION                       "Action"
#define BCSJSONACTIONPERFORMER_ACTIONPARAMS                 "Parameters"

#define BCSJSONACTIONPERFORMER_ACTION_UPDATE                "Update"
#define BCSJSONACTIONPERFORMER_ACTION_DELETE                "Delete"
#define BCSJSONACTIONPERFORMER_ACTION_COPYBUILDINGTYPE      "CopyBuildingType"
#define BCSJSONACTIONPERFORMER_ACTION_COPYSPACETYPE         "CopySpaceType"

#define BCSFLOORMANAGER_ACTION_ADDFLOOR                     "AddFloor"

#define BCSBUILDINGTYPEMANAGER_ACTION_ADDBUILDINGTYPE       "AddBuildingType"
#define BCSBUILDINGTYPEMANAGER_ACTION_ADDSPACETYPE          "AddSpaceType"


//=======================================================================================
// @bsiclass                                     Jonas.Valiunas                 05/2018
//=======================================================================================
struct IBCSJsonActionPerformer
    {
    protected:
    
        //! performs action based on the json message
        //! @param[in]  actionData JSON object that holds serialized action data
        virtual void _PerformJsonAction(Json::Value const& actionData) = 0;

    public:
        //! performs action based on the json message
        //! @param[in]  actionData JSON object that holds serialized action data
        void PerformJsonAction(Json::Value const& actionData) { _PerformJsonAction(actionData); };
    };

//=======================================================================================
// @bsiclass                                     Jonas.Valiunas                 04/2018
//=======================================================================================
struct IBCSSerializable
    {
    protected:
        //! Serializes element data to a JSON object
        //! @param[in,out]  elementData JSON object that will hold serialized data
        virtual void _SerializeProperties(Json::Value& elementData) const = 0;

        //! Formats serialized element data in a JSON object
        //! @param[in, out]  elementData JSON object that holds serialized data
        virtual void _FormatSerializedProperties(Json::Value& elementData) const = 0;

    public:
        //! Serializes element data to a JSON object
        //! @param[in,out]  elementData JSON object that will hold serialized data
        void SerializeProperties(Json::Value& elementData) const { _SerializeProperties(elementData);};

        //! Formats serialized element data in a JSON object
        //! @param[in, out]  elementData JSON object that holds serialized data
        void FormatSerializedProperties(Json::Value& elementData) const { _FormatSerializedProperties(elementData);};
    };

END_BUILDING_SHARED_NAMESPACE