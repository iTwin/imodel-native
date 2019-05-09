/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/Render.h>
#include <Dwg/DwgDb/DwgDbCommon.h>
#include <Dwg/DwgDb/BasicTypes.h>
#include <Dwg/DwgDb/DwgResBuf.h>
#include <Dwg/DwgDb/DwgDrawables.h>

#define SCHEMAName_AttributeDefinitions     "DwgAttributeDefinitions"
#define SCHEMALabel_AttributeDefinitions    "DWG Attribute Definitions"
#define SCHEMAAlias_AttributeDefinitions    "DwgAttdefs"
#define REALDWG_REGISTRY_ROOTKEY            L"REALDWG_REGISTRY_ROOTKEY"
#define ISVALID_Thickness(t)                (fabs(t) > 1.0e-8)
#define ISVALID_Distance(d)                 (fabs(d) > 1.0e-8)
#define GETTHICKNESS_From(en)               (en->GetThickness())
#define CHAR_LineFeed                       0x0A
#define CHAR_CarriageReturn                 0x0D

USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_DWGDB

BEGIN_DWG_NAMESPACE

//======================================================================================
// !A helper class of common utilities useful for DWG conversion
//===============+===============+===============+===============+===============+======
struct DwgHelper : BentleyApi::NonCopyableClass
    {
private:
    DwgHelper () {}
public:
    DWG_EXPORT static StandardUnit     GetStandardUnitFromDwgUnit (DwgDbUnits const& dwgUnit);
    DWG_EXPORT static StandardUnit     GetStandardUnitFromUnitName (Utf8StringCR stringIn);
    DWG_EXPORT static AnglePrecision   GetAngularUnits (AngleMode* angleMode, int16_t dwgAUPREC);
    DWG_EXPORT static DwgDbLineWeight  GetDwgLineWeightFromWeightName (Utf8StringCR stringIn);
    DWG_EXPORT static double           GetTransparencyFromDwg (DwgTransparencyCR dwgTransparency, DwgDbObjectIdCP layerId = nullptr, DwgDbObjectIdCP blockId = nullptr);
    DWG_EXPORT static DPoint3d         DefaultPlacementPoint (DwgDbEntityCR entity);
    DWG_EXPORT static RenderMode       GetRenderModeFromVisualStyle (DwgDbVisualStyleCR visualStyle);
    DWG_EXPORT static BentleyStatus    UpdateViewFlagsFromVisualStyle (ViewFlags& viewFlags, DwgDbObjectIdCR id);
    DWG_EXPORT static BentleyStatus    GetLayoutOrBlockName (Utf8StringR nameOut, DwgDbBlockTableRecordCR blockIn);
    DWG_EXPORT static void             ComputeMatrixFromArbitraryAxis (RotMatrixR matrix, DVec3dCR normal);
    DWG_EXPORT static void             ComputeMatrixFromXZ (RotMatrixR matrix, DVec3dCR xDirection, DVec3dCR normal);
    DWG_EXPORT static void             CreateArc2d (DEllipse3dR ellipse, DPoint3dCR start, DPoint3dCR end, double bulgeFactor);
    DWG_EXPORT static DPoint3d         ComputeBulgePoint (double bulgeFactor, DPoint3dCR start, DPoint3dCR end);
    DWG_EXPORT static void             SetViewFlags (ViewFlags& flags, bool grid, bool acs, bool background, bool transparent, bool clipFront, bool clipBack, DwgDbDatabaseCR dwg);
    DWG_EXPORT static Utf8CP           ToUtf8CP (DwgStringCR fromString, bool nullIfEmpty = false);
    DWG_EXPORT static void             ValidateStyleName (Utf8String& out, DwgStringCR in);
    DWG_EXPORT static size_t           ConvertEscapeCodes (TextStringR text, bvector<DSegment3d>* underlines, bvector<DSegment3d>* overlines);
    DWG_EXPORT static BentleyStatus    CreateUnderOrOverline (TextStringCR text, DSegment3dR line, bool underOrOver);
    DWG_EXPORT static BentleyStatus    ResetPositionForBackwardAndUpsideDown (Dgn::TextStringR dgnText, bool backward, bool upsidedown);
    DWG_EXPORT static DgnFontType      GetFontType (DwgFontInfoCR dwgFont);
    DWG_EXPORT static ColorDef         GetColorDefFromACI (int16_t acColorIndex);
    DWG_EXPORT static ColorDef         GetColorDefFromTrueColor (DwgCmEntityColorCR acColor);
    DWG_EXPORT static ColorDef         GetColorDefFromTrueColor (DwgCmColorCR acColor);
    DWG_EXPORT static void             GetDgnGradientColor (GradientSymbR gradientOut, DwgGiGradientFillCR gradientIn);
    DWG_EXPORT static void             SetGradientFrom (DwgGiGradientFillR gradientOut, DwgDbHatchCR hatchIn);
    DWG_EXPORT static Utf8String       GetAttrdefECSchemaName (DwgDbDatabaseCP dwg);
    DWG_EXPORT static Utf8String       GetAttrdefECClassNameFromBlockName (WCharCP blockName);
    DWG_EXPORT static DRange2d         GetRangeFrom (DPoint2dCR center, double width, double height);
    DWG_EXPORT static double           GetAbsolutePDSIZE (double pdsize, double vportHeight);
    DWG_EXPORT static ICurvePrimitivePtr CreateCurvePrimitive (DwgDbLineCR arc, TransformCP transform = nullptr);
    DWG_EXPORT static ICurvePrimitivePtr CreateCurvePrimitive (DwgDbArcCR arc, TransformCP transform = nullptr);
    DWG_EXPORT static ICurvePrimitivePtr CreateCurvePrimitive (DwgDbCircleCR circle, TransformCP transform = nullptr);
    DWG_EXPORT static ICurvePrimitivePtr CreateCurvePrimitive (DwgDbEllipseCR ellipse, TransformCP transform = nullptr);
    DWG_EXPORT static ICurvePrimitivePtr CreateCurvePrimitive (DwgDbFaceCR face, TransformCP transform = nullptr);
    DWG_EXPORT static ICurvePrimitivePtr CreateCurvePrimitive (DwgDbSplineCR spline, TransformCP transform = nullptr, bool makeLinestring = false);
    DWG_EXPORT static ICurvePrimitivePtr CreateCurvePrimitive (DwgDbEntityCR entity, TransformCP transform = nullptr);
    DWG_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDbCircleCR circle, CurveVector::BoundaryType type = CurveVector::BOUNDARY_TYPE_Outer, TransformCP transform = nullptr);
    DWG_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDbEllipseCR ellipse, CurveVector::BoundaryType type = CurveVector::BOUNDARY_TYPE_Outer, TransformCP transform = nullptr);
    DWG_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDbFaceCR face, CurveVector::BoundaryType type = CurveVector::BOUNDARY_TYPE_Outer, TransformCP transform = nullptr);
    DWG_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDbRegionCR region, CurveVector::BoundaryType type = CurveVector::BOUNDARY_TYPE_Outer, TransformCP transform = nullptr);
    DWG_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDbPolylineCR polyline, CurveVector::BoundaryType type = CurveVector::BOUNDARY_TYPE_Outer, TransformCP transform = nullptr);
    DWG_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDb2dPolylineCR polyline2d, CurveVector::BoundaryType type = CurveVector::BOUNDARY_TYPE_Outer, TransformCP transform = nullptr);
    DWG_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDb3dPolylineCR polyline3d, CurveVector::BoundaryType type = CurveVector::BOUNDARY_TYPE_Outer, TransformCP transform = nullptr);
    DWG_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDbSplineCR spline, CurveVector::BoundaryType type = CurveVector::BOUNDARY_TYPE_Outer, TransformCP transform = nullptr, bool makeLinestring = false);
    DWG_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDbEntityCR entity, CurveVector::BoundaryType type = CurveVector::BOUNDARY_TYPE_Outer, TransformCP transform = nullptr);
    DWG_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDbObjectId entityId, CurveVector::BoundaryType type = CurveVector::BOUNDARY_TYPE_Outer, TransformCP transform = nullptr);
    DWG_EXPORT static ClipVectorPtr    CreateClipperFromEntity (DwgDbObjectId entityId, double* frontClip = nullptr, double* backClip = nullptr, TransformCP entityToClipper = nullptr, TransformCP clipperToModel = nullptr);
    DWG_EXPORT static DwgFileVersion   CheckDwgVersionString (Utf8StringCR versionString);
    DWG_EXPORT static Utf8String       GetStringFromDwgVersion (DwgFileVersion dwgVersion);
    DWG_EXPORT static bool             SniffDwgFile (BeFileNameCR dwgName, DwgFileVersion* versionOut = nullptr);
    DWG_EXPORT static bool             SniffDxfFile (BeFileNameCR dxfName, DwgFileVersion* versionOut = nullptr);
    DWG_EXPORT static bool             CanOpenForWrite (BeFileNameCR path);
    DWG_EXPORT static uint32_t         GetDwgImporterVersion ();
    DWG_EXPORT static BentleyStatus    GetImporterModuleVersion (Utf8StringR versionString);
    DWG_EXPORT static bool             GetTransformForSharedParts (TransformP out, double* uniformScale, TransformCR inTrans);
    DWG_EXPORT static bool             NegateScaleForSharedParts (double& partScale, TransformCR blockTransform);
    DWG_EXPORT static Utf8String       CompareSubcatAppearance (DgnSubCategory::Appearance const& a1, DgnSubCategory::Appearance const& a2);
    DWG_EXPORT static rapidjson::Value GetJsonFromDoubleArray (double const* array, size_t count, rapidjson::MemoryPoolAllocator<>& allocator);
    DWG_EXPORT static void             GetDoubleArrayFromJson (double* array, size_t count, rapidjson::Value const& jsonValue);
    DWG_EXPORT static bool             IsElementOwnedByJobSubject (DgnDbCR db, DgnElementId checkId, DgnElementId jobSubjectId);
    };  // DwgHelper

END_DWG_NAMESPACE
