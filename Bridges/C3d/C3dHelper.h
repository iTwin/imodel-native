/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/Render.h>

USING_NAMESPACE_BENTLEY_RENDER

BEGIN_C3D_NAMESPACE

//======================================================================================
//! A helper class of common utilities useful for C3D conversion
//===============+===============+===============+===============+===============+======
struct C3dHelper : BentleyApi::NonCopyableClass
    {
private:
    C3dHelper () {}
public:
    EXPORT_ATTRIBUTE static BentleyStatus   GetLinearCurves (CurveVectorR curves, GeometrySourceCP source);
    EXPORT_ATTRIBUTE static BentleyStatus   CopyGeometrySource (GeometricElement3dP target, GeometrySourceCP source);
    EXPORT_ATTRIBUTE static DgnElementId    GetCivilReferenceElementId (DwgSourceAspects::ObjectAspectCR aspect, DgnElementId* baseId = nullptr);
    EXPORT_ATTRIBUTE static DgnElementId    GetCivilReferenceElementId (DwgImporter::ElementImportResultsCR results, DgnElementId* baseId = nullptr);
    EXPORT_ATTRIBUTE static BentleyStatus   AddCivilReferenceElementId (DwgImporter::ElementImportResultsR results, DgnElementId id, DgnElementId baseId = DgnElementId());
    EXPORT_ATTRIBUTE static Utf8String      UpdateElementIdToJson (Utf8StringCR existingJson, DgnElementId elementId, DgnElementId baseId = DgnElementId());
    EXPORT_ATTRIBUTE static BentleyStatus   SetPropertiesInResults (DwgImporter::ElementImportResultsR results, ECN::IECInstanceCR ecInstance);
    };  // C3dHelper

END_C3D_NAMESPACE
