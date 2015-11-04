/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/ECSqlStatementIterator.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
    };

//=======================================================================================
//! Helper class for creating iterators on ECSqlStatement-s
//! @ingroup PlanningGroup
//=======================================================================================
template <class ITERATOR_ENTRY_TYPE>
struct ECSqlStatementIterator
{
private:
    ITERATOR_ENTRY_TYPE m_entry;
    BeSQLite::EC::CachedECSqlStatementPtr m_statement = nullptr;
    bool m_isAtEnd = true;
    int m_idSelectColumnIndex = -1;

    bool IsEqual(ECSqlStatementIterator const& rhs) const
        {
        if (m_isAtEnd && rhs.m_isAtEnd)
            return true;
        if (m_isAtEnd != rhs.m_isAtEnd)
            return false;

        BeAssert(m_statement.IsValid() && rhs.m_statement.IsValid());
        BeSQLite::EC::ECInstanceId thisId = m_statement->GetValueId<BeSQLite::EC::ECInstanceId>(m_idSelectColumnIndex);
        
        // Do NOT delete the next line and simply use rhs.m_statement on the subsequent.
        // Android GCC 4.9 and clang 6.1.0 cannot deduce the templates when you try to combine it all up.
        BeSQLite::EC::CachedECSqlStatementPtr rhsStatement = rhs.m_statement;
        BeSQLite::EC::ECInstanceId rhsId = rhsStatement->GetValueId<BeSQLite::EC::ECInstanceId>(rhs.m_idSelectColumnIndex);
        
        return thisId == rhsId;
        }

    void MoveNext()
        {
        if (m_isAtEnd)
            {
            BeAssert(false && "Do not attempt to iterate beyond the end of the instances.");
            return;
            }
        BeSQLite::DbResult stepStatus = m_statement->Step();
        BeAssert(stepStatus == BeSQLite::BE_SQLITE_ROW || stepStatus == BeSQLite::BE_SQLITE_DONE);
        if (stepStatus != BeSQLite::BE_SQLITE_ROW)
            m_isAtEnd = true;
        }

    void MoveFirst()
        {
        if (!m_statement.IsValid())
            {
            m_isAtEnd = true;
            return;
            }
        m_statement->Reset();
        m_isAtEnd = false;
        MoveNext();
        }

public:
    ECSqlStatementIterator() {}

    BeSQLite::EC::ECSqlStatement* Prepare(DgnDbCR dgndb, Utf8CP ecSql, uint32_t idSelectColumnIndex)
        {
        m_statement = dgndb.GetPreparedECSqlStatement(ecSql);
        if (m_statement.IsNull())
            {
            BeAssert(false);
            return nullptr;
            }

        m_isAtEnd = false;
        m_idSelectColumnIndex = (int) idSelectColumnIndex;
        m_entry = ITERATOR_ENTRY_TYPE(m_statement.get());
        return m_statement.get();
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

