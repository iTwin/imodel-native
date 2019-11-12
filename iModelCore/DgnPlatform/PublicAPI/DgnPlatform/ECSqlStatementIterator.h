/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <ECDb/ECDbApi.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Helper class for creating iterator entries on ECSqlStatement-s
//! @ingroup PlanningGroup
//=======================================================================================
struct ECSqlStatementEntry
    {
protected:
    BeSQLite::EC::ECSqlStatement* m_statement;
    ECSqlStatementEntry(BeSQLite::EC::ECSqlStatement* statement = nullptr) : m_statement(statement) {}
public:
    BeSQLite::EC::ECSqlStatement* GetStatement() const { return m_statement; }
    };

//=======================================================================================
//! Helper base class for creating iterator entries on ECSqlStatement-s
//! @ingroup PlanningGroup
//=======================================================================================
struct ECSqlStatementIteratorBase
{
protected:
    BeSQLite::EC::CachedECSqlStatementPtr m_statement = nullptr;
    bool m_isAtEnd = true;
    int m_idSelectColumnIndex = -1;

    ECSqlStatementIteratorBase() {}

    DGNPLATFORM_EXPORT bool IsEqual(ECSqlStatementIteratorBase const& rhs) const;
    DGNPLATFORM_EXPORT void MoveNext();
    DGNPLATFORM_EXPORT void MoveFirst();
    DGNPLATFORM_EXPORT BeSQLite::EC::ECSqlStatement* PrepareStatement(DgnDbCR dgndb, Utf8CP ecSql, uint32_t idSelectColumnIndex);
};

//=======================================================================================
//! Helper class for creating iterators on ECSqlStatement-s
//! @ingroup PlanningGroup
//=======================================================================================
template <class ITERATOR_ENTRY_TYPE>
struct ECSqlStatementIterator : ECSqlStatementIteratorBase
{
private:
    ITERATOR_ENTRY_TYPE m_entry;
public:
    ECSqlStatementIterator() {}

    BeSQLite::EC::ECSqlStatement* Prepare(DgnDbCR dgndb, Utf8CP ecSql, uint32_t idSelectColumnIndex)
        {
        auto stmt = PrepareStatement(dgndb, ecSql, idSelectColumnIndex);
        if (nullptr != stmt)
            m_entry = ITERATOR_ENTRY_TYPE(stmt);

        return stmt;
        }

    bool operator!=(ECSqlStatementIterator const& rhs) const
        {
        return !(rhs == *this);
        }

    bool operator==(ECSqlStatementIterator const& rhs) const
        {
        return IsEqual(rhs);
        }

    ECSqlStatementIterator& operator++()
        {
        MoveNext();
        return *this;
        }

    const ITERATOR_ENTRY_TYPE &operator* () const
        {
        BeAssert(!m_isAtEnd && "Do not attempt to get the value of the iterator when it is at its end.");
        return m_entry;
        }

    ECSqlStatementIterator begin()
        {
        MoveFirst();
        return *this;
        }

    ECSqlStatementIterator end()
        {
        return ECSqlStatementIterator();
        }

    BeSQLite::EC::ECSqlStatement* GetStatement() const
        {
        return m_statement.get();
        }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

