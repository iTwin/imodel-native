/*--------------------------------------------------------------------------------------+
| 
|  $Source: PublicAPI/DgnPlatform/SavedSelection.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include "DgnElement.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// This class is used to store the specification for a set elements. The set is defined
// in JSON format created by the SelectionManager in ECPresentation layer.
// @bsiclass                                     		Bill.Steinbock  03/2018
//=======================================================================================
struct SavedSelection
{
public:
    BE_JSON_NAME(selectionData);

    //! Create a DgnCode for a SavedSelection given a name that is meant to be unique within the scope of the iModel
    static DgnCode CreateCode(DgnDbR dgndb, Utf8StringCR name) {return CodeSpec::CreateCode(dgndb, BIS_CODESPEC_SavedSelection, name);}

    //! Return the DgnClassId for the BisCore:SavedSelection ECClass
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SavedSelection));}

    //! Convenience method to get the ElementId for SavedSelection by name. If no match is found then an invalid Id is returned.
    static DgnElementId GetElementIdByName(DgnDbR db, Utf8CP name)
        {
        if (Utf8String::IsNullOrEmpty(name))
            return DgnElementId();

        DgnCode code = SavedSelection::CreateCode(db, name);

        return db.Elements().QueryElementIdByCode(code);
        }

    //! Returns the Json data that defines the Selection Elements.
    static JsonValueCR GetSelectionData(DefinitionElementCR selection) 
        { 
        return selection.GetJsonProperties(json_selectionData()); 
        } 

    //! Sets the Json data that defines the Selection Elements.
    static void SetSelectionData(DefinitionElementR selection, Json::Value selectionData) 
        {
        // empty() checks for a empty arrayValue and null objectValues
        if (selectionData.empty())
            selection.RemoveJsonProperties(json_selectionData());
        else
            selection.SetJsonProperties(json_selectionData(), selectionData);
        }

    //! Convenience method to create a new SavedSelection element.
    //! @param[in] model The model to store the Saved Selection.
    //! @param[in] name The name will be used to form the element's DgnCode
    //! @param[in] selectionData The json value that holds the selection data to be saved in the element.
    static DefinitionElementPtr Create(DefinitionModelR model, Utf8CP name, Json::Value selectionData = Json::Value(Json::nullValue))
        {
        if (Utf8String::IsNullOrEmpty(name))
            return nullptr;

        DefinitionElementPtr selection = new DefinitionElement(DgnElement::CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model.GetDgnDb(), name)));
        if (!selectionData.empty())
            SavedSelection::SetSelectionData(*selection, selectionData);

        return selection;
        }
};


END_BENTLEY_DGN_NAMESPACE
