/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbPolicyManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
struct ECDbPolicy
    {
    private:
        bool m_isSupported;
        Utf8String m_notSupportedMessage;

        ECDbPolicy(bool isSupported, Utf8CP notSupportedMessage) : m_isSupported(isSupported), m_notSupportedMessage(notSupportedMessage) {}

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
        Utf8CP GetNotSupportedMessage() const { return m_notSupportedMessage.c_str(); }

        //Factory
        //! Creates a 'Supported' policy
        static ECDbPolicy CreateSupported() { return ECDbPolicy(true, nullptr); }

        //! Creates a 'Not supported' policy
        static ECDbPolicy CreateNotSupported(Utf8CP notSupportedMessage = nullptr) { return ECDbPolicy(false, notSupportedMessage); }
    };

//=======================================================================================
//! Policy assertion for which a policy is to be requested by the ECDbPolicyManager
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECDbPolicyAssertion
    {
    public:
        //=======================================================================================
        //! Available policy assertions
        // @bsiclass                                                Krischan.Eberle      12/2013
        //+===============+===============+===============+===============+===============+======
        enum class Type
            {
            IsValidInECSql //!< @see IsValidInECSqlPolicyAssertion
            };

    private:
        virtual Type _GetType() const = 0;

    public:
        virtual ~ECDbPolicyAssertion() {}

        Type GetType() const { return _GetType(); }
    };

//=======================================================================================
//! Policy whether a given ECClass can be used in ECSQL or not.
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct IsValidInECSqlPolicyAssertion : ECDbPolicyAssertion
    {
    private:
        bool m_useECSqlTypeFilter;
        ECSqlType m_ecSqlTypeFilter;
        bool m_isPolymorphicClassExpression;

        static IsValidInECSqlPolicyAssertion s_noECSqlTypeFilterAssertionFlyweight;

        IsValidInECSqlPolicyAssertion()
            : m_useECSqlTypeFilter(false), m_isPolymorphicClassExpression(false)
            {}

        IsValidInECSqlPolicyAssertion(ECSqlType ecSqlTypeFilter, bool isPolymorphicClassExpression)
            : m_useECSqlTypeFilter(true), m_ecSqlTypeFilter(ecSqlTypeFilter), m_isPolymorphicClassExpression(isPolymorphicClassExpression)
            {}

        virtual ECDbPolicyAssertion::Type _GetType() const override { return Type::IsValidInECSql; }


    public:
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
        //! nott
        //! @remarks Don't call this if IsValidInECSqlPolicyAssertion::UseECSqlTypeFilter is false.
        //! @return true if the policy for a polymorphic class expression is requested. false otherwise
        bool IsPolymorphicClassExpression() const { return m_isPolymorphicClassExpression; }

        static ECDbPolicyAssertion const& Get();
        static IsValidInECSqlPolicyAssertion Get(ECSqlType ecSqlTypeFilter, bool isPolymorphicClassExpression);
    };


//=======================================================================================
//! Determines whether a given ECDb feature is supported in a given context.
// @bsiclass                                                Krischan.Eberle      12/2013
//+===============+===============+===============+===============+===============+======
struct ECDbPolicyManager
    {
    private:
        ECDbPolicyManager();
        ~ECDbPolicyManager();

        static ECDbPolicy DoGetClassPolicy(ClassMap const& classMap, IsValidInECSqlPolicyAssertion const& assertion);

    public:
        static ECDbPolicy GetClassPolicy(ClassMap const& classMap, ECDbPolicyAssertion const& assertion);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
