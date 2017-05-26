/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlColumnInfo.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/ECSqlColumnInfo.h>

using namespace ECN;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//********************** ECSqlColumnInfo **************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo::ECSqlColumnInfo() {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo::ECSqlColumnInfo(ECTypeDescriptor const& dataType, DateTime::Info const& dateTimeInfo, ECN::ECStructClassCP structType, ECPropertyCP ecProperty, bool isGeneratedProperty, ECSqlPropertyPath const& propertyPath, ECN::ECClassCR rootClass, Utf8CP rootClassAlias)
    : m_dataType(dataType), m_dateTimeInfo(dateTimeInfo), m_structType(structType), m_property(ecProperty), m_isGeneratedProperty(isGeneratedProperty), m_propertyPath(propertyPath), m_rootClass(&rootClass), m_rootClassAlias(rootClassAlias)
    {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlColumnInfo ECSqlColumnInfo::CreateTopLevel(bool isGeneratedProperty, ECSqlPropertyPath const& propertyPath, ECN::ECClassCR rootClass, Utf8CP rootClassAlias)
    {
    BeAssert(propertyPath.Size() > 0);
    ECPropertyCP ecProperty = propertyPath.GetLeafEntry().GetProperty();
    BeAssert(ecProperty != nullptr);
    DateTime::Info dateTimeInfo;
    ECStructClassCP structType = nullptr;
    ECTypeDescriptor typeDescriptor = DetermineDataType(dateTimeInfo, structType, *ecProperty);
    return ECSqlColumnInfo(typeDescriptor, dateTimeInfo, structType, ecProperty, isGeneratedProperty, propertyPath, rootClass, rootClassAlias);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlColumnInfo ECSqlColumnInfo::CreateChild(ECSqlColumnInfo const& parent, ECPropertyCR childProperty)
    {
    DateTime::Info dateTimeInfo;
    ECStructClassCP structType = nullptr;
    ECTypeDescriptor dataType = DetermineDataType(dateTimeInfo, structType, childProperty);
    ECSqlPropertyPath childPropPath;
    childPropPath.InsertEntriesAtBeginning(parent.GetPropertyPath());
    childPropPath.AddEntry(childProperty);

    return ECSqlColumnInfo(dataType, dateTimeInfo, structType, &childProperty, parent.IsGeneratedProperty(), childPropPath, parent.GetRootClass(), parent.GetRootClassAlias());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlColumnInfo ECSqlColumnInfo::CreateForArrayElement(ECSqlColumnInfo const& parent, int arrayIndex)
    {
    ECTypeDescriptor arrayElementDataType;
    DateTime::Info dateTimeInfo;
    ECStructClassCP structType = nullptr;
    if (parent.GetDataType().IsPrimitiveArray())
        {
        arrayElementDataType = ECTypeDescriptor::CreatePrimitiveTypeDescriptor(parent.GetDataType().GetPrimitiveType());
        dateTimeInfo = parent.m_dateTimeInfo;
        }
    else
        {
        BeAssert(parent.GetDataType().IsStructArray());
        structType = parent.m_structType;
        arrayElementDataType = ECTypeDescriptor::CreateStructTypeDescriptor();
        }

    ECSqlPropertyPath childPropPath;
    childPropPath.InsertEntriesAtBeginning(parent.GetPropertyPath());
    childPropPath.AddEntry(arrayIndex);

    return ECSqlColumnInfo(arrayElementDataType, dateTimeInfo, structType, nullptr, parent.IsGeneratedProperty(), childPropPath, parent.GetRootClass(), parent.GetRootClassAlias());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECTypeDescriptor ECSqlColumnInfo::DetermineDataType(DateTime::Info& dateTimeInfo, ECN::ECStructClassCP& structType, ECPropertyCR ecProperty)
    {
    if (ecProperty.GetIsPrimitive())
        {
        const PrimitiveType primType = ecProperty.GetAsPrimitiveProperty()->GetType();
        if (primType == PRIMITIVETYPE_DateTime)
            {
            if (ECObjectsStatus::Success != CoreCustomAttributeHelper::GetDateTimeInfo(dateTimeInfo, ecProperty))
                {
                LOG.errorv("Could not read DateTimeInfo custom attribute from the primitive ECProperty %s:%s.",
                           ecProperty.GetClass().GetFullName(), ecProperty.GetName().c_str());
                BeAssert(false && "Could not read DateTimeInfo custom attribute from the corresponding primitive ECProperty.");
                }
            }

        return ECTypeDescriptor::CreatePrimitiveTypeDescriptor(primType);
        }

    if (ecProperty.GetIsStruct())
        {
        structType = &ecProperty.GetAsStructProperty()->GetType();
        return ECTypeDescriptor::CreateStructTypeDescriptor();
        }

    if (ecProperty.GetIsPrimitiveArray())
        {
        const PrimitiveType primType = ecProperty.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
        if (primType == PRIMITIVETYPE_DateTime)
            {
            if (ECObjectsStatus::Success != CoreCustomAttributeHelper::GetDateTimeInfo(dateTimeInfo, ecProperty))
                {
                LOG.errorv("Could not read DateTimeInfo custom attribute from the primitive array ECProperty %s:%s.",
                           ecProperty.GetClass().GetFullName(), ecProperty.GetName().c_str());
                BeAssert(false && "Could not read DateTimeInfo custom attribute from the corresponding primitive array ECProperty.");
                }
            }

        return ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor(primType);
        }

    if (ecProperty.GetIsStructArray())
        {
        structType = &ecProperty.GetAsStructArrayProperty()->GetStructElementType();
        return ECTypeDescriptor::CreateStructArrayTypeDescriptor();
        }
        

    if (ecProperty.GetIsNavigation())
        {
        NavigationECPropertyCP navProp = ecProperty.GetAsNavigationProperty();
        return ECTypeDescriptor::CreateNavigationTypeDescriptor(navProp->GetType(), navProp->IsMultiple());
        }

    BeAssert(false && "Unhandled ECProperty type. Adjust code to new ECProperty type");
    return ECTypeDescriptor();
    }

//********************** ECSqlPropertyPath **************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::ECSqlPropertyPath() {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::const_iterator ECSqlPropertyPath::begin() const { return const_iterator(m_entryList.begin()); }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::const_iterator ECSqlPropertyPath::end() const { return const_iterator(m_entryList.end()); }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPropertyPath::AddEntry(ECPropertyCR ecProperty)
    {
    m_entryList.push_back(Entry::Create(ecProperty));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPropertyPath::AddEntry(int arrayIndex)
    {
    m_entryList.push_back(Entry::Create(arrayIndex));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPropertyPath::InsertEntriesAtBeginning(ECSqlPropertyPath const& pathToInsert)
    {
    m_entryList.insert(m_entryList.begin(), pathToInsert.m_entryList.begin(), pathToInsert.m_entryList.end());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlPropertyPath::SetLeafArrayIndex(int newArrayIndex)
    {
    const size_t entryCount = Size();
    if (entryCount == 0)
        return ERROR;

    Entry& entry = *GetEntry(entryCount - 1);
    return entry.SetArrayIndex(newArrayIndex);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECSqlPropertyPath::ToString(ECSqlPropertyPath::FormatOptions options) const
    {
    Utf8String str;
    bool isFirstEntry = true;
    for (EntryPtr const& entry : m_entryList)
        {
        Entry::Kind entryKind = entry->GetKind();
        if (!isFirstEntry && entryKind == Entry::Kind::Property)
            str.append(".");

        if (entryKind == Entry::Kind::Property)
            str.append(Utf8String(entry->GetProperty()->GetName()));
        else
            {
            BeAssert(entryKind == Entry::Kind::ArrayIndex);
            Utf8String arrayIndexStr;
            if (options == ECSqlPropertyPath::FormatOptions::WithArrayIndex)
                arrayIndexStr.Sprintf("[%d]", entry->GetArrayIndex());
            else if (options == ECSqlPropertyPath::FormatOptions::WithArrayDescriptor)
                arrayIndexStr = "[]";
            else if (options == ECSqlPropertyPath::FormatOptions::Simple)
                {
                }

            str.append(arrayIndexStr);
            }

        isFirstEntry = false;
        }

    return str;
    }



//********************** ECSqlPropertyPath::Entry **************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlPropertyPath::EntryPtr ECSqlPropertyPath::Entry::Create(ECN::ECPropertyCR ecProperty)
    {
    return new Entry(ecProperty);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlPropertyPath::EntryPtr ECSqlPropertyPath::Entry::Create(int arrayIndex)
    {
    return new Entry(arrayIndex);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlPropertyPath::Entry::SetArrayIndex(int newArrayIndex)
    {
    if (GetKind() != Kind::ArrayIndex)
        {
        BeAssert(GetKind() == Kind::ArrayIndex && "ECSqlPropertyPath::Entry::SetArrayIndex can only be called if GetKind () == Kind::ArrayIndex.");
        return ERROR;
        }

    m_arrayIndex = newArrayIndex;
    return SUCCESS;
    }

//********************** ECSqlPropertyPath::const_iterator **************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::const_iterator::const_iterator(bvector<EntryPtr>::const_iterator innerIterator) : m_innerIterator(innerIterator)
    {}

END_BENTLEY_SQLITE_EC_NAMESPACE