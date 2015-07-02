/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsDb.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

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
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsStrokePatternComponent::CreateRscFromDgnDb(V10LineCode** rscOut, DgnDbR project, LsComponentId componentId, bool useRscComponentTypes)
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
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsPointComponent::CreateRscFromDgnDb(V10LinePoint** rscOut, DgnDbR project, LsComponentId componentId, bool useRscComponentTypes)
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
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsCompoundComponent::CreateRscFromDgnDb(V10Compound** rscOut, DgnDbR project, LsComponentId componentId, bool useRscComponentTypes)
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
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus LsSymbolComponent::CreateRscFromDgnDb(V10Symbol** rscOut, DgnDbR project, LsComponentId componentId, bool useRscComponentTypes)
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
// @bsimethod                                                   John.Gooding    11/2012
//--------------+------------------------------------------------------------------------
void LsDefinition::InitializeJsonObject (Json::Value& jsonObj)
    {
    InitializeJsonObject (jsonObj, m_location.GetComponentId(), m_location.GetComponentType(),
                          m_attributes, (uint32_t) m_unitDef); // WIP: m_unitDef double --> UInt32 conversion
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
void LsDefinition::InitializeJsonObject (Json::Value& jsonObj, LsComponentId componentId, LsComponentType componentType, uint32_t flags, double unitDefinition)
    {
    jsonObj.clear();

    jsonObj[DGNPROPERTYBLOB_CompId] = componentId.GetValue();
    jsonObj[DGNPROPERTYBLOB_CompType] = static_cast <uint16_t> (componentType);  //  defined(NOTNOW) (uint32_t)remapRscTypeToElmType(componentType);
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
LsComponentType LsDefinition::GetComponentType (Json::Value& lsDefinition)
    {
    LsComponentType retval = (LsComponentType)lsDefinition[DGNPROPERTYBLOB_CompType].asUInt();
    if (!LsComponent::IsValidComponentType(retval))
        {
        BeAssert(LsComponent::IsValidComponentType(retval));
        retval = LsComponentType::Unknown;
        }

    return retval;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LsComponentId LsDefinition::GetComponentId (Json::Value& lsDefinition)
    {
    return LsComponentId(lsDefinition[DGNPROPERTYBLOB_CompId].asUInt());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2013
//---------------------------------------------------------------------------------------
void LsComponent::QueryComponentIds(bset<LsComponentTypeAndId>& ids, DgnDbCR project, LsComponentType lsType)
    {
    //  No default constructor so we have to initialize it.
    LineStyleProperty::ComponentProperty spec = LineStyleProperty::Compound();

    switch(lsType)
        {
        case LsComponentType::Compound:
            break;
        case LsComponentType::LineCode:
            spec = LineStyleProperty::LineCode();
            break;
        case LsComponentType::LinePoint:
            spec = LineStyleProperty::LinePoint();
            break;
        case LsComponentType::PointSymbol:
            spec = LineStyleProperty::PointSym();
            break;

        default:
            BeAssert(false && "bad lsType argument to QueryComponentIds");
            return;
        }

    Statement stmt;
    stmt.Prepare (project, SqlPrintfString("SELECT Id FROM " BEDB_TABLE_Property " WHERE Namespace=? AND Name=?"));

    stmt.BindText(1, spec.GetNamespace(), Statement::MakeCopy::No);
    stmt.BindText(2, spec.GetName(), Statement::MakeCopy::No);
    while (stmt.Step() == BE_SQLITE_ROW)
        {
        LsComponentTypeAndId     componentId;
        componentId.m_id = stmt.GetValueInt(0);
        componentId.m_type = (uint32_t)lsType;
        ids.insert(componentId);
        };
    }
