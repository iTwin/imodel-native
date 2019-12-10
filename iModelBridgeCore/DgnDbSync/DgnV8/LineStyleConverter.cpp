/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

#define MAX_LINECODE_RENAME_RETRIES 2
//---------------------------------------------------------------------------------------
// DgnDb's LsComponentType has the same values as V8's LsElementType so this is essentially
// just a cast unless new types are added to LsElementType.
//
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
static LsComponentType convertToLsComponentType(DgnV8Api::LsElementType elementType)
    {
    LsComponentType retval = (LsComponentType)elementType;
    if (!LsComponent::IsValidComponentType(retval))
        {
        BeAssert(LsComponent::IsValidComponentType(retval));
        return LsComponentType::Unknown;
        }

    return retval;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    07/2015
//---------------------------------------------------------------------------------------
LineStyleStatus LineStyleConverter::ConvertRasterImageComponent(Dgn::LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsRasterImageComponent const& rasterComponent)
    {
    Json::Value     jsonValue(Json::objectValue);

    SetDescription(jsonValue, rasterComponent);

    byte const* image;
    Bentley::Point2d imageSize;
    UInt32 flags;
    rasterComponent._GetRasterTexture(image, imageSize, flags);

    jsonValue["x"] = imageSize.x;
    jsonValue["y"] = imageSize.y;
    jsonValue["flags"] = flags;

    double trueWidth = rasterComponent.GetMaxWidth(nullptr);

    if (trueWidth != 0)
        jsonValue["trueWidth"] = trueWidth;

    uint32_t nImageBytes = (uint32_t)rasterComponent.GetImageBufferSize();

    return LsComponent::AddRasterComponentAsJson (v10Id, GetDgnDb(), jsonValue, image, nImageBytes);
    }

//---------------------------------------------------------------------------------------
/*
    ConvertLineCode creates an entry in the be_Prop table using namespace dgn_LStyle and Name LineCodeV1. The main body of the definition is Json saved in StrData.
    The LineCode code treats 0 as a default value for some of the properties and does not emit the property for some of the 0 values.  In this example, zero-valued properties
    that were omitted are "phase", "options", "maxIter", and "capMode".

    Here is an example of the Json portion.  The Json properties correspond to the fields of th V8 LineCodeRsc.

    {
    "descr":"water",
    "strokes":
        [
        {"endWidth":10000.0, "length":10000.0, "strokeMode":1, "widthMode":1},
        {"endWidth":0.0,"length":10000.0,"orgWidth":10000.0,"strokeMode":1,"widthMode":1},
        {"endWidth":10000.0,"length":10000.0,"strokeMode":1,"widthMode":2},
        ............

    We assume that the caller has already determined that this stroke pattern has not been mapped.
*/
// @bsimethod                                                   John.Gooding    09/2014
//---------------------------------------------------------------------------------------
LineStyleStatus LineStyleConverter::ConvertLineCode (LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheStrokePatternComponent const& strokePattern, double lsScale)
    {
    //  When converting a line style from V8 we have to convert it from the V8 units to meters.  That could be done either by changing the line style scale factor or by scaling
    //  the components.  I addedd lsScale to make it possible to scale the components but then decided it is better to scale the line style so lsScale should always be 1.
    //  It should be possible to remove the lsScale parameter.
    BeAssert(1.0 == lsScale);

    Json::Value     jsonValue(Json::objectValue);

    SetDescription(jsonValue, strokePattern);

    v10Id = LsComponentId();

    double phase = 0.0;
    uint32_t options = 0;
    uint32_t  maxIterate = 0;

    // Set phase, maxIterate, and options
    if (strokePattern.HasIterationLimit ())
        {
        maxIterate = strokePattern.GetIterationLimit ();
        options |= LCOPT_ITERATION;
        }

    if (strokePattern.IsSingleSegment())
        options |= LCOPT_SEGMENT;

    switch (strokePattern.GetPhaseMode ())
        {
        case DgnV8Api::LsCacheStrokePatternComponent::PHASEMODE_Fixed:
            phase = strokePattern.GetDistancePhase () * lsScale;
            break;
        case DgnV8Api::LsCacheStrokePatternComponent::PHASEMODE_Fraction:
            phase = strokePattern.GetFractionalPhase ();
            options |= LCOPT_AUTOPHASE;
            break;
        case DgnV8Api::LsCacheStrokePatternComponent::PHASEMODE_Center:
            phase = 0;
            options |= LCOPT_CENTERSTRETCH;
            break;
        }

    if (phase != 0)
        jsonValue["phase"] = phase;

    if (options != 0)
        jsonValue["options"] = options;

    if (maxIterate != 0)
        jsonValue["maxIter"] = maxIterate;

    Json::Value strokes(Json::arrayValue);
    uint32_t nStrokes = (uint32_t)strokePattern.GetNumberStrokes();

    for (uint32_t index = 0; index < nStrokes; ++index)
        {
        DgnV8Api::LsCacheStroke const* stroke = strokePattern.GetStrokeCP(index);
        BeAssert(stroke != nullptr);
        if (nullptr == stroke)
            continue;
        Json::Value  entry(Json::objectValue);
        entry["length"] = stroke->GetLength() * lsScale;
        if (stroke->GetStartWidth() != 0)
            entry["orgWidth"] = stroke->GetStartWidth() * lsScale;
        if (stroke->GetEndWidth() != stroke->GetStartWidth())
            entry["endWidth"] = stroke->GetEndWidth() * lsScale;
        int32_t strokeMode = 0;
        if (stroke->IsDash ())
            strokeMode |= LCSTROKE_DASH;
        if (stroke->IsDashFirst () != stroke->IsDash())
            strokeMode |= LCSTROKE_SINVERT;
        if (stroke->IsDashLast () != stroke->IsDash())
            strokeMode |= LCSTROKE_EINVERT;
        if (stroke->IsRigid ())
            strokeMode |= LCSTROKE_RAY;
        if (stroke->IsStretchable ())
            strokeMode |= LCSTROKE_SCALE;
        if (strokeMode != 0)
            entry["strokeMode"] = strokeMode;
        if (stroke->GetWidthMode() != 0)
            entry["widthMode"] = stroke->GetWidthMode();
        if ((int32_t)stroke->GetCapMode() != 0)
            entry["capMode"] = (int32_t)stroke->GetCapMode();

        strokes[index] = entry;
        }

    jsonValue["strokes"] = strokes;

    return LsComponent::AddComponentAsJsonProperty(v10Id, GetDgnDb(), LsComponentType::LineCode, jsonValue);
    }

//---------------------------------------------------------------------------------------
/*
    ConvertLinePoint creates an entry in the be_Prop table using namespace dgn_LStyle and Name LinePointV1. The main body of the definition
    is Json saved in StrData. The Json properties correspond to the fields of th V8 LinePointRsc.

    ConvertLinePoint typically does not emit properties that have a value of 0 since 0 is considered to be the default value.

    Here is an example showing the Json portion of the definition. In this case "xOffset", "yOffset", "xOffset", and "angle" were
    all zero so the corresponding properties were not added to the Json object.

    {
    "descr":"Tree Line - Point",
    "lcId":10,
    "symbols":[
        {"mod1":1,"strokeNum":0,"symId":10}
        ]
    }
*/
// @bsimethod                                                   John.Gooding    09/2014
//---------------------------------------------------------------------------------------
LineStyleStatus LineStyleConverter::ConvertLinePoint (LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCachePointComponent const& linePointComponent, double lsScale)
    {
    Json::Value     jsonValue(Json::objectValue);

    SetDescription(jsonValue, linePointComponent);

    v10Id = LsComponentId();

    //  A PointComponent definition is invalid without the stroke pattern that guides the layout of the points.
    DgnV8Api::LsCacheStrokePatternComponent const* strokePattern = linePointComponent.GetStrokeComponentCP();
    BeAssert(nullptr != strokePattern);
    if (nullptr == strokePattern)
        return LINESTYLE_STATUS_ComponentNotFound;

    LsComponentId newId;
    ConvertLsComponent(newId, v8File, *strokePattern, lsScale);
    LsPointComponent::SaveLineCodeIdToJson(jsonValue, newId);

    Json::Value symbols(Json::arrayValue);
    for (unsigned i = 0; i < linePointComponent.GetNumberSymbols(); ++i)
        {
        Json::Value  entry(Json::objectValue);

        DgnV8Api::LsCacheSymbolReference const* symbolRef = linePointComponent.GetSymbolCP(i);
        BeAssert(nullptr != symbolRef);
        DgnV8Api::LsCacheSymbolComponent const* symbolComponent = symbolRef->GetSymbolComponentCP();
        if (nullptr == symbolComponent)
            continue;
        LsComponentId symbolId;

        ConvertLsComponent (symbolId, v8File, *symbolComponent, lsScale);
        if (!symbolId.IsValid())
            continue;
        LsPointComponent::SaveSymbolIdToJson(entry, symbolId);

        entry["strokeNum"] = symbolRef->GetStrokeNumber();
        if (symbolRef->GetXOffset() != 0.0)
            entry["xOffset"] = symbolRef->GetXOffset() * lsScale;
        if (symbolRef->GetYOffset() != 0.0)
            entry["yOffset"] = symbolRef->GetYOffset() * lsScale;
        if (symbolRef->GetAngle() != 0.0)
            entry["angle"] = symbolRef->GetAngle();

        //  To get the value for m_mod1, assign the various values and then get extract mod1.
        LsSymbolReference   sr;
        sr.SetNoPartial(symbolRef->GetNoPartial());
        sr.SetClipPartial(symbolRef->GetClipPartial());
        sr.SetStretchable(symbolRef->GetStretchable());
        sr.SetDgnDb(symbolRef->GetProject());
        sr.SetUseElementColor(symbolRef->GetUseElementColor());        //  use color from element
        sr.SetUseElementWeight(symbolRef->GetUseElementWeight());      //  use weight from element
        sr.SetJustification((LsSymbolReference::StrokeJustification)symbolRef->GetJustification());
        sr.SetRotationMode((LsSymbolReference::RotationMode)symbolRef->GetRotationMode());
        sr.SetVertexMask((LsSymbolReference::VertexMask)symbolRef->GetVertexMask());

        if (sr.GetMod1() != 0)
            entry["mod1"] = sr.GetMod1();

        symbols.append(entry);
        }

    jsonValue["symbols"]=symbols;

    return LsComponent::AddComponentAsJsonProperty(v10Id, GetDgnDb(), LsComponentType::LinePoint, jsonValue);
    }

//---------------------------------------------------------------------------------------
/*
    ConvertCompoundComponent creates an entry in the be_Prop table using namespace dgn_LStyle and Name CompoundV1.

    Here is an example showing the Json portion of the definition.  The offset property defaults to 0.

    {
    "descr":"Rail Road - Compound",
    "comps":[
        {"id":8,"type":4},
        {"id":9,"offset":-20000.0,"type":3},
        {"id":9,"offset":20000.0,"type":3}
        ]
    }

    The Json properties correspond to the fields of th V8 LinePointRsc.
*/
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LineStyleStatus LineStyleConverter::ConvertCompoundComponent (LsComponentId&v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheCompoundComponent const& compoundComponent, double lsScale)
    {
    Json::Value     jsonValue(Json::objectValue);

    SetDescription(jsonValue, compoundComponent);

    v10Id = LsComponentId();

    uint32_t numComponents = (uint32_t)compoundComponent.GetNumComponents();
    Json::Value components(Json::arrayValue);

    for (size_t i = 0; i < numComponents; ++i)
        {
        DgnV8Api::LsCacheComponent const* child = compoundComponent.GetComponentCP(i);
        if (nullptr == child)
            {
            BeAssert(nullptr != child);
            return LINESTYLE_STATUS_ComponentNotFound;
            }

        uint32_t type = (uint32_t)child->GetElementType();
        double offset = compoundComponent.GetOffsetToComponent(i) * lsScale;

        uint32_t id;
        DgnV8Api::LsCacheInternalComponent const* internalComponent = dynamic_cast<DgnV8Api::LsCacheInternalComponent const*>(child);
        if (NULL != internalComponent)
            {
            BeAssert(LsComponentType::Internal == (LsComponentType)type);
            id = internalComponent->GetHardwareStyle();
            }
        else
            {
            LsComponentId newId;
            ConvertLsComponent(newId, v8File, *child, lsScale);
            id = newId.GetValue();
            type = (uint32_t)newId.GetType();
            }

        Json::Value  entry(Json::objectValue);
        if (offset != 0)
            entry["offset"] = offset;
        entry["id"] = id;
        entry["type"] = type;
        components.append(entry);
        }

    jsonValue["comps"]=components;

    return LsComponent::AddComponentAsJsonProperty(v10Id, GetDgnDb(), LsComponentType::Compound, jsonValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2014
//---------------------------------------------------------------------------------------
bool LineStyleConverter::V8Location::operator<(const V8Location &rhs) const
    {
    if (m_v8componentKey < rhs.m_v8componentKey) return true;         if (m_v8componentKey > rhs.m_v8componentKey) return false;
    if (m_v8componentType < rhs.m_v8componentType) return true;       if (m_v8componentType > rhs.m_v8componentType) return false;
    if (m_isElement < rhs.m_isElement) return true;                   if (m_isElement > rhs.m_isElement) return false;
    return m_v8fileId < rhs.m_v8fileId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2014
//---------------------------------------------------------------------------------------
LineStyleConverter::V8Location::V8Location(DgnV8Api::LsCacheComponent const& component, Converter& converter)
    {
    DgnV8Api::LsLocation const* lsLoc = component.GetLocation();

    m_isElement         = lsLoc->IsElement();
    m_v8componentKey    = lsLoc->GetIdentKey();

    //  We don't bother recording RscFileHandle because we assume that during a session the
    //  search paths through resource files will be the same every time
    //  and that a search for the same ID and type will yield the same resource. Therefore,
    //  we don't try to check location based on RscFileHandle.  We only do a file test
    //  if the component definition comes from an element.
    if (m_isElement)
        m_v8fileId = converter.GetRepositoryLinkId(*lsLoc->GetSourceFile());

    m_v8componentType   = (uint32_t)lsLoc->GetElementType();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2014
//---------------------------------------------------------------------------------------
LineStyleStatus LineStyleConverter::ConvertLsComponent (LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheComponent const& component, double lsScale)
    {
    //  Pretty sure lsScale is a vestige from when the converter used to scale components at convert time.  Now it is always done at runtime.
    BeAssert(1.0 == lsScale);
    LsComponentType compType = (LsComponentType)component.GetElementType();

    if (DgnV8Api::LsElementType::Internal == (DgnV8Api::LsElementType)compType)
        {
        DgnV8Api::LsCacheInternalComponent const* internalComponent = dynamic_cast<DgnV8Api::LsCacheInternalComponent const*>(&component);
        BeAssert(NULL != internalComponent);
        v10Id = LsComponentId(LsComponentType::Internal, internalComponent->GetLineCode());
        return LINESTYLE_STATUS_Success;
        }

    //  There are bad files that lead to DgnV8's GetElementType returning the wrong value -- a value that is different
    //  than what is used to create the file.  Therefore we rely on dynamic_cast.
    if (dynamic_cast<DgnV8Api::LsCacheStrokePatternComponent const*>(&component))
        compType = LsComponentType::LineCode;
    else if (dynamic_cast<DgnV8Api::LsCacheCompoundComponent const*>(&component))
        compType = LsComponentType::Compound;
    else if (dynamic_cast<DgnV8Api::LsCachePointComponent const*>(&component))
        compType = LsComponentType::LinePoint;
    else if (dynamic_cast<DgnV8Api::LsCacheSymbolComponent const*>(&component))
        compType = LsComponentType::PointSymbol;
    else if (dynamic_cast<DgnV8Api::LsRasterImageComponent const*>(&component))
        compType = LsComponentType::RasterImage;

    BeAssert(compType == (LsComponentType)component.GetElementType());

    //  If the component has already been imported then use the existing definition.
    V8Location  v8Location(component, m_converter);
    auto result = m_v8ComponentToV10Id.find(v8Location);
    if (result != m_v8ComponentToV10Id.end())
        {
        v10Id = LsComponentId(convertToLsComponentType(component.GetElementType()), result->second);
        return LINESTYLE_STATUS_Success;
        }

    LineStyleStatus retval = LINESTYLE_STATUS_Success;

    switch (compType)
        {
        case LsComponentType::LineCode:
            {
            DgnV8Api::LsCacheStrokePatternComponent const& sp = (DgnV8Api::LsCacheStrokePatternComponent const&)component;
            retval = ConvertLineCode (v10Id, v8File, sp, lsScale);
            }
            break;

        case LsComponentType::Compound:
            {
            DgnV8Api::LsCacheCompoundComponent const&compound = (DgnV8Api::LsCacheCompoundComponent const&)component;
            retval = ConvertCompoundComponent(v10Id, v8File, compound, lsScale);
            }
            break;

        case LsComponentType::LinePoint:
            {
            DgnV8Api::LsCachePointComponent const&lp = (DgnV8Api::LsCachePointComponent const&)component;
            retval = ConvertLinePoint(v10Id, v8File, lp, lsScale);
            }
            break;

        case LsComponentType::PointSymbol:
            {
            DgnV8Api::LsCacheSymbolComponent const&ps = (DgnV8Api::LsCacheSymbolComponent const&)component;
            retval = ConvertPointSymbol(v10Id, v8File, ps, lsScale);
            }
            break;

        case LsComponentType::RasterImage:
            {
            DgnV8Api::LsRasterImageComponent const&ri = (DgnV8Api::LsRasterImageComponent const&)component;
            retval = ConvertRasterImageComponent(v10Id, v8File, ri);
            }
            break;

        default:
            BeAssert(false && "unexpected component type");
            return LINESTYLE_STATUS_BadFormat;
        }

    m_v8ComponentToV10Id[v8Location] = v10Id.GetValue();

    return retval;
    }

//---------------------------------------------------------------------------------------
// Turn a DGN line code (1 through 7) into a line style referring to a component defined as
// { LsComponentType::Internal, line-code-value }.
//
//  This is only called by GetOrConvertElementLineCode.  GetOrConvertElementLineCode first
//  looks for an existing definition and only calls this if the existing definition is not found.
//
// @bsimethod                                                   John.Gooding    05/2016
//---------------------------------------------------------------------------------------
LineStyleStatus LineStyleConverter::ConvertElementLineCode(DgnStyleId& newId, uint32_t lineCode)
    {
    BeAssert(lineCode >= 1 && lineCode <= 7);
    char lsName[20];
    sprintf(lsName, "lc%d", lineCode);
    double componentScale =  1;
    uint32_t lineStyleAttributes = LSATTR_UNITDEV | LSATTR_NOSNAP;
    LsComponentId   v10ComponentId(LsComponentType::Internal, lineCode);

    BentleyStatus result = GetDgnDb().LineStyles().Insert(newId, lsName, v10ComponentId, lineStyleAttributes, componentScale);
    for (uint32_t i = 0; SUCCESS != result && i < MAX_LINECODE_RENAME_RETRIES; ++i)
        {
        //  This could be valid if the converter imported a style with a name like lc4, but it is more likely due to us inserting the same line code more than once.
        //  GetOrConvertElementLineCode should prevent that from happening.
        strcat(lsName, "_");
        result = GetDgnDb().LineStyles().Insert(newId, lsName, v10ComponentId, lineStyleAttributes, componentScale);
        }

    if (SUCCESS != result)
        {
        BeAssert(SUCCESS == result);
        newId = DgnStyleId();
        return LINESTYLE_STATUS_Error;
        }

    Utf8String nameKey(lsName);
    m_lsDefNameToIdMap[nameKey] = newId;

    return LINESTYLE_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
//  The BIM file format does not support hardcoded line codes in geometry so we replaced
//  the hardcoded 1 through 7 with line styles that use the line codes.  The name of the
//  line style is generated as lc# where # is the DGN line code.
//
//  GetSyncInfo().FindLineStyle finds the line code to line style mapping if this is not the
//  first element using the line code.  If it is the first element using the line code
//  but the project is being updated, the project may already contain a definition of the line style.
//  If so, use that.  If not, create the corresponding line style.
//
// @bsimethod                                                   John.Gooding    04/2017
//---------------------------------------------------------------------------------------
LineStyleStatus LineStyleConverter::GetOrConvertElementLineCode(DgnStyleId& newId, uint32_t lineCode)
    {
    char lsName[20];
    sprintf(lsName, "lc%d", lineCode);

    for (uint32_t i = 0; i < MAX_LINECODE_RENAME_RETRIES; ++i)
        {
        DgnStyleId styleId = LineStyleElement::QueryId(GetDgnDb(), lsName);

        if (styleId.IsValid())
            {
            //  We already have a line style with this name.  Use it if it is defined correctly.
            LineStyleElementCPtr ls = LineStyleElement::Get(GetDgnDb(), styleId);

            if (ls.IsValid())
                {
                Utf8String  data (ls->GetData());

                Json::Value  jsonObj (Json::objectValue);
                if (Json::Reader::Parse(data, jsonObj))
                    {
                    LsComponentId compId = LsDefinition::GetComponentId(jsonObj);

                    if (compId.GetType() == LsComponentType::Internal && compId.GetValue() == lineCode)
                        {
                        // The project file already contained the expected definition. Use that.
                        newId = styleId;
                        return LINESTYLE_STATUS_Success;
                        }
                    }
                }
            }

        //  In the unlikely case that there is a line style name something like lc5 that is not
        //  the desired line style we try again with lc5_, lc5__, etc.
        strcat(lsName, "_");
        }

    //  The project file does not contain the "lc#" line style.  Create it.
    return ConvertElementLineCode(newId, lineCode);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2014
//---------------------------------------------------------------------------------------
LineStyleStatus LineStyleConverter::ConvertLineStyle (DgnStyleId& newId, double& componentScale, DgnV8Api::LsDefinition*v8ls, DgnV8Api::DgnFile&v8File)
    {
    if (BSISUCCESS != m_converter.MustBeInSharedChannel("linestyles must be converted in the shared channel."))
        return LineStyleStatus::LINESTYLE_STATUS_Error;

    Utf8String  lineStyleName(v8ls->GetStyleName().c_str());

    //  First check to see if it is already mapped.  There are some files with line style names that
    //  differ only in case.  When MicroStation encounters them, it only loads the first one.  It
    //  will not load multiple line styles that differ only by case.  To mimic that behavior, this code
    //  creates the m_lsDefNameToIdMap using lower case names.
    Utf8String  nameLC(lineStyleName.c_str());
    nameLC.ToLower();
    auto mappedId = m_lsDefNameToIdMap.find(nameLC);
    if (mappedId != m_lsDefNameToIdMap.end())
        {
        newId = mappedId->second;

        if (!newId.IsValid())
            return LineStyleStatus::LINESTYLE_STATUS_Error;

        LsCacheR lsCache = GetDgnDb().LineStyles().GetCache();
        LsDefinitionP nameRec = lsCache.GetLineStyleP(newId);

        if (nullptr == nameRec)
            return LineStyleStatus::LINESTYLE_STATUS_Error;

        componentScale = nameRec->GetUnitsDefinition();

        return LINESTYLE_STATUS_Success;
        }

    LsComponentId v10ComponentId;
    DgnV8Api::LsCacheComponent const* v8component = v8ls->GetComponentCP(&v8File.GetDictionaryModel());
    if (nullptr == v8component)
        {
        m_converter.ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::MissingLsDefinition(), NULL, lineStyleName.c_str());
        m_lsDefNameToIdMap[nameLC] = DgnStyleId((uint64_t)1LL);
        return LINESTYLE_STATUS_ComponentNotFound;
        }

    uint32_t  lineStyleAttributes = v8ls->GetAttributes();

    auto unitsDefModel = GetUnitsDefinitionModel();
    if (nullptr == unitsDefModel)
        {
        m_converter.ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::MissingLsDefinitionFile(), NULL, lineStyleName.c_str());
        return LineStyleStatus::LINESTYLE_STATUS_Error;
        }

    DgnV8Api::ModelInfo const& v8ModelInfo = unitsDefModel->GetModelInfo();
    double uorPerMeter = DgnV8Api::ModelInfo::GetUorPerMeter(&v8ModelInfo);
    double muPerMeter = uorPerMeter/DgnV8Api::ModelInfo::GetUorPerMaster(&v8ModelInfo);

    //  Need to handle units that are Master, Uor, or Device.
    //
    //  Suport for line styles with device units has been dropped. We don't have a suitable replacement.
    //
    //  We are turning "Master units line styles" into "Meters line styles".  When converting to meters we have a choice of converting all components or just changing the
    //  line style's scale factor.  I've elected to make the change in the line style scale, leaving the component values unchanged.
    BeAssert(v8ls->IsUnitsMaster() || v8ls->IsUnitsUOR());

    //  This value can be used to convert a V8 line style value to a DgnDb line style value.  A line style value is a width or distance encountered in a line style component
    //  or LineStyleParams. During conversion it should be used to scale startWidth, endWidth, and distPhase of LineStyleParams. It also should be used to scale
    //  m_storedScale or m_muDef of LsSymbolComponent, m_offset of LsSymbolReference, m_offset for compound component offset, m_length, m_orgWidth, and m_endWidth of LsStroke,
    componentScale =  v8ls->GetUnitsDefinition();

    //  Judging from the code in LsSymbology.cpp we want to leave unitsDefinition as zero if IsUnitsDevice is true.  It looks like GetUnitsDefinition is simply a flag indicating
    //  whether to adjust for GetPixelSizeAtPoint.
    if (!v8ls->IsUnitsDevice() && componentScale < mgds_fc_epsilon)
        componentScale = 1.0;

    if (v8ls->IsUnitsMaster())
        {
        lineStyleAttributes = (lineStyleAttributes & ~LSATTR_UNITMASK)   |  static_cast<int>(LsUnit::Meters);
        componentScale /= muPerMeter;
        }
    else if (v8ls->IsUnitsUOR())
        {
        lineStyleAttributes = (lineStyleAttributes & ~LSATTR_UNITMASK)   |  static_cast<int>(LsUnit::Meters);
        componentScale /= uorPerMeter;
        }


    double lsUnitDef = componentScale;

    //  If the project is being updated, the style may have been mapped in a previous session.  If so,
    //  it may be in the file even though it is not in the m_lsDefNameToIdMap map.
    DgnStyleId styleId = LineStyleElement::QueryId(GetDgnDb(), lineStyleName.c_str());
    BentleyStatus result = BSIERROR;
    if (styleId.IsValid())
        {
        //  We already have a line style with this name.  Use it.
        //  NEEDSWORK LineStyles This logic only executes if the file is being updated and the existing
        //  project contains a definition of the line style. This logic should update the line style definition if
        //  it has changed.  Maybe it should update it unconditionally.
        LineStyleElementCPtr ls = LineStyleElement::Get(GetDgnDb(), styleId);

        if (ls.IsValid())
            {
            m_lsDefNameToIdMap[nameLC] = styleId;
            newId = styleId;
            result = BSISUCCESS;
            }
        }

    if (BSISUCCESS != result)
        {
        //  I think the final argument can be eliminated.  I've added "BeAssert(1.0 == lsScale)" to ConvertLsComponent to prove it.
        ConvertLsComponent(v10ComponentId, v8File, *v8component, 1.0);
        // NB: Since we don't specify a model, the Insert function will automatically put the new linestyle element in the DictionaryModel.
        result = GetDgnDb().LineStyles().Insert(newId, lineStyleName.c_str(), v10ComponentId, lineStyleAttributes, lsUnitDef);
        }

    if (BSISUCCESS == result)
        {
        m_lsDefNameToIdMap[nameLC] = newId;
        return LINESTYLE_STATUS_Success;
        }

    //  Avoid repeating the error message for this line style.
    m_lsDefNameToIdMap[nameLC] = DgnStyleId((uint64_t)1LL);

    return LINESTYLE_STATUS_SQLITE_Error;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2014
//---------------------------------------------------------------------------------------
void LineStyleConverter::SetDescription (V10ComponentBase*v10, DgnV8Api::LsCacheComponent const& component)
    {
    WString  wDescr (component.GetDescription().c_str());
    Utf8String  descr;
    descr.Assign(wDescr.c_str());
    v10->SetDescription(descr.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LineStyleConverter::SetDescription (JsonValueR json, DgnV8Api::LsCacheComponent const& component)
    {
    WString  wDescr (component.GetDescription().c_str());
    Utf8String  descr;
    descr.Assign(wDescr.c_str());
    json["descr"] = descr.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2014
//---------------------------------------------------------------------------------------
LineStyleConverterPtr LineStyleConverter::Create(Converter&converter)
    {
    return new LineStyleConverter(converter);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
DgnStyleId Converter::_RemapLineStyle(double&unitsScale, DgnV8Api::DgnFile&v8File, int32_t srcLineStyleNum, bool required)
    {
    unitsScale = 1;
    if (0 == srcLineStyleNum)
        return DgnStyleId((uint64_t)srcLineStyleNum);

    if (STYLE_ByLevel == srcLineStyleNum || STYLE_ByCell == srcLineStyleNum)
        return DgnStyleId((uint64_t)srcLineStyleNum);

    //  There are 2 data structures used for mapping from V8 line styles to DgnDb line styles.
    //
    //  m_lsDefNameToIdMap maps based on name.  A line style with a given name is imported once,
    //  regardless of how many files are imported to the project.  The converter uses the first
    //  occurrence based on name. It does not report duplicate names as an error. It does not
    //  try to confirm that the line styles are the same.
    //
    //  m_lineStyle maps from V8 style number to DgnDb style number. The information in
    //  m_lineStyle is scoped by file ID.  There may be many entries in m_lineStyle that
    //  map to the same DgnDb ID.
    //
    //  It seems odd to have line codes, i.e., style numbers 1-7, use file scoping.  However, if
    //  we don't everything that calls FindLineStyle has to know to treat 1-7 differently.
    //
    RepositoryLinkId v8FileId = GetRepositoryLinkId(v8File);
    SyncInfo::V8StyleId  srcLineStyleId(v8FileId, srcLineStyleNum);

    bool  foundStyle = false;
    DgnStyleId mappedId = GetSyncInfo().FindLineStyle(unitsScale, foundStyle, srcLineStyleId);
    if (foundStyle)
        //  Use the mappedId even if it is invalid if it cam from SyncInfo.  We do this to avoid emitting the
        //  "line style not found" error message more than once for a given style.
        return mappedId;

    if (IS_LINECODE(srcLineStyleNum))
        {
        m_lineStyleConverter->GetOrConvertElementLineCode(mappedId, srcLineStyleNum);

        GetSyncInfo().InsertLineStyle(mappedId, 1, srcLineStyleId);
        return mappedId;
        }

    //  If srcLineStyleNum is greater than 0 look in the system map first.  That is a MicroStation rule, so we follow it when using the MicroStation API.
    DgnV8Api::LsDefinition*   v8LsDef = srcLineStyleNum > 0 ? DgnV8Api::LsSystemMap::GetSystemMapP (true)->Find (srcLineStyleNum) : NULL;
    if (NULL == v8LsDef)
        v8LsDef = DgnV8Api::LsMap::FindInDgnOrRsc(&v8File, srcLineStyleNum);

    if (NULL == v8LsDef)
        {
        if (!required)
            //  The missing line style is the source file's list of mapped line styles but we don't know whether it is used so we do not want to complain about it
            //  and we do not want too map it to the default.  We will do that if we find an actual use of it.
            return DgnStyleId();

        //  It is needed by an element so we need to warn that it has not been found.
        WString  lsName (DgnV8Api::LineStyleManager::GetNameFromNumber(srcLineStyleNum, &v8File).c_str());
        if (lsName[0] == 0)
            ReportIssueV(IssueSeverity::Warning, IssueCategory::InconsistentData(), Issue::LineStyleNumberError(), NULL, srcLineStyleNum);
        else
            {
            Utf8String lsName8(lsName.c_str());
            ReportIssueV(IssueSeverity::Warning, IssueCategory::InconsistentData(), Issue::MissingLsDefinition(), NULL, lsName8.c_str());
            }

        GetSyncInfo().InsertLineStyle(DgnStyleId(), unitsScale, srcLineStyleId);
        return DgnStyleId();
        }

    m_lineStyleConverter->ConvertLineStyle(mappedId, unitsScale, v8LsDef, v8File);

    GetSyncInfo().InsertLineStyle(mappedId, unitsScale, srcLineStyleId);
    return mappedId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
void Converter::ConvertAllLineStyles(DgnV8Api::DgnFile&v8File)
    {
    if (BSISUCCESS != MustBeInSharedChannel("linestyles must be converted in the shared channel."))
        return;

    /* According to Chuck:
        * DGN & DGNLib use the units of the default model.
        * RSC uses the units that the linestyle is defined in: UORs, Master, or Pixels.
          (The default is Master and that's what is most used, but there's a flag on each style for which set of units it is.
        * LIN files are in meters. The associated SHP files are unitless (fonts) so the scale from the LIN has to be applied.
    */
    auto defaultModel = v8File.LoadModelById(v8File.GetDefaultModelId());   // hold refcountedptr to keep this model alive within the scope of this function.
    if (!defaultModel.IsValid())
        {
        ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::CorruptData(), Converter::Issue::CannotLoadModel(), nullptr, Utf8String(v8File.GetFileName().c_str()).c_str());
        return;
        }
    m_lineStyleConverter->SetUnitsDefinitionModel(defaultModel.get());

    // Initialize the graphics subsystem to produce bitmaps of linestyles.
    // This was required when the line style converter converted line styles to textures
    //  at conversion time.  We haven't done that in a long time so I think this is no longer
    //  required.

    DgnV8Api::LsMap* lsMap = DgnV8Api::LsMap::GetMapPtr(v8File, true);

    for (int32_t i = 1; i <= MAX_LINECODE; i++)
        {
        double unitsScale;
        _RemapLineStyle(unitsScale, v8File, i, true);
        }

    //  Get the iterator to the ID map of the DGN file.  Each entry in the
    //  map corresponds to a NameRec (i.e., LsDefinition) in the DGN file.
    //  The entry in the DGN file is at least a name-ID mapping going in
    //  both directions. The entry in the DGN may also contain a reference to
    //  an element that defines the component.  If it does not refer to an element
    //  then the name is used to look up a corresponding NameRec in a resource file.
    //  That will contain a reference to the top level component in the resource file.
    for (DgnV8Api::LsMapIterator ls = lsMap->begin(); ls != lsMap->end(); ++ls)
        {
        double unitsScale;
        _RemapLineStyle(unitsScale, v8File, ls->GetStyleNumber(), false);
        }

    m_lineStyleConverter->SetUnitsDefinitionModel(nullptr);
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
