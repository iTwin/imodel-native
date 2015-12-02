/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnChangeSummary.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <DgnPlatform/DgnDbTables.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Utility to extract Dgn related information from ChangeSet-s
//=======================================================================================
struct DgnChangeSummary : BeSQLite::EC::ChangeSummary
{
private:
    DgnDbR m_dgndb;
    BeSQLite::EC::ECSqlStatementCache m_statementCache;

    void FindChangedRelationshipEndIds(BeSQLite::EC::ECInstanceIdSet& endInstanceIds, Utf8CP relationshipSchemaName, Utf8CP relationshipClassName, ECN::ECRelationshipEnd relationshipEnd);
    void FindUpdatedInstanceIds(BeSQLite::EC::ECInstanceIdSet& updatedInstanceIds, Utf8CP schemaName, Utf8CP className);
    void FindRelatedInstanceIds(DgnElementIdSet& relatedElements, Utf8CP ecsql, BeSQLite::EC::ECInstanceIdSet const& inInstances);

    BentleyStatus ParseClassFullName(Utf8StringR schemaName, Utf8StringR className, Utf8CP classFullName);
    BentleyStatus GetElementsWithAspectUpdates(DgnElementIdSet& elementIds, Utf8CP elementClassFullName, Utf8CP aspectRelationshipClassFullName, Utf8CP aspectClassFullName);

public:
    template<typename T_Derived> struct Entry
    {
        typedef BeSQLite::EC::ChangeSummary::InstanceIterator::Entry Impl;
    private:
        Impl m_impl;
    public:
        explicit Entry(Impl const& impl) : m_impl(impl) { }

        Impl const& GetImpl() const { return this->m_impl; } //!< Get the underlying InstanceIterator::Entry
        BeSQLite::DbOpcode GetDbOpcode() const { return this->GetImpl().GetDbOpcode(); } //!< Get the operation which changed the element
        bool IsIndirectChange() const { return 0 != this->GetImpl().GetIndirect(); } //!< Returns true if this entry reflects an indirect change

        T_Derived& operator++() { ++(this->m_impl); return static_cast<T_Derived&>(*this); } //!< Increment
        T_Derived const& operator*() const { return static_cast<T_Derived const&>(*this); } //!< Dereference
        bool operator==(Entry const& rhs) const { return this->m_impl == rhs.m_impl; } //!< Compare for equality
        bool operator!=(Entry const& rhs) const { return this->m_impl != rhs.m_impl; } //!< Compare for inequality
    };

    template<typename T_Entry> struct Iterator
    {
        typedef BeSQLite::EC::ChangeSummary::InstanceIterator Impl;
    private:
        Impl m_impl;
    protected:
        Iterator(DgnChangeSummary const& summary, ECN::ECClassId classId, BeSQLite::EC::ChangeSummary::QueryDbOpcode opcodes)
            : m_impl(summary, Impl::Options(classId, true, opcodes)) { }
    public:
        typedef T_Entry const_iterator;
        typedef const_iterator iterator;

        const_iterator begin() const { return T_Entry(this->m_impl.begin()); } //!< Return an iterator to the first entry
        const_iterator end() const { return T_Entry(this->m_impl.end()); } //!< Return an iterator just beyond the last entry
    };

    //! An entry in an element iterator
    struct ElementEntry : Entry<ElementEntry>
    {
    private:
        DGNPLATFORM_EXPORT DgnModelId GetModelId(bool before) const;
        DGNPLATFORM_EXPORT AuthorityIssuedCode GetCode(bool before) const;
    public:
        //! Constructor
        explicit ElementEntry(Impl const& impl) : Entry(impl) { }

        DgnElementId GetElementId() const { return DgnElementId(GetImpl().GetInstanceId().GetValue()); }
        DgnModelId GetOriginalModelId() const { return GetModelId(true); } //!< Get the original model ID before the change (for deletes and updates)
        DgnModelId GetCurrentModelId() const { return GetModelId(false); } //!< Get the current model ID after the change (for inserts and updates)

        AuthorityIssuedCode GetOriginalCode() const { return GetCode(true); } //!< Get the original code before the change (for deletes, and updates in which the code was modified)
        AuthorityIssuedCode GetCurrentCode() const { return GetCode(false); } //!< Get the current code after the change (for inserts, and updates in which the code was modified)
    };

    //! An iterator over the elements which have changed.
    struct ElementIterator : Iterator<ElementEntry>
    {
    public:
        //! Constructor, optionally filtered for particular operations
        DGNPLATFORM_EXPORT explicit ElementIterator(DgnChangeSummary const& summary, BeSQLite::EC::ChangeSummary::QueryDbOpcode opcodes=BeSQLite::EC::ChangeSummary::QueryDbOpcode::All);
    };

    struct ModelEntry : Entry<ModelEntry>
    {
    private:
        DGNPLATFORM_EXPORT AuthorityIssuedCode GetCode(bool before) const;
    public:
        explicit ModelEntry(Impl const& impl) : Entry(impl) { }

        DgnModelId GetModelId() const { return DgnModelId(GetImpl().GetInstanceId().GetValue()); }

        AuthorityIssuedCode GetOriginalCode() const { return GetCode(true); }
        AuthorityIssuedCode GetCurrentCode() const { return GetCode(false); }
    };

    struct ModelIterator : Iterator<ModelEntry>
    {
    public:
        DGNPLATFORM_EXPORT explicit ModelIterator(DgnChangeSummary const& summary, BeSQLite::EC::ChangeSummary::QueryDbOpcode opcodes=BeSQLite::EC::ChangeSummary::QueryDbOpcode::All);
    };

    //! Constructor
    DgnChangeSummary(DgnDbR dgndb) : BeSQLite::EC::ChangeSummary(dgndb), m_dgndb(dgndb), m_statementCache(5) {}

    //! Get elements that have changed.  
    DGNPLATFORM_EXPORT void GetChangedElements(DgnElementIdSet& elementIds, BeSQLite::EC::ChangeSummary::QueryDbOpcode queryOpcode);

    //! Get elements that have updated geometries
    DGNPLATFORM_EXPORT void GetElementsWithGeometryUpdates(DgnElementIdSet& elementIds);

    //! Get elements that have updated items
    DGNPLATFORM_EXPORT void GetElementsWithItemUpdates(DgnElementIdSet& elementIds);

    //! Get an iterator over elements that have changed
    //! @param[in]      opcodes Optionally filters changes by operation (delete, insert, update)
    //! @return An iterator over the elements which have changed.
    ElementIterator MakeElementIterator(QueryDbOpcode opcodes=QueryDbOpcode::All) const { return ElementIterator(*this, opcodes); }

    //! Get an iterator over models that have changed
    //! @param[in]      opcodes Optionally filters changes by operation (delete, insert, update)
    //! @return An iterator over the models which have changed.
    ModelIterator MakeModelIterator(QueryDbOpcode opcodes=QueryDbOpcode::All) const { return ModelIterator(*this, opcodes); }

    //! Returns the DgnDb for which this change summary was created
    DgnDbR GetDgnDb() const { return m_dgndb; }

    //! Populate a set of those codes which were newly assigned within these changes, and those which were discarded.
    //! @param[in]      assigned  Codes which were newly assigned within these changes.
    //! @param[in]      discarded Codes which were previously assigned, and removed within these changes.
    DGNPLATFORM_EXPORT void GetCodes(AuthorityIssuedCodeSet& assigned, AuthorityIssuedCodeSet& discarded) const;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__

