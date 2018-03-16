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
#include "ElementHandler.h"

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_ElementHandler {struct SavedSelection;}

//=======================================================================================
// @bsiclass                                                    Shaun.Sewall    05/17
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SavedSelection : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_SavedSelection, DefinitionElement);
    friend struct dgn_ElementHandler::SavedSelection;

protected:
    explicit SavedSelection(CreateParams const& params) : T_Super(params) {} 

public:
    BE_JSON_NAME(selectionData);

    //! Create a DgnCode for a SavedSelection given a name that is meant to be unique within the scope of the specified DefinitionModel
    static DgnCode CreateCode(DefinitionModelR scope, Utf8StringCR name) {return CodeSpec::CreateCode(BIS_CODESPEC_SavedSelection, scope, name);}

    //! Construct a new SavedSelection
    //! @param[in] model The DefinitionModel to contain the SavedSelection
    //! @param[in] selectionName The name of the SavedSelection.
    SavedSelection(DefinitionModelR model, Utf8StringCR selectionName)
        : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, selectionName))) {}

    //! Return the DgnClassId for the BisCore:SavedSelection ECClass
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SavedSelection));}

    //! Convenience method to get the ElementId for SavedSelection by name. If no match is found then an invalid Id is returned.
    static DgnElementId GetElementIdByName(DgnDbR db, Utf8CP name)
        {
        if (Utf8String::IsNullOrEmpty(name))
            return DgnElementId();

        DefinitionModelR dictionary = db.GetDictionaryModel();
        DgnCode code = SavedSelection::CreateCode(dictionary, name);

        return db.Elements().QueryElementIdByCode(code);
        }

    JsonValueCR GetSelectionData() const { return m_jsonProperties[json_selectionData()]; } //!< Returns the Json data that defines the Selection Elements.

    void SetSelectionData(Json::Value selectionData) //!< Sets the Json data that defines the Selection Elements.
        {
        if (selectionData.isNull() || selectionData.empty())
            RemoveJsonProperties(json_selectionData());

        SetJsonProperties(json_selectionData(), selectionData);
        }

    //! Convenience method to create a new SavedSelection element.
    //! @param[in] db The DgnDb to store the Saved Selection.
    //! @param[in] name The name will be used to form the element's DgnCode
    //! @param[in] selectionData The json value that holds the selection data to be saved in the element.
    static SavedSelectionPtr Create(DgnDbR db, Utf8CP name, Json::Value selectionData = Json::Value(Json::nullValue))
        {
        if (Utf8String::IsNullOrEmpty(name))
            return nullptr;

        DefinitionModelR dictionary = db.GetDictionaryModel();
        SavedSelectionPtr selection = new SavedSelection(dictionary, name);
        if (!selectionData.isNull() && !selectionData.empty())
            selection->SetSelectionData(selectionData);

        return selection;
        }

};

//=======================================================================================
//! @private
//=======================================================================================
namespace dgn_ElementHandler
{
    //! The ElementHandler for SavedSelection
    struct EXPORT_VTABLE_ATTRIBUTE SavedSelection : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_SavedSelection, Dgn::SavedSelection, SavedSelection, Definition, DGNPLATFORM_EXPORT)
    };
}

END_BENTLEY_DGN_NAMESPACE
