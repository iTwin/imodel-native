/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlBinder.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include <ECDb/IECSqlBinder.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#define ECSQLSYS_PARAM_FORMAT "_ecdb_ecsqlparam_ix%d"
#define ECSQLSYS_PARAM_Id "_ecdb_ecsqlparam_id"

#define ECSQLSYS_SQLPARAM_FORMAT "_ecdb_sqlparam_ix%d"
#define ECSQLSYS_SQLPARAM_Id "_ecdb_sqlparam_id"

struct ECSqlStatementBase;

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    08/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlBinder : IECSqlBinder
    {
    public:
        struct SqlParamNameGenerator final : NonCopyableClass
            {
        private:
            Utf8String m_nameRoot;
            int m_currentSuffix = 0;

        public:
            SqlParamNameGenerator(ECSqlPrepareContext& ctx, Utf8CP nameRoot) : m_nameRoot(nameRoot) 
                {
                if (m_nameRoot.empty())
                    m_nameRoot.Sprintf(ECSQLSYS_SQLPARAM_FORMAT, ctx.IncrementSystemSqlParameterSuffix());
                }

            Utf8String GetNextName()
                {
                BeAssert(!m_nameRoot.empty());
                //suffix is 1-based
                m_currentSuffix++;
                Utf8String nextName;
                nextName.Sprintf(":%s_col%d", m_nameRoot.c_str(), m_currentSuffix);
                return nextName;
                }
            };
    private:
        ECSqlStatementBase& m_ecsqlStatement;
        ECSqlTypeInfo m_typeInfo;
        std::vector<Utf8String> m_mappedSqlParameterNames;
        std::function<void(ECInstanceId bindValue)> m_onBindECInstanceIdEventHandler;
        std::unique_ptr<std::vector<IECSqlBinder*>> m_onBindEventHandlers = nullptr;
        bool m_hasToCallOnBeforeStep = false;
        bool m_hasToCallOnClearBindings = false;

        virtual ECSqlStatus _OnBeforeStep() { return ECSqlStatus::Success; }
        virtual void _OnClearBindings() {}

    protected:
        ECSqlBinder(ECSqlPrepareContext&, ECSqlTypeInfo const&, SqlParamNameGenerator&, int mappedSqlParameterCount, bool hasToCallOnBeforeStep, bool hasToCallOnClearBindings);
        //! Use this ctor for compound binders where the mapped sql parameter count depends on its member binders
        ECSqlBinder(ECSqlPrepareContext& ctx, ECSqlTypeInfo const& typeInfo, SqlParamNameGenerator& paramNameGen, bool hasToCallOnBeforeStep, bool hasToCallOnClearBindings) : ECSqlBinder(ctx, typeInfo, paramNameGen, -1, hasToCallOnBeforeStep, hasToCallOnClearBindings) {}

        void AddChildMemberMappedSqlParameterIndices(ECSqlBinder const& memberBinder)
            {
            std::vector<Utf8String> const& memberBinderMappedParameterNames = memberBinder.GetMappedSqlParameterNames();
            m_mappedSqlParameterNames.insert(m_mappedSqlParameterNames.end(), memberBinderMappedParameterNames.begin(), memberBinderMappedParameterNames.end());
            }

        std::function<void(ECInstanceId bindValue)> GetOnBindECInstanceIdEventHandler() const { return m_onBindECInstanceIdEventHandler; }
        std::vector<IECSqlBinder*>* GetOnBindEventHandlers() { return m_onBindEventHandlers.get(); }


        ECSqlStatus LogSqliteError(DbResult sqliteStat, Utf8CP errorMessageHeader = nullptr) const;

        Statement& GetSqliteStatementR() const;
        ECSqlStatementBase& GetECSqlStatementR() const { return m_ecsqlStatement; }
        ECDbCR GetECDb() const;
        static Statement::MakeCopy ToBeSQliteBindMakeCopy(IECSqlBinder::MakeCopy makeCopy);

    public:
        virtual ~ECSqlBinder() {}

        bool HasToCallOnBeforeStep() const { return m_hasToCallOnBeforeStep; }
        bool HasToCallOnClearBindings() const { return m_hasToCallOnClearBindings; }

        std::vector<Utf8String> const& GetMappedSqlParameterNames() const { return m_mappedSqlParameterNames; }

        ECSqlTypeInfo const& GetTypeInfo() const { return m_typeInfo; }

        ECSqlStatus OnBeforeStep() { return _OnBeforeStep(); }
        void OnClearBindings() { return _OnClearBindings(); }
        ECSqlStatus SetOnBindEventHandler(IECSqlBinder& binder);
        void SetOnBindECInstanceIdEventHandler(std::function<void(ECInstanceId bindValue)> eventHandler) { BeAssert(m_onBindECInstanceIdEventHandler == nullptr); m_onBindECInstanceIdEventHandler = eventHandler; }
    };

