/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECInstanceAdapterHelper.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::unique_ptr<ECValueBindingInfo> ECValueBindingInfoFactory::CreateBindingInfo(ECN::ECEnablerCR enabler, ECN::ECPropertyCR ecProperty, Utf8StringCR propertyAccessString, int ecsqlParameterIndex)
    {
    if (ecProperty.GetIsStruct())
        {
        StructECPropertyCP structProp = ecProperty.GetAsStructProperty();
        BeAssert(structProp != nullptr);
        ECStructClassCR structType = structProp->GetType();
        return StructECValueBindingInfo::Create(enabler, structType, &propertyAccessString, ecsqlParameterIndex);
        }

    uint32_t propIndex = 0;
    if (ECObjectsStatus::Success != enabler.GetPropertyIndex(propIndex, propertyAccessString.c_str()))
        return nullptr;

    if (ecProperty.GetIsPrimitive())
        return PrimitiveECValueBindingInfo::Create(ecsqlParameterIndex, propIndex);

    if (ecProperty.GetIsArray())
        return ArrayECValueBindingInfo::Create(ecProperty, ecsqlParameterIndex, propIndex);

    if (ecProperty.GetIsNavigation())
        return NavigationECValueBindingInfo::Create(ecsqlParameterIndex, propIndex);

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


//***********************************************************************************************
// PrimitiveECValueBinding
//***********************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::unique_ptr<PrimitiveECValueBindingInfo> PrimitiveECValueBindingInfo::Create(int ecsqlParameterIndex, uint32_t propertyIndex)
    {
    return std::unique_ptr<PrimitiveECValueBindingInfo>(new PrimitiveECValueBindingInfo(ecsqlParameterIndex, propertyIndex));
    }


//***********************************************************************************************
// StructECValueBinding
//***********************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::unique_ptr<StructECValueBindingInfo> StructECValueBindingInfo::Create(ECN::ECEnablerCR parentEnabler, ECN::ECStructClassCR structType, Utf8StringCP parentPropertyAccessString, int ecsqlParameterIndex)
    {
    return std::unique_ptr<StructECValueBindingInfo>(new StructECValueBindingInfo(parentEnabler, structType, parentPropertyAccessString, ecsqlParameterIndex));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::unique_ptr<StructECValueBindingInfo> StructECValueBindingInfo::CreateForNestedStruct(ECN::ECStructClassCR structType)
    {
    return Create(*structType.GetDefaultStandaloneEnabler(), structType, nullptr, UNSET_PARAMETERINDEX);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
StructECValueBindingInfo::StructECValueBindingInfo(ECN::ECEnablerCR parentEnabler, ECN::ECStructClassCR structType, Utf8StringCP parentPropertyAccessString, int ecsqlParameterIndex)
    : ECValueBindingInfo(Type::Struct, ecsqlParameterIndex)
    {
    for (ECPropertyCP memberProp : structType.GetProperties(true))
        {
        Utf8String memberAccessString;
        if (parentPropertyAccessString != nullptr && !parentPropertyAccessString->empty())
            memberAccessString.append(*parentPropertyAccessString).append(".");

        memberAccessString.append(memberProp->GetName());

        std::unique_ptr<ECValueBindingInfo> memberBinding = ECValueBindingInfoFactory::CreateBindingInfo(parentEnabler, *memberProp, memberAccessString, UNSET_PARAMETERINDEX);
        BeAssert(memberBinding != nullptr);
        m_memberBindingInfos[memberProp->GetId()] = std::move(memberBinding);
        }
    }



//***********************************************************************************************
// ArrayECValueBinding
//***********************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
std::unique_ptr<ArrayECValueBindingInfo> ArrayECValueBindingInfo::Create(ECN::ECPropertyCR prop, int ecsqlParameterIndex, uint32_t arrayPropIndex)
    {
    return std::unique_ptr<ArrayECValueBindingInfo>(new ArrayECValueBindingInfo(prop, ecsqlParameterIndex, arrayPropIndex));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
ArrayECValueBindingInfo::ArrayECValueBindingInfo(ECN::ECPropertyCR prop, int ecsqlParameterIndex, uint32_t arrayPropIndex)
    : ECValueBindingInfo(Type::Array, ecsqlParameterIndex, arrayPropIndex)
    {
    StructArrayECPropertyCP structArrayProp = prop.GetAsStructArrayProperty();
    //if this is a struct array, generate bindings for the struct element type.
    //This is not necessary for prim arrays as they don't have any extra information needed
    //to read out the values
    if (nullptr != structArrayProp)
        {
        ECStructClassCR structType = structArrayProp->GetStructElementType();
        m_structArrayElementBindingInfo = StructECValueBindingInfo::CreateForNestedStruct(structType);
        }
    }


//***********************************************************************************************
// ECValueBindingCollection
//***********************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECValueBindingInfoCollection::AddBindingInfo(ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty, int ecsqlParameterIndex)
    {
    return AddBindingInfo(*ecClass.GetDefaultStandaloneEnabler(), ecProperty, ecProperty.GetName(), ecsqlParameterIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   03/16
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECValueBindingInfoCollection::AddBindingInfo(ECN::ECEnablerCR ecEnabler, ECN::ECPropertyCR ecProperty, Utf8StringCR accessString, int ecsqlParameterIndex)
    {
    std::unique_ptr<ECValueBindingInfo> binding = ECValueBindingInfoFactory::CreateBindingInfo(ecEnabler, ecProperty, accessString, ecsqlParameterIndex);
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
    std::unique_ptr<ECValueBindingInfo> binding = ECValueBindingInfoFactory::CreateSystemBindingInfo(kind, ecsqlParameterIndex);
    if (binding == nullptr)
        return nullptr;

    ECSqlSystemPropertyBindingInfo* bindingP = static_cast<ECSqlSystemPropertyBindingInfo*>(binding.get());
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
            case ECValueBindingInfo::Type::Navigation:
                return BindNavigationValue(binder, instance, static_cast<NavigationECValueBindingInfo const&> (valueBindingInfo));
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
    if (!instanceInfo.HasInstance()) // bind null in that case
        return SUCCESS;

    //avoid to copy strings/blobs from ECInstance into ECValue and from there into ECSqlStatement. As lifetime of ECInstance
    //string/blob owner is longer than ECInstance adapter operation takes we do not need to make copies.
    value.SetAllowsPointersIntoInstanceMemory(instanceInfo.AllowPointersIntoInstanceMemory());
    if (ECObjectsStatus::Success != instanceInfo.GetInstance().GetValue(value, valueBindingInfo.GetPropertyIndex()))
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
            stat = binder.BindBlob(static_cast<const void* const> (blob), (int) blobSize, DetermineMakeCopy(value));
            break;
            }
            case ECN::PRIMITIVETYPE_Boolean:
                stat = binder.BindBoolean(value.GetBoolean());
                break;

            case ECN::PRIMITIVETYPE_DateTime:
            {
            DateTime::Info metadata;
            const int64_t ceTicks = value.GetDateTimeTicks(metadata);
            const uint64_t jdMsec = DateTime::CommonEraMillisecondsToJulianDay(ceTicks / 10000);
            stat = binder.BindDateTime(jdMsec, metadata);
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
            stat = binder.BindBlob(static_cast<const void* const> (blob), (int) blobSize, DetermineMakeCopy(value));
            break;
            }

            case ECN::PRIMITIVETYPE_Integer:
                stat = binder.BindInt(value.GetInteger());
                break;

            case ECN::PRIMITIVETYPE_Long:
                stat = binder.BindInt64(value.GetLong());
                break;

            case ECN::PRIMITIVETYPE_Point2d:
                stat = binder.BindPoint2d(value.GetPoint2d());
                break;

            case ECN::PRIMITIVETYPE_Point3d:
                stat = binder.BindPoint3d(value.GetPoint3d());
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
            stat = binder.BindText(value.GetUtf8CP(), DetermineMakeCopy(value));
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
    if (!instance.HasInstance()) // bind null in that case
        return SUCCESS;

    for (auto const& kvPair : valueBindingInfo.GetMemberBindingInfos())
        {
        ECValueBindingInfo const& memberBinding = *kvPair.second;

        //don't attempt to call ECSqlStatement::BindNull on the member as it will cause a null member to be created in the JSON
        //for struct arrays. So if the struct member is unset in the IECInstance, skip the call to BindValue.
        if (IsPropertyValueNull(instance.GetInstance(), memberBinding))
            continue;
        
        if (SUCCESS != BindValue(binder[kvPair.first], instance, memberBinding))
            return ERROR;
        }

    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   06/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECInstanceAdapterHelper::BindArrayValue(IECSqlBinder& binder, ECInstanceInfo const& instanceInfo, ArrayECValueBindingInfo const& valueBindingInfo)
    {
    if (!instanceInfo.HasInstance()) // bind null in that case
        return SUCCESS;

    IECInstanceCR instance = instanceInfo.GetInstance();
    const uint32_t arrayPropIndex = valueBindingInfo.GetPropertyIndex();
    ECValue arrayValue;
    //avoid to copy strings/blobs from ECInstance into ECValue and from there into ECSqlStatement. As lifetime of ECInstance
    //string/blob owner is longer than ECInstance adapter operation takes we do not need to make copies.
    arrayValue.SetAllowsPointersIntoInstanceMemory(true);
    if (ECObjectsStatus::Success != instance.GetValue(arrayValue, arrayPropIndex))
        return ERROR;

    BeAssert(arrayValue.IsArray());
    const ArrayInfo arrayInfo = arrayValue.GetArrayInfo();
    const uint32_t arrayLength = arrayInfo.GetCount();

    for (uint32_t i = 0; i < arrayLength; i++)
        {
        IECSqlBinder& arrayElementBinder = binder.AddArrayElement();

        ECValue elementValue;
        //avoid to copy strings/blobs from ECInstance into ECValue and from there into ECSqlStatement. As lifetime of ECInstance
        //string/blob owner is longer than ECInstance adapter operation takes we do not need to make copies.
        elementValue.SetAllowsPointersIntoInstanceMemory(true);
        if (ECObjectsStatus::Success != instance.GetValue(elementValue, arrayPropIndex, i))
            return ERROR;

        if (!valueBindingInfo.IsStructArray())
            {
            BeAssert(elementValue.IsPrimitive());

            if (elementValue.IsNull())
                continue; //bind null in that case

            if (SUCCESS != BindPrimitiveValue(arrayElementBinder, elementValue))
                return ERROR;
            }
        else
            {
            BeAssert(elementValue.IsStruct());
            BeAssert(arrayInfo.IsStructArray());
            IECInstancePtr structInstance = elementValue.GetStruct();
            if (structInstance == nullptr || IsPropertyValueNull(*structInstance, valueBindingInfo.GetStructArrayElementBindingInfo()))
                continue; //bind null (extra check seems necessary as ECValue::IsNull always returns false for structs

            if (SUCCESS != BindStructValue(arrayElementBinder, ECInstanceInfo(*structInstance, false), valueBindingInfo.GetStructArrayElementBindingInfo()))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   11/16
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECInstanceAdapterHelper::BindNavigationValue(IECSqlBinder& binder, ECInstanceInfo const& instanceInfo, NavigationECValueBindingInfo const& valueBindingInfo)
    {
    if (!instanceInfo.HasInstance()) // bind null in that case
        return SUCCESS;

    ECValue value;
    if (ECObjectsStatus::Success != instanceInfo.GetInstance().GetValue(value, valueBindingInfo.GetPropertyIndex()))
        return ERROR;

    if (value.IsNull())
        return binder.BindNull() == ECSqlStatus::Success ? SUCCESS : ERROR;

    ECValue::NavigationInfo const& navInfo = value.GetNavigationInfo();
    ECInstanceId navId = navInfo.GetId<ECInstanceId>();
    ECClassId relClassId = navInfo.GetRelationshipClassId();
    return binder.BindNavigation(navId, relClassId) == ECSqlStatus::Success ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   07/14
//+---------------+---------------+---------------+---------------+---------------+------
//static
BentleyStatus ECInstanceAdapterHelper::BindECSqlSystemPropertyValue(IECSqlBinder& binder, ECInstanceInfo const& instanceInfo, ECSqlSystemPropertyBindingInfo const& valueBindingInfo)
    {
    switch (valueBindingInfo.GetKind())
        {
            case ECValueBindingInfo::SystemPropertyKind::ECInstanceId:
            {
            //bind instance's ecinstanceid
            if (instanceInfo.HasInstanceId())
                return ECSqlStatus::Success == binder.BindId(instanceInfo.GetInstanceId()) ? SUCCESS : ERROR;

            return ECSqlStatus::Success == binder.BindNull() ? SUCCESS : ERROR; //-> ECDb will auto-generate the id
            }

            case ECValueBindingInfo::SystemPropertyKind::SourceECInstanceId:
                return ECSqlStatus::Success == binder.BindId(instanceInfo.GetSourceId()) ? SUCCESS : ERROR;

            case ECValueBindingInfo::SystemPropertyKind::TargetECInstanceId:
                return ECSqlStatus::Success == binder.BindId(instanceInfo.GetTargetId()) ? SUCCESS : ERROR;

            default:
                BeAssert(false);
                return ERROR;
        }
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
        PrimitiveArrayECPropertyCP arrayProp = prop.GetAsPrimitiveArrayProperty();
        if (nullptr != arrayProp)
            return arrayProp->IsCalculated();

        StructArrayECProperty const* structArrayProp = prop.GetAsStructArrayProperty();
        if (nullptr == structArrayProp)
            return false;

        structType = &structArrayProp->GetStructElementType();
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
bool ECInstanceAdapterHelper::HasReadonlyPropertiesAreUpdatableOption(ECDbCR ecdb, ECClassCR ecClass, Utf8CP ecsqlOptions)
    {
    if (Utf8String::IsNullOrEmpty(ecsqlOptions))
        return false;

    Utf8String dummyECSql;
    dummyECSql.Sprintf("SELECT NULL FROM ONLY %s ECSQLOPTIONS %s", ecClass.GetECSqlName().c_str(), ecsqlOptions);
    
    ECSqlParser parser;
    std::unique_ptr<Exp> parseTree = parser.Parse(ecdb, dummyECSql.c_str(), ScopedIssueReporter(ecdb, true));
    if (parseTree == nullptr)
        return false;

    SingleSelectStatementExp const* selectStmtExp = nullptr;
    if (parseTree->GetType() == Exp::Type::Select)
        {
        SelectStatementExp const& stmtExp = parseTree->GetAs<SelectStatementExp>();
        BeAssert(stmtExp.GetChildrenCount() != 0);
        selectStmtExp = &stmtExp.GetFirstStatement();
        }
    else if (parseTree->GetType() == Exp::Type::SingleSelect)
        selectStmtExp = &parseTree->GetAs<SingleSelectStatementExp>();

    if (selectStmtExp == nullptr)
        {
        BeAssert(false);
        return false;
        }

    OptionsExp const* optionsExp = selectStmtExp->GetOptions();
    return optionsExp != nullptr && optionsExp->HasOption(OptionsExp::READONLYPROPERTIESAREUPDATABLE_OPTION);
    }




//---------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   02/17
//+---------------+---------------+---------------+---------------+---------------+------
//static
bool ECInstanceAdapterHelper::IsPropertyValueNull(ECN::IECInstanceCR instance, ECValueBindingInfo const& bindingInfo)
    {
    if (bindingInfo.GetType() == ECValueBindingInfo::Type::Struct)
        {
        StructECValueBindingInfo const& structBindingInfo = static_cast<StructECValueBindingInfo const&>(bindingInfo);
        for (auto const& member : structBindingInfo.GetMemberBindingInfos())
            {
            if (!IsPropertyValueNull(instance, *member.second))
                return false;
            }

        return true;
        }


    ECValue v;
    instance.GetValue(v, bindingInfo.GetPropertyIndex());
    switch (bindingInfo.GetType())
        {
            case ECValueBindingInfo::Type::Primitive:
                return v.IsNull();

            case ECValueBindingInfo::Type::Navigation:
                return v.GetNavigationInfo().GetId<BeInt64Id>().IsValid();

            case ECValueBindingInfo::Type::Array:
                return v.IsNull() || v.GetArrayInfo().GetCount() == 0;

            default:
                BeAssert(false);
                return false;
        }
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
        return lhs.GetId() == rhs.GetId();

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