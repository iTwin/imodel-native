/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbPolicyManager.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECDbPolicyManager.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//********************* ECDbPolicy ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
ECDbPolicy::ECDbPolicy (bool isSupported, Utf8CP notSupportedMessage) 
    : m_isSupported (isSupported), m_notSupportedMessage (notSupportedMessage)
    {}
    
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
ECDbPolicy::ECDbPolicy (ECDbPolicy const& rhs)
    : m_isSupported (rhs.m_isSupported), m_notSupportedMessage (rhs.m_notSupportedMessage)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
ECDbPolicy& ECDbPolicy::operator= (ECDbPolicy const& rhs)
    {
    if (this != &rhs)
        {
        m_isSupported = rhs.m_isSupported;
        m_notSupportedMessage = rhs.m_notSupportedMessage;
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
ECDbPolicy::ECDbPolicy (ECDbPolicy&& rhs)
    : m_isSupported (std::move (rhs.m_isSupported)), m_notSupportedMessage (std::move (rhs.m_notSupportedMessage))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
ECDbPolicy& ECDbPolicy::operator= (ECDbPolicy&& rhs)
    {
    if (this != &rhs)
        {
        m_isSupported = std::move (rhs.m_isSupported);
        m_notSupportedMessage = std::move (rhs.m_notSupportedMessage);
        }

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
ECDbPolicy ECDbPolicy::CreateSupported ()
    {
    return ECDbPolicy (true, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
ECDbPolicy ECDbPolicy::CreateNotSupported (Utf8CP notSupportedMessage)
    {
    return ECDbPolicy (false, notSupportedMessage);
    }

//********************* ECDbPolicyAssertion ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
ECDbPolicyAssertion::Type ECDbPolicyAssertion::GetType () const
    {
    return _GetType ();
    }


//********************* IsValidInECSqlPolicyAssertion ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
IsValidInECSqlPolicyAssertion IsValidInECSqlPolicyAssertion::s_noECSqlTypeFilterAssertionFlyweight;

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
ECDbPolicyAssertion const& IsValidInECSqlPolicyAssertion::Get ()
    {
    return s_noECSqlTypeFilterAssertionFlyweight;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
IsValidInECSqlPolicyAssertion IsValidInECSqlPolicyAssertion::Get (ECSqlType ecSqlTypeFilter, bool isPolymorphicClassExpression)
    {
    return IsValidInECSqlPolicyAssertion (ecSqlTypeFilter, isPolymorphicClassExpression);
    }


//********************* ECDbPolicyManager ******************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
ECDbPolicy ECDbPolicyManager::GetClassPolicy (IClassMap const& classMap, ECDbPolicyAssertion const& assertion)
    {
    switch (assertion.GetType ())
        {
        case ECDbPolicyAssertion::Type::IsValidInECSql:
            return DoGetClassPolicy (classMap, static_cast<IsValidInECSqlPolicyAssertion const&> (assertion));

        default:
            return ECDbPolicy::CreateNotSupported ();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    12/2013
//---------------------------------------------------------------------------------------
//static
ECDbPolicy ECDbPolicyManager::DoGetClassPolicy (IClassMap const& classMap, IsValidInECSqlPolicyAssertion const& assertion)
    {
    auto const& ecClass = classMap.GetClass ();
    auto const& className = ecClass.GetName ();
    //generally not supported - regardless of ECSqlType
    if (classMap.GetMapStrategy().IsNotMapped ())
        {
        Utf8String notSupportedMessage;
        notSupportedMessage.Sprintf ("ECClass '%s' is not supported in ECSQL as it was not mapped to a table."
                        " The ECClass might have been marked with 'NotMapped' in the ECSchema or is generally not supported by ECDb."
                        " In that case, please see the log for details about why the class was not mapped.", 
                        Utf8String (className).c_str ());
        return ECDbPolicy::CreateNotSupported (notSupportedMessage.c_str ());
        }

    BeAssert (!ecClass.GetSchema ().IsStandardSchema () || (!className.Equals ("AnyClass") && !className.Equals ("InstanceCount")) && "AnyClass or InstanceCount class should already be caught by IsNotMapped check.");

    //if policy for specific ECSQL type was requested, check that now
    if (assertion.UseECSqlTypeFilter ())
        {
        // currently, polymorphism doesn't affect the policies.

        switch (assertion.GetECSqlType ())
            {
                case ECSqlType::Insert:
                    {
                    //Inserting into abstract classes is not possible (by definition of abstractness)
                    if (ecClass.GetClassModifier() == ECClassModifier::Abstract)
                        return ECDbPolicy::CreateNotSupported ("ECClass is an abstract class which is not instantiable and therefore cannot be used in an ECSQL INSERT statement.");

                    //Inserting into structs is not possible (by definition structs are not instantiable)
                    if (ecClass.IsStructClass())
                        return ECDbPolicy::CreateNotSupported ("ECClass is a struct which is not instantiable and therefore cannot be used in an ECSQL INSERT statement.");
                    }
            }
        }

    return ECDbPolicy::CreateSupported ();
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
