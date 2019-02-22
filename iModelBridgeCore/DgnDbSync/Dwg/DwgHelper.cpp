/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/DwgHelper.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

struct EscapeCode
    {
    size_t      m_index;            // array index to WString
    WChar       m_code;             // escape code type
    EscapeCode (size_t i, WChar c) : m_index(i), m_code(c) {}
    };
typedef bvector<EscapeCode>     T_CodeList;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnit    DwgHelper::GetStandardUnitFromDwgUnit (DwgDbUnits const& dwgUnit)
    {
    struct DwgDgnUnitsPair
        {
        DwgDbUnits              m_dwgDbUnitValue;
        StandardUnit            m_dgnUnitNumber;
        };
    static struct DwgDgnUnitsPair    s_dwgUnitValueToDgnUnitPairs[] =
        {
        {DwgDbUnits::Undefined,          StandardUnit::None},
        {DwgDbUnits::Inches,             StandardUnit::EnglishInches},
        {DwgDbUnits::Feet,               StandardUnit::EnglishFeet},
        {DwgDbUnits::Miles,              StandardUnit::EnglishMiles},
        {DwgDbUnits::Millimeters,        StandardUnit::MetricMillimeters},
        {DwgDbUnits::Centimeters,        StandardUnit::MetricCentimeters},
        {DwgDbUnits::Meters,             StandardUnit::MetricMeters},
        {DwgDbUnits::Kilometers,         StandardUnit::MetricKilometers},
        {DwgDbUnits::Microinches,        StandardUnit::EnglishMicroInches},
        {DwgDbUnits::Mils,               StandardUnit::EnglishMils},
        {DwgDbUnits::Yards,              StandardUnit::EnglishYards},
        {DwgDbUnits::Angstroms,          StandardUnit::NoSystemAngstroms},           // 1 angstroms = 1-E10 meters
        {DwgDbUnits::Nanometers,         StandardUnit::MetricNanometers},
        {DwgDbUnits::Microns,            StandardUnit::MetricMicrometers},
        {DwgDbUnits::Decimeters,         StandardUnit::MetricDecimeters},
        {DwgDbUnits::Dekameters,         StandardUnit::MetricDekameters},
        {DwgDbUnits::Hectometers,        StandardUnit::MetricHectometers},
        {DwgDbUnits::Gigameters,         StandardUnit::MetricGigameters},
        {DwgDbUnits::Astronomical,       StandardUnit::NoSystemAstronomicalUnits},   // 1 astronomical = 0.149597870691 terameters
        {DwgDbUnits::LightYears,         StandardUnit::NoSystemLightYears},          // 1 light year = 9.460730472580800 petameters
        {DwgDbUnits::Parsecs,            StandardUnit::NoSystemParsecs},             // 1 parsecs = 30.856776 petameters
        {DwgDbUnits::USSurveyFeet,       StandardUnit::EnglishSurveyFeet},           // >= R2017
        {DwgDbUnits::USSurveyInch,       StandardUnit::EnglishSurveyInches},         // >= R2017
        {DwgDbUnits::USSurveyYard,       StandardUnit::EnglishYards},                // >= R2017 - is this even a valid unit??
        {DwgDbUnits::USSurveyMile,       StandardUnit::EnglishSurveyMiles},          // >= R2017
        };
    static size_t   s_numUnits = _countof(s_dwgUnitValueToDgnUnitPairs);

    for (size_t i=0; i < s_numUnits; i++)
        if (dwgUnit == s_dwgUnitValueToDgnUnitPairs[i].m_dwgDbUnitValue)
            return s_dwgUnitValueToDgnUnitPairs[i].m_dgnUnitNumber;

    return StandardUnit::None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
StandardUnit    DwgHelper::GetStandardUnitFromUnitName (Utf8StringCR stringIn)
    {
    struct UnitsName
        {
        Utf8String      m_name;
        StandardUnit    m_unit;
        };
    static UnitsName    s_unitsNames[] =
        {
        {"Meters",          StandardUnit::MetricMeters},
        {"Decimeters",      StandardUnit::MetricDecimeters},
        {"Centimeters",     StandardUnit::MetricCentimeters},
        {"Millimeters",     StandardUnit::MetricMillimeters},
        {"Micrometers",     StandardUnit::MetricMicrometers},
        {"Nanometers",      StandardUnit::MetricNanometers},
        {"Kilometers",      StandardUnit::MetricKilometers},
        {"Inches",          StandardUnit::EnglishInches},
        {"Feet",            StandardUnit::EnglishFeet},
        {"MicroInches",     StandardUnit::EnglishMicroInches},
        {"Miles",           StandardUnit::EnglishMiles},
        {"Yards",           StandardUnit::EnglishYards},
        {"Mils",            StandardUnit::EnglishMils},
        {"SurveyInches",    StandardUnit::EnglishSurveyInches},
        {"SurveyFeet",      StandardUnit::EnglishSurveyFeet},
        {"SurveyMiles",     StandardUnit::EnglishSurveyMiles},
        {"Picometers",      StandardUnit::MetricPicometers},
        {"Femtometers",     StandardUnit::MetricFemtometers},
        {"Petameters",      StandardUnit::MetricPetameters},
        {"Terameters",      StandardUnit::MetricTerameters},
        {"Gigameters",      StandardUnit::MetricGigameters},
        {"Megameters",      StandardUnit::MetricMegameters},
        {"Hectometers",     StandardUnit::MetricHectometers},
        {"Dekameters",      StandardUnit::MetricDekameters},
        {"Picas",           StandardUnit::EnglishPicas},
        {"Points",          StandardUnit::EnglishPoints},
        {"Furlongs",        StandardUnit::EnglishFurlongs},
        {"Chains",          StandardUnit::EnglishChains},
        {"Rods",            StandardUnit::EnglishRods},
        {"Fathoms",         StandardUnit::EnglishFathoms},
        {"Parsecs",         StandardUnit::NoSystemParsecs},
        {"LightYears",      StandardUnit::NoSystemLightYears},
        {"Astronomical",    StandardUnit::NoSystemAstronomicalUnits},
        {"NauticalMiles",   StandardUnit::NoSystemNauticalMiles},
        {"Angstroms",       StandardUnit::NoSystemAngstroms},
        };
    static size_t       s_numUnits = _countof (s_unitsNames);

    for (size_t i = 0; i < s_numUnits; i++)
        {
        if (s_unitsNames[i].m_name.EqualsI(stringIn))
            return  s_unitsNames[i].m_unit;
        }

    return StandardUnit::None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
AnglePrecision  DwgHelper::GetAngularUnits (AngleMode* angleMode, int16_t dwgAUPREC)
    {
    /*-------------------------------------------------------------------------------
    When DDøMM'SS" format is used, the value of dimadec works in this way:
    0       = display degrees only, DDø
    1 or 2  = dipslay degrees and minutes, DDøMM'
    3 or 4  = normal display with 0 decimal place of accuracy, DDøMM'SS"

    The effective number of decimal places actually starts after 4.

    ANGLE_MODE_ has different value than ANGLE_FORMAT_. Dimstyle uses ANGLE_FORMAT_
    and also a "floating" concept.  This method only return ANGLE_MODE_ for file header.
    -------------------------------------------------------------------------------*/
    AnglePrecision  anglePrecision = AnglePrecision::Whole;

    switch (dwgAUPREC)
        {
        case 0:
            anglePrecision = AnglePrecision::Whole;
            if (nullptr != angleMode)
                *angleMode = AngleMode::Degrees;
            break;
        case 1:
        case 2:
            if (nullptr == angleMode)
                {
                anglePrecision = AnglePrecision::Use1Place;
                }
            else
                {
                anglePrecision = AnglePrecision::Whole;
                *angleMode = AngleMode::DegMin;
                }
            break;
        case 3:
        case 4:
            if (nullptr == angleMode)
                {
                anglePrecision = AnglePrecision::Use2Places;
                }
            else
                {
                anglePrecision = AnglePrecision::Whole;
                *angleMode = AngleMode::DegMinSec;
                }
            break;
            
        default:
            anglePrecision = static_cast<AnglePrecision> (dwgAUPREC - 4);
            if (nullptr != angleMode)
                *angleMode = AngleMode::DegMinSec;
            break;
        }

    return  anglePrecision;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbLineWeight DwgHelper::GetDwgLineWeightFromWeightName (Utf8StringCR stringIn)
    {
    struct UnitsName
        {
        Utf8String      m_name;
        DwgDbLineWeight m_lineweight;
        };
    static UnitsName    s_weightsNames[] =
        {
        {"0.00mm",      DwgDbLineWeight::Weight000},
        {"0.05mm",      DwgDbLineWeight::Weight005},
        {"0.09mm",      DwgDbLineWeight::Weight009},
        {"0.13mm",      DwgDbLineWeight::Weight013},
        {"0.15mm",      DwgDbLineWeight::Weight015},
        {"0.18mm",      DwgDbLineWeight::Weight018},
        {"0.20mm",      DwgDbLineWeight::Weight020},
        {"0.25mm",      DwgDbLineWeight::Weight025},
        {"0.30mm",      DwgDbLineWeight::Weight030},
        {"0.35mm",      DwgDbLineWeight::Weight035},
        {"0.40mm",      DwgDbLineWeight::Weight040},
        {"0.50mm",      DwgDbLineWeight::Weight050},
        {"0.53mm",      DwgDbLineWeight::Weight053},
        {"0.60mm",      DwgDbLineWeight::Weight060},
        {"0.70mm",      DwgDbLineWeight::Weight070},
        {"0.80mm",      DwgDbLineWeight::Weight080},
        {"0.90mm",      DwgDbLineWeight::Weight090},
        {"1.00mm",      DwgDbLineWeight::Weight100},
        {"1.06mm",      DwgDbLineWeight::Weight106},
        {"1.20mm",      DwgDbLineWeight::Weight120},
        {"1.40mm",      DwgDbLineWeight::Weight140},
        {"1.58mm",      DwgDbLineWeight::Weight158},
        {"2.00mm",      DwgDbLineWeight::Weight200},
        {"2.11mm",      DwgDbLineWeight::Weight211},
        };
    static size_t       s_numWeights = _countof (s_weightsNames);

    for (size_t i = 0; i < s_numWeights; i++)
        {
        if (s_weightsNames[i].m_name.EqualsI(stringIn))
            return  s_weightsNames[i].m_lineweight;
        }

    return DwgDbLineWeight::Weight000;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
double          DwgHelper::GetTransparencyFromDwg (DwgTransparencyCR dwgTransparency, DwgDbObjectIdCP layerId, DwgDbObjectIdCP blockId)
    {
    double      dgnTransparency = 0.0;

    if (dwgTransparency.IsByAlpha())
        {
        // DWG has reversed meaning by transparecy value: 0=completely transparent; 1=completely opaque.
        dgnTransparency = 1.0 - (double)(dwgTransparency.GetAlpha() & 0XFF) / 255.0;
        // round down percent value:
        dgnTransparency = floor(100 * dgnTransparency) / 100.0;
        /*-------------------------------------------------------------------------------
        Apply effective transparency to element.
        deriving from the fomula:
        NetTransparency = 1 - (1 - elememntTransparency) * (1 - levelTransparency) * (1 - modelTransparency)
        to equation:
        elementTransparency = (netTransparency - levelTransparency) / (1 - levelTransparency)

        However, this only works for levelTransparency <= netTransparency.  When layer transparency is greater
        than entity transparency, make entity transparency "ByLayer".
        -------------------------------------------------------------------------------*/
        if (nullptr != layerId && layerId->IsValid())
            {
            DwgDbLayerTableRecordPtr layer(*layerId, DwgDbOpenMode::ForRead);
            if (!layer.IsNull())
                {
                DwgTransparency     layerTransparency = layer->GetTransparency ();
                double              levelTransparency = DwgHelper::GetTransparencyFromDwg (layerTransparency);

                if (fabs(1.0 - levelTransparency) < 0.01)
                    dgnTransparency = 0.0;
                else
                    dgnTransparency = (dgnTransparency - levelTransparency) / (1.0 - levelTransparency);

                if (dgnTransparency < 0.0)
                    dgnTransparency = 0.0;  // TRANSPARENCY_ByLayer;
                }
            }
        }
    else if (dwgTransparency.IsByLayer())
        {
        /*-------------------------------------------------------------------------------
        FUTUREWORK_TRANSPARENCY: MicroStation needs to support BYLAYER transparency!

        Temporary workaround: set BYLAYER transparency to 0%, which gets round-tripped back
        to BYLAYER.  Level transparency always affects element, as shown in above formula.
        This can work because a DWG entity will only have either BYLAYER/BYBLOCK or an 
        effective value.
        -------------------------------------------------------------------------------*/
        dgnTransparency = 0.0;
        }
    else if (dwgTransparency.IsByBlock() && nullptr != blockId)
        {
        /*-------------------------------------------------------------------------------
        FUTUREWORK_TRANSPARENCY: MicroStation needs to support BYCELL transparency!

        Temporary workaround:
        1) Set effective value on element based on block's first instance's transparency.
        2) Set BYBLOCK transparency to 1% for round-trip purpose, if the block has no
            instance.
        This is rasther a kludge workaround, but it's better than losing BYBLOCK on a DWG 
        save or not displaying transparency at all on DWG open.
        -------------------------------------------------------------------------------*/
        dgnTransparency = 0.0;

/*
        AcDbDatabase*   dwg = this->GetDatabase ();
        AcDbObjectId    ownerId = *blockId;
        if (!ownerId.isValid() && ownerId != acdbSymUtil()->blockModelSpaceId(dwg) && ownerId != acdbSymUtil()->blockPaperSpaceId(dwg))
            {
            AcDbBlockTableRecordPointer block(ownerId, AcDb::kForRead);
            if (Acad::eOk == block.openStatus() && !RealDwgUtil::IsModelOrPaperSpace(block.object()))
                {
                AcDbObjectIdArray       insertIds;
                if (Acad::eOk == block->getBlockReferenceIds(insertIds) && insertIds.length() > 0)
                    {
                    AcDbBlockReferencePointer insert(insertIds[0], AcDb::kForRead);
                    if (Acad::eOk == insert.openStatus())
                        return  this->GetDgnTransparency (insert->transparency(), insert);
                    }
                }
            }
*/
        }

    return  fabs(dgnTransparency);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d        DwgHelper::DefaultPlacementPoint (DwgDbEntityCR entity)
    {
    DPoint3d        placementPoint;
    DRange3d        entRange;
    DPoint3dArray   gripPoints;

    if (DwgDbStatus::Success == entity.GetGripPoints(gripPoints) && gripPoints.size() > 0)
        {
        // set the first grip point as the placement point
        placementPoint = gripPoints[0];
        }
    else if (DwgDbStatus::Success == entity.GetRange(entRange) && !entRange.IsNull())
        {
        // set the center as the placement point
        placementPoint = entRange.LocalToGlobal (0.5, 0.5, 0.5);
        }
    else
        {
        // any other better way?
        placementPoint.Zero ();
        }

    return  placementPoint;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
RenderMode      DwgHelper::GetRenderModeFromVisualStyle (DwgDbVisualStyleCR visualStyle)
    {
    switch (visualStyle.GetType())
        {
        case DwgGiVisualStyle::RenderType::ShadesOfGray:
        case DwgGiVisualStyle::RenderType::Shaded:
        case DwgGiVisualStyle::RenderType::Flat:
            return RenderMode::SmoothShade;

        case DwgGiVisualStyle::RenderType::FaceOnly:
        case DwgGiVisualStyle::RenderType::ShadedWithEdges:
        case DwgGiVisualStyle::RenderType::FlatWithEdges:
        case DwgGiVisualStyle::RenderType::Conceptual:
            return RenderMode::SolidFill;

        case DwgGiVisualStyle::RenderType::Gouraud:
        case DwgGiVisualStyle::RenderType::GouraudWithEdges:
            return RenderMode::SmoothShade;

        case DwgGiVisualStyle::RenderType::Sketchy:
        case DwgGiVisualStyle::RenderType::Hidden:
            return RenderMode::HiddenLine;

        case DwgGiVisualStyle::RenderType::Realistic:
            return RenderMode::SmoothShade;

        case DwgGiVisualStyle::RenderType::Custom:
            {
            // custom rendering - check face lighting model
            DwgGiVisualStyleOperations::Operation   op;
            DwgGiVariantCR  faceModel = visualStyle.GetTrait (DwgGiVisualStyleProperties::Property::FaceLightingModel, &op);
            if (DwgGiVisualStyleOperations::Operation::Set == op && DwgGiVariant::Integer == faceModel.GetType())
                {
                DwgGiVisualStyleProperties::FaceLightingModel rendering = faceModel.AsEnum ();
                switch (rendering)
                    {
                    case DwgGiVisualStyleProperties::FaceLightingModel::Constant:
                        return RenderMode::SolidFill;

                    case DwgGiVisualStyleProperties::FaceLightingModel::Phong:
                    case DwgGiVisualStyleProperties::FaceLightingModel::Gooch:
                    case DwgGiVisualStyleProperties::FaceLightingModel::Zebra:
                    default:
                        return RenderMode::SmoothShade;
                    }
                }
            return  RenderMode::Wireframe;
            }

        case DwgGiVisualStyle::RenderType::Dim:
        case DwgGiVisualStyle::RenderType::Brighten:
        case DwgGiVisualStyle::RenderType::Thicken:
        case DwgGiVisualStyle::RenderType::LinePattern:
        case DwgGiVisualStyle::RenderType::FacePattern:
        case DwgGiVisualStyle::RenderType::ColorChange:
        case DwgGiVisualStyle::RenderType::Basic:
        case DwgGiVisualStyle::RenderType::Wireframe2d:
        case DwgGiVisualStyle::RenderType::Wireframe3d:
            return  RenderMode::Wireframe;
        }

    return  RenderMode::Wireframe;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgHelper::UpdateViewFlagsFromVisualStyle (ViewFlags& viewFlags, DwgDbObjectIdCR id)
    {
    DwgDbVisualStylePtr     visualStyle (id, DwgDbOpenMode::ForRead);
    if (visualStyle.IsNull())
        return  BSIERROR;

    // set render mode
    viewFlags.SetRenderMode (DwgHelper::GetRenderModeFromVisualStyle(*visualStyle.get()));
    
    DwgGiVariant    var(false);
    DwgGiVisualStyleOperations::Operation   op(DwgGiVisualStyleOperations::Operation::InvalidOperation);

    // set visible edges
    viewFlags.SetShowVisibleEdges (false);

    var = visualStyle->GetTrait (DwgGiVisualStyleProperties::Property::EdgeModel, &op);
    if (DwgGiVisualStyleOperations::Operation::Set == op && DwgGiVariant::Integer == var.GetType())
        {
        DwgGiVisualStyleProperties::EdgeModel visibleEdges = var.AsEnum ();
        switch (visibleEdges)
            {
            // WIP - support displaying isolines around a round body?
            case DwgGiVisualStyleProperties::EdgeModel::Isolines:
            case DwgGiVisualStyleProperties::EdgeModel::NoEdges:
                viewFlags.SetShowVisibleEdges (false);
                break;
            case DwgGiVisualStyleProperties::EdgeModel::FacetEdges:
            default:
                viewFlags.SetShowVisibleEdges (true);

                var = visualStyle->GetTrait (DwgGiVisualStyleProperties::Property::EdgeColor, &op);
                if (DwgGiVisualStyleOperations::Operation::Set == op && DwgGiVariant::Color == var.GetType())
                    {
                    DwgCmColor  color = var.AsColor ();
                    // WIP - set color for visible edges?
                    }
                break;
            }
        }

    // set hidden edges
    viewFlags.SetShowHiddenEdges (false);

    var = visualStyle->GetTrait (DwgGiVisualStyleProperties::Property::EdgeStyles, &op);
    if (DwgGiVisualStyleOperations::Operation::Set == op && DwgGiVariant::Integer == var.GetType())
        {
        DwgGiVisualStyleProperties::EdgeStyles  edgeStyles = var.AsEnum ();
        switch (edgeStyles)
            {
            case DwgGiVisualStyleProperties::EdgeStyles::NoEdgeStyle:
                viewFlags.SetShowHiddenEdges (false);
                break;
            case DwgGiVisualStyleProperties::EdgeStyles::VisibleFlag:
            case DwgGiVisualStyleProperties::EdgeStyles::SilhouetteFlag:
            case DwgGiVisualStyleProperties::EdgeStyles::ObscuredFlag:
            case DwgGiVisualStyleProperties::EdgeStyles::IntersectionFlag:
            default:
                viewFlags.SetShowHiddenEdges (true);
                break;
            }

        if (viewFlags.ShowHiddenEdges())
            {
            var = visualStyle->GetTrait (DwgGiVisualStyleProperties::Property::EdgeObscuredColor, &op);
            if (DwgGiVisualStyleOperations::Operation::Set == op && DwgGiVariant::Color == var.GetType())
                {
                DwgCmColor  color = var.AsColor ();
                // WIP - set color for hidden edges?
                }
            var = visualStyle->GetTrait (DwgGiVisualStyleProperties::Property::EdgeObscuredLinePattern, &op);
            if (DwgGiVisualStyleOperations::Operation::Set == op && DwgGiVariant::Integer == var.GetType())
                {
                DwgGiVisualStyleProperties::EdgeLinePattern linePattern = var.AsEnum ();
                // WIP - set linestyle for hidden edges?
                }
            }
        }

    // set shadows
    viewFlags.SetShowShadows (false);

    var = visualStyle->GetTrait (DwgGiVisualStyleProperties::Property::DisplayShadowType, &op);
    if (DwgGiVisualStyleOperations::Operation::Set == op && DwgGiVariant::Integer == var.GetType())
        {
        DwgGiVisualStyleProperties::DisplayShadowType shadows = var.AsEnum ();
        switch (shadows)
            {
            case DwgGiVisualStyleProperties::DisplayShadowType::None:
                viewFlags.SetShowShadows (false);
                break;
            case DwgGiVisualStyleProperties::DisplayShadowType::GroundPlane:
            case DwgGiVisualStyleProperties::DisplayShadowType::Full:
            case DwgGiVisualStyleProperties::DisplayShadowType::FullAndGround:
            default:
                viewFlags.SetShowShadows (true);
                break;
            }
        }

    // set materials, lights, etc
    viewFlags.SetShowSourceLights (true);
    viewFlags.SetShowMaterials (true);
    viewFlags.SetShowTextures (true);

    var = visualStyle->GetTrait (DwgGiVisualStyleProperties::Property::DisplayStyles, &op);
    if (DwgGiVisualStyleOperations::Operation::Set == op && DwgGiVariant::Integer == var.GetType())
        {
        uint32_t    styleFlags = var.AsULong ();
        if (DwgGiVisualStyleProperties::DisplayStyles::NoDisplayStyle == static_cast<DwgGiVisualStyleProperties::DisplayStyles>(styleFlags))
            {
            viewFlags.SetShowSourceLights (false);
            viewFlags.SetShowMaterials (false);
            viewFlags.SetShowTextures (false);
            }
        else
            {
            if ((static_cast<uint32_t>(DwgGiVisualStyleProperties::DisplayStyles::LightingFlag) & styleFlags) != 0)
                viewFlags.SetShowSourceLights (true);
            if ((static_cast<uint32_t>(DwgGiVisualStyleProperties::DisplayStyles::MaterialsFlag) & styleFlags) != 0)
                viewFlags.SetShowMaterials (true);
            if ((static_cast<uint32_t>(DwgGiVisualStyleProperties::DisplayStyles::TexturesFlag) & styleFlags) != 0)
                viewFlags.SetShowTextures (true);
            }
        }

    return  BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgHelper::SetViewFlags (ViewFlags& viewFlags, bool grid, bool acs, bool background, bool transparent, bool clipFront, bool clipBack, DwgDbDatabaseCR dwg)
    {
    viewFlags.InitDefaults ();

    viewFlags.SetShowConstructions (true);
    viewFlags.SetShowDimensions (true);
    viewFlags.SetShowWeights (dwg.GetLineweightDisplay());
    viewFlags.SetShowStyles (true);
    viewFlags.SetShowTransparency (transparent);                            // an ACAD registry option if in modelspace
    viewFlags.SetShowFill (viewFlags.ShowPatterns() == dwg.GetFILLMODE());  // ACAD fill mode controls both fill & pattern
    viewFlags.SetShowGrid (grid);
    viewFlags.SetShowAcsTriad (acs);
    viewFlags.SetShowClipVolume (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgHelper::GetLayoutOrBlockName (Utf8StringR nameOut, DwgDbBlockTableRecordCR blockIn)
    {
    nameOut.clear ();

    if (blockIn.IsLayout())
        {
        DwgDbLayoutPtr  layout(blockIn.GetLayoutId(), DwgDbOpenMode::ForRead);
        if (!layout.IsNull())
            nameOut.Assign (layout->GetName().c_str());
        }

    if (nameOut.empty())
        nameOut.Assign (blockIn.GetName().c_str());

    return  nameOut.empty() ? BSIERROR : BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgHelper::ComputeMatrixFromArbitraryAxis (RotMatrixR matrix, DVec3dCR normal)
    {
    static double   s_1_over_64 = 0.015625;
    DVec3d          world = DVec3d::From (0.0, 0.0, 0.0);
    if (fabs(normal.x) < s_1_over_64 && fabs(normal.y) < s_1_over_64)
        world.y = 1.0;
    else
        world.z = 1.0;

    DVec3d      zAxis = normal;
    zAxis.Normalize ();

    DVec3d      xAxis, yAxis;
    xAxis.NormalizedCrossProduct (world, zAxis);
    yAxis.NormalizedCrossProduct (zAxis, xAxis);

    matrix.InitFromColumnVectors (xAxis, yAxis, zAxis);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          01/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgHelper::ComputeMatrixFromXZ (RotMatrixR matrix, DVec3dCR xDirection, DVec3dCR normal)
    {
    DVec3d      xAxis = xDirection;
    xAxis.Normalize ();
    if (xAxis.IsZero())
        xAxis.Init (1.0, 0.0, 0.0);

    DVec3d      zAxis = normal;
    zAxis.Normalize ();
    if (zAxis.IsZero())
        zAxis.Init (0.0, 0.0, 1.0);

    DVec3d      yAxis = DVec3d::FromCrossProduct (zAxis, xAxis);

    matrix.InitFromColumnVectors (xAxis, yAxis, zAxis);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                     EarlinLutz 03/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgHelper::CreateArc2d (DEllipse3dR ellipse, DPoint3dCR start, DPoint3dCR end, double bulgeFactor)
    {
    double      sweep = 4.0 * atan (bulgeFactor);

    // Find tan(included angle / 2)
    double      tangentHalfAngle = tan(sweep * 0.5);

    // U = chord vector
    DVec3d      u;

    u.DifferenceOf (end, start);
    u.Scale (u, 0.5);
    u.z = 0.0;

    // V = perpendicular to chord vector, same length.
    DVec3d      v;
    v.x = -u.y;
    v.y = u.x;
    v.z = 0.0;

    // Center = p0 + U + -V/tangentHalfAngle
    DPoint3d    center;
    center.SumOf (start, u, 1.0, v, 1.0/tangentHalfAngle);

    // Radius
    DPoint3d    r;
    r.DifferenceOf (start, center);
    double radius = r.Magnitude();

    // Start angle
    DPoint3d    x = {1.0, 0.0, 0.0};
    double      theta0 = x.AngleToXY (r);

    // Setup for ellipse
    ellipse.Init (center.x, center.y, 0.0, radius, 0.0, 0.0, 0.0, radius, 0.0, theta0, sweep);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d        DwgHelper::ComputeBulgePoint (double bulgeFactor, DPoint3dCR start, DPoint3dCR end)
    {
    // half chord vector
    DPoint3d    halfChordVector;
    halfChordVector.DifferenceOf (end, start);
    halfChordVector.Scale (0.5);

    // chord height vectors - perpendicular to chord
    DPoint3d    chordHeightVector;
    chordHeightVector.x = -halfChordVector.y;
    chordHeightVector.y = halfChordVector.x;
    chordHeightVector.z = halfChordVector.z;

    // mid-arc point = start + halfChordVector + chordHeightVector
    DPoint3d    midArcPoint;
    midArcPoint.SumOf (start, halfChordVector, 1.0, chordHeightVector, -bulgeFactor);

    return  midArcPoint;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP          DwgHelper::ToUtf8CP (DwgStringCR fromString, bool nullIfEmpty)
    {
    static Utf8String   s_utf8Buffer;

    if (fromString.IsEmpty())
        {
        if (nullIfEmpty)
            return  nullptr;
        s_utf8Buffer.assign ("??");
        }
    else
        {
        if (BSISUCCESS != BeStringUtilities::WCharToUtf8(s_utf8Buffer, fromString.c_str(), fromString.GetLength()))
            s_utf8Buffer.assign ("xx");
        }

    return  s_utf8Buffer.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgHelper::ValidateStyleName (Utf8String& out, DwgStringCR in)
    {
    Utf8CP      converted = DwgHelper::ToUtf8CP (in, true);
    if (nullptr == converted)
        out.assign ("Unnamed");
    else
        out.assign (converted);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void     CreateLineSeg (DSegment3d& line, DPoint3dCP origins, size_t start, size_t end, TransformCR toWorld, double shiftY, double endWidth)
    {
    line.point[0] = origins[start];
    line.point[1] = origins[end];

    // shift y-coordinate to make an over/underline
    line.point[0].y += shiftY;
    line.point[1].y += shiftY;

    // if it is the last character, add its width to complete the under/overline
    line.point[1].x += endWidth;

    toWorld.Multiply (&line.point[0], &line.point[0], 2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void     AddLineSegs (bvector<DSegment3d>& linesegs, T_CodeList const& lineCodes, DPoint3dCP origins, TransformCR toWorld, double shiftY, double endWidth, size_t nGlyphs, WChar code)
    {
    size_t      startIndex = -1;
    DSegment3d  lineseg;

    // walkd through saved escape codes in their original sequence and extract start & end indices for under/overlines:
    for (auto const& lineCode : lineCodes)
        {
        size_t  currIndex = lineCode.m_index;

        if (lineCode.m_code == code)
            {
            if (-1 == startIndex)
                {
                // under/overline starts
                startIndex = currIndex;
                }
            else
                {
                // under/overline ends
                CreateLineSeg (lineseg, origins, startIndex, currIndex, toWorld, shiftY, 0.0);
                linesegs.push_back (lineseg);

                startIndex = -1;
                }
            }
        }

    // if an under/overline has started but has never ended, add a line from start to the end of the string:
    if (startIndex != -1 && startIndex < nGlyphs)
        {
        size_t  endIndex = nGlyphs - 1;
        CreateLineSeg (lineseg, origins, startIndex, endIndex, toWorld, shiftY, endWidth);
        linesegs.push_back (lineseg);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/16
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          DwgHelper::ConvertEscapeCodes (TextStringR text, bvector<DSegment3d>* underlines, bvector<DSegment3d>* overlines)
    {
    // WIP - a workaround for the lack of a TextBlock to process %%n markups
    WString     wString;
    wString.AssignUtf8 (text.GetText().c_str());

    size_t          nDecoded = 0;
    size_t          nChars = wString.size ();
    if (nChars < 3)
        return  nDecoded;

    DgnFontCR       font = text.GetStyle().GetFont ();
    DgnShxFontCP    shxFont = dynamic_cast <DgnShxFontCP> (&font);

    // get coded symbols:
    DgnShxFont::Metadata    escapeCodes;
    if (nullptr != shxFont)
        {
        // SHX
        escapeCodes = shxFont->GetMetadata ();
        }
    else
        {
        // TTF
        escapeCodes.m_degreeCode = 0xB0;
        escapeCodes.m_diameterCode = 0x2205;
        escapeCodes.m_plusMinusCode = 0xB1;
        }

    // First run - convert all %% escape codes expcept for under/overlines:
    WString         decodedString;
    T_CodeList      lineCodes;
    WCharCP         pNextChar = wString.c_str ();
    WCharCP         pEndChar = pNextChar + nChars;

    while (pNextChar < pEndChar)
        {
        if (*pNextChar == L'%' && *(pNextChar + 1) == L'%')
            {
            WChar       escapeCode = *(pNextChar + 2);
            switch (escapeCode)
                {
                case L'c':
                case L'C':
                    escapeCode = escapeCodes.m_diameterCode;
                    break;
                case L'd':
                case L'D':
                    escapeCode = escapeCodes.m_degreeCode;
                    break;
                case L'p':
                case L'P':
                    escapeCode = escapeCodes.m_plusMinusCode;
                    break;
                case L'u':
                case L'U':
                    // no symbol - save escape code for post processing (sequence important!)
                    escapeCode = 0;
                    lineCodes.push_back (EscapeCode(decodedString.size(), L'U'));
                    break;
                case L'o':
                case L'O':
                    // no symbol - save escape code for post processing (sequence important!)
                    escapeCode = 0;
                    lineCodes.push_back (EscapeCode(decodedString.size(), L'O'));
                    break;
                case L'%':
                default:
                    // escapeCode itself is the symbol
                    break;
                }
            
            // append the symbol to the new string:
            if (0 != escapeCode)
                decodedString.append (1, escapeCode);

            // move on to check next set of characters:
            pNextChar += 3;
            nDecoded++;
            continue;
            }

        // no escape codes, just copy the character
        decodedString.append (1, *pNextChar);
        pNextChar++;
        }

    if (nDecoded == 0 || decodedString.size() == 0)
        return  0;

    // reset the text with the decoded string:
    Utf8String      utf8String (decodedString);
    text.SetText (utf8String.c_str());

    // Second run - create line segments for underlines and overlines, if any
    if (lineCodes.size() > 0 && (nullptr != underlines || nullptr != overlines))
        {
        DPoint3dCP  origins = text.GetGlyphOrigins ();
        if (nullptr == origins)
            return  0;

        Transform   toWorld = text.ComputeTransform ();
        DRange2d    range = text.GetRange ();
        size_t      nGlyphs = text.GetNumGlyphs ();
        double      underlineSpace = 0.2 * range.YLength ();
        double      overlineSpace = 1.2 * range.YLength ();
        double      endCharWidth = range.XLength() - origins[nGlyphs - 1].x;

        // collect underline & overline in separate runs as the markups can mix in between
        if (nullptr != underlines)
            AddLineSegs (*underlines, lineCodes, origins, toWorld, -underlineSpace, endCharWidth, nGlyphs, L'U');

        if (nullptr != overlines)
            AddLineSegs (*overlines, lineCodes, origins, toWorld, overlineSpace, endCharWidth, nGlyphs, L'O');
        }

    return  nDecoded;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef        DwgHelper::GetColorDefFromACI (int16_t acColorIndex)
    {
    // below call gets a value of low byte blue, the second byte green, and the third byte red & the highest byte the color method.
    uint32_t    bgr = DwgCmEntityColor::GetRGBFromIndex (acColorIndex);
    uint32_t    rgb = (bgr & 0x00ff0000) >> 16 | (bgr & 0x0000ff00) | (bgr & 0x000000ff) << 16;

    // invert foreground color
    return  rgb == 0 ? ColorDef::White() : ColorDef (rgb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef        DwgHelper::GetColorDefFromTrueColor (DwgCmColorCR acColor)
    {
    // get BGR and swap bytes
    uint32_t    bgr = acColor.GetRGB ();
    uint32_t    rgb = (bgr & 0x00ff0000) >> 16 | (bgr & 0x0000ff00) | (bgr & 0x000000ff) << 16;
    return  ColorDef (rgb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef        DwgHelper::GetColorDefFromTrueColor (DwgCmEntityColorCR acColor)
    {
    // get BGR and swap bytes
    uint32_t    bgr = acColor.GetRGB ();
    uint32_t    rgb = (bgr & 0x00ff0000) >> 16 | (bgr & 0x0000ff00) | (bgr & 0x000000ff) << 16;
    return  ColorDef (rgb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgHelper::GetDgnGradientColor (GradientSymbR gradientOut, DwgGiGradientFillCR gradientIn)
    {
    // Convert DWG gradient fill to DGN
    Render::GradientSymb::Mode  mode = Render::GradientSymb::Mode::None;
    bool                        inverted = false;
    switch (gradientIn.GetType())
        {
        case DwgGiGradientFill::Type::Linear:           mode = Render::GradientSymb::Mode::Linear;        break;
        case DwgGiGradientFill::Type::Cylinder:         mode = Render::GradientSymb::Mode::Cylindrical;   break;
        case DwgGiGradientFill::Type::Spherical:        mode = Render::GradientSymb::Mode::Spherical;     break;
        case DwgGiGradientFill::Type::Hemispherical:    mode = Render::GradientSymb::Mode::Hemispherical; break;
        case DwgGiGradientFill::Type::Curved:           mode = Render::GradientSymb::Mode::Curved;        break;
        case DwgGiGradientFill::Type::InvCylinder:      mode = Render::GradientSymb::Mode::Cylindrical;
            inverted = true;
            break;
        case DwgGiGradientFill::Type::InvSpherical:     mode = Render::GradientSymb::Mode::Spherical;
            inverted = true;
            break;
        case DwgGiGradientFill::Type::InvHemispherical: mode = Render::GradientSymb::Mode::Hemispherical;
            inverted = true;
            break;
        case DwgGiGradientFill::Type::InvCurved:        mode = Render::GradientSymb::Mode::Curved;
            inverted = true;
            break;
        }
    gradientOut.SetMode (mode);

    uint16_t    flags = static_cast<uint16_t> (Render::GradientSymb::Flags::None);
    if (inverted)
        flags |= static_cast<uint16_t> (Render::GradientSymb::Flags::Invert);
    gradientOut.SetFlags (static_cast<Render::GradientSymb::Flags>(flags));

    gradientOut.SetAngle (gradientIn.GetAngle());
    gradientOut.SetShift (gradientIn.GetShift());

    // WIP - gradientOut.SetTint (tint);

    DwgColorArray   dwgColors;
    uint16_t        nColors = (uint16_t)gradientIn.GetColors (dwgColors);
    if (nColors > 0)
        {
        bvector<ColorDef>   keyColors;
        bvector<double>     keyValues;

        uint16_t    count = 0;
        for (DwgCmColorCR cmColor : dwgColors)
            {
            if (cmColor.IsByACI())
                keyColors.push_back (DwgHelper::GetColorDefFromACI(cmColor.GetIndex()));
            else
                keyColors.push_back (ColorDef(cmColor.GetRed(), cmColor.GetGreen(), cmColor.GetBlue()));

            keyValues.push_back (static_cast<double> (count++ / nColors));
            if (count == 1)
                count++;
            }

        gradientOut.SetKeys (nColors, &keyColors.front(), &keyValues.front());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/16
+---------------+---------------+---------------+---------------+---------------+------*/
void            DwgHelper::SetGradientFrom (DwgGiGradientFillR gradientOut, DwgDbHatchCR hatchIn)
    {
    DwgGiGradientFill::Type type = DwgGiGradientFill::Type::Linear;
    DwgString               name = hatchIn.GetGradientName ();

    // name to type
    if (0 == name.EqualsI(L"Spherical"))
        type = DwgGiGradientFill::Type::Spherical;
    else if (0 == name.EqualsI(L"InvSpherical"))
        type = DwgGiGradientFill::Type::InvSpherical;
    else if (0 == name.EqualsI(L"Hemispherical"))
        type = DwgGiGradientFill::Type::Hemispherical;
    else if (0 == name.EqualsI(L"InvHemispherical"))
        type = DwgGiGradientFill::Type::InvHemispherical;
    else if (0 == name.EqualsI(L"Cylinder"))
        type = DwgGiGradientFill::Type::Cylinder;
    else if (0 == name.EqualsI(L"InvCylinder"))
        type = DwgGiGradientFill::Type::InvCylinder;
    else if (0 == name.EqualsI(L"Curved"))
        type = DwgGiGradientFill::Type::Curved;
    else if (0 == name.EqualsI(L"InvCurved"))
        type = DwgGiGradientFill::Type::InvCurved;

    gradientOut.SetType (type);
    gradientOut.SetAngle (hatchIn.GetGradientAngle());
    gradientOut.SetShift (hatchIn.GetGradientShift());
    gradientOut.SetTint (hatchIn.GetGradientTint());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgHelper::GetAttrdefECSchemaName (DwgDbDatabaseCP dwg)
    {
    // build a per-file attrdef schema name
    Utf8String  schemaName = SCHEMAName_AttributeDefinitions;
    if (dwg != nullptr)
        {
        Utf8String  filename(BeFileName::GetFileNameWithoutExtension(dwg->GetFileName().c_str()).c_str());
        schemaName += "_" + filename;
        if (!ECNameValidation::IsValidName(schemaName.c_str()))
            ECNameValidation::EncodeToValidName (schemaName, schemaName.c_str());
        }
    return  schemaName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String      DwgHelper::GetAttrdefECClassNameFromBlockName (WCharCP blockName)
    {
    // build an ECClassName representing a collection of attribute definitions in a block:
    Utf8String  validName;
    ECNameValidation::EncodeToValidName (validName, (nullptr == blockName || 0 == *blockName) ? "??" : Utf8String(blockName));

    return Utf8PrintfString("%s%s", validName.c_str(), BIS_CLASS_ElementAspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DRange2d        DwgHelper::GetRangeFrom (DPoint2dCR center, double width, double height)
    {
    double      halfWidth = 0.5 * width;
    double      halfHeight = 0.5 * height;

    DRange2d    range;
    range.low.x = center.x - halfWidth;
    range.low.y = center.y - halfHeight;
    range.high.x = center.x + halfWidth;
    range.high.y = center.y + halfHeight;

    return  range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/17
+---------------+---------------+---------------+---------------+---------------+------*/
double          DwgHelper::GetAbsolutePDSIZE (double pdsize, double vportHeight)
    {
    /*-----------------------------------------------------------------------------------
    When PDSIZE > 0, which indicates an absolute size, both OpenDWG and RealDWG honors the value which
    causes the GI drawable to correctly reflect the displayed geometry.  When PDSIZE <= 0, while OpenDWG
    honors the size as percentage of the viewport height via a callback OdGiViewport::getViewportDcCorners,
    RealDWG does not appear to honor the value.  We work it around by calculating an absolute size from
    the viewport size and override PDSIZE to have point drawable produce the correct geometry.
    -----------------------------------------------------------------------------------*/
    if (pdsize > 0.0)
        return  pdsize;

    // the default value 0.0 implies 5.0%
    if (pdsize == 0.0)
        pdsize = -5.0;
    else if (pdsize < 100.0)
        pdsize = -100.0;

    // calc point size from a percentage of the viewport height (ACAD appears to apply 1/2 of the height):
    double  displaySize = -0.005 * pdsize * vportHeight;

    return  displaySize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr  DwgHelper::CreateCurvePrimitive (DwgDbLineCR line, TransformCP transform)
    {
    DPoint3d    points[2] = {line.GetStartPoint(), line.GetEndPoint()};
    if (nullptr != transform)
        transform->Multiply (points, 2);
    return  ICurvePrimitive::CreateLine (points[0], points[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr  DwgHelper::CreateCurvePrimitive (DwgDbArcCR arc, TransformCP transform)
    {
    DPoint3d    center = arc.GetCenter ();
    DVec3d      normal = arc.GetNormal ();
    double      radius = arc.GetRadius ();
    double      start = arc.GetStartAngle ();
    double      swept = arc.GetEndAngle() - start;

    if (!Angle::IsFullCircle(swept))
        swept = Angle::AdjustToSweep (swept, 0, Angle::TwoPi());

    if (nullptr != transform)
        {
        transform->Multiply (center);
        transform->Multiply (normal);
        normal.Normalize ();

        double  toMeters = 1.0;
        if (transform->IsRigidScale(toMeters))
            radius *= toMeters;
        }
    else
        {
        normal.Normalize ();
        }

    DVec3d      xAxis, yAxis;
    RotMatrix   matrix;
    DwgHelper::ComputeMatrixFromArbitraryAxis (matrix, normal);
    matrix.GetColumn (xAxis, 0);
    matrix.GetColumn (yAxis, 1);

    DEllipse3d  ellipse = DEllipse3d::FromScaledVectors (center, xAxis, yAxis, radius, radius, start, swept);
    return  ICurvePrimitive::CreateArc (ellipse);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr  DwgHelper::CreateCurvePrimitive (DwgDbCircleCR circle, TransformCP transform)
    {
    DPoint3d    center = circle.GetCenter ();
    DVec3d      normal = circle.GetNormal ();
    double      radius = 0.5 * circle.GetDiameter ();

    if (nullptr != transform)
        {
        transform->Multiply (center);
        transform->Multiply (normal);
        normal.Normalize ();

        double  toMeters = 1.0;
        if (transform->IsRigidScale(toMeters))
            radius *= toMeters;
        }
    
    return ICurvePrimitive::CreateArc (DEllipse3d::FromCenterNormalRadius(center, normal, radius));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  DwgHelper::CreateCurveVectorFrom (DwgDbCircleCR circle, CurveVector::BoundaryType type, TransformCP transform)
    {
    auto geom = DwgHelper::CreateCurvePrimitive (circle, transform);
    return  geom.IsValid() ? CurveVector::Create(geom, type) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr  DwgHelper::CreateCurvePrimitive (DwgDbEllipseCR ellipse, TransformCP transform)
    {
    DPoint3d    center = ellipse.GetCenter ();
    DVec3d      major = ellipse.GetMajorAxis ();
    DVec3d      minor = ellipse.GetMinorAxis ();
    double      start = ellipse.GetStartParam ();
    double      swept = ellipse.GetEndParam() - start;

    if (!Angle::IsFullCircle(swept))
        swept = Angle::AdjustToSweep (swept, 0.0, msGeomConst_2pi);

    if (nullptr != transform)
        {
        transform->Multiply (center);
        transform->Multiply (major);
        transform->Multiply (minor);
        }

    return ICurvePrimitive::CreateArc (DEllipse3d::FromVectors(center, major, minor, start, swept));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  DwgHelper::CreateCurveVectorFrom (DwgDbEllipseCR ellipse, CurveVector::BoundaryType type, TransformCP transform)
    {
    auto geom = DwgHelper::CreateCurvePrimitive (ellipse, transform);
    return  geom.IsValid() ? CurveVector::Create(geom, type) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  DwgHelper::CreateCurveVectorFrom (DwgDbPolylineCR polyline, CurveVector::BoundaryType type, TransformCP transform)
    {
    PolylineFactory factory (polyline, true);
    factory.SetBoundaryType (type);

    if (nullptr != transform)
        factory.TransformData (*transform);

    return factory.CreateCurveVector (false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  DwgHelper::CreateCurveVectorFrom (DwgDb2dPolylineCR polyline2d, CurveVector::BoundaryType type, TransformCP transform)
    {
    BeAssert (false && "2DPolyline clipper not implemented yet!");
    return  nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  DwgHelper::CreateCurveVectorFrom (DwgDb3dPolylineCR polyline3d, CurveVector::BoundaryType type, TransformCP transform)
    {
    BeAssert (false && "3DPolyline clipper not implemented yet!");
    return  nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr  DwgHelper::CreateCurvePrimitive (DwgDbSplineCR spline, TransformCP transform, bool makeLinestring)
    {
    if (makeLinestring)
        {
        static int32_t  s_nPoints = 100;
        DPoint3dArray   points;
        if (DwgDbStatus::Success == spline.GetSamplePoints(s_nPoints, points) && points.size() > 0)
            {
            if (nullptr != transform)
                transform->Multiply (&points.front(), points.size());
            return ICurvePrimitive::CreateLineString (points);
            }
        }

    DPoint3dArray       poles;
    DwgDbDoubleArray    knots, weights;

    int16_t degree = 0;
    bool    rational = false, closed = false, periodic = false;
    double  poleTolerance = 1.0e-3, knotTolerance = 1.0e-5;

    if (DwgDbStatus::Success != spline.GetNurbsData (degree, rational, closed, periodic, poles, knots, weights, poleTolerance, knotTolerance))
        return  nullptr;

    if (nullptr != transform)
        transform->Multiply (&poles.front(), poles.size());

    int16_t order = degree + 1;

    // Use DWG knots only if they are a full set; otherwise use computed knots:
    if (knots.size() != BsplineParam::NumberAllocatedKnots(poles.size(), order, false))
        knots.clear ();

    MSBsplineCurvePtr   bspline = MSBsplineCurve::CreateFromPolesAndOrder (poles, &weights, &knots, order, closed, false);
    if (!bspline.IsValid())
        return  nullptr;

    return ICurvePrimitive::CreateBsplineCurve (*bspline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  DwgHelper::CreateCurveVectorFrom (DwgDbSplineCR spline, CurveVector::BoundaryType type, TransformCP transform, bool makeLinestring)
    {
    auto geom = DwgHelper::CreateCurvePrimitive (spline, transform, makeLinestring);
    return  geom.IsValid() ? CurveVector::Create(geom, type) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr  DwgHelper::CreateCurvePrimitive (DwgDbFaceCR face, TransformCP transform)
    {
    DPoint3dArray   points;
    for (uint16_t i = 0; i < 4; i++)
        {
        DPoint3d    point;
        if (face.GetVertexAt(point, i) != DwgDbStatus::Success)
            return  nullptr;
        if (nullptr != transform)
            transform->Multiply (point);
        points.push_back (point);
        }

    return ICurvePrimitive::CreateLineString (points);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  DwgHelper::CreateCurveVectorFrom (DwgDbFaceCR face, CurveVector::BoundaryType type, TransformCP transform)
    {
    auto geom = DwgHelper::CreateCurvePrimitive (face, transform);
    return  geom.IsValid() ? CurveVector::Create(geom, type) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  DwgHelper::CreateCurveVectorFrom (DwgDbRegionCR region, CurveVector::BoundaryType type, TransformCP transform)
    {
    auto shape = CurveVector::Create (type);
    if (!shape.IsValid())
        return  shape;

    DwgDbObjectPArray   entities;
    auto status = region.Explode (entities);
    if (status != DwgDbStatus::Success || entities.empty())
        return  nullptr;

    double      pointTol = 1.0e-10;
    DRange3d    range;
    if (DwgDbStatus::Success == region.GetRange(range))
        pointTol = 0.001 * range.DiagonalDistance ();

    auto numEntities = entities.size ();
    for (int i = 0; i < numEntities; i++)
        {
        auto entity = DwgDbEntity::Cast (entities[i]);
        if (nullptr != entity && shape.IsValid())
            {
            // if it's a region, recursively drop it:
            auto nestedRegion = DwgDbRegion::Cast (entity);
            if (nullptr != nestedRegion)
                {
                auto nestedShape = DwgHelper::CreateCurveVectorFrom (*nestedRegion, type, transform);
                if (nestedShape.IsValid())
                    shape->Add (nestedShape);
                else
                    BeAssert (false && "Nested region failed!");
                ::free (entities[i]);
                continue;
                }

            // expect primitive geometries only
            auto curve = DwgHelper::CreateCurvePrimitive (*entity, transform);
            if (curve.IsValid())
                shape->Add (curve);
            else // WIP - chain complex elements?
                shape = nullptr;
            }
        ::free (entities[i]);
        }

    // ensure non empty
    if (shape->empty())
        shape = nullptr;

    // ensure points are in order
    if (shape.IsValid())
        shape->ReorderForSmallGaps ();

    return  shape;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr  DwgHelper::CreateCurvePrimitive (DwgDbEntityCR entity, TransformCP transform)
    {
    DwgDbArcCP  arc = DwgDbArc::Cast (&entity);
    if (arc != nullptr)
        return  DwgHelper::CreateCurvePrimitive (*arc, transform);

    DwgDbLineCP line = DwgDbLine::Cast (&entity);
    if (line != nullptr)
        return  DwgHelper::CreateCurvePrimitive (*line, transform);
    
    DwgDbCircleCP   circle = DwgDbCircle::Cast (&entity);
    if (circle != nullptr)
        return  DwgHelper::CreateCurvePrimitive (*circle, transform);

    DwgDbEllipseCP  ellipse = DwgDbEllipse::Cast (&entity);
    if (ellipse != nullptr)
        return  DwgHelper::CreateCurvePrimitive (*ellipse, transform);

    DwgDbSplineCP   spline = DwgDbSpline::Cast (&entity);
    if (spline != nullptr)
        return  DwgHelper::CreateCurvePrimitive (*spline, transform, true);

    DwgDbFaceCP     face = DwgDbFace::Cast (&entity);
    if (face != nullptr)
        return  DwgHelper::CreateCurvePrimitive (*face, transform);

#ifdef DEBUG
    auto name = entity.GetDwgClassName ();
    BeAssert (false && "Unsupported entity type for curve primitive!");
#endif

    return  nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  DwgHelper::CreateCurveVectorFrom (DwgDbEntityCR entity, CurveVector::BoundaryType type, TransformCP transform)
    {
    DwgDbCircleCP   circle = DwgDbCircle::Cast (&entity);
    if (circle != nullptr)
        return  DwgHelper::CreateCurveVectorFrom (*circle, type, transform);

    DwgDbEllipseCP  ellipse = DwgDbEllipse::Cast (&entity);
    if (ellipse != nullptr)
        return  DwgHelper::CreateCurveVectorFrom (*ellipse, type, transform);
    
    DwgDbPolylineCP polyline = DwgDbPolyline::Cast (&entity);
    if (polyline != nullptr)
        return  DwgHelper::CreateCurveVectorFrom (*polyline, type, transform);
    
    DwgDb2dPolylineCP   polyline2d = DwgDb2dPolyline::Cast (&entity);
    if (polyline2d != nullptr)
        return  DwgHelper::CreateCurveVectorFrom (*polyline2d, type, transform);
    
    DwgDb3dPolylineCP   polyline3d = DwgDb3dPolyline::Cast (&entity);
    if (polyline3d != nullptr)
        return  DwgHelper::CreateCurveVectorFrom (*polyline3d, type, transform);
    
    // drop Spline curve to linestring - currently BSpline curve does not seem to clip DgnView well, TFS#589853.
    DwgDbSplineCP   spline = DwgDbSpline::Cast (&entity);
    if (spline != nullptr)
        return  DwgHelper::CreateCurveVectorFrom (*spline, type, transform, true);
    
    DwgDbRegionCP   region = DwgDbRegion::Cast (&entity);
    if (region != nullptr)
        return  DwgHelper::CreateCurveVectorFrom (*region, type, transform);
    
    DwgDbFaceCP     face = DwgDbFace::Cast (&entity);
    if (face != nullptr)
        return  DwgHelper::CreateCurveVectorFrom (*face, type, transform);
    
#ifdef DEBUG
    auto name = entity.GetDwgClassName ();
    BeAssert (false && "Unsupported entity type for boundary!");
#endif

    return  nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr  DwgHelper::CreateCurveVectorFrom (DwgDbObjectId entityId, CurveVector::BoundaryType type, TransformCP transform)
    {
    if (entityId.IsNull())
        return  nullptr;

    DwgDbCirclePtr  circle(entityId, DwgDbOpenMode::ForRead);
    if (circle.OpenStatus() == DwgDbStatus::Success)
        return  DwgHelper::CreateCurveVectorFrom (*circle.get(), type, transform);

    DwgDbEllipsePtr ellipse(entityId, DwgDbOpenMode::ForRead);
    if (ellipse.OpenStatus() == DwgDbStatus::Success)
        return  DwgHelper::CreateCurveVectorFrom (*ellipse.get(), type, transform);
    
    DwgDbArcPtr     arc(entityId, DwgDbOpenMode::ForRead);
    if (arc.OpenStatus() == DwgDbStatus::Success)
        return  CurveVector::Create(DwgHelper::CreateCurvePrimitive(*arc.get(), transform), type);

    DwgDbLinePtr    line(entityId, DwgDbOpenMode::ForRead);
    if (line.OpenStatus() == DwgDbStatus::Success)
        return  CurveVector::Create(DwgHelper::CreateCurvePrimitive(*line.get(), transform), type);

    DwgDbPolylinePtr polyline(entityId, DwgDbOpenMode::ForRead);
    if (polyline.OpenStatus() == DwgDbStatus::Success)
        return  DwgHelper::CreateCurveVectorFrom (*polyline.get(), type, transform);
    
    DwgDb2dPolylinePtr polyline2d(entityId, DwgDbOpenMode::ForRead);
    if (polyline2d.OpenStatus() == DwgDbStatus::Success)
        return  DwgHelper::CreateCurveVectorFrom (*polyline2d.get(), type, transform);
    
    DwgDb3dPolylinePtr polyline3d(entityId, DwgDbOpenMode::ForRead);
    if (polyline3d.OpenStatus() == DwgDbStatus::Success)
        return  DwgHelper::CreateCurveVectorFrom (*polyline3d.get(), type, transform);
    
    // drop Spline curve to linestring - currently BSpline curve does not seem to clip DgnView well, TFS#589853.
    DwgDbSplinePtr  spline(entityId, DwgDbOpenMode::ForRead);
    if (spline.OpenStatus() == DwgDbStatus::Success)
        return  DwgHelper::CreateCurveVectorFrom (*spline.get(), type, transform, true);
    
    DwgDbRegionPtr  region(entityId, DwgDbOpenMode::ForRead);
    if (region.OpenStatus() == DwgDbStatus::Success)
        return  DwgHelper::CreateCurveVectorFrom (*region.get(), type, transform);
    
    DwgDbFacePtr    face(entityId, DwgDbOpenMode::ForRead);
    if (face.OpenStatus() == DwgDbStatus::Success)
        return  DwgHelper::CreateCurveVectorFrom (*face.get(), type, transform);
    
#ifdef DEBUG
    auto name = entityId.GetDwgClassName ();
    BeAssert (false && "Unsupported entity type for boundary!");
#endif

    return  nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ClipVectorPtr   DwgHelper::CreateClipperFromEntity (DwgDbObjectId entityId, double* frontClip, double* backClip, TransformCP entityToClipper, TransformCP clipperToModel)
    {
    /*-----------------------------------------------------------------------------------
    Create a CurveVector w/coordinates on the clipper plane.
    Valid clipping entity types are Circle, Ellipse, Polyline, 2dPolyline, 3dPolyline, Region, Spline, Face.
    -----------------------------------------------------------------------------------*/
    CurveVectorPtr  curve = DwgHelper::CreateCurveVectorFrom (entityId, CurveVector::BOUNDARY_TYPE_Outer, entityToClipper);
    if (curve.IsNull())
        return  nullptr;

    // the clipper constructor expects CurveVector in model/world coordinate system:
    if (nullptr != clipperToModel)
        curve->TransformInPlace (*clipperToModel);

    return  ClipVector::CreateFromCurveVector(*curve, 0.001, 0.1, frontClip, backClip);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bmap<Utf8String, DwgFileVersion> CreateStringToVersionMap ()
    {
    // DWG version telltale strings to our supported enum versions
    bmap<Utf8String, DwgFileVersion> mappedVersion;

    mappedVersion["AC1002"] = DwgFileVersion::R2_5;
    mappedVersion["AC1003"] = DwgFileVersion::R2_6;
    mappedVersion["AC1004"] = DwgFileVersion::R9;
    mappedVersion["AC1005"] = DwgFileVersion::R9;
    mappedVersion["AC1006"] = DwgFileVersion::R10;
    mappedVersion["AC1007"] = DwgFileVersion::R10;
    mappedVersion["AC1008"] = DwgFileVersion::R10;
    mappedVersion["AC1009"] = DwgFileVersion::R11;
    mappedVersion["AC1010"] = DwgFileVersion::R11;
    mappedVersion["AC1011"] = DwgFileVersion::R11;
    mappedVersion["AC1012"] = DwgFileVersion::R13;
    mappedVersion["AC1013"] = DwgFileVersion::R14;
    mappedVersion["AC1014"] = DwgFileVersion::R14;
    mappedVersion["AC1015"] = DwgFileVersion::R2000;
    mappedVersion["AC1018"] = DwgFileVersion::R2004;
    mappedVersion["AC1021"] = DwgFileVersion::R2007;
    mappedVersion["AC1024"] = DwgFileVersion::R2010;
    mappedVersion["AC1027"] = DwgFileVersion::R2013;
    mappedVersion["AC1032"] = DwgFileVersion::R2018;
    return  mappedVersion;
    };
static bmap<Utf8String, DwgFileVersion> const s_stringToVersionMap = CreateStringToVersionMap ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bmap<DwgFileVersion, Utf8String> CreateDisplayVersionMap ()
    {
    // read-able DWG version strings
    bmap<DwgFileVersion, Utf8String> mappedString;

    mappedString[DwgFileVersion::Invalid] = "Invalid";
    mappedString[DwgFileVersion::Newer]   = "Too new";
    mappedString[DwgFileVersion::Unknown] = "Unknown";
    mappedString[DwgFileVersion::R2_6]    = "R2.6";
    mappedString[DwgFileVersion::R9]      = "R9";
    mappedString[DwgFileVersion::R10]     = "R10";
    mappedString[DwgFileVersion::R11]     = "R11";
    mappedString[DwgFileVersion::R13]     = "R13";
    mappedString[DwgFileVersion::R14]     = "R14";
    mappedString[DwgFileVersion::R2000]   = "R2000";
    mappedString[DwgFileVersion::R2004]   = "R2004";
    mappedString[DwgFileVersion::R2007]   = "R2007";
    mappedString[DwgFileVersion::R2010]   = "R2010";
    mappedString[DwgFileVersion::R2013]   = "R2013";
    mappedString[DwgFileVersion::R2018]   = "R2018";
    return  mappedString;
    };
static bmap<DwgFileVersion, Utf8String> const s_displayVersions = CreateDisplayVersionMap ();

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/17
+---------------+---------------+---------------+---------------+---------------+------*/
DwgFileVersion  DwgHelper::CheckDwgVersionString (Utf8StringCR versionString)
    {
    if (versionString.size() < 3 || versionString.size() > 6)
        return  DwgFileVersion::Invalid;

    auto found = s_stringToVersionMap.find (versionString.c_str());
    if (found != s_stringToVersionMap.end())
        return found->second;

    // The file could be a newer version, unsupported version or not a DWG at all - now check for "AC1" or "AC2":
    if ('A' == versionString[0] && 'C' == versionString[1] && ('1' == versionString[2] || '2' == versionString[2]))
        {
        // smells like a valid DWG, but is it a newer version not yet supported or a version not known to us?  Check the number followed by AC1:
        char    verBuffer[4];
        ::strncpy (verBuffer, &versionString[3], 3);
        verBuffer[3] = 0;

        char*   end = nullptr;
        int     verFromFile = ::strtol(verBuffer, &end, 10);
        int     maxSupported = LONG_MAX;

        // lookup the telltale string we support as the MAX version
        for (auto entry : s_stringToVersionMap)
            {
            if (DwgFileVersion::MAX == entry.second)
                {
                maxSupported = ::strtol(&entry.first[3], &end, 10);
                break;
                }
            }

        if (LONG_MAX == verFromFile || LONG_MIN == verFromFile || 0 == verFromFile)
            return DwgFileVersion::Invalid;
        else if (verFromFile > maxSupported)
            return DwgFileVersion::Newer;
        else
            return DwgFileVersion::Unknown;
        }

    return  DwgFileVersion::Invalid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String  DwgHelper::GetStringFromDwgVersion (DwgFileVersion dwgVersion)
    {
    // get a readable string for a DWG version
    auto found = s_displayVersions.find (dwgVersion);
    if (found != s_displayVersions.end())
        return  found->second;

    return  "Invalid";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DwgHelper::SniffDwgFile (BeFileNameCR dwgName, DwgFileVersion* versionOut)
    {
    BeFile          dwgFile;
    BeFileStatus    readStatus = dwgFile.Open (dwgName.c_str(), BeFileAccess::Read);
    if (BeFileStatus::Success != readStatus)
        return  false;

    // sniff only the first 6 ascii chars in file
    char    header[7] = { 0 };
    readStatus = dwgFile.Read (header, nullptr, 6);
    dwgFile.Close ();

    if (BeFileStatus::Success != readStatus)
        return  false;

    DwgFileVersion  foundVersion = DwgHelper::CheckDwgVersionString (header);
    if (nullptr != versionOut)
        *versionOut = foundVersion;

    // treat unknown version as a valid DWG file
    return DwgFileVersion::Invalid != foundVersion && DwgFileVersion::Newer != foundVersion;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          10/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DwgHelper::SniffDxfFile (BeFileNameCR dxfName, DwgFileVersion* versionOut)
    {
    /*-----------------------------------------------------------------------------------
    A fully qualified DXF file starts with a header section like this:
        0
    SECTION
        2
    HEADER
        9
    $ACADVER
        1
    AC10XX
        ...
    where AC10XX is the same string format as in DWG.  We check for "$ACADVER" followed
    by "AC10".  We do not want to read any file that happens to contain the string "AC10".
    -----------------------------------------------------------------------------------*/
    BeFile  dxfFile;
    BeFileStatus readStatus = dxfFile.Open (dxfName.c_str(), BeFileAccess::Read);
    if (BeFileStatus::Success != readStatus)
        return  false;

    // sniff first 4KB
    char header[4097] = { 0 };
    readStatus = dxfFile.Read (header, nullptr, sizeof(header));
    dxfFile.Close ();

    if (BeFileStatus::Success != readStatus)
        return  false;

    // check DXB
    if (0 == ::strncmp (header, "AutoCAD Binary DXF", 18))
        {
        if (nullptr != versionOut)
            *versionOut = DwgHelper::CheckDwgVersionString (Utf8String(&header[54], 6));
        return  true;
        }

    // check DXF
    CharCP  checkString = ::strstr (header, "$ACADVER");
    if (nullptr != checkString && nullptr != (checkString = ::strstr(checkString, "AC10")))
        {
        if (nullptr != versionOut)
            *versionOut = DwgHelper::CheckDwgVersionString (Utf8String(checkString, 6));
        return  true;
        }

    /*-----------------------------------------------------------------------------------
    A file with incomplete header or no header at all can still be a valid DXF file. A valid
    DXF should generally start with ENTITIES section. Unfortunately some crappy DXF files 
    may have huge table sections before reaching to ENTITIES section - TFS#346565.  Opening 
    the entire file just to sniff ENTITIES section suffers performance penalty.  We have 
    settled with sniffing SECTION instead.
    -----------------------------------------------------------------------------------*/
    if (nullptr != (checkString = ::strstr(header, "SECTION")))
        {
        // a DXF file without a version specified will be treated as the lowest version the toolkit supports:
        if (nullptr != versionOut)
            *versionOut = DwgFileVersion::Unknown;
        return true;
        }
    
    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DwgHelper::CanOpenForWrite (BeFileNameCR path)
    {
#ifdef BENTLEY_WIN32
    ::FILE* stream = nullptr;
    bool check = ::_wfopen_s(&stream, path.c_str(), L"r+") == 0;
    if (nullptr != stream)
        ::fclose (stream);
    return check;
#else
    BeAssert (false && "Unsupported platform!");
    return  false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t    DwgHelper::GetDwgImporterVersion ()
    {
    uint32_t    toolkitVersion = 0, importerVersion = 0;
#ifdef DLM_API_NUMBER
    // parse the DLL suffix "####b#"
    BeAssert (::sscanf(DLM_API_NUMBER, "%ub%u", &toolkitVersion, &importerVersion) == 2);
#else
    BeAssert (false && "DLM_API_NUMBER should be passed through the makefile!");
#endif
    return  importerVersion;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          05/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DwgHelper::GetTransformForSharedParts (TransformP out, double* uniformScale, TransformCR in)
    {
    // Factor out a transform to a pure uniform rotation + a translation for shared parts.
    RotMatrix   matrix, rotation, skew;
    DPoint3d    translation;

    in.GetMatrix (matrix);
    in.GetTranslation (translation);

    auto isValid = matrix.RotateAndSkewFactors (rotation, skew, 0, 1);
    if (isValid)
        {
        isValid = skew.IsIdentity ();
        if (!isValid)
            {
            RotMatrix   rigid;
            double      scale = 1.0;

            isValid = skew.IsRigidSignedScale (rigid, scale);
            if (isValid && nullptr != uniformScale)
                *uniformScale = scale;
            }
        }
    else
        {
        // should not occur!
        rotation.InitIdentity ();
        skew.InitIdentity ();
        }

    if (nullptr != out)
        {
        out->SetMatrix (rotation);
        out->SetTranslation (translation);
        }

    return  isValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/18
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DwgHelper::CompareSubcatAppearance (DgnSubCategory::Appearance const& a1, DgnSubCategory::Appearance const& a2)
    {
    // compare two inputs and return the first difference in a suffix string from the second input:
    Utf8String  diff;
    if (a1.IsInvisible() != a2.IsInvisible())
        diff = a2.IsInvisible() ? "_off" : "_on";
    else if (a1.GetDontPlot() != a2.GetDontPlot())
        diff = a2.GetDontPlot() ? "_pltOff" : "_pltOn";
    else if (a1.GetDontSnap() != a2.GetDontSnap())
        diff = a2.GetDontSnap() ? "_snpOff" : "_snpOn";
    else if (a1.GetDontLocate() != a2.GetDontLocate())
        diff = a2.GetDontLocate() ? "_locOff" : "_locOn";
    else if (a1.GetColor() != a2.GetColor())
        diff.Sprintf ("_clr0x%x", a2.GetColor().GetValue());
    else if (a1.GetWeight() != a2.GetWeight())
        diff.Sprintf ("_wt%d", a2.GetWeight());
    else if (a1.GetStyle() != a2.GetStyle())
        diff.Sprintf ("_st%lld", a2.GetStyle().IsValid() ? a2.GetStyle().GetValue() : 0);
    else if (a1.GetDisplayPriority() != a2.GetDisplayPriority())
        diff.Sprintf ("_pr%d", a2.GetDisplayPriority());
    else if (a1.GetRenderMaterial() != a2.GetRenderMaterial())
        diff.Sprintf ("_mat%lld", a2.GetRenderMaterial().IsValid() ? a2.GetRenderMaterial().GetValue() : 0);
    else if (a1.GetTransparency() != a2.GetTransparency())
        diff.Sprintf ("_trns%g", a2.GetTransparency());
    return  diff;
    }

