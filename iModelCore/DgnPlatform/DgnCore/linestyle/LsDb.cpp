/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsDb.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define PROPNAME_Descr "Descr"
#define PROPNAME_Data "Data"

static Utf8CP DGNPROPERTYBLOB_CompId                = "compId";
static Utf8CP DGNPROPERTYBLOB_CompType              = "compType";
static Utf8CP DGNPROPERTYBLOB_Flags                 = "flags";
static Utf8CP DGNPROPERTYBLOB_UnitDef               = "unitDef";


//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//---------------------------------------------------------------------------------------
void V10ComponentBase::GetDescription (Utf8P target) const
    {
    BeStringUtilities::Strncpy(target, LS_MAX_DESCR, m_descr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    07/2015
//---------------------------------------------------------------------------------------
void V10ComponentBase::SetVersion()
    {
    m_version = InitialDgnDb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//---------------------------------------------------------------------------------------
void V10ComponentBase::SetDescription (Utf8CP source)
    {
    memset (m_descr, 0, sizeof (m_descr));
    BeStringUtilities::Strncpy(m_descr, LS_MAX_DESCR, source);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentPtr LsStrokePatternComponent::_Import(DgnImportContext& importer) const
    {
    LsStrokePatternComponentP result = new LsStrokePatternComponent(this);

    //  Save to destination and record ComponentId in clone

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsStrokePatternComponent::CreateRscFromDgnDb(V10LineCode** rscOut, DgnDbR project, LsComponentId componentId)
    {
    *rscOut = NULL;
    uint32_t propertySize;

    if (BE_SQLITE_ROW != project.QueryPropertySize(propertySize, LineStyleProperty::LineCode(), componentId.GetValue(), 0))
        return BSIERROR;

    V10LineCode* lineCodeData = (V10LineCode*)bentleyAllocator_malloc (propertySize);

    project.QueryProperty(lineCodeData, propertySize, LineStyleProperty::LineCode(), componentId.GetValue(), 0);
    BeAssert (propertySize == V10LineCode::GetBufferSize(lineCodeData->m_nStrokes));

    *rscOut = lineCodeData;
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentPtr LsPointComponent::_Import(DgnImportContext& importer) const
    {
    LsPointComponentP cloned = new LsPointComponent(*this, false);
    if (cloned->m_strokeComponent.IsValid())
        {
        LsComponentPtr ptr = LsComponent::GetImportedComponent(cloned->m_strokeComponent->GetId(), importer);
        cloned->m_strokeComponent = dynamic_cast<LsStrokePatternComponent*>(ptr.get());
        }

    for (LsSymbolReference& symbref : cloned->m_symbols)
        {
        if (symbref.m_symbol.IsValid())
            {
            LsComponentPtr ptr = LsComponent::GetImportedComponent(symbref.m_symbol->GetId(), importer);
            symbref.m_symbol  = dynamic_cast<LsSymbolComponentP>(ptr.get());
            }
        }

    //  Add to destination file.

    return cloned;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsPointComponent::CreateRscFromDgnDb(V10LinePoint** rscOut, DgnDbR project, LsComponentId componentId)
    {
    *rscOut = NULL;
    uint32_t propertySize;

    if (BE_SQLITE_ROW != project.QueryPropertySize(propertySize, LineStyleProperty::LinePoint(), componentId.GetValue(), 0))
        return BSIERROR;

    V10LinePoint* linePointData = (V10LinePoint*)bentleyAllocator_malloc (propertySize);

    project.QueryProperty(linePointData, propertySize, LineStyleProperty::LinePoint(), componentId.GetValue(), 0);
    BeAssert (propertySize == V10LinePoint::GetBufferSize(linePointData->m_nSymbols));

    *rscOut = linePointData;

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentPtr LsCompoundComponent::_Import(DgnImportContext& importer) const
    {
    LsCompoundComponentP    result = new LsCompoundComponent(*this);

    for (auto& compOffset: result->m_components)
        compOffset.m_subComponent = LsComponent::GetImportedComponent(compOffset.m_subComponent->GetId(), importer);


    //  Save to destination and record ComponentId in clone

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsCompoundComponent::CreateRscFromDgnDb(V10Compound** rscOut, DgnDbR project, LsComponentId componentId)
    {
    *rscOut = NULL;
    uint32_t propertySize;

    if (BE_SQLITE_ROW != project.QueryPropertySize(propertySize, LineStyleProperty::Compound(), componentId.GetValue(), 0))
        return BSIERROR;

    V10Compound* compoundData = (V10Compound*)bentleyAllocator_malloc (propertySize);

    if (BE_SQLITE_ROW != project.QueryProperty(compoundData, propertySize, LineStyleProperty::Compound(), componentId.GetValue(), 0))
        return BSIERROR;

    *rscOut = compoundData;
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentPtr LsSymbolComponent::_Import(DgnImportContext& importer) const
    {
    LsSymbolComponentP result = new LsSymbolComponent(*this);

    importer.RemapGeomPartId(result->m_geomPartId);

    //  Save to destination and record ComponentId in clone

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsSymbolComponent::CreateRscFromDgnDb(V10Symbol** rscOut, DgnDbR project, LsComponentId componentId)
    {
    *rscOut = NULL;
    uint32_t propertySize;

    if (BE_SQLITE_ROW != project.QueryPropertySize(propertySize, LineStyleProperty::PointSym(), componentId.GetValue(), 0))
        return BSIERROR;

    V10Symbol* symbolData = (V10Symbol*)bentleyAllocator_malloc (propertySize);

    if (BE_SQLITE_ROW != project.QueryProperty(symbolData, propertySize, LineStyleProperty::PointSym(), componentId.GetValue(), 0))
        return BSIERROR;

    *rscOut = symbolData;
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    07/2015
//---------------------------------------------------------------------------------------
BentleyStatus LsRasterImageComponent::CreateRscFromDgnDb(V10RasterImage** rscOut, DgnDbR project, LsComponentId componentId)
    {
    *rscOut = NULL;
    uint32_t propertySize;

    if (BE_SQLITE_ROW != project.QueryPropertySize(propertySize, LineStyleProperty::RasterImage(), componentId.GetValue(), 0))
        return BSIERROR;

    V10RasterImage* rasterData = (V10RasterImage*)bentleyAllocator_malloc (propertySize);

    if (BE_SQLITE_ROW != project.QueryProperty(rasterData, propertySize, LineStyleProperty::RasterImage(), componentId.GetValue(), 0))
        return BSIERROR;

    *rscOut = rasterData;
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//--------------+------------------------------------------------------------------------
void LsDefinition::InitializeJsonObject (Json::Value& jsonObj)
    {
    InitializeJsonObject (jsonObj, m_location.GetComponentId(), m_attributes, (uint32_t) m_unitDef); // WIP: m_unitDef double --> UInt32 conversion
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
void LsDefinition::InitializeJsonObject (Json::Value& jsonObj, LsComponentId componentId, uint32_t flags, double unitDefinition)
    {
    jsonObj.clear();

    jsonObj[DGNPROPERTYBLOB_CompId] = componentId.GetValue();
    jsonObj[DGNPROPERTYBLOB_CompType] = static_cast <uint16_t> (componentId.GetType());
    jsonObj[DGNPROPERTYBLOB_Flags] = flags;
    jsonObj[DGNPROPERTYBLOB_UnitDef] = unitDefinition;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
double LsDefinition::GetUnitDef (Json::Value& lsDefinition)
    {
    return lsDefinition[DGNPROPERTYBLOB_UnitDef].asDouble();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
uint32_t LsDefinition::GetAttributes (Json::Value& lsDefinition)
    {
    return lsDefinition[DGNPROPERTYBLOB_Flags].asUInt();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LsComponentId LsDefinition::GetComponentId (Json::Value& lsDefinition)
    {
    LsComponentType typeValue = (LsComponentType)lsDefinition[DGNPROPERTYBLOB_CompType].asUInt();
    if (!LsComponent::IsValidComponentType(typeValue))
        {
        BeAssert(LsComponent::IsValidComponentType(typeValue));
        typeValue = LsComponentType::Unknown;
        }

    uint32_t idValue = lsDefinition[DGNPROPERTYBLOB_CompId].asUInt();
    return LsComponentId(typeValue, idValue);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentId LsComponent::Import(LsComponentId sourceId, DgnImportContext& importer)
    {
    if (!importer.IsBetweenDbs())
        return sourceId;

    LsComponentId result = importer.FindLineStyleComponentId(sourceId);
    if (result.IsValid())
        return result;

    LsComponentPtr srcComponent = importer.GetSourceDb().Styles().LineStyles().GetLsComponent(sourceId);
    if (!srcComponent.IsValid())
        return LsComponentId();

    LsComponentPtr clonedComponent = srcComponent->_Import(importer);
    LsComponentId  clonedId = clonedComponent->GetId();
    importer.AddLineStyleComponentId(sourceId, clonedId);
    return clonedId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentPtr LsComponent::GetImportedComponent(LsComponentId sourceId, DgnImportContext& importer)
    {
    LsComponentId   resultId = Import(sourceId, importer);
    if (!resultId.IsValid())
        return nullptr;

    return importer.GetDestinationDb().Styles().LineStyles().GetLsComponent(resultId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
DgnDbStatus LineStyleElement::_ExtractSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ExtractSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        Utf8String descr = stmt.GetValueText(params.GetSelectIndex(PROPNAME_Descr));
        Utf8String data = stmt.GetValueText(params.GetSelectIndex(PROPNAME_Data));

        //  m_data.Init(baseModelId, source, descr);
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LineStyleElement::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<LineStyleElementCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr == other)
        return;

    m_data = other->m_data;
    m_description = other->m_description;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
DgnDbStatus LineStyleElement::BindParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    BeAssert(0 < m_data.size());
    if (m_data.size() <= 0)
        return DgnDbStatus::BadArg;

    if (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(PROPNAME_Data), m_data.c_str(), IECSqlBinder::MakeCopy::No))
        return DgnDbStatus::BadArg;

    if (m_description.SizeInBytes() > 0 && ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(PROPNAME_Descr), m_description.c_str(), IECSqlBinder::MakeCopy::No))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
DgnDbStatus LineStyleElement::_BindInsertParams(BeSQLite::EC::ECSqlStatement&stmt)
    {
    auto status = T_Super::_BindInsertParams(stmt);
    if (DgnDbStatus::Success == status)
        status = BindParams(stmt);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
DgnDbStatus LineStyleElement::_BindUpdateParams(BeSQLite::EC::ECSqlStatement&stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    if (DgnDbStatus::Success == status)
        status = BindParams(stmt);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
size_t LineStyleElement::QueryCount(DgnDbR db)
    {
    CachedECSqlStatementPtr select = db.GetPreparedECSqlStatement("SELECT count(*) FROM " DGN_SCHEMA(DGN_CLASSNAME_LineStyle));
    if (!select.IsValid())
        return 0;
    
    if (BE_SQLITE_ROW != select->Step())
        return 0;

    return static_cast<size_t>(select->GetValueInt(0));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LineStyleElement::Iterator LineStyleElement::MakeIterator(DgnDbR db)
    {
    Iterator iter;
    iter.Prepare(db, "SELECT ECInstanceId, Code.[Value], Descr, Data FROM " DGN_SCHEMA(DGN_CLASSNAME_LineStyle), 0);

    return iter;
    }

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(LineStyleHandler);

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LineStyleHandler::_GetClassParams(ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(PROPNAME_Data);
    params.Add(PROPNAME_Descr);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
DgnStyleId DgnImportContext::RemapLineStyleId(DgnStyleId sourceId)
    {
    if (!IsBetweenDbs())
        return sourceId;

    DgnStyleId dest = FindLineStyleId(sourceId);
    if (dest.IsValid())
        return dest;

    //  NEEDSWORK_LINESTYLES importers are not finished so don't pass along bad data.
    return DgnStyleId(); // DgnLineStyles::ImportLineStyle(source, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LsComponentId DgnImportContext::FindLineStyleComponentId(LsComponentId sourceId) const
    {
    auto const& iter = m_importedComponents.find(sourceId);
    if (iter == m_importedComponents.end())
        return LsComponentId();

    return LsComponentId(sourceId.GetType(), iter->second);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void DgnImportContext::AddLineStyleComponentId(LsComponentId sourceId, LsComponentId targetId)
    {
    BeAssert(sourceId.GetType() == targetId.GetType());
    BeAssert(m_importedComponents.find(sourceId) == m_importedComponents.end());
    m_importedComponents[sourceId] = targetId.GetValue();
    }

#if defined(NOTNOW)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
DgnStyleId LineStyleElement::ImportLineStyle(DgnStyleId srcStyleId, DgnImportContext& importer)
    {
    //  See if we already have a line style with the same code in the destination Db.
    //  If so, we'll map the source line style to it.  
    LineStyleElementCPtr srcStyle = LineStyleElement::Get(importer.GetSourceDb(), srcStyleId);
    BeAssert(srcStyle.IsValid());
    if (!srcStyle.IsValid())
        {
        BeAssert(false && "invalid source line style ID");
        return DgnStyleId();
        }

    DgnStyleId dstStyleId = QueryId(importer.GetDestinationDb(), srcStyle->GetName());
    if (dstStyleId.IsValid())
        {
        //  *** TBD: Check if the line style definitions match. If not, rename and remap
        importer.AddLineStyleId(srcStyleId, dstStyleId);
        return dstStyleId;
        }

    //  No such line style in the destination Db. Ask the source LineStyleElement to import itself.
    auto importedElem = srcStyle->Import(importer.GetDestinationDb(), importer);
    return importedElem.IsValid()? importedElem->GetElementId() : DgnStyleId();
    }
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE

