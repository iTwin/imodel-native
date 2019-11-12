/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "DgnV8OpenRoadsDesigner.h"

BEGIN_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE

//=======================================================================================
//! The DgnDomain for the DgnV8OpenRoadsDesigner schema.
//! @ingroup GROUP_DgnV8OpenRoadsDesigner
//=======================================================================================
struct DgnV8OpenRoadsDesignerDomain : Dgn::DgnDomain
{
DOMAIN_DECLARE_MEMBERS(DgnV8OpenRoadsDesignerDomain, DGNV8OPENROADSDESIGNER_EXPORT)

public:
    //! @private
    DgnV8OpenRoadsDesignerDomain();

    //! @private
    DGNV8OPENROADSDESIGNER_EXPORT static Dgn::DgnDbStatus SetGeometricElementAsBoundingContentForSheet(Dgn::GeometricElementCR boundingElm, Dgn::Sheet::ElementCR sheet);

    //! Get the DgnElementIdSet containing the DgnElementIds of all of the NamedBoundaries that are the bounding elements for drawings in SheetModels
    //! @param[in] dgnDb The DgnDb to search for bounding elements
    //! @return DgnElementSet containing boundary DgnElementIds
    DGNV8OPENROADSDESIGNER_EXPORT static Dgn::DgnElementIdSet QueryElementIdsBoundingContentForSheets(Dgn::DgnDbCR dgnDb);

    //! Get the DgnElementIdSet of any SheetModles that are bounded by the \p boundingElm.  
    //! @param[in] A geometric element that bounds a SheetModel.  This can be obtained by getting the GeometricElement from the DgnElementIds returned by QueryElementIdsBoundingContentForSheets()
    //! @return ElementIdSet containing all of the sheets that are physically located within the area defined by \p boundingElm
    DGNV8OPENROADSDESIGNER_EXPORT static Dgn::DgnElementIdSet QuerySheetIdsBoundedBy(Dgn::GeometricElementCR boundingElm);

private:
    WCharCP _GetSchemaRelativePath() const override { return V8ORD_SCHEMA_PATH; }
}; // DgnV8OpenRoadsDesignerDomain

END_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE
