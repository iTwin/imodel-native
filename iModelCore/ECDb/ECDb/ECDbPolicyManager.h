/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbPolicyManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "ClassMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Represents the information whether a given ECDb feature is supported or not
//! in a given context
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECDbPolicy final
    {
    private:
        bool m_isSupported;
        Utf8String m_notSupportedMessage;

        ECDbPolicy(bool isSupported) : m_isSupported(isSupported) {}
        ECDbPolicy(Utf8StringCR notSupportedMessage) : m_isSupported(false), m_notSupportedMessage(notSupportedMessage) {}

    public:
        ~ECDbPolicy() {}

        ECDbPolicy(ECDbPolicy const& rhs) : m_isSupported(rhs.m_isSupported), m_notSupportedMessage(rhs.m_notSupportedMessage) {}
        ECDbPolicy& operator= (ECDbPolicy const& rhs);
        ECDbPolicy(ECDbPolicy&& rhs) : m_isSupported(std::move(rhs.m_isSupported)), m_notSupportedMessage(std::move(rhs.m_notSupportedMessage)) {}
        ECDbPolicy& operator= (ECDbPolicy&& rhs);

        //! Gets a value indicating whether the policy is supported or not.
        //! @remarks If the policy is not supported, ECDbPolicy::GetNotSupportedMessage may contain
        //! an explanation.
        //! @return true, if the policy is supported, false otherwise.
        bool IsSupported() const { return m_isSupported; }

        //! Gets the reason why a policy is not supported in the given context
        //! @return Reason why policy is not supported. Returns nullptr if ECDbPolicy::IsSupported is true.
        Utf8StringCR GetNotSupportedMessage() const { return m_notSupportedMessage; }

        //Factory
        //! Creates a 'Supported' policy
        static ECDbPolicy CreateSupported() { return ECDbPolicy(true); }

        //! Creates a 'Not supported' policy
        static ECDbPolicy CreateNotSupported() { return ECDbPolicy(false); }
        static ECDbPolicy CreateNotSupported(Utf8StringCR notSupportedMessage) { return ECDbPolicy(notSupportedMessage); }
    };

//=======================================================================================
//! Policy assertion for which a policy is to be requested by the ECDbPolicyManager
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECDbPolicyAssertion
    {
    public:
        enum class Type
            {
            ClassIsValidInECSql,
            ECCrudPermission,
            ECSchemaImportPermission
            };

    private:
        Type m_type;

    protected:
        explicit ECDbPolicyAssertion(Type type) : m_type(type) {}

    public:
        virtual ~ECDbPolicyAssertion() {}

        Type GetType() const { return m_type; }
    };

//=======================================================================================
//! Policy whether a given ECClass can be used in ECSQL or not.
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ClassIsValidInECSqlPolicyAssertion final : ECDbPolicyAssertion
    {
    private:
        ClassMap const& m_classMap;
        bool m_useECSqlTypeFilter;
        ECSqlType m_ecSqlTypeFilter;
        bool m_isPolymorphicClassExpression;

    public:
        ClassIsValidInECSqlPolicyAssertion(ClassMap const& classMap, ECSqlType ecSqlTypeFilter, bool isPolymorphicClassExpression)
            : ECDbPolicyAssertion(Type::ClassIsValidInECSql), m_classMap(classMap), m_useECSqlTypeFilter(true), m_ecSqlTypeFilter(ecSqlTypeFilter), m_isPolymorphicClassExpression(isPolymorphicClassExpression)
            {}

        explicit ClassIsValidInECSqlPolicyAssertion(ClassMap const& classMap) 
                : ECDbPolicyAssertion(Type::ClassIsValidInECSql), m_classMap(classMap), m_useECSqlTypeFilter(false), m_isPolymorphicClassExpression(false)
            {}

        ClassMap const& GetClassMap() const { return m_classMap; }

        //! Gets a value indicating whether the requested policy should consider the ECSQL type filter or not.
        //! @remarks IsValidInECSqlPolicyAssertion::IsPolymorphicClassExpression is to be ignored, too, if the type filter is not considered.
        //! @return true if the policy is requested for the ECSQL type only specified by IsValidInECSqlPolicyAssertion::GetECSqlType.
        //!         false if the policy is requested for ignoring the ECSQL type filter.
        bool UseECSqlTypeFilter() const { return m_useECSqlTypeFilter; }

        //! Gets the ECSQL type for which the policy is requested.
        //! @remarks Don't call this if IsValidInECSqlPolicyAssertion::UseECSqlTypeFilter is false.
        //! @return ECSQL type filter
        ECSqlType GetECSqlType() const { return m_ecSqlTypeFilter; }

        //! Gets a value indicating whether the policy is requested for a polymorphic class expression or
        //! not
        //! @remarks Don't call this if IsValidInECSqlPolicyAssertion::UseECSqlTypeFilter is false.
        //! @return true if the policy for a polymorphic class expression is requested. false otherwise
        bool IsPolymorphicClassExpression() const { return m_isPolymorphicClassExpression; }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      11/2016
//+===============+===============+===============+===============+===============+======
struct ECCrudPermissionPolicyAssertion final : ECDbPolicyAssertion
    {
    private:
        ECDbCR m_ecdb;
        bool m_isWriteOperation;
        ECCrudWriteToken const* m_token;

    public:
        ECCrudPermissionPolicyAssertion(ECDbCR ecdb, bool isWriteOperation, ECCrudWriteToken const* token)
            : ECDbPolicyAssertion(ECDbPolicyAssertion::Type::ECCrudPermission), m_ecdb(ecdb), m_isWriteOperation(isWriteOperation), m_token(token)
            {}

        ECDbCR GetECDb() const { return m_ecdb; }
        bool IsWriteOperation() const { return m_isWriteOperation; }
        ECCrudWriteToken const* GetToken() const { return m_token; }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      12/2016
//+===============+===============+===============+===============+===============+======
struct ECSchemaImportPermissionPolicyAssertion final : ECDbPolicyAssertion
    {
    private:
        ECDbCR m_ecdb;
        ECSchemaImportToken const* m_token;

    public:
        ECSchemaImportPermissionPolicyAssertion(ECDbCR ecdb, ECSchemaImportToken const* token)
            : ECDbPolicyAssertion(ECDbPolicyAssertion::Type::ECSchemaImportPermission), m_ecdb(ecdb), m_token(token)
            {}

        ECDbCR GetECDb() const { return m_ecdb; }
        ECSchemaImportToken const* GetToken() const { return m_token; }
    };

//=======================================================================================
//! Determines whether a given ECDb feature is supported in a given context.
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECDbPolicyManager final
    {
    private:
        ECDbPolicyManager();
        ~ECDbPolicyManager();

        static ECDbPolicy DoGetPolicy(ClassIsValidInECSqlPolicyAssertion const&);
        static ECDbPolicy DoGetPolicy(ECCrudPermissionPolicyAssertion const&);
        static ECDbPolicy DoGetPolicy(ECSchemaImportPermissionPolicyAssertion const&);

    public:
        static ECDbPolicy GetPolicy(ECDbPolicyAssertion const&);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
