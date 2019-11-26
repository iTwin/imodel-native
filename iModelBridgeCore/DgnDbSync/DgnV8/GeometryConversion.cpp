/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <DgnPlatform/Annotations/TextAnnotationElement.h>
#include <DgnPlatform/AutoRestore.h>
#include <GeomSerialization/GeomSerializationApi.h>


DGNV8_BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
/*=================================================================================**//**
* PSolidCoreAPI can't be included, we don't have the parasolid headers (vendor api)...
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct SolidUtil
{
static BentleyStatus DisjoinBody (bvector<ISolidKernelEntityPtr>& out, ISolidKernelEntityCR in);
};

/*=================================================================================**//**
* PSolidCoreAPI can't be included, we don't have the parasolid headers (vendor api)...
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PSolidUtil
{
static CurveVectorPtr PlanarSheetBodyToCurveVector(ISolidKernelEntityCR in);
static CurveVectorPtr WireBodyToCurveVector(ISolidKernelEntityCR in);
static int GetEntityTag (ISolidKernelEntityCR entity, bool* isOwned = nullptr);
static BentleyStatus TransformBody (int body, TransformCR transform);
static size_t DebugMemoryUsage(ISolidKernelEntityCR in);
}; // PSolidUtil
DGNV8_END_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (BENTLEYCONFIG_PARASOLID)
#include <BRepCore/PSolidUtil.h>
#endif

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

static BentleyApi::DPoint2d const* DoInterop(Bentley::DPoint2d const*source) { return (BentleyApi::DPoint2d const* )source; }
static BentleyApi::DPoint2dCR DoInterop(Bentley::DPoint2d const&source) { return (BentleyApi::DPoint2dCR)source; }

static BentleyApi::DPoint3d const* DoInterop(Bentley::DPoint3d const*source) { return (BentleyApi::DPoint3d const* )source; }
static BentleyApi::DPoint3dR DoInterop(Bentley::DPoint3d&source) { return (BentleyApi::DPoint3dR)source; }
static BentleyApi::DPoint3dCR DoInterop(Bentley::DPoint3d const&source) { return (BentleyApi::DPoint3dCR)source; }
static Bentley::DPoint3dR DoInterop(BentleyApi::DPoint3d&source) { return (Bentley::DPoint3dR)source; }

static BentleyApi::DVec3d const* DoInterop(Bentley::DVec3d const*source) { return (BentleyApi::DVec3d const* )source; }
static BentleyApi::DVec3dR DoInterop(Bentley::DVec3d&source) { return (BentleyApi::DVec3dR)source; }

static BentleyApi::RotMatrixCR DoInterop(Bentley::RotMatrix const&source) { return (BentleyApi::RotMatrixCR)source; }
static Bentley::RotMatrixR DoInterop(BentleyApi::RotMatrix&source) { return (Bentley::RotMatrixR)source; }

static BentleyApi::TransformR DoInterop(Bentley::Transform&source) { return (BentleyApi::TransformR)source; }
static BentleyApi::TransformCR DoInterop(Bentley::Transform const&source) { return (BentleyApi::TransformCR)source; }
static Bentley::TransformCR DoInterop(BentleyApi::Transform const&source) { return (Bentley::TransformCR)source; }



//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
void Converter::ConvertLineStyleParams(Render::LineStyleParams& lsParams, DgnV8Api::LineStyleParams const* v8lsParams, double uorPerMeter, double componentScale, double modelLsScale)
    {
    if (nullptr == v8lsParams)
        {
        lsParams.Init();
        return;
        }

    if (uorPerMeter == 0.0)
        {
        BeAssert(uorPerMeter != 0.0);
        uorPerMeter = 1;
        }

    lsParams.modifiers = v8lsParams->modifiers; //   & ~STYLEMOD_TRUE_WIDTH;
    lsParams.reserved = 0;
    lsParams.scale = v8lsParams->scale;
    lsParams.dashScale = v8lsParams->dashScale;
    lsParams.gapScale= v8lsParams->gapScale;
    lsParams.fractPhase = v8lsParams->fractPhase;

    if (1.0 != modelLsScale)
        {
        if (lsParams.scale)
            lsParams.scale *= modelLsScale;
        else
            lsParams.scale = modelLsScale;

        lsParams.modifiers |= STYLEMOD_SCALE;
        }

    lsParams.distPhase = v8lsParams->distPhase;

    if (0 != (v8lsParams->modifiers & STYLEMOD_TRUE_WIDTH))
        {
        //  The value was in UORs.  In DgnDb it is in meters
        lsParams.startWidth = v8lsParams->startWidth/uorPerMeter;
        lsParams.endWidth = v8lsParams->endWidth/uorPerMeter;
        }
    else
        {
        //  The value is in line style units. Convert it to the new line style units.
        lsParams.startWidth = v8lsParams->startWidth*componentScale;
        lsParams.endWidth = v8lsParams->endWidth*componentScale;
        }

    lsParams.normal = DoInterop(v8lsParams->normal);
    lsParams.rMatrix = DoInterop(v8lsParams->rMatrix);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertCurveVector(BentleyApi::CurveVectorPtr& clone, Bentley::CurveVectorCR v8Curves, TransformCP v8ToDgnDbTrans)
    {
    clone = BentleyApi::CurveVector::Create((BentleyApi::CurveVector::BoundaryType) v8Curves.GetBoundaryType());

    for (Bentley::ICurvePrimitivePtr v8Curve : v8Curves)
        {
        if (!v8Curve.IsValid())
            continue;

        switch (v8Curve->GetCurvePrimitiveType())
            {
            case Bentley::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                {
                BentleyApi::DSegment3dCP segment = (BentleyApi::DSegment3dCP) v8Curve->GetLineCP();

                clone->push_back(BentleyApi::ICurvePrimitive::CreateLine(*segment));
                break;
                }

            case Bentley::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                {
                Bentley::bvector<Bentley::DPoint3d> const* v8Points = v8Curve->GetLineStringCP();
                bvector<DPoint3d> points;

                for (auto const& v8 : *v8Points)
                    {
                    // NOTE: Disconnects should only appear in linestrings, change to boundary to None and eliminate.
                    //       If disconnect linestring is part of a closed boundary, just ignore it.
                    if (v8.IsDisconnect())
                        {
                        switch (clone->GetBoundaryType())
                            {
                            case BentleyApi::CurveVector::BOUNDARY_TYPE_Open:
                                {
                                clone->SetBoundaryType(BentleyApi::CurveVector::BoundaryType::BOUNDARY_TYPE_None);

                                // fall through...
                                }

                            case BentleyApi::CurveVector::BOUNDARY_TYPE_None:
                                {
                                if (points.size() > 1)
                                    clone->push_back(BentleyApi::ICurvePrimitive::CreateLineString(points));

                                points.clear();
                                break;
                                }
                            }

                        continue;
                        }

                    points.push_back(DoInterop(v8));
                    }

                if (points.size() > 1)
                    clone->push_back(BentleyApi::ICurvePrimitive::CreateLineString(points));
                break;
                }

            case Bentley::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                {
                BentleyApi::DEllipse3dCP arc = (BentleyApi::DEllipse3dCP) v8Curve->GetArcCP();

                clone->push_back(BentleyApi::ICurvePrimitive::CreateArc(*arc));
                break;
                }

            case Bentley::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
                {
                BentleyApi::MSInterpolationCurveCP fitCurve = (BentleyApi::MSInterpolationCurveCP)  v8Curve->GetInterpolationCurveCP();

                clone->push_back(BentleyApi::ICurvePrimitive::CreateInterpolationCurve(*fitCurve));
                break;
                }

            case Bentley::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                {
                BentleyApi::DSpiral2dPlacementCP spiralData = (BentleyApi::DSpiral2dPlacementCP) v8Curve->GetSpiralPlacementCP();
                double uorsPerMeter = 1.0;

                if (nullptr != v8ToDgnDbTrans)
                    {
                    DVec3d scaleVec;
                    RotMatrix rMatrix;

                    // NOTE: CreateSpiral assumes data is meters, need to adjust default max stroke length to account for uor->meter conversion scale...
                    v8ToDgnDbTrans->GetMatrix(rMatrix);
                    rMatrix.NormalizeColumnsOf(rMatrix, scaleVec);
                    uorsPerMeter = 1.0 / DoubleOps::Max(scaleVec.x, scaleVec.y, scaleVec.z);
                    }

                clone->push_back(BentleyApi::ICurvePrimitive::CreateSpiral(*spiralData->spiral, spiralData->frame, spiralData->fractionA, spiralData->fractionB, 10.0 * uorsPerMeter));
                break;
                }

            case Bentley::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
                {
                clone->push_back(BentleyApi::ICurvePrimitive::CreateAkimaCurve(DoInterop(&v8Curve->GetAkimaCurveCP()->front()), v8Curve->GetAkimaCurveCP()->size()));
                break;
                }

            case Bentley::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                {
                BentleyApi::MSBsplineCurveCP bcurve = (BentleyApi::MSBsplineCurveCP) v8Curve->GetProxyBsplineCurveCP();

                clone->push_back(BentleyApi::ICurvePrimitive::CreateBsplineCurve(*bcurve));
                break;
                }

            case Bentley::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
                {
                clone->push_back(BentleyApi::ICurvePrimitive::CreatePointString(DoInterop(&v8Curve->GetPointStringCP()->front()), v8Curve->GetPointStringCP()->size()));
                break;
                }

            case Bentley::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                {
                BentleyApi::CurveVectorPtr  nestedClone;

                // Nested curve vector can occur with BOUNDARY_TYPE_None and not just union/parity regions...
                ConvertCurveVector(nestedClone, *v8Curve->GetChildCurveVectorCP(), v8ToDgnDbTrans);

                if (!nestedClone.IsValid())
                    {
                    BeAssert(false);
                    break;
                    }

                clone->push_back(BentleyApi::ICurvePrimitive::CreateChildCurveVector_SwapFromSource(*nestedClone));
                break;
                }

            default:
                {
                BeAssert(false && "Unexpected entry in V8 CurveVector.");
                break;
                }
            }
        }
#ifndef NDEBUG
    for (auto curve : *clone)
        BeAssert (curve.IsValid());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertSolidPrimitive(BentleyApi::ISolidPrimitivePtr& clone, Bentley::ISolidPrimitiveCR v8Entity)
    {
    switch (v8Entity.GetSolidPrimitiveType())
        {
        case Bentley::SolidPrimitiveType_DgnTorusPipe:
            {
            Bentley::DgnTorusPipeDetail  detail;

            if (!v8Entity.TryGetDgnTorusPipeDetail(detail))
                break;

            clone = BentleyApi::ISolidPrimitive::CreateDgnTorusPipe((BentleyApi::DgnTorusPipeDetailCR) detail);
            break;
            }

        case Bentley::SolidPrimitiveType_DgnCone:
            {
            Bentley::DgnConeDetail  detail;

            if (!v8Entity.TryGetDgnConeDetail(detail))
                break;

            if (detail.m_vector0.Magnitude() < 1.0e-8 || detail.m_vector90.Magnitude() < 1.0e-8)
                return; // Ignore garbage cones, turns into zingers when transformed...

            clone = BentleyApi::ISolidPrimitive::CreateDgnCone((BentleyApi::DgnConeDetailCR) detail);
            break;
            }

        case Bentley::SolidPrimitiveType_DgnBox:
            {
            Bentley::DgnBoxDetail  detail;

            if (!v8Entity.TryGetDgnBoxDetail(detail))
                break;

            clone = BentleyApi::ISolidPrimitive::CreateDgnBox((BentleyApi::DgnBoxDetailCR) detail);
            break;
            }

        case Bentley::SolidPrimitiveType_DgnSphere:
            {
            Bentley::DgnSphereDetail  detail;

            if (!v8Entity.TryGetDgnSphereDetail(detail))
                break;

            clone = BentleyApi::ISolidPrimitive::CreateDgnSphere((BentleyApi::DgnSphereDetailCR) detail);
            break;
            }

        case Bentley::SolidPrimitiveType_DgnExtrusion:
            {
            Bentley::DgnExtrusionDetail  detail;

            if (!v8Entity.TryGetDgnExtrusionDetail(detail))
                break;

            BentleyApi::CurveVectorPtr profileClone;

            ConvertCurveVector(profileClone, *detail.m_baseCurve, nullptr);

            BentleyApi::DgnExtrusionDetail detailClone(profileClone, DoInterop(detail.m_extrusionVector), detail.m_capped);

            clone = BentleyApi::ISolidPrimitive::CreateDgnExtrusion(detailClone);
            break;
            }

        case Bentley::SolidPrimitiveType_DgnRotationalSweep:
            {
            Bentley::DgnRotationalSweepDetail  detail;

            if (!v8Entity.TryGetDgnRotationalSweepDetail(detail))
                break;

            BentleyApi::CurveVectorPtr profileClone;

            ConvertCurveVector(profileClone, *detail.m_baseCurve, nullptr);

            BentleyApi::DgnRotationalSweepDetail detailClone(profileClone, DoInterop(detail.m_axisOfRotation.origin), DoInterop(detail.m_axisOfRotation.direction), detail.m_sweepAngle, detail.m_capped);

            detailClone.m_numVRules = detail.m_numVRules;
            clone = BentleyApi::ISolidPrimitive::CreateDgnRotationalSweep(detailClone);
            break;
            }

        case Bentley::SolidPrimitiveType_DgnRuledSweep:
            {
            Bentley::DgnRuledSweepDetail  detail;

            if (!v8Entity.TryGetDgnRuledSweepDetail(detail))
                break;

            BentleyApi::DgnRuledSweepDetail detailClone;

            for (Bentley::CurveVectorPtr curves : detail.m_sectionCurves)
                {
                BentleyApi::CurveVectorPtr profileClone;

                ConvertCurveVector(profileClone, *curves, nullptr);
                detailClone.m_sectionCurves.push_back(profileClone);
                }

            detailClone.m_capped = detail.m_capped;
            clone = BentleyApi::ISolidPrimitive::CreateDgnRuledSweep(detailClone);
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertMSBsplineSurface(BentleyApi::MSBsplineSurfacePtr& clone, Bentley::MSBsplineSurfaceCR v8Entity)
    {
    clone = BentleyApi::MSBsplineSurface::CreatePtr();

    clone->CopyFrom((BentleyApi::MSBsplineSurfaceCR) v8Entity);
   }
// #define TEST_AUXDATA
// #define TEST_AUXDATA_HEIGHT
// #define TEST_AUXDATA_SPHERICAL_WITH_NORMALS
// #define TEST_AUXDATA_WAVES
// #define TEST_AUXDATA_TO_JSON
// #define TEST_AUXDATA_CANTILEVER


#ifdef TEST_AUXDATA

PolyfaceAuxDataPtr              s_testAuxData;
static void getTestAuxData(Bentley::PolyfaceQueryCR polyface);
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertPolyface(BentleyApi::PolyfaceHeaderPtr& clone, Bentley::PolyfaceQueryCR v8Entity)
    {
    // NOTE: Assumes that V8 color types have already been resolved to valid int colors...
    BentleyApi::PolyfaceQueryCarrier sourceData(v8Entity.GetNumPerFace(), v8Entity.GetTwoSided(), v8Entity.GetPointIndexCount(),
                                                v8Entity.GetPointCount(), DoInterop(v8Entity.GetPointCP()), v8Entity.GetPointIndexCP(),
                                                v8Entity.GetNormalCount(), DoInterop(v8Entity.GetNormalCP()), v8Entity.GetNormalIndexCP(),
                                                v8Entity.GetParamCount(), DoInterop(v8Entity.GetParamCP()), v8Entity.GetParamIndexCP(),
                                                v8Entity.GetColorCount(), v8Entity.GetColorIndexCP(), v8Entity.GetIntColorCP(),
                                                v8Entity.GetIlluminationNameCP(), v8Entity.GetMeshStyle(), v8Entity.GetNumPerRow());

    clone = BentleyApi::PolyfaceHeader::New();
    clone->CopyFrom(sourceData);

#ifdef TEST_AUXDATA
    clone->SetAuxData(s_testAuxData);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertTextString(TextStringPtr& clone, Bentley::TextStringCR v8Text, DgnFileR dgnFile, Converter& converter)
    {
    uint32_t v8FontId = 0;
    dgnFile.GetDgnFontMapP()->GetFontNumber(v8FontId, v8Text.GetProperties().GetFont(), false);

    DgnFont const& dbFont = converter._RemapV8Font(dgnFile, v8FontId);
    Utf8String dbTextValue(v8Text.GetString());

    DgnDbApi::TextString dbText;
    dbText.SetText(dbTextValue.c_str());
    dbText.SetOrigin(DoInterop(v8Text.GetOrigin()));
    dbText.SetOrientation(DoInterop(v8Text.GetRotMatrix()));
    dbText.GetStyleR().SetFont(dbFont);
    dbText.GetStyleR().SetIsBold(v8Text.GetProperties().IsBold());
    dbText.GetStyleR().SetIsItalic(v8Text.GetProperties().IsItalic());
    dbText.GetStyleR().SetSize(DoInterop(v8Text.GetProperties().GetFontSize()));

    // DgnV8 sub-/super-script is a hard-coded display-time scale and shift.
    if (v8Text.GetProperties().IsSubScript() || v8Text.GetProperties().IsSuperScript())
        {
        DPoint2d scaledSize = dbText.GetStyle().GetSize();
        scaledSize.Scale(0.3);
        dbText.GetStyleR().SetSize(scaledSize);
        }

    // Because DgnV8 has unsupported underline (and overline) styles, never tell this hacked DB
    // TextString to draw an underline even if it's present, and draw it manually ourselves.

    // Internal implementation detail: A DgnV8 TextString will report 0 glyphs unless the caller
    // performed layout with a listener that claimed to capture the glyphs.
    struct ShimGlyphLayoutListener : DgnV8Api::IDgnGlyphLayoutListener
        {
        virtual void _OnGlyphAnnounced(DgnV8Api::DgnGlyph&, Bentley::DPoint3d const&) override {}
        virtual UInt32 _OnFontAnnounced(DgnV8Api::TextString const&) override {return 0;}
        virtual bool _DidCacheGlyphs() override {return true;}
        };
    static ShimGlyphLayoutListener s_shimGlyphLayoutListener;

    // Force the DgnV8 TextString to do its layout pass.
    // If it cannot perform layout (should never happen, but we've seen issues resolving fallback fonts), then we cannot
	// transplant corrupt V8 layout data and pretend the TextString is valid... need to let DgnDb try to recover later.
	if (SUCCESS != v8Text.LoadGlyphs(&s_shimGlyphLayoutListener))
		{
		// TODO: log that this element won't necessarily match fidelity.
		clone = dbText.Clone();
		return;
		}

    // Mark the DB TextString as valid so it doesn't try to perform its own layout later.
    dbText.m_isValid = true;

    // Directly copy the DgnV8 TextString's layout information into the DB TextString's cache.
    size_t v8NumGlyphs = v8Text.GetNumGlyphs();
    dbText.m_glyphs.resize(v8NumGlyphs);
    dbText.m_glyphIds.resize(v8NumGlyphs);
    dbText.m_glyphOrigins.resize(v8NumGlyphs);

    // Has the side effect of loading the font data, which is required for FindGlyphCP anyway.
    if (!dbText.GetStyle().GetFont().IsResolved())
        { BeAssert(false); }

    DgnFontStyle dbFontStyle = DgnFont::FontStyleFromBoldItalic(dbText.GetStyle().IsBold(), dbText.GetStyle().IsItalic());

    // N.B. Ensure to use the TextString's font object. It took steps to resolve the font (vs. this
    // function's local dbFont variable), and this data needs to match its exact font.
    DgnFontCR resolvedDbFont = dbText.GetStyle().GetFont();

    // In terms of a glyph ID within a TT font, PowerPlatform supports both glyph and character
    // index, but does not currently expose what "glyph code" actually means in the public API (only
    // DgnTrueTypeGlyph actually retains this information, which is in a CPP file). It's unclear to
    // me in Uniscribe's documentation if setting SCRIPT_ANALYSIS::fNoGlyphIndex to FALSE
    // necessarily forces use of glyph indices, but it's highly suggestive. PP normally sets to
    // FALSE, except if !T_HOST.GetDgnFontManager().IsGlyphShapingEnabled() ||
    // layoutContext.IsVertical(), so that's as good as we can get unless/until we can change PP's
    // API to explicitly expose this information.
    bvector<unsigned int> derivedGlyphIndices;
    bool useDerivedGlyphIndices = false;
    if ((DgnFontType::TrueType == resolvedDbFont.GetType())
            && (!DgnV8Api::DgnFontManager::GetManager().IsGlyphShapingEnabled()
                || v8Text.GetProperties().IsVertical()))
            {
            useDerivedGlyphIndices = true;
            derivedGlyphIndices = ((DgnTrueTypeFontCR)resolvedDbFont).ComputeGlyphIndices(dbText.GetText().c_str(), dbText.GetStyle().IsBold(), dbText.GetStyle().IsItalic());

            // I can't image how this would differ, but I'd rather be defensive since we'll use
            // v8Text.GetNumGlyphs as loop control below.
            BeAssert(derivedGlyphIndices.size() == v8Text.GetNumGlyphs());
            if (derivedGlyphIndices.size() < v8Text.GetNumGlyphs())
                {
                // Fill with 0's... better than crashing.
                derivedGlyphIndices.resize(v8Text.GetNumGlyphs());
                }
            }

    for (size_t iV8Glyph = 0; iV8Glyph < v8Text.GetNumGlyphs(); ++iV8Glyph)
        {
        DgnGlyph::T_Id resolvedGlyphId = useDerivedGlyphIndices ? static_cast<DgnGlyph::T_Id>(derivedGlyphIndices[iV8Glyph]) : v8Text.GetGlyphCodes()[iV8Glyph];

        dbText.m_glyphIds[iV8Glyph] = resolvedGlyphId;
        dbText.m_glyphs[iV8Glyph] = resolvedDbFont.FindGlyphCP(resolvedGlyphId, dbFontStyle);
        dbText.m_glyphOrigins[iV8Glyph] = DoInterop(v8Text.GetGlyphOrigins()[iV8Glyph]);
        }

    dbText.m_range.low = DoInterop(v8Text.GetExtents().low);
    dbText.m_range.high = DoInterop(v8Text.GetExtents().high);

    clone = dbText.Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::ConvertSolidKernelEntity(IBRepEntityPtr& clone, Bentley::ISolidKernelEntityCR v8Entity)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    // NOTE: As long as we don't call PK_SESSION_start in the Db code we can continue to use the
    //       frustrum registered by the v8 PSolidCore; which is necessary since they share pskernel...
    BeAssert(!DgnDbApi::PSolidKernelManager::IsSessionStarted());
    DgnDbApi::PSolidKernelManager::SetExternalFrustrum(true);

    clone = DgnDbApi::PSolidUtil::CreateNewEntity(DgnV8Api::PSolidUtil::GetEntityTag(v8Entity), DoInterop(v8Entity.GetEntityTransform()), false); // <- Not owned...
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::InitLineStyle(Render::GeometryParams& params, DgnModelRefR styleModelRef, int32_t srcLineStyleNum, DgnV8Api::LineStyleParams const* v8lsParams)
    {
    DgnFileP styleFile = styleModelRef.GetDgnFileP();
    if (nullptr == styleFile)
        {
        ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::MissingLsDefinitionFile(), NULL,
                     IssueReporter::FmtModelRef(styleModelRef).c_str());
        return;
        }

    double          unitsScale;

    DgnStyleId mappedStyleId = _RemapLineStyle(unitsScale, *styleFile, srcLineStyleNum, true);
    if (!mappedStyleId.IsValid())
        return;

    double modelLsScale = styleModelRef.GetRoot()->GetLineStyleScale();
    DgnV8Api::ModelInfo const& v8ModelInfo = styleModelRef.GetRoot()->GetModelInfo();
    double uorPerMeter = DgnV8Api::ModelInfo::GetUorPerMeter(&v8ModelInfo);
    Render::LineStyleParams lsParams;
    ConvertLineStyleParams(lsParams, v8lsParams, uorPerMeter, unitsScale, modelLsScale);
    LineStyleInfoPtr lsInfo = LineStyleInfo::Create(mappedStyleId, &lsParams);
    params.SetLineStyle(lsInfo.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Converter::InitGeometryParams(Render::GeometryParams& params, DgnV8Api::ElemDisplayParams& paramsV8, DgnV8Api::ViewContext& context, bool is3d, DgnV8ModelCR v8Model)
    {
    // NOTE: Resolve loses information like WEIGHT_BYLEVEL and we may be called multiple times (ex. disjoint brep)...
    AutoRestore<DgnV8Api::ElemDisplayParams> saveParamsV8(&paramsV8);

    UInt32  rawColor  = paramsV8.GetLineColor();
    UInt32  rawFill   = paramsV8.GetFillColor();
    UInt32  rawWeight = paramsV8.GetWeight();
    Int32   rawStyle  = paramsV8.GetLineStyle();

    // Apply ignores and resolve effective now that we've saved off the ByLevel information...
    paramsV8.Resolve(context);

    // NOTE: ElemHeaderOverrides have been pushed but have not been applied to paramsV8...
    DgnV8Api::LevelId v8Level = (0 == paramsV8.GetLevel() ? DGNV8_LEVEL_DEFAULT_LEVEL_ID : paramsV8.GetLevel());
    DgnV8Api::ElemHeaderOverrides const* ovr = context.GetHeaderOvr();

    if (nullptr != ovr && ovr->GetFlags().level)
        v8Level = ovr->AdjustLevel(v8Level, *context.GetCurrentModel());

    DgnSubCategoryId subCategoryId = params.GetSubCategoryId(); // see if the caller wants to specify an override SubCategoryId
    if (!subCategoryId.IsValid()) // if not, look up the SubCategoryId from V8 LevelId
        subCategoryId = GetSyncInfo().GetSubCategory(v8Level, v8Model, is3d? SyncInfo::LevelExternalSourceAspect::Type::Spatial: SyncInfo::LevelExternalSourceAspect::Type::Drawing);
    DgnCategoryId categoryId = DgnSubCategory::QueryCategoryId(GetDgnDb(), subCategoryId);

    params = Render::GeometryParams();
    params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);

    if (!is3d)
        {
        if (DgnV8Api::DISPLAYPRIORITY_BYCELL != paramsV8.GetElementDisplayPriority())
            params.SetDisplayPriority(paramsV8.GetElementDisplayPriority());
        else if (nullptr != ovr)
            params.SetDisplayPriority(ovr->GetDisplayPriority());
        }

    switch (paramsV8.GetElementClass())
        {
        case DgnV8Api::DgnElementClass::Primary:
        case DgnV8Api::DgnElementClass::PrimaryRule: // Shouldn't see this normally...
            params.SetGeometryClass(Render::DgnGeometryClass::Primary);
            break;

        case DgnV8Api::DgnElementClass::Construction:
        case DgnV8Api::DgnElementClass::ConstructionRule: // Shouldn't see this normally...
            params.SetGeometryClass(Render::DgnGeometryClass::Construction);
            break;

        case DgnV8Api::DgnElementClass::Dimension:
            params.SetGeometryClass(Render::DgnGeometryClass::Dimension);
            break;

        case DgnV8Api::DgnElementClass::PatternComponent:
            params.SetGeometryClass(Render::DgnGeometryClass::Pattern);
            break;

        case DgnV8Api::DgnElementClass::LinearPatterned: // Don't intend to support this anymore...
        default:
            params.SetGeometryClass(Render::DgnGeometryClass::Primary);
            break;
        }

    if (nullptr != ovr && ovr->GetFlags().color)
        {
        if (DgnV8Api::COLOR_BYLEVEL != ovr->GetColor())
            {
            DgnV8Api::IntColorDef intColorDef;

            if (SUCCESS == DgnV8Api::DgnColorMap::ExtractElementColorInfo(&intColorDef, nullptr, nullptr, nullptr, nullptr, ovr->GetColor(), *context.GetCurrentModel()->GetDgnFileP()))
                params.SetLineColor(ColorDef(intColorDef.m_int));
            }
        }
    else if (DgnV8Api::COLOR_BYLEVEL != rawColor)
        {
        params.SetLineColor(ColorDef(paramsV8.GetLineColorTBGR()));
        }

    if (nullptr != ovr && ovr->GetFlags().weight)
        {
        if (DgnV8Api::WEIGHT_BYLEVEL != ovr->GetWeight())
            params.SetWeight(ovr->GetWeight());
        }
    else if (DgnV8Api::WEIGHT_BYLEVEL != rawWeight)
        {
        params.SetWeight(paramsV8.GetWeight());
        }

    DgnModelRefP styleModelRef;

    if (nullptr != ovr && ovr->GetFlags().style && nullptr != (styleModelRef = (nullptr == ovr->GetLineStyleModelRef()) ? context.GetCurrentModel() :ovr->GetLineStyleModelRef()))
        {
        if (DgnV8Api::STYLE_BYLEVEL != ovr->GetLineStyle() && ovr->GetLineStyle() != 0)
            {
            InitLineStyle(params, *styleModelRef, ovr->GetLineStyle(), ovr->GetLineStyleParams());
            }
        }
    else if (paramsV8.GetLineStyle() != 0 && nullptr != (styleModelRef = (nullptr == paramsV8.GetLineStyleModelRef()) ? context.GetCurrentModel() : paramsV8.GetLineStyleModelRef()))
        {
        DgnV8Api::LineStyleParams const* lsParamsV8 = paramsV8.GetLineStyleParams();

        // Ugh...still need modifiers like start/end width for STYLE_BYLEVEL... :(
        if (DgnV8Api::STYLE_BYLEVEL != rawStyle || (lsParamsV8 && 0 != lsParamsV8->modifiers))
            InitLineStyle(params, *styleModelRef, paramsV8.GetLineStyle(), lsParamsV8);
        }
    else if (DgnV8Api::STYLE_BYLEVEL != rawStyle)
        {
        params.SetLineStyle(nullptr);
        }

    params.SetTransparency(paramsV8.GetTransparency());

    if (DgnV8Api::FillDisplay::Never != paramsV8.GetFillDisplay())
        {
        params.SetFillDisplay((Render::FillDisplay) paramsV8.GetFillDisplay());

        if (nullptr != paramsV8.GetGradient())
            {
            DgnV8Api::GradientSymb const& gradient = *paramsV8.GetGradient();
            Render::GradientSymbPtr gradientPtr = Render::GradientSymb::Create();

            gradientPtr->SetMode((Render::GradientSymb::Mode) gradient.GetMode());
            gradientPtr->SetFlags((Render::GradientSymb::Flags) gradient.GetFlags());
            gradientPtr->SetShift(gradient.GetShift());
            gradientPtr->SetTint(gradient.GetTint());
            gradientPtr->SetAngle(gradient.GetAngle());

            bvector<ColorDef> keyColors;
            bvector<double>   keyValues;

            for (int i=0; i < gradient.GetNKeys(); i++)
                {
                double keyValue;
                Bentley::RgbColorDef keyColor;

                gradient.GetKey(keyColor, keyValue, i);

                DgnDbApi::ColorDef keyColorDef(keyColor.red, keyColor.green, keyColor.blue);

                keyColors.push_back(keyColorDef);
                keyValues.push_back(keyValue);
                }

            gradientPtr->SetKeys((uint16_t) keyColors.size(), &keyColors.front(), &keyValues.front());

            params.SetGradient(gradientPtr.get());
            }
        else
            {
            if (nullptr != ovr && ovr->GetFlags().color)
                {
                if (DgnV8Api::COLOR_BYLEVEL != ovr->GetColor())
                    {
                    DgnV8Api::IntColorDef intColorDef;

                    if (SUCCESS == DgnV8Api::DgnColorMap::ExtractElementColorInfo(&intColorDef, nullptr, nullptr, nullptr, nullptr, ovr->GetColor(), *context.GetCurrentModel()->GetDgnFileP()))
                        params.SetFillColor(ColorDef(intColorDef.m_int));
                    }
                }
            else if (DgnV8Api::DgnColorMap::INDEX_Background == rawFill)
                {
                params.SetFillColorFromViewBackground(DgnV8Api::DgnColorMap::INDEX_Background != rawColor);
                }
            else if (DgnV8Api::COLOR_BYLEVEL != rawFill)
                {
                params.SetFillColor(ColorDef(paramsV8.GetFillColorTBGR()));
                }
            }
        }

    if (paramsV8.IsRenderable())
        {
        if (nullptr != paramsV8.GetMaterial())
            {
            RenderMaterialId materialId = GetRemappedMaterial(paramsV8.GetMaterial());
            DgnSubCategoryCPtr          dgnDbSubCategory = DgnSubCategory::Get(GetDgnDb(), subCategoryId);

            if (!dgnDbSubCategory.IsValid() || dgnDbSubCategory->GetAppearance().GetRenderMaterial() != materialId)
                params.SetMaterialId(materialId);

            if (nullptr != paramsV8.GetMaterialUVDetailP())
                {
                // params.SetMaterialUVDetails();
                }
            }
        }

    params.Resolve(GetDgnDb()); // Need to be able to check for a stroked linestyle...
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct V8SymbolGraphicsCollector : DgnV8Api::IElementGraphicsProcessor
{
private:

DgnV8Api::ViewContext*      m_context;
Bentley::Transform          m_conversionScale;
Bentley::Transform          m_currentTransform;
DgnV8Api::ElemDisplayParams m_currentDisplayParams;

Converter&                  m_converter;
DgnV8ModelCR                m_v8Model;

bvector<GeometricPrimitivePtr>  m_symbolGeometry;
bvector<Render::GeometryParams> m_symbolParams;
bool                            m_allowMultiSymb = false;
bool                            m_foundText = false;

protected:

virtual DgnV8Api::DrawPurpose _GetDrawPurpose() override {return DgnV8Api::DrawPurpose::DgnDbConvert;} // Required to avoid resolving ByLevel...

virtual bool _ProcessAsBody(bool isCurved) const override {return false;}
virtual bool _ProcessAsFacets(bool isPolyface) const override {return false;}
virtual void _AnnounceContext(DgnV8Api::ViewContext& context) override {m_context = &context;}
virtual void _AnnounceTransform(Bentley::TransformCP trans) override {if (trans) m_currentTransform = *trans; else m_currentTransform.InitIdentity();}
virtual void _AnnounceElemDisplayParams(DgnV8Api::ElemDisplayParams const& displayParams) override {m_currentDisplayParams = displayParams;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Bentley::BentleyStatus _ProcessCurveVector(Bentley::CurveVectorCR curves, bool isFilled) override
    {
    BentleyApi::CurveVectorPtr clone;

    Transform v8ToDgnDbTrans = DoInterop(Bentley::Transform::FromProduct(m_conversionScale, m_currentTransform));

    Converter::ConvertCurveVector(clone, curves, &v8ToDgnDbTrans);

    GeometricPrimitivePtr elemGeom = GeometricPrimitive::Create(clone);

    AddSymbolGeometry(elemGeom, isFilled && curves.IsAnyRegionType());

    return Bentley::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Bentley::BentleyStatus _ProcessSolidPrimitive(Bentley::ISolidPrimitiveCR primitive) override
    {
    BentleyApi::ISolidPrimitivePtr clone;

    Converter::ConvertSolidPrimitive(clone, primitive);

    GeometricPrimitivePtr elemGeom = GeometricPrimitive::Create(clone);

    AddSymbolGeometry(elemGeom, false);

    return Bentley::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Bentley::BentleyStatus _ProcessSurface(Bentley::MSBsplineSurfaceCR surface) override
    {
    BentleyApi::MSBsplineSurfacePtr clone;

    Converter::ConvertMSBsplineSurface(clone, surface);

    GeometricPrimitivePtr elemGeom = GeometricPrimitive::Create(clone);

    AddSymbolGeometry(elemGeom, false);

    return Bentley::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Bentley::BentleyStatus _ProcessFacets(Bentley::PolyfaceQueryCR meshData, bool isFilled) override
    {
    BentleyApi::PolyfaceHeaderPtr clone;

    Converter::ConvertPolyface(clone, meshData);

    GeometricPrimitivePtr elemGeom = GeometricPrimitive::Create(clone);

    AddSymbolGeometry(elemGeom, false);

    return Bentley::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/18
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Bentley::BentleyStatus _ProcessBody(Bentley::ISolidKernelEntityCR entity, Bentley::IFaceMaterialAttachmentsCP attachments)
    {
    // Don't allow BReps in symbols...output non-BRep face geometry...
    DgnV8Api::DgnPlatformLib::GetHost().GetSolidsKernelAdmin()._OutputBodyAsSurfaces(entity, *m_context, true, attachments);

    return Bentley::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Bentley::BentleyStatus _ProcessTextString(Bentley::TextStringCR v8Text)
    {
    m_foundText = true; // Ugh, we don't have a good place to clear this...hopefully a single symbol won't contain both text and shapes...

    return Bentley::ERROR; // Is there any reason we shouldn't always drop symbol text?
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void AddSymbolGeometry(GeometricPrimitivePtr& geometry, bool isFilled)
    {
    Transform   v8ToDgnDbTrans = DoInterop(Bentley::Transform::FromProduct(m_conversionScale, m_currentTransform));

    geometry->TransformInPlace(v8ToDgnDbTrans);
    m_symbolGeometry.push_back(geometry);

    // NOTE: Establish symbol symbology using first drawn when multi-symbology symbol isn't supported...
    if (0 != m_symbolParams.size() && !m_allowMultiSymb)
        return;

    Render::GeometryParams geomParams;
    m_converter.InitGeometryParams(geomParams, m_currentDisplayParams, *m_context, true, m_v8Model);

    if (isFilled && Render::FillDisplay::Never == geomParams.GetFillDisplay() && m_foundText)
        geomParams.SetFillDisplay(Render::FillDisplay::Always); // Dropped glyph regions should always be drawn filled...

    m_symbolParams.push_back(geomParams);
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
V8SymbolGraphicsCollector(Converter& converter, DgnV8ModelCR v8Model, BentleyApi::TransformCR conversionScale) : m_converter(converter), m_v8Model(v8Model), m_context(nullptr), m_conversionScale(DoInterop(conversionScale)) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<GeometricPrimitivePtr>& GetSymbolGeometry() {return m_symbolGeometry;}
bvector<Render::GeometryParams>& GetSymbolDisplayParams() {return m_symbolParams;}
void SetAllowMultiSymb() {m_allowMultiSymb = true;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ProcessSymbol(DgnV8Api::IDisplaySymbol& symbol, DgnV8ModelR model) {DgnV8Api::ElementGraphicsOutput::Process(symbol, model, *this);}

}; // V8SymbolGraphicsCollector

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeometryPartId Converter::QueryGeometryPartId(Utf8StringCR name)
    {
    return SyncInfo::GeomPartExternalSourceAspect::GetAspectByTag(*m_dgndb, GetJobDefinitionModel()->GetModeledElementId(), name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Converter::QueryGeometryPartTag(DgnGeometryPartId partId)
    {
    auto geomPart = m_dgndb->Elements().Get<DgnGeometryPart>(partId);
    if (!geomPart.IsValid())
        {
        BeAssert(false);
        return "";
        }
    auto aspect = SyncInfo::GeomPartExternalSourceAspect::GetAspect(*geomPart);
    return aspect.IsValid()? aspect.GetIdentifier(): "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      11/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::RecordGeometryPartId(DgnGeometryPartId partId, Utf8StringCR partTag)
    {
    auto aspect = SyncInfo::GeomPartExternalSourceAspect::CreateAspect(GetJobDefinitionModel()->GetModeledElementId(), partTag, GetDgnDb());
    auto partElem = GetDgnDb().Elements().GetForEdit<DgnGeometryPart>(partId);
    aspect.AddAspect(*partElem);
    return partElem->Update().IsValid()? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool Converter::InitPatternParams(PatternParamsR pattern, DgnV8Api::PatternParams const& patternV8, Bentley::bvector<DgnV8Api::DwgHatchDefLine> const& defLinesV8, Bentley::DPoint3d& origin, DgnV8Api::ViewContext& context)
    {
    double uorPerMeter = DgnV8Api::ModelInfo::GetUorPerMeter(&context.GetCurrentModel()->GetDgnModelP()->GetModelInfo());

    if (DgnV8Api::PatternParamsModifierFlags::None != (patternV8.modifiers & DgnV8Api::PatternParamsModifierFlags::Cell))
        {
        Utf8String nameStr;
        nameStr.Assign(patternV8.cellName);
        Utf8PrintfString partTag("PatternV8-%lld-%s-%lld", GetRepositoryLinkId(*context.GetCurrentModel()->GetDgnFileP()).GetValue(), nameStr.c_str(), patternV8.cellId);
        DgnGeometryPartId partId = QueryGeometryPartId(partTag.c_str());

        if (!partId.IsValid())
            {
            DgnV8Api::ElementHandle v8Eh(patternV8.cellId, context.GetCurrentModel());

            if (!v8Eh.IsValid())
                return false;

            bool isPointCell = (!v8Eh.GetElementCP()->hdr.dhdr.props.b.s && v8Eh.GetElementCP()->hdr.dhdr.props.b.r);
            double scale = 1.0/uorPerMeter;
            Transform scaleTransform = Transform::FromScaleFactors(scale, scale, scale);

            V8SymbolGraphicsCollector collector(*this, context.GetCurrentModel()->GetDgnFileP()->GetDictionaryModel(), scaleTransform);

            if (!isPointCell)
                collector.SetAllowMultiSymb();

            DgnV8Api::ElementGraphicsOutput::Process(v8Eh, collector);
            bvector<GeometricPrimitivePtr>& geometry = collector.GetSymbolGeometry();

            if (0 == geometry.size())
                return false;

            size_t iSymb = 0;
            GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(GetDgnDb(), true);

            for (GeometricPrimitivePtr geom : geometry)
                {
                if (!isPointCell)
                    builder->Append(collector.GetSymbolDisplayParams().at(iSymb++));

                builder->Append(*geom);
                }

            DgnGeometryPartPtr geomPart = DgnGeometryPart::Create(*GetJobDefinitionModel());

            if (SUCCESS != builder->Finish(*geomPart) || !GetDgnDb().Elements().Insert<DgnGeometryPart>(*geomPart).IsValid())
                return false;

            partId = geomPart->GetId();
            RecordGeometryPartId(partId, partTag);
            }

        pattern.SetSymbolId(partId);
        pattern.SetPrimarySpacing(patternV8.space1);
        pattern.SetSecondarySpacing(patternV8.space2);
        pattern.SetPrimaryAngle(patternV8.angle1);
        pattern.SetScale(patternV8.scale * uorPerMeter); // Need uor scale, not user scale...full V8 to DgnDb transform is applied to PatternParams later...
        }
    else if (DgnV8Api::PatternParamsModifierFlags::None != (patternV8.modifiers & DgnV8Api::PatternParamsModifierFlags::DwgHatchDef))
        {
        bvector<DwgHatchDefLine> dwgHatchDef;

        for (auto defLineV8 : defLinesV8)
            {
            DwgHatchDefLine defLine;

            defLine.m_angle   = defLineV8.angle;
            defLine.m_through = DoInterop(defLineV8.through);
            defLine.m_offset  = DoInterop(defLineV8.offset);
            defLine.m_nDashes = (defLineV8.nDashes < MAX_DWG_HATCH_LINE_DASHES ? defLineV8.nDashes : MAX_DWG_HATCH_LINE_DASHES);

            for (short iDash = 0; iDash < defLine.m_nDashes; iDash++)
                defLine.m_dashes[iDash] = defLineV8.dashes[iDash];

            dwgHatchDef.push_back(defLine);
            }

        pattern.SetDwgHatchDef(dwgHatchDef);
        pattern.SetPrimaryAngle(patternV8.angle1); // NOTE: angle/scale baked into hatch def lines, saved for placement info...
        pattern.SetScale(patternV8.scale * uorPerMeter); // Need uor scale, not user scale...full V8 to DgnDb transform is applied to PatternParams later...

        if (DgnV8Api::PatternParamsModifierFlags::None == (patternV8.modifiers & DgnV8Api::PatternParamsModifierFlags::DwgHatchOrigin))
            {
            // Old style DWG Hatch Definitions are implicitly about (0,0), need to ignore "element" origin...
            patternV8.rMatrix.MultiplyTranspose(origin);
            origin.x = origin.y = 0.0;
            patternV8.rMatrix.Multiply(origin);
            }
        }
    else
        {
        pattern.SetPrimarySpacing(patternV8.space1);
        pattern.SetPrimaryAngle(patternV8.angle1);

        if (DgnV8Api::PatternParamsModifierFlags::None != (patternV8.modifiers & DgnV8Api::PatternParamsModifierFlags::Space2))
            {
            pattern.SetSecondarySpacing(patternV8.space2);
            pattern.SetSecondaryAngle(patternV8.angle2);
            }
        }

    pattern.SetOrientation(DoInterop(patternV8.rMatrix));
    pattern.SetOrigin(DoInterop(origin));

    if (DgnV8Api::PatternParamsModifierFlags::None != (patternV8.modifiers & DgnV8Api::PatternParamsModifierFlags::Color))
        {
        DgnV8Api::IntColorDef intColorDef;

        if (SUCCESS == DgnV8Api::DgnColorMap::ExtractElementColorInfo(&intColorDef, nullptr, nullptr, nullptr, nullptr, patternV8.color, *context.GetCurrentModel()->GetDgnFileP()))
            pattern.SetColor(ColorDef(intColorDef.m_int));
        }

    if (DgnV8Api::PatternParamsModifierFlags::None != (patternV8.modifiers & DgnV8Api::PatternParamsModifierFlags::Weight))
        pattern.SetWeight(patternV8.weight);

    pattern.SetSnappable(DgnV8Api::PatternParamsModifierFlags::None != (patternV8.modifiers & DgnV8Api::PatternParamsModifierFlags::Snap));

    return true;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct DisplayPointSymbol : DgnV8Api::IDisplaySymbol
{
private:
    DgnV8Api::LsCacheSymbolComponent const& m_symbolComponent;

public:
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
DisplayPointSymbol(DgnV8Api::LsCacheSymbolComponent const& symbolComponent) : m_symbolComponent(symbolComponent) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
void _Draw(DgnV8Api::ViewContext& context) override
    {
    DgnV8Api::XGraphicsContainer xgContainer(m_symbolComponent.GetXGraphicsData(), m_symbolComponent.GetXGraphicsSize());
    xgContainer.Draw(context);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
virtual StatusInt _GetRange(Bentley::DRange3d& range) const override
    {
    return m_symbolComponent._GetRange(range);
    }

}; // DisplayPointSymbol

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2014
//---------------------------------------------------------------------------------------
LineStyleStatus LineStyleConverter::ConvertPointSymbol(LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheSymbolComponent const& symbolComponent, double lsScale)
    {
    if (BSISUCCESS != m_converter.MustBeInSharedChannel("linestyles must be converted in the shared channel."))
        return LineStyleStatus::LINESTYLE_STATUS_Error;

    v10Id = LsComponentId();

    DisplayPointSymbol  displaySymbol(symbolComponent);
    Transform  trans;
    trans.InitFromScaleFactors(lsScale, lsScale, lsScale);

    V8SymbolGraphicsCollector collector(m_converter, v8File.GetDictionaryModel(), trans /* v8uors_to_mm */);

    collector.SetAllowMultiSymb(); // Symbols can contain color, weight, and line code changes...
    collector.ProcessSymbol(displaySymbol, v8File.GetDictionaryModel());

    size_t iSymb = 0;
    bvector<GeometricPrimitivePtr>& geometry = collector.GetSymbolGeometry();
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(GetDgnDb(), true);

    DRange3d totalRange;
    totalRange.Init();

    for (GeometricPrimitivePtr elemGeom : geometry)
        {
        DRange3d localRange;

        elemGeom->GetRange(localRange, nullptr);
        totalRange.Extend(localRange);

        builder->Append(collector.GetSymbolDisplayParams().at(iSymb++));
        builder->Append(*elemGeom);
        }

    DgnGeometryPartPtr geomPart = DgnGeometryPart::Create(GetDgnDb().GetDictionaryModel()); // Yes, the geom parts for lstyles go into the dictionary model, because the lstyles themselves go there. This is in the Shared channel.
    if (SUCCESS != builder->Finish(*geomPart))
        return LINESTYLE_STATUS_ConvertingComponent;

    GetDgnDb().Elements().Insert<DgnGeometryPart>(*geomPart);

    Json::Value     jsonValue(Json::objectValue);
    SetDescription(jsonValue, symbolComponent);
    v10Id = LsComponentId();

    totalRange.high.Subtract(totalRange.low);

    uint32_t symFlags = 0;
    if (symbolComponent.Is3d())
        symFlags |= LSSYM_3D;
    if (symbolComponent.IsNoScale())
        symFlags |= LSSYM_NOSCALE;

    LsSymbolComponent::SaveSymbolDataToJson(jsonValue, totalRange.low, totalRange.high, geomPart->GetId(), symFlags, symbolComponent.GetStoredUnitScale());

    return LsComponent::AddComponentAsJsonProperty(v10Id, GetDgnDb(), LsComponentType::PointSymbol, jsonValue);
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnV8PathEntry
{
GeometricPrimitivePtr   m_geometry;
Render::GeometryParams  m_geomParams;

bool        m_is3dDest;
bool        m_is3dSrc;
bool        m_isMarked;
double      m_partScale;
Transform   m_goopTrans;
Transform   m_geomToWorld;
Transform   m_v8ToDgnDbTrans;

Bentley::CurveVectorPtr          m_curve;
Bentley::ISolidPrimitivePtr      m_primitive;
Bentley::MSBsplineSurfacePtr     m_surface;
Bentley::PolyfaceHeaderPtr       m_mesh;
DgnV8Api::ISolidKernelEntityPtr  m_brep;
DgnV8Api::TextStringPtr          m_text;

IFaceMaterialAttachmentsPtr      m_attachments;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8PathEntry(TransformCR currentTransform, TransformCR conversionScale, bool is3dDest, bool is3dSrc) : m_is3dDest(is3dDest), m_is3dSrc(is3dSrc)
    {
    m_isMarked = false;
    m_partScale = 1.0;
    m_goopTrans = Transform::FromIdentity();
    m_geomToWorld = Transform::FromIdentity();
    m_v8ToDgnDbTrans = Transform::FromProduct(conversionScale, currentTransform);

    DPoint3d    origin;
    RotMatrix   rMatrix, rotation, skewFactor;

    m_v8ToDgnDbTrans.GetTranslation(origin);
    m_v8ToDgnDbTrans.GetMatrix(rMatrix);

    if (rMatrix.RotateAndSkewFactors(rotation, skewFactor, 0, 1))
        {
        // Unit scaling and pushed transforms containing scale, skew, mirror, etc. need to be applied to geometry...
        if (!skewFactor.IsIdentity())
            {
            double    goopScale;
            RotMatrix deScaledMatrix;

            m_goopTrans = Transform::From(skewFactor);
            m_partScale = 0.0;

            // NOTE: XGSymbol can use non-uniform scale for "pipe" length which we can't support with origin+YPR...
            if (skewFactor.IsRigidSignedScale(deScaledMatrix, goopScale))
                {
                double      convertScale;
                RotMatrix   convertRMatrix;

                conversionScale.GetMatrix(convertRMatrix);

                if (convertRMatrix.IsRigidSignedScale(deScaledMatrix, convertScale))
                    m_partScale = goopScale/convertScale; // Factor out uor scaling for V8 symbol scale...
                }
            }

        m_geomToWorld.InitFrom(rotation, origin);

        // Ensure that we have a transform that will result in a valid Placement2d if destination is 2d...
        FixupTransformForPlacement2d(m_geomToWorld);
        }
    else
        {
        // Just give up and apply full transform to geometry?!?
        m_goopTrans = m_v8ToDgnDbTrans;
        m_partScale = 0.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void FixupTransformForPlacement2d(TransformR transform)
    {
    if (m_is3dDest)
        return;

    DPoint3d            origin;
    RotMatrix           rMatrix;
    YawPitchRollAngles  angles;

    transform.GetMatrix(rMatrix);
    YawPitchRollAngles::TryFromRotMatrix(angles, rMatrix);

    // Placement2d will only use yaw...
    if (0.0 != angles.GetPitch().Degrees() || 0.0 != angles.GetRoll().Degrees())
        {
        YawPitchRollAngles tmpAngles(AngleInDegrees(), angles.GetPitch(), angles.GetRoll());
        transform = Transform::FromProduct(transform, tmpAngles.ToTransform(DPoint3d::FromZero()));
        }

    transform.GetTranslation(origin);
    origin.z = 0.0; // Make sure z is zero...
    transform.SetTranslation(origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricPrimitivePtr GetGeometry(DgnFileR dgnFile, Converter& converter, bool isPartCreate)
    {
    bool doFlatten = (!m_is3dDest && m_is3dSrc);

    if (!m_geometry.IsValid())
        {
        if (m_curve.IsValid())
            {
            BentleyApi::CurveVectorPtr clone;

            Converter::ConvertCurveVector(clone, *m_curve, &m_v8ToDgnDbTrans);
            m_geometry = GeometricPrimitive::Create(clone);
            }
        else if (m_primitive.IsValid())
            {
            if (!m_is3dDest) // Drop solids found in 3d sheet to wireframe...
                {
                Bentley::CurveVectorPtr wires = DgnV8Api::WireframeGeomUtil::CollectCurves(*m_primitive, true, true);

                if (wires.IsValid())
                    {
                    BentleyApi::CurveVectorPtr clone;

                    Converter::ConvertCurveVector(clone, *wires, nullptr); // m_v8ToDgnDbTrans only needed for spirals...
                    m_geometry = GeometricPrimitive::Create(clone);
                    doFlatten = true;
                    }
                }
            else
                {
                BentleyApi::ISolidPrimitivePtr clone;

                Converter::ConvertSolidPrimitive(clone, *m_primitive);
                m_geometry = GeometricPrimitive::Create(clone);
                }
            }
        else if (m_surface.IsValid())
            {
            if (!m_is3dDest) // Drop surfaces found in 3d sheet to wireframe...
                {
                Bentley::CurveVectorPtr wires = DgnV8Api::WireframeGeomUtil::CollectCurves(*m_surface, true, true);

                if (wires.IsValid())
                    {
                    BentleyApi::CurveVectorPtr clone;

                    Converter::ConvertCurveVector(clone, *wires, nullptr); // m_v8ToDgnDbTrans only needed for spirals...
                    m_geometry = GeometricPrimitive::Create(clone);
                    doFlatten = true;
                    }
                }
            else
                {
                BentleyApi::MSBsplineSurfacePtr clone;

                Converter::ConvertMSBsplineSurface(clone, *m_surface);
                m_geometry = GeometricPrimitive::Create(clone);
                }
            }
        else if (m_mesh.IsValid())
            {
            if (!m_is3dDest) // Drop mesh found in 3d sheet or 2d model to shapes, 2d mesh is "ok" in MicroStation but we don't want to support this in a 2d GeometryStream.
                {
                Bentley::PolyfaceVisitorPtr visitor = Bentley::PolyfaceVisitor::Attach(*m_mesh, true);

                visitor->SetNumWrap(1);

                Bentley::CurveVectorPtr facetShapes = Bentley::CurveVector::Create(Bentley::CurveVector::BOUNDARY_TYPE_UnionRegion);

                for (visitor->Reset(); visitor->AdvanceToNextFace(); )
                    {
                    Bentley::CurveVectorPtr facet = Bentley::CurveVector::CreateLinear(&visitor->Point().front(), visitor->Point().size(), Bentley::CurveVector::BOUNDARY_TYPE_Outer);

                    facetShapes->Add(facet);
                    }

                BentleyApi::CurveVectorPtr clone;

                Converter::ConvertCurveVector(clone, *facetShapes, nullptr); // m_v8ToDgnDbTrans only needed for spirals...
                m_geometry = GeometricPrimitive::Create(clone);
                doFlatten = true;
                }
            else
                {
                BentleyApi::PolyfaceHeaderPtr clone;

                Converter::ConvertPolyface(clone, *m_mesh);
                m_geometry = GeometricPrimitive::Create(clone);
#ifdef TEST_AUXDATA_TO_JSON
                Utf8String      jsonString;
                auto            jsonClone = IGeometry::Create(clone);

                jsonClone->TryTransformInPlace(m_v8ToDgnDbTrans);
                if (BentleyApi::IModelJson::TryGeometryToIModelJsonString (jsonString, *jsonClone))
                    {
                    FILE*       file = std::fopen("d:\\tmp\\RadialWave.json", "w");
                    fwrite (jsonString.c_str(), 1, jsonString.size(), file);
                    fclose(file);
                    }
#endif
                }
            }
        else if (m_brep.IsValid())
            {
            if (!m_is3dDest) // Drop surfaces found in 3d sheet to wireframe...
                {
                Bentley::CurveVectorPtr wires = DgnV8Api::WireframeGeomUtil::CollectCurves(*m_brep, true, true);

                if (wires.IsValid())
                    {
                    BentleyApi::CurveVectorPtr clone;

                    Converter::ConvertCurveVector(clone, *wires, nullptr); // m_v8ToDgnDbTrans only needed for spirals...
                    m_geometry = GeometricPrimitive::Create(clone);
                    doFlatten = true;
                    }
                }
            else
                {
#if defined (BENTLEYCONFIG_PARASOLID)
                IBRepEntityPtr clone;

                Converter::ConvertSolidKernelEntity(clone, *m_brep);

                if (clone.IsValid() && m_attachments.IsValid())
                    DgnDbApi::PSolidUtil::SetFaceAttachments(*clone, m_attachments.get());

                m_geometry = GeometricPrimitive::Create(clone);
#endif
                }
            }
        else if (m_text.IsValid())
            {
            TextStringPtr clone;

            Converter::ConvertTextString(clone, *m_text, dgnFile, converter);
            m_geometry = GeometricPrimitive::Create(clone);
            }
        else
            {
            BeAssert(false);
            }

        if (!m_geometry.IsValid())
            return nullptr;

        Transform goopTrans = m_goopTrans;

        if (isPartCreate && 0.0 != m_partScale && 1.0 != fabs(m_partScale))
            {
            double partScale = fabs(1.0/m_partScale);

            goopTrans.ScaleMatrixColumns(goopTrans, partScale, partScale, partScale);
            }

        if (!goopTrans.IsIdentity())
            {
            m_geometry->TransformInPlace(goopTrans);

            // NOTE: Information in GeometryParams may need to be transformed (ex. linestyles, gradient angle/scale, patterns, etc.)
            //       LineStyleParams only needs to be rotated for placement, the conversion scale has already been accounted for...
            m_geomParams.ApplyTransform(goopTrans, 0x01); // <- 0x01 is lazy/stealth way of specifying not to scale line style...
            }

        // Check 3D-to-2D and flatten the geometry here. Avoids Placement2d issues with geometry from 3d sheets with non-zero z values, etc.
        if (doFlatten)
            {
            Transform   flattenTrans;

            flattenTrans.InitIdentity();
            flattenTrans.form3d[2][2] = 0.0;

            m_geometry->TransformInPlace(flattenTrans);
            }
        }

    return m_geometry;
    }


    /*---------------------------------------------------------------------------------**//**
     * @bsimethod                                                    Vern.Francisco   01/18
     +---------------+---------------+---------------+---------------+---------------+------*/
    GeometricPrimitivePtr GetGeometry (DgnFileR dgnFile, LightWeightConverter& converter, bool isPartCreate)
        {
        bool doFlatten = (!m_is3dDest && m_is3dSrc);

        if (!m_geometry.IsValid ())
            {
            if (m_curve.IsValid ())
                {
                BentleyApi::CurveVectorPtr clone;

                Converter::ConvertCurveVector (clone, *m_curve, &m_v8ToDgnDbTrans);
                m_geometry = GeometricPrimitive::Create (clone);
                }
            else if (m_primitive.IsValid ())
                {
                if (!m_is3dDest) // Drop solids found in 3d sheet to wireframe...
                    {
                    Bentley::CurveVectorPtr wires = DgnV8Api::WireframeGeomUtil::CollectCurves (*m_primitive, true, true);

                    if (wires.IsValid ())
                        {
                        BentleyApi::CurveVectorPtr clone;

                        Converter::ConvertCurveVector (clone, *wires, nullptr); // m_v8ToDgnDbTrans only needed for spirals...
                        m_geometry = GeometricPrimitive::Create (clone);
                        doFlatten = true;
                        }
                    }
                else
                    {
                    BentleyApi::ISolidPrimitivePtr clone;

                    Converter::ConvertSolidPrimitive (clone, *m_primitive);
                    m_geometry = GeometricPrimitive::Create (clone);
                    }
                }
            else if (m_surface.IsValid ())
                {
                if (!m_is3dDest) // Drop surfaces found in 3d sheet to wireframe...
                    {
                    Bentley::CurveVectorPtr wires = DgnV8Api::WireframeGeomUtil::CollectCurves (*m_surface, true, true);

                    if (wires.IsValid ())
                        {
                        BentleyApi::CurveVectorPtr clone;

                        Converter::ConvertCurveVector (clone, *wires, nullptr); // m_v8ToDgnDbTrans only needed for spirals...
                        m_geometry = GeometricPrimitive::Create (clone);
                        doFlatten = true;
                        }
                    }
                else
                    {
                    BentleyApi::MSBsplineSurfacePtr clone;

                    Converter::ConvertMSBsplineSurface (clone, *m_surface);
                    m_geometry = GeometricPrimitive::Create (clone);
                    }
                }
            else if (m_mesh.IsValid ())
                {
                if (!m_is3dDest) // Drop mesh found in 3d sheet or 2d model to shapes, 2d mesh is "ok" in MicroStation but we don't want to support this in a 2d GeometryStream.
                    {
                    Bentley::PolyfaceVisitorPtr visitor = Bentley::PolyfaceVisitor::Attach (*m_mesh, true);

                    visitor->SetNumWrap (1);

                    Bentley::CurveVectorPtr facetShapes = Bentley::CurveVector::Create (Bentley::CurveVector::BOUNDARY_TYPE_UnionRegion);

                    for (visitor->Reset (); visitor->AdvanceToNextFace (); )
                        {
                        Bentley::CurveVectorPtr facet = Bentley::CurveVector::CreateLinear (&visitor->Point ().front (), visitor->Point ().size (), Bentley::CurveVector::BOUNDARY_TYPE_Outer);

                        facetShapes->Add (facet);
                        }

                    BentleyApi::CurveVectorPtr clone;

                    Converter::ConvertCurveVector (clone, *facetShapes, nullptr); // m_v8ToDgnDbTrans only needed for spirals...
                    m_geometry = GeometricPrimitive::Create (clone);
                    doFlatten = true;
                    }
                else
                    {
                    BentleyApi::PolyfaceHeaderPtr clone;

                    Converter::ConvertPolyface (clone, *m_mesh);
                    m_geometry = GeometricPrimitive::Create (clone);
#ifdef TEST_AUXDATA_TO_JSON
                    Utf8String      jsonString;
                    auto            jsonClone = IGeometry::Create (clone);

                    jsonClone->TryTransformInPlace (m_v8ToDgnDbTrans);
                    if (BentleyApi::IModelJson::TryGeometryToIModelJsonString (jsonString, *jsonClone))
                        {
                        FILE*       file = std::fopen ("d:\\tmp\\RadialWave.json", "w");
                        fwrite (jsonString.c_str (), 1, jsonString.size (), file);
                        fclose (file);
                        }
#endif
                    }
                }
            else if (m_brep.IsValid ())
                {
                if (!m_is3dDest) // Drop surfaces found in 3d sheet to wireframe...
                    {
                    Bentley::CurveVectorPtr wires = DgnV8Api::WireframeGeomUtil::CollectCurves (*m_brep, true, true);

                    if (wires.IsValid ())
                        {
                        BentleyApi::CurveVectorPtr clone;

                        Converter::ConvertCurveVector (clone, *wires, nullptr); // m_v8ToDgnDbTrans only needed for spirals...
                        m_geometry = GeometricPrimitive::Create (clone);
                        doFlatten = true;
                        }
                    }
                else
                    {
#if defined (BENTLEYCONFIG_PARASOLID)
                    IBRepEntityPtr clone;

                    Converter::ConvertSolidKernelEntity (clone, *m_brep);

                    if (clone.IsValid () && m_attachments.IsValid ())
                        DgnDbApi::PSolidUtil::SetFaceAttachments (*clone, m_attachments.get ());

                    m_geometry = GeometricPrimitive::Create (clone);
#endif
                    }
                }
            else if (m_text.IsValid ())
                {
                TextStringPtr clone;

                LightWeightConverter::ConvertTextString (clone, *m_text, dgnFile, converter);
                m_geometry = GeometricPrimitive::Create (clone);
                }
            else
                {
                BeAssert (false);
                }

            if (!m_geometry.IsValid ())
                return nullptr;

            Transform goopTrans = m_goopTrans;

            if (isPartCreate && 0.0 != m_partScale && 1.0 != fabs (m_partScale))
                {
                double partScale = fabs (1.0 / m_partScale);

                goopTrans.ScaleMatrixColumns (goopTrans, partScale, partScale, partScale);
                }

            if (!goopTrans.IsIdentity ())
                {
                m_geometry->TransformInPlace (goopTrans);

                // NOTE: Information in GeometryParams may need to be transformed (ex. linestyles, gradient angle/scale, patterns, etc.)
                //       LineStyleParams only needs to be rotated for placement, the conversion scale has already been accounted for...
                m_geomParams.ApplyTransform (goopTrans, 0x01); // <- 0x01 is lazy/stealth way of specifying not to scale line style...
                }

            // Check 3D-to-2D and flatten the geometry here. Avoids Placement2d issues with geometry from 3d sheets with non-zero z values, etc.
            if (doFlatten)
                {
                Transform   flattenTrans;

                flattenTrans.InitIdentity ();
                flattenTrans.form3d[2][2] = 0.0;

                m_geometry->TransformInPlace (flattenTrans);
                }
            }

        return m_geometry;
        }


}; // DgnV8PathEntry

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnV8PathGeom
{
Bentley::DisplayPathPtr m_path;
bvector<DgnV8PathEntry> m_entries;

DgnV8PathGeom(DgnV8Api::DisplayPath const* path) {m_path = path->Create(); m_path->SetPath(path);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void AddEntry(DgnV8PathEntry& entry)
    {
    size_t  insertIndex = m_entries.size();

    if (m_entries.size() > 1)
        {
        for (size_t i=insertIndex; i > 0; --i)
            {
            DgnV8PathEntry const& pathEntry = m_entries.at(i-1);

            if (pathEntry.m_geomParams.GetCategoryId() != entry.m_geomParams.GetCategoryId())
                continue; // Order by category to minimize number of elements...

            insertIndex = i;
            break;
            }
        }

    if (insertIndex < m_entries.size())
        {
        m_entries.insert(m_entries.begin()+insertIndex, entry);
        return;
        }

    m_entries.push_back(entry);
    }

}; // DgnV8PathGeom

//=======================================================================================
// @bsiclass
//=======================================================================================
struct DgnV8PartReference
{
DgnGeometryPartId       m_partId;
double                  m_scale;
Render::GeometryParams  m_geomParams;
Transform               m_geomToLocal;
DRange3d                m_localRange;
bool                    m_isLevelByCell;
bool                    m_isLevelValid;

};

static DgnV8Api::ElementRefAppData::Key s_dgnV8PartReferencetCacheKey;
//=======================================================================================
// @bsiclass                                                    Brien.Bastings  09/16
//=======================================================================================
struct DgnV8PartReferenceAppData : DgnV8Api::ElementRefAppData
{
protected:

virtual bool _OnElemChanged(Bentley::ElementRefP host, bool qvCacheDeleted, DgnV8Api::ElemRefChangeReason reason) override {return true;}
virtual void _OnCleanup(Bentley::ElementRefP host, bool unloadingCache, Bentley::HeapZone& zone) override {if (unloadingCache) return; zone.Free(this, sizeof *this);}

public:

    struct T_PartScaleKey
    {
    double m_scale;

    bool operator < (T_PartScaleKey const& rhs) const
        {
        if (DoubleOps::WithinTolerance(m_scale, rhs.m_scale, 1.0e-5))
            return false;

        return (m_scale < rhs.m_scale);
        }

    T_PartScaleKey() : m_scale(0.0) {}
    T_PartScaleKey(double scale) : m_scale(scale) {}
    };

    struct T_PartGeom
    {
    bvector<DgnV8PartReference> m_geomParts;
    bool m_filled = false;
    };

typedef bmap<T_PartScaleKey, T_PartGeom> T_ScaledPartGeom;
T_ScaledPartGeom m_map;

DgnV8PartReferenceAppData () {}

static T_PartGeom* GetCache(Bentley::ElementRefP elRef, double v8SymbolScale, bool allowCreate)
    {
    if (nullptr == elRef)
        return nullptr;

    DgnV8PartReferenceAppData* cache = (DgnV8PartReferenceAppData*) elRef->FindAppData(s_dgnV8PartReferencetCacheKey);

    if (nullptr == cache)
        {
        if (!allowCreate)
            return nullptr;

        Bentley::HeapZone& zone = elRef->GetHeapZone();

        cache = new ((DgnV8PartReferenceAppData*) zone.Alloc(sizeof (DgnV8PartReferenceAppData))) DgnV8PartReferenceAppData();

        if (SUCCESS != elRef->AddAppData(s_dgnV8PartReferencetCacheKey, cache, zone))
            {
            zone.Free(cache, sizeof (*cache));

            return nullptr;
            }
        }

    T_PartScaleKey partScale(v8SymbolScale);
    T_ScaledPartGeom::iterator found = cache->m_map.find(partScale);

    if (found == cache->m_map.end())
        {
        if (!allowCreate)
            return nullptr;

        T_PartGeom partGeom;

        partGeom.m_filled = false;
        cache->m_map[partScale] = partGeom;

        found = cache->m_map.find(partScale);
        }

    return &found->second;
    }

}; // DgnV8PartReferenceAppData

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  09/17
+===============+===============+===============+===============+===============+======*/
struct PostInstancePartCacheAppData: BeSQLite::Db::AppData
{
protected:

typedef bmap<DgnGeometryPartId, GeometricPrimitivePtr> T_PartIdToGeom;
T_PartIdToGeom m_map;

public:

static BeSQLite::Db::AppData::Key const& GetKey()
    {
    static BeSQLite::Db::AppData::Key s_key;
    return s_key;
    }

static GeometricPrimitivePtr GetPart(DgnGeometryPartId partId, DgnDbR db)
    {
    auto cache = db.ObtainAppData(GetKey(), []() { return new PostInstancePartCacheAppData(); });

    T_PartIdToGeom::iterator found = cache->m_map.find(partId);

    if (found == cache->m_map.end())
        {
        DgnGeometryPartCPtr partGeom = db.Elements().Get<DgnGeometryPart>(partId);
        GeometricPrimitivePtr geom;

        if (partGeom.IsValid())
            {
            GeometryCollection collection(partGeom->GetGeometryStream(), db);

            for (auto iter : collection)
                {
                GeometricPrimitivePtr thisGeom = iter.GetGeometryPtr();

                if (geom.IsValid())
                    {
                    geom = nullptr; // Post instance parts should only have a single geometric primitive...
                    break;
                    }

                geom = thisGeom;
                }
            }

        cache->m_map[partId] = geom;
        found = cache->m_map.find(partId);
        }

    return found->second;
    }

}; // PostInstancePartCacheAppData

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct V8GraphicsCollector : DgnV8Api::IElementGraphicsProcessor
{
private:

DgnV8Api::ViewContext*      m_context;
Bentley::Transform          m_conversionScale;
Bentley::Transform          m_currentTransform;
DgnV8Api::ElemDisplayParams m_currentDisplayParams;

Converter&                  m_converter;
DgnModel&                   m_model;
ResolvedModelMapping        m_v8mt;

bvector<DgnV8PathGeom>      m_v8PathGeom;
bool                        m_inBRepConvertFaces;

ElementConversionResults& m_results;

protected:

virtual DgnV8Api::DrawPurpose _GetDrawPurpose() override {return DgnV8Api::DrawPurpose::DgnDbConvert;} // Required so that xg symbols get pushed onto DisplayPath...

virtual bool _ProcessAsBody(bool isCurved) const override {return true;}
virtual bool _ProcessAsFacets(bool isPolyface) const override {return isPolyface;}
virtual void _AnnounceContext(DgnV8Api::ViewContext& context) override {m_context = &context;}
virtual void _AnnounceTransform(Bentley::TransformCP trans) override {if (trans) m_currentTransform = *trans; else m_currentTransform.InitIdentity();}
virtual void _AnnounceElemDisplayParams(DgnV8Api::ElemDisplayParams const& displayParams) override {m_currentDisplayParams = displayParams;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ExpandPatterns() const
    {
    // NOTE: Since we don't handle patterns on elements that don't support IAreaFillPropertiesQuery, just spew their graphics...
    DgnV8Api::DisplayPath const* path = m_context->GetCurrDisplayPath();
    Bentley::ElementRefP elRef = (nullptr != path ? path->GetCursorElem() : nullptr);

    if (nullptr == elRef)
        return false;

    DgnV8Api::ElementHandle v8Eh(elRef);
    DgnV8Api::IAreaFillPropertiesQuery* areaObj = dynamic_cast<DgnV8Api::IAreaFillPropertiesQuery*> (&v8Eh.GetHandler());

    if (nullptr != areaObj)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Bentley::BentleyStatus _ProcessCurveVector(Bentley::CurveVectorCR curves, bool isFilled) override
    {
    DgnV8PathGeom& pathGeom = GetDgnV8PathGeom();
    DgnV8PathEntry pathEntry(DoInterop(m_currentTransform), DoInterop(m_conversionScale), m_model.Is3d(), m_context->GetCurrentModel()->Is3d());

    m_converter.InitGeometryParams(pathEntry.m_geomParams, m_currentDisplayParams, *m_context, m_model.Is3d(), m_v8mt.GetV8Model());

    // NOTE: Unfortunately PatternParams isn't part of ElemDisplayParams in V8, so we can only check any
    //       element that output a region curve vector to see if it supports IAreaFillPropertiesQuery.
    //       There's no way to get at a PatternParams buried in an XGraphicsContainer either...but then
    //       MicroStation's pattern tools (ex. show pattern) don't work with these either...
    if (curves.IsAnyRegionType())
        {
        Bentley::ElementRefP elRef = pathGeom.m_path->GetCursorElem();

        if (nullptr != elRef)
            {
            DgnV8Api::ElementHandle v8Eh(elRef);
            DgnV8Api::IAreaFillPropertiesQuery* areaObj = dynamic_cast<DgnV8Api::IAreaFillPropertiesQuery*> (&v8Eh.GetHandler());

            if (nullptr != areaObj)
                {
                Bentley::PatternParamsPtr v8Pattern;
                Bentley::bvector<DgnV8Api::DwgHatchDefLine> v8DefLines;
                Bentley::DPoint3d origin;

                if (areaObj->GetPattern(v8Eh, v8Pattern, &v8DefLines, &origin, 0)) // Not going to worry about mline patterns...
                    {
                    PatternParamsPtr pattern = PatternParams::Create();

                    if (m_converter.InitPatternParams(*pattern, *v8Pattern, v8DefLines, origin, *m_context))
                        {
                        DgnV8Api::IAssocRegionQuery* assocRegion = dynamic_cast<DgnV8Api::IAssocRegionQuery*> (&v8Eh.GetHandler());

                        if (nullptr != assocRegion)
                            {
                            DgnV8Api::RegionParams regionParams;

                            if (SUCCESS == assocRegion->GetParams(v8Eh, regionParams))
                                {
                                DgnV8Api::ElementHandle templateEh;

                                DgnV8Api::ComplexHeaderDisplayHandler::GetComponentForDisplayParams(templateEh, v8Eh);
                                pattern->SetInvisibleBoundary(templateEh.IsValid() ? templateEh.GetElementCP()->hdr.dhdr.props.b.invisible : regionParams.GetInvisibleBoundary());
                                }
                            }

                        pathEntry.m_geomParams.SetPatternParams(pattern.get());
                        }
                    }
                }
            }
        }

    pathEntry.m_curve = curves.Clone();
    pathGeom.AddEntry(pathEntry);

    return Bentley::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Bentley::BentleyStatus _ProcessSolidPrimitive(Bentley::ISolidPrimitiveCR primitive) override
    {
    DgnV8PathGeom& pathGeom = GetDgnV8PathGeom();
    DgnV8PathEntry pathEntry(DoInterop(m_currentTransform), DoInterop(m_conversionScale), m_model.Is3d(), m_context->GetCurrentModel()->Is3d());

    m_converter.InitGeometryParams(pathEntry.m_geomParams, m_currentDisplayParams, *m_context, m_model.Is3d(), m_v8mt.GetV8Model());

    pathEntry.m_primitive = primitive.Clone();
    pathGeom.AddEntry(pathEntry);

    return Bentley::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Bentley::BentleyStatus _ProcessSurface(Bentley::MSBsplineSurfaceCR surface) override
    {
    DgnV8PathGeom& pathGeom = GetDgnV8PathGeom();
    DgnV8PathEntry pathEntry(DoInterop(m_currentTransform), DoInterop(m_conversionScale), m_model.Is3d(), m_context->GetCurrentModel()->Is3d());

    m_converter.InitGeometryParams(pathEntry.m_geomParams, m_currentDisplayParams, *m_context, m_model.Is3d(), m_v8mt.GetV8Model());

    pathEntry.m_surface = Bentley::MSBsplineSurface::CreatePtr();
    pathEntry.m_surface->CopyFrom(surface);
    pathGeom.AddEntry(pathEntry);

    return Bentley::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Bentley::BentleyStatus _ProcessFacets(Bentley::PolyfaceQueryCR meshData, bool isFilled) override
    {
    DgnV8PathGeom& pathGeom = GetDgnV8PathGeom();
    DgnV8PathEntry pathEntry(DoInterop(m_currentTransform), DoInterop(m_conversionScale), m_model.Is3d(), m_context->GetCurrentModel()->Is3d());

#ifdef TEST_AUXDATA
    getTestAuxData(meshData);
#endif

    m_converter.InitGeometryParams(pathEntry.m_geomParams, m_currentDisplayParams, *m_context, m_model.Is3d(), m_v8mt.GetV8Model());

    BentleyApi::RgbFactor const* rgbColors = (BentleyApi::RgbFactor const*) meshData.GetDoubleColorCP();
    uint32_t const* intColors = meshData.GetIntColorCP();
    uint32_t const* tableColors = meshData.GetColorTableCP();
    bvector<uint32_t> intColorsVec;

    // Color table indices aren't supported (and we don't need both RgbFactor and TBGR int color)...
    if (nullptr == intColors)
        {
        if (nullptr != rgbColors)
            {
            for (size_t iColor = 0; iColor < meshData.GetColorCount(); iColor++)
                intColorsVec.push_back(rgbColors[iColor].ToIntColor());

            intColors = &intColorsVec.front();
            }
        else if (nullptr != tableColors)
            {
            DgnFileR dgnFile = *m_context->GetCurrentModel()->GetDgnFileP();

            for (size_t iColor = 0; iColor < meshData.GetColorCount(); iColor++)
                {
                DgnV8Api::IntColorDef mappedColor(255, 255, 255); // Initialized to white...

                // Allegedly COLOR_BYCELL/COLOR_BYLEVEL are acceptable values, resolve effective value...
                if (DgnV8Api::COLOR_BYCELL == tableColors[iColor])
                    {
                    ElemHeaderOverridesCP ovr = m_context->GetHeaderOvr();

                    if (ovr)
                        DgnV8Api::DgnColorMap::ExtractElementColorInfo(&mappedColor, nullptr, nullptr, nullptr, nullptr, ovr->GetColor(), dgnFile);
                    }
                else if (DgnV8Api::COLOR_BYLEVEL == tableColors[iColor])
                    {
                    DgnV8Api::LevelHandle level = m_context->GetCurrentModel()->GetLevelCacheR().GetLevel(m_context->GetCurrentDisplayParams()->GetLevel());

                    if (level.IsValid())
                        {
                        DgnV8Api::LevelDefinitionColor colorDef = level.GetByLevelColor();
                        DgnV8Api::DgnColorMap::ExtractElementColorInfo(&mappedColor, nullptr, nullptr, nullptr, nullptr, colorDef.GetColor(), *colorDef.GetDefinitionFile());
                        }
                    }
                else
                    {
                    DgnV8Api::DgnColorMap::ExtractElementColorInfo(&mappedColor, nullptr, nullptr, nullptr, nullptr, tableColors[iColor], dgnFile);
                    }

                intColorsVec.push_back(mappedColor.m_int);
                }

            intColors = &intColorsVec.front();
            }
        }

    Bentley::PolyfaceQueryCarrier sourceData(meshData.GetNumPerFace(), meshData.GetTwoSided(), meshData.GetPointIndexCount(),
                                             meshData.GetPointCount(), meshData.GetPointCP(), meshData.GetPointIndexCP(),
                                             meshData.GetNormalCount(), meshData.GetNormalCP(), meshData.GetNormalIndexCP(),
                                             meshData.GetParamCount(), meshData.GetParamCP(), meshData.GetParamIndexCP(),
                                             meshData.GetColorCount(), meshData.GetColorIndexCP(), nullptr, nullptr, intColors, nullptr,
                                             meshData.GetIlluminationNameCP(), meshData.GetMeshStyle(), meshData.GetNumPerRow());

    pathEntry.m_mesh = Bentley::PolyfaceHeader::New();
    pathEntry.m_mesh->CopyFrom(sourceData);
    pathGeom.AddEntry(pathEntry);

    return Bentley::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ProcessLargeBody(Bentley::ISolidKernelEntityCR entity, Bentley::IFaceMaterialAttachmentsCP attachments)
    {
    // What is a good criteria for detecting a rediculous BRep??? A single body larger than 30 mb will certainly not work well...
    if (m_inBRepConvertFaces || DgnV8Api::PSolidUtil::DebugMemoryUsage(entity) < 30000000)
        return false;

    // Output each face, create curve vector for planar faces and sheet bodies for everything else. Dropping non-analytic faces isn't very robust and facets are meh...
    m_inBRepConvertFaces = true;
    DgnV8Api::DgnPlatformLib::GetHost().GetSolidsKernelAdmin()._OutputBodyAsSurfaces(entity, *m_context, false, attachments);
    m_inBRepConvertFaces = false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ProcessSingleBody(Bentley::ISolidKernelEntityCR entity, Bentley::IFaceMaterialAttachmentsCP attachments)
    {
    if (ProcessLargeBody(entity, attachments))
        {
        m_converter.ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Unsupported(), Converter::Issue::LargeBRep(), nullptr);
        return;
        }

    DgnV8PathGeom& pathGeom = GetDgnV8PathGeom();
    DgnV8PathEntry pathEntry(DoInterop(m_currentTransform), DoInterop(m_conversionScale), m_model.Is3d(), m_context->GetCurrentModel()->Is3d());

    m_converter.InitGeometryParams(pathEntry.m_geomParams, m_currentDisplayParams, *m_context, m_model.Is3d(), m_v8mt.GetV8Model());

    if (SUCCESS != DgnV8Api::DgnPlatformLib::GetHost().GetSolidsKernelAdmin()._CopyEntity(pathEntry.m_brep, entity))
        return;

    if (nullptr != attachments && !attachments->_GetFaceToSubElemIdMap().empty() && !attachments->_GetFaceAttachmentsMap().empty())
        {
        FaceAttachment attachment;

        if (!pathEntry.m_geomParams.IsLineColorFromSubCategoryAppearance())
            attachment.SetColor(pathEntry.m_geomParams.GetLineColor().GetValue(), pathEntry.m_geomParams.GetTransparency());

        if (!pathEntry.m_geomParams.IsMaterialFromSubCategoryAppearance())
            attachment.SetMaterial(pathEntry.m_geomParams.GetMaterialId());

        pathEntry.m_attachments = DgnDbApi::PSolidUtil::CreateNewFaceAttachments(DgnV8Api::PSolidUtil::GetEntityTag(*pathEntry.m_brep), attachment);

        if (!pathEntry.m_attachments.IsValid())
            {
            pathGeom.AddEntry(pathEntry);
            return;
            }

        DgnV8Api::T_FaceToSubElemIdMap const& faceToSubElemIdMapV8 = attachments->_GetFaceToSubElemIdMap();
        DgnV8Api::T_FaceAttachmentsMap const& faceAttachmentsMapV8 = attachments->_GetFaceAttachmentsMap();
        int32_t lowestFound = -1;

        // Normalize face index values that would have been adjusted for uniqueness when multiple bodies output by a single element...
        for (DgnV8Api::T_FaceToSubElemIdMap::const_iterator curr = faceToSubElemIdMapV8.begin(); curr != faceToSubElemIdMapV8.end(); ++curr)
            {
            if (-1 == lowestFound || curr->second < lowestFound)
                lowestFound = curr->second;
            }

        DgnDbApi::T_FaceAttachmentsVec const& faceAttachmentsVecDb = pathEntry.m_attachments->_GetFaceAttachmentsVec();
        bmap<int32_t, uint32_t> subElemIdToFaceMapDb; // Need fast reverse search, since body has been copied, face tags won't match...
        bvector<PK_FACE_t> faces;

        DgnDbApi::PSolidTopo::GetBodyFaces(faces, DgnV8Api::PSolidUtil::GetEntityTag(*pathEntry.m_brep));

        for (int iFace = 0; iFace < (int) faces.size(); iFace++)
            subElemIdToFaceMapDb[iFace + 1] = faces[iFace]; // subElemId is 1 based face index...

        DgnDbApi::PSolidAttrib::CreateFaceMaterialIndexAttributeDef(); // Create attribute definition if we haven't already, this attribute definition isn't setup by the V8 frustrum...

        for (DgnV8Api::T_FaceToSubElemIdMap::const_iterator curr = faceToSubElemIdMapV8.begin(); curr != faceToSubElemIdMapV8.end(); ++curr)
            {
            bmap <int32_t, uint32_t>::const_iterator foundIndexDb = subElemIdToFaceMapDb.find(curr->second - lowestFound + 1);

            if (foundIndexDb == subElemIdToFaceMapDb.end())
                continue;

            DgnV8Api::T_FaceAttachmentsMap::const_iterator foundFaceV8 = faceAttachmentsMapV8.find(curr->first);

            if (foundFaceV8 == faceAttachmentsMapV8.end())
                continue;

            DgnV8Api::ElemDisplayParams faceParamsV8(m_currentDisplayParams);
            Render::GeometryParams faceParamsDb;

            foundFaceV8->second.ToElemDisplayParams(faceParamsV8);
            m_converter.InitGeometryParams(faceParamsDb, faceParamsV8, *m_context, m_model.Is3d(), m_v8mt.GetV8Model());

            if (!faceParamsDb.IsLineColorFromSubCategoryAppearance())
                attachment.SetColor(faceParamsDb.GetLineColor().GetValue(), faceParamsDb.GetTransparency());

            if (!faceParamsDb.IsMaterialFromSubCategoryAppearance())
                attachment.SetMaterial(faceParamsDb.GetMaterialId());

            size_t attachmentIndex = 0;
            DgnDbApi::T_FaceAttachmentsVec::const_iterator foundAttachmentDb = std::find(faceAttachmentsVecDb.begin(), faceAttachmentsVecDb.end(), attachment);

            if (foundAttachmentDb == faceAttachmentsVecDb.end())
                {
                const_cast<DgnDbApi::T_FaceAttachmentsVec&>(faceAttachmentsVecDb).push_back(attachment);
                attachmentIndex = faceAttachmentsVecDb.size()-1;
                }
            else
                {
                attachmentIndex = std::distance(faceAttachmentsVecDb.begin(), foundAttachmentDb);
                }

            DgnDbApi::PSolidAttrib::SetFaceMaterialIndexAttribute(foundIndexDb->second, attachmentIndex); // Call with 0 will remove an existing attrib...avoid weird roundtrip scenario issues?
            }

        if (faceAttachmentsVecDb.size() < 2)
            pathEntry.m_attachments = nullptr; // ex. FeatureSolid w/uniform symbology, no reason to keep attachments.
        }

    pathGeom.AddEntry(pathEntry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Bentley::BentleyStatus _ProcessBody(Bentley::ISolidKernelEntityCR entity, Bentley::IFaceMaterialAttachmentsCP attachments)
    {
    // Prevent Polyface from BRep from picking up bogus fill since Polyface supports fill but BRep ignored it (ProStructures elements)...
    if (DgnV8Api::FillDisplay::Never != m_currentDisplayParams.GetFillDisplay())
        m_currentDisplayParams.SetFillDisplay(DgnV8Api::FillDisplay::Never);

    // NOTE: Don't separate regions when there are face attachments...
    if (nullptr == attachments && !m_inBRepConvertFaces)
        {
        // Split large disjoint bodies into separate regions (ex. STEP importer erroneously created these)...
        Bentley::bvector<DgnV8Api::ISolidKernelEntityPtr> out;

        if (SUCCESS == DgnV8Api::SolidUtil::DisjoinBody(out, entity))
            {
            for (DgnV8Api::ISolidKernelEntityPtr& thisEntity : out)
                {
                ProcessSingleBody(*thisEntity, nullptr);

                m_converter.ReportProgress();
                }

            return Bentley::SUCCESS;
            }
        }

    if (m_inBRepConvertFaces)
        m_converter.ReportProgress();

    ProcessSingleBody(entity, attachments);

    return Bentley::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
// Return ERROR to drop to geometry; SUCCESS to otherwise skip.
* @bsimethod                                                    BrienBastings   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Bentley::BentleyStatus _ProcessTextString(Bentley::TextStringCR v8Text)
    {
    /*
    DgnV8 and DgnDb TextString and glyph layout have substantial differences. Further, I do not want to drag the boat anchor of DgnV8 forward just for WYSIWYG initial display.
    However, one key thing that DgnDb TextString adds is the ability to cache and serialize glyph information, such that as display time, they are not re-computed.
    Thus, the strategy is to make a DgnDb TextString, but manually tweak its cached values such that it will look like DgnV8, but can still be read and understood as a DgnDb TextString.
    This means that on-edit, DgnDb rules will apply and the string may drastically change, but at least when first opened, it will display like DgnV8.
    Most differences between DgnV8 and DgnDb largely have to do with character spacing, vertical-ness, and DWG-ness, which can be captured by simply positioning glyphs as DgnV8 would dictate.
    DgnV8 also supports additional adornment (e.g. underline/overline) and fraction options, which must also be special-cased.
    */

    // DgnDb does not support symbol text elements, which do not have meaningful text content, and cannot be represented as Unicode. Drop to geometry to get fidelity.
    if (v8Text.GetProperties().GetFont().IsSymbolFont())
        return Bentley::ERROR;

    // No string? Then not worth converting...
    if (Bentley::WString::IsNullOrEmpty(v8Text.GetString()))
        return Bentley::SUCCESS;

    // 0.0 height or width? Then not worth converting... wouldn't draw, and a 0.0 height will cause a divide-by-zero since we persist width as a factor.
    if ((0.0 == v8Text.GetProperties().GetFontSize().x) || (0.0 == v8Text.GetProperties().GetFontSize().y))
        return Bentley::SUCCESS;

    DgnV8PathGeom& pathGeom = GetDgnV8PathGeom();
    DgnV8PathEntry pathEntry(DoInterop(m_currentTransform), DoInterop(m_conversionScale), m_model.Is3d(), m_context->GetCurrentModel()->Is3d());

    m_converter.InitGeometryParams(pathEntry.m_geomParams, m_currentDisplayParams, *m_context, m_model.Is3d(), m_v8mt.GetV8Model());

    Bentley::DPoint3d                  lowerLeft = v8Text.GetOrigin();
    Bentley::RotMatrix                 rMatrix = v8Text.GetRotMatrix();
    DgnV8Api::TextStringPropertiesPtr  props = v8Text.GetProperties().Clone();

    pathEntry.m_text = DgnV8Api::TextString::Create(v8Text.GetString(), &lowerLeft, &rMatrix, *props);
    pathGeom.AddEntry(pathEntry);

    // Guess whether it is necessary to explicitly output text background and adornments...
    DgnV8Api::DisplayPath const* path = m_context->GetCurrDisplayPath();

    if (nullptr != path)
        {
        DgnV8Api::ElementHandle v8Eh(path->GetCursorElem());
        DgnV8Api::ElementHandle::XAttributeIter iterator(v8Eh, DgnV8Api::XAttributeHandlerId(DgnV8Api::XATTRIBUTEID_XGraphics, DgnV8Api::XGraphicsMinorId_Data), 0);

        // Adornments/Background already will already be recored as geometry in XGraphics container...
        if (iterator.IsValid())
            return Bentley::SUCCESS;
        }

    // And then let DgnV8 emit its styled adornments directly as geometry since a DB TextString won't know how to draw them.
    v8Text.DrawTextAdornments(*m_context);
    v8Text.DrawTextBackground(*m_context);

    return Bentley::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8PathGeom& GetDgnV8PathGeom()
    {
    DgnV8Api::DisplayPath const* path = m_context->GetCurrDisplayPath();

    if (0 == m_v8PathGeom.size() || !m_v8PathGeom.back().m_path->IsSamePath(path, true))
        {
        DgnV8PathGeom pathGeom(path);

        m_v8PathGeom.push_back(pathGeom);
        }

    return m_v8PathGeom.back();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool GetBasisTransform(TransformR basisTransform, double& v8SymbolScale, Bentley::DgnPlatform::ElementHandle const& v8eh)
    {
    Bentley::Transform localToGeom;
    ConvertToDgnDbElementExtension* upx = ConvertToDgnDbElementExtension::Cast(v8eh.GetHandler());

    v8SymbolScale = 0.0;

    bool basisTransformSuppliedByExtension = false;
    if (nullptr != upx)
        basisTransformSuppliedByExtension = upx->_GetBasisTransform(localToGeom, v8eh, m_converter);
    for (auto xdomain : XDomainRegistry::s_xdomains)
        basisTransformSuppliedByExtension |= xdomain->_GetBasisTransform(localToGeom, v8eh, m_converter);

    if (!basisTransformSuppliedByExtension)
        {
        // NOTE: Attempt to preserve local coordinate system of V8 element for making geom parts.
        //       We can't just call DisplayHandler::GetBasisTransform as it doesn't return a good local
        //       frame in many cases but still returns SUCCESS. Check for known/usually ok cases.
        if (DgnV8Api::EXTENDED_ELM == v8eh.GetElementType() && DgnV8Api::XGraphicsContainer::IsPresent(v8eh))
            {
            // Using the XGraphics symbol transform will handle both XGraphic symbols and SmartFeatures.
            if (SUCCESS != DgnV8Api::XGraphicsContainer::ExtractTransform(localToGeom, v8eh))
                return false;

            Bentley::DVec3d      scaleVec;
            Bentley::DPoint3d    origin;
            Bentley::RotMatrix   rMatrix, invRMatrix;

            localToGeom.GetTranslation(origin);
            localToGeom.GetMatrix(rMatrix);

            if (!invRMatrix.InverseOf(rMatrix))
                return false;

            rMatrix.NormalizeColumnsOf(rMatrix, scaleVec);

            if (DoubleOps::AlmostEqual(scaleVec.x, scaleVec.y, 1.0e-5) && DoubleOps::AlmostEqual(scaleVec.x, scaleVec.z, 1.0e-5))
                v8SymbolScale = scaleVec.x;

            localToGeom.InitFrom(rMatrix, origin);
            }
        else
            {
            // Shared cells and user defined cells should have a good basis...
            DgnV8Api::ICellQuery* cellQuery;

            if (nullptr == (cellQuery = dynamic_cast <DgnV8Api::ICellQuery*> (&v8eh.GetHandler())))
                return false; // Not a cell, shared cell instance, or shared cell def...

            if (cellQuery->IsNormalCell(v8eh) && cellQuery->IsAnonymous(v8eh) && !cellQuery->IsPointCell(v8eh))
                return false; // Not a user defined cell, edit->group does not set a useful orientation...

            if (!v8eh.GetDisplayHandler()->GetBasisTransform(v8eh, localToGeom))
                return false;

            // Basis transform is expected to be normalized...need to get V8 cell scale directly...
            Bentley::DVec3d cellScale;

            if (SUCCESS == cellQuery->ExtractScale(cellScale, v8eh))
                {
                double minScale = DoubleOps::Min(cellScale.x, cellScale.y, cellScale.z);
                double maxScale = DoubleOps::Max(cellScale.x, cellScale.y, cellScale.z);

                if (DoubleOps::AlmostEqual(minScale, maxScale, 1.0e-5))
                    v8SymbolScale = minScale;
                }
            }
        }

    localToGeom = Bentley::Transform::FromProduct(m_conversionScale, localToGeom); // Apply unit scaling...

    // NOTE: Ensure rotation is squared up and normalized...
    DPoint3d    origin;
    RotMatrix   rMatrix;

    localToGeom.GetTranslation(DoInterop(origin));
    localToGeom.GetMatrix(DoInterop(rMatrix));

    if (rMatrix.Determinant() < 0.0)
        v8SymbolScale *= -1.0;

    rMatrix.SquareAndNormalizeColumns(rMatrix, 0, 1);

    // Don't use basis transform from element if it can't be represented by Placement2d (conversion of 3d sheet geometry to 2d)...
    if (!m_model.Is3d())
        {
        YawPitchRollAngles  angles;

        YawPitchRollAngles::TryFromRotMatrix(angles, rMatrix);

        if (0 != BeNumerical::Compare(origin.z, 0.0) || 0.0 != angles.GetPitch().Degrees() || 0.0 != angles.GetRoll().Degrees())
            return false;
        }

    basisTransform.InitFrom(rMatrix, origin);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  01/16
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String GetElementLabel(Bentley::DgnPlatform::ElementHandle const& v8eh)
    {
    DgnV8Api::ICellQuery* cellQuery;

    if (nullptr == (cellQuery = dynamic_cast <DgnV8Api::ICellQuery*> (&v8eh.GetHandler())))
        return ""; // Not a cell, shared cell instance, or shared cell def...

    WChar strBuffer[Bentley::DgnPlatform::MAX_CELLNAME_LENGTH];

    if (SUCCESS != cellQuery->ExtractName(strBuffer, Bentley::DgnPlatform::MAX_CELLNAME_LENGTH, v8eh))
        return "";

    Utf8String str;

    str.Assign(strBuffer);

    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void BuildNewElement(Dgn::GeometryBuilderPtr& builder, DgnClassId elementClassId, DgnCategoryId targetCategoryId, DgnCode elementCode, DgnV8EhCR v8eh)
    {
    // NOTE: Make sure we always have element for parent (ex. cell with ec instance on all children)...will have invalid builder in this case.
    DgnCategoryId categoryId = (builder.IsValid() ? builder->GetGeometryParams().GetCategoryId() : targetCategoryId);

    if (!m_results.m_element.IsValid() && categoryId == targetCategoryId)
        {
        DgnCode         parentCode = elementCode;
        Utf8String      parentLabel = GetElementLabel(v8eh);

        if (!parentCode.IsValid() && m_converter.WantDebugCodes())
            parentCode = m_converter.CreateDebuggingCode(v8eh);

        DgnElementPtr   gel = Converter::CreateNewElement(m_model, elementClassId, categoryId, parentCode, parentLabel.c_str());

        if (gel.IsValid() && (!builder.IsValid() || SUCCESS == builder->Finish(*gel->ToGeometrySourceP())))
            m_results.m_element = gel.get();

        builder = nullptr;
        return;
        }

    if (!builder.IsValid())
        return;

    DgnClassId      childClassId;
    DgnCode         childCode;

    for (auto xdomain : XDomainRegistry::s_xdomains)
        xdomain->_DetermineElementParams(childClassId, childCode, categoryId, v8eh, m_converter, nullptr, m_v8mt);

    if (!childClassId.IsValid())
        childClassId = m_converter.ComputeElementClassIgnoringEcContent(v8eh, m_v8mt);

    if (!childCode.IsValid() && m_converter.WantDebugCodes())
        childCode = m_converter.CreateDebuggingCode(v8eh);

    DgnElementPtr   gel = Converter::CreateNewElement(m_model, childClassId, categoryId, childCode);

    if (gel.IsValid() && SUCCESS == builder->Finish(*gel->ToGeometrySourceP()))
        {
        ElementConversionResults resultsForChild;
        resultsForChild.m_element = gel.get();
        m_results.m_childElements.push_back(resultsForChild);
        }

    builder = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::ElementRefP GetCurrentInstanceElement(DgnV8Api::DisplayPath const& path)
    {
    Bentley::ElementRefP elRef = path.GetCursorElem();

    if (DgnV8Api::XGraphicsContainer::IsXGraphicsSymbol(elRef))
        return elRef;

    if (!elRef->IsComplexComponent())
        return nullptr;

    Bentley::ElementRefP parentRef = elRef;

    do
        {
        if (DgnV8Api::SHAREDCELL_DEF_ELM == parentRef->GetElementType())
            return elRef;

        } while (nullptr != (parentRef = parentRef->GetParentElementRef()));

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::ElementRefP GetOutermostSCDef(DgnV8Api::DisplayPath const& path)
    {
    for (int iPath = 0; iPath < path.GetCount(); ++iPath)
        {
        Bentley::ElementRefP pathElRef = path.GetPathElem(iPath);

        if (DgnV8Api::SHAREDCELL_DEF_ELM == pathElRef->GetElementType())
            return pathElRef;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   05/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GetPartTag(Converter& converter, Bentley::ElementRefP instanceElRef, Utf8CP prefix, uint32_t sequenceNo, double partScale)
    {
    BeAssert(sequenceNo > 0); // First entry starts at 1...

    if (sequenceNo > 1)
        { // Note: partScale < 0.0 implies "mirrored" which means a different codeValue must be generated
        return Utf8PrintfString(partScale < 0.0 ? "%s-%lld-M%lld-%d" : "%s-%lld-%lld-%d", prefix, converter.GetRepositoryLinkId(*instanceElRef->GetDgnModelP()->GetDgnFileP()).GetValue(), instanceElRef->GetElementId(), sequenceNo-1);
        }

    return Utf8PrintfString(partScale < 0.0 ? "%s-%lld-M%lld" : "%s-%lld-%lld", prefix, converter.GetRepositoryLinkId(*instanceElRef->GetDgnModelP()->GetDgnFileP()).GetValue(), instanceElRef->GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GetPartReferences(bvector<DgnV8PartReference>& geomParts, double v8SymbolScale, DgnV8EhCR v8eh)
    {
    switch (v8eh.GetElementType())
        {
        case DgnV8Api::SHARED_CELL_ELM:
            {
            DgnV8PartReferenceAppData::T_PartGeom* partCache = nullptr;

            if (nullptr != (partCache = DgnV8PartReferenceAppData::GetCache(DgnV8Api::CellUtil::FindSharedCellDefinition(*v8eh.GetElementCP(), *v8eh.GetDgnFileP()), v8SymbolScale, false))) // Don't create...
                geomParts = partCache->m_geomParts;
            break;
            }

        case DgnV8Api::EXTENDED_ELM:
            {
            DgnV8Api::DependencyLinkage const* depLinkage;

            if (SUCCESS != DgnV8Api::DependencyManagerLinkage::GetLinkage(&depLinkage, v8eh, 10000/*DEPENDENCYAPPID_MicroStation*/, DEPENDENCYAPPVALUE_XGraphicsSymbol))
                break;

            for (int iSymbol = 0; iSymbol < depLinkage->nRoots; ++iSymbol)
                {
                Bentley::ElementRefP symbolElRef = v8eh.GetDgnModelP()->FindByElementId(depLinkage->root.elemid[iSymbol]);

                if (nullptr == symbolElRef)
                    continue;

                DgnV8PartReferenceAppData::T_PartGeom* partCache = nullptr;

                if (nullptr == (partCache = DgnV8PartReferenceAppData::GetCache(symbolElRef, v8SymbolScale, false))) // Don't create...
                    {
                    geomParts.clear(); // All symbols must have a cache, a partial cache isn't useful...
                    break;
                    }

                if (geomParts.empty())
                    geomParts = partCache->m_geomParts;
                else
                    geomParts.insert(geomParts.end(), partCache->m_geomParts.begin(), partCache->m_geomParts.end());
                }
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void CreatePartReferences(bvector<DgnV8PartReference>& geomParts, TransformCR basisTransform, double v8SymbolScale)
    {
    for (DgnV8PathGeom& pathGeom : m_v8PathGeom)
        {
        for (DgnV8PathEntry& pathEntry : pathGeom.m_entries)
            {
            if (0.0 == pathEntry.m_partScale)
                return; // Not suitable for creating a part, ex. non-uniform scale...

            // NOTE: Part instancing with stroked linestyles isn't supported for display...
            if (pathEntry.m_curve.IsValid() && pathEntry.m_geomParams.HasStrokedLineStyle())
                {
                // Apply would-be part transform directly to linestyle params...
                if (!DoubleOps::AlmostEqual(1.0, pathEntry.m_partScale, 1.0e-5))
                    const_cast<Render::LineStyleInfoR> (*pathEntry.m_geomParams.GetLineStyle()).GetStyleParamsR().ApplyTransform(Transform::FromScaleFactors(pathEntry.m_partScale, pathEntry.m_partScale, pathEntry.m_partScale));
                return;
                }
            }
        }

    int         iEntry = 0;
    Transform   invBasisTrans;

    invBasisTrans.InverseOf(basisTransform);

    // NOTE: For a shared cell instance, all geometry (including that from nested instances) is cached
    //       on the outermost shared cell definition. XGraphics symbols don't nest, so we can just
    //       cache the geometry for each symbol.
    DgnV8PartReferenceAppData::T_PartGeom* partCache = nullptr;

    for (DgnV8PathGeom& pathGeom : m_v8PathGeom)
        {
        Bentley::ElementRefP instanceElRef = GetCurrentInstanceElement(*pathGeom.m_path);

        if (nullptr == instanceElRef)
            {
            BeAssert(geomParts.empty()); // If one entry is from a V8 instance, then all entries should be...
            geomParts.clear();
            return;
            }

        Bentley::ElementRefP scDefElRef = GetOutermostSCDef(*pathGeom.m_path);

        if (nullptr == partCache)
            partCache = DgnV8PartReferenceAppData::GetCache(nullptr != scDefElRef ? scDefElRef : instanceElRef, v8SymbolScale, true); // Create if doesn't exist...

        uint32_t sequenceNo = 0;
        DgnFileP dgnFile = pathGeom.m_path->GetRoot()->GetDgnFileP();

        for (DgnV8PathEntry& pathEntry : pathGeom.m_entries)
            {
            sequenceNo++; // Always increment, even if geometry conversion fails...

            if (0 == (++iEntry % 10))
                m_converter.ReportProgress();

            Transform         geomToLocal = Transform::FromProduct(invBasisTrans, pathEntry.m_geomToWorld);
            Utf8String        partTag = GetPartTag(m_converter, instanceElRef, nullptr == scDefElRef ? "XGSymbV8" : "SCDefV8", sequenceNo, pathEntry.m_partScale);
            DgnGeometryPartId partId = m_converter.QueryGeometryPartId(partTag);
            DRange3d          localRange = DRange3d::NullRange();

            if (!partId.IsValid())
                {
                GeometricPrimitivePtr geometry = pathEntry.GetGeometry(*dgnFile, m_converter, true);

                if (!geometry.IsValid())
                    continue;

                GeometryBuilderPtr  partBuilder = GeometryBuilder::CreateGeometryPart(m_model.GetDgnDb(), m_model.Is3d());

                partBuilder->Append(*geometry);

                DgnGeometryPartPtr  geomPart = DgnGeometryPart::Create(*(m_converter.GetJobDefinitionModel()));

                if (SUCCESS == partBuilder->Finish(*geomPart) && m_model.GetDgnDb().Elements().Insert<DgnGeometryPart>(*geomPart).IsValid())
                    {
                    partId = geomPart->GetId();
                    m_converter.RecordGeometryPartId(partId, partTag);
                    localRange = geomPart->GetBoundingBox();
                    }
                }
            else
                {
                if (SUCCESS != DgnGeometryPart::QueryGeometryPartRange(localRange, m_model.GetDgnDb(), partId))
                    partId = DgnGeometryPartId(); // Shouldn't happen, we know part exists...
                }

            if (!partId.IsValid())
                {
                BeAssert(false); // Why couldn't part be created???
                continue;
                }

            DgnV8PartReference partRef;

            partRef.m_partId = partId;
            partRef.m_scale = pathEntry.m_partScale;
            partRef.m_geomParams = pathEntry.m_geomParams;
            partRef.m_geomToLocal = geomToLocal;
            partRef.m_localRange = localRange;
            partRef.m_isLevelByCell = DgnV8Api::LEVEL_BYCELL == pathGeom.m_path->GetCursorElem()->GetUnstableMSElementCP()->ehdr.level;
            partRef.m_isLevelValid = pathGeom.m_path->GetCursorElem()->GetUnstableMSElementCP()->ehdr.isGraphics;

            geomParts.push_back(partRef);

            if (nullptr != partCache && !partCache->m_filled)
                partCache->m_geomParts.push_back(partRef);
            }

        if (nullptr != partCache && nullptr == scDefElRef)
            {
            partCache->m_filled = true;
            partCache = nullptr;
            }
        }

    if (nullptr != partCache)
        partCache->m_filled = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsValidForPostInstancing(GeometricPrimitiveR geometry, DgnV8Api::DisplayPath const& path)
    {
    if (!m_model.Is3d())
        return false;

    // NOTE: Determine if geometry is worth instancing.
    //       Always reject text. Instancing may have undesirable ramifications for editing, etc.
    //       Always reject line/point strings! The CurveVector round trips as a CurvePrimitive from GeometryCollection so the type check always fails!!!
    switch (geometry.GetGeometryType())
        {
        case GeometricPrimitive::GeometryType::SolidPrimitive:
        case GeometricPrimitive::GeometryType::BsplineSurface:
        case GeometricPrimitive::GeometryType::Polyface:
        case GeometricPrimitive::GeometryType::BRepEntity:
            break;

        default:
            return false;
        }

    Bentley::ElementRefP elRef = path.GetCursorElem();

    if (elRef->IsComplexComponent())
        elRef = elRef->GetOutermostParentOrSelf();

    // Normal cell components are good candidates for post-instancing...and it shouldn't be a huge problem if we create a part and don't find other instances...
    if (DgnV8Api::CELL_HEADER_ELM == elRef->GetElementType() && !elRef->GetUnstableMSElementCP()->hdr.dhdr.props.b.h)
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsMatchingPartGeometry(DgnGeometryPartId partId, DgnDbR db, GeometricPrimitiveR geometry)
    {
    GeometricPrimitivePtr partGeometry = PostInstancePartCacheAppData::GetPart(partId, db);

    if (!partGeometry.IsValid() || !partGeometry->IsSameStructureAndGeometry(geometry, 1.0e-5))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void PostInstanceGeometry(Dgn::GeometryBuilderR builder, GeometricPrimitiveR geometry, Render::GeometryParamsR params, uint32_t sequenceNo, Bentley::ElementRefP instanceElRef)
    {
    DRange3d localRange;
    DgnGeometryPartId partId;

    if (geometry.GetRange(localRange))
        {
        auto range = m_converter.GetRangePartIdMap().equal_range(PartRangeKey(localRange));

        for (auto iter = range.first; iter != range.second; ++iter)
            {
            if (IsMatchingPartGeometry(iter->second, m_model.GetDgnDb(), geometry))
                {
                partId = iter->second;
                break;
                }
            }
        }

    if (!partId.IsValid())
        {
        Utf8String         partTag = GetPartTag(m_converter, instanceElRef, "CvtV8", sequenceNo, 1.0);
        DgnGeometryPartPtr geomPart = DgnGeometryPart::Create(*(m_converter.GetJobDefinitionModel()));
        GeometryBuilderPtr partBuilder = GeometryBuilder::CreateGeometryPart(m_model.GetDgnDb(), true);

        partBuilder->Append(geometry);

        if (SUCCESS == partBuilder->Finish(*geomPart))
            {
            bool isValidInstance = (GeometricPrimitive::GeometryType::BRepEntity != geometry.GetGeometryType());

            // NOTE: Don't create instance for a bad/failed brep. Polyface/CurveVector from part will never match so we'd end up with singleton parts...
            if (!isValidInstance)
                {
                GeometryCollection collection(geomPart->GetGeometryStream(), m_model.GetDgnDb());

                for (auto iter : collection)
                    {
                    if (GeometryCollection::Iterator::EntryType::BRepEntity != iter.GetEntryType())
                        continue;

                    isValidInstance = true;
                    break;
                    }
                }

            if (isValidInstance && m_model.GetDgnDb().Elements().Insert<DgnGeometryPart>(*geomPart).IsValid())
                {
                m_converter.GetRangePartIdMap().insert(Converter::RangePartIdMap::value_type(PartRangeKey(geomPart->GetBoundingBox()), partId = geomPart->GetId()));
                m_converter.RecordGeometryPartId(geomPart->GetId(), partTag);
                }
            }
        }

    builder.Append(params);

    // If part couldn't be created (ex. same XGraphicsSymbol with different non-uniform scales) add non-instanced geometry to builder...
    if (partId.IsValid())
        builder.Append(partId, Transform::FromIdentity());
    else
        builder.Append(geometry);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplySharedCellInstanceOverrides(DgnV8EhCR v8eh, Render::GeometryParamsR geomParams)
    {
    if (DgnV8Api::SHARED_CELL_ELM != v8eh.GetElementType())
        return;

    DgnV8Api::SCOverride scOvr = v8eh.GetElementCP()->sharedCell.m_override;

    if (0 == *((UInt16*) (&scOvr)))
        return;

    if (scOvr.level)
        {
        DgnV8Api::LevelId v8Level = v8eh.GetElementCP()->hdr.ehdr.level;

        if (0 != v8Level && DgnV8Api::LEVEL_BYCELL != v8Level)
            {
            DgnSubCategoryId subCategoryId = m_converter.GetSyncInfo().GetSubCategory(v8Level, m_v8mt.GetV8Model(), m_model.Is3d() ? SyncInfo::LevelExternalSourceAspect::Type::Spatial: SyncInfo::LevelExternalSourceAspect::Type::Drawing);
            DgnCategoryId categoryId = DgnSubCategory::QueryCategoryId(m_converter.GetDgnDb(), subCategoryId);

            geomParams.SetCategoryId(categoryId, false); // <- Don't clear appearance overrides...
            geomParams.SetSubCategoryId(subCategoryId, false);
            }
        }

    if (scOvr.color)
        {
        DgnV8Api::IntColorDef intColorDef;

        if (SUCCESS == DgnV8Api::DgnColorMap::ExtractElementColorInfo(&intColorDef, nullptr, nullptr, nullptr, nullptr, v8eh.GetElementCP()->hdr.dhdr.symb.color, *v8eh.GetDgnFileP()))
            {
            geomParams.SetLineColor(ColorDef(intColorDef.m_int));
            geomParams.SetFillColor(ColorDef(intColorDef.m_int));
            }
        }

    if (scOvr.weight)
        {
        UInt32  v8Weight = v8eh.GetElementCP()->hdr.dhdr.symb.weight;

        if (DgnV8Api::WEIGHT_BYCELL != v8Weight && DgnV8Api::WEIGHT_BYLEVEL != v8Weight)
            geomParams.SetWeight(v8Weight);
        }

    if (scOvr.style)
        {
        Int32   v8Style = v8eh.GetElementCP()->hdr.dhdr.symb.style;

        if (DgnV8Api::STYLE_BYCELL != v8Style && DgnV8Api::STYLE_BYLEVEL != v8Style)
            {
            double      unitsScale;
            DgnStyleId  mappedStyleId = m_converter._RemapLineStyle(unitsScale, *v8eh.GetDgnFileP(), v8Style, true);

            if (mappedStyleId.IsValid())
                {
                LineStyleInfoPtr lsInfo = LineStyleInfo::Create(mappedStyleId, nullptr);

                geomParams.SetLineStyle(lsInfo.get());
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/17
+---------------+---------------+---------------+---------------+---------------+------*/
struct IsMarkedEntry
    {
    IsMarkedEntry() {}
    bool operator()(DgnV8PathEntry& entry) const {return entry.m_isMarked;}
    }; // IsMarkedEntry

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DoGeometrySimplification()
    {
    for (DgnV8PathGeom& pathGeom : m_v8PathGeom)
        {
        if (pathGeom.m_entries.size() < 50)
            continue; // Leave "small" amounts of geometry alone...

        size_t convexShapeCount = 0;
        DgnV8PathEntry* firstPathEntry = nullptr;

        for (DgnV8PathEntry& pathEntry : pathGeom.m_entries)
            {
            if (!pathEntry.m_is3dSrc || !pathEntry.m_is3dDest)
                return; // If either source or destination is 2d, leave geometry alone...

            if (!pathEntry.m_curve.IsValid())
                continue;

            if (!pathEntry.m_curve->IsClosedPath() || pathEntry.m_curve->ContainsNonLinearPrimitive())
                continue; // Leave anything that isn't potentially just a shape alone...

            pathEntry.m_curve->ConsolidateAdjacentPrimitives(false); // Don't remove co-linear points...

            if (Bentley::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString != pathEntry.m_curve->HasSingleCurvePrimitive())
                continue;

            if (Render::FillDisplay::Never != pathEntry.m_geomParams.GetFillDisplay() || nullptr != pathEntry.m_geomParams.GetPatternParams())
                continue; // Don't create mesh from filled/patterned shapes...

            Bentley::bvector<Bentley::DPoint3d> const* v8Points = pathEntry.m_curve->front()->GetLineStringCP();

            // NOTE: Bentley::PolygonOps::IsConvex implementation didn't exist, just the declaration...
            if (!bsiGeom_testPolygonConvex(&v8Points->front(), (int) v8Points->size()))
                continue;

            if (nullptr == firstPathEntry)
                firstPathEntry = &pathEntry;
            else if (!Converter::IsTransformEqualWithTolerance(firstPathEntry->m_v8ToDgnDbTrans,pathEntry.m_v8ToDgnDbTrans))
                continue; // Require same V8->DgnDb transform...
            else if (!firstPathEntry->m_geomParams.IsEquivalent(pathEntry.m_geomParams))
                continue; // Require same symbology...

            pathEntry.m_isMarked = true;
            convexShapeCount++;
            }

        if (convexShapeCount < 25)
            continue;

        Bentley::IFacetOptionsPtr facetOptions = Bentley::IFacetOptions::Create();
        Bentley::IPolyfaceConstructionPtr polyBuilder = Bentley::IPolyfaceConstruction::Create(*facetOptions);

        for (DgnV8PathEntry& pathEntry : pathGeom.m_entries)
            {
            if (!pathEntry.m_isMarked)
                continue;

            polyBuilder->AddTriangulation(*(const_cast<Bentley::bvector<Bentley::DPoint3d>*> (pathEntry.m_curve->front()->GetLineStringCP())));
            }

        Bentley::PolyfaceHeaderPtr mesh = polyBuilder->GetClientMeshPtr();

        if (!mesh.IsValid())
            continue;

        firstPathEntry->m_mesh = mesh;
        firstPathEntry->m_curve = nullptr;
        firstPathEntry->m_isMarked = false;

        pathGeom.m_entries.erase(std::remove_if(pathGeom.m_entries.begin(), pathGeom.m_entries.end(), IsMarkedEntry()), pathGeom.m_entries.end());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool IgnorePublicChildren(DgnV8EhCR v8eh)
    {
    ConvertToDgnDbElementExtension* upx = ConvertToDgnDbElementExtension::Cast(v8eh.GetHandler());

    if (nullptr != upx && upx->_IgnorePublicChildren())
        return true;
    for (auto xdomain : XDomainRegistry::s_xdomains)
        {
        if (xdomain->_IgnorePublicChildren())
            return true;
        }
    return DgnV8Api::CellUtil::IsPointCell(v8eh); // Don't create assemblies for point cells, they are meant to have uniform level and symbology...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisablePostInstancing(DgnV8EhCR v8eh)
    {
    ConvertToDgnDbElementExtension* upx = ConvertToDgnDbElementExtension::Cast(v8eh.GetHandler());

    if (nullptr != upx && upx->_DisablePostInstancing())
        return true;

    for (auto xdomain : XDomainRegistry::s_xdomains)
        {
        if (xdomain->_DisablePostInstancing())
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool AppendAsSubGraphics(DgnV8PathGeom& currPathGeom, DgnV8PathEntry& currPathEntry)
    {
    bool    done = false;
    bool    found = false;
    size_t  geom3dCount = 0;
    size_t  entryCount = 0;

    for (DgnV8PathGeom& pathGeom : m_v8PathGeom)
        {
        if (!found && (&pathGeom != &currPathGeom))
            continue;

        for (DgnV8PathEntry& pathEntry : pathGeom.m_entries)
            {
            if (&pathEntry == &currPathEntry)
                found = true;

            if (!found)
                continue;

            // Since entries are grouped by category...can stop looking when category changes...
            if (done = (pathEntry.m_geomParams.GetCategoryId() != currPathEntry.m_geomParams.GetCategoryId()))
                break;

            // NOTE: Don't create a sub-graphic for every geometric primitive from a dimension or dropped pattern...
            if (DgnGeometryClass::Pattern == pathEntry.m_geomParams.GetGeometryClass() || DgnGeometryClass::Dimension == pathEntry.m_geomParams.GetGeometryClass())
                continue;

            // If we have potentially expensive 3d geometry to process, always include sub-graphic range...
            if (pathEntry.m_primitive.IsValid() || pathEntry.m_surface.IsValid() || pathEntry.m_mesh.IsValid() || pathEntry.m_brep.IsValid())
                {
                if (++geom3dCount > 1)
                    return true;
                }

            if (++entryCount < 25)
                continue;

            return true;
            }

        if (done)
            break;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsValidGraphicElement(DgnV8EhCR v8eh)
    {
    DgnV8Api::MSElement const& v8el = *v8eh.GetElementCP();

    // Skip displayable elements marked invisible as well as non-graphic components of cells (ex. type 38/39)...
    if (!v8el.ehdr.isGraphics || v8el.hdr.dhdr.props.b.invisible)
        {
        static size_t s_nReported;
        Utf8String info;
        ++s_nReported;
        if (s_nReported <= 1000)
            info = Utf8PrintfString("Ignoring element %s because it is %s.", Converter::IssueReporter::FmtElement(v8eh).c_str(), !v8el.ehdr.isGraphics ? "non-graphical" : "invisible");
        else if (s_nReported == 1001)
            info = "More than 1000 elements have been ignored, either because they are invisible or non-graphical.";
        if (!info.empty())
            m_converter.ReportIssue(Converter::IssueSeverity::Info, Converter::IssueCategory::Sync(), Converter::Issue::Message(), info.c_str());
        return false;
        }

    // Skip elements with invalid ranges, they would not have displayed in V8 and likely contain bad/corrupt data...
    Bentley::DRange3d range;
    if (!DgnV8Api::DataConvert::CheckedScanRangeToDRange3d(range, v8el.hdr.dhdr.range, v8el.hdr.dhdr.props.b.is3d))
        return false;

    if (!v8el.hdr.dhdr.props.b.is3d)
        range.low.z = range.high.z = 0.0;

    if (range.IsEmpty())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsViewIndependent(DgnV8EhCR v8eh)
    {
    switch (v8eh.GetElementType())
        {
        case DgnV8Api::TEXT_ELM:
        case DgnV8Api::TEXT_NODE_ELM:
            return v8eh.GetElementCP()->hdr.dhdr.props.b.r; // Text only checks relative bit unlike cells that also check snappable...
        default:
            return DgnV8Api::CellUtil::IsPointCell(v8eh);
        }
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ProcessElement(DgnClassId elementClassId, bool hasV8PrimaryECInstance, DgnCategoryId targetCategoryId, DgnCode elementCode, DgnV8EhCR v8eh)
    {
    if (!IsValidGraphicElement(v8eh))
        return;

    bool        isValidBasis = false;
    bool        isValidBasisAndScale = false;
    bool        isViewIndependent = false;
    bool        haveSCOverrides = false;
    double      v8SymbolScale = 0.0;
    Transform   basisTransform;

    bvector<DgnV8PartReference> geomParts;
    DgnV8Api::ChildElemIter v8childEh(v8eh, DgnV8Api::ExposeChildrenReason::Query);

    if (v8childEh.IsValid() && !IgnorePublicChildren(v8eh))
        {
        for (; v8childEh.IsValid(); v8childEh = v8childEh.ToNext())
            {
            ElementConversionResults childResults;

            auto catid = m_converter.GetSyncInfo().GetCategory(v8childEh, m_v8mt);
            if (BentleyStatus::SUCCESS != m_converter.ConvertElement(childResults, v8childEh, m_v8mt, catid, false) || !childResults.m_element.IsValid())
                continue; // Conversion of component with ECInstances did not produce any geometry, ignore it...

            m_results.m_childElements.push_back(childResults);
            }
        }
    else
        {
        isValidBasis = GetBasisTransform(basisTransform, v8SymbolScale, v8eh);
        isValidBasisAndScale = (isValidBasis && 0.0 != v8SymbolScale);
        isViewIndependent = (isValidBasis && IsViewIndependent(v8eh));

        if (isValidBasisAndScale)
            GetPartReferences(geomParts, v8SymbolScale, v8eh);

        if (geomParts.empty())
            {
            if (isValidBasisAndScale && DgnV8Api::SHARED_CELL_ELM == v8eh.GetElementType() && 0 != *((UInt16*) (&v8eh.GetElementCP()->sharedCell.m_override)))
                {
                DgnV8Api::EditElementHandle v8ehNoOvr(v8eh.GetElementCP(), v8eh.GetModelRef());
                DgnV8Api::SCOverride* scOvr = &v8ehNoOvr.GetElementP()->sharedCell.m_override;

                // Make sure part cache has geometry params that aren't affected by SCOverride...
                scOvr->level = scOvr->color = scOvr->style = scOvr->weight = false;
                DgnV8Api::ElementGraphicsOutput::Process(v8ehNoOvr, *this);
                haveSCOverrides = true;
                }
            else
                {
                DgnV8Api::ElementGraphicsOutput::Process(v8eh, *this);
                }

            DoGeometrySimplification();
            }
        }

    if (isValidBasisAndScale && !m_v8PathGeom.empty())
        CreatePartReferences(geomParts, basisTransform, v8SymbolScale);

    GeometryBuilderPtr builder;

    if (!geomParts.empty())
        {
        // NOTE: Level from shared cell instance that does not have a level override is
        //       meaningless except for components that use DgnV8Api::LEVEL_BYCELL.
        //       Getting a valid level to pass to ProcessElement would be expensive, so detect
        //       this situation and choose the category from the first part as necessary.
        //       This avoids creating un-necessary assemblies/parent element w/o Geometry...
        //       Purposely doesn't check if target is "GetUncategorizedCategory" as instance level
        //       doesn't have to be 0 (ex. relative level nonsense, etc.)
        bool targetCategoryValid = false;
        DgnCategoryId lastCategoryId = targetCategoryId;

        for (DgnV8PartReference& partRef : geomParts)
            {
            Transform geomToLocal = partRef.m_geomToLocal;
            Render::GeometryParams geomParams = partRef.m_geomParams;

            if (1.0 != fabs(partRef.m_scale))
                {
                double partScale = fabs(partRef.m_scale);

                geomToLocal.ScaleMatrixColumns(geomToLocal, partScale, partScale, partScale);
                }

            // Apply SCOverride now for a shared cell that was deemed ok for a GeometryPart..
            ApplySharedCellInstanceOverrides(v8eh, geomParams);

            if (partRef.m_isLevelValid && !partRef.m_isLevelByCell)
                lastCategoryId = geomParams.GetCategoryId();

            if (!targetCategoryValid && targetCategoryId == lastCategoryId)
                targetCategoryValid = true;

            // Need to ignore cached category for an XGraphic symbol, 107 elements don't have a valid level and inherit from parent 106, which means it can change...
            if (!partRef.m_isLevelValid && targetCategoryId != geomParams.GetCategoryId())
                geomParams.SetCategoryId(targetCategoryId, false); // Preserve appearance overrides...

            // If category changes, need to create a new element/assembly...
            if (builder.IsValid() && builder->GetGeometryParams().GetCategoryId() != lastCategoryId)
                BuildNewElement(builder, elementClassId, targetCategoryId, elementCode, v8eh);

            if (!builder.IsValid()) {
                builder = GeometryBuilder::Create(m_model, lastCategoryId, basisTransform);
                if (isViewIndependent)
                    builder->SetHeaderFlags(GeometryStreamIO::Header::Flags::ViewIndependent);
            }

            builder->Append(geomParams);
            builder->Append(partRef.m_partId, geomToLocal, partRef.m_localRange);
            }

        // NOTE: Don't need an extra call to BuildNewElement with invalid builder to ensure we have a parent because of targetCategoryValid check...
        BuildNewElement(builder, elementClassId, targetCategoryValid ? targetCategoryId : lastCategoryId, elementCode, v8eh);
        return;
        }

    uint32_t    iEntry = 0;
    Transform   firstLocalToGeom = Transform::FromIdentity();
    Transform   firstGeomToWorld = Transform::FromIdentity();

    for (DgnV8PathGeom& pathGeom : m_v8PathGeom)
        {
        uint32_t iPathEntry = 0;
        DgnFileP dgnFile = pathGeom.m_path->GetRoot()->GetDgnFileP();

        for (DgnV8PathEntry& pathEntry : pathGeom.m_entries)
            {
            iPathEntry++;

            if (0 == (++iEntry % 10))
                m_converter.ReportProgress();

#ifdef TEST_AUXDATA_FILE
            GeometricPrimitivePtr geometry = createFromTestFile();
#else
            GeometricPrimitivePtr geometry = pathEntry.GetGeometry(*dgnFile, m_converter, false);
#endif

            if (!geometry.IsValid())
                continue;

            // Apply SCOverride now for a shared cell that was rejected for a GeometryPart...
            if (haveSCOverrides)
                ApplySharedCellInstanceOverrides(v8eh, pathEntry.m_geomParams);

            // If category changes or GeometryStream is getting un-wieldy, need to create a new element/assembly...
            if (builder.IsValid() && (builder->GetGeometryParams().GetCategoryId() != pathEntry.m_geomParams.GetCategoryId() || builder->GetCurrentSize() > 20000000))
                BuildNewElement(builder, elementClassId, targetCategoryId, elementCode, v8eh);

            Transform   localToGeom;

            if (!builder.IsValid())
                {
                Transform   localToWorld;

                if (isValidBasis)
                    {
                    Transform   invBasisTrans, geomToLocal;

                    invBasisTrans.InverseOf(basisTransform);
                    geomToLocal = Transform::FromProduct(invBasisTrans, pathEntry.m_geomToWorld);
                    localToGeom.InverseOf(geomToLocal);
                    localToWorld = basisTransform; // Basis is the placement we want to use...
                    }
                else
                    {
                    geometry->GetLocalCoordinateFrame(localToGeom);

                    // Ensure that we have a transform that will result in a valid Placement2d if destination is 2d...
                    pathEntry.FixupTransformForPlacement2d(localToGeom);

                    localToWorld = Transform::FromProduct(pathEntry.m_geomToWorld, localToGeom);
                    }

                firstGeomToWorld = pathEntry.m_geomToWorld;
                firstLocalToGeom = localToGeom;

                builder = GeometryBuilder::Create(m_model, pathEntry.m_geomParams.GetCategoryId(), localToWorld);

                if (!builder.IsValid())
                    {
//                  BeAssert(false); // Bad placement for 2d, invalid categoryId???
                    continue;
                    }

                // NOTE: Don't create large multi-primitive GeometryStreams without calling SetAppendAsSubGraphics to store sub-ranges...
                if (AppendAsSubGraphics(pathGeom, pathEntry))
                    builder->SetAppendAsSubGraphics();

                if (isViewIndependent)
                    builder->SetHeaderFlags(GeometryStreamIO::Header::Flags::ViewIndependent);
                }
            else
                {
                // First piece of geometry added to builder established the frame transform...
                if (!firstGeomToWorld.IsEqual(pathEntry.m_geomToWorld))
                    {
                    Transform   worldToFirstGeom, geomToFirstGeom, firstGeomToFirstLocal, geomToFirstLocal;

                    worldToFirstGeom.InverseOf(firstGeomToWorld);
                    geomToFirstGeom = Transform::FromProduct(worldToFirstGeom, pathEntry.m_geomToWorld);
                    firstGeomToFirstLocal.InverseOf(firstLocalToGeom);
                    geomToFirstLocal = Transform::FromProduct(firstGeomToFirstLocal, geomToFirstGeom);
                    localToGeom.InverseOf(geomToFirstLocal);
                    }
                else
                    {
                    localToGeom = firstLocalToGeom;
                    }
                }

            if (!localToGeom.IsIdentity())
                {
                Transform geomToLocal;

                geomToLocal.InverseOf(localToGeom);
                geometry->TransformInPlace(geomToLocal);
                pathEntry.m_geomParams.ApplyTransform(geomToLocal, 0x01); // <- Don't scale linestyles...
                }

            if (IsValidForPostInstancing(*geometry, *pathGeom.m_path) && !DisablePostInstancing(v8eh))
                {
                // Create parts for geometry that wasn't instanced in V8 but was deemed worth instancing...
                PostInstanceGeometry(*builder, *geometry, pathEntry.m_geomParams, iPathEntry, pathGeom.m_path->GetCursorElem());
                }
            else
                {
                builder->Append(pathEntry.m_geomParams);
                builder->Append(*geometry);
                }
            }
        }

    if (builder.IsValid())
        BuildNewElement(builder, elementClassId, targetCategoryId, elementCode, v8eh);

    // Make sure we have parent element by calling BuildNewElement with invalid builder...
    BuildNewElement(builder, elementClassId, targetCategoryId, elementCode, v8eh);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
V8GraphicsCollector(ElementConversionResults& results, Converter& converter, ResolvedModelMapping const& v8mm) :
                    m_results(results), m_converter(converter), m_model(v8mm.GetDgnModel()), m_v8mt(v8mm),
                    m_context(nullptr), m_conversionScale(DoInterop(v8mm.GetTransform())), m_inBRepConvertFaces(false) {}

}; // V8GraphicsCollector

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   12/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Converter::_CreateElementAndGeom(ElementConversionResults& results, ResolvedModelMapping const& v8mm, DgnClassId elementClassId,
                                               bool hasV8PrimaryECInstance, DgnCategoryId targetCategoryId, DgnCode elementCode, DgnV8EhCR v8eh)
    {
    V8GraphicsCollector collector(results, *this, v8mm);

    collector.ProcessElement(elementClassId, hasV8PrimaryECInstance, targetCategoryId, elementCode, v8eh);

    return results.m_element.IsValid()? BSISUCCESS: BSIERROR;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  8/2016
//----------------------------------------------------------------------------------------
BentleyApi::CurveVectorPtr Converter::ConvertV8Curve(Bentley::CurveVectorCR v8Curves, TransformCP v8ToDgnDbTrans)
    {
    BentleyApi::CurveVectorPtr curve;
    ConvertCurveVector(curve, v8Curves, v8ToDgnDbTrans);
    return curve;
    }


//--------------------------------------------------------------------------------------
//! The following code is a clone of the method used in the Converter class. These
//! were cloned to support the LightWeightConverter which deals only with graphics
//! convertion. The EC and SyncInfo is handled at the application level.
//!
// @bsimethod                                                    Vern.Francisco   12/17
//---------------+---------------+---------------+---------------+---------------+------


//---------------------------------------------------------------------------------
// @bsimethod                                                    Vern.Francisco   12/17
//---------------+---------------+---------------+---------------+---------------+------

void LightWeightConverter::InitGeometryParams (Render::GeometryParams& params, DgnV8Api::ElemDisplayParams& paramsV8, DgnV8Api::ViewContext& context, bool is3d)
    {
    // NOTE: Resolve loses information like WEIGHT_BYLEVEL and we may be called multiple times (ex. disjoint brep)...
    AutoRestore<DgnV8Api::ElemDisplayParams> saveParamsV8 (&paramsV8);

    UInt32  rawColor = paramsV8.GetLineColor ();
    UInt32  rawFill = paramsV8.GetFillColor ();
    UInt32  rawWeight = paramsV8.GetWeight ();
    Int32   rawStyle = paramsV8.GetLineStyle ();

    // Apply ignores and resolve effective now that we've saved off the ByLevel information...
    paramsV8.Resolve (context);

    // NOTE: ElemHeaderOverrides have been pushed but have not been applied to paramsV8...
    DgnV8Api::LevelId v8Level = (0 == paramsV8.GetLevel () ? DGNV8_LEVEL_DEFAULT_LEVEL_ID : paramsV8.GetLevel ());
    DgnV8Api::ElemHeaderOverrides const* ovr = context.GetHeaderOvr ();

    if (nullptr != ovr && ovr->GetFlags ().level)
        v8Level = ovr->AdjustLevel (v8Level, *context.GetCurrentModel ());

    DgnSubCategoryId subCategoryId = params.GetSubCategoryId (); // see if the caller wants to specify an override SubCategoryId
    if (!subCategoryId.IsValid ())
        subCategoryId = GetSubCategory (v8Level, is3d ? SyncInfo::LevelExternalSourceAspect::Type::Spatial : SyncInfo::LevelExternalSourceAspect::Type::Drawing);

    DgnCategoryId categoryId = DgnSubCategory::QueryCategoryId (GetDgnDb (), subCategoryId);

    params = Render::GeometryParams ();
    params.SetCategoryId (categoryId);
    params.SetSubCategoryId (subCategoryId);

    if (!is3d)
        {
        if (DgnV8Api::DISPLAYPRIORITY_BYCELL != paramsV8.GetElementDisplayPriority ())
            params.SetDisplayPriority (paramsV8.GetElementDisplayPriority ());
        else if (nullptr != ovr)
            params.SetDisplayPriority (ovr->GetDisplayPriority ());
        }

    switch (paramsV8.GetElementClass ())
        {
        case DgnV8Api::DgnElementClass::Primary:
        case DgnV8Api::DgnElementClass::PrimaryRule: // Shouldn't see this normally...
            params.SetGeometryClass (Render::DgnGeometryClass::Primary);
            break;

        case DgnV8Api::DgnElementClass::Construction:
        case DgnV8Api::DgnElementClass::ConstructionRule: // Shouldn't see this normally...
            params.SetGeometryClass (Render::DgnGeometryClass::Construction);
            break;

        case DgnV8Api::DgnElementClass::Dimension:
            params.SetGeometryClass (Render::DgnGeometryClass::Dimension);
            break;

        case DgnV8Api::DgnElementClass::PatternComponent:
            params.SetGeometryClass (Render::DgnGeometryClass::Pattern);
            break;

        case DgnV8Api::DgnElementClass::LinearPatterned: // Don't intend to support this anymore...
        default:
            params.SetGeometryClass (Render::DgnGeometryClass::Primary);
            break;
        }

    if (nullptr != ovr && ovr->GetFlags ().color)
        {
        if (DgnV8Api::COLOR_BYLEVEL != ovr->GetColor ())
            {
            DgnV8Api::IntColorDef intColorDef;

            if (SUCCESS == DgnV8Api::DgnColorMap::ExtractElementColorInfo (&intColorDef, nullptr, nullptr, nullptr, nullptr, ovr->GetColor (), *context.GetCurrentModel ()->GetDgnFileP ()))
                params.SetLineColor (ColorDef (intColorDef.m_int));
            }
        }
    else if (DgnV8Api::COLOR_BYLEVEL != rawColor)
        {
        params.SetLineColor (ColorDef (paramsV8.GetLineColorTBGR ()));
        }

    if (nullptr != ovr && ovr->GetFlags ().weight)
        {
        if (DgnV8Api::WEIGHT_BYLEVEL != ovr->GetWeight ())
            params.SetWeight (ovr->GetWeight ());
        }
    else if (DgnV8Api::WEIGHT_BYLEVEL != rawWeight)
        {
        params.SetWeight (paramsV8.GetWeight ());
        }

    DgnModelRefP styleModelRef;

    if (nullptr != ovr && ovr->GetFlags ().style && nullptr != (styleModelRef = (nullptr == ovr->GetLineStyleModelRef ()) ? context.GetCurrentModel () : ovr->GetLineStyleModelRef ()))
        {
        if (DgnV8Api::STYLE_BYLEVEL != ovr->GetLineStyle () && ovr->GetLineStyle () != 0)
            {
            InitLineStyle (params, *styleModelRef, ovr->GetLineStyle (), ovr->GetLineStyleParams ());
            }
        }
    else if (paramsV8.GetLineStyle () != 0 && nullptr != (styleModelRef = (nullptr == paramsV8.GetLineStyleModelRef ()) ? context.GetCurrentModel () : paramsV8.GetLineStyleModelRef ()))
        {
        DgnV8Api::LineStyleParams const* lsParamsV8 = paramsV8.GetLineStyleParams ();

        // Ugh...still need modifiers like start/end width for STYLE_BYLEVEL... :(
        if (DgnV8Api::STYLE_BYLEVEL != rawStyle || (lsParamsV8 && 0 != lsParamsV8->modifiers))
            InitLineStyle (params, *styleModelRef, paramsV8.GetLineStyle (), lsParamsV8);
        }
    else if (DgnV8Api::STYLE_BYLEVEL != rawStyle)
        {
        params.SetLineStyle (nullptr);
        }

    params.SetTransparency (paramsV8.GetTransparency ());

    if (DgnV8Api::FillDisplay::Never != paramsV8.GetFillDisplay ())
        {
        params.SetFillDisplay ((Render::FillDisplay) paramsV8.GetFillDisplay ());

        if (nullptr != paramsV8.GetGradient ())
            {
            DgnV8Api::GradientSymb const& gradient = *paramsV8.GetGradient ();
            Render::GradientSymbPtr gradientPtr = Render::GradientSymb::Create ();

            gradientPtr->SetMode ((Render::GradientSymb::Mode) gradient.GetMode ());
            gradientPtr->SetFlags ((Render::GradientSymb::Flags) gradient.GetFlags ());
            gradientPtr->SetShift (gradient.GetShift ());
            gradientPtr->SetTint (gradient.GetTint ());
            gradientPtr->SetAngle (gradient.GetAngle ());

            bvector<ColorDef> keyColors;
            bvector<double>   keyValues;

            for (int i = 0; i < gradient.GetNKeys (); i++)
                {
                double keyValue;
                Bentley::RgbColorDef keyColor;

                gradient.GetKey (keyColor, keyValue, i);

                DgnDbApi::ColorDef keyColorDef (keyColor.red, keyColor.green, keyColor.blue);

                keyColors.push_back (keyColorDef);
                keyValues.push_back (keyValue);
                }

            gradientPtr->SetKeys ((uint16_t)keyColors.size (), &keyColors.front (), &keyValues.front ());

            params.SetGradient (gradientPtr.get ());
            }
        else
            {
            if (nullptr != ovr && ovr->GetFlags ().color)
                {
                if (DgnV8Api::COLOR_BYLEVEL != ovr->GetColor ())
                    {
                    DgnV8Api::IntColorDef intColorDef;

                    if (SUCCESS == DgnV8Api::DgnColorMap::ExtractElementColorInfo (&intColorDef, nullptr, nullptr, nullptr, nullptr, ovr->GetColor (), *context.GetCurrentModel ()->GetDgnFileP ()))
                        params.SetFillColor (ColorDef (intColorDef.m_int));
                    }
                }
            else if (DgnV8Api::DgnColorMap::INDEX_Background == rawFill)
                {
                params.SetFillColorFromViewBackground (DgnV8Api::DgnColorMap::INDEX_Background != rawColor);
                }
            else if (DgnV8Api::COLOR_BYLEVEL != rawFill)
                {
                params.SetFillColor (ColorDef (paramsV8.GetFillColorTBGR ()));
                }
            }
        }

    if (paramsV8.IsRenderable ())
        {
        if (nullptr != paramsV8.GetMaterial ())
            {
            RenderMaterialId materialId = GetRemappedMaterial (paramsV8.GetMaterial ());
            DgnSubCategoryCPtr          dgnDbSubCategory = DgnSubCategory::Get (GetDgnDb (), subCategoryId);

            if (!dgnDbSubCategory.IsValid () || dgnDbSubCategory->GetAppearance ().GetRenderMaterial () != materialId)
                params.SetMaterialId (materialId);

            if (nullptr != paramsV8.GetMaterialUVDetailP ())
                {
                // params.SetMaterialUVDetails();
                }
            }
        }

    params.Resolve (GetDgnDb ()); // Need to be able to check for a stroked linestyle...
    }




//=================================================================================
//* @bsiclass
//*
//* This struct is a copy of the V8SymbolGraphicsCollector with references to Converter
//* and SyncInfo removed. This was needed by the Plant Group to support multi-client
//* graphics sync-ing to a briefcase.
//*
//*                                                                Vern.Francisco 11-2017
//+===============+===============+===============+===============+===============+======
struct LightWeightV8SymbolGraphicsCollector : DgnV8Api::IElementGraphicsProcessor
    {
    private:

        DgnV8Api::ViewContext*          m_context;
        Bentley::Transform              m_conversionScale;
        Bentley::Transform              m_currentTransform;
        DgnV8Api::ElemDisplayParams     m_currentDisplayParams;
        DgnCategoryId                   m_defaultCategoryId;
        DgnSubCategoryId                m_defaultSubCategoryId;
        DgnDbPtr                        m_dgnDb;
        bvector<GeometricPrimitivePtr>  m_symbolGeometry;
        bvector<Render::GeometryParams> m_symbolParams;
        bool                            m_allowMultiSymb = false;
        LightWeightConverter&           m_converter;

    protected:

        virtual DgnV8Api::DrawPurpose _GetDrawPurpose() override { return DgnV8Api::DrawPurpose::DgnDbConvert; } // Required to avoid resolving ByLevel...

        virtual bool _ProcessAsBody(bool isCurved) const override { return false; }
        virtual bool _ProcessAsFacets(bool isPolyface) const override { return false; }
        virtual void _AnnounceContext(DgnV8Api::ViewContext& context) override { m_context = &context; }
        virtual void _AnnounceTransform(Bentley::TransformCP trans) override { if (trans) m_currentTransform = *trans; else m_currentTransform.InitIdentity(); }
        virtual void _AnnounceElemDisplayParams(DgnV8Api::ElemDisplayParams const& displayParams) override { m_currentDisplayParams = displayParams; }

        //---------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual Bentley::BentleyStatus _ProcessCurveVector(Bentley::CurveVectorCR curves, bool isFilled) override
            {
            BentleyApi::CurveVectorPtr clone;

            Transform v8ToDgnDbTrans = DoInterop(Bentley::Transform::FromProduct(m_conversionScale, m_currentTransform));

            Converter::ConvertCurveVector(clone, curves, &v8ToDgnDbTrans);

            GeometricPrimitivePtr elemGeom = GeometricPrimitive::Create(clone);

            AddSymbolGeometry(elemGeom);

            return Bentley::SUCCESS;
            }

        //---------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        virtual Bentley::BentleyStatus _ProcessTextString(Bentley::TextStringCR v8Text)
            {
            return Bentley::ERROR; // Is there any reason we shouldn't always drop symbol text?
            }

        //---------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        void AddSymbolGeometry(GeometricPrimitivePtr& geometry)
            {
            Transform   v8ToDgnDbTrans = DoInterop(Bentley::Transform::FromProduct(m_conversionScale, m_currentTransform));

            geometry->TransformInPlace(v8ToDgnDbTrans);
            m_symbolGeometry.push_back(geometry);

            // NOTE: Establish symbol symbology using first drawn when multi-symbology symbol isn't supported...
            if (0 != m_symbolParams.size() && !m_allowMultiSymb)
                return;

            Render::GeometryParams geomParams;

            m_converter.InitGeometryParams(geomParams, m_currentDisplayParams, *m_context, true);
            m_symbolParams.push_back(geomParams);
            }

    public:

        //---------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        LightWeightV8SymbolGraphicsCollector(LightWeightConverter& converter, DgnDbPtr dgnDb, BentleyApi::TransformCR conversionScale, DgnCategoryId defaultCategory, DgnSubCategoryId defaultSubCategory) : m_context(nullptr), m_conversionScale(DoInterop(conversionScale)), m_converter(converter)
            {
            m_dgnDb = dgnDb;
            m_defaultCategoryId = defaultCategory;
            m_defaultSubCategoryId = defaultSubCategory;
            }

        //---------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        bvector<GeometricPrimitivePtr>& GetSymbolGeometry() { return m_symbolGeometry; }
        bvector<Render::GeometryParams>& GetSymbolDisplayParams() { return m_symbolParams; }
        void SetAllowMultiSymb() { m_allowMultiSymb = true; }

        //---------------------------------------------------------------------------------------
        //* @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+------
        void ProcessSymbol(DgnV8Api::IDisplaySymbol& symbol, DgnV8ModelR model) { DgnV8Api::ElementGraphicsOutput::Process(symbol, model, *this); }

        void InitLineStyle(Render::GeometryParams& params, DgnModelRefR styleModelRef, int32_t srcLineStyleNum, DgnV8Api::LineStyleParams const* v8lsParams);

    }; // LightWeightV8SymbolGraphicsCollector


//---------------------------------------------------------------------------------------
//* @bsimethod                                                    Vern.Francisco 11/2017
//*
//* This method was copied from the Converter Class and references to the Sync-Info were
//* removed.
//*
//+---------------+---------------+---------------+---------------+---------------+------
void LightWeightV8SymbolGraphicsCollector::InitLineStyle(Render::GeometryParams& params, DgnModelRefR styleModelRef, int32_t srcLineStyleNum, DgnV8Api::LineStyleParams const* v8lsParams)
    {
    DgnFileP styleFile = styleModelRef.GetDgnFileP();
    if (nullptr == styleFile)
        {
        return;
        }

    double          unitsScale;

    DgnStyleId mappedStyleId;

    double modelLsScale = styleModelRef.GetRoot()->GetLineStyleScale();
    DgnV8Api::ModelInfo const& v8ModelInfo = styleModelRef.GetRoot()->GetModelInfo();
    double uorPerMeter = DgnV8Api::ModelInfo::GetUorPerMeter(&v8ModelInfo);
    Render::LineStyleParams lsParams;
    LightWeightConverter::ConvertLineStyleParams(lsParams, v8lsParams, uorPerMeter, 1.0 /*unitsScale Need new method*/, modelLsScale);
    LineStyleInfoPtr lsInfo = LineStyleInfo::Create(mappedStyleId, &lsParams);
    params.SetLineStyle(lsInfo.get());
    }


//-------------------------------------------------------------------------------------
//* @bsimethod                                                    Vern.Francisco 11/2017
//*
//* This method was copied from the LineStyleConverter Class and references to the Sync-Info were
//* removed.
//*
//+---------------+---------------+---------------+---------------+---------------+------
LineStyleStatus LightWeightLineStyleConverter::ConvertPointSymbol(LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheSymbolComponent const& symbolComponent, double lsScale)
    {
    v10Id = LsComponentId();

    DisplayPointSymbol  displaySymbol(symbolComponent);
    Transform  trans;
    trans.InitFromScaleFactors(lsScale, lsScale, lsScale);

    LightWeightV8SymbolGraphicsCollector collector( dynamic_cast<LightWeightConverter&>(m_converter), m_dgnDb, trans /* v8uors_to_mm */, m_defaultCategoryId, m_defaultSubCategoryId);

    collector.SetAllowMultiSymb(); // Symbols can contain color, weight, and line code changes...
    collector.ProcessSymbol(displaySymbol, v8File.GetDictionaryModel());

    size_t iSymb = 0;
    bvector<GeometricPrimitivePtr>& geometry = collector.GetSymbolGeometry();
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(GetDgnDb(), true);

    DRange3d totalRange;
    totalRange.Init();

    for (GeometricPrimitivePtr elemGeom : geometry)
        {
        DRange3d localRange;

        elemGeom->GetRange(localRange, nullptr);
        totalRange.Extend(localRange);

        builder->Append(collector.GetSymbolDisplayParams().at(iSymb++));
        builder->Append(*elemGeom);
        }

    DgnGeometryPartPtr geomPart = DgnGeometryPart::Create(GetDgnDb().GetDictionaryModel());
    if (SUCCESS != builder->Finish(*geomPart))
        return LINESTYLE_STATUS_ConvertingComponent;

    GetDgnDb().Elements().Insert<DgnGeometryPart>(*geomPart);

    Json::Value     jsonValue(Json::objectValue);
    SetDescription(jsonValue, symbolComponent);
    v10Id = LsComponentId();

    totalRange.high.Subtract(totalRange.low);

    uint32_t symFlags = 0;
    if (symbolComponent.Is3d())
        symFlags |= LSSYM_3D;
    if (symbolComponent.IsNoScale())
        symFlags |= LSSYM_NOSCALE;

    LsSymbolComponent::SaveSymbolDataToJson(jsonValue, totalRange.low, totalRange.high, geomPart->GetId(), symFlags, symbolComponent.GetStoredUnitScale());

    return LsComponent::AddComponentAsJsonProperty(v10Id, GetDgnDb(), LsComponentType::PointSymbol, jsonValue);
    }



//---------------------------------------------------------------------------------
// @bsimethod                                                    Vern.Francisco   12/17
//---------------+---------------+---------------+---------------+---------------+------

struct V8GraphicsLightWeightCollector : DgnV8Api::IElementGraphicsProcessor
    {
    private:

        DgnV8Api::ViewContext*      m_context;
        Bentley::Transform          m_conversionScale;
        Bentley::Transform          m_currentTransform;
        DgnV8Api::ElemDisplayParams m_currentDisplayParams;
        LightWeightConverter&       m_converter;
        DgnModelR                   m_model;
        bvector<DgnV8PathGeom>      m_v8PathGeom;
        bool                        m_inBRepConvertFaces;
        GeometryBuilderPtr          m_builder;

    protected:

        virtual DgnV8Api::DrawPurpose _GetDrawPurpose() override { return DgnV8Api::DrawPurpose::DgnDbConvert; } // Required so that xg symbols get pushed onto DisplayPath...

        virtual bool _ProcessAsBody(bool isCurved) const override { return true; }
        virtual bool _ProcessAsFacets(bool isPolyface) const override { return isPolyface; }
        virtual void _AnnounceContext(DgnV8Api::ViewContext& context) override { m_context = &context; }
        virtual void _AnnounceTransform(Bentley::TransformCP trans) override { if (trans) m_currentTransform = *trans; else m_currentTransform.InitIdentity(); }
        virtual void _AnnounceElemDisplayParams(DgnV8Api::ElemDisplayParams const& displayParams) override { m_currentDisplayParams = displayParams; }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        virtual bool _ExpandPatterns() const
            {
            // NOTE: Since we don't handle patterns on elements that don't support IAreaFillPropertiesQuery, just spew their graphics...
            DgnV8Api::DisplayPath const* path = m_context->GetCurrDisplayPath();
            Bentley::ElementRefP elRef = (nullptr != path ? path->GetCursorElem() : nullptr);

            if (nullptr == elRef)
                return false;

            DgnV8Api::ElementHandle v8Eh(elRef);
            DgnV8Api::IAreaFillPropertiesQuery* areaObj = dynamic_cast<DgnV8Api::IAreaFillPropertiesQuery*> (&v8Eh.GetHandler());

            if (nullptr != areaObj)
                return false;

            return true;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        virtual Bentley::BentleyStatus _ProcessCurveVector(Bentley::CurveVectorCR curves, bool isFilled) override
            {
            DgnV8PathGeom& pathGeom = GetDgnV8PathGeom();
            DgnV8PathEntry pathEntry(DoInterop(m_currentTransform), DoInterop(m_conversionScale), m_model.Is3d(), m_context->GetCurrentModel()->Is3d());

            m_converter.InitGeometryParams(pathEntry.m_geomParams, m_currentDisplayParams, *m_context, m_model.Is3d());

            // NOTE: Need to apply pushed transforms (ex. shared cell scale) to linestyle params...
            //       GeometryBuilder will ignore the linestyle for other types of GeometricPrimitive so it's ok
            //       that we only account for scale here in _ProcessCurveVector.
            if (pathEntry.m_geomParams.IsTransformable())
                {
                RotMatrix   rMatrix, rotation, skewFactor;

                DoInterop(m_currentTransform).GetMatrix(rMatrix);

                if (rMatrix.RotateAndSkewFactors(rotation, skewFactor, 0, 1) && !skewFactor.IsIdentity())
                    pathEntry.m_geomParams.ApplyTransform(Transform::From(skewFactor));
                }

            // NOTE: Unfortunately PatternParams isn't part of ElemDisplayParams in V8, so we can only check any
            //       element that output a region curve vector to see if it supports IAreaFillPropertiesQuery.
            //       There's no way to get at a PatternParams buried in an XGraphicsContainer either...but then
            //       MicroStation's pattern tools (ex. show pattern) don't work with these either...
            if (curves.IsAnyRegionType())
                {
                Bentley::ElementRefP elRef = pathGeom.m_path->GetCursorElem();

                if (nullptr != elRef)
                    {
                    DgnV8Api::ElementHandle v8Eh(elRef);
                    DgnV8Api::IAreaFillPropertiesQuery* areaObj = dynamic_cast<DgnV8Api::IAreaFillPropertiesQuery*> (&v8Eh.GetHandler());

                    if (nullptr != areaObj)
                        {
                        Bentley::PatternParamsPtr v8Pattern;
                        Bentley::bvector<DgnV8Api::DwgHatchDefLine> v8DefLines;
                        Bentley::DPoint3d origin;

                        if (areaObj->GetPattern(v8Eh, v8Pattern, &v8DefLines, &origin, 0)) // Not going to worry about mline patterns...
                            {
                            PatternParamsPtr pattern = PatternParams::Create();
                            if (m_converter.InitPatternParams(*pattern, *v8Pattern, v8DefLines, origin, *m_context))
                                {
                                DgnV8Api::IAssocRegionQuery* assocRegion = dynamic_cast<DgnV8Api::IAssocRegionQuery*> (&v8Eh.GetHandler());

                                if (nullptr != assocRegion)
                                    {
                                    DgnV8Api::RegionParams regionParams;

                                    if (SUCCESS == assocRegion->GetParams(v8Eh, regionParams))
                                        {
                                        DgnV8Api::ElementHandle templateEh;

                                        DgnV8Api::ComplexHeaderDisplayHandler::GetComponentForDisplayParams(templateEh, v8Eh);
                                        pattern->SetInvisibleBoundary(templateEh.IsValid() ? templateEh.GetElementCP()->hdr.dhdr.props.b.invisible : regionParams.GetInvisibleBoundary());
                                        }
                                    }

                                pathEntry.m_geomParams.SetPatternParams(pattern.get());
                                }
                            }
                        }
                    }
                }

            pathEntry.m_curve = curves.Clone();
            pathGeom.AddEntry(pathEntry);

            return Bentley::SUCCESS;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        virtual Bentley::BentleyStatus _ProcessSolidPrimitive(Bentley::ISolidPrimitiveCR primitive) override
            {
            DgnV8PathGeom& pathGeom = GetDgnV8PathGeom();
            DgnV8PathEntry pathEntry(DoInterop(m_currentTransform), DoInterop(m_conversionScale), m_model.Is3d(), m_context->GetCurrentModel()->Is3d());

            m_converter.InitGeometryParams(pathEntry.m_geomParams, m_currentDisplayParams, *m_context, m_model.Is3d());

            pathEntry.m_primitive = primitive.Clone();
            pathGeom.AddEntry(pathEntry);

            return Bentley::SUCCESS;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        virtual Bentley::BentleyStatus _ProcessSurface(Bentley::MSBsplineSurfaceCR surface) override
            {
            DgnV8PathGeom& pathGeom = GetDgnV8PathGeom();
            DgnV8PathEntry pathEntry(DoInterop(m_currentTransform), DoInterop(m_conversionScale), m_model.Is3d(), m_context->GetCurrentModel()->Is3d());

            m_converter.InitGeometryParams(pathEntry.m_geomParams, m_currentDisplayParams, *m_context, m_model.Is3d());

            pathEntry.m_surface = Bentley::MSBsplineSurface::CreatePtr();
            pathEntry.m_surface->CopyFrom(surface);
            pathGeom.AddEntry(pathEntry);

            return Bentley::SUCCESS;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        virtual Bentley::BentleyStatus _ProcessFacets(Bentley::PolyfaceQueryCR meshData, bool isFilled) override
            {
            DgnV8PathGeom& pathGeom = GetDgnV8PathGeom();
            DgnV8PathEntry pathEntry(DoInterop(m_currentTransform), DoInterop(m_conversionScale), m_model.Is3d(), m_context->GetCurrentModel()->Is3d());

            m_converter.InitGeometryParams(pathEntry.m_geomParams, m_currentDisplayParams, *m_context, m_model.Is3d());

            BentleyApi::RgbFactor const* rgbColors = (BentleyApi::RgbFactor const*) meshData.GetDoubleColorCP();
            uint32_t const* intColors = meshData.GetIntColorCP();
            uint32_t const* tableColors = meshData.GetColorTableCP();
            bvector<uint32_t> intColorsVec;

            // Color table indices aren't supported (and we don't need both RgbFactor and TBGR int color)...
            if (nullptr == intColors)
                {
                if (nullptr != rgbColors)
                    {
                    for (size_t iColor = 0; iColor < meshData.GetColorCount(); iColor++)
                        intColorsVec.push_back(rgbColors[iColor].ToIntColor());

                    intColors = &intColorsVec.front();
                    }
                else if (nullptr != tableColors)
                    {
                    DgnFileR dgnFile = *m_context->GetCurrentModel()->GetDgnFileP();

                    for (size_t iColor = 0; iColor < meshData.GetColorCount(); iColor++)
                        {
                        DgnV8Api::IntColorDef mappedColor(255, 255, 255); // Initialized to white...

                                                                            // Allegedly COLOR_BYCELL/COLOR_BYLEVEL are acceptable values, resolve effective value...
                        if (DgnV8Api::COLOR_BYCELL == tableColors[iColor])
                            {
                            ElemHeaderOverridesCP ovr = m_context->GetHeaderOvr();

                            if (ovr)
                                DgnV8Api::DgnColorMap::ExtractElementColorInfo(&mappedColor, nullptr, nullptr, nullptr, nullptr, ovr->GetColor(), dgnFile);
                            }
                        else if (DgnV8Api::COLOR_BYLEVEL == tableColors[iColor])
                            {
                            DgnV8Api::LevelHandle level = m_context->GetCurrentModel()->GetLevelCacheR().GetLevel(m_context->GetCurrentDisplayParams()->GetLevel());

                            if (level.IsValid())
                                {
                                DgnV8Api::LevelDefinitionColor colorDef = level.GetByLevelColor();
                                DgnV8Api::DgnColorMap::ExtractElementColorInfo(&mappedColor, nullptr, nullptr, nullptr, nullptr, colorDef.GetColor(), *colorDef.GetDefinitionFile());
                                }
                            }
                        else
                            {
                            DgnV8Api::DgnColorMap::ExtractElementColorInfo(&mappedColor, nullptr, nullptr, nullptr, nullptr, tableColors[iColor], dgnFile);
                            }

                        intColorsVec.push_back(mappedColor.m_int);
                        }

                    intColors = &intColorsVec.front();
                    }
                }

            Bentley::PolyfaceQueryCarrier sourceData(meshData.GetNumPerFace(), meshData.GetTwoSided(), meshData.GetPointIndexCount(),
                                                        meshData.GetPointCount(), meshData.GetPointCP(), meshData.GetPointIndexCP(),
                                                        meshData.GetNormalCount(), meshData.GetNormalCP(), meshData.GetNormalIndexCP(),
                                                        meshData.GetParamCount(), meshData.GetParamCP(), meshData.GetParamIndexCP(),
                                                        meshData.GetColorCount(), meshData.GetColorIndexCP(), nullptr, nullptr, intColors, nullptr,
                                                        meshData.GetIlluminationNameCP(), meshData.GetMeshStyle(), meshData.GetNumPerRow());

            pathEntry.m_mesh = Bentley::PolyfaceHeader::New();
            pathEntry.m_mesh->CopyFrom(sourceData);
            pathGeom.AddEntry(pathEntry);

            return Bentley::SUCCESS;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        bool ProcessLargeBody(Bentley::ISolidKernelEntityCR entity, Bentley::IFaceMaterialAttachmentsCP attachments)
            {
            // What is a good criteria for detecting a rediculous BRep??? A single body larger than 30 mb will certainly not work well...
            if (m_inBRepConvertFaces || DgnV8Api::PSolidUtil::DebugMemoryUsage(entity) < 30000000)
                return false;

            // Output each face, create curve vector for planar faces and sheet bodies for everything else. Dropping non-analytic faces isn't very robust and facets are meh...
            m_inBRepConvertFaces = true;
            DgnV8Api::DgnPlatformLib::GetHost().GetSolidsKernelAdmin()._OutputBodyAsSurfaces(entity, *m_context, false, attachments);
            m_inBRepConvertFaces = false;

            return true;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        void ProcessSingleBody(Bentley::ISolidKernelEntityCR entity, Bentley::IFaceMaterialAttachmentsCP attachments)
            {
            if (ProcessLargeBody(entity, attachments))
                {
//                m_converter.ReportIssue(Converter::IssueSeverity::Warning, Converter::IssueCategory::Unsupported(), Converter::Issue::LargeBRep(), nullptr);
                return;
                }

            DgnV8PathGeom& pathGeom = GetDgnV8PathGeom();
            DgnV8PathEntry pathEntry(DoInterop(m_currentTransform), DoInterop(m_conversionScale), m_model.Is3d(), m_context->GetCurrentModel()->Is3d());

            m_converter.InitGeometryParams(pathEntry.m_geomParams, m_currentDisplayParams, *m_context, m_model.Is3d());

            if (SUCCESS != DgnV8Api::DgnPlatformLib::GetHost().GetSolidsKernelAdmin()._CopyEntity(pathEntry.m_brep, entity))
                return;

            if (nullptr != attachments && !attachments->_GetFaceToSubElemIdMap().empty() && !attachments->_GetFaceAttachmentsMap().empty())
                {
                FaceAttachment attachment;

                if (!pathEntry.m_geomParams.IsLineColorFromSubCategoryAppearance())
                    attachment.SetColor(pathEntry.m_geomParams.GetLineColor().GetValue(), pathEntry.m_geomParams.GetTransparency());

                if (!pathEntry.m_geomParams.IsMaterialFromSubCategoryAppearance())
                    attachment.SetMaterial(pathEntry.m_geomParams.GetMaterialId());

                pathEntry.m_attachments = DgnDbApi::PSolidUtil::CreateNewFaceAttachments(DgnV8Api::PSolidUtil::GetEntityTag(*pathEntry.m_brep), attachment);

                if (!pathEntry.m_attachments.IsValid())
                    {
                    pathGeom.AddEntry(pathEntry);
                    return;
                    }

                DgnV8Api::T_FaceToSubElemIdMap const& faceToSubElemIdMapV8 = attachments->_GetFaceToSubElemIdMap();
                DgnV8Api::T_FaceAttachmentsMap const& faceAttachmentsMapV8 = attachments->_GetFaceAttachmentsMap();
                int32_t lowestFound = -1;

                // Normalize face index values that would have been adjusted for uniqueness when multiple bodies output by a single element...
                for (DgnV8Api::T_FaceToSubElemIdMap::const_iterator curr = faceToSubElemIdMapV8.begin(); curr != faceToSubElemIdMapV8.end(); ++curr)
                    {
                    if (-1 == lowestFound || curr->second < lowestFound)
                        lowestFound = curr->second;
                    }

                DgnDbApi::T_FaceAttachmentsVec const& faceAttachmentsVecDb = pathEntry.m_attachments->_GetFaceAttachmentsVec();
                bmap<int32_t, uint32_t> subElemIdToFaceMapDb;
                bvector<PK_FACE_t> faces;

                DgnDbApi::PSolidTopo::GetBodyFaces(faces, DgnV8Api::PSolidUtil::GetEntityTag(*pathEntry.m_brep));

                for (int iFace = 0; iFace < (int) faces.size(); iFace++)
                    subElemIdToFaceMapDb[iFace + 1] = faces[iFace]; // subElemId is 1 based face index...

                DgnDbApi::PSolidAttrib::CreateFaceMaterialIndexAttributeDef(); // Create attribute definition if we haven't already, this attribute definition isn't setup by the V8 frustrum...

                for (DgnV8Api::T_FaceToSubElemIdMap::const_iterator curr = faceToSubElemIdMapV8.begin(); curr != faceToSubElemIdMapV8.end(); ++curr)
                    {
                    bmap <int32_t, uint32_t>::const_iterator foundIndexDb = subElemIdToFaceMapDb.find(curr->second - lowestFound + 1);

                    if (foundIndexDb == subElemIdToFaceMapDb.end())
                        continue;

                    DgnV8Api::T_FaceAttachmentsMap::const_iterator foundFaceV8 = faceAttachmentsMapV8.find(curr->first);

                    if (foundFaceV8 == faceAttachmentsMapV8.end())
                        continue;

                    DgnV8Api::ElemDisplayParams faceParamsV8(m_currentDisplayParams);
                    Render::GeometryParams faceParamsDb;

                    foundFaceV8->second.ToElemDisplayParams(faceParamsV8);
                    m_converter.InitGeometryParams(faceParamsDb, faceParamsV8, *m_context, m_model.Is3d());

                    if (!faceParamsDb.IsLineColorFromSubCategoryAppearance())
                        attachment.SetColor(faceParamsDb.GetLineColor().GetValue(), faceParamsDb.GetTransparency());

                    if (!faceParamsDb.IsMaterialFromSubCategoryAppearance())
                        attachment.SetMaterial(faceParamsDb.GetMaterialId());

                    size_t attachmentIndex = 0;
                    DgnDbApi::T_FaceAttachmentsVec::const_iterator foundAttachmentDb = std::find(faceAttachmentsVecDb.begin(), faceAttachmentsVecDb.end(), attachment);

                    if (foundAttachmentDb == faceAttachmentsVecDb.end())
                        {
                        const_cast<DgnDbApi::T_FaceAttachmentsVec&>(faceAttachmentsVecDb).push_back(attachment);
                        attachmentIndex = faceAttachmentsVecDb.size() - 1;
                        }
                    else
                        {
                        attachmentIndex = std::distance(faceAttachmentsVecDb.begin(), foundAttachmentDb);
                        }

                    DgnDbApi::PSolidAttrib::SetFaceMaterialIndexAttribute(foundIndexDb->second, attachmentIndex); // Call with 0 will remove an existing attrib...avoid weird roundtrip scenario issues?
                    }

                if (faceAttachmentsVecDb.size() < 2)
                    pathEntry.m_attachments = nullptr; // ex. FeatureSolid w/uniform symbology, no reason to keep attachments.
                }

            pathGeom.AddEntry(pathEntry);
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        virtual Bentley::BentleyStatus _ProcessBody(Bentley::ISolidKernelEntityCR entity, Bentley::IFaceMaterialAttachmentsCP attachments)
            {
            // Prevent Polyface from BRep from picking up bogus fill since Polyface supports fill but BRep ignored it (ProStructures elements)...
            if (DgnV8Api::FillDisplay::Never != m_currentDisplayParams.GetFillDisplay())
                m_currentDisplayParams.SetFillDisplay(DgnV8Api::FillDisplay::Never);

            // NOTE: Don't separate regions when there are face attachments...
            if (nullptr == attachments && !m_inBRepConvertFaces)
                {
                // Split large disjoint bodies into separate regions (ex. STEP importer erroneously created these)...
                Bentley::bvector<DgnV8Api::ISolidKernelEntityPtr> out;

                if (SUCCESS == DgnV8Api::SolidUtil::DisjoinBody(out, entity))
                    {
                    for (DgnV8Api::ISolidKernelEntityPtr& thisEntity : out)
                        {
                        ProcessSingleBody(*thisEntity, nullptr);

//                        m_converter.ReportProgress();
                        }

                    return Bentley::SUCCESS;
                    }
                }

        //    if (m_inBRepConvertFaces)
        //        m_converter.ReportProgress();

            ProcessSingleBody(entity, attachments);

            return Bentley::SUCCESS;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        virtual Bentley::BentleyStatus _ProcessTextString(Bentley::TextStringCR v8Text)
            {
            /*
            DgnV8 and DgnDb TextString and glyph layout have substantial differences. Further, I do not want to drag the boat anchor of DgnV8 forward just for WYSIWYG initial display.
            However, one key thing that DgnDb TextString adds is the ability to cache and serialize glyph information, such that as display time, they are not re-computed.
            Thus, the strategy is to make a DgnDb TextString, but manually tweak its cached values such that it will look like DgnV8, but can still be read and understood as a DgnDb TextString.
            This means that on-edit, DgnDb rules will apply and the string may drastically change, but at least when first opened, it will display like DgnV8.
            Most differences between DgnV8 and DgnDb largely have to do with character spacing, vertical-ness, and DWG-ness, which can be captured by simply positioning glyphs as DgnV8 would dictate.
            DgnV8 also supports additional adornment (e.g. underline/overline) and fraction options, which must also be special-cased.
            */

            // DgnDb does not support symbol text elements, which do not have meaningful text content, and cannot be represented as Unicode. Drop to geometry to get fidelity.
            if (v8Text.GetProperties().GetFont().IsSymbolFont())
                return Bentley::ERROR;

            // No string? Then not worth converting...
            if (Bentley::WString::IsNullOrEmpty(v8Text.GetString()))
                return Bentley::SUCCESS;

            // 0.0 height or width? Then not worth converting... wouldn't draw, and a 0.0 height will cause a divide-by-zero since we persist width as a factor.
            if ((0.0 == v8Text.GetProperties().GetFontSize().x) || (0.0 == v8Text.GetProperties().GetFontSize().y))
                return Bentley::SUCCESS;

            DgnV8PathGeom& pathGeom = GetDgnV8PathGeom();
            DgnV8PathEntry pathEntry(DoInterop(m_currentTransform), DoInterop(m_conversionScale), m_model.Is3d(), m_context->GetCurrentModel()->Is3d());

            m_converter.InitGeometryParams(pathEntry.m_geomParams, m_currentDisplayParams, *m_context, m_model.Is3d());

            Bentley::DPoint3d                  lowerLeft = v8Text.GetOrigin();
            Bentley::RotMatrix                 rMatrix = v8Text.GetRotMatrix();
            DgnV8Api::TextStringPropertiesPtr  props = v8Text.GetProperties().Clone();

            pathEntry.m_text = DgnV8Api::TextString::Create(v8Text.GetString(), &lowerLeft, &rMatrix, *props);
            pathGeom.AddEntry(pathEntry);

            // Guess whether it is necessary to explicitly output text background and adornments...
            DgnV8Api::DisplayPath const* path = m_context->GetCurrDisplayPath();

            if (nullptr != path)
                {
                DgnV8Api::ElementHandle v8Eh(path->GetCursorElem());
                DgnV8Api::ElementHandle::XAttributeIter iterator(v8Eh, DgnV8Api::XAttributeHandlerId(DgnV8Api::XATTRIBUTEID_XGraphics, DgnV8Api::XGraphicsMinorId_Data), 0);

                // Adornments/Background already will already be recored as geometry in XGraphics container...
                if (iterator.IsValid())
                    return Bentley::SUCCESS;
                }

            // And then let DgnV8 emit its styled adornments directly as geometry since a DB TextString won't know how to draw them.
            // NEEDSWORK. It appears the drawing of Adornments is not working. At least the underline is always drawn at 0,0. for now
            // just ignoring them. The backgroug is working but is causing display issues in Gist.
            v8Text.DrawTextAdornments(*m_context);
            v8Text.DrawTextBackground(*m_context);

            return Bentley::SUCCESS;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        DgnV8PathGeom& GetDgnV8PathGeom()
            {
            DgnV8Api::DisplayPath const* path = m_context->GetCurrDisplayPath();

            if (0 == m_v8PathGeom.size() || !m_v8PathGeom.back().m_path->IsSamePath(path, true))
                {
                DgnV8PathGeom pathGeom(path);

                m_v8PathGeom.push_back(pathGeom);
                }

            return m_v8PathGeom.back();
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        bool GetBasisTransform(TransformR basisTransform, double& v8SymbolScale, Bentley::DgnPlatform::ElementHandle const& v8eh)
            {
            Bentley::Transform localToGeom;
            ConvertToDgnDbElementExtension* upx = ConvertToDgnDbElementExtension::Cast(v8eh.GetHandler());

            v8SymbolScale = 0.0;

            bool basisTransformSuppliedByExtension = false;
            if (nullptr != upx)
                basisTransformSuppliedByExtension = upx->_GetBasisTransform(localToGeom, v8eh, (Converter&)m_converter);
            for (auto xdomain : XDomainRegistry::s_xdomains)
                basisTransformSuppliedByExtension |= xdomain->_GetBasisTransform(localToGeom, v8eh, (Converter&) m_converter);

            if (!basisTransformSuppliedByExtension)
                {
                // NOTE: Attempt to preserve local coordinate system of V8 element for making geom parts.
                //       We can't just call DisplayHandler::GetBasisTransform as it doesn't return a good local
                //       frame in many cases but still returns SUCCESS. Check for known/usually ok cases.
                if (DgnV8Api::EXTENDED_ELM == v8eh.GetElementType() && DgnV8Api::XGraphicsContainer::IsPresent(v8eh))
                    {
                    // Using the XGraphics symbol transform will handle both XGraphic symbols and SmartFeatures.
                    if (SUCCESS != DgnV8Api::XGraphicsContainer::ExtractTransform(localToGeom, v8eh))
                        return false;

                    Bentley::DVec3d      scaleVec;
                    Bentley::DPoint3d    origin;
                    Bentley::RotMatrix   rMatrix, invRMatrix;

                    localToGeom.GetTranslation(origin);
                    localToGeom.GetMatrix(rMatrix);

                    if (!invRMatrix.InverseOf(rMatrix))
                        return false;

                    rMatrix.NormalizeColumnsOf(rMatrix, scaleVec);

                    if (DoubleOps::AlmostEqual(scaleVec.x, scaleVec.y, 1.0e-5) && DoubleOps::AlmostEqual(scaleVec.x, scaleVec.z, 1.0e-5))
                        v8SymbolScale = scaleVec.x;

                    localToGeom.InitFrom(rMatrix, origin);
                    }
                else
                    {
                    // Shared cells and user defined cells should have a good basis...
                    DgnV8Api::ICellQuery* cellQuery;

                    if (nullptr == (cellQuery = dynamic_cast <DgnV8Api::ICellQuery*> (&v8eh.GetHandler())))
                        return false; // Not a cell, shared cell instance, or shared cell def...

                    if (cellQuery->IsNormalCell(v8eh) && cellQuery->IsAnonymous(v8eh))
                        return false; // Not a user defined cell, edit->group does not set a useful orientation...

                    if (!v8eh.GetDisplayHandler()->GetBasisTransform(v8eh, localToGeom))
                        return false;

                    // Basis transform is expected to be normalized...need to get V8 cell scale directly...
                    Bentley::DVec3d cellScale;

                    if (SUCCESS == cellQuery->ExtractScale(cellScale, v8eh))
                        {
                        double minScale = DoubleOps::Min(cellScale.x, cellScale.y, cellScale.z);
                        double maxScale = DoubleOps::Max(cellScale.x, cellScale.y, cellScale.z);

                        if (DoubleOps::AlmostEqual(minScale, maxScale, 1.0e-5))
                            v8SymbolScale = minScale;
                        }
                    }
                }

            localToGeom = Bentley::Transform::FromProduct(m_conversionScale, localToGeom); // Apply unit scaling...
                                                                                            // NOTE: Ensure rotation is squared up and normalized...
            DPoint3d    origin;
            RotMatrix   rMatrix;

            localToGeom.GetTranslation(DoInterop(origin));
            localToGeom.GetMatrix(DoInterop(rMatrix));

            if (rMatrix.Determinant() < 0.0)
                v8SymbolScale *= -1.0;

            rMatrix.SquareAndNormalizeColumns(rMatrix, 0, 1);

            // Don't use basis transform from element if it can't be represented by Placement2d (conversion of 3d sheet geometry to 2d)...
            if (!m_model.Is3d())
                {
                YawPitchRollAngles  angles;

                YawPitchRollAngles::TryFromRotMatrix(angles, rMatrix);

                if (0 != BeNumerical::Compare(origin.z, 0.0) || 0.0 != angles.GetPitch().Degrees() || 0.0 != angles.GetRoll().Degrees())
                    return false;
                }

            basisTransform.InitFrom(rMatrix, origin);

            return true;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        static Utf8String GetElementLabel(Bentley::DgnPlatform::ElementHandle const& v8eh)
            {
            DgnV8Api::ICellQuery* cellQuery;

            if (nullptr == (cellQuery = dynamic_cast <DgnV8Api::ICellQuery*> (&v8eh.GetHandler())))
                return ""; // Not a cell, shared cell instance, or shared cell def...

            WChar strBuffer[Bentley::DgnPlatform::MAX_CELLNAME_LENGTH];

            if (SUCCESS != cellQuery->ExtractName(strBuffer, Bentley::DgnPlatform::MAX_CELLNAME_LENGTH, v8eh))
                return "";

            Utf8String str;

            str.Assign(strBuffer);

            return str;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        Bentley::ElementRefP GetCurrentInstanceElement(DgnV8Api::DisplayPath const& path)
            {
            Bentley::ElementRefP elRef = path.GetCursorElem();

            if (DgnV8Api::XGraphicsContainer::IsXGraphicsSymbol(elRef))
                return elRef;

            if (!elRef->IsComplexComponent())
                return nullptr;

            Bentley::ElementRefP parentRef = elRef;

            do
                {
                if (DgnV8Api::SHAREDCELL_DEF_ELM == parentRef->GetElementType())
                    return elRef;

                } while (nullptr != (parentRef = parentRef->GetParentElementRef()));

                return nullptr;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        Bentley::ElementRefP GetOutermostSCDef(DgnV8Api::DisplayPath const& path)
            {
            for (int iPath = 0; iPath < path.GetCount(); ++iPath)
                {
                Bentley::ElementRefP pathElRef = path.GetPathElem(iPath);

                if (DgnV8Api::SHAREDCELL_DEF_ELM == pathElRef->GetElementType())
                    return pathElRef;
                }

            return nullptr;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        DgnCode GetPartCode(Bentley::ElementRefP instanceElRef, Utf8CP prefix, uint32_t sequenceNo, double partScale)
            {
            BeAssert(sequenceNo > 0); // First entry starts at 1...

            if (sequenceNo > 1)
                { // Note: partScale < 0.0 implies "mirrored" which means a different codeValue must be generated
                Utf8PrintfString partCodeValue(partScale < 0.0 ? "%s-%ld-M%lld-%d" : "%s-%ld-%lld-%d", prefix, m_converter.GetConverterId(), instanceElRef->GetElementId(), sequenceNo - 1);
                return m_converter.CreateCode(partCodeValue);
                }

            Utf8PrintfString partCodeValue(partScale < 0.0 ? "%s-%ld-M%lld" : "%s-%ld-%lld", prefix, m_converter.GetConverterId(), instanceElRef->GetElementId());
            return m_converter.CreateCode(partCodeValue);
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        void GetPartReferences(bvector<DgnV8PartReference>& geomParts, double v8SymbolScale, DgnV8EhCR v8eh)
            {
            switch (v8eh.GetElementType())
                {
                    case DgnV8Api::SHARED_CELL_ELM:
                    {
                    DgnV8PartReferenceAppData::T_PartGeom* partCache = nullptr;

                    if (nullptr != (partCache = DgnV8PartReferenceAppData::GetCache(DgnV8Api::CellUtil::FindSharedCellDefinition(*v8eh.GetElementCP(), *v8eh.GetDgnFileP()), v8SymbolScale, false))) // Don't create...
                        geomParts = partCache->m_geomParts;
                    break;
                    }

                    case DgnV8Api::EXTENDED_ELM:
                    {
                    DgnV8Api::DependencyLinkage const* depLinkage;

                    if (SUCCESS != DgnV8Api::DependencyManagerLinkage::GetLinkage(&depLinkage, v8eh, 10000/*DEPENDENCYAPPID_MicroStation*/, DEPENDENCYAPPVALUE_XGraphicsSymbol))
                        break;

                    for (int iSymbol = 0; iSymbol < depLinkage->nRoots; ++iSymbol)
                        {
                        Bentley::ElementRefP symbolElRef = v8eh.GetDgnModelP()->FindByElementId(depLinkage->root.elemid[iSymbol]);

                        if (nullptr == symbolElRef)
                            continue;

                        DgnV8PartReferenceAppData::T_PartGeom* partCache = nullptr;

                        if (nullptr == (partCache = DgnV8PartReferenceAppData::GetCache(symbolElRef, v8SymbolScale, false))) // Don't create...
                            {
                            geomParts.clear(); // All symbols must have a cache, a partial cache isn't useful...
                            break;
                            }

                        if (geomParts.empty())
                            geomParts = partCache->m_geomParts;
                        else
                            geomParts.insert(geomParts.end(), partCache->m_geomParts.begin(), partCache->m_geomParts.end());
                        }
                    break;
                    }
                }
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------

        void CreatePartReferences(bvector<DgnV8PartReference>& geomParts, TransformCR basisTransform, double v8SymbolScale)
            {
            for (DgnV8PathGeom& pathGeom : m_v8PathGeom)
                {
                for (DgnV8PathEntry& pathEntry : pathGeom.m_entries)
                    {
                    if (0.0 == pathEntry.m_partScale)
                        return; // Not suitable for creating a part, ex. non-uniform scale...

                    if (pathEntry.m_curve.IsValid() && pathEntry.m_geomParams.HasStrokedLineStyle())
                        return;
                    }
                }

            int         iEntry = 0;
            Transform   invBasisTrans;

            invBasisTrans.InverseOf(basisTransform);

            // NOTE: For a shared cell instance, all geometry (including that from nested instances) is cached
            //       on the outermost shared cell definition. XGraphics symbols don't nest, so we can just
            //       cache the geometry for each symbol.
            DgnV8PartReferenceAppData::T_PartGeom* partCache = nullptr;

            for (DgnV8PathGeom& pathGeom : m_v8PathGeom)
                {
                Bentley::ElementRefP instanceElRef = GetCurrentInstanceElement(*pathGeom.m_path);

                if (nullptr == instanceElRef)
                    {
                    BeAssert(geomParts.empty()); // If one entry is from a V8 instance, then all entries should be...
                    geomParts.clear();
                    return;
                    }

                Bentley::ElementRefP scDefElRef = GetOutermostSCDef(*pathGeom.m_path);

                if (nullptr == partCache)
                    partCache = DgnV8PartReferenceAppData::GetCache(nullptr != scDefElRef ? scDefElRef : instanceElRef, v8SymbolScale, true); // Create if doesn't exist...

                uint32_t sequenceNo = 0;
                DgnFileP dgnFile = pathGeom.m_path->GetRoot()->GetDgnFileP();

                for (DgnV8PathEntry& pathEntry : pathGeom.m_entries)
                    {
                    sequenceNo++; // Always increment, even if geometry conversion fails...

                   // if (0 == (++iEntry % 10))
                   //     m_converter.ReportProgress();

                    Transform         geomToLocal = Transform::FromProduct(invBasisTrans, pathEntry.m_geomToWorld);
                    DgnCode           partCode = GetPartCode(instanceElRef, nullptr == scDefElRef ? "XGSymbV8" : "SCDefV8", sequenceNo, pathEntry.m_partScale);
                    DgnGeometryPartId partId = DgnGeometryPart::QueryGeometryPartId(*(m_converter.GetJobDefinitionModel()), partCode.GetValueUtf8());
                    DRange3d          localRange = DRange3d::NullRange();

                    if (!partId.IsValid())
                        {
                        GeometricPrimitivePtr geometry = pathEntry.GetGeometry(*dgnFile, m_converter, true);

                        if (!geometry.IsValid())
                            continue;

                        GeometryBuilderPtr  partBuilder = GeometryBuilder::CreateGeometryPart(m_model.GetDgnDb(), m_model.Is3d());

                        partBuilder->Append(*geometry);

                        DgnGeometryPartPtr  geomPart = DgnGeometryPart::Create(*(m_converter.GetJobDefinitionModel()), partCode.GetValueUtf8());

                        if (SUCCESS == partBuilder->Finish(*geomPart) && m_model.GetDgnDb().Elements().Insert<DgnGeometryPart>(*geomPart).IsValid())
                            {
                            partId = geomPart->GetId();
                            localRange = geomPart->GetBoundingBox();
                            }
                        }
                    else
                        {
                        if (SUCCESS != DgnGeometryPart::QueryGeometryPartRange(localRange, m_model.GetDgnDb(), partId))
                            partId = DgnGeometryPartId(); // Shouldn't happen, we know part exists...
                        }

                    if (!partId.IsValid())
                        {
                        BeAssert(false); // Why couldn't part be created???
                        continue;
                        }

                    DgnV8PartReference partRef;

                    partRef.m_partId = partId;
                    partRef.m_scale = pathEntry.m_partScale;
                    partRef.m_geomParams = pathEntry.m_geomParams;
                    partRef.m_geomToLocal = geomToLocal;
                    partRef.m_localRange = localRange;
                    partRef.m_isLevelByCell = DgnV8Api::LEVEL_BYCELL == pathGeom.m_path->GetCursorElem()->GetUnstableMSElementCP()->ehdr.level;
                    partRef.m_isLevelValid = true;

                    geomParts.push_back(partRef);

                    if (nullptr != partCache && !partCache->m_filled)
                        partCache->m_geomParts.push_back(partRef);
                    }

                if (nullptr != partCache && nullptr == scDefElRef)
                    {
                    partCache->m_filled = true;
                    partCache = nullptr;
                    }
                }

            if (nullptr != partCache)
                partCache->m_filled = true;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------
        bool IsValidForPostInstancing(GeometricPrimitiveR geometry, DgnV8Api::DisplayPath const& path)
            {
            if (!m_model.Is3d())
                return false;

            // NOTE: Determine if geometry is worth instancing.
            //       Always reject text. Instancing may have undesirable ramifications for editing, etc.
            //       Always reject line/point strings! The CurveVector round trips as a CurvePrimitive from GeometryCollection so the type check always fails!!!
            switch (geometry.GetGeometryType())
                {
                    case GeometricPrimitive::GeometryType::SolidPrimitive:
                    case GeometricPrimitive::GeometryType::BsplineSurface:
                    case GeometricPrimitive::GeometryType::Polyface:
                    case GeometricPrimitive::GeometryType::BRepEntity:
                        break;

                    default:
                        return false;
                }

            Bentley::ElementRefP elRef = path.GetCursorElem();

            if (elRef->IsComplexComponent())
                elRef = elRef->GetOutermostParentOrSelf();

            // Normal cell components are good candidates for post-instancing...and it shouldn't be a huge problem if we create a part and don't find other instances...
            if (DgnV8Api::CELL_HEADER_ELM == elRef->GetElementType() && !elRef->GetUnstableMSElementCP()->hdr.dhdr.props.b.h)
                return true;

            return false;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------
        bool IsMatchingPartGeometry(DgnGeometryPartId partId, DgnDbR db, GeometricPrimitiveR geometry)
            {
            GeometricPrimitivePtr partGeometry = PostInstancePartCacheAppData::GetPart(partId, db);

            if (!partGeometry.IsValid() || !partGeometry->IsSameStructureAndGeometry(geometry, 1.0e-5))
                return false;

            return true;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------
        void ApplySharedCellInstanceOverrides(DgnV8EhCR v8eh, Render::GeometryParamsR geomParams)
            {
            if (DgnV8Api::SHARED_CELL_ELM != v8eh.GetElementType())
                return;

            DgnV8Api::SCOverride scOvr = v8eh.GetElementCP()->sharedCell.m_override;

            if (0 == *((UInt16*) (&scOvr)))
                return;

            if (scOvr.level)
                {
                DgnV8Api::LevelId v8Level = v8eh.GetElementCP()->hdr.ehdr.level;

                if (0 != v8Level && DgnV8Api::LEVEL_BYCELL != v8Level)
                    {
                    DgnSubCategoryId subCategoryId = m_converter.GetSubCategory(v8Level, m_model.Is3d() ? SyncInfo::LevelExternalSourceAspect::Type::Spatial : SyncInfo::LevelExternalSourceAspect::Type::Drawing);
                    DgnCategoryId categoryId = DgnSubCategory::QueryCategoryId(m_converter.GetDgnDb(), subCategoryId);

                    geomParams.SetCategoryId(categoryId, false); // <- Don't clear appearance overrides...
                    geomParams.SetSubCategoryId(subCategoryId, false);
                    }
                }

            if (scOvr.color)
                {
                DgnV8Api::IntColorDef intColorDef;

                if (SUCCESS == DgnV8Api::DgnColorMap::ExtractElementColorInfo(&intColorDef, nullptr, nullptr, nullptr, nullptr, v8eh.GetElementCP()->hdr.dhdr.symb.color, *v8eh.GetDgnFileP()))
                    {
                    geomParams.SetLineColor(ColorDef(intColorDef.m_int));
                    geomParams.SetFillColor(ColorDef(intColorDef.m_int));
                    }
                }

            if (scOvr.weight)
                {
                UInt32  v8Weight = v8eh.GetElementCP()->hdr.dhdr.symb.weight;

                if (DgnV8Api::WEIGHT_BYCELL != v8Weight && DgnV8Api::WEIGHT_BYLEVEL != v8Weight)
                    geomParams.SetWeight(v8Weight);
                }

            if (scOvr.style)
                {
                Int32   v8Style = v8eh.GetElementCP()->hdr.dhdr.symb.style;

                if (DgnV8Api::STYLE_BYCELL != v8Style && DgnV8Api::STYLE_BYLEVEL != v8Style)
                    {
                    double      unitsScale;
                    DgnStyleId  mappedStyleId = m_converter._RemapLineStyle(unitsScale, *v8eh.GetDgnFileP(), v8Style, true);

                    if (mappedStyleId.IsValid())
                        {
                        LineStyleInfoPtr lsInfo = LineStyleInfo::Create(mappedStyleId, nullptr);

                        geomParams.SetLineStyle(lsInfo.get());
                        }
                    }
                }
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------
        struct IsMarkedEntry
            {
            IsMarkedEntry() {}
            bool operator()(DgnV8PathEntry& entry) const { return entry.m_isMarked; }
            }; // IsMarkedEntry

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------
        void DoGeometrySimplification()
            {
            for (DgnV8PathGeom& pathGeom : m_v8PathGeom)
                {
                if (pathGeom.m_entries.size() < 50)
                    continue; // Leave "small" amounts of geometry alone...

                size_t convexShapeCount = 0;
                DgnV8PathEntry* firstPathEntry = nullptr;

                for (DgnV8PathEntry& pathEntry : pathGeom.m_entries)
                    {
                    if (!pathEntry.m_is3dSrc || !pathEntry.m_is3dDest)
                        return; // If either source or destination is 2d, leave geometry alone...

                    if (!pathEntry.m_curve.IsValid())
                        continue;

                    if (!pathEntry.m_curve->IsClosedPath() || pathEntry.m_curve->ContainsNonLinearPrimitive())
                        continue; // Leave anything that isn't potentially just a shape alone...

                    pathEntry.m_curve->ConsolidateAdjacentPrimitives(false); // Don't remove co-linear points...

                    if (Bentley::ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString != pathEntry.m_curve->HasSingleCurvePrimitive())
                        continue;

                    if (Render::FillDisplay::Never != pathEntry.m_geomParams.GetFillDisplay() || nullptr != pathEntry.m_geomParams.GetPatternParams())
                        continue; // Don't create mesh from filled/patterned shapes...

                    Bentley::bvector<Bentley::DPoint3d> const* v8Points = pathEntry.m_curve->front()->GetLineStringCP();

                    // NOTE: Bentley::PolygonOps::IsConvex implementation didn't exist, just the declaration...
                    if (!bsiGeom_testPolygonConvex(&v8Points->front(), (int) v8Points->size()))
                        continue;

                    if (nullptr == firstPathEntry)
                        firstPathEntry = &pathEntry;
                    else if (!firstPathEntry->m_v8ToDgnDbTrans.IsEqual(pathEntry.m_v8ToDgnDbTrans))
                        continue; // Require same V8->DgnDb transform...
                    else if (!firstPathEntry->m_geomParams.IsEquivalent(pathEntry.m_geomParams))
                        continue; // Require same symbology...

                    pathEntry.m_isMarked = true;
                    convexShapeCount++;
                    }

                if (convexShapeCount < 25)
                    continue;

                Bentley::IFacetOptionsPtr facetOptions = Bentley::IFacetOptions::Create();
                Bentley::IPolyfaceConstructionPtr polyBuilder = Bentley::IPolyfaceConstruction::Create(*facetOptions);

                for (DgnV8PathEntry& pathEntry : pathGeom.m_entries)
                    {
                    if (!pathEntry.m_isMarked)
                        continue;

                    polyBuilder->AddTriangulation(*(const_cast<Bentley::bvector<Bentley::DPoint3d>*> (pathEntry.m_curve->front()->GetLineStringCP())));
                    }

                Bentley::PolyfaceHeaderPtr mesh = polyBuilder->GetClientMeshPtr();

                if (!mesh.IsValid())
                    continue;

                firstPathEntry->m_mesh = mesh;
                firstPathEntry->m_curve = nullptr;
                firstPathEntry->m_isMarked = false;

                pathGeom.m_entries.erase(std::remove_if(pathGeom.m_entries.begin(), pathGeom.m_entries.end(), IsMarkedEntry()), pathGeom.m_entries.end());
                }
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------
        bool IgnorePublicChildren(DgnV8EhCR v8eh)
            {
            ConvertToDgnDbElementExtension* upx = ConvertToDgnDbElementExtension::Cast(v8eh.GetHandler());

            if (nullptr != upx && upx->_IgnorePublicChildren())
                return true;
            for (auto xdomain : XDomainRegistry::s_xdomains)
                {
                if (xdomain->_IgnorePublicChildren())
                    return true;
                }
            return false;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------
        bool DisablePostInstancing(DgnV8EhCR v8eh)
            {
            ConvertToDgnDbElementExtension* upx = ConvertToDgnDbElementExtension::Cast(v8eh.GetHandler());

            if (nullptr != upx && upx->_DisablePostInstancing())
                return true;

            for (auto xdomain : XDomainRegistry::s_xdomains)
                {
                if (xdomain->_DisablePostInstancing())
                    return true;
                }

            return false;
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------
        bool AppendAsSubGraphics(DgnV8PathGeom& currPathGeom, DgnV8PathEntry& currPathEntry)
            {
            bool    done = false;
            bool    found = false;
            size_t  geom3dCount = 0;
            size_t  entryCount = 0;

            for (DgnV8PathGeom& pathGeom : m_v8PathGeom)
                {
                if (!found && (&pathGeom != &currPathGeom))
                    continue;

                for (DgnV8PathEntry& pathEntry : pathGeom.m_entries)
                    {
                    if (&pathEntry == &currPathEntry)
                        found = true;

                    if (!found)
                        continue;

                    // Since entries are grouped by category...can stop looking when category changes...
                    if (done = (pathEntry.m_geomParams.GetCategoryId() != currPathEntry.m_geomParams.GetCategoryId()))
                        break;

                    // NOTE: Don't create a sub-graphic for every geometric primitive from a dimension or dropped pattern...
                    if (DgnGeometryClass::Pattern == pathEntry.m_geomParams.GetGeometryClass() || DgnGeometryClass::Dimension == pathEntry.m_geomParams.GetGeometryClass())
                        continue;

                    // If we have potentially expensive 3d geometry to process, always include sub-graphic range...
                    if (pathEntry.m_primitive.IsValid() || pathEntry.m_surface.IsValid() || pathEntry.m_mesh.IsValid() || pathEntry.m_brep.IsValid())
                        {
                        if (++geom3dCount > 1)
                            return true;
                        }

                    if (++entryCount < 25)
                        continue;

                    return true;
                    }

                if (done)
                    break;
                }

            return false;
            }

    public:

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------
        void ProcessElement(DgnCategoryId targetCategoryId, DgnV8EhCR v8eh)
            {
            if (!v8eh.GetElementCP()->ehdr.isGraphics || v8eh.GetElementCP()->hdr.dhdr.props.b.invisible)
                return; // Skip displayable elements marked invisible as well as non-graphic components of cells (ex. type 38/39)...

            bool        isValidBasis = false;
            bool        isValidBasisAndScale = false;
            double      v8SymbolScale = 0.0;
            Transform   basisTransform;

            bvector<DgnV8PartReference> geomParts;
            DgnV8Api::ChildElemIter v8childEh(v8eh, DgnV8Api::ExposeChildrenReason::Query);

            if (v8childEh.IsValid() && !IgnorePublicChildren(v8eh))
                {
                for (; v8childEh.IsValid(); v8childEh = v8childEh.ToNext())
                    {
                    if (BentleyStatus::SUCCESS != m_converter.ConvertElement(v8childEh, m_builder, targetCategoryId))
                        continue; // Conversion of component with ECInstances did not produce any geometry, ignore it...
                    }
                return;
                }
            else
                {
                isValidBasis = GetBasisTransform(basisTransform, v8SymbolScale, v8eh);
                isValidBasisAndScale = (isValidBasis && 0.0 != v8SymbolScale);

                if (isValidBasisAndScale)
                    GetPartReferences(geomParts, v8SymbolScale, v8eh);

                if (geomParts.empty())
                    {
                    if (isValidBasisAndScale && DgnV8Api::SHARED_CELL_ELM == v8eh.GetElementType() && 0 != *((UInt16*) (&v8eh.GetElementCP()->sharedCell.m_override)))
                        {
                        DgnV8Api::EditElementHandle v8ehNoOvr(v8eh.GetElementCP(), v8eh.GetModelRef());
                        DgnV8Api::SCOverride* scOvr = &v8ehNoOvr.GetElementP()->sharedCell.m_override;

                        // Make sure part cache has geometry params that aren't affected by SCOverride...
                        scOvr->level = scOvr->color = scOvr->style = scOvr->weight = false;
                        DgnV8Api::ElementGraphicsOutput::Process(v8ehNoOvr, *this);
                        }
                    else
                        {
                        DgnV8Api::ElementGraphicsOutput::Process(v8eh, *this);
                        }

                    DoGeometrySimplification();
                    }
                }

            if (isValidBasisAndScale && !m_v8PathGeom.empty())
                CreatePartReferences(geomParts, basisTransform, v8SymbolScale);

            if (!geomParts.empty())
                {
                // NOTE: Level from shared cell instance that does not have a level override is
                //       meaningless except for components that use DgnV8Api::LEVEL_BYCELL.
                //       Getting a valid level to pass to ProcessElement would be expensive, so detect
                //       this situation and choose the category from the first part as necessary.
                //       This avoids creating un-necessary assemblies/parent element w/o Geometry...
                //       Purposely doesn't check if target is "GetUncategorizedCategory" as instance level
                //       doesn't have to be 0 (ex. relative level nonsense, etc.)
                bool targetCategoryValid = false;
                DgnCategoryId lastCategoryId = targetCategoryId;

                for (DgnV8PartReference& partRef : geomParts)
                    {
                    Transform geomToLocal = partRef.m_geomToLocal;
                    Render::GeometryParams geomParams = partRef.m_geomParams;

                    if (1.0 != fabs(partRef.m_scale))
                        {
                        double partScale = fabs(partRef.m_scale);

                        geomToLocal.ScaleMatrixColumns(geomToLocal, partScale, partScale, partScale);
                        }

                    ApplySharedCellInstanceOverrides(v8eh, geomParams); // Apply SCOverride now for a shared cell that was deemed ok for a GeometryPart...

                    if (!partRef.m_isLevelByCell)
                        lastCategoryId = geomParams.GetCategoryId();

                    if (!targetCategoryValid && targetCategoryId == lastCategoryId)
                        targetCategoryValid = true;

                    // If category changes, need to create a new element/assembly...
      //Needswork              if (builder.IsValid() && builder->GetGeometryParams().GetCategoryId() != lastCategoryId)
      //                  BuildNewElement(builder, elementClassId, targetCategoryId, elementCode, v8eh);

                   // if (!builder.IsValid())
                    //    builder = GeometryBuilder::Create(m_model, lastCategoryId, basisTransform);

                    m_builder->Append(geomParams);
                    m_builder->Append(partRef.m_partId, geomToLocal, partRef.m_localRange);
                    }

                return;
                }

            uint32_t    iEntry = 0;
            Transform   firstLocalToGeom = Transform::FromIdentity();
            Transform   firstGeomToWorld = Transform::FromIdentity();

            for (DgnV8PathGeom& pathGeom : m_v8PathGeom)
                {
                uint32_t iPathEntry = 0;
                DgnFileP dgnFile = pathGeom.m_path->GetRoot()->GetDgnFileP();

                for (DgnV8PathEntry& pathEntry : pathGeom.m_entries)
                    {
                    iPathEntry++;

                 //   if (0 == (++iEntry % 10))
                 //       m_converter.ReportProgress();

                    GeometricPrimitivePtr geometry = pathEntry.GetGeometry(*dgnFile, m_converter, false);

                    if (!geometry.IsValid())
                        continue;

                    // If category changes or GeometryStream is getting un-wieldy, need to create a new element/assembly...
      //Needswork              if (builder.IsValid() && (builder->GetGeometryParams().GetCategoryId() != pathEntry.m_geomParams.GetCategoryId() || builder->GetCurrentSize() > 20000000))
      //Needswork                  BuildNewElement(builder, elementClassId, targetCategoryId, elementCode, v8eh);

                    Transform   localToGeom;

                    if (!m_builder.IsValid())
                        {
                        Transform   localToWorld;

                        if (isValidBasis)
                            {
                            Transform   invBasisTrans, geomToLocal;

                            invBasisTrans.InverseOf(basisTransform);
                            geomToLocal = Transform::FromProduct(invBasisTrans, pathEntry.m_geomToWorld);
                            localToGeom.InverseOf(geomToLocal);
                            localToWorld = basisTransform; // Basis is the placement we want to use...
                            }
                        else
                            {
                            geometry->GetLocalCoordinateFrame(localToGeom);

                            // Ensure that we have a transform that will result in a valid Placement2d if destination is 2d...
                            pathEntry.FixupTransformForPlacement2d(localToGeom);

                            localToWorld = Transform::FromProduct(pathEntry.m_geomToWorld, localToGeom);
                            }

                        firstGeomToWorld = pathEntry.m_geomToWorld;
                        firstLocalToGeom = localToGeom;

                    //    builder = GeometryBuilder::Create(m_model, pathEntry.m_geomParams.GetCategoryId(), localToWorld);

                        if (!m_builder.IsValid())
                            {
                            //                  BeAssert(false); // Bad placement for 2d, invalid categoryId???
                            continue;
                            }
                                                    // NOTE: Don't create large multi-primitive GeometryStreams without calling SetAppendAsSubGraphics to store sub-ranges...
                        if (AppendAsSubGraphics(pathGeom, pathEntry))
                            m_builder->SetAppendAsSubGraphics();
                        }
                    else
                        {
                        // First piece of geometry added to builder established the frame transform...
                        if (!firstGeomToWorld.IsEqual(pathEntry.m_geomToWorld))
                            {
                            Transform   worldToFirstGeom, geomToFirstGeom, firstGeomToFirstLocal, geomToFirstLocal;

                            worldToFirstGeom.InverseOf(firstGeomToWorld);
                            geomToFirstGeom = Transform::FromProduct(worldToFirstGeom, pathEntry.m_geomToWorld);
                            firstGeomToFirstLocal.InverseOf(firstLocalToGeom);
                            geomToFirstLocal = Transform::FromProduct(firstGeomToFirstLocal, geomToFirstGeom);
                            localToGeom.InverseOf(geomToFirstLocal);
                            }
                        else
                            {
                            localToGeom = firstLocalToGeom;
                            }
                        }

                    if (!localToGeom.IsIdentity())
                        {
                        Transform geomToLocal;

                        geomToLocal.InverseOf(localToGeom);
                        geometry->TransformInPlace(geomToLocal);
                        pathEntry.m_geomParams.ApplyTransform(geomToLocal, 0x01); // <- Don't scale linestyles...
                        }

                    m_builder->Append(pathEntry.m_geomParams);
                    m_builder->Append(*geometry);
                    }
                }
            }

        //---------------------------------------------------------------------------------
        // @bsimethod                                                    Vern.Francisco   12/17
        //---------------+---------------+---------------+---------------+---------------+------
        V8GraphicsLightWeightCollector(LightWeightConverter& converter, DgnModelR model, Transform matrix, GeometryBuilderPtr builder) :
            m_converter(converter), m_model(model), m_context(nullptr), m_inBRepConvertFaces(false)
            {
            // NEEDSWORK scale should be 1.0/uorpermeter
            //DgnV8Api::ModelInfo const& v8ModelInfo = activeModelRef->GetRoot()->GetModelInfo();
            //double uorPerMeter = DgnV8Api::ModelInfo::GetUorPerMeter(&v8ModelInfo);
            //Bim02Bentley::Transform matrix;
            //matrix.InitFromScaleFactors(1.0 / uorPerMeter, 1.0 / uorPerMeter, 1.0 / uorPerMeter);

            m_conversionScale = DoInterop(matrix);
            m_builder = builder;
            }

    }; // V8GraphicsCollector

//-------------------------------------------------------------------------------------------------
// @bsimethod                                                    Vern.Francisco 12/17
//+-------------- - +-------------- - +-------------- - +-------------- - +-------------- - +------
BentleyStatus LightWeightConverter::_CreateElementGeom(DgnCategoryId targetCategoryId, DgnV8EhCR v8eh, GeometryBuilderPtr builder)
    {
    // NEEDSWORK scale should be 1.0/uorpermeter
    DgnV8Api::ModelInfo const& v8ModelInfo = m_v8Model->GetModelInfo();
    double uorPerMeter = DgnV8Api::ModelInfo::GetUorPerMeter(&v8ModelInfo);
    Transform matrix;
    matrix.InitFromScaleFactors(1.0 / uorPerMeter, 1.0 / uorPerMeter, 1.0 / uorPerMeter);


    V8GraphicsLightWeightCollector collector(*this, _GetModel(), matrix, builder);

    collector.ProcessElement( targetCategoryId, v8eh);

    //  return results.m_element.IsValid() ? BSISUCCESS : BSIERROR;
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------
// @bsimethod                                                    Vern.Francisco   12/17
//---------------+---------------+---------------+---------------+---------------+------

bool LightWeightConverter::InitPatternParams(PatternParamsR pattern, DgnV8Api::PatternParams const& patternV8, Bentley::bvector<DgnV8Api::DwgHatchDefLine> const& defLinesV8, Bentley::DPoint3d& origin, DgnV8Api::ViewContext& context)
    {
    double uorPerMeter = DgnV8Api::ModelInfo::GetUorPerMeter(&context.GetCurrentModel()->GetDgnModelP()->GetModelInfo());

    if (DgnV8Api::PatternParamsModifierFlags::None != (patternV8.modifiers & DgnV8Api::PatternParamsModifierFlags::Cell))
        {
        Utf8String nameStr;
        nameStr.Assign(patternV8.cellName);

        Utf8PrintfString partCodeValue("PatternV8-%ld-%s-%lld", m_converterId, nameStr.c_str(), patternV8.cellId);
        DgnCode partCode = CreateCode(partCodeValue);
        DgnGeometryPartId partId = DgnGeometryPart::QueryGeometryPartId(*GetJobDefinitionModel(), partCode.GetValueUtf8());

        if (!partId.IsValid())
            {
            DgnV8Api::ElementHandle v8Eh(patternV8.cellId, context.GetCurrentModel());

            if (!v8Eh.IsValid())
                return false;

            bool isPointCell = (!v8Eh.GetElementCP()->hdr.dhdr.props.b.s && v8Eh.GetElementCP()->hdr.dhdr.props.b.r);
            double scale = 1.0 / uorPerMeter;
            Transform scaleTransform = Transform::FromScaleFactors(scale, scale, scale);
            LightWeightV8SymbolGraphicsCollector collector( *this, &(GetDgnDb()), scaleTransform, m_defaultCategoryId, m_defaultSubCategoryId);

            if (!isPointCell)
                collector.SetAllowMultiSymb();

            DgnV8Api::ElementGraphicsOutput::Process(v8Eh, collector);
            bvector<GeometricPrimitivePtr>& geometry = collector.GetSymbolGeometry();

            if (0 == geometry.size())
                return false;

            size_t iSymb = 0;
            GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(GetDgnDb(), true);

            for (GeometricPrimitivePtr geom : geometry)
                {
                if (!isPointCell)
                    builder->Append(collector.GetSymbolDisplayParams().at(iSymb++));

                builder->Append(*geom);
                }

            DgnGeometryPartPtr geomPart = DgnGeometryPart::Create(*GetJobDefinitionModel(), partCode.GetValueUtf8());

            if (SUCCESS != builder->Finish(*geomPart) || !GetDgnDb().Elements().Insert<DgnGeometryPart>(*geomPart).IsValid())
                return false;

            partId = geomPart->GetId();
            }

        pattern.SetSymbolId(partId);
        pattern.SetPrimarySpacing(patternV8.space1);
        pattern.SetSecondarySpacing(patternV8.space2);
        pattern.SetPrimaryAngle(patternV8.angle1);
        pattern.SetScale(patternV8.scale * uorPerMeter); // Need uor scale, not user scale...full V8 to DgnDb transform is applied to PatternParams later...
        }
    else if (DgnV8Api::PatternParamsModifierFlags::None != (patternV8.modifiers & DgnV8Api::PatternParamsModifierFlags::DwgHatchDef))
        {
        bvector<DwgHatchDefLine> dwgHatchDef;

        for (auto defLineV8 : defLinesV8)
            {
            DwgHatchDefLine defLine;

            defLine.m_angle = defLineV8.angle;
            defLine.m_through = DoInterop(defLineV8.through);
            defLine.m_offset = DoInterop(defLineV8.offset);
            defLine.m_nDashes = (defLineV8.nDashes < MAX_DWG_HATCH_LINE_DASHES ? defLineV8.nDashes : MAX_DWG_HATCH_LINE_DASHES);

            for (short iDash = 0; iDash < defLine.m_nDashes; iDash++)
                defLine.m_dashes[iDash] = defLineV8.dashes[iDash];

            dwgHatchDef.push_back(defLine);
            }

        pattern.SetDwgHatchDef(dwgHatchDef);
        pattern.SetPrimaryAngle(patternV8.angle1); // NOTE: angle/scale baked into hatch def lines, saved for placement info...
        pattern.SetScale(patternV8.scale * uorPerMeter); // Need uor scale, not user scale...full V8 to DgnDb transform is applied to PatternParams later...

        if (DgnV8Api::PatternParamsModifierFlags::None == (patternV8.modifiers & DgnV8Api::PatternParamsModifierFlags::DwgHatchOrigin))
            {
            // Old style DWG Hatch Definitions are implicitly about (0,0), need to ignore "element" origin...
            patternV8.rMatrix.MultiplyTranspose(origin);
            origin.x = origin.y = 0.0;
            patternV8.rMatrix.Multiply(origin);
            }
        }
    else
        {
        pattern.SetPrimarySpacing(patternV8.space1);
        pattern.SetPrimaryAngle(patternV8.angle1);

        if (DgnV8Api::PatternParamsModifierFlags::None != (patternV8.modifiers & DgnV8Api::PatternParamsModifierFlags::Space2))
            {
            pattern.SetSecondarySpacing(patternV8.space2);
            pattern.SetSecondaryAngle(patternV8.angle2);
            }
        }

    pattern.SetOrientation(DoInterop(patternV8.rMatrix));
    pattern.SetOrigin(DoInterop(origin));

    if (DgnV8Api::PatternParamsModifierFlags::None != (patternV8.modifiers & DgnV8Api::PatternParamsModifierFlags::Color))
        {
        DgnV8Api::IntColorDef intColorDef;

        if (SUCCESS == DgnV8Api::DgnColorMap::ExtractElementColorInfo(&intColorDef, nullptr, nullptr, nullptr, nullptr, patternV8.color, *context.GetCurrentModel()->GetDgnFileP()))
            pattern.SetColor(ColorDef(intColorDef.m_int));
        }

    if (DgnV8Api::PatternParamsModifierFlags::None != (patternV8.modifiers & DgnV8Api::PatternParamsModifierFlags::Weight))
        pattern.SetWeight(patternV8.weight);

    pattern.SetSnappable(DgnV8Api::PatternParamsModifierFlags::None != (patternV8.modifiers & DgnV8Api::PatternParamsModifierFlags::Snap));

    return true;
    }


//---------------------------------------------------------------------------------
// @bsimethod                                                    Vern.Francisco   12/17
//---------------+---------------+---------------+---------------+---------------+------
void LightWeightConverter::InitLineStyle(Render::GeometryParams& params, DgnModelRefR styleModelRef, int32_t srcLineStyleNum, DgnV8Api::LineStyleParams const* v8lsParams)
    {
    DgnFileP styleFile = styleModelRef.GetDgnFileP();
    if (nullptr == styleFile)
        {
    //    ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::MissingLsDefinitionFile(), NULL,
        //                IssueReporter::FmtModelRef(styleModelRef));
        return;
        }

    double          unitsScale;

    DgnStyleId mappedStyleId = _RemapLineStyle(unitsScale, *styleFile, srcLineStyleNum, true);
    if (!mappedStyleId.IsValid())
        return;

    double modelLsScale = styleModelRef.GetRoot()->GetLineStyleScale();
    DgnV8Api::ModelInfo const& v8ModelInfo = styleModelRef.GetRoot()->GetModelInfo();
    double uorPerMeter = DgnV8Api::ModelInfo::GetUorPerMeter(&v8ModelInfo);
    Render::LineStyleParams lsParams;
    ConvertLineStyleParams(lsParams, v8lsParams, uorPerMeter, unitsScale, modelLsScale);
    LineStyleInfoPtr lsInfo = LineStyleInfo::Create(mappedStyleId, &lsParams);
    params.SetLineStyle(lsInfo.get());
    }


//---------------------------------------------------------------------------------
// @bsimethod                                                    Vern.Francisco   12/17
//---------------+---------------+---------------+---------------+---------------+------
void LightWeightConverter::InitGeometryParams(Render::GeometryParams& params, DgnV8Api::ElemDisplayParams& paramsV8, DgnV8Api::ViewContext& context, bool is3d, DgnV8ModelCR v8Model)
    {
    // NOTE: Resolve loses information like WEIGHT_BYLEVEL and we may be called multiple times (ex. disjoint brep)...
    AutoRestore<DgnV8Api::ElemDisplayParams> saveParamsV8(&paramsV8);

    UInt32  rawColor = paramsV8.GetLineColor();
    UInt32  rawFill = paramsV8.GetFillColor();
    UInt32  rawWeight = paramsV8.GetWeight();
    Int32   rawStyle = paramsV8.GetLineStyle();

    // Apply ignores and resolve effective now that we've saved off the ByLevel information...
    paramsV8.Resolve(context);

    // NOTE: ElemHeaderOverrides have been pushed but have not been applied to paramsV8...
    DgnV8Api::LevelId v8Level = (0 == paramsV8.GetLevel() ? DGNV8_LEVEL_DEFAULT_LEVEL_ID : paramsV8.GetLevel());
    DgnV8Api::ElemHeaderOverrides const* ovr = context.GetHeaderOvr();

    if (nullptr != ovr && ovr->GetFlags().level)
        v8Level = ovr->AdjustLevel(v8Level, *context.GetCurrentModel());

    DgnSubCategoryId subCategoryId = params.GetSubCategoryId(); // see if the caller wants to specify an override SubCategoryId
    if (!subCategoryId.IsValid()) // if not, look up the SubCategoryId from V8 LevelId
        subCategoryId = GetSubCategory(v8Level, v8Model, is3d ? SyncInfo::LevelExternalSourceAspect::Type::Spatial : SyncInfo::LevelExternalSourceAspect::Type::Drawing);

    DgnCategoryId categoryId = DgnSubCategory::QueryCategoryId(GetDgnDb(), subCategoryId);

    params = Render::GeometryParams();
    params.SetCategoryId(categoryId);
    params.SetSubCategoryId(subCategoryId);

    if (!is3d)
        {
        if (DgnV8Api::DISPLAYPRIORITY_BYCELL != paramsV8.GetElementDisplayPriority())
            params.SetDisplayPriority(paramsV8.GetElementDisplayPriority());
        else if (nullptr != ovr)
            params.SetDisplayPriority(ovr->GetDisplayPriority());
        }

    switch (paramsV8.GetElementClass())
        {
            case DgnV8Api::DgnElementClass::Primary:
            case DgnV8Api::DgnElementClass::PrimaryRule: // Shouldn't see this normally...
                params.SetGeometryClass(Render::DgnGeometryClass::Primary);
                break;

            case DgnV8Api::DgnElementClass::Construction:
            case DgnV8Api::DgnElementClass::ConstructionRule: // Shouldn't see this normally...
                params.SetGeometryClass(Render::DgnGeometryClass::Construction);
                break;

            case DgnV8Api::DgnElementClass::Dimension:
                params.SetGeometryClass(Render::DgnGeometryClass::Dimension);
                break;

            case DgnV8Api::DgnElementClass::PatternComponent:
                params.SetGeometryClass(Render::DgnGeometryClass::Pattern);
                break;

            case DgnV8Api::DgnElementClass::LinearPatterned: // Don't intend to support this anymore...
            default:
                params.SetGeometryClass(Render::DgnGeometryClass::Primary);
                break;
        }

    if (nullptr != ovr && ovr->GetFlags().color)
        {
        if (DgnV8Api::COLOR_BYLEVEL != ovr->GetColor())
            {
            DgnV8Api::IntColorDef intColorDef;

            if (SUCCESS == DgnV8Api::DgnColorMap::ExtractElementColorInfo(&intColorDef, nullptr, nullptr, nullptr, nullptr, ovr->GetColor(), *context.GetCurrentModel()->GetDgnFileP()))
                params.SetLineColor(ColorDef(intColorDef.m_int));
            }
        }
    else if (DgnV8Api::COLOR_BYLEVEL != rawColor)
        {
        params.SetLineColor(ColorDef(paramsV8.GetLineColorTBGR()));
        }

    if (nullptr != ovr && ovr->GetFlags().weight)
        {
        if (DgnV8Api::WEIGHT_BYLEVEL != ovr->GetWeight())
            params.SetWeight(ovr->GetWeight());
        }
    else if (DgnV8Api::WEIGHT_BYLEVEL != rawWeight)
        {
        params.SetWeight(paramsV8.GetWeight());
        }

    DgnModelRefP styleModelRef;

    if (nullptr != ovr && ovr->GetFlags().style && nullptr != (styleModelRef = (nullptr == ovr->GetLineStyleModelRef()) ? context.GetCurrentModel() : ovr->GetLineStyleModelRef()))
        {
        if (DgnV8Api::STYLE_BYLEVEL != ovr->GetLineStyle() && ovr->GetLineStyle() != 0)
            {
            InitLineStyle(params, *styleModelRef, ovr->GetLineStyle(), ovr->GetLineStyleParams());
            }
        }
    else if (paramsV8.GetLineStyle() != 0 && nullptr != (styleModelRef = (nullptr == paramsV8.GetLineStyleModelRef()) ? context.GetCurrentModel() : paramsV8.GetLineStyleModelRef()))
        {
        DgnV8Api::LineStyleParams const* lsParamsV8 = paramsV8.GetLineStyleParams();

        // Ugh...still need modifiers like start/end width for STYLE_BYLEVEL... :(
        if (DgnV8Api::STYLE_BYLEVEL != rawStyle || (lsParamsV8 && 0 != lsParamsV8->modifiers))
            InitLineStyle(params, *styleModelRef, paramsV8.GetLineStyle(), lsParamsV8);
        }

    params.SetTransparency(paramsV8.GetTransparency());

    if (DgnV8Api::FillDisplay::Never != paramsV8.GetFillDisplay())
        {
        params.SetFillDisplay((Render::FillDisplay) paramsV8.GetFillDisplay());

        if (nullptr != paramsV8.GetGradient())
            {
            DgnV8Api::GradientSymb const& gradient = *paramsV8.GetGradient();
            Render::GradientSymbPtr gradientPtr = Render::GradientSymb::Create();

            gradientPtr->SetMode((Render::GradientSymb::Mode) gradient.GetMode());
            gradientPtr->SetFlags((Render::GradientSymb::Flags) gradient.GetFlags());
            gradientPtr->SetShift(gradient.GetShift());
            gradientPtr->SetTint(gradient.GetTint());
            gradientPtr->SetAngle(gradient.GetAngle());

            bvector<ColorDef> keyColors;
            bvector<double>   keyValues;

            for (int i = 0; i < gradient.GetNKeys(); i++)
                {
                double keyValue;
                Bentley::RgbColorDef keyColor;

                gradient.GetKey(keyColor, keyValue, i);

                DgnDbApi::ColorDef keyColorDef(keyColor.red, keyColor.green, keyColor.blue);

                keyColors.push_back(keyColorDef);
                keyValues.push_back(keyValue);
                }

            gradientPtr->SetKeys((uint16_t) keyColors.size(), &keyColors.front(), &keyValues.front());

            params.SetGradient(gradientPtr.get());
            }
        else
            {
            if (nullptr != ovr && ovr->GetFlags().color)
                {
                if (DgnV8Api::COLOR_BYLEVEL != ovr->GetColor())
                    {
                    DgnV8Api::IntColorDef intColorDef;

                    if (SUCCESS == DgnV8Api::DgnColorMap::ExtractElementColorInfo(&intColorDef, nullptr, nullptr, nullptr, nullptr, ovr->GetColor(), *context.GetCurrentModel()->GetDgnFileP()))
                        params.SetFillColor(ColorDef(intColorDef.m_int));
                    }
                }
            else if (DgnV8Api::DgnColorMap::INDEX_Background == rawFill)
                {
                params.SetFillColorFromViewBackground(DgnV8Api::DgnColorMap::INDEX_Background != rawColor);
                }
            else if (DgnV8Api::COLOR_BYLEVEL != rawFill)
                {
                params.SetFillColor(ColorDef(paramsV8.GetFillColorTBGR()));
                }
            }
        }

    if (paramsV8.IsRenderable())
        {
        if (nullptr != paramsV8.GetMaterial())
            {
            RenderMaterialId materialId = GetRemappedMaterial(paramsV8.GetMaterial());
            DgnSubCategoryCPtr          dgnDbSubCategory = DgnSubCategory::Get(GetDgnDb(), subCategoryId);

            if (!dgnDbSubCategory.IsValid() || dgnDbSubCategory->GetAppearance().GetRenderMaterial() != materialId)
                params.SetMaterialId(materialId);

            if (nullptr != paramsV8.GetMaterialUVDetailP())
                {
                // params.SetMaterialUVDetails();
                }
            }
        }

    params.Resolve(GetDgnDb()); // Need to be able to check for a stroked linestyle...
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco     01/2018
//!
//! This function is a clone of the Converter methods to support the LigthWeightConverter
//!
//---------------------------------------------------------------------------------------

void LightWeightConverter::ConvertLineStyleParams(Render::LineStyleParams& lsParams, DgnV8Api::LineStyleParams const* v8lsParams, double uorPerMeter, double componentScale, double modelLsScale)
    {
    if (nullptr == v8lsParams)
        {
        lsParams.Init();
        return;
        }

    if (uorPerMeter == 0.0)
        {
        BeAssert(uorPerMeter != 0.0);
        uorPerMeter = 1;
        }

    lsParams.modifiers = v8lsParams->modifiers; //   & ~STYLEMOD_TRUE_WIDTH;
    lsParams.reserved = 0;
    lsParams.scale = v8lsParams->scale;
    lsParams.dashScale = v8lsParams->dashScale;
    lsParams.gapScale = v8lsParams->gapScale;
    lsParams.fractPhase = v8lsParams->fractPhase;

    if (1.0 != modelLsScale)
        {
        if (lsParams.scale)
            lsParams.scale *= modelLsScale;
        else
            lsParams.scale = modelLsScale;

        lsParams.modifiers |= STYLEMOD_SCALE;
        }

    lsParams.distPhase = v8lsParams->distPhase;

    if (0 != (v8lsParams->modifiers & STYLEMOD_TRUE_WIDTH))
        {
        //  The value was in UORs.  In DgnDb it is in meters
        lsParams.startWidth = v8lsParams->startWidth / uorPerMeter;
        lsParams.endWidth = v8lsParams->endWidth / uorPerMeter;
        }
    else
        {
        //  The value is in line style units. Convert it to the new line style units.
        lsParams.startWidth = v8lsParams->startWidth*componentScale;
        lsParams.endWidth = v8lsParams->endWidth*componentScale;
        }

    lsParams.normal = DoInterop(v8lsParams->normal);
    lsParams.rMatrix = DoInterop(v8lsParams->rMatrix);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco     01/2018
//!
//! This function is a clone of the Converter methods to support the LigthWeightConverter
//!
//---------------------------------------------------------------------------------------
void LightWeightConverter::ConvertTextString (TextStringPtr& clone, Bentley::TextStringCR v8Text, DgnFileR dgnFile, LightWeightConverter& converter)
    {
    uint32_t v8FontId = 0;
    dgnFile.GetDgnFontMapP ()->GetFontNumber (v8FontId, v8Text.GetProperties ().GetFont (), false);

    DgnFont const& dbFont = converter._RemapV8Font (dgnFile, v8FontId);
    Utf8String dbTextValue (v8Text.GetString ());

    DgnDbApi::TextString dbText;
    dbText.SetText (dbTextValue.c_str ());
    dbText.SetOrigin (DoInterop (v8Text.GetOrigin ()));
    dbText.SetOrientation (DoInterop (v8Text.GetRotMatrix ()));
    dbText.GetStyleR ().SetFont (dbFont);
    dbText.GetStyleR ().SetIsBold (v8Text.GetProperties ().IsBold ());
    dbText.GetStyleR ().SetIsItalic (v8Text.GetProperties ().IsItalic ());
    dbText.GetStyleR ().SetSize (DoInterop (v8Text.GetProperties ().GetFontSize ()));

    // DgnV8 sub-/super-script is a hard-coded display-time scale and shift.
    if (v8Text.GetProperties ().IsSubScript () || v8Text.GetProperties ().IsSuperScript ())
        {
        DPoint2d scaledSize = dbText.GetStyle ().GetSize ();
        scaledSize.Scale (0.3);
        dbText.GetStyleR ().SetSize (scaledSize);
        }
    // Because DgnV8 has unsupported underline (and overline) styles, never tell this hacked DB TextString to draw an underline even if it's present, and draw it manually ourselves.

    // Internal implementation detail: A DgnV8 TextString will report 0 glyphs unless the caller performed layout with a listener that claimed to capture the glyphs.
    struct ShimGlyphLayoutListener : DgnV8Api::IDgnGlyphLayoutListener
        {
        virtual void _OnGlyphAnnounced (DgnV8Api::DgnGlyph&, Bentley::DPoint3d const&) override {}
        virtual UInt32 _OnFontAnnounced (DgnV8Api::TextString const&) override { return 0; }
        virtual bool _DidCacheGlyphs () override { return true; }
        };
    static ShimGlyphLayoutListener s_shimGlyphLayoutListener;

    // Force the DgnV8 TextString to do its layout pass.
    v8Text.LoadGlyphs (&s_shimGlyphLayoutListener);

    // Mark the DB TextString as valid so it doesn't try to perform its own layout later.
    dbText.m_isValid = true;

    // Directly copy the DgnV8 TextString's layout information into the DB TextString's cache.
    size_t v8NumGlyphs = v8Text.GetNumGlyphs ();
    dbText.m_glyphs.resize (v8NumGlyphs);
    dbText.m_glyphIds.resize (v8NumGlyphs);
    dbText.m_glyphOrigins.resize (v8NumGlyphs);

    // Has the side effect of loading the font data, which is required for FindGlyphCP anyway.
    if (!dbText.GetStyle ().GetFont ().IsResolved ())
        {
        BeAssert (false);
        }

    DgnFontStyle dbFontStyle = DgnFont::FontStyleFromBoldItalic (dbText.GetStyle ().IsBold (), dbText.GetStyle ().IsItalic ());

    // N.B. Ensure to use the TextString's font object. It took steps to resolve the font (vs. this
    // function's local dbFont variable), and this data needs to match its exact font.
    DgnFontCR resolvedDbFont = dbText.GetStyle ().GetFont ();

    // In terms of a glyph ID within a TT font, PowerPlatform supports both glyph and character
    // index, but does not currently expose what "glyph code" actually means in the public API (only
    // DgnTrueTypeGlyph actually retains this information, which is in a CPP file). It's unclear to
    // me in Uniscribe's documentation if setting SCRIPT_ANALYSIS::fNoGlyphIndex to FALSE
    // necessarily forces use of glyph indices, but it's highly suggestive. PP normally sets to
    // FALSE, except if !T_HOST.GetDgnFontManager().IsGlyphShapingEnabled() ||
    // layoutContext.IsVertical(), so that's as good as we can get unless/until we can change PP's
    // API to explicitly expose this information.
    bvector<unsigned int> derivedGlyphIndices;
    bool useDerivedGlyphIndices = false;
    if ((DgnFontType::TrueType == resolvedDbFont.GetType ())
        && (!DgnV8Api::DgnFontManager::GetManager ().IsGlyphShapingEnabled ()
            || v8Text.GetProperties ().IsVertical ()))
        {
        useDerivedGlyphIndices = true;
        derivedGlyphIndices = ((DgnTrueTypeFontCR)resolvedDbFont).ComputeGlyphIndices (dbText.GetText ().c_str (), dbText.GetStyle ().IsBold (), dbText.GetStyle ().IsItalic ());

        // I can't image how this would differ, but I'd rather be defensive since we'll use
        // v8Text.GetNumGlyphs as loop control below.
        BeAssert (derivedGlyphIndices.size () == v8Text.GetNumGlyphs ());
        if (derivedGlyphIndices.size () < v8Text.GetNumGlyphs ())
            {
            // Fill with 0's... better than crashing.
            derivedGlyphIndices.resize (v8Text.GetNumGlyphs ());
            }
        }

    for (size_t iV8Glyph = 0; iV8Glyph < v8Text.GetNumGlyphs (); ++iV8Glyph)
        {
        DgnGlyph::T_Id resolvedGlyphId = useDerivedGlyphIndices ? derivedGlyphIndices[iV8Glyph] : v8Text.GetGlyphCodes ()[iV8Glyph];

        dbText.m_glyphIds[iV8Glyph] = resolvedGlyphId;
        dbText.m_glyphs[iV8Glyph] = resolvedDbFont.FindGlyphCP (resolvedGlyphId, dbFontStyle);
        dbText.m_glyphOrigins[iV8Glyph] = DoInterop (v8Text.GetGlyphOrigins ()[iV8Glyph]);
        }

    dbText.m_range.low = DoInterop (v8Text.GetExtents ().low);
    dbText.m_range.high = DoInterop (v8Text.GetExtents ().high);

    clone = dbText.Clone ();
    }


#ifdef TEST_AUXDATA
#ifdef TEST_AUXDATA_WAVES

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void addRadialWaveChannels(PolyfaceAuxData::ChannelsR channels, Bentley::PolyfaceQueryCR polyface)
    {
    bvector<double>     zeroScalarData(polyface.GetPointCount(), 0.0), scalarHeightData, scalarSlopeData;
    bvector<double>     zeroDisplacementData(3*polyface.GetPointCount(), 0.0), displacementData;
    DPoint3dCP          points = DoInterop(polyface.GetPointCP());
    DRange3d            range = DRange3d::From(points, polyface.GetPointCount());
    DPoint3d            center = DPoint3d::FromInterpolate(range.low, .5, range.high);
    double              radius = range.DiagonalDistance() / 2.0;
    double              maxHeight = radius/10.0;


    for(size_t i=0; i<polyface.GetPointCount(); i++)
        {
        DPoint3dCR  point = points[i];
        double      angle = point.Distance(center) / radius * msGeomConst_2pi;
        double      height = maxHeight * sin(angle);
        double      slope = fabs(cos(angle));

        scalarHeightData.push_back(height);
        scalarSlopeData.push_back(slope);
        displacementData.push_back(0.0);
        displacementData.push_back(0.0);
        displacementData.push_back(height);
        }

    bvector<PolyfaceAuxChannel::DataPtr>   displacementDataVector, scalarHeightDataVector, scalarSlopeDataVector;

    displacementDataVector.push_back(new PolyfaceAuxChannel::Data(0.0, std::move(zeroDisplacementData)));
    displacementDataVector.push_back(new PolyfaceAuxChannel::Data(1.0, std::move(displacementData)));
    displacementDataVector.push_back(new PolyfaceAuxChannel::Data(2.0, std::move(zeroDisplacementData)));

    scalarHeightDataVector.push_back(new PolyfaceAuxChannel::Data(0.0, std::move(zeroScalarData)));
    scalarHeightDataVector.push_back(new PolyfaceAuxChannel::Data(1.0, std::move(scalarHeightData)));
    scalarHeightDataVector.push_back(new PolyfaceAuxChannel::Data(2.0, std::move(zeroScalarData)));

    scalarSlopeDataVector.push_back(new PolyfaceAuxChannel::Data(0.0, std::move(zeroScalarData)));
    scalarSlopeDataVector.push_back(new PolyfaceAuxChannel::Data(1.0, std::move(scalarSlopeData)));
    scalarSlopeDataVector.push_back(new PolyfaceAuxChannel::Data(2.0, std::move(zeroScalarData)));


    channels.push_back (new PolyfaceAuxChannel(PolyfaceAuxChannel::DataType::Vector, "Radial Displacement", "Radial: Time", std::move(displacementDataVector)));
    channels.push_back (new PolyfaceAuxChannel(PolyfaceAuxChannel::DataType::Scalar, "Radial Height", "Radial: Time", std::move(scalarHeightDataVector)));
    channels.push_back (new PolyfaceAuxChannel(PolyfaceAuxChannel::DataType::Scalar, "Radial Slope", "Radial: Time", std::move(scalarSlopeDataVector)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void addLinearWaveChannels(PolyfaceAuxData::ChannelsR channels, Bentley::PolyfaceQueryCR polyface)
    {
    bvector<PolyfaceAuxChannel::DataPtr>   displacementDataVector, heightDataVector, slopeDataVector;
    DPoint3dCP          points = DoInterop(polyface.GetPointCP());
    DRange3d            range = DRange3d::From(points, polyface.GetPointCount());
    double              rangeDiagonal = range.DiagonalDistance();

    static double       s_waveHeight = rangeDiagonal / 20.0;
    static double       s_waveLength = rangeDiagonal / 2.0;
    static size_t       s_frameCount = 10;

    for (size_t i=0; i<s_frameCount; i++)
        {
        double              fraction = (double) i / (double) (s_frameCount - 1);
        double              waveCenter = s_waveLength * fraction;
        bvector<double>     heightData, slopeData, displacementData;

        for(size_t i=0; i<polyface.GetPointCount(); i++)
            {
            DPoint3dCR  point = points[i];
            double      theta = msGeomConst_2pi * (point.x - waveCenter) / s_waveLength;
            double      height = s_waveHeight * sin(theta);
            double      slope = fabs(cos(theta));

            heightData.push_back(height);
            slopeData.push_back(slope);
            displacementData.push_back(0.0);
            displacementData.push_back(0.0);
            displacementData.push_back(height);
            }
        displacementDataVector.push_back(new PolyfaceAuxChannel::Data(i, std::move(displacementData)));
        heightDataVector.push_back(new PolyfaceAuxChannel::Data(i, std::move(heightData)));
        slopeDataVector.push_back(new PolyfaceAuxChannel::Data(i, std::move(slopeData)));
        }

    PolyfaceAuxChannelPtr       displacementChannel = new PolyfaceAuxChannel(PolyfaceAuxChannel::DataType::Vector, "Linear: Displacement", "Linear: Time", std::move(displacementDataVector));
    PolyfaceAuxChannelPtr       heightChannel = new PolyfaceAuxChannel(PolyfaceAuxChannel::DataType::Scalar, "Linear: Height", "Linear: Time", std::move(heightDataVector));
    PolyfaceAuxChannelPtr       slopeChannel = new PolyfaceAuxChannel(PolyfaceAuxChannel::DataType::Scalar, "Linear: Slope", "Linear: Time", std::move(slopeDataVector));

    channels.push_back (displacementChannel);
    channels.push_back (heightChannel);
    channels.push_back (slopeChannel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void getTestAuxData(Bentley::PolyfaceQueryCR polyface)
    {
    bvector<int32_t>            indices(polyface.GetPointIndexCount());
    PolyfaceAuxData::Channels   channels;

    memcpy (indices.data(), polyface.GetPointIndexCP(), indices.size() * sizeof(int32_t));

    addLinearWaveChannels(channels, polyface);
    addRadialWaveChannels(channels, polyface);
    s_testAuxData = new PolyfaceAuxData(std::move(indices), std::move(channels));
    }

#endif

#ifdef TEST_AUXDATA_SPHERICAL_WITH_NORMALS

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void getTestAuxData(Bentley::PolyfaceQueryCR polyface, DRange1dR scalarRange, double v8ToDbScale)
    {
    bvector<int32_t>    indices(polyface.GetPointIndexCount());
    bvector<double>     zeroScalarData(polyface.GetPointCount(), 0.0), scalarData;
    bvector<double>     zeroDisplacementData(3*polyface.GetPointCount(), 0.0), displacementData;
    bvector<double>     initialNormalData, normalData;
    DPoint3dCP          points = DoInterop(polyface.GetPointCP());
    DRange3d            range = DRange3d::From(points, polyface.GetPointCount());
    DPoint3d            center = DPoint3d::FromInterpolate(range.low, .5, range.high);
    double              radius = range.DiagonalDistance() / 2.0;
    double              maxHeight = radius/10.0;

    memcpy (indices.data(), polyface.GetPointIndexCP(), indices.size() * sizeof(int32_t));

    for(size_t i=0; i<polyface.GetPointCount(); i++)
        {
        DPoint3dCR  point = points[i];
        DVec3d      delta = DVec3d::FromStartEnd(point, center);
        double      x = point.Distance(center);
        double      height = sqrt(radius * radius - x * x);
        DVec3d      normal;                                                               dg

        scalarData.push_back(height);
        displacementData.push_back(0.0);
        displacementData.push_back(0.0);
        displacementData.push_back(height);

        normal = DVec3d::From(delta.x, delta.y, height);
        normal.Normalize();
        normalData.push_back(normal.x);
        normalData.push_back(normal.y);
        normalData.push_back(normal.z);

        initialNormalData.push_back(0.0);
        initialNormalData.push_back(0.0);
        initialNormalData.push_back(1.0);
        }
    scalarRange = DRange1d::From(scalarData.data(), scalarData.size());

    bvector<PolyfaceAuxChannel::DataPtr>   displacementDataVector, scalarDataVector, normalDataVector;

    displacementDataVector.push_back(new PolyfaceAuxChannel::Data(0.0, std::move(zeroDisplacementData)));
    displacementDataVector.push_back(new PolyfaceAuxChannel::Data(1.0, std::move(displacementData)));
    displacementDataVector.push_back(new PolyfaceAuxChannel::Data(2.0, std::move(zeroDisplacementData)));

    scalarDataVector.push_back(new PolyfaceAuxChannel::Data(0.0, std::move(zeroScalarData)));
    scalarDataVector.push_back(new PolyfaceAuxChannel::Data(1.0, std::move(scalarData)));
    scalarDataVector.push_back(new PolyfaceAuxChannel::Data(2.0, std::move(zeroScalarData)));

    normalDataVector.push_back(new PolyfaceAuxChannel::Data(0.0, std::move(initialNormalData)));
    normalDataVector.push_back(new PolyfaceAuxChannel::Data(1.0, std::move(normalData)));
    normalDataVector.push_back(new PolyfaceAuxChannel::Data(2.0, std::move(initialNormalData)));

    PolyfaceAuxChannelPtr       displacementChannel = new PolyfaceAuxChannel(PolyfaceAuxChannel::DataType::Vector, "Displacement", nullptr, std::move(displacementDataVector));
    PolyfaceAuxChannelPtr       scalarChannel = new PolyfaceAuxChannel(PolyfaceAuxChannel::DataType::Scalar, "Scalar", nullptr, std::move(scalarDataVector));
    PolyfaceAuxChannelPtr       normalChannel = new PolyfaceAuxChannel(PolyfaceAuxChannel::DataType::Covector, "Normal", nullptr, std::move(normalDataVector));

    PolyfaceAuxData::Channels   channels;

    channels.push_back (displacementChannel);
    channels.push_back (scalarChannel);
    channels.push_back (normalChannel);
    s_testAuxData = new PolyfaceAuxData(std::move(indices), std::move(channels));
    }
#endif



#ifdef TEST_AUXDATA_HEIGHT
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void getTestAuxData(Bentley::PolyfaceQueryCR polyface, DRange1dR scalarRange)
    {
    bvector<double>      zeroData, heightData;
    bvector<int32_t>    indices(polyface.GetPointIndexCount());
    DPoint3dCP          points = DoInterop(polyface.GetPointCP());

    memcpy (indices.data(), polyface.GetPointIndexCP(), indices.size() * sizeof(int32_t));
    for(size_t i=0; i<polyface.GetPointCount(); i++)
        {
        DPoint3dCR  point = points[i];
        zeroData.push_back(0.0);
        heightData.push_back(point.z);
        }
    scalarRange = DRange1d::From(heightData.data(), heightData.size());

    bvector<PolyfaceAuxChannel::DataPtr>   heightDataVector;

    heightDataVector.push_back(new PolyfaceAuxChannel::Data(1.0, std::move(heightData)));

    PolyfaceAuxChannelPtr         heightChannel = new PolyfaceAuxChannel(PolyfaceAuxChannel::DataType::Distance, "Height", nullptr, std::move(heightDataVector));
    PolyfaceAuxData::Channels           channels;
    channels.push_back (heightChannel);
    PolyfaceAuxDataPtr              auxData = new PolyfaceAuxData(std::move(indices), std::move(channels));

    s_testAuxData = new PolyfaceAuxData(std::move(indices), std::move(channels));
    }
#endif
#ifdef TEST_AUXDATA_CANTILEVER
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void getTestAuxData(Bentley::PolyfaceQueryCR polyface)
    {
    static              double s_aluminumModulus = 10.0E6;      // PSI.
    static              double s_inchesPerMeter = 39.37;
    static              double s_loadPounds = 400.0;
    size_t              nPoints = polyface.GetPointCount();
    bvector<double>     zeroStress(nPoints, 0.0), zeroDisplacement(3*nPoints, 0.0), stressData, displacementData;
    bvector<int32_t>    indices(polyface.GetPointIndexCount());
    DRange3d            range = DRange3d::From((DPoint3dCP) polyface.GetPointCP(), polyface.GetPointCount());
    double              width =  (range.high.y - range.low.y) / s_inchesPerMeter;
    double              height = (range.high.z - range.low.z) / s_inchesPerMeter;
    double              length = (range.high.x - range.low.x) / s_inchesPerMeter;
    double              momentOfInertia = width * height / 12.0;
    double              zOrigin = (range.low.z + range.high.z) / 2.0;
    static double       s_exageration = 5.0;

    memcpy (indices.data(), polyface.GetPointIndexCP(), indices.size() * sizeof(int32_t));
    for(uint32_t i=0; i<nPoints; i++)
        {
        Bentley::DPoint3dCR point = polyface.GetPointCP()[i];

        double      x = (point.x - range.low.x) / s_inchesPerMeter;
        double      z = (point.z - zOrigin) / s_inchesPerMeter;
        stressData.push_back(z * s_loadPounds * (length - x) / (6.0 * momentOfInertia));
        displacementData.push_back(0.0);
        displacementData.push_back(0.0);
        displacementData.push_back(-s_exageration * s_loadPounds * x * x * (3.0 * length - x) / (6.0 * s_aluminumModulus * momentOfInertia));
        }

    bvector<PolyfaceAuxChannel::DataPtr>   displacementDataVector, stressDataVector;

    displacementDataVector.push_back(new PolyfaceAuxChannel::Data(0.0, std::move(zeroDisplacement)));
    displacementDataVector.push_back(new PolyfaceAuxChannel::Data(1.0, std::move(displacementData)));
    displacementDataVector.push_back(new PolyfaceAuxChannel::Data(2.0, std::move(zeroDisplacement)));

    stressDataVector.push_back(new PolyfaceAuxChannel::Data(0.0, std::move(zeroStress)));
    stressDataVector.push_back(new PolyfaceAuxChannel::Data(1.0, std::move(stressData)));
    stressDataVector.push_back(new PolyfaceAuxChannel::Data(2.0, std::move(zeroStress)));

    PolyfaceAuxChannelPtr       displacementChannel = new PolyfaceAuxChannel(PolyfaceAuxChannel::DataType::Vector, "Displacement", nullptr, std::move(displacementDataVector));
    PolyfaceAuxChannelPtr       stressChannel = new PolyfaceAuxChannel(PolyfaceAuxChannel::DataType::Scalar, "Stress", nullptr, std::move(stressDataVector));
    PolyfaceAuxData::Channels   channels;

    channels.push_back (displacementChannel);
    channels.push_back (stressChannel);

    s_testAuxDataSettings = new ThematicGradientSettings(DRange1d::From(stressData.data(), stressData.size()));
    s_testAuxData = new PolyfaceAuxData(std::move(indices), std::move(channels));
    }
#endif


#endif
END_DGNDBSYNC_DGNV8_NAMESPACE
