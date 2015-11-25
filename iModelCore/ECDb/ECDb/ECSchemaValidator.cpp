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
bool ECSchemaValidator::ValidateSchemas(ECSchemaValidationResult& result, bvector<ECN::ECSchemaP> const& schemas, bool supportLegacySchemas)
    {
    std::vector<std::unique_ptr<ECSchemaValidationRule>> validationTasks;
    validationTasks.push_back(std::move(std::unique_ptr<ECSchemaValidationRule>(new SchemaNamespacePrefixRule())));

    bool valid = true;
    for (ECSchemaCP schema : schemas)
        {
        for (auto& task : validationTasks)
            {
            bool succeeded = task->ValidateSchemas(schemas, *schema);
            if (!succeeded)
                valid = false;
            }

        bool succeeded = ValidateSchema(result, *schema, supportLegacySchemas);
        if (!succeeded)
            valid = false;
        }

    for (auto& task : validationTasks)
        {
        task->AddErrorToResult(result);
        }

    return valid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    05/2014
//---------------------------------------------------------------------------------------
//static
bool ECSchemaValidator::ValidateSchema (ECSchemaValidationResult& result, ECN::ECSchemaCR schema, bool supportLegacySchemas)
    {
    std::vector<std::unique_ptr<ECSchemaValidationRule>> validationTasks;
    validationTasks.push_back(std::move(std::unique_ptr<ECSchemaValidationRule> (new CaseInsensitiveClassNamesRule (supportLegacySchemas))));
    validationTasks.push_back(std::move(std::unique_ptr<ECSchemaValidationRule>(new ValidRelationshipConstraintsRule())));
    validationTasks.push_back(std::move(std::unique_ptr<ECSchemaValidationRule>(new ConsistentClassHierarchyRule())));

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
// @bsimethod                                 Krischan.Eberle                    09/2015
//---------------------------------------------------------------------------------------
bool ECSchemaValidationRule::ValidateSchemas(bvector<ECN::ECSchemaP> const& schemas, ECN::ECSchemaCR schema)
    {
    return _ValidateSchemas(schemas, schema);
    }

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
    Utf8String violatingClassesStr;
    bool isFirstSet = true;
    for (auto const& kvPair : GetInvalidClasses ())
        {
        if (!isFirstSet)
            violatingClassesStr.append (" - ");

        bool isFirstClass = true;
        for (auto violatingClass : kvPair.second)
            {
            if (!isFirstClass)
                violatingClassesStr.append (", ");
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
    str.Sprintf (strTemplate, schema->GetName ().c_str (), violatingClassesStr.c_str ());

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

    Utf8String violatingPropsStr;

    bool isFirstSet = true;
    for (auto const& kvPair : GetInvalidProperties ())
        {
        if (!isFirstSet)
            violatingPropsStr.append (" - ");

        bool isFirstProp = true;
        for (auto violatingProp : kvPair.second)
            {
            if (!isFirstProp)
                violatingPropsStr.append (", ");

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
    str.Sprintf (strTemplate, m_ecClass.GetFullName (), violatingPropsStr.c_str ());

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

    Utf8String violatingPropsStr;

    bool isFirstProp = true;
    for (auto violatingProp : GetInvalidProperties ())
        {
        if (!isFirstProp)
            violatingPropsStr.append (", ");

        violatingPropsStr.append (violatingProp->GetName ());
        isFirstProp = false;
        }

    Utf8CP strTemplate = "ECClass '%s' contains struct or array ECProperties which are of the same type or a derived type than the ECClass. Conflicting ECProperties: %s.";

    Utf8String str;
    str.Sprintf (strTemplate, m_ecClass.GetFullName (), violatingPropsStr.c_str ());

    return std::move (str);
    }

//**********************************************************************
// RelationshipConstraintIsNotARelationshipClassRule
//**********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
ValidRelationshipConstraintsRule::ValidRelationshipConstraintsRule()
    : ECSchemaValidationRule(Type::ValidRelationshipConstraints), m_error(nullptr)
    {
    m_error = std::unique_ptr<Error>(new Error(GetType()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool ValidRelationshipConstraintsRule::_ValidateSchema(ECN::ECSchemaCR schema, ECN::ECClassCR ecClass)
    {
    ECRelationshipClassCP relClass = ecClass.GetRelationshipClassCP();
    if (relClass == nullptr)
        return true;

    const bool isAbstract = !relClass->GetIsDomainClass() && !relClass->GetIsStruct() && !relClass->GetIsCustomAttributeClass();
    return ValidateConstraint(*relClass, isAbstract, relClass->GetSource()) && ValidateConstraint(*relClass, isAbstract, relClass->GetTarget());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool ValidRelationshipConstraintsRule::ValidateConstraint(ECN::ECRelationshipClassCR relClass, bool isAbstractRelClass, ECN::ECRelationshipConstraintCR constraint) const
    {
    ECRelationshipConstraintClassList const& constraintClasses = constraint.GetConstraintClasses();
    const size_t constraintClassCount = constraintClasses.size();
    if (isAbstractRelClass)
        {
        if (constraintClassCount > 0)
            {
            //if rel is abstract, constraint must not have classes. if rel is not abstract, constraint must have classes
            m_error->AddInconsistency(relClass, Error::Kind::IsAbstractAndConstraintsAreDefined);
            return false;
            }
        }
    else
        {
        if (constraintClassCount == 0)
            {
            //if rel is not abstract, constraint must have classes
            m_error->AddInconsistency(relClass, Error::Kind::IncompleteConstraintDefinition);
            return false;
            }
        }

    bool valid = true;
    for (ECRelationshipConstraintClassCP constraintClass : constraintClasses)
        {
        ECClassCR constraintECClass = constraintClass->GetClass();
        if (IClassMap::IsAnyClass(constraintECClass))
            {
            m_error->AddInconsistency(relClass, Error::Kind::HasAnyClassConstraint);
            valid = false;
            }

        ECRelationshipClassCP relClassAsConstraint = constraintClass->GetClass().GetRelationshipClassCP();
        if (relClassAsConstraint != nullptr)
            {
            m_error->AddInconsistency(relClass, Error::Kind::HasRelationshipClassAsConstraint, relClassAsConstraint);
            valid = false;
            }
        }

    return valid;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSchemaValidationRule::Error> ValidRelationshipConstraintsRule::_GetError() const
    {
    if (!m_error->HasInconsistencies())
        return nullptr;

    return std::move(m_error);
    }


//**********************************************************************
// RelationshipConstraintIsNotARelationshipClassRule::Error
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
Utf8String ValidRelationshipConstraintsRule::Error::_ToString() const
    {
    if (!HasInconsistencies())
        return "";

    Utf8String str("Found ECRelationshipClasses with invalid constraint definitions. Conflicting ECRelationshipClasses: ");
    bool isFirstItem = true;
    for (Inconsistency const& inconsistency : m_inconsistencies)
        {
        if (!isFirstItem)
            str.append(" - ");

        str.append("Relationship ").append(inconsistency.m_relationshipClass->GetFullName()).append(":");
        
        const Kind kind = inconsistency.m_kind;

        if (Enum::Contains(kind, Kind::HasAnyClassConstraint))
            str.append(" AnyClass must not be used as constraint.");

        if (Enum::Contains(kind, Kind::HasRelationshipClassAsConstraint))
            {
            BeAssert(inconsistency.m_relationshipClassAsConstraintClass != nullptr);
            str.append(" The relationship class ").append(inconsistency.m_relationshipClassAsConstraintClass->GetFullName()).append(" is specified as constraint class which is not supported.");
            }

        if (Enum::Contains(kind, Kind::IncompleteConstraintDefinition))
            str.append(" At least one constraint definition is not complete.");

        if (Enum::Contains(kind, Kind::IsAbstractAndConstraintsAreDefined))
            str.append(" The relationship class is abstract and therefore constraints must not be defined (as they are not inherited anyways).");

        isFirstItem = false;
        }

    return std::move(str);
    }

//**********************************************************************
// ConsistentClassHierarchyRule
//**********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
ConsistentClassHierarchyRule::ConsistentClassHierarchyRule()
    : ECSchemaValidationRule(Type::ConsistentClassHierarchy), m_error(nullptr)
    {
    m_error = std::unique_ptr<Error>(new Error(GetType()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool ConsistentClassHierarchyRule::_ValidateSchema(ECN::ECSchemaCR schema, ECN::ECClassCR baseClass)
    {
    const ClassKind baseClassKind = DetermineClassKind(baseClass);

    bool valid = true;
    for (ECN::ECClassCP subclass : baseClass.GetDerivedClasses())
        {
        ClassKind subclassKind = DetermineClassKind(*subclass);
        const bool baseIsRelationship = baseClassKind == ClassKind::Relationship;
        const bool subIsRelationship = subclassKind == ClassKind::Relationship;
        if (baseIsRelationship != subIsRelationship ||
            (baseClassKind != ClassKind::Abstract && subclassKind != ClassKind::Abstract && baseClassKind != subclassKind))
            {
            m_error->AddInconsistency(baseClass, baseClassKind, *subclass, subclassKind);
            valid = false;
            }
        }

    return valid;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSchemaValidationRule::Error> ConsistentClassHierarchyRule::_GetError() const
    {
    if (!m_error->HasInconsistencies())
        return nullptr;

    return std::move(m_error);
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
//static
ConsistentClassHierarchyRule::ClassKind ConsistentClassHierarchyRule::DetermineClassKind(ECN::ECClassCR ecclass)
    {
    if (ecclass.GetRelationshipClassCP() != nullptr)
        return ClassKind::Relationship;

    if (ecclass.GetIsStruct())
        return ClassKind::Struct;

    if (ecclass.GetIsCustomAttributeClass())
        return ClassKind::CustomAttribute;

    if (!ecclass.GetIsDomainClass())
        return ClassKind::Abstract;

    return ClassKind::Regular;
    }

//**********************************************************************
// ConsistentClassHierarchyRule::Error
//**********************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
void ConsistentClassHierarchyRule::Error::AddInconsistency(ECN::ECClassCR baseClass, ClassKind baseClassKind, ECN::ECClassCR subclass, ClassKind subclassKind)
    {
    m_inconsistencies.push_back({InvalidClass(baseClass, baseClassKind), InvalidClass(subclass, subclassKind)});
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
Utf8String ConsistentClassHierarchyRule::Error::_ToString() const
    {
    if (!HasInconsistencies())
        return "";

    Utf8String str("Found ECClasses with a class hierarchy of inconsistent class types. Conflicting ECClasses: ");
    bool isFirstItem = true;
    for (std::pair<InvalidClass,InvalidClass> const& inconsistency : m_inconsistencies)
        {
        if (!isFirstItem)
            str.append("; ");

        str.append("(Base: ").append(inconsistency.first.ToString()).append(", Derived: ").append(inconsistency.second.ToString()).append(")");
        isFirstItem = false;
        }

    return std::move(str);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
Utf8String ConsistentClassHierarchyRule::Error::InvalidClass::ToString() const
    {
    Utf8CP kindStr = nullptr;
    switch (m_kind)
        {
            case ClassKind::Regular:
                kindStr = "Domain class";
                break;

            case ClassKind::Abstract:
                kindStr = "Abstract";
                break;

            case ClassKind::Struct:
                kindStr = "Struct";
                break;

            case ClassKind::CustomAttribute:
                kindStr = "CustomAttribute";
                break;

            case ClassKind::Relationship:
                kindStr = "Relationship";
                break;

            default:
                BeAssert(false);
                kindStr = "";
                break;
        }

    Utf8String str(m_class->GetFullName());
    str.append(" (").append(kindStr).append(")");
    return std::move(str);
    }

//**********************************************************************
// SchemaNamespacePrefixRule
//**********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
SchemaNamespacePrefixRule::SchemaNamespacePrefixRule()
    : ECSchemaValidationRule(Type::SchemaNamespacePrefix), m_error(nullptr)
    {
    m_error = std::unique_ptr<Error>(new Error(GetType()));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
bool SchemaNamespacePrefixRule::_ValidateSchemas(bvector<ECSchemaP> const& schemas, ECN::ECSchemaCR schema)
    {
    bool valid = true;
    Utf8StringCR prefix = schema.GetNamespacePrefix().c_str();
    if (prefix.empty() || !ECNameValidation::IsValidName(prefix.c_str()))
        {
        m_error->AddInvalidPrefix(schema);
        valid = false;
        }

    return valid;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
std::unique_ptr<ECSchemaValidationRule::Error> SchemaNamespacePrefixRule::_GetError() const
    {
    if (!m_error->HasInconsistencies())
        return nullptr;

    return std::move(m_error);
    }

//**********************************************************************
// SchemaNamespacePrefixRule::Error
//**********************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                    07/2015
//---------------------------------------------------------------------------------------
Utf8String SchemaNamespacePrefixRule::Error::_ToString() const
    {
    if (m_invalidNamespacePrefixes.empty())
        return "";

    Utf8String error("Found ECSchemas with invalid namespace prefixes: ");
    bool isFirstItem = true;
    for (ECSchemaCP schema : m_invalidNamespacePrefixes)
        {
        if (!isFirstItem)
            error.append("; ");

        Utf8String descr;
        descr.Sprintf("'%': Prefix '%s' is empty or not a valid EC name", schema->GetFullSchemaName().c_str(), schema->GetNamespacePrefix().c_str());
        error.append(descr);
        isFirstItem = false;
        }

    return std::move(error);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
