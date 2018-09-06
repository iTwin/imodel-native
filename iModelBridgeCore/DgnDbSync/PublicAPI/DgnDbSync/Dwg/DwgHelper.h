/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnDbSync/Dwg/DwgHelper.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/Render.h>
#include <DgnDbSync/Dwg/DwgDb/DwgDbCommon.h>
#include <DgnDbSync/Dwg/DwgDb/BasicTypes.h>
#include <DgnDbSync/Dwg/DwgDb/DwgResBuf.h>
#include <DgnDbSync/Dwg/DwgDb/DwgDrawables.h>
#include <DgnDbSync/DgnDbSync.h>

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

BEGIN_DGNDBSYNC_DWG_NAMESPACE

struct DwgHelper : NonCopyableClass
    {
    DGNDBSYNC_EXPORT static StandardUnit     GetStandardUnitFromDwgUnit (DwgDbUnits const& dwgUnit);
    DGNDBSYNC_EXPORT static StandardUnit     GetStandardUnitFromUnitName (Utf8StringCR stringIn);
    DGNDBSYNC_EXPORT static AnglePrecision   GetAngularUnits (AngleMode* angleMode, int16_t dwgAUPREC);
    DGNDBSYNC_EXPORT static DwgDbLineWeight  GetDwgLineWeightFromWeightName (Utf8StringCR stringIn);
    DGNDBSYNC_EXPORT static double           GetTransparencyFromDwg (DwgTransparencyCR dwgTransparency, DwgDbObjectIdCP layerId = nullptr, DwgDbObjectIdCP blockId = nullptr);
    DGNDBSYNC_EXPORT static DPoint3d         DefaultPlacementPoint (DwgDbEntityCR entity);
    DGNDBSYNC_EXPORT static RenderMode       GetRenderModeFromVisualStyle (DwgDbVisualStyleCR visualStyle);
    DGNDBSYNC_EXPORT static BentleyStatus    UpdateViewFlagsFromVisualStyle (ViewFlags& viewFlags, DwgDbObjectIdCR id);
    DGNDBSYNC_EXPORT static BentleyStatus    GetLayoutOrBlockName (Utf8StringR nameOut, DwgDbBlockTableRecordCR blockIn);
    DGNDBSYNC_EXPORT static void             ComputeMatrixFromArbitraryAxis (RotMatrixR matrix, DVec3dCR normal);
    DGNDBSYNC_EXPORT static void             ComputeMatrixFromXZ (RotMatrixR matrix, DVec3dCR xDirection, DVec3dCR normal);
    DGNDBSYNC_EXPORT static void             CreateArc2d (DEllipse3dR ellipse, DPoint3dCR start, DPoint3dCR end, double bulgeFactor);
    DGNDBSYNC_EXPORT static DPoint3d         ComputeBulgePoint (double bulgeFactor, DPoint3dCR start, DPoint3dCR end);
    DGNDBSYNC_EXPORT static void             SetViewFlags (ViewFlags& flags, bool grid, bool acs, bool background, bool transparent, bool clipFront, bool clipBack, DwgDbDatabaseCR dwg);
    DGNDBSYNC_EXPORT static Utf8CP           ToUtf8CP (DwgStringCR fromString, bool nullIfEmpty = false);
    DGNDBSYNC_EXPORT static void             ValidateStyleName (Utf8String& out, DwgStringCR in);
    DGNDBSYNC_EXPORT static size_t           ConvertEscapeCodes (TextStringR text, bvector<DSegment3d>* underlines, bvector<DSegment3d>* overlines);
    DGNDBSYNC_EXPORT static ColorDef         GetColorDefFromACI (int16_t acColorIndex);
    DGNDBSYNC_EXPORT static ColorDef         GetColorDefFromTrueColor (DwgCmEntityColorCR acColor);
    DGNDBSYNC_EXPORT static ColorDef         GetColorDefFromTrueColor (DwgCmColorCR acColor);
    DGNDBSYNC_EXPORT static void             GetDgnGradientColor (GradientSymbR gradientOut, DwgGiGradientFillCR gradientIn);
    DGNDBSYNC_EXPORT static void             SetGradientFrom (DwgGiGradientFillR gradientOut, DwgDbHatchCR hatchIn);
    DGNDBSYNC_EXPORT static Utf8String       GetAttrdefECClassNameFromBlockName (WCharCP blockName);
    DGNDBSYNC_EXPORT static DRange2d         GetRangeFrom (DPoint2dCR center, double width, double height);
    DGNDBSYNC_EXPORT static double           GetAbsolutePDSIZE (double pdsize, double vportHeight);
    DGNDBSYNC_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDbCircleCR circle, TransformCP transform = nullptr);
    DGNDBSYNC_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDbEllipseCR ellipse, TransformCP transform = nullptr);
    DGNDBSYNC_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDbFaceCR face, TransformCP transform = nullptr);
    DGNDBSYNC_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDbRegionCR region, TransformCP transform = nullptr);
    DGNDBSYNC_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDbPolylineCR polyline, TransformCP transform = nullptr);
    DGNDBSYNC_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDb2dPolylineCR polyline2d, TransformCP transform = nullptr);
    DGNDBSYNC_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDb3dPolylineCR polyline3d, TransformCP transform = nullptr);
    DGNDBSYNC_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDbSplineCR spline, TransformCP transform = nullptr, bool makeLinestring = false);
    DGNDBSYNC_EXPORT static CurveVectorPtr   CreateCurveVectorFrom (DwgDbObjectId entityId, TransformCP transform = nullptr);
    DGNDBSYNC_EXPORT static ClipVectorPtr    CreateClipperFromEntity (DwgDbObjectId entityId, double* frontClip = nullptr, double* backClip = nullptr, TransformCP entityToClipper = nullptr, TransformCP clipperToModel = nullptr);
    DGNDBSYNC_EXPORT static DwgFileVersion   CheckDwgVersionString (Utf8StringCR versionString);
    DGNDBSYNC_EXPORT static Utf8String       GetStringFromDwgVersion (DwgFileVersion dwgVersion);
    DGNDBSYNC_EXPORT static bool             SniffDwgFile (BeFileNameCR dwgName, DwgFileVersion* versionOut = nullptr);
    DGNDBSYNC_EXPORT static bool             SniffDxfFile (BeFileNameCR dxfName, DwgFileVersion* versionOut = nullptr);
    DGNDBSYNC_EXPORT static bool             CanOpenForWrite (BeFileNameCR path);
    DGNDBSYNC_EXPORT static uint32_t         GetDwgImporterVersion ();
    DGNDBSYNC_EXPORT static bool             GetTransformForSharedParts (TransformP out, double* uniformScale, TransformCR inTrans);
    };  // DwgHelper

END_DGNDBSYNC_DWG_NAMESPACE
