/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ClassMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Represents the information whether a given ECDb feature is supported or not
//! in a given context
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct Policy final
    {
    private:
        bool m_isSupported;
        Utf8String m_notSupportedMessage;

        explicit Policy(bool isSupported) : m_isSupported(isSupported) {}
        explicit Policy(Utf8StringCR notSupportedMessage) : m_isSupported(false), m_notSupportedMessage(notSupportedMessage) {}

    public:
        ~Policy() {}

        Policy(Policy const& rhs) : m_isSupported(rhs.m_isSupported), m_notSupportedMessage(rhs.m_notSupportedMessage) {}
        Policy& operator= (Policy const& rhs);
        Policy(Policy&& rhs) : m_isSupported(std::move(rhs.m_isSupported)), m_notSupportedMessage(std::move(rhs.m_notSupportedMessage)) {}
        Policy& operator= (Policy&& rhs);

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
        static Policy CreateSupported() { return Policy(true); }

        //! Creates a 'Not supported' policy
        static Policy CreateNotSupported() { return Policy(false); }
        static Policy CreateNotSupported(Utf8StringCR notSupportedMessage) { return Policy(notSupportedMessage); }
    };

//=======================================================================================
//! Policy assertion for which a policy is to be requested by the PolicyManager
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PolicyAssertion
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
        explicit PolicyAssertion(Type type) : m_type(type) {}

    public:
        virtual ~PolicyAssertion() {}

        Type GetType() const { return m_type; }
    };

//=======================================================================================
//! Policy whether a given ECClass can be used in ECSQL or not.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ClassIsValidInECSqlPolicyAssertion final : PolicyAssertion
    {
    private:
        ClassMap const& m_classMap;
        ECSqlType m_ecSqlTypeFilter;
        bool m_isPolymorphicClassExpression = false;

    public:
        ClassIsValidInECSqlPolicyAssertion(ClassMap const& classMap, ECSqlType ecSqlTypeFilter, bool isPolymorphicClassExpression)
            : PolicyAssertion(Type::ClassIsValidInECSql), m_classMap(classMap), m_ecSqlTypeFilter(ecSqlTypeFilter), m_isPolymorphicClassExpression(isPolymorphicClassExpression) {}

        ClassMap const& GetClassMap() const { return m_classMap; }

        //! Gets the ECSQL type for which the policy is requested.
        //! @return ECSQL type filter
        ECSqlType GetECSqlType() const { return m_ecSqlTypeFilter; }

        //! Gets a value indicating whether the policy is requested for a polymorphic class expression or
        //! not
        //! @return true if the policy for a polymorphic class expression is requested. false otherwise
        bool IsPolymorphicClassExpression() const { return m_isPolymorphicClassExpression; }
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ECCrudPermissionPolicyAssertion final : PolicyAssertion
    {
    private:
        ECDbCR m_ecdb;
        bool m_isWriteOperation;
        ECCrudWriteToken const* m_token;

    public:
        ECCrudPermissionPolicyAssertion(ECDbCR ecdb, bool isWriteOperation, ECCrudWriteToken const* token)
            : PolicyAssertion(PolicyAssertion::Type::ECCrudPermission), m_ecdb(ecdb), m_isWriteOperation(isWriteOperation), m_token(token)
            {}

        ECDbCR GetECDb() const { return m_ecdb; }
        bool IsWriteOperation() const { return m_isWriteOperation; }
        ECCrudWriteToken const* GetToken() const { return m_token; }
    };

//=======================================================================================
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct SchemaImportPermissionPolicyAssertion final : PolicyAssertion
    {
    private:
        ECDbCR m_ecdb;
        SchemaImportToken const* m_token;

    public:
        SchemaImportPermissionPolicyAssertion(ECDbCR ecdb, SchemaImportToken const* token)
            : PolicyAssertion(PolicyAssertion::Type::ECSchemaImportPermission), m_ecdb(ecdb), m_token(token)
            {}

        ECDbCR GetECDb() const { return m_ecdb; }
        SchemaImportToken const* GetToken() const { return m_token; }
    };

//=======================================================================================
//! Determines whether a given ECDb feature is supported in a given context.
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct PolicyManager final
    {
    private:
        PolicyManager();
        ~PolicyManager();

        static Policy DoGetPolicy(ClassIsValidInECSqlPolicyAssertion const&);
        static Policy DoGetPolicy(ECCrudPermissionPolicyAssertion const&);
        static Policy DoGetPolicy(SchemaImportPermissionPolicyAssertion const&);

    public:
        static Policy GetPolicy(PolicyAssertion const&);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
