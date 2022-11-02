/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "DgnDb.h"
#include "DgnElement.h"
#include "ElementHandler.h"

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_ElementHandler {struct ColorBook;}

//=======================================================================================
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ColorBook : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_ColorBook, DefinitionElement);
    friend struct dgn_ElementHandler::ColorBook;

protected:
    BeJsConst GetColorBookColors() const {return GetJsonProperties(json_colorBookColors());}
    BeJsValue GetColorBookColorsR() {return GetJsonPropertiesR(json_colorBookColors());}

    explicit ColorBook(CreateParams const& params) : T_Super(params) {}

public:
    BE_PROP_NAME(Description);
    BE_JSON_NAME(colorBookColors);

    //! Create a DgnCode for a ColorBook given a name that is meant to be unique within the scope of the specified DefinitionModel
    static DgnCode CreateCode(DefinitionModelR scope, Utf8StringCR name) {return CodeSpec::CreateCode(BIS_CODESPEC_ColorBook, scope, name);}

    //! Construct a new ColorBook
    //! @param[in] model The DefinitionModel to contain the ColorBook
    //! @param[in] colorBookName The name of the ColorBook.
    //! @param[in] description The optional description of the ColorBook
    ColorBook(DefinitionModelR model, Utf8StringCR colorBookName, Utf8StringCR description="")
        : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, colorBookName))) {if (!description.empty()) SetDescription(description);}

    //! Return the DgnClassId for the BisCore:ColorBook ECClass
    static DgnClassId QueryClassId(DgnDbR db) {return DgnClassId(db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_ColorBook));}

    //! Add a named color into this ColorBook
    void AddColor(Utf8CP colorName, ColorDefCR color) {GetColorBookColorsR()[colorName] = color.GetValue();}

    //! Remove a color from this ColorBook
    void RemoveColor(Utf8CP colorName) {GetColorBookColorsR().removeMember(colorName);}

    //! Return the ColorDef of the specified name from this ColorBook
    ColorDef GetColor(Utf8CP colorName) {return ColorDef(GetColorBookColors()[colorName].asUInt(0));}

    //! Get the description of this ColorBook
    Utf8String GetDescription() const {return GetPropertyValueString(prop_Description());}
    //! Set the description for this ColorBook
    DgnDbStatus SetDescription(Utf8StringCR description) {return SetPropertyValue(prop_Description(), description.c_str());}
};

//=======================================================================================
//! @private
//=======================================================================================
namespace dgn_ElementHandler
{
    //! The ElementHandler for ColorBook
    struct EXPORT_VTABLE_ATTRIBUTE ColorBook : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_ColorBook, Dgn::ColorBook, ColorBook, Definition, DGNPLATFORM_EXPORT)
    };
}

END_BENTLEY_DGN_NAMESPACE
