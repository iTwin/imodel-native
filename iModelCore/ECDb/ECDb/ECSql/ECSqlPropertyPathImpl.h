/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlPropertyPathImpl.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/ECSqlColumnInfo.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! ECSqlPropertyPath::Impl is the private implementation of ECSqlPropertyPath hidden from the public headers
//! (PIMPL idiom)
//! PIMPL is used here so that shared_ptr can be used. Without PIMPL shared_ptr would show
//! up in the public API which is not allowed.
//! @bsiclass                                                Krischan.Eberle      10/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlPropertyPath::Impl
    {
public:
    typedef std::vector<std::shared_ptr<Entry>> EntryList;

private:
    EntryList m_entryList;

    EntryList const& GetEntryList () const;
    std::shared_ptr<Entry> const& GetEntry (size_t index) const;

public:
    Impl ();
    ~Impl ();

    Impl (Impl const& rhs);
    Impl& operator= (Impl const& rhs);
    Impl (Impl&& rhs);
    Impl& operator= (Impl&& rhs);

    //ECSqlPropertyPath API mirrors
    //  public API
    size_t Size () const;
    EntryCR At (size_t index) const;
    EntryCR GetLeafEntry () const;
    const_iterator begin () const;
    const_iterator end () const;
    Utf8String ToString (ECSqlPropertyPath::FormatOptions options) const;
    //  helpers
    void InsertParentEntries (ECSqlPropertyPath const& parent);
    void AddEntry (ECN::ECPropertyCR ecProperty);
    void AddEntry (int arrayIndex);
    BentleyStatus SetLeafArrayIndex (int newArrayIndex);
    };

//=======================================================================================
//! ECSqlPropertyPath::const_iterator::Impl is the private implementation of 
//! ECSqlPropertyPath::const_iterator hidden from the public headers
//! (PIMPL idiom)
//! PIMPL is used here so that shared_ptr can be used. Without PIMPL shared_ptr would show
//! up in the public API which is not allowed.
//! The PIMPL does not mirror the iterator interface, but just wraps the std::vector iterator
//! @bsiclass                                                Krischan.Eberle      10/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlPropertyPath::const_iterator::Impl
    {
    public:
        ECSqlPropertyPath::Impl::EntryList::const_iterator m_iterator;

        explicit Impl (ECSqlPropertyPath::Impl::EntryList::const_iterator const& innerIterator)
            : m_iterator (innerIterator)
            {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE