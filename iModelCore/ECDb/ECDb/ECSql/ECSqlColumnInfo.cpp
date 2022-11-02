/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <ECDb/ECSqlColumnInfo.h>

using namespace ECN;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//********************** ECSqlColumnInfo **************************************
//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo::ECSqlColumnInfo() {}

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo::ECSqlColumnInfo(ECTypeDescriptor const& dataType, DateTime::Info const& dateTimeInfo, ECN::ECStructClassCP structType, ECPropertyCP ecProperty, ECPropertyCP originProperty, bool isSystemProperty, bool isGeneratedProperty, ECSqlPropertyPath const& propertyPath, RootClass const& rootClass)
    : m_dataType(dataType), m_dateTimeInfo(dateTimeInfo), m_structType(structType), m_property(ecProperty), m_originProperty(originProperty), m_isSystemProperty(isSystemProperty), m_isGeneratedProperty(isGeneratedProperty), m_propertyPath(propertyPath), m_rootClass(rootClass)
    {}

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECEnumerationCP ECSqlColumnInfo::GetEnumType() const
    {
    ECN::ECPropertyCP prop = nullptr;
    ECSqlPropertyPath::Entry const& leafEntry = m_propertyPath.GetLeafEntry();
    if (leafEntry.GetKind() == ECSqlPropertyPath::Entry::Kind::Property)
        prop = leafEntry.GetProperty();
    else
        {
        ECSqlPropertyPath::Entry const& arrayPropEntry = m_propertyPath.At(m_propertyPath.Size() - 2);
        BeAssert(arrayPropEntry.GetKind() == ECSqlPropertyPath::Entry::Kind::Property);
        prop = arrayPropEntry.GetProperty();
        }

    if (prop == nullptr)
        {
        BeAssert(prop != nullptr);
        return nullptr;
        }

    PrimitiveECPropertyCP primProp = prop->GetAsPrimitiveProperty();
    if (primProp != nullptr)
        return primProp->GetEnumeration();

    PrimitiveArrayECPropertyCP primArrayProp = prop->GetAsPrimitiveArrayProperty();
    if (primArrayProp != nullptr)
        return primArrayProp->GetEnumeration();

    return nullptr;
    }

//********************** ECSqlColumnInfo::RootClass **************************************
//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo::RootClass::RootClass() {}

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo::RootClass::RootClass(ECN::ECClassCR ecClass, Utf8CP tableSpace, Utf8CP alias) : m_class(&ecClass), m_tableSpace(tableSpace), m_alias(alias) {}

//********************** ECSqlPropertyPath **************************************
//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::ECSqlPropertyPath() {}

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::const_iterator ECSqlPropertyPath::begin() const { return const_iterator(m_entryList.begin()); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::const_iterator ECSqlPropertyPath::end() const { return const_iterator(m_entryList.end()); }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPropertyPath::AddEntry(ECPropertyCR ecProperty)
    {
    m_entryList.push_back(Entry::Create(ecProperty));
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPropertyPath::AddEntry(int arrayIndex)
    {
    m_entryList.push_back(Entry::Create(arrayIndex));
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPropertyPath::InsertEntriesAtBeginning(ECSqlPropertyPath const& pathToInsert)
    {
    m_entryList.insert(m_entryList.begin(), pathToInsert.m_entryList.begin(), pathToInsert.m_entryList.end());
    }

//--------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlPropertyPath::EntryPtr ECSqlPropertyPath::Entry::Create(ECN::ECPropertyCR ecProperty)
    {
    return new Entry(ecProperty);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlPropertyPath::EntryPtr ECSqlPropertyPath::Entry::Create(int arrayIndex)
    {
    return new Entry(arrayIndex);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::const_iterator::const_iterator(bvector<EntryPtr>::const_iterator innerIterator) : m_innerIterator(innerIterator)
    {}

END_BENTLEY_SQLITE_EC_NAMESPACE