/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPropertyPathImpl.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlPropertyPathImpl.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::Impl::Impl ()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                             Krischan.Eberle      10/2013
//---------------------------------------------------------------------------------------
ECSqlPropertyPath::Impl::~Impl ()
    { 
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::Impl::Impl (Impl const& rhs)
    : m_entryList (rhs.m_entryList)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::Impl& ECSqlPropertyPath::Impl::operator= (Impl const& rhs)
    {
    if (this != &rhs)
        {
        m_entryList = rhs.m_entryList;
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::Impl::Impl (Impl&& rhs)
    : m_entryList (std::move (rhs.m_entryList))
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::Impl& ECSqlPropertyPath::Impl::operator= (Impl&& rhs)
    {
    if (this != &rhs)
        {
        m_entryList = std::move (rhs.m_entryList);
        }

    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPropertyPath::Impl::AddEntry (ECPropertyCR ecProperty)
    {
    auto entry = make_shared<Entry> (ecProperty);
    m_entryList.push_back (entry);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPropertyPath::Impl::AddEntry (int arrayIndex)
    {
    auto entry = make_shared<Entry> (arrayIndex);
    m_entryList.push_back (entry);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
void ECSqlPropertyPath::Impl::InsertParentEntries (ECSqlPropertyPath const& parent)
    {
    auto const& parentEntries = parent.m_pimpl->GetEntryList ();
    auto begin = m_entryList.begin ();
    m_entryList.insert (begin, parentEntries.begin (), parentEntries.end ());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECSqlPropertyPath::Impl::SetLeafArrayIndex (int newArrayIndex)
    {
    const auto entryCount = Size ();
    if (entryCount == 0)
        return ERROR;

    Entry& entry = *GetEntry (entryCount - 1);
    return entry.SetArrayIndex (newArrayIndex);
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
size_t ECSqlPropertyPath::Impl::Size () const
    {
    return GetEntryList ().size ();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
shared_ptr<ECSqlPropertyPath::Entry> const& ECSqlPropertyPath::Impl::GetEntry (size_t entryIndex) const
    {
    return GetEntryList ().at (entryIndex);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::Impl::EntryList const& ECSqlPropertyPath::Impl::GetEntryList() const
    {
    return m_entryList;
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::EntryCR ECSqlPropertyPath::Impl::At (size_t index) const
    {
    return *GetEntry (index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::EntryCR ECSqlPropertyPath::Impl::GetLeafEntry () const
    {
    BeAssert (Size () > 0 && "GetLeafEntry must not be called on empty ECSqlPropertyPath");
    return At (Size () - 1);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::const_iterator ECSqlPropertyPath::Impl::begin () const
    {
    ECSqlPropertyPath::const_iterator::Impl* pimpl = new ECSqlPropertyPath::const_iterator::Impl (m_entryList.begin ());
    //const_iterator takes ownership of pimpl
    return ECSqlPropertyPath::const_iterator (pimpl);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 06/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath::const_iterator ECSqlPropertyPath::Impl::end () const
    {
    ECSqlPropertyPath::const_iterator::Impl* pimpl = new ECSqlPropertyPath::const_iterator::Impl (m_entryList.end ());
    //const_iterator takes ownership of pimpl
    return ECSqlPropertyPath::const_iterator (pimpl);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 10/2013
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECSqlPropertyPath::Impl::ToString (ECSqlPropertyPath::FormatOptions options) const
    {
    Utf8String str;
    bool isFirstEntry = true;
    for (auto const& entry : m_entryList)
        {
        auto entryType = entry->GetKind ();
        if (!isFirstEntry && entryType == Entry::Kind::Property)
            str.append (".");

        if (entryType == Entry::Kind::Property)
            str.append (Utf8String (entry->GetProperty ()->GetName ()));
        else
            {
            BeAssert (entryType == Entry::Kind::ArrayIndex);            
            Utf8String arrayIndexStr;
            if (options == ECSqlPropertyPath::FormatOptions::WithArrayIndex)
                arrayIndexStr.Sprintf ("[%d]", entry->GetArrayIndex ());
            else if (options == ECSqlPropertyPath::FormatOptions::WithArrayDescriptor)
                arrayIndexStr = "[]";
            else if (options == ECSqlPropertyPath::FormatOptions::Simple)
                {
                }

            str.append (arrayIndexStr);
            }

        isFirstEntry = false;
        }

    return str;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
