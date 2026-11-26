/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "../DgnDb.h"

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! Converts the deprecated BisCore:TextAnnotationData aspect of an element into a self-contained JSON object.
//! Every AnnotationTextStyle, AnnotationFrameStyle, and AnnotationLeaderStyle referenced by the legacy annotation is resolved
//! into effective property values, and fonts are resolved to their names, so the output contains no element or font IDs.
//! Content that could not be resolved (e.g. a deleted style or an unknown font) is described in the "issues" array.
//! The JSON layout is documented by the LegacyTextAnnotationJson TypeScript interface in @bentley/imodeljs-native.
//! @ingroup GROUP_Annotation
// @bsiclass
//=======================================================================================
struct LegacyTextAnnotationJson
{
    //! Version of the JSON layout written by Read.
    static constexpr int32_t CURRENT_VERSION = 1;

    //! Read the legacy text annotation of an element and write its JSON representation to @p out.
    //! @return DgnDbStatus::Success when @p out was populated; DgnDbStatus::NotFound when the element has no legacy text annotation (no aspect, or a NULL or empty blob);
    //! DgnDbStatus::VersionTooNew when the blob was written by a newer version; otherwise an error status (e.g. a corrupt blob), with a description in @p errorMessage.
    DGNPLATFORM_EXPORT static DgnDbStatus Read(BeJsValue out, DgnDbR db, DgnElementId elementId, Utf8StringP errorMessage = nullptr);
};

END_BENTLEY_DGN_NAMESPACE
