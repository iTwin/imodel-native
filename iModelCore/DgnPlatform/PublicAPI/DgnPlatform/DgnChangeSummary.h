/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnChangeSummary.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

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
    //! An iterator over the elements which have changed.
    struct ElementIterator
    {
        typedef BeSQLite::EC::ChangeSummary::InstanceIterator Impl;

        //! An entry in the iterator
        struct Entry
        {
            typedef ElementIterator::Impl::Entry Impl;
        private:
            Impl m_impl;

            DGNPLATFORM_EXPORT DgnModelId GetModelId(bool old) const;
        public:
            //! Constructor
            explicit Entry(Impl const& impl) : m_impl (impl) { }

            Impl const& GetImpl() const { return m_impl; } //!< Get the underlying InstanceIterator::Entry
            BeSQLite::DbOpcode GetDbOpcode() const { return GetImpl().GetDbOpcode(); } //!< Get the operation which changed the element
            bool IsIndirectChange() const { return 0 != GetImpl().GetIndirect(); } //!< Returns true if this entry reflects an indirect change

            DgnElementId GetElementId() const { return DgnElementId(GetImpl().GetInstanceId().GetValue()); }
            DgnModelId GetOriginalModelId() const { return GetModelId(true); } //!< Get the original model ID before the change (for deletes and updates)
            DgnModelId GetCurrentModelId() const { return GetModelId(false); } //!< Get the current model ID after the change (for inserts and updates)

            Entry& operator++() { ++m_impl; return *this; } //!< Increment
            Entry const& operator*() const { return *this; } //!< Dereference
            bool operator==(Entry const& rhs) const { return m_impl == rhs.m_impl; } //!< Compare for equality
            bool operator!=(Entry const& rhs) const { return m_impl != rhs.m_impl; } //!< Compare for inequality
        };
    private:
        Impl m_impl;
    public:
        //! Constructor, optionally filtered for particular operations
        DGNPLATFORM_EXPORT explicit ElementIterator(DgnChangeSummary const& summary, BeSQLite::EC::ChangeSummary::QueryDbOpcode opcodes=BeSQLite::EC::ChangeSummary::QueryDbOpcode::All);

        typedef Entry const_iterator;
        typedef const_iterator iterator;

        const_iterator begin() const { return Entry(m_impl.begin()); } //!< Return an iterator to the first entry
        const_iterator end() const { return Entry(m_impl.end()); } //!< Return an iterator just beyond the last entry
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

    //! Returns the DgnDb for which this change summary was created
    DgnDbR GetDgnDb() const { return m_dgndb; }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__

