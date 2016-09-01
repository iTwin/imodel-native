/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECInstanceAdapterHelper.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECInstanceAdapterHelper.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::unique_ptr<ECValueBindingInfo> ECValueBindingInfoFactory::CreateBindingInfo(ECN::ECEnablerCR enabler, ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString, int ecsqlParameterIndex)
    {
    if (ecProperty.GetIsStruct())
        {
        auto structProp = ecProperty.GetAsStructProperty();
        BeAssert(structProp != nullptr);
        ECClassCR structType = structProp->GetType();
        return StructECValueBindingInfo::Create(enabler, structType, propertyAccessString, ecsqlParameterIndex);
        }

    //propIndex only needed for prims and arrays
    uint32_t propIndex = 0;
    auto ecStat = enabler.GetPropertyIndex(propIndex, propertyAccessString);
    if (ecStat != ECObjectsStatus::Success)
        return nullptr;

    if (ecProperty.GetIsPrimitive() || ecProperty.GetIsNavigation())
        return PrimitiveECValueBindingInfo::Create(propIndex, ecsqlParameterIndex);
    else if (ecProperty.GetIsArray())
        return ArrayECValueBindingInfo::Create(ecProperty, propIndex, ecsqlParameterIndex);

    BeAssert(false && "Unhandled ECProperty type. Adjust the code for this new ECProperty type");
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::unique_ptr<ECValueBindingInfo> ECValueBindingInfoFactory::CreateSystemBindingInfo(ECValueBindingInfo::SystemPropertyKind kind, int ecsqlParameterIndex)
    {
    return ECSqlSystemPropertyBindingInfo::Create(kind, ecsqlParameterIndex);
    }



//***********************************************************************************************
// ECValueBindingInfo
//***********************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
bool ECValueBindingInfo::HasECSqlParameterIndex() const
    {
    return m_ecsqlParameterIndex > UNSET_INDEX;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
int ECValueBindingInfo::GetECSqlParameterIndex() const
    {
    BeAssert(HasECSqlParameterIndex());
    return m_ecsqlParameterIndex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
ECValueBindingInfo::Type ECValueBindingInfo::GetType() const
    {
    return m_type;
    }


//***********************************************************************************************
// ECSqlSystemPropertyBindingInfo
//***********************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::unique_ptr<ECSqlSystemPropertyBindingInfo> ECSqlSystemPropertyBindingInfo::Create(SystemPropertyKind kind, int ecsqlParameterIndex)
    {
    return std::unique_ptr<ECSqlSystemPropertyBindingInfo>(new ECSqlSystemPropertyBindingInfo(kind, ecsqlParameterIndex));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSystemPropertyBindingInfo::ECSqlSystemPropertyBindingInfo(SystemPropertyKind kind, int ecsqlParameterIndex)
    : ECValueBindingInfo(Type::ECSqlSystemProperty, ecsqlParameterIndex), m_kind(kind)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
ECValueBindingInfo::SystemPropertyKind ECSqlSystemPropertyBindingInfo::GetKind() const
    {
    return m_kind;
    }


//***********************************************************************************************
// PrimitiveECValueBinding
//***********************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::unique_ptr<PrimitiveECValueBindingInfo> PrimitiveECValueBindingInfo::Create(uint32_t propertyIndex, int ecsqlParameterIndex)
    {
    return std::unique_ptr<PrimitiveECValueBindingInfo>(new PrimitiveECValueBindingInfo(propertyIndex, ecsqlParameterIndex));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
PrimitiveECValueBindingInfo::PrimitiveECValueBindingInfo(uint32_t propertyIndex, int ecsqlParameterIndex)
    : ECValueBindingInfo(Type::Primitive, ecsqlParameterIndex), m_propertyIndex(propertyIndex)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t PrimitiveECValueBindingInfo::GetPropertyIndex() const
    {
    return m_propertyIndex;
    }

//***********************************************************************************************
// StructECValueBinding
//***********************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::unique_ptr<StructECValueBindingInfo> StructECValueBindingInfo::Create(ECN::ECEnablerCR parentEnabler, ECN::ECClassCR structType, Utf8CP parentPropertyAccessString, int ecsqlParameterIndex)
    {
    return std::unique_ptr<StructECValueBindingInfo>(new StructECValueBindingInfo(parentEnabler, structType, parentPropertyAccessString, ecsqlParameterIndex));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::unique_ptr<StructECValueBindingInfo> StructECValueBindingInfo::CreateForNestedStruct(ECN::ECClassCR structType)
    {
    return Create(*structType.GetDefaultStandaloneEnabler(), structType, nullptr, UNSET_INDEX);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
StructECValueBindingInfo::StructECValueBindingInfo(ECN::ECEnablerCR parentEnabler, ECN::ECClassCR structType, Utf8CP parentPropertyAccessString, int ecsqlParameterIndex)
    : ECValueBindingInfo(Type::Struct, ecsqlParameterIndex)
    {
    for (auto memberProp : structType.GetProperties(true))
        {
        Utf8String memberAccessString;
        if (!Utf8String::IsNullOrEmpty(parentPropertyAccessString))
            {
            memberAccessString = parentPropertyAccessString;
            memberAccessString.append(".");
            }

        memberAccessString.append(memberProp->GetName());

        auto memberBinding = ECValueBindingInfoFactory::CreateBindingInfo(parentEnabler, *memberProp, memberAccessString.c_str(), UNSET_INDEX);
        BeAssert(memberBinding != nullptr);
        m_memberBindingInfos[memberProp->GetId()] = std::move(memberBinding);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
std::map<ECN::ECPropertyId, std::unique_ptr<ECValueBindingInfo>> const& StructECValueBindingInfo::GetMemberBindingInfos() const
    {
    return m_memberBindingInfos;
    }


//***********************************************************************************************
// ArrayECValueBinding
//***********************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::unique_ptr<ArrayECValueBindingInfo> ArrayECValueBindingInfo::Create(ECN::ECPropertyCR prop, uint32_t arrayPropIndex, int ecsqlParameterIndex)
    {
    return std::unique_ptr<ArrayECValueBindingInfo>(new ArrayECValueBindingInfo(prop, arrayPropIndex, ecsqlParameterIndex));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
ArrayECValueBindingInfo::ArrayECValueBindingInfo(ECN::ECPropertyCR prop, uint32_t arrayPropIndex, int ecsqlParameterIndex)
    : ECValueBindingInfo(Type::Array, ecsqlParameterIndex), m_arrayPropIndex(arrayPropIndex)
    {
    auto structArrayProp = prop.GetAsStructArrayProperty();
    //if this is a struct array, generate bindings for the struct element type.
    //This is not necessary for prim arrays as they don't have any extra information needed
    //to read out the values
    if (nullptr != structArrayProp)
        {
        auto structType = structArrayProp->GetStructElementType();
        m_structArrayElementBindingInfo = StructECValueBindingInfo::CreateForNestedStruct(*structType);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
uint32_t ArrayECValueBindingInfo::GetArrayPropertyIndex() const
    {
    return m_arrayPropIndex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
bool ArrayECValueBindingInfo::IsStructArray() const
    {
    return m_structArrayElementBindingInfo != nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
StructECValueBindingInfo const& ArrayECValueBindingInfo::GetStructArrayElementBindingInfo() const
    {
    BeAssert(IsStructArray());
    return *m_structArrayElementBindingInfo;
    }


//***********************************************************************************************
// ECValueBindingCollection
//***********************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECValueBindingInfoCollection::AddBindingInfo(ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty, int ecsqlParameterIndex)
    {
    auto binding = ECValueBindingInfoFactory::CreateBindingInfo(*ecClass.GetDefaultStandaloneEnabler(), ecProperty, ecProperty.GetName().c_str(), ecsqlParameterIndex);
    if (binding == nullptr)
        return ERROR;

    m_bindingInfos.push_back(std::move(binding));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/16
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECValueBindingInfoCollection::AddBindingInfo(ECN::ECEnablerCR ecEnabler, ECN::ECPropertyCR ecProperty, Utf8CP accessString, int ecsqlParameterIndex)
    {
    auto binding = ECValueBindingInfoFactory::CreateBindingInfo(ecEnabler, ecProperty, accessString, ecsqlParameterIndex);
    if (binding == nullptr)
        return ERROR;

    m_bindingInfos.push_back(std::move(binding));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlSystemPropertyBindingInfo* ECValueBindingInfoCollection::AddBindingInfo(ECValueBindingInfo::SystemPropertyKind kind, int ecsqlParameterIndex)
    {
    auto binding = ECValueBindingInfoFactory::CreateSystemBindingInfo(kind, ecsqlParameterIndex);
    if (binding == nullptr)
        return nullptr;

    auto bindingP = (ECSqlSystemPropertyBindingInfo*) binding.get();
    m_bindingInfos.push_back(std::move(binding));
    return bindingP;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
ECValueBindingInfoCollection::const_iterator ECValueBindingInfoCollection::begin() const
    {
    return const_iterator(m_bindingInfos.begin());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
ECValueBindingInfoCollection::const_iterator ECValueBindingInfoCollection::end() const
    {
    return const_iterator(m_bindingInfos.end());
    }


//***********************************************************************************************
// ECInstanceAdapterHelper
//***********************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECInstanceAdapterHelper::BindValue(IECSqlBinder& binder, ECInstanceInfo const& instance, ECValueBindingInfo const& valueBindingInfo)
    {
    switch (valueBindingInfo.GetType())
        {
            case ECValueBindingInfo::Type::Primitive:
                return BindPrimitiveValue(binder, instance, static_cast<PrimitiveECValueBindingInfo const&> (valueBindingInfo));
            case ECValueBindingInfo::Type::Struct:
                return BindStructValue(binder, instance, static_cast<StructECValueBindingInfo const&> (valueBindingInfo));
            case ECValueBindingInfo::Type::Array:
                return BindArrayValue(binder, instance, static_cast<ArrayECValueBindingInfo const&> (valueBindingInfo));

            case ECValueBindingInfo::Type::ECSqlSystemProperty:
                return BindECSqlSystemPropertyValue(binder, instance, static_cast<ECSqlSystemPropertyBindingInfo const&> (valueBindingInfo));

            default:
                BeAssert(false && "Unhandled value of enum ECValueBinding::Type");
                return ERROR;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECInstanceAdapterHelper::BindPrimitiveValue(IECSqlBinder& binder, ECInstanceInfo const& instanceInfo, PrimitiveECValueBindingInfo const& valueBindingInfo)
    {
    ECValue value;
    //avoid to copy strings/blobs from ECInstance into ECValue and from there into ECSqlStatement. As lifetime of ECInstance
    //string/blob owner is longer than ECInstance adapter operation takes we do not need to make copies.
    value.SetAllowsPointersIntoInstanceMemory(true);
    auto ecStat = instanceInfo.GetInstance().GetValue(value, valueBindingInfo.GetPropertyIndex());
    if (ecStat != ECObjectsStatus::Success)
        return ERROR;

    return BindPrimitiveValue(binder, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                   02/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECInstanceAdapterHelper::BindPrimitiveValue(IECSqlBinder& binder, ECN::ECValueCR value)
    {
    BeAssert(value.IsPrimitive());

    if (value.IsNull())
        return SUCCESS;

    ECSqlStatus stat = ECSqlStatus::Success;
    switch (value.GetPrimitiveType())
        {
            case ECN::PRIMITIVETYPE_Binary:
            {
            size_t blobSize;
            Byte const* const blob = value.GetBinary(blobSize);

            //if blob owner is IECInstance which will be alive until ECInstance adapter is done executing,
            //we don't need to copy
            const auto makeCopy = DetermineMakeCopy(value);
            stat = binder.BindBinary(static_cast<const void* const> (blob), (int) blobSize, makeCopy);
            break;
            }
            case ECN::PRIMITIVETYPE_Boolean:
                stat = binder.BindBoolean(value.GetBoolean());
                break;

            case ECN::PRIMITIVETYPE_DateTime:
            {
            bool hasMetadata = false;
            DateTime::Info metadata;
            const int64_t ceTicks = value.GetDateTimeTicks(hasMetadata, metadata);
            const uint64_t jdHns = DateTime::CommonEraTicksToJulianDay(ceTicks);

            DateTime::Info const* actualMetadata = hasMetadata ? &metadata : nullptr;
            stat = binder.BindDateTime(jdHns, actualMetadata);
            break;

            }

            case ECN::PRIMITIVETYPE_Double:
                stat = binder.BindDouble(value.GetDouble());
                break;

            case ECN::PRIMITIVETYPE_IGeometry:
            {
            //call ECValue::GetBinary to get the Geometry blob as stored in the ECInstance. No need to 
            //deserialize it to IGeometry and then back to a blob when inserting in database
            size_t blobSize;
            auto blob = value.GetBinary(blobSize);

            //if blob owner is IECInstance which will be alive until ECInstance adapter is done executing,
            //we don't need to copy
            const auto makeCopy = DetermineMakeCopy(value);
            stat = binder.BindGeometryBlob(static_cast<const void* const> (blob), (int) blobSize, makeCopy);
            break;
            }

            case ECN::PRIMITIVETYPE_Integer:
                stat = binder.BindInt(value.GetInteger());
                break;

            case ECN::PRIMITIVETYPE_Long:
                stat = binder.BindInt64(value.GetLong());
                break;

            case ECN::PRIMITIVETYPE_Point2D:
                stat = binder.BindPoint2D(value.GetPoint2D());
                break;

            case ECN::PRIMITIVETYPE_Point3D:
                stat = binder.BindPoint3D(value.GetPoint3D());
                break;

            case ECN::PRIMITIVETYPE_String:
            {
            //if string owner is IECInstance which will be alive until ECInstance adapter is done executing,
            //we don't need to copy
            //and we need to determine 'makecopy' before calling value.GetUtf8CP as the latter would do a 
            //conversion to Utf8 where the converted string is no longer owned by the IECInstance, but by the ECValue
            //which will go out of scope before we execute the ECSQL statement.

            //This is still a bit risky, as it relies that ECValue::GetUtf8CP has not been called before.
            //Once ECValue has a method to check ownership, we can improve this.
            const auto makeCopy = DetermineMakeCopy(value);
            stat = binder.BindText(value.GetUtf8CP(), makeCopy);
            break;
            }

            default:
            {
            BeAssert(false && "ECN::PrimitiveType which is not supported by ECSqlBindECValueHelper. Please update ECSqlBindECValueHelper accordingly.");
            return ERROR;
            }
        }

    return stat.IsSuccess() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECInstanceAdapterHelper::BindStructValue(IECSqlBinder& binder, ECInstanceInfo const& instance, StructECValueBindingInfo const& valueBindingInfo)
    {
    IECSqlStructBinder& structBinder = binder.BindStruct();
    for (auto const& kvPair : valueBindingInfo.GetMemberBindingInfos())
        {
        ECValueBindingInfo const& memberBinding = *kvPair.second;

        auto stat = BindValue(structBinder.GetMember(kvPair.first), instance, memberBinding);
        if (stat != SUCCESS)
            return stat;

        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECInstanceAdapterHelper::BindArrayValue(IECSqlBinder& binder, ECInstanceInfo const& instanceInfo, ArrayECValueBindingInfo const& valueBindingInfo)
    {
    IECInstanceCR instance = instanceInfo.GetInstance();
    const uint32_t arrayPropIndex = valueBindingInfo.GetArrayPropertyIndex();
    ECValue arrayValue;
    //avoid to copy strings/blobs from ECInstance into ECValue and from there into ECSqlStatement. As lifetime of ECInstance
    //string/blob owner is longer than ECInstance adapter operation takes we do not need to make copies.
    arrayValue.SetAllowsPointersIntoInstanceMemory(true);
    auto ecStat = instance.GetValue(arrayValue, arrayPropIndex);
    if (ecStat != ECObjectsStatus::Success)
        return ERROR;

    BeAssert(arrayValue.IsArray());
    const ArrayInfo arrayInfo = arrayValue.GetArrayInfo();
    const uint32_t arrayLength = arrayInfo.GetCount();
    IECSqlArrayBinder& arrayBinder = binder.BindArray(arrayLength);

    for (uint32_t i = 0; i < arrayLength; i++)
        {
        ECValue elementValue;
        //avoid to copy strings/blobs from ECInstance into ECValue and from there into ECSqlStatement. As lifetime of ECInstance
        //string/blob owner is longer than ECInstance adapter operation takes we do not need to make copies.
        elementValue.SetAllowsPointersIntoInstanceMemory(true);
        ecStat = instance.GetValue(elementValue, arrayPropIndex, i);
        if (ecStat != ECObjectsStatus::Success)
            return ERROR;

        auto& arrayElementBinder = arrayBinder.AddArrayElement();

        BentleyStatus stat = SUCCESS;
        if (!valueBindingInfo.IsStructArray())
            {
            BeAssert(elementValue.IsPrimitive());
            stat = BindPrimitiveValue(arrayElementBinder, elementValue);
            }
        else
            {
            BeAssert(elementValue.IsStruct());
            BeAssert(arrayInfo.IsStructArray());
            auto structInstance = elementValue.GetStruct();
            stat = BindStructValue(arrayElementBinder, ECInstanceInfo(*structInstance), valueBindingInfo.GetStructArrayElementBindingInfo());
            }

        if (stat != SUCCESS)
            return stat;
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECInstanceAdapterHelper::BindECSqlSystemPropertyValue(IECSqlBinder& binder, ECInstanceInfo const& instanceInfo, ECSqlSystemPropertyBindingInfo const& valueBindingInfo)
    {
    ECSqlStatus stat;

    const auto systemPropertyKind = valueBindingInfo.GetKind();
    if (systemPropertyKind == ECValueBindingInfo::SystemPropertyKind::ECInstanceId)
        {
        //bind instance's ecinstanceid
        if (instanceInfo.HasInstanceId())
            stat = binder.BindId(instanceInfo.GetInstanceId());
        else
            stat = binder.BindNull(); //-> ECDb will auto-generate the id
        }
    else
        {
        auto relInstance = dynamic_cast<IECRelationshipInstanceCP> (&instanceInfo.GetInstance());
        if (relInstance == nullptr)
            {
            BeAssert(false && "Programmer error. Instance passed to ECInstanceAdapterHelper::BindECSqlSystemPropertyValue is expected to be an IECRelationshipInstance.");
            return ERROR;
            }

        auto endInstance = systemPropertyKind == ECValueBindingInfo::SystemPropertyKind::SourceECInstanceId || systemPropertyKind == ECValueBindingInfo::SystemPropertyKind::SourceECClassId ?
            relInstance->GetSource() : relInstance->GetTarget();
        if (endInstance.IsNull())
            {
            if (systemPropertyKind == ECValueBindingInfo::SystemPropertyKind::SourceECClassId)
                {
                BeAssert(false && "Source Instance of the relationship is null");
                }
            else
                {
                BeAssert(false && "Target Instance of the relationship is null ");
                }
            return ERROR;
            }
        if (systemPropertyKind == ECValueBindingInfo::SystemPropertyKind::SourceECInstanceId || systemPropertyKind == ECValueBindingInfo::SystemPropertyKind::TargetECInstanceId)
            {
            //bind constraint ecinstanceid
            ECInstanceId endInstanceId;
            if (SUCCESS != ECInstanceId::FromString(endInstanceId, endInstance->GetInstanceId().c_str()))
                {
                BeAssert(false && "Programmer error. Could not convert IECInstance::GetInstanceId to an ECInstanceId object.");
                return ERROR;
                }

            stat = binder.BindId(endInstanceId);
            }
        else
            {
            //Bind constraint class id
            BeAssert(systemPropertyKind == ECValueBindingInfo::SystemPropertyKind::SourceECClassId || systemPropertyKind == ECValueBindingInfo::SystemPropertyKind::TargetECClassId);
            stat = binder.BindId(endInstance->GetClass().GetId());
            }
        }

    return stat.IsSuccess() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECInstanceAdapterHelper::IsOrContainsCalculatedProperty(ECN::ECPropertyCR prop)
    {
    ECClassCP structType = nullptr;
    if (prop.GetIsPrimitive())
        return prop.IsCalculated();

    if (prop.GetIsArray())
        {
        ArrayECPropertyCP arrayProp = prop.GetAsArrayProperty();
        if (arrayProp->GetKind() == ARRAYKIND_Primitive)
            return arrayProp->IsCalculated();

        StructArrayECProperty const* structArrayProp = prop.GetAsStructArrayProperty();
        if (nullptr == structArrayProp)
            return false;

        structType = structArrayProp->GetStructElementType();
        }
    else if (prop.GetIsStruct())
        structType = &prop.GetAsStructProperty()->GetType();

    if (structType != nullptr)
        {
        for (ECPropertyCP structMemberProp : structType->GetProperties(true))
            {
            if (IsOrContainsCalculatedProperty(*structMemberProp))
                return true;
            }
        }

    return false;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   01/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECInstanceAdapterHelper::TryGetCurrentTimeStampProperty(ECN::ECPropertyCP& currentTimeStampProp, ECN::ECClassCR ecClass)
    {
    currentTimeStampProp = nullptr;
    auto ca = ecClass.GetCustomAttribute("ClassHasCurrentTimeStampProperty");
    if (ca == nullptr)
        return false;

    ECValue v;
    ca->GetValue(v, "PropertyName");

    if (v.IsNull())
        return false;

    if (v.IsUtf8())
        currentTimeStampProp = ecClass.GetPropertyP(v.GetUtf8CP(), true);
    else
        currentTimeStampProp = ecClass.GetPropertyP(v.GetWCharCP(), true);

    return currentTimeStampProp != nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   01/15
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECInstanceAdapterHelper::HasReadonlyPropertiesAreUpdatableOption(ECDbCR ecdb, ECClassCR ecClass, Utf8CP ecsqlOptions)
    {
    if (Utf8String::IsNullOrEmpty(ecsqlOptions))
        return false;

    Utf8String dummyECSql;
    dummyECSql.Sprintf("SELECT NULL FROM ONLY %s ECSQLOPTIONS %s", ecClass.GetECSqlName().c_str(), ecsqlOptions);
    
    ECSqlParser parser;
    std::unique_ptr<Exp> parseTree = parser.Parse(ecdb, dummyECSql.c_str());
    if (parseTree == nullptr)
        return false;

    SingleSelectStatementExp const* selectStmtExp = nullptr;
    if (parseTree->GetType() == Exp::Type::Select)
        {
        SelectStatementExp const* stmtExp = static_cast<SelectStatementExp const*>(parseTree.get());
        BeAssert(stmtExp->GetChildrenCount() != 0);
        selectStmtExp = &stmtExp->GetCurrent();
        }
    else if (parseTree->GetType() == Exp::Type::SingleSelect)
        selectStmtExp = static_cast<SingleSelectStatementExp const*>(parseTree.get());

    if (selectStmtExp == nullptr)
        {
        BeAssert(selectStmtExp != nullptr);
        return false;
        }

    OptionsExp const* optionsExp = selectStmtExp->GetOptions();
    return optionsExp != nullptr && optionsExp->HasOption(OptionsExp::READONLYPROPERTIESAREUPDATABLE_OPTION);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECInstanceAdapterHelper::SetECInstanceId(ECN::IECInstanceR instance, ECInstanceId ecInstanceId)
    {
    if (!ecInstanceId.IsValid())
        return ERROR;

    Utf8Char instanceIdStr[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    ecInstanceId.ToString(instanceIdStr);
    return instance.SetInstanceId(instanceIdStr) == ECObjectsStatus::Success ? SUCCESS : ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECN::IECInstancePtr ECInstanceAdapterHelper::CreateECInstance(ECN::ECClassCR ecclass)
    {
    ECRelationshipClassCP relClass = ecclass.GetRelationshipClassCP();
    if (relClass == nullptr)
        return ecclass.GetDefaultStandaloneEnabler()->CreateInstance();

    return StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*relClass)->CreateRelationshipInstance();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
void ECInstanceAdapterHelper::LogFailure(Utf8CP operationName, ECN::IECInstanceCR instance, Utf8CP errorMessage)
    {
    Utf8String displayLabel;
    instance.GetDisplayLabel(displayLabel);
    LOG.errorv("Failed to %s ECInstance '%s'. %s", operationName, displayLabel.c_str(), errorMessage);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECInstanceAdapterHelper::Equals(ECN::ECClassCR lhs, ECN::ECClassCR rhs)
    {
    if (lhs.HasId() && rhs.HasId())
        {
        return lhs.GetId() == rhs.GetId();
        }

    return strcmp(lhs.GetFullName(), rhs.GetFullName()) == 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
IECSqlBinder::MakeCopy ECInstanceAdapterHelper::DetermineMakeCopy(ECN::ECValueCR ecValue)
    {
    //if original string is not UTF-8, it will be converted to UTF-8 and the new string stored in ECValue.
    //In that case ECSqlStatement needs to keep its own copy of the string as the ECValue will go out
    //of scope before the statement is executed.
    if (ecValue.IsString() && !ecValue.IsUtf8())
        return IECSqlBinder::MakeCopy::Yes;

    return ecValue.AllowsPointersIntoInstanceMemory() ? IECSqlBinder::MakeCopy::No : IECSqlBinder::MakeCopy::Yes;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE