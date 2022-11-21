/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECDb/ECSqlStatement.h>
#include "PragmaStatementExp.h"
#include "ECSqlPreparedStatement.h"
#include <any>


BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct PragmaManager;
//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaResult : NonCopyableClass{
    private:
        struct Field final : public IECSqlValue {
            private:
                ECSqlColumnInfo m_ecsqlColumnInfo;
                PragmaResult& m_result;
                int m_columnIndex;
                ECSqlColumnInfoCR _GetColumnInfo() const override { return m_ecsqlColumnInfo; }
                bool _IsNull() const override;
                bool _GetBoolean() const override;
                double _GetDouble() const override;
                int64_t _GetInt64() const override;
                Utf8CP _GetText() const override;
                int _GetInt() const override;
                // unsupported value type
                void const* _GetBlob(int* blobSize) const override;
                uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
                double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
                DPoint2d _GetPoint2d() const override;
                DPoint3d _GetPoint3d() const override;
                IGeometryPtr _GetGeometry() const override;
                IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override;
                IECSqlValueIterable const& _GetStructIterable() const override;
                int _GetArrayLength() const override;
                IECSqlValueIterable const& _GetArrayIterable() const override;
            public:
                Field(ECSqlColumnInfo const& ecsqlColumnIndex, int columnIndex, PragmaResult& result)
                    :m_ecsqlColumnInfo(ecsqlColumnIndex),m_columnIndex(columnIndex),m_result(result){};
                ~Field() {}
        };
        virtual DbResult _Step() = 0;
        virtual DbResult _Reset() = 0;
        virtual DbResult _Init() = 0;
        virtual BeJsValue* _CurrentRow() = 0;

        ECDbCR m_ecdb;
        mutable BeMutex m_mutex;
        bool m_init;
        bool m_allowSchemaChange;
        ECN::ECSchemaPtr m_schema;
        ECN::ECEntityClassP m_class;
        std::vector<std::unique_ptr<Field>> m_columns;

    protected:
        BeMutex& GetMutex() const { return m_mutex; }
        bool IsInitCalled() const { return m_init; }

    public:
        explicit PragmaResult(ECDbCR);
        ECDbCR GetECDb() const { return m_ecdb; }
        DbResult Step();
        DbResult Reset();
        int GetColumnCount() const;
        void FreezeSchemaChanges();
        bool IsSchemaChangesAllowed() const;
        IECSqlValue const& GetValue(int columnIndex) const;
        ECPropertyCP AppendProperty(Utf8StringCR name, ECN::PrimitiveType type);
        virtual ~PragmaResult(){}
};


//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaManager final {
    friend struct PragmaHelp;
    enum class Operation {
        Read,
        Write,
    };
    using RowSet = std::unique_ptr<PragmaResult>;

    struct Handler {
        enum class Type {
            Global,
            Schema,
            Class,
            Property,
            Any
        };

    private:
        Utf8String m_name;
        Utf8String m_description;
        Type m_type;
    public:
        Handler(Type type, Utf8CP name, Utf8CP descr) :m_type(type), m_name(name), m_description(descr) {}
        Utf8StringCR GetName() const { return m_name; }
        Utf8StringCR GetDescription() const { return m_description; }
        Type GetType() const { return m_type; }
        Utf8CP GetTypeString() const;
        virtual ~Handler(){}
    };
    struct GlobalHandler : Handler{
        GlobalHandler(Utf8CP name, Utf8CP descr):Handler(Type::Global, name, descr){}
        virtual DbResult Read(RowSet&, ECDbCR, PragmaVal const&) =0;
        virtual DbResult Write(RowSet&, ECDbCR, PragmaVal const&) = 0;
        virtual ~GlobalHandler(){}
    };
    struct SchemaHandler : Handler{
        SchemaHandler(Utf8CP name, Utf8CP descr):Handler(Type::Schema, name, descr){}
        virtual DbResult Read(RowSet&, ECDbCR, PragmaVal const&, ECSchemaCR) =0;
        virtual DbResult Write(RowSet&, ECDbCR, PragmaVal const&, ECSchemaCR) = 0;
        virtual ~SchemaHandler(){}
    };
    struct ClassHandler : Handler{
        ClassHandler(Utf8CP name, Utf8CP descr):Handler(Type::Class, name, descr){}
        virtual DbResult Read(RowSet&, ECDbCR, PragmaVal const&, ECClassCR) =0;
        virtual DbResult Write(RowSet&, ECDbCR, PragmaVal const&, ECClassCR) = 0;
        virtual ~ClassHandler(){}
    };
    struct PropertyHandler : Handler{
        PropertyHandler(Utf8CP name, Utf8CP descr):Handler(Type::Property, name, descr){}
        virtual DbResult Read(RowSet&, ECDbCR, PragmaVal const&, ECPropertyCR) =0;
        virtual DbResult Write(RowSet&, ECDbCR, PragmaVal const&, ECPropertyCR) =0;
        virtual ~PropertyHandler(){}
    };
    struct AnyHandler : Handler{
        AnyHandler(Utf8CP name, Utf8CP descr):Handler(Type::Any, name, descr){}
        virtual DbResult Read(RowSet&, ECDbCR, PragmaVal const&, std::vector<Utf8String> const&) =0;
        virtual DbResult Write(RowSet&, ECDbCR, PragmaVal const&, std::vector<Utf8String> const&) = 0;
        virtual ~AnyHandler(){}
    };
   using HandlerMap = std::map<Utf8CP, std::unique_ptr<Handler>, CompareIUtf8Ascii>;