struct IdECSqlBinder;

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      08/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlBinderFactory
    {
    private:
        ECSqlBinderFactory();
        ~ECSqlBinderFactory();

        static bool RequiresNoopBinder(ECSqlPrepareContext&, PropertyMap const&, ECSqlSystemPropertyInfo const& sysPropertyInfo);
    public:
        static std::unique_ptr<ECSqlBinder> CreateBinder(ECSqlPrepareContext&, ECSqlTypeInfo const&, ECSqlBinder::SqlParamNameGenerator&);
        static std::unique_ptr<ECSqlBinder> CreateBinder(ECSqlPrepareContext&, ParameterExp const& parameterExp);
        static std::unique_ptr<ECSqlBinder> CreateBinder(ECSqlPrepareContext& ctx, PropertyMap const& propMap, ECSqlBinder::SqlParamNameGenerator& gen) { return CreateBinder(ctx, ECSqlTypeInfo(propMap), gen); }

        static std::unique_ptr<IdECSqlBinder> CreateIdBinder(ECSqlPrepareContext&, PropertyMap const&, ECSqlSystemPropertyInfo const&, ECSqlBinder::SqlParamNameGenerator&);

    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    03/2017
//+===============+===============+===============+===============+===============+======
struct ProxyECSqlBinder final : IECSqlBinder
    {
private:
    std::vector<IECSqlBinder*> m_binders;
    std::map<Utf8CP, std::unique_ptr<ProxyECSqlBinder>> m_structMemberProxyBinders;
    std::unique_ptr<ProxyECSqlBinder> m_arrayElementProxyBinder;

    ECSqlStatus _BindNull() override
        {
        for (IECSqlBinder* binder : m_binders)
            {
            ECSqlStatus stat = binder->BindNull();
            if (!stat.IsSuccess())
                return stat;
            }

        return ECSqlStatus::Success;
        }

    ECSqlStatus _BindBoolean(bool value) override
        {
        for (IECSqlBinder* binder : m_binders)
            {
            ECSqlStatus stat = binder->BindBoolean(value);
            if (!stat.IsSuccess())
                return stat;
            }

        return ECSqlStatus::Success;
        }

    ECSqlStatus _BindBlob(const void* value, int blobSize, IECSqlBinder::MakeCopy makeCopy) override
        {
        for (IECSqlBinder* binder : m_binders)
            {
            ECSqlStatus stat = binder->BindBlob(value, blobSize, makeCopy);
            if (!stat.IsSuccess())
                return stat;
            }

        return ECSqlStatus::Success;
        }

    ECSqlStatus _BindZeroBlob(int blobSize) override
        {
        for (IECSqlBinder* binder : m_binders)
            {
            ECSqlStatus stat = binder->BindZeroBlob(blobSize);
            if (!stat.IsSuccess())
                return stat;
            }

        return ECSqlStatus::Success;
        }

    ECSqlStatus _BindDateTime(double julianDay, DateTime::Info const& dtInfo) override
        {
        for (IECSqlBinder* binder : m_binders)
            {
            ECSqlStatus stat = binder->BindDateTime(julianDay, dtInfo);
            if (!stat.IsSuccess())
                return stat;
            }

        return ECSqlStatus::Success;
        }

    ECSqlStatus _BindDateTime(uint64_t julianDayMsec, DateTime::Info const& dtInfo) override
        {
        for (IECSqlBinder* binder : m_binders)
            {
            ECSqlStatus stat = binder->BindDateTime(julianDayMsec, dtInfo);
            if (!stat.IsSuccess())
                return stat;
            }

        return ECSqlStatus::Success;
        }

    ECSqlStatus _BindDouble(double value) override
        {
        for (IECSqlBinder* binder : m_binders)
            {
            ECSqlStatus stat = binder->BindDouble(value);
            if (!stat.IsSuccess())
                return stat;
            }

        return ECSqlStatus::Success;
        }

    ECSqlStatus _BindInt(int value) override
        {
        for (IECSqlBinder* binder : m_binders)
            {
            ECSqlStatus stat = binder->BindInt(value);
            if (!stat.IsSuccess())
                return stat;
            }

        return ECSqlStatus::Success;
        }

    ECSqlStatus _BindInt64(int64_t value) override
        {
        for (IECSqlBinder* binder : m_binders)
            {
            ECSqlStatus stat = binder->BindInt64(value);
            if (!stat.IsSuccess())
                return stat;
            }

        return ECSqlStatus::Success;
        }

    ECSqlStatus _BindPoint2d(DPoint2dCR value) override
        {
        for (IECSqlBinder* binder : m_binders)
            {
            ECSqlStatus stat = binder->BindPoint2d(value);
            if (!stat.IsSuccess())
                return stat;
            }

        return ECSqlStatus::Success;
        }
    ECSqlStatus _BindPoint3d(DPoint3dCR value) override
        {
        for (IECSqlBinder* binder : m_binders)
            {
            ECSqlStatus stat = binder->BindPoint3d(value);
            if (!stat.IsSuccess())
                return stat;
            }

        return ECSqlStatus::Success;
        }

    ECSqlStatus _BindText(Utf8CP value, IECSqlBinder::MakeCopy makeCopy, int byteCount) override
        {
        for (IECSqlBinder* binder : m_binders)
            {
            ECSqlStatus stat = binder->BindText(value, makeCopy, byteCount);
            if (!stat.IsSuccess())
                return stat;
            }

        return ECSqlStatus::Success;
        }

    IECSqlBinder& _BindStructMember(Utf8CP structMemberPropertyName) override
        {
        auto it = m_structMemberProxyBinders.find(structMemberPropertyName);
        if (it != m_structMemberProxyBinders.end())
            return *it->second;

        auto ret = m_structMemberProxyBinders.insert(std::make_pair(structMemberPropertyName, std::make_unique<ProxyECSqlBinder>()));
        ProxyECSqlBinder& memberProxyBinder = *ret.first->second;

        for (IECSqlBinder* binderP : m_binders)
            {
            IECSqlBinder& binder = *binderP;
            memberProxyBinder.AddBinder(binder[structMemberPropertyName]);
            }

        return memberProxyBinder;
        }

    IECSqlBinder& _BindStructMember(ECN::ECPropertyId structMemberPropertyId) override;

    IECSqlBinder& _AddArrayElement() override
        {
        m_arrayElementProxyBinder = std::make_unique<ProxyECSqlBinder>();
        
        for (IECSqlBinder* binderP : m_binders)
            {
            IECSqlBinder& binder = *binderP;
            m_arrayElementProxyBinder->AddBinder(binder.AddArrayElement());
            }

        return *m_arrayElementProxyBinder;
        }

 public:
    ProxyECSqlBinder() {}
    void AddBinder(IECSqlBinder& binder) { m_binders.push_back(&binder); }
    };

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    08/2013
//+===============+===============+===============+===============+===============+======
struct ECSqlParameterMap : NonCopyableClass
    {
    private:
        std::vector<std::unique_ptr<ECSqlBinder>> m_ownedBinders;
        std::vector<ECSqlBinder*> m_binders;
        std::vector<ECSqlBinder*> m_internalSqlParameterBinders;
        bmap<Utf8String, int, CompareIUtf8Ascii> m_nameToIndexMapping;

        std::vector<ECSqlBinder*> m_bindersToCallOnClearBindings;
        std::vector<ECSqlBinder*> m_bindersToCallOnStep;

        bool Contains(int& ecsqlParameterIndex, Utf8StringCR ecsqlParameterName) const;

    public:
        ECSqlParameterMap() {}
        ~ECSqlParameterMap() {}

        //! @remarks only named parameters have an identity. Therefore each unnamed parameters has its own binder
        bool TryGetBinder(ECSqlBinder*&, Utf8StringCR ecsqlParameterName) const;
        //!@param[in] ecsqlParameterIndex ECSQL parameter index (1-based)
        ECSqlStatus TryGetBinder(ECSqlBinder*&, int ecsqlParameterIndex) const;

        //!@param[in] internalBinderIndex Index of the internal binder as stored in the internal binder vector (0-based)
        ECSqlStatus TryGetInternalBinder(ECSqlBinder*&, size_t internalBinderIndex) const;

        //!@return ECSQL Parameter index (1-based) or -1 if index could not be found for @p ecsqlParameterName
        int GetIndexForName(Utf8StringCR ecsqlParameterName) const;

        ECSqlBinder* AddBinder(ECSqlPrepareContext&, ParameterExp const&);
        ECSqlBinder* AddInternalECInstanceIdBinder(ECSqlPrepareContext&);
        //@deprecated
        ECSqlBinder* AddProxyBinder(int ecsqlParameterIndex, ECSqlBinder&, Utf8StringCR parameterName);

        ECSqlStatus OnBeforeStep();

        //Bindings in SQLite have already been cleared at this point. The method
        //allows subclasses to clean-up additional resources tied to binding parameters
        void OnClearBindings();

        //@deprecated
        ECSqlStatus RemapForJoinTable(ECSqlPrepareContext&);

    };



//=======================================================================================
// @bsiclass                                                 Krischan.Eberle    07/2014
//+===============+===============+===============+===============+===============+======
struct ArrayConstraintValidator
    {
    private:
        ArrayConstraintValidator();
        ~ArrayConstraintValidator();

    public:
        static ECSqlStatus Validate(ECDbCR, ECSqlTypeInfo const& expected, uint32_t actualArrayLength);
        static ECSqlStatus ValidateMaximum(ECDbCR, ECSqlTypeInfo const& expected, uint32_t actualArrayLength);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE