/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include "ECDbInternalTypes.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
// @bsiclass                                                 Krischan.Eberle       11/2017
//======================================================================================
struct DbTableSpace final
    {
    public:
        enum class Type
            {
            Main,
            Temp,
            Attached
            };

    private:
        Type m_type = Type::Main;
        Utf8String m_name;
        Utf8String m_filePath;

        // Bentley coding guideline: don't free static non-POD members
        static DbTableSpace const* s_main;

    public:
        explicit DbTableSpace(Utf8CP name, Utf8CP filePath = nullptr);

        bool operator==(DbTableSpace const& rhs) const { return m_type == rhs.m_type && (m_type != Type::Attached || m_name.EqualsIAscii(rhs.m_name)); }
        bool operator!=(DbTableSpace const& rhs) const { return !(*this == rhs); }

        bool IsValid() const { return !m_name.empty(); }

        Utf8StringCR GetName() const { return m_name; }
        Utf8StringCR GetFilePath() const { return m_filePath; }
        bool IsMain() const { return m_type == Type::Main; }
        bool IsTemp() const { return m_type == Type::Temp; }
        bool IsAttached() const { return m_type == Type::Attached; }

       
        static bool IsAttachedECDbFile(ECDbCR, Utf8CP tableSpace);
        static bool Exists(ECDbCR, Utf8CP tableSpace);
        
        static DbTableSpace const& Main() { return *s_main; }

        static bool IsAny(Utf8CP tableSpace) { return Utf8String::IsNullOrEmpty(tableSpace); }
        static bool IsMain(Utf8CP tableSpace);
        static bool IsTemp(Utf8CP tableSpace);
    };


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      06/2017
//+===============+===============+===============+===============+===============+======
struct Utf8StringVirtualSet final : VirtualSet
    {
    private:
        std::function<bool(Utf8CP)> m_predicate;

        bool _IsInSet(int nVals, DbValue const* values) const override
            {
            BeAssert(nVals == 1);
            return m_predicate(values[0].GetValueText());
            }

    public:
        explicit Utf8StringVirtualSet(std::function<bool(Utf8CP)> const& predicate) : VirtualSet(), m_predicate(predicate) {}

        ~Utf8StringVirtualSet() {}
    };

//======================================================================================
// @bsiclass                                             Krischan.Eberle       11/2017
//======================================================================================
struct DbUtilities final
    {
private:
    DbUtilities() = delete;
    ~DbUtilities() = delete;

public:

    //! If not found, returns still SUCCESS, but an invalid class id.
    //! Returns ERROR if statement failed to execute
    static BentleyStatus QueryRowClassId(ECN::ECClassId&, ECDbCR, Utf8StringCR tableName, Utf8StringCR classIdColName, Utf8StringCR pkColName, ECInstanceId id);

    template<typename TId>
    static TId GetLastInsertedId(ECDbCR ecdb)
        {
        const int64_t id = ecdb.GetLastInsertRowId();
        if (id <= 0)
            {
            BeAssert(false && "Could not retrieve last inserted row id from SQLite");
            return TId();
            }

        return TId((uint64_t) id);
        }

    static bool TableSpaceExists(ECDbCR ecdb, Utf8CP tableSpace);
    static BentleyStatus GetTableSpaces(std::vector<Utf8String>& tableSpaces, ECDbCR ecdb, bool onlyAttachedTableSpaces = false);
    static Utf8String GetAttachedFilePath(ECDbCR, Utf8CP tableSpace);

    static bool TableExists(ECDbCR ecdb, Utf8CP tableName, Utf8CP tableSpace = nullptr)
        {
        if (Utf8String::IsNullOrEmpty(tableSpace))
            return BE_SQLITE_OK == ecdb.TryExecuteSql(Utf8PrintfString("SELECT NULL FROM [%s]", tableName).c_str());

        return BE_SQLITE_OK == ecdb.TryExecuteSql(Utf8PrintfString("SELECT NULL FROM [%s].[%s]", tableSpace, tableName).c_str());
        }

    static bool IndexExists(ECDbCR ecdb, Utf8CP indexName, Utf8CP tableSpace = nullptr)
        {
        CachedStatementPtr stmt;
        if (Utf8String::IsNullOrEmpty(tableSpace))
            stmt = ecdb.GetCachedStatement("SELECT 1 FROM sqlite_master where type='index' AND name=?");
        else
            stmt = ecdb.GetCachedStatement(Utf8PrintfString("SELECT 1 FROM %s.sqlite_master where type='index' AND name=?", tableSpace).c_str());

        if (stmt == nullptr)
            {
            BeAssert(false);
            return false;
            }

        stmt->BindText(1, indexName, Statement::MakeCopy::No);
        return stmt->Step() == BE_SQLITE_ROW;
        }

    };
END_BENTLEY_SQLITE_EC_NAMESPACE

