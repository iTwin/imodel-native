/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlColumnInfo.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
ECSqlColumnInfo::ECSqlColumnInfo(ECTypeDescriptor const& dataType, DateTime::Info const& dateTimeInfo, ECN::ECStructClassCP structType, ECPropertyCP ecProperty, bool isSystemProperty, bool isGeneratedProperty, ECSqlPropertyPath const& propertyPath, RootClass const& rootClass)
    : m_dataType(dataType), m_dateTimeInfo(dateTimeInfo), m_structType(structType), m_property(ecProperty), m_isSystemProperty(isSystemProperty), m_isGeneratedProperty(isGeneratedProperty), m_propertyPath(propertyPath), m_rootClass(rootClass)
    {}

//********************** ECSqlColumnInfo::RootClass **************************************
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2018
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo::RootClass::RootClass() {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 01/2018
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlColumnInfo::RootClass::RootClass(ECN::ECClassCR ecClass, Utf8CP tableSpace, Utf8CP alias) : m_class(&ecClass), m_tableSpace(tableSpace), m_alias(alias) {}

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