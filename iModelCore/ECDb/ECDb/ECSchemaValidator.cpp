/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSchemaValidator.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSchemaValidator.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2014
//---------------------------------------------------------------------------------------
//static
bool ECSchemaValidator::ValidateSchema (ECSchemaValidationResult& result, ECN::ECSchemaCR schema, bool supportLegacySchemas)
    {
    std::vector<std::unique_ptr<ECSchemaValidationRule>> validationTasks;
    validationTasks.push_back (std::move (std::unique_ptr<ECSchemaValidationRule> (new CaseInsensitiveClassNamesRule (supportLegacySchemas))));
    validationTasks.push_back(std::move(std::unique_ptr<ECSchemaValidationRule>(new MapStrategyRule())));
    // validationTasks.push_back (std::move (std::unique_ptr<ECSchemaValidationRule> (new StructWithRegularBaseClassRule (supportLegacySchemas))));
    bool valid = true;
    for (ECClassCP ecClass : schema.GetClasses ())
        {
        for (auto& task : validationTasks)
            {
            bool succeeded = task->ValidateSchema (schema, *ecClass);
            if (!succeeded)
                valid = false;
            }

        bool succeeded = ValidateClass (result, *ecClass, supportLegacySchemas);
        if (!succeeded)
            valid = false;
        }

    for (auto& task : validationTasks)
        {
        task->AddErrorToResult (result);
        }

    return valid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
//static
bool ECSchemaValidator::ValidateClass (ECSchemaValidationResult& result, ECN::ECClassCR ecClass, bool supportLegacySchemas)
    {
    std::vector<std::unique_ptr<ECSchemaValidationRule>> validationTasks;
    validationTasks.push_back (std::move (std::unique_ptr<ECSchemaValidationRule> (new CaseInsensitivePropertyNamesRule (ecClass, supportLegacySchemas))));
    validationTasks.push_back (std::move (std::unique_ptr<ECSchemaValidationRule> (new NoPropertiesOfSameTypeAsClassRule (ecClass))));

    bool valid = true;
    for (ECPropertyCP prop : ecClass.GetProperties (true))
        {
        for (auto& task : validationTasks)
            {
            bool succeeded = task->ValidateClass (ecClass, *prop);
            if (!succeeded)
                valid = false;
            }
        }

    for (auto& task : validationTasks)
        {
        task->AddErrorToResult (result);
        }

    return valid;
    }


//**********************************************************************
// ECSchemaValidationResult
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
void ECSchemaValidationResult::AddError (std::unique_ptr<ECSchemaValidationRule::Error> error)
    {
    m_errors.push_back (std::move (error));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
void ECSchemaValidationResult::ToString (std::vector<Utf8String>& errorMessages) const
    {
    for (auto& error : m_errors)
        {
        errorMessages.push_back (error->ToString ());
        }
    }


//**********************************************************************
// ECSchemaValidationRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
bool ECSchemaValidationRule::ValidateSchema (ECN::ECSchemaCR schema, ECN::ECClassCR ecClass)
    {
    return _ValidateSchema (schema, ecClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
bool ECSchemaValidationRule::ValidateClass (ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty)
    {
    return _ValidateClass (ecClass, ecProperty);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
void ECSchemaValidationRule::AddErrorToResult (ECSchemaValidationResult& result) const
    {
    auto error = _GetError ();
    if (error != nullptr)
        result.AddError (std::move (error));
    }

//**********************************************************************
// ECSchemaValidationRule::Error
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
Utf8String ECSchemaValidationRule::Error::ToString() const
    {
    return _ToString();
    }


//**********************************************************************
// MapStrategyRule
//**********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                Muhammad.zaighum                    04/2015
//---------------------------------------------------------------------------------------
MapStrategyRule::MapStrategyRule()
    : ECSchemaValidationRule(Type::InvalidMapStrategy), m_error(nullptr) 
    {
    m_error = std::unique_ptr<Error>(new Error(GetType()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Muhammad.zaighum                    04/2015
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSchemaValidationRule::Error> MapStrategyRule::_GetError() const
    {
    if (!m_error->IsError ())
        return nullptr;

    return std::move(m_error);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Muhammad.zaighum                    04/2015
//---------------------------------------------------------------------------------------
Utf8String MapStrategyRule::Error::_ToString() const
    {
    if (!IsError())
        return "";

    Utf8CP errorMsgTemplate = "ECSchema '%s' contains ECClasses that have a 'ClassMap' custom attribute with invalid values for the properties MapStrategy or MapStrategyOptions: %s.";
    Utf8String classList;
    ECClassCP firstClass = nullptr;
    for (ECClassCP ecClass : m_classesWithInvalidMapStrategy)
        {
        if (firstClass != nullptr)
            classList.append(", ");

        classList.append(Utf8String(ecClass->GetName()));
        firstClass = ecClass;
        }

    Utf8String error;
    error.Sprintf(errorMsgTemplate, Utf8String(firstClass->GetSchema().GetName()).c_str(), classList.c_str());
    return std::move(error);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Muhammad.zaighum                    04/2015
//---------------------------------------------------------------------------------------
bool MapStrategyRule::_ValidateSchema(ECN::ECSchemaCR schema, ECN::ECClassCR ecClass)
    {
    ECDbClassMap customClassMap;
    if (ECDbMapCustomAttributeHelper::TryGetClassMap(customClassMap, ecClass))
        {
        ECDbClassMap::MapStrategy mapStrategyCA;
        UserECDbMapStrategy strat;
        if (ECOBJECTS_STATUS_Success != customClassMap.TryGetMapStrategy(mapStrategyCA) ||
            SUCCESS != UserECDbMapStrategy::TryParse(strat, mapStrategyCA))
            {
            m_error->AddClassWithInvalidMapStrategy(ecClass);
            return false;
            }
        }

    return true;
    }


//**********************************************************************
// CaseInsensitiveClassNamesRule
//**********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
CaseInsensitiveClassNamesRule::CaseInsensitiveClassNamesRule (bool supportLegacySchemas) 
: ECSchemaValidationRule (Type::CaseInsensitiveClassNames), m_supportLegacySchemas (supportLegacySchemas), m_error (nullptr)
    {
    m_error = std::unique_ptr<Error> (new Error (GetType (), m_supportLegacySchemas));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
bool CaseInsensitiveClassNamesRule::_ValidateSchema (ECN::ECSchemaCR schema, ECN::ECClassCR ecClass)
    {
    bool valid = true;

    auto& invalidClasses = m_error->GetInvalidClassesR ();

    auto const& className = ecClass.GetName ();
    auto it = m_classNameSet.find (className.c_str ());
    if (it != m_classNameSet.end ()) //found case insensitive duplicate
        {
        auto& violatingClassBucket = invalidClasses[className.c_str ()];
        if (violatingClassBucket.empty ())
            {
            auto firstViolatingClass = schema.GetClassCP (*it);
            BeAssert (firstViolatingClass != nullptr);
            violatingClassBucket.insert (firstViolatingClass);
            }

        violatingClassBucket.insert (&ecClass);
        valid = false;
        }

    m_classNameSet.insert (className.c_str ());

    //In legacy support mode, this is not an error
    if (m_supportLegacySchemas)
        return true;

    return valid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSchemaValidationRule::Error> CaseInsensitiveClassNamesRule::_GetError () const
    {
    if (m_error->GetInvalidClasses ().empty ())
        return nullptr;

    return std::move (m_error);
    }



//**********************************************************************
// CaseInsensitiveClassNamesRule::Error
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
bset<ECN::ECClassCP> const* CaseInsensitiveClassNamesRule::Error::TryGetInvalidClasses (ECN::ECClassCR ecClass) const
    {
    auto classIt = m_invalidClasses.find (ecClass.GetName ().c_str ());
    if (classIt == m_invalidClasses.end ())
        return nullptr;

    BeAssert (!classIt->second.empty ());
    return &classIt->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
Utf8String CaseInsensitiveClassNamesRule::Error::_ToString () const
    {
    if (GetInvalidClasses ().empty ())
        return "";

    ECSchemaCP schema = nullptr;
    WString violatingClassesStr;
    bool isFirstSet = true;
    for (auto const& kvPair : GetInvalidClasses ())
        {
        if (!isFirstSet)
            violatingClassesStr.append (L" - ");

        bool isFirstClass = true;
        for (auto violatingClass : kvPair.second)
            {
            if (!isFirstClass)
                violatingClassesStr.append (L", ");
            else
                //capture schema (which is the same for all violating classes) for output reasons
                schema = &violatingClass->GetSchema ();

            violatingClassesStr.append (violatingClass->GetName ());
            isFirstClass = false;
            }

        isFirstSet = false;
        }


    Utf8CP strTemplate = nullptr;
    if (m_supportLegacySchemas)
        strTemplate = "ECSchema '%s' contains ECClasses for which names only differ by case. ECDb might have to skip some of these during the import (see further messages). Please try to fix the schema. Conflicting ECClasses: %s.";
    else
        strTemplate = "ECSchema '%s' contains ECClasses for which names only differ by case. ECDb does not support case sensitive class names. Conflicting ECClasses: %s.";

    Utf8String str;
    str.Sprintf (strTemplate, Utf8String (schema->GetName ()).c_str (), Utf8String (violatingClassesStr).c_str ());

    return std::move (str);
    }


//**********************************************************************
// CaseInsensitivePropertyNamesRule
//**********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
CaseInsensitivePropertyNamesRule::CaseInsensitivePropertyNamesRule (ECClassCR ecClass, bool supportLegacySchemas)
: ECSchemaValidationRule (Type::CaseInsensitivePropertyNames), m_supportLegacySchemas (supportLegacySchemas), m_error (nullptr)
    {
    m_error = std::unique_ptr<Error> (new Error (GetType (), ecClass, m_supportLegacySchemas));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
bool CaseInsensitivePropertyNamesRule::_ValidateClass (ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty)
    {
    bool valid = true;

    auto& invalidProperties = m_error->GetInvalidPropertiesR ();

    auto const& propName = ecProperty.GetName ();
    auto it = m_propertyNameSet.find (propName.c_str ());
    if (it != m_propertyNameSet.end ()) //found case insensitive duplicate
        {
        auto& violatingPropBucket = invalidProperties[propName.c_str ()];
        if (violatingPropBucket.empty ())
            {
            auto firstViolatingProp = ecClass.GetPropertyP (*it);
            BeAssert (firstViolatingProp != nullptr);
            violatingPropBucket.insert (firstViolatingProp);
            }

        violatingPropBucket.insert (&ecProperty);
        valid = false;
        }

    m_propertyNameSet.insert (propName.c_str ());

    //In legacy support mode, this is not an error
    if (m_supportLegacySchemas)
        return true;

    return valid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSchemaValidationRule::Error> CaseInsensitivePropertyNamesRule::_GetError () const
    {
    if (m_error->GetInvalidProperties ().empty ())
        return nullptr;

    return std::move (m_error);
    }


//**********************************************************************
// CaseInsensitivePropertyNamesRule::Error
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
Utf8String CaseInsensitivePropertyNamesRule::Error::_ToString () const
    {
    if (GetInvalidProperties ().empty ())
        return "";

    WString violatingPropsStr;

    bool isFirstSet = true;
    for (auto const& kvPair : GetInvalidProperties ())
        {
        if (!isFirstSet)
            violatingPropsStr.append (L" - ");

        bool isFirstProp = true;
        for (auto violatingProp : kvPair.second)
            {
            if (!isFirstProp)
                violatingPropsStr.append (L", ");

            violatingPropsStr.append (violatingProp->GetName ());
            isFirstProp = false;
            }

        isFirstSet = false;
        }

    Utf8CP strTemplate = nullptr;
    if (m_supportLegacySchemas)
        strTemplate = "ECClass '%s' contains ECProperties for which names only differ by case. ECDb might have to skip some of these during the import (see further messages). Please try to fix the schema. Conflicting ECProperties: %s.";
    else
        strTemplate = "ECClass '%s' contains ECProperties for which names only differ by case. ECDb does not support case sensitive property names. Conflicting ECProperties: %s.";

    Utf8String str;
    str.Sprintf (strTemplate, Utf8String (m_ecClass.GetFullName ()).c_str (), Utf8String (violatingPropsStr).c_str ());

    return std::move (str);
    }


//**********************************************************************
// NoPropertiesOfSameTypeAsClassRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
NoPropertiesOfSameTypeAsClassRule::NoPropertiesOfSameTypeAsClassRule (ECClassCR ecClass)
: ECSchemaValidationRule (Type::NoPropertiesOfSameTypeAsClass), m_error (nullptr)
    {
    m_error = std::unique_ptr<Error> (new Error (GetType (), ecClass));
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
bool NoPropertiesOfSameTypeAsClassRule::_ValidateClass (ECN::ECClassCR ecClass, ECN::ECPropertyCR ecProperty)
    {
    ECClassCP structType = nullptr;
    if (ecProperty.GetIsStruct ())
        structType = &ecProperty.GetAsStructProperty ()->GetType ();
    else if (ecProperty.GetIsArray ())
        {
        auto arrayProp = ecProperty.GetAsArrayProperty ();
        if (arrayProp->GetKind () == ARRAYKIND_Struct)
            structType = arrayProp->GetStructElementType ();
        }

    if (structType == nullptr)
        return true; //prop is of primitive type or prim array type -> no validation needed

    bool isValid = !structType->Is (&ecClass);
    if (!isValid)
        m_error->GetInvalidPropertiesR ().push_back (&ecProperty);

    return isValid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSchemaValidationRule::Error> NoPropertiesOfSameTypeAsClassRule::_GetError () const
    {
    if (m_error->GetInvalidProperties ().empty ())
        return nullptr;

    return std::move (m_error);
    }

//**********************************************************************
// NoPropertiesOfSameTypeAsClassRule::Error
//**********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    06/2014
//---------------------------------------------------------------------------------------
Utf8String NoPropertiesOfSameTypeAsClassRule::Error::_ToString () const
    {
    if (GetInvalidProperties ().empty ())
        return "";

    WString violatingPropsStr;

    bool isFirstProp = true;
    for (auto violatingProp : GetInvalidProperties ())
        {
        if (!isFirstProp)
            violatingPropsStr.append (L", ");

        violatingPropsStr.append (violatingProp->GetName ());
        isFirstProp = false;
        }

    Utf8CP strTemplate = "ECClass '%s' contains struct or array ECProperties which are of the same type or a derived type than the ECClass. Conflicting ECProperties: %s.";

    Utf8String str;
    str.Sprintf (strTemplate, Utf8String (m_ecClass.GetFullName ()).c_str (), Utf8String (violatingPropsStr).c_str ());

    return std::move (str);
    }

//**********************************************************************
// StructWithRegularBaseClassRule
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         05/2015
//---------------------------------------------------------------------------------------
StructWithRegularBaseClassRule::StructWithRegularBaseClassRule (bool supportLegacySchemas)
: ECSchemaValidationRule (Type::CaseInsensitiveClassNames), m_supportLegacySchemas (supportLegacySchemas), m_error (nullptr)
    {
    m_error = std::unique_ptr<Error> (new Error (GetType (), m_supportLegacySchemas));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         05/2015
//---------------------------------------------------------------------------------------
bool StructWithRegularBaseClassRule::_ValidateSchema (ECN::ECSchemaCR schema, ECN::ECClassCR ecClass)
    {
    bool valid = true;

    if (!ecClass.GetIsStruct ())
        return valid;

    auto& invalidClasses = m_error->GetInvalidClassesR ();
    for (auto baseClass : ecClass.GetBaseClasses ())
        {
        if (baseClass->GetIsStruct ())
            {
            invalidClasses [&ecClass].push_back (baseClass);
            if (valid) valid = false;
            }
        }

    //In legacy support mode, this is not an error
    if (m_supportLegacySchemas)
        return true;

    return valid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         05/2015
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSchemaValidationRule::Error> StructWithRegularBaseClassRule::_GetError () const
    {
    if (m_error->GetInvalidClasses ().empty ())
        return nullptr;

    return std::move (m_error);
    }

//**********************************************************************
// CaseInsensitiveClassNamesRule::Error
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         05/2015
//---------------------------------------------------------------------------------------
bvector<ECN::ECClassCP> const* StructWithRegularBaseClassRule::Error::TryGetInvalidBaseClasses (ECN::ECClassCR ecClass) const
    {
    auto classIt = m_invalidClasses.find (&ecClass);
    if (classIt == m_invalidClasses.end ())
        return nullptr;

    BeAssert (!classIt->second.empty ());
    return &classIt->second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Affan.Khan                         05/2015
//---------------------------------------------------------------------------------------
Utf8String StructWithRegularBaseClassRule::Error::_ToString () const
    {
    if (GetInvalidClasses ().empty ())
        return "";

    ECSchemaCP schema = nullptr;
    WString violatingClassesStr;
    for (auto const& kvPair : GetInvalidClasses ())
        {
        violatingClassesStr.append (kvPair.first->GetFullName ());
        violatingClassesStr.append (L" has following none struct base classes ( ");

        bool isFirstClass = true;
        for (auto violatingClass : kvPair.second)
            {
            if (!isFirstClass)
                violatingClassesStr.append (L", ");
            else
                //capture schema (which is the same for all violating classes) for output reasons
                schema = &violatingClass->GetSchema ();

            violatingClassesStr.append (violatingClass->GetName ());
            isFirstClass = false;
            }

        violatingClassesStr.append (L") ");
        }

    Utf8CP strTemplate = nullptr;
    if (m_supportLegacySchemas)
        strTemplate = "Struct classes should not have base class that is not a struct class. %s";
    else
        strTemplate = "ECDb does not support Struct classes that have at least one base class that is not a struct class. %s";;

    Utf8String str;
    str.Sprintf (strTemplate, Utf8String (violatingClassesStr).c_str ());

    return std::move (str);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
