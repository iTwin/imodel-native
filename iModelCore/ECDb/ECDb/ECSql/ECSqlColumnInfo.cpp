/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlColumnInfo.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
ECSqlColumnInfo::ECSqlColumnInfo() : m_property(nullptr), m_isGeneratedProperty(false), m_rootClass(nullptr) {}


//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo::ECSqlColumnInfo(ECTypeDescriptor const& dataType, ECPropertyCP ecProperty, bool isGeneratedProperty, ECSqlPropertyPath&& propertyPath, ECN::ECClassCR rootClass, Utf8CP rootClassAlias)
    : m_dataType(dataType), m_property(ecProperty), m_isGeneratedProperty(isGeneratedProperty), m_propertyPath(std::move(propertyPath)), m_rootClass(&rootClass), m_rootClassAlias(rootClassAlias)
    {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath& ECSqlColumnInfo::GetPropertyPathR() { return m_propertyPath; }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlColumnInfo ECSqlColumnInfo::CreateTopLevel(bool isGeneratedProperty, ECSqlPropertyPath&& propertyPath, ECN::ECClassCR rootClass, Utf8CP rootClassAlias)
    {
    BeAssert(propertyPath.Size() > 0);
    ECPropertyCP ecProperty = propertyPath.GetLeafEntry().GetProperty();
    BeAssert(ecProperty != nullptr);
    return ECSqlColumnInfo(DetermineDataType(*ecProperty), ecProperty, isGeneratedProperty, std::move(propertyPath), rootClass, rootClassAlias);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlColumnInfo ECSqlColumnInfo::CreateChild(ECSqlColumnInfo const& parent, ECPropertyCR childProperty)
    {
    auto dataType = DetermineDataType(childProperty);
    ECSqlPropertyPath childPropPath;
    childPropPath.InsertEntriesAtBeginning(parent.GetPropertyPath());
    childPropPath.AddEntry(childProperty);

    return ECSqlColumnInfo(dataType, &childProperty, parent.IsGeneratedProperty(), std::move(childPropPath), parent.GetRootClass(), parent.GetRootClassAlias());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECSqlColumnInfo ECSqlColumnInfo::CreateForArrayElement(ECSqlColumnInfo const& parent, int arrayIndex)
    {
    ECTypeDescriptor arrayElementDataType;
    if (parent.GetDataType().IsPrimitiveArray())
        arrayElementDataType = ECTypeDescriptor::CreatePrimitiveTypeDescriptor(parent.GetDataType().GetPrimitiveType());
    else
        {
        BeAssert(parent.GetDataType().IsStructArray());
        arrayElementDataType = ECTypeDescriptor::CreateStructTypeDescriptor();
        }

    ECSqlPropertyPath childPropPath;
    childPropPath.InsertEntriesAtBeginning(parent.GetPropertyPath());
    childPropPath.AddEntry(arrayIndex);

    return ECSqlColumnInfo(arrayElementDataType, nullptr, parent.IsGeneratedProperty(), std::move(childPropPath), parent.GetRootClass(), parent.GetRootClassAlias());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECTypeDescriptor ECSqlColumnInfo::DetermineDataType(ECPropertyCR ecProperty)
    {
    if (ecProperty.GetIsPrimitive())
        return ECTypeDescriptor::CreatePrimitiveTypeDescriptor(ecProperty.GetAsPrimitiveProperty()->GetType());
    else if (ecProperty.GetIsStruct())
        return ECTypeDescriptor::CreateStructTypeDescriptor();
    else if (ecProperty.GetIsArray())
        {
        auto arrayProp = ecProperty.GetAsArrayProperty();
        if (arrayProp->GetKind() == ARRAYKIND_Primitive)
            return ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor(arrayProp->GetPrimitiveElementType());
        else
            return ECTypeDescriptor::CreateStructArrayTypeDescriptor();
        }
    else if (ecProperty.GetIsNavigation())
        {
        auto navProp = ecProperty.GetAsNavigationProperty();
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