private:
    ECDbCR m_ecdb;

    mutable std::map<Handler::Type, HandlerMap> m_handlers;

    DbResult PrepareGlobal(RowSet&,Utf8StringCR, PragmaVal const&, Operation) const;
    DbResult PrepareSchema(RowSet&,Utf8StringCR, PragmaVal const&, Operation, ECN::ECSchemaCR) const;
    DbResult PrepareClass(RowSet&,Utf8StringCR, PragmaVal const&, Operation, ECN::ECClassCR) const;
    DbResult PrepareProperty(RowSet&,Utf8StringCR, PragmaVal const&, Operation, ECN::ECPropertyCR) const;
    DbResult PrepareAny(RowSet&,Utf8StringCR, PragmaVal const&, Operation, std::vector<Utf8String> const&) const;
    DbResult Prepare(RowSet&,Utf8StringCR, PragmaVal const&, Operation, std::vector<Utf8String> const&) const;
    DbResult DefaultGlobal(RowSet&,Utf8StringCR, PragmaVal const&, Operation) const;
    DbResult DefaultSchema(RowSet&,Utf8StringCR, PragmaVal const&, Operation, ECN::ECSchemaCR) const;
    DbResult DefaultClass(RowSet&,Utf8StringCR, PragmaVal const&, Operation, ECN::ECClassCR) const;
    DbResult DefaultProperty(RowSet&,Utf8StringCR, PragmaVal const&, Operation, ECN::ECPropertyCR) const;
    DbResult DefaultAny(RowSet&,Utf8StringCR, PragmaVal const&, Operation, std::vector<Utf8String> const&) const;

    Handler* GetHandler(Handler::Type type, Utf8StringCR name) const;
    template<typename T=Handler>
    T* GetHandlerAs(Handler::Type type, Utf8StringCR name) const {
        auto handler = GetHandler(type, name);
        if (handler != nullptr) {
            BeAssert(dynamic_cast<T*>(handler) != nullptr);
        }
        return static_cast<T*>(handler);
    }
    void InitSystemPragmas();

public:
    explicit PragmaManager(ECDbCR ecdb) : m_ecdb(ecdb) { InitSystemPragmas(); }
    BentleyStatus Register(std::unique_ptr<Handler>);
    DbResult Prepare(RowSet&, PragmaStatementExp const& exp) const;
    ECDbCR GetECDb() const { return m_ecdb; }
};

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct StaticPragmaResult final : public PragmaResult {
    private:
        int m_curRow;
        BeJsDocument m_doc;
        std::unique_ptr<BeJsValue> m_row;
        DbResult _Step()  override;
        DbResult _Reset() override;
        DbResult _Init() override;
        BeJsValue* _CurrentRow() override;

    public:
        explicit StaticPragmaResult(ECDbCR ecdb): PragmaResult(ecdb),m_curRow(-1) {}
        BeJsValue AppendRow();
        virtual ~StaticPragmaResult(){}
};

//=======================================================================================
//! PragmaECSqlPreparedStatement for ECSQL for pragma
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PragmaECSqlPreparedStatement final: public IECSqlPreparedStatement {
private:
    ECSqlParameterMap m_parameterMap;
    std::unique_ptr<PragmaResult> m_resultSet;
    IECSqlBinder& _GetBinder(int parameterIndex) const override;
    int _GetParameterIndex(Utf8CP parameterName) const override;
    int _TryGetParameterIndex(Utf8CP parameterName) const override;
    ECSqlStatus _ClearBindings() override;
    Utf8CP _GetNativeSql() const override { return ""; }

protected:
    ECSqlStatus _Prepare(ECSqlPrepareContext&, Exp const&) override;
    ECSqlStatus _Reset() override;

public:
    PragmaECSqlPreparedStatement(ECDb const& ecdb) : IECSqlPreparedStatement(ecdb, ECSqlType::Pragma, false) {}
    virtual ~PragmaECSqlPreparedStatement() {}
    DbResult DoStep();
    int GetColumnCount() const;
    IECSqlValue const& GetValue(int columnIndex) const;
    ECSqlParameterMap const& GetParameterMap() const { return m_parameterMap; }
    ECSqlParameterMap& GetParameterMapR() { return m_parameterMap; }
    DbResult Step();
};

END_BENTLEY_SQLITE_EC_NAMESPACE
