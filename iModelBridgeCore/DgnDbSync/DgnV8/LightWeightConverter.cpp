/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/LightWeightConverter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <GeoCoord\BaseGeoCoord.h>

// Via RequiredRepository entry in the DgnV8ConverterDLL Part. The way this piece was designed was that is was so small, every library gets and builds as source.
#include "../../V8IModelExtraFiles/V8IModelExtraFiles.h"

#include <VersionedDgnV8Api/PSolid/PSolidCore.h>
#if defined (BENTLEYCONFIG_PARASOLID) 
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

#undef min
#undef max
#undef GetClassName

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE


//----------------------------------------------------------------------------------------
// @bsimethod                                    Vern.Francisco                      1/18
//+---------------+---------------+---------------+---------------+---------------+-------
void LightWeightConverter::Initialize()
    {
    // Directly register basic DgnV8 converter extensions here (that platform owns).
    // In the future, may need an extensibility point here to allow apps and/or arbitrary DLLs to participate in this process.
    ConvertV8TextToDgnDbExtension::Register();
    ConvertV8TagToDgnDbExtension::Register();
    ConvertV8Lights::Register();
    ConvertThreeMxAttachment::Register();
    ConvertScalableMeshAttachment::Register();
    ConvertDetailingSymbolExtension::Register();
   // CifSheetExaggeratedViewHandlerStandin::Register();

    //Ensure tha V8i::DgnGeocoord is using the GCS library from this application admin.
    //    DgnV8Api::ConfigurationManager::UndefineVariable(L"MS_GEOCOORDINATE_DATA");
    //    DgnV8Api::ConfigurationManager::DefineVariable(L"MS_GEOCOORDINATE_DATA", T_HOST.GetGeoCoordinationAdmin()._GetDataDirectory().c_str());

    //    Bentley::GeoCoordinates::BaseGCS::Initialize(T_HOST.GetGeoCoordinationAdmin()._GetDataDirectory().c_str());
    //    InitCustomGcsDir(argc, argv);

    // Must register all domains as required, so that OpenDgnDb will import them. We are not allowed to import schemas later in the conversion process.
    //  Note that this bridge delivers the domains that it uses, and their schemas are in the bridge's assets directory, not the platform's assets directory.
    //    DgnDomains::RegisterDomain(FunctionalDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No, &bridgeAssetsDir);
    //    DgnDomains::RegisterDomain(Raster::RasterDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No, &bridgeAssetsDir);
    //    DgnDomains::RegisterDomain(PointCloud::PointCloudDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No, &bridgeAssetsDir);
    //    DgnDomains::RegisterDomain(ThreeMx::ThreeMxDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No, &bridgeAssetsDir);
    //    DgnDomains::RegisterDomain(ScalableMeshSchema::ScalableMeshDomain::GetDomain(), DgnDomain::Required::Yes, DgnDomain::Readonly::No, &bridgeAssetsDir);
    //    ScalableMesh::ScalableMeshLib::Initialize(*new SMHost());

    //    for (auto xdomain : XDomainRegistry::s_xdomains)
    //        xdomain->_RegisterDomain(bridgeAssetsDir);

    //    DomainInitCaller caller;
    //    DgnV8Api::ElementHandlerManager::EnumerateAvailableHandlers(caller);
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco     01/2018
//---------------------------------------------------------------------------------------
void LightWeightConverter::SetMaterialUsed(RenderMaterialId id) { m_materialUsed.insert(id); }
bool LightWeightConverter::GetMaterialUsed(RenderMaterialId id) const { return m_materialUsed.find(id) != m_materialUsed.end(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco     01/2018
//---------------------------------------------------------------------------------------
RenderMaterialId LightWeightConverter::GetRemappedMaterial(DgnV8Api::Material const* material)
    {
    auto const& found = m_materialRemap.find((void*) material);                // TBD.   Figure out why using DgnV8Api::Material* as key won't compile.

    if (found != m_materialRemap.end())          // First look for direct mapping to material address.
        {
        SetMaterialUsed(found->second);
        return found->second;
        }

    Utf8String utfMaterialName(material->GetName().c_str()), utfPaletteName(material->GetPalette().GetName().c_str());

    auto const& foundByName = m_materialNameRemap.find(T_MaterialNameKey(utfMaterialName, utfPaletteName));

    if (foundByName != m_materialNameRemap.end())
        {
        // NEEDS_WORK -- Verify that material matches.
        SetMaterialUsed(foundByName->second);
        return foundByName->second;
        }

    return RenderMaterialId();
    }






//---------------------------------------------------------------------------------------
//
//  *******[Vern] This method was just copied from the normal LineStyleConverter *******
//
// @bsimethod                                                   Vern.Francisco    11/2017
//---------------------------------------------------------------------------------------
static LsComponentType convertToLsComponentType(DgnV8Api::LsElementType elementType)
    {
    LsComponentType retval = (LsComponentType) elementType;
    if (!LsComponent::IsValidComponentType(retval))
        {
        BeAssert(LsComponent::IsValidComponentType(retval));
        return LsComponentType::Unknown;
        }

    return retval;
    }

//---------------------------------------------------------------------------------------
//
//  *******[Vern] This method was just copied from the normal LineStyleConverter *******
//
// @bsimethod                                                   Vern.Francisco    11/2017
//---------------------------------------------------------------------------------------
bool LightWeightLineStyleConverter::V8Location::operator<(const V8Location &rhs) const
    {
    if (m_v8componentKey < rhs.m_v8componentKey) return true;         if (m_v8componentKey > rhs.m_v8componentKey) return false;
    if (m_v8componentType < rhs.m_v8componentType) return true;       if (m_v8componentType > rhs.m_v8componentType) return false;
    if (m_isElement < rhs.m_isElement) return true;                   if (m_isElement > rhs.m_isElement) return false;
    return m_v8fileId < rhs.m_v8fileId;
    }

//---------------------------------------------------------------------------------------
//
//  *******[Vern] This method was just copied from the normal LineStyleConverter *******
//
// @bsimethod                                                   Vern.Francisco    11/2017
//---------------------------------------------------------------------------------------
LightWeightLineStyleConverter::V8Location::V8Location(DgnV8Api::LsCacheComponent const& component, DgnDbPtr dgnDb)
    {
    DgnV8Api::LsLocation const* lsLoc = component.GetLocation();

    m_isElement = lsLoc->IsElement();
    m_v8componentKey = lsLoc->GetIdentKey();

    //  We don't bother recording RscFileHandle because we assume that during a session the 
    //  search paths through resource files will be the same every time
    //  and that a search for the same ID and type will yield the same resource. Therefore,
    //  we don't try to check location based on RscFileHandle.  We only do a file test
    //  if the component definition comes from an element.
    //   if (m_isElement)
    //       m_v8fileId = Converter::GetV8FileSyncInfoIdFromAppData(*lsLoc->GetSourceFile());

    m_v8componentType = (uint32_t) lsLoc->GetElementType();
    }


//---------------------------------------------------------------------------------------
//
//  *******[Vern] This method was just copied from the normal LineStyleConverter *******
//
// @bsimethod                                                   Vern.Francisco    11/2017
//---------------------------------------------------------------------------------------
LineStyleStatus LightWeightLineStyleConverter::ConvertLsComponent(LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheComponent const& component, double lsScale)
    {
    //  Pretty sure lsScale is a vestige from when the converter used to scale components at convert time.  Now it is always done at runtime.
    BeAssert(1.0 == lsScale);
    LsComponentType compType = (LsComponentType) component.GetElementType();

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

    BeAssert(compType == (LsComponentType) component.GetElementType());

    //  If the component has already been imported then use the existing definition.
    V8Location  v8Location(component, m_dgnDb);
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
            retval = ConvertLineCode(v10Id, v8File, sp, lsScale);
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
//  *******[Vern] This method was just copied from the normal LineStyleConverter *******
//
// @bsimethod                                                   Vern.Francisco    11/2017
//---------------------------------------------------------------------------------------
LineStyleStatus LightWeightLineStyleConverter::ConvertElementLineCode(DgnStyleId& newId, uint32_t lineCode)
    {
    BeAssert(lineCode >= 1 && lineCode <= 7);
    Utf8String lsName;
    lsName.Sprintf("%s:lc%d", m_codePrefix.c_str(), lineCode);
    double componentScale = 1;
    uint32_t lineStyleAttributes = LSATTR_UNITDEV | LSATTR_NOSNAP;
    LsComponentId   v10ComponentId(LsComponentType::Internal, lineCode);

    BentleyStatus result = GetDgnDb().LineStyles().Insert(newId, lsName.c_str(), v10ComponentId, lineStyleAttributes, componentScale);

    if (SUCCESS != result)
        {
        BeAssert(SUCCESS == result);
        newId = DgnStyleId();
        return LINESTYLE_STATUS_Error;
        }

    m_lsDefNameToIdMap[lsName] = newId;

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
//  *******[Vern] This method was just copied from the normal LineStyleConverter *******
//
// @bsimethod                                                   Vern.Francisco    11/2017
//---------------------------------------------------------------------------------------
LineStyleStatus LightWeightLineStyleConverter::GetOrConvertElementLineCode(DgnStyleId& newId, uint32_t lineCode)
    {

    Utf8String lsName;
    lsName.Sprintf("%s:lc%d", m_codePrefix.c_str(), lineCode);

    DgnStyleId styleId = LineStyleElement::QueryId(GetDgnDb(), lsName.c_str());

    if (styleId.IsValid())
        {
        //  We already have a line style with this name.  Use it if it is defined correctly.
        LineStyleElementCPtr ls = LineStyleElement::Get(GetDgnDb(), styleId);

        if (ls.IsValid())
            {
            Utf8String  data(ls->GetData());

            Json::Value  jsonObj(Json::objectValue);
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

    //  The project file does not contain the "lc#" line style.  Create it.
    return ConvertElementLineCode(newId, lineCode);
    }



//---------------------------------------------------------------------------------------
//
//  *******[Vern] This method was just copied from the normal LineStyleConverter *******
//
// @bsimethod                                                   Vern.Francisco    11/2017
//---------------------------------------------------------------------------------------
LineStyleStatus LightWeightLineStyleConverter::ConvertLineStyle(DgnStyleId& newId, double& componentScale, DgnV8Api::LsDefinition*v8ls, DgnV8Api::DgnFile&v8File)
    {
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
        return LINESTYLE_STATUS_Success;
        }

    LsComponentId v10ComponentId;
    DgnV8Api::LsCacheComponent const* v8component = v8ls->GetComponentCP(&v8File.GetDictionaryModel());
    if (nullptr == v8component)
        {
        //        m_converter.ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::MissingData(), Converter::Issue::MissingLsDefinition(), NULL, lineStyleName.c_str());
        m_lsDefNameToIdMap[nameLC] = DgnStyleId((uint64_t) 1LL);
        return LINESTYLE_STATUS_ComponentNotFound;
        }

    uint32_t  lineStyleAttributes = v8ls->GetAttributes();

    DgnV8Api::ModelInfo const& v8ModelInfo = GetRootModel()->GetModelInfo();
    double uorPerMeter = DgnV8Api::ModelInfo::GetUorPerMeter(&v8ModelInfo);
    double muPerMeter = uorPerMeter / DgnV8Api::ModelInfo::GetUorPerMaster(&v8ModelInfo);

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
    componentScale = v8ls->GetUnitsDefinition();

    //  Judging from the code in LsSymbology.cpp we want to leave unitsDefinition as zero if IsUnitsDevice is true.  It looks like GetUnitsDefinition is simply a flag indicating 
    //  whether to adjust for GetPixelSizeAtPoint.
    if (!v8ls->IsUnitsDevice() && componentScale < mgds_fc_epsilon)
        componentScale = 1.0;

    if (v8ls->IsUnitsMaster())
        {
        lineStyleAttributes = (lineStyleAttributes & ~LSATTR_UNITMASK) | static_cast<int>(LsUnit::Meters);
        componentScale /= muPerMeter;
        }
    else if (v8ls->IsUnitsUOR())
        {
        lineStyleAttributes = (lineStyleAttributes & ~LSATTR_UNITMASK) | static_cast<int>(LsUnit::Meters);
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
        result = GetDgnDb().LineStyles().Insert(newId, lineStyleName.c_str(), v10ComponentId, lineStyleAttributes, lsUnitDef);
        }

    if (BSISUCCESS == result)
        {
        m_lsDefNameToIdMap[nameLC] = newId;
        return LINESTYLE_STATUS_Success;
        }

    //  Avoid repeating the error message for this line style.
    m_lsDefNameToIdMap[nameLC] = DgnStyleId((uint64_t) 1LL);

    return LINESTYLE_STATUS_SQLITE_Error;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco    11/2017
//---------------------------------------------------------------------------------------

LightWeightLineStyleConverterPtr LightWeightLineStyleConverter::Create(LightWeightConverter& converter, DgnDbPtr dgnDb, Utf8String codePrefix, DgnCategoryId defaultCategory, DgnSubCategoryId defaultSubCategory)
    {
    return new LightWeightLineStyleConverter(converter, dgnDb, codePrefix, defaultCategory, defaultSubCategory);
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco   01/2018
//---------------------------------------------------------------------------------------
DgnStyleId LightWeightConverter::_RemapLineStyle(double&unitsScale, DgnV8Api::DgnFile&v8File, int32_t srcLineStyleNum, bool required)
    {
    unitsScale = 1;
    if (0 == srcLineStyleNum)
        return DgnStyleId((uint64_t) srcLineStyleNum);

    if (STYLE_ByLevel == srcLineStyleNum || STYLE_ByCell == srcLineStyleNum)
        return DgnStyleId((uint64_t) srcLineStyleNum);

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

    bool  foundStyle = false;
    DgnStyleId mappedId = FindLineStyle(unitsScale, foundStyle, srcLineStyleNum);

    if (foundStyle)
        //  Use the mappedId even if it is invalid if it cam from SyncInfo.  We do this to avoid emitting the
        //  "line style not found" error message more than once for a given style.
        return mappedId;

    if (IS_LINECODE(srcLineStyleNum))
        {
        m_lineStyleConverter->GetOrConvertElementLineCode(mappedId, srcLineStyleNum);
        InsertLineStyle(mappedId, 1, srcLineStyleNum);
        return mappedId;
        }

    //  If srcLineStyleNum is greater than 0 look in the system map first.  That is a MicroStation rule, so we follow it when using the MicroStation API.
    DgnV8Api::LsDefinition*   v8LsDef = srcLineStyleNum > 0 ? DgnV8Api::LsSystemMap::GetSystemMapP(true)->Find(srcLineStyleNum) : NULL;
    if (NULL == v8LsDef)
        v8LsDef = DgnV8Api::LsMap::FindInDgnOrRsc(&v8File, srcLineStyleNum);

    if (NULL == v8LsDef)
        {
        if (!required)
            //  The missing line style is the source file's list of mapped line styles but we don't know whether it is used so we do not want to complain about it
            //  and we do not want too map it to the default.  We will do that if we find an actual use of it. 
            return DgnStyleId();

        //  It is needed by an element so we need to warn that it has not been found.
        WString  lsName(DgnV8Api::LineStyleManager::GetNameFromNumber(srcLineStyleNum, &v8File).c_str());
        if (lsName[0] == 0)
            {
            //  ReportIssueV(IssueSeverity::Warning, IssueCategory::InconsistentData(), Issue::LineStyleNumberError(), NULL, srcLineStyleNum);
            }
        else
            {
            Utf8String lsName8(lsName.c_str());
            //   ReportIssueV(IssueSeverity::Warning, IssueCategory::InconsistentData(), Issue::MissingLsDefinition(), NULL, lsName8.c_str());
            }

        InsertLineStyle(DgnStyleId(), unitsScale, srcLineStyleNum);
        return DgnStyleId();
        }

    m_lineStyleConverter->ConvertLineStyle(mappedId, unitsScale, v8LsDef, v8File);

    InsertLineStyle(mappedId, unitsScale, srcLineStyleNum);
    return mappedId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco    11/2017
//---------------------------------------------------------------------------------------

void LightWeightConverter::SetRootModel(DgnV8ModelP r)
    {
    m_lineStyleConverter->SetRootModel(r);
    }




//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco    11/2017
//---------------------------------------------------------------------------------------
LineStyleStatus LightWeightLineStyleConverter::ConvertLinePoint(LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCachePointComponent const& linePointComponent, double lsScale)
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
        LsComponentId symbolId;

        ConvertLsComponent(symbolId, v8File, *symbolComponent, lsScale);
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

    jsonValue["symbols"] = symbols;

    return LsComponent::AddComponentAsJsonProperty(v10Id, GetDgnDb(), LsComponentType::LinePoint, jsonValue);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco    11/2017
//---------------------------------------------------------------------------------------
LineStyleStatus LightWeightLineStyleConverter::ConvertCompoundComponent(LsComponentId&v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheCompoundComponent const& compoundComponent, double lsScale)
    {
    Json::Value     jsonValue(Json::objectValue);

    SetDescription(jsonValue, compoundComponent);

    v10Id = LsComponentId();

    uint32_t numComponents = (uint32_t) compoundComponent.GetNumComponents();
    Json::Value components(Json::arrayValue);

    for (size_t i = 0; i < numComponents; ++i)
        {
        DgnV8Api::LsCacheComponent const* child = compoundComponent.GetComponentCP(i);
        if (nullptr == child)
            {
            BeAssert(nullptr != child);
            return LINESTYLE_STATUS_ComponentNotFound;
            }

        uint32_t type = (uint32_t) child->GetElementType();
        double offset = compoundComponent.GetOffsetToComponent(i) * lsScale;

        uint32_t id;
        DgnV8Api::LsCacheInternalComponent const* internalComponent = dynamic_cast<DgnV8Api::LsCacheInternalComponent const*>(child);
        if (NULL != internalComponent)
            {
            BeAssert(LsComponentType::Internal == (LsComponentType) type);
            id = internalComponent->GetHardwareStyle();
            }
        else
            {
            LsComponentId newId;
            ConvertLsComponent(newId, v8File, *child, lsScale);
            id = newId.GetValue();
            type = (uint32_t) newId.GetType();
            }

        Json::Value  entry(Json::objectValue);
        if (offset != 0)
            entry["offset"] = offset;
        entry["id"] = id;
        entry["type"] = type;
        components.append(entry);
        }

    jsonValue["comps"] = components;

    return LsComponent::AddComponentAsJsonProperty(v10Id, GetDgnDb(), LsComponentType::Compound, jsonValue);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco    11/2017
//---------------------------------------------------------------------------------------
LineStyleStatus LightWeightLineStyleConverter::ConvertLineCode(LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsCacheStrokePatternComponent const& strokePattern, double lsScale)
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
    if (strokePattern.HasIterationLimit())
        {
        maxIterate = strokePattern.GetIterationLimit();
        options |= LCOPT_ITERATION;
        }

    if (strokePattern.IsSingleSegment())
        options |= LCOPT_SEGMENT;

    switch (strokePattern.GetPhaseMode())
        {
            case DgnV8Api::LsCacheStrokePatternComponent::PHASEMODE_Fixed:
                phase = strokePattern.GetDistancePhase() * lsScale;
                break;
            case DgnV8Api::LsCacheStrokePatternComponent::PHASEMODE_Fraction:
                phase = strokePattern.GetFractionalPhase();
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
    uint32_t nStrokes = (uint32_t) strokePattern.GetNumberStrokes();

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
        if (stroke->IsDash())
            strokeMode |= LCSTROKE_DASH;
        if (stroke->IsDashFirst() != stroke->IsDash())
            strokeMode |= LCSTROKE_SINVERT;
        if (stroke->IsDashLast() != stroke->IsDash())
            strokeMode |= LCSTROKE_EINVERT;
        if (stroke->IsRigid())
            strokeMode |= LCSTROKE_RAY;
        if (stroke->IsStretchable())
            strokeMode |= LCSTROKE_SCALE;
        if (strokeMode != 0)
            entry["strokeMode"] = strokeMode;
        if (stroke->GetWidthMode() != 0)
            entry["widthMode"] = stroke->GetWidthMode();
        if ((int32_t) stroke->GetCapMode() != 0)
            entry["capMode"] = (int32_t) stroke->GetCapMode();

        strokes[index] = entry;
        }

    jsonValue["strokes"] = strokes;

    return LsComponent::AddComponentAsJsonProperty(v10Id, GetDgnDb(), LsComponentType::LineCode, jsonValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco    11/2017
//---------------------------------------------------------------------------------------
LineStyleStatus LightWeightLineStyleConverter::ConvertRasterImageComponent(Dgn::LsComponentId& v10Id, DgnV8Api::DgnFile&v8File, DgnV8Api::LsRasterImageComponent const& rasterComponent)
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

    uint32_t nImageBytes = (uint32_t) rasterComponent.GetImageBufferSize();

    return LsComponent::AddRasterComponentAsJson(v10Id, GetDgnDb(), jsonValue, image, nImageBytes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2014
//---------------------------------------------------------------------------------------
void LightWeightLineStyleConverter::SetDescription(V10ComponentBase*v10, DgnV8Api::LsCacheComponent const& component)
    {
    WString  wDescr(component.GetDescription().c_str());
    Utf8String  descr;
    descr.Assign(wDescr.c_str());
    v10->SetDescription(descr.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LightWeightLineStyleConverter::SetDescription(JsonValueR json, DgnV8Api::LsCacheComponent const& component)
    {
    WString  wDescr(component.GetDescription().c_str());
    Utf8String  descr;
    descr.Assign(wDescr.c_str());
    json["descr"] = descr.c_str();
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                   Vern.Francisco   01/2018
//---------------------------------------------------------------------------------------
BentleyStatus LightWeightConverter::ConvertElement(DgnV8EhCR v8eh, GeometryBuilderPtr builder, DgnCategoryId categoryId)
    {

    //if element doesn't have any ECInstances and is a non-graphical element, just skip the element
    if ( NULL != v8eh.GetElementRef() && !v8eh.GetElementRef()->IsGraphics())
        return BentleyApi::SUCCESS;

    if (!categoryId.IsValid())
        {
        categoryId = m_defaultCategoryId;
        if (!categoryId.IsValid())
            {
            Utf8String instanceStr = "";
//            ReportIssue(IssueSeverity::Warning, IssueCategory::CorruptData(), Issue::ConvertFailure(), Utf8PrintfString("[%s] - Invalid level %d%s - Using Default Uncategorized category instead.", IssueReporter::FmtElement(v8eh).c_str(), v8eh.GetElementCP()->ehdr.level, instanceStr.c_str()).c_str());
            categoryId = m_defaultCategoryId;
            }
        }


    if (BentleyApi::SUCCESS != _CreateElementGeom(categoryId, v8eh, builder ))
        return BSIERROR;

    //if (wouldBe3dMismatch(results, v8mm))
    //    {
    //    ReportIssue(IssueSeverity::Error, IssueCategory::Unsupported(), Issue::UnsupportedPrimaryInstance(), IssueReporter::FmtElement(v8eh).c_str());
    //    elementClassId = ComputeElementClassIgnoringEcContent(v8eh, v8mm);
    //    auto was = m_skipECContent;
    //    m_skipECContent = true;
    //    results.m_element = nullptr;
    //    results.m_v8PrimaryInstance = V8ECInstanceKey();
    //    hasPrimaryInstance = false;
    //    auto res = _CreateElementAndGeom(results, v8mm, elementClassId, hasPrimaryInstance, categoryId, elementCode, v8eh);
    //    m_skipECContent = was;
    //    if (BentleyApi::SUCCESS != res)
    //        return BSIERROR;
    //    }
    //      }

    return BentleyApi::SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Vern.Francisco                      12/17
// +---------------+---------------+---------------+---------------+---------------+------

DgnStyleId LightWeightConverter::FindLineStyle(double& unitsScale, bool& foundStyle, Int32 v8Id)
    {
    //// WIP_CONVERTER -- read syncInfo
    auto i = m_lineStyle.find(v8Id);
    if (i == m_lineStyle.end())
        {
        unitsScale = 1;
        foundStyle = false;
        return DgnStyleId();
        }

    foundStyle = true;
    unitsScale = i->second.m_unitsScale;
    return i->second.m_id;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                    Vern.Francisco                      12/17
// +---------------+---------------+---------------+---------------+---------------+------

DbResult LightWeightConverter::InsertLineStyle(DgnStyleId newId, double componentScale, Int32 oldId)
    {
    MappedLineStyle mapEntry(newId, componentScale);
    m_lineStyle[oldId] = mapEntry;

    // WIP_CONVERTER -- write syncinfo 
    return BE_SQLITE_DONE;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Vern.Francisco                      12/17
// +---------------+---------------+---------------+---------------+---------------+------

DgnSubCategoryId LightWeightConverter::GetSubCategory(uint32_t v8levelid, SyncInfo::Level::Type ltype)
    {

    DgnV8Api::LevelCache const& lc = m_v8Model->GetLevelCache();
    DgnV8Api::LevelHandle level = lc.GetLevel(v8levelid);
    if (!level.IsValid())
        return m_defaultSubCategoryId;

    auto rootSubCatId = ConvertLevelToSubCategory(level, *m_v8Model, m_defaultCategoryId);

    if ( !rootSubCatId.IsValid() )
        return m_defaultSubCategoryId;

    return rootSubCatId;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Vern.Francisco                      12/17
// +---------------+---------------+---------------+---------------+---------------+------
DgnSubCategoryId LightWeightConverter::ConvertLevelToSubCategory(DgnV8Api::LevelHandle const& level, DgnV8ModelCR v8Model, DgnCategoryId catid)
    {
    if (!catid.IsValid())
        return m_defaultSubCategoryId;

    Utf8String dbSubCategoryName(level.GetName());

    //// *** NEEDS WORK - we should not have models with no names!
    //// *** vvvTEMPORARY FIXvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
    //if (dbSubCategoryName.empty())
    //    dbSubCategoryName = Utf8PrintfString("model_%llu", dgnModel->GetModeledElementId().GetValue());
    // *** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    //dbSubCategoryName.Trim(); // in DgnDb, we don't allow leading or trailing blanks
    //dbSubCategoryName.insert(0, _GetNamePrefix().c_str());
    //DgnDbTable::ReplaceInvalidCharacters(dbSubCategoryName, DgnCategory::GetIllegalCharacters(), '_');

    DgnSubCategoryId dbSubCategoryId = DgnSubCategory::QuerySubCategoryId(GetDgnDb(), DgnSubCategory::CreateCode(GetDgnDb(), catid, dbSubCategoryName));
    if (dbSubCategoryId.IsValid())
        {
        //  We've already created a subcategory by this name. Map this V8 level to it.
        if (LOG_LEVEL_IS_SEVERITY_ENABLED(NativeLogging::LOG_TRACE))
            LOG_LEVEL.tracev("merged level [%s] (%d) -sub-> [%s] (%d)", Utf8String(level.GetName()).c_str(), level.GetLevelId(), dbSubCategoryName.c_str(), dbSubCategoryId.GetValue());
        }
    else
        {
        //  First time we've seen a level for a model with this name. Create a subcategory for it.
        DgnSubCategory::Appearance appear;
        ComputeSubCategoryAppearanceFromLevel(appear, level);

        DgnSubCategory newSubCategoryData(DgnSubCategory::CreateParams(GetDgnDb(), catid, dbSubCategoryName, appear, Utf8String(level->GetDescription())));

        auto newSubCategory = newSubCategoryData.Insert();
        if (!newSubCategory.IsValid())
            {
            if (LOG_LEVEL_IS_SEVERITY_ENABLED(NativeLogging::LOG_TRACE))
                LOG_LEVEL.tracev("failed to insert subcategory for level [%s] (%d) as [%s]", Utf8String(level.GetName()).c_str(), level.GetLevelId(), newSubCategoryData.GetSubCategoryName().c_str());
            BeAssert(false);
            return DgnSubCategoryId();
            }

        dbSubCategoryId = newSubCategory->GetSubCategoryId();
        BeAssert(dbSubCategoryId.IsValid());

        if (LOG_LEVEL_IS_SEVERITY_ENABLED(NativeLogging::LOG_TRACE))
            LOG_LEVEL.tracev("inserted level [%s] (%d) -> [%s] (%d)", Utf8String(level.GetName()).c_str(), level.GetLevelId(), newSubCategory->GetSubCategoryName().c_str(), dbSubCategoryId.GetValue());
        }

    // NEEDSWORK LevelMap cache is needed. 

    //m_syncInfo.InsertLevel(dbSubCategoryId, SyncInfo::V8ModelSource(v8Model), level);
    return dbSubCategoryId;
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                    Vern.Francisco                      12/17
// +---------------+---------------+---------------+---------------+---------------+------
DgnCode LightWeightConverter::CreateCode(Utf8StringCR value) const
    {
    auto codeSpec = m_dgndb->CodeSpecs().GetCodeSpec(m_businessKeyCodeSpecId);
    BeDataAssert(codeSpec.IsValid());
    // May NEEDWORK
    return codeSpec.IsValid() ? codeSpec->CreateCode (value) : DgnCode();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Vern.Francisco                      12/17
// +---------------+---------------+---------------+---------------+---------------+------
void LightWeightConverter::ComputeSubCategoryAppearanceFromLevel(DgnSubCategory::Appearance& appear, DgnV8Api::LevelHandle const& level)
    {
    // Note: we do not import the "override" symbology and on/off flags. We use view-specific level overrides to give the user an equivalent way to customize symbology.
    // *** WIP_V10_LEVELS: How to carry over existing override symbology to new view-oriented system?

    DgnV8Api::IntColorDef v8ColorDef;
    DgnV8Api::DgnColorMap::ExtractElementColorInfo(&v8ColorDef, nullptr, nullptr, nullptr, nullptr, level->GetByLevelColor().GetColor(), *level->GetByLevelColor().GetDefinitionFile());

//    auto material = m_syncInfo.FindMaterialByV8Id(level->GetByLevelMaterial().GetId(), *level->GetByLevelMaterial().GetDefinitionFile(), *level->GetByLevelMaterial().GetDefinitionModel());
    auto color = ColorDef(v8ColorDef.m_int);
    auto weight = level->GetByLevelWeight();
    //  WIP_V10_LEVELS -- Do we care about whether there is a line style scale?
    double unitsScale = 0;
    bool foundStyle = false;
    auto lineStyleId = FindLineStyle(unitsScale, foundStyle, (uint32_t) level->GetByLevelLineStyle().GetStyle());

    appear.SetInvisible(false); // It never makes sense to define a SubCategory that is invisible. If a level is off in a view, then we will turn off this SubCategory in that view.
    appear.SetDisplayPriority(level->GetDisplayPriority());
    appear.SetTransparency(level->GetTransparency());
    appear.SetDontPlot(!level->GetPlot());
    appear.SetDontSnap(!level->GetSnap());
    appear.SetDontLocate(!level->GetLocate());
    appear.SetColor(color);
    appear.SetWeight(weight);
    appear.SetStyle(lineStyleId);
  //  if (material.IsValid())
  //      appear.SetRenderMaterial(material);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Vern.Francisco                      12/17
// +---------------+---------------+---------------+---------------+---------------+------
void LightWeightConverter::EmbedFonts()
    {
    _EmbedFonts();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Vern.Francisco                      12/17
// +---------------+---------------+---------------+---------------+---------------+------

LightWeightConverter::LightWeightConverter(DgnDbPtr dgndb, DgnV8FileP dgnV8File, DgnModelPtr model, DgnV8ModelP v8Model, Utf8String codePrefix, DgnCategoryId defaultCategoryId, DgnSubCategoryId defaultSubCategoryId, Int32 converterId)
    {
    m_dgndb = dgndb; m_rootFile = dgnV8File; m_model = model;
    m_v8Model = v8Model; m_defaultCategoryId = defaultCategoryId;
    m_converterId = converterId;
    m_defaultSubCategoryId = defaultSubCategoryId;
    m_lineStyleConverter = LightWeightLineStyleConverter::Create(*this, dgndb, codePrefix, defaultCategoryId, defaultSubCategoryId);
    m_lineStyleConverter->SetRootModel(v8Model);
    InitBusinessKeyCodeSpec();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                    Vern.Francisco                      12/17
// +---------------+---------------+---------------+---------------+---------------+------

LightWeightConverter::~LightWeightConverter()
    {}

static const Utf8CP s_codeSpecNameLW = "DgnV8LW"; // TBD: One CodeSpec per V8 file?

//----------------------------------------------------------------------------------------
// @bsimethod                                    Vern.Francisco                      12/17
// +---------------+---------------+---------------+---------------+---------------+------

void LightWeightConverter::InitBusinessKeyCodeSpec()
    {
    m_businessKeyCodeSpecId = m_dgndb->CodeSpecs().QueryCodeSpecId(s_codeSpecNameLW);
    if (!m_businessKeyCodeSpecId.IsValid())
        {
        CodeSpecPtr codeSpec = CodeSpec::Create(*m_dgndb, s_codeSpecNameLW);
        BeAssert(codeSpec.IsValid());
        if (codeSpec.IsValid())
            {
            codeSpec->Insert();
            m_businessKeyCodeSpecId = codeSpec->GetCodeSpecId();
            }
        }

    BeAssert(m_businessKeyCodeSpecId.IsValid());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
DefinitionModelPtr LightWeightConverter::GetJobDefinitionModel()
    {
    if (m_jobDefinitionModelId.IsValid())
        return m_dgndb->Models().Get<DefinitionModel>(m_jobDefinitionModelId);

    SubjectCPtr job = m_dgndb->Elements().GetRootSubject();
    Utf8PrintfString partitionName("Definition Model For %s", job->GetDisplayLabel().c_str());
    DgnCode partitionCode = DefinitionPartition::CreateCode(*job, partitionName);
    DgnElementId partitionId = m_dgndb->Elements().QueryElementIdByCode(partitionCode);
    m_jobDefinitionModelId = DgnModelId(partitionId.GetValueUnchecked());
    if (m_jobDefinitionModelId.IsValid())
        return m_dgndb->Models().Get<DefinitionModel>(m_jobDefinitionModelId);

    DefinitionPartitionPtr ed = DefinitionPartition::Create(*job, partitionName.c_str());
    DefinitionPartitionCPtr partition = ed->InsertT<DefinitionPartition>();
    if (!partition.IsValid())
        return DefinitionModelPtr();

    DefinitionModelPtr defModel = DefinitionModel::CreateAndInsert(*partition);
    if (!defModel.IsValid())
        return DefinitionModelPtr();

    m_jobDefinitionModelId = defModel->GetModelId();
    return defModel;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE

