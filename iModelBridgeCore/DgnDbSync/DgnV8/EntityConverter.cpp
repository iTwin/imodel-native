/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/EntityConverter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE
using namespace BeSQLite::EC;
using namespace BECN;

//****************************************************************************************
// BisClassConverter
//****************************************************************************************
#define DGNDBSYNCV8_ECSCHEMA_NAME "DgnDbSyncV8"
#define CUSTOMCLASSMAP_CLASSNAME "ClassMap"

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
//static
ECClassName BisClassConverter::GetElementBisBaseClassName(BisConversionRule conversionRule)
    {
    switch (conversionRule)
        {
        case BisConversionRule::ToSpatialLocationElement:
            return ECClassName(GENERIC_DOMAIN_NAME, GENERIC_CLASS_SpatialLocation);
        case BisConversionRule::ToPhysicalObject:
        case BisConversionRule::ToDefaultBisClass:
            return ECClassName(GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject);
        case BisConversionRule::ToDefaultBisBaseClass:
        case BisConversionRule::ToPhysicalElement:
            return ECClassName(BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalElement);
        case BisConversionRule::ToDrawingGraphic:
            return ECClassName(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic);
        case BisConversionRule::ToGroup:
            return ECClassName(BIS_ECSCHEMA_NAME, BIS_CLASS_GroupInformationElement);
        case BisConversionRule::ToGenericGroup:
            return ECClassName(GENERIC_DOMAIN_NAME, GENERIC_CLASS_Group);
        default:
            BeAssert(false);
            return ECClassName();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
//static
ECClassName BisClassConverter::GetElementAspectBisBaseClassName(BisConversionRule conversionRule)
    {
    BisConversionRuleHelper::ElementAspectKind aspectKind;
    if (BSISUCCESS != BisConversionRuleHelper::TryDetermineElementAspectKind(aspectKind, conversionRule))
        return ECClassName();

    // This switch is currently not very useful since TryDetermineElementAspectKind will only return ElementMultiAspect
    switch (aspectKind)
        {
        case BisConversionRuleHelper::ElementAspectKind::ElementAspect:
            //return ECClassName(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementAspect);
        case BisConversionRuleHelper::ElementAspectKind::ElementMultiAspect:
            return ECClassName(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect);

        default:
            BeAssert(false);
            return ECClassName();
        }
    }

#define ElementAspectSuffix BIS_CLASS_ElementAspect

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
//static
Utf8CP BisClassConverter::GetAspectClassSuffix(BisConversionRule conversionRule)
    {
    BisConversionRuleHelper::ElementAspectKind aspectKind;
    if (BSISUCCESS != BisConversionRuleHelper::TryDetermineElementAspectKind(aspectKind, conversionRule))
        return nullptr;

    switch (aspectKind)
        {
        case BisConversionRuleHelper::ElementAspectKind::ElementMultiAspect:
        case BisConversionRuleHelper::ElementAspectKind::ElementAspect:
            return ElementAspectSuffix;

        default:
            BeAssert(false);
            return nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisClassConverter::CheckBaseAndDerivedClassesForBisification(SchemaConversionContext& context, ECN::ECClassCP childClass, BisConversionRule childRule, bvector<ECClassP> classes, bool isBaseClassCheck)
    {
    Converter& converter = context.GetConverter();
    for (BECN::ECClassP ecClass : classes)
        {
        ECClassName v8ClassName(*ecClass);
        BisConversionRule existingRule;
        BECN::ECClassId existingV8ClassId;
        bool hasSecondary;
        if (V8ECClassInfo::TryFind(existingV8ClassId, existingRule, context.GetDgnDb(), v8ClassName, hasSecondary))
            {
            if (BisConversionRule::ToDefaultBisBaseClass == existingRule && BisConversionRule::ToDefaultBisBaseClass != childRule)
                {
                if (BSISUCCESS != V8ECClassInfo::Update(converter, existingV8ClassId, childRule))
                    return BSIERROR;
                }
            // If a class is using the ToAspectOnly rule, all of its children must also become aspects
            else if (!isBaseClassCheck && BisConversionRule::ToAspectOnly == childRule)
                {
                if (BSISUCCESS != V8ECClassInfo::Update(converter, existingV8ClassId, childRule))
                    return BSIERROR;
                }
            else if (!isBaseClassCheck && ((BisConversionRule::ToDrawingGraphic == existingRule && BisConversionRule::ToPhysicalElement == childRule) ||
                                          (BisConversionRule::ToPhysicalElement == existingRule && BisConversionRule::ToDrawingGraphic == childRule)))
                {
                if (BSISUCCESS != V8ECClassInfo::Update(converter, existingV8ClassId, BisConversionRule::TransformedUnbisifiedAndIgnoreInstances))
                    return BSIERROR;
                }
            }
        else if (BSISUCCESS != V8ECClassInfo::Insert(converter, v8ClassName, childRule))
            return BSIERROR;
        if (BSISUCCESS != CheckBaseAndDerivedClassesForBisification(context, ecClass, childRule, isBaseClassCheck ? ecClass->GetBaseClasses() : ecClass->GetDerivedClasses(), isBaseClassCheck))
            return BSIERROR;
        }
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisClassConverter::EnsureBaseClassesAndDerivedClassesAreSet(SchemaConversionContext& context, bool isBaseClassCheck)
    {
    for (bpair<Utf8String, BECN::ECSchemaP> const& kvpair : context.GetSchemas())
        {
        BECN::ECSchemaP schema = kvpair.second;
        //only interested in the domain schemas, so skip standard, system and supp schemas
        if (context.ExcludeSchemaFromBisification(*schema))
            continue;

        if (schema->GetAlias().empty())
            {
            schema->SetAlias(schema->GetName());
            context.ReportIssue(Converter::IssueSeverity::Warning, "ECSchema '%s' doesn't have an alias. Set it to the schema name.", schema->GetFullSchemaName().c_str());
            }

        for (BECN::ECClassP v8Class : schema->GetClasses())
            {
            ECClassName v8ClassName(*v8Class);

            BisConversionRule existingRule;
            BECN::ECClassId existingV8ClassId;
            bool hasSecondary;
            const bool alreadyExists = V8ECClassInfo::TryFind(existingV8ClassId, existingRule, context.GetDgnDb(), v8ClassName, hasSecondary);
            BeAssert(alreadyExists);
            if (BSISUCCESS != CheckBaseAndDerivedClassesForBisification(context, v8Class, existingRule, isBaseClassCheck ? v8Class->GetBaseClasses() : v8Class->GetDerivedClasses(), isBaseClassCheck))
                return BSIERROR;
            }
        }
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     04/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisClassConverter::PreprocessConversion(SchemaConversionContext& context)
    {
    Converter& converter = context.GetConverter();
    ECClassRemovalContext removeContext(context);
    for (bpair<Utf8String, BECN::ECSchemaP> const& kvpair : context.GetSchemas())
        {
        BECN::ECSchemaP schema = kvpair.second;
        //only interested in the domain schemas, so skip standard, system and supp schemas
        if (context.ExcludeSchemaFromBisification(*schema))
            continue;

        if (schema->GetAlias().empty())
            {
            schema->SetAlias(schema->GetName());
            context.ReportIssue(Converter::IssueSeverity::Warning, "ECSchema '%s' doesn't have an alias. Set it to the schema name.", schema->GetFullSchemaName().c_str());
            }

        for (BECN::ECClassP v8Class : schema->GetClasses())
            {
            ECClassName v8ClassName(*v8Class);

            BisConversionRule existingRule;
            BECN::ECClassId existingV8ClassId;
            bool hasSecondary;
            const bool alreadyExists = V8ECClassInfo::TryFind(existingV8ClassId, existingRule, context.GetDgnDb(), v8ClassName, hasSecondary);

            //Determine class based rule. This also evaluates the ConversionRule CA on the class, if it exists
            BisConversionRule rule;
            if (BSISUCCESS != BisConversionRuleHelper::ConvertToBisConversionRule(rule, converter, *v8Class))
                return BSIERROR;

            if (alreadyExists && (rule == existingRule || BisConversionRuleHelper::ClassNeedsBisification(rule)))
                continue; //no update or further actions needed

            bool needsV8ECClassInfoInsertOrUpdate = true;
            switch (rule)
                {
                case BisConversionRule::Ignored:
                    {
                    removeContext.AddClassToRemove(*v8Class);
                    needsV8ECClassInfoInsertOrUpdate = false;
                    break;
                    }

                case BisConversionRule::IgnoredPolymorphically:
                    {
                    removeContext.AddClassToRemove(*v8Class, true);
                    needsV8ECClassInfoInsertOrUpdate = false;
                    break;
                    }

                default:
                    break;
                }

            if (needsV8ECClassInfoInsertOrUpdate)
                {
                if (alreadyExists)
                    {
                    BeAssert(rule != existingRule);
                    if (BSISUCCESS != V8ECClassInfo::Update(converter, existingV8ClassId, rule))
                        return BSIERROR;
                    }
                else
                    {
                    if (BSISUCCESS != V8ECClassInfo::Insert(converter, v8ClassName, rule))
                        return BSIERROR;
                    }
                }
            }
        }

    StopWatch timer(true);
    if (BSISUCCESS != EnsureBaseClassesAndDerivedClassesAreSet(context, true))
        return BSIERROR;
    if (BSISUCCESS != EnsureBaseClassesAndDerivedClassesAreSet(context, false))
        return BSIERROR;
    if (BSISUCCESS != EnsureBaseClassesAndDerivedClassesAreSet(context, true))
        return BSIERROR;
    ConverterLogging::LogPerformance(timer, "BaseClass/DerivedClass cleanup");

    if (BSISUCCESS != removeContext.FixClassHierarchies())
        return BSIERROR;

    for (BECN::ECClassP ignoredClass : removeContext.GetClasses())
        {
        if (BECN::ECObjectsStatus::Success != ignoredClass->GetSchemaR().DeleteClass(*ignoredClass))
            return BSIERROR;
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisClassConverter::ConvertECClass(SchemaConversionContext& context, ECClassName const& v8ClassName)
    {
    return ConvertECClass(context, v8ClassName, nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisClassConverter::ConvertECClass(SchemaConversionContext& context, ECClassName const& v8ClassName, BisConversionRule const* parentBisConversionRuleCP)
    {
    BisConversionRule conversionRule;
    bool found = false;
    bool hasSecondary;
    if (!V8ECClassInfo::TryFind(conversionRule, context.GetDgnDb(), v8ClassName, hasSecondary))
        {
        Utf8String str(v8ClassName.GetClassName());
        if (str.EndsWith(ElementAspectSuffix))
            {
            if (1 == str.ReplaceAll(ElementAspectSuffix, ""))
                {
                ECClassName tmp(v8ClassName.GetSchemaName(), str.c_str());
                if (V8ECClassInfo::TryFind(conversionRule, context.GetDgnDb(), tmp, hasSecondary))
                    {
                    found = true;
                    }
                }
            }
        }
    else
        found = true;

    if (!found)
        {
        BeAssert(false);
        return BSIERROR;
        }

    if (parentBisConversionRuleCP != nullptr)
        {
        //Use parent rule, if it is more derived than this rule
        BisConversionRule parentRule = *parentBisConversionRuleCP;
        if (parentRule != BisConversionRule::ToDefaultBisBaseClass && conversionRule == BisConversionRule::ToDefaultBisBaseClass)
            conversionRule = parentRule;

        // if parent rule conflicts with child rule, use parent rule
        if (parentRule == BisConversionRule::ToPhysicalElement && conversionRule == BisConversionRule::ToDrawingGraphic)
            conversionRule = parentRule;
        else if (parentRule == BisConversionRule::ToDrawingGraphic && conversionRule == BisConversionRule::ToPhysicalElement)
            conversionRule = parentRule;
        }

    BECN::ECClassP inputClass = context.GetInputClass(v8ClassName.GetSchemaName(), v8ClassName.GetClassName());
    if (BSISUCCESS != DoConvertECClass(context, conversionRule, *inputClass, v8ClassName, hasSecondary))
        return BSIERROR;

    for (BECN::ECClassP childClass : inputClass->GetDerivedClasses())
        {
        ECClassName childClassName(*childClass);
        if (BSISUCCESS != ConvertECClass(context, childClassName, &conversionRule))
            return BSIERROR;
        }

    return BSISUCCESS;
    }



//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisClassConverter::DoConvertECClass(SchemaConversionContext& context, BisConversionRule bisConversionRule, BECN::ECClassR inputClass, ECClassName const& v8ClassName, bool hasSecondary)
    {
    //need to remove ECDbHints form original schema before injecting BIS base classes as otherwise we would remove the hint from the BIS base class
    RemoveDuplicateClassMapCustomAttributes(inputClass);

    //v8 apps never enforced array bounds, but in DgnDb we will now. To avoid legacy issues we just set the bounds to unlimited
    //in the BISified schema which semantically matches what it should have been in v8 and therefore doesn't cause issues in the strict DgnDb world.
    // Also need to ensure that none of the class's properties have a name conflict with an ECDb reserved property name
    if (BSISUCCESS != ValidateClassProperties(context, inputClass))
        return BSIERROR;

    if (context.IsAlreadyConverted(inputClass) || !BisConversionRuleHelper::ClassNeedsBisification(bisConversionRule))
        return BSISUCCESS;

    BECN::ECSchemaR targetSchema = inputClass.GetSchemaR();

    BECN::ECClassCP elementBisBaseClass = nullptr;
    BECN::ECClassCP elementAspectBisBaseClass = nullptr;
    GetBisBaseClasses(elementBisBaseClass, elementAspectBisBaseClass, context, bisConversionRule);

    //Class hierarchy for aspects will be established after schema conversion to not collide with moving properties from properties to aspect classes
    BECN::ECClassP elementAspectClass = &inputClass;
    if (bisConversionRule != BisConversionRule::ToAspectOnly)
        {
        //Original v8 ECClass becomes Element subclass
        AddBaseClass(inputClass, *elementBisBaseClass);
        }
    bool toAspect = bisConversionRule == BisConversionRule::ToAspectOnly || hasSecondary;

    if (toAspect)
        {
        //create ElementAspect class
        Utf8String elementAspectClassName(inputClass.GetName());
        Utf8CP aspectSuffix = GetAspectClassSuffix(BisConversionRule::ToAspectOnly);
        BeAssert(!Utf8String::IsNullOrEmpty(aspectSuffix));
        elementAspectClassName.append(aspectSuffix);

        if (!Utf8String::IsNullOrEmpty(aspectSuffix))
            {
            if (!inputClass.GetIsDisplayLabelDefined())
                inputClass.SetDisplayLabel(inputClass.GetDisplayLabel());
                
            //AspectOnly -> secondary instances. They don't get an element, so we take the original v8 class and just rename it
            //to aspect naming convention.
            if (BisConversionRule::ToAspectOnly == bisConversionRule) 
                {
                if (ECN::ECObjectsStatus::Success != targetSchema.RenameClass(inputClass, elementAspectClassName.c_str()))
                    {
                    context.ReportIssue(Converter::IssueSeverity::Fatal, "ECSchema BISification failed. Could not rename v8 ECClass '%s' to ElementAspect class '%s' in target schema.",
                                        inputClass.GetFullName(),
                                        elementAspectClassName.c_str());
                    return BSIERROR;
                    }
                }
            // Otherwise, we need to have both the regular class and an aspect class
            else
                {
                elementAspectClass = nullptr;
                ECN::ECObjectsStatus stat = targetSchema.CopyClass(elementAspectClass, inputClass, elementAspectClassName);
                if (ECN::ECObjectsStatus::Success != stat || nullptr == elementAspectClass)
                    {
                    context.ReportIssue(Converter::IssueSeverity::Error, "ECSchema BISification failed.  Could not create a copy of %s as an aspect class.  Secondary instances of this class will not be created.", inputClass.GetFullName());
                    return BSIERROR;
                    }
                }
            }
        BeAssert(elementAspectBisBaseClass != nullptr);
        }

    if (toAspect)
        return context.AddClassMapping(inputClass, *elementAspectClass, toAspect, *elementAspectBisBaseClass);
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     04/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisClassConverter::ConvertECRelationshipClass(SchemaConversionContext& context, ECN::ECRelationshipClassR inputClass, ECN::ECSchemaReadContextP syncContext)
    {
    RemoveDuplicateClassMapCustomAttributes(inputClass);

    ProcessConstraints(inputClass, context.GetDefaultConstraintClass(), context);

    bool ignoreBisBase = false;
    ECN::SchemaKey conversionKey(Utf8String(inputClass.GetSchema().GetName().c_str()).append("_DgnDbSync").c_str(), 1, 0);
    ECN::ECSchemaPtr conversionSchema = syncContext->LocateSchema(conversionKey, ECN::SchemaMatchType::Latest);

    if (conversionSchema.IsValid())
        {
        ECN::ECClassCP ecClass = conversionSchema->GetClassCP(inputClass.GetName().c_str());
        if (nullptr != ecClass)
            ignoreBisBase = ecClass->GetCustomAttribute("ForceIgnoreRelationshipBISBaseClass") != nullptr;
        }

    if (!ignoreBisBase)
        {
        BECN::ECRelationshipClassCP baseClass = context.GetDomainRelationshipBaseClass(inputClass);
        if (baseClass == nullptr)
            {
            context.ReportIssue(Converter::IssueSeverity::Info, "Unable to bis-ify ECRelationshipClass '%s'.", inputClass.GetFullName());
            if (inputClass.GetStrength() != BECN::StrengthType::Referencing)
                {
                Utf8CP oldStrengthStr = inputClass.GetStrength() == BECN::StrengthType::Embedding ? "Embedding" : "Holding";
                context.ReportIssue(Converter::IssueSeverity::Warning, "BISIfication required changing relationship strength of ECRelationshipClass '%s' from '%s' to 'Referencing'.",
                                    inputClass.GetFullName(), oldStrengthStr);
                inputClass.SetStrength(StrengthType::Referencing);
                }
            }
        else
            {
            // The source multiplicity has to be set based on the base class
            inputClass.GetSource().SetMultiplicity(baseClass->GetSource().GetMultiplicity());

            if (inputClass.GetStrength() != baseClass->GetStrength())
                {
                Utf8CP oldStrengthStr = nullptr;
                BECN::StrengthType oldStrength = inputClass.GetStrength();
                if (oldStrength == BECN::StrengthType::Referencing)
                    oldStrengthStr = "Referencing";
                else if (oldStrength == BECN::StrengthType::Embedding)
                    oldStrengthStr = "Embedding";
                else
                    oldStrengthStr = "Holding";

                BECN::StrengthType newStrength = baseClass->GetStrength();
                Utf8CP newStrengthStr = newStrength == BECN::StrengthType::Referencing ? "Referencing" : "Embedding";

                context.ReportIssue(Converter::IssueSeverity::Warning, "BISIfication required changing relationship strength of ECRelationshipClass '%s' from '%s' to '%s'.",
                                    inputClass.GetFullName(), oldStrengthStr, newStrengthStr);

                inputClass.SetStrength(newStrength);
                }

            if (inputClass.GetStrengthDirection() != baseClass->GetStrengthDirection())
                {
                BECN::ECRelatedInstanceDirection newStrength = baseClass->GetStrengthDirection();
                context.ReportIssue(Converter::IssueSeverity::Warning, "BISIfication required changing relationship strength direction of ECRelationshipClass '%s' from '%s' to '%s'.",
                                    inputClass.GetFullName(), (inputClass.GetStrengthDirection() == ECRelatedInstanceDirection::Forward) ? "Forward" : "Backward",
                                    newStrength == ECRelatedInstanceDirection::Forward ? "Forward" : "Backward");

                inputClass.SetStrengthDirection(newStrength);
                }

            if (SUCCESS != AddBaseClass(inputClass, *baseClass))
                return BSIERROR;
            }
        }
    else
        {
        if (inputClass.GetStrength() != BECN::StrengthType::Referencing)
            {
            Utf8CP oldStrengthStr = inputClass.GetStrength() == BECN::StrengthType::Embedding ? "Embedding" : "Holding";
            context.ReportIssue(Converter::IssueSeverity::Warning, "BISIfication required changing relationship strength of ECRelationshipClass '%s' from '%s' to 'Referencing'.",
                                inputClass.GetFullName(), oldStrengthStr);
            inputClass.SetStrength(StrengthType::Referencing);
            }
        }

    for (BECN::ECClassP childClass : inputClass.GetDerivedClasses())
        {
        BECN::ECRelationshipClassP relClass = childClass->GetRelationshipClassP();
        ECClassName childClassName(*childClass);
        if (BSISUCCESS != ConvertECRelationshipClass(context, *relClass, syncContext))
            return BSIERROR;
        }
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2016
//---------------+---------------+---------------+---------------+---------------+-------
void BisClassConverter::ProcessConstraints(BECN::ECRelationshipClassR inputClass, BECN::ECEntityClassP defaultConstraintClass, SchemaConversionContext& context)
    {
    ConvertECRelationshipConstraint(inputClass.GetSource(), inputClass, defaultConstraintClass, context, true);
    ConvertECRelationshipConstraint(inputClass.GetTarget(), inputClass, defaultConstraintClass, context, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
//static
bool ContainsAnyClass(BECN::ECRelationshipConstraintClassList const& constraintClassList)
    {
    for (BECN::ECClassCP constraintClass : constraintClassList)
        {
        if (constraintClass->GetSchema().IsStandardSchema() && constraintClass->GetName().EqualsI("AnyClass"))
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
void BisClassConverter::ConvertECRelationshipConstraint(BECN::ECRelationshipConstraintR constraint, BECN::ECRelationshipClassR relClass, BECN::ECEntityClassP defaultConstraintClass, SchemaConversionContext& context, bool isSource)
    {
    // The source multiplicity will be set after a base class is determined
    if (!isSource)
        constraint.SetMultiplicity(BECN::RelationshipMultiplicity::ZeroMany());

    if (ContainsAnyClass(constraint.GetConstraintClasses()))
        {
        constraint.RemoveConstraintClasses();
        Utf8String msg;
        msg.Sprintf("ECRelationshipClass '%s' contains 'AnyClass' as a constraint.  AnyClass is no longer supported and will be replaced with '%s.'",
                    relClass.GetFullName(), defaultConstraintClass->GetName().c_str());
        context.ReportIssue(Converter::IssueSeverity::Warning, msg.c_str());
        }
    
    if (0 == constraint.GetConstraintClasses().size())
        {
        if (ECObjectsStatus::SchemaNotFound == constraint.AddClass(*defaultConstraintClass))
            {
            relClass.GetSchemaR().AddReferencedSchema(defaultConstraintClass->GetSchemaR());
            constraint.AddClass(*defaultConstraintClass);
            }
        }
    else if (relClass.HasBaseClasses())
        {
        for (auto constraintClass : constraint.GetConstraintClasses())
            {
            ECRelationshipClassCP baseClass = relClass.GetBaseClasses()[0]->GetRelationshipClassCP();
            ECRelationshipConstraintR baseConstraint = (isSource) ? baseClass->GetSource() : baseClass->GetTarget();
            
            bvector<ECClassCP> constraintsToRemove;
            for (auto baseConstraintClass : baseConstraint.GetConstraintClasses())
                {
                if (!constraintClass->Is(baseConstraintClass))
                    constraintsToRemove.push_back(baseConstraintClass);
                }

            if (constraintsToRemove.size() > 0)
                {
                BECN::ECObjectsStatus status = baseConstraint.AddClass(*defaultConstraintClass);
                if (BECN::ECObjectsStatus::Success != status)
                    return;

                for (ECClassCP ecClass : constraintsToRemove)
                    {
                    if (ecClass->IsEntityClass())
                        baseConstraint.RemoveClass(*ecClass->GetEntityClassCP());
                    else if (ecClass->IsRelationshipClass())
                        baseConstraint.RemoveClass(*ecClass->GetRelationshipClassCP());
                    }
                }
            }
        }

    IECInstancePtr dropConstraintCA = constraint.GetCustomAttribute("DropConstraints");
    if (dropConstraintCA.IsValid())
        {
        ECValue drop;
        if (ECObjectsStatus::Success == dropConstraintCA->GetValue(drop, "Drop"))
            {
            bvector<Utf8String> tokens;
            BeStringUtilities::Split(drop.GetUtf8CP(), ";", tokens);
            for (Utf8StringCR token : tokens)
                {
                bvector<Utf8String> components;
                BeStringUtilities::Split(token.c_str(), ":", components);
                ECClassCP constraintClassToDrop = nullptr;
                if (components.size() == 1)
                    constraintClassToDrop = relClass.GetSchema().GetClassCP(components[0].c_str());
                else
                    {
                    ECSchemaCP referencedSchema = relClass.GetSchema().GetSchemaByAliasP(components[0]);
                    if (nullptr == referencedSchema)
                        {
                        Utf8PrintfString error("DropConstraints custom attribute on %s constraint on %s defines %s.  A schema by this alias could not be found.  Constraint class not dropped.  This could cause validation issues.", isSource ? "source" : "target", relClass.GetFullName(), token.c_str());
                        context.ReportIssue(Converter::IssueSeverity::Error, error.c_str());
                        }
                    else
                        constraintClassToDrop = referencedSchema->GetClassCP(components[1].c_str());
                    }
                if (nullptr != constraintClassToDrop)
                    {
                    if (BECN::ECObjectsStatus::Success != constraint.RemoveClass(*constraintClassToDrop->GetEntityClassCP()))
                        {
                        context.ReportIssue(Converter::IssueSeverity::Error, "Failed to remove constraint class %s from %s constraint on relationship %s.  This could cause validation issues with the schema.", constraintClassToDrop->GetFullName(), isSource ? "source" : "target", relClass.GetFullName());
                        }
                    }
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     04/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisClassConverter::FinalizeConversion(SchemaConversionContext& context)
    {
    //order is important, so use a vector for capturing the aspect only classes
    bvector<BECN::ECClassP> aspectOnlyClasses;
    for (std::pair<BECN::ECClassCP, SchemaConversionContext::ElementAspectDefinition> const& kvPair : context.GetAspectMappings())
        {
        BECN::ECClassCP inputClass = kvPair.first;

        SchemaConversionContext::ElementAspectDefinition const& aspectDef = kvPair.second;
        BECN::ECClassP aspectClass = aspectDef.m_aspectClass;
        if (aspectDef.m_isAspectOnly)
            {
            BeAssert(std::find(aspectOnlyClasses.begin(), aspectOnlyClasses.end(), aspectClass) == aspectOnlyClasses.end());
            aspectOnlyClasses.push_back(aspectClass);
            }

        //first add domain base classes
        for (BECN::ECClassCP inputBaseClass : inputClass->GetBaseClasses())
            {
            SchemaConversionContext::ElementAspectDefinition const* baseAspectDefinition = context.TryGetAspectMapping(*inputBaseClass);
            if (baseAspectDefinition == nullptr)
                continue; //doesn't have a mapping -> nothing to do (no error)

            BECN::ECClassCP aspectBaseClass = baseAspectDefinition->m_aspectClass;
            BeAssert(aspectBaseClass != nullptr);
            if (BSISUCCESS != AddBaseClass(*aspectClass, *aspectBaseClass))
                return BSIERROR;
            }

        //then add BIS aspect base class (if domain base class have this already, it will not be added again)
        if (BSISUCCESS != AddBaseClass(*aspectClass, *aspectDef.m_aspectBisBaseClass))
            return BSIERROR;
        }

    //For aspect only classes, no new classes were created, but the input class just renamed. So we need to remove
    //the domain base classes that have now become element subclasses
    for (BECN::ECClassP aspectOnlyClass : aspectOnlyClasses)
        {
        bvector<BECN::ECClassCP> baseClasses;
        baseClasses.insert(baseClasses.end(), aspectOnlyClass->GetBaseClasses().begin(), aspectOnlyClass->GetBaseClasses().end());
        for (BECN::ECClassCP baseClass : baseClasses)
            {
            if (baseClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_Element))
                {
                // We need to preserve the properties from the domain schemas, though
                for (ECN::ECPropertyCP ecProperty : baseClass->GetProperties(true))
                    {
                    ECPropertyP destProperty;
                    if (ecProperty->GetClass().GetSchema().GetName().Equals(BIS_ECSCHEMA_NAME))
                        continue;
                    aspectOnlyClass->CopyProperty(destProperty, ecProperty, true);
                    }

                if (BECN::ECObjectsStatus::Success != aspectOnlyClass->RemoveBaseClass(*baseClass))
                    {
                    context.ReportIssue(Converter::IssueSeverity::Fatal, "ECSchema BISification failed. Could not remove v8 base ECClass '%s' from v8 secondary instance ECClass '%s' in target schema.",
                                        baseClass->GetFullName(),
                                        aspectOnlyClass->GetFullName());
                    return BSIERROR;
                    }
                }
            }
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
void BisClassConverter::GetBisBaseClasses(BECN::ECClassCP& elementBaseClass, BECN::ECClassCP& elementAspectBaseClass, SchemaConversionContext& context, BisConversionRule conversionRule)
    {
    if (conversionRule != BisConversionRule::ToAspectOnly)
        {
        elementBaseClass = context.GetBaseClass(GetElementBisBaseClassName(conversionRule));
        BeAssert(elementBaseClass != nullptr);
        }
    elementAspectBaseClass = context.GetBaseClass(GetElementAspectBisBaseClassName(BisConversionRule::ToAspectOnly));

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisClassConverter::AddBaseClass(BECN::ECClassR targetClass, BECN::ECClassCR baseClass)
    {
    std::function <bool(BECN::ECClassCR ecClass, BECN::ECClassCR candidateBaseClass)> isAlreadyBaseClass;
    isAlreadyBaseClass = [&isAlreadyBaseClass] (BECN::ECClassCR ecClass, BECN::ECClassCR candidateBaseClass)
        {
        for (BECN::ECClassCP actualBaseClass : ecClass.GetBaseClasses())
            {
            if (actualBaseClass == &candidateBaseClass)
                return true;

            if (isAlreadyBaseClass(*actualBaseClass, candidateBaseClass))
                return true;
            }

        return false;
        };

    if (!isAlreadyBaseClass(targetClass, baseClass))
        {
        //add base schema as reference if it wasn't referenced yet (otherwise InsertBaseClass will fail)
        BECN::ECSchemaR targetSchema = targetClass.GetSchemaR();
        BECN::ECSchemaCR baseSchema = baseClass.GetSchema();
        if (&targetSchema != &baseSchema && !BECN::ECSchema::IsSchemaReferenced(targetSchema, baseSchema))
            targetSchema.AddReferencedSchema(const_cast<BECN::ECSchemaR> (baseSchema));

        //add base classes first, so that BIS base types appear first in the base class list. By spec
        //EC traverses base classes in a depth-first fashion, and DgnDn's handler look up code needs to find the 
        //most specialized BIS base class first
        BECN::ECObjectsStatus status;
        if (BECN::ECObjectsStatus::Success != (status = targetClass.AddBaseClass(baseClass, true, true)))
            {
            if (BECN::ECObjectsStatus::DataTypeMismatch != status)
                return BSIERROR;
            // else, move the property?
            }
        }

    return BSISUCCESS;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisClassConverter::MoveProperties(BECN::ECClassR toClass, BECN::ECClassR fromClass)
    {
    //first copy then remove properties in from class
    bvector<Utf8StringCP> fromPropNames;
    for (BECN::ECPropertyCP fromProp : fromClass.GetProperties(false))
        {
        BECN::ECPropertyP newProp = nullptr;
        if (BECN::ECObjectsStatus::Success != toClass.CopyProperty(newProp, fromProp, true))
            return BSIERROR;

        fromPropNames.push_back(&fromProp->GetName());
        }

    for (Utf8StringCP fromPropName : fromPropNames)
        {
        if (BECN::ECObjectsStatus::Success != fromClass.RemoveProperty(*fromPropName))
            return BSIERROR;
        }


    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisClassConverter::RemoveDuplicateClassMapCustomAttributes(BECN::ECClassR ecClass)
    {
    ecClass.RemoveCustomAttribute(CUSTOMCLASSMAP_CLASSNAME);
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     09/2015
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisClassConverter::ValidateClassProperties(SchemaConversionContext& context, BECN::ECClassR ecClass)
    {
    bvector<Utf8CP> reservedNames {"ECInstanceId", "Id", "ECClassId", "SourceECInstanceId", "SourceId", "SourceECClassId", "TargetECInstanceId", "TargetId", "TargetECClassId"};
    for (BECN::ECPropertyP prop : ecClass.GetProperties(false))
        {
        // Need to make sure that property name does not conflict with one of the reserved system properties or aliases.
        Utf8CP thisName = prop->GetName().c_str();
        auto found = std::find_if(reservedNames.begin(), reservedNames.end(), [thisName](Utf8CP reserved) ->bool { return BeStringUtilities::StricmpAscii(thisName, reserved) == 0; });
        if (found != reservedNames.end())
            {
            ecClass.RenameConflictProperty(prop, true);
            }
        BECN::ECClassCP structType = nullptr;

        BECN::ArrayECPropertyP arrayProp = prop->GetAsArrayPropertyP();
        if (arrayProp != nullptr)
            {
            arrayProp->SetMinOccurs(0);
            arrayProp->SetMaxOccurs(UINT_MAX);

            if (arrayProp->GetIsStructArray())
                structType = &arrayProp->GetAsStructArrayProperty()->GetStructElementType();
            }
        else
            {
            BECN::StructECPropertyP structProp = prop->GetAsStructPropertyP();
            if (structProp != nullptr)
                structType = &structProp->GetType();
            }

        if (structType != nullptr)
            {
            //for array props in structs we only change the bounds if the struct is defined in a schema
            //which is included in the input schema list and which is not a standard/system/supp schema
            BECN::ECSchemaCR structSchema = structType->GetSchema();
            bmap<Utf8String, BECN::ECSchemaP> const& inputSchemas = context.GetSchemas();
            if (context.ExcludeSchemaFromBisification(structSchema) ||
                inputSchemas.find(structSchema.GetName()) == inputSchemas.end())
                continue;

            //const_cast should be safe...
            BECN::ECClassP structTypeP = const_cast<BECN::ECClassP> (structType);
            BeAssert(structTypeP != nullptr);

            if (BSISUCCESS != ValidateClassProperties(context, *structTypeP))
                return BSIERROR;
            }
        }

    return BSISUCCESS;
    }

//****************************************************************************************
// ElementConverter::SchemaConversionContext
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
BisClassConverter::SchemaConversionContext::SchemaConversionContext(Converter& converter, BECN::ECSchemaReadContext& readContext)
    : m_converter(converter), m_domainRelationshipBaseClass(nullptr), m_defaultConstraintClass(nullptr)
    {
    //need a schema map keyed on name only. SchemaCache doesn't support that
    bvector<BECN::ECSchemaP> schemaCache;
    readContext.GetCache().GetSchemas(schemaCache);

    for (auto schema : schemaCache)
        m_inputSchemaMap[Utf8String(schema->GetName())] = schema;


    BECN::SchemaKey dgnSchemaKey(BIS_ECSCHEMA_NAME, 1, 0);
    BECN::ECSchemaPtr dgnSchema = readContext.LocateSchema(dgnSchemaKey, BECN::SchemaMatchType::Latest);
    if (dgnSchema == nullptr)
        {
        BeAssert(false && "Locating Dgn ECSchema failed");
        ReportIssue(Converter::IssueSeverity::Fatal, "Could not locate ECSchema " BIS_ECSCHEMA_NAME ".");
        }
    else
        {
        //read context is owner of the schema, so just return raw pointer
        m_baseSchemaCache[BIS_ECSCHEMA_NAME] = dgnSchema.get();
        }

    BECN::SchemaKey genericSchemaKey(GENERIC_DOMAIN_NAME, 1, 0);
    BECN::ECSchemaPtr genericSchema = readContext.LocateSchema(genericSchemaKey, BECN::SchemaMatchType::Latest);
    if (genericSchema == nullptr)
        {
        BeAssert(false && "Locating GenericDomain ECSchema failed");
        ReportIssue(Converter::IssueSeverity::Fatal, "Could not locate ECSchema " GENERIC_DOMAIN_NAME ".");
        }
    else
        {
        //read context is owner of the schema, so just return raw pointer
        m_baseSchemaCache[GENERIC_DOMAIN_NAME] = genericSchema.get();
        }

    BECN::SchemaKey functionalSchemaKey(FUNCTIONAL_DOMAIN_NAME, 1, 0);
    BECN::ECSchemaPtr functionalSchema = readContext.LocateSchema(functionalSchemaKey, BECN::SchemaMatchType::Latest);
    if (functionalSchema == nullptr)
        {
        BeAssert(false && "Locating FunctionalDomain ECSchema failed");
        ReportIssue(Converter::IssueSeverity::Fatal, "Could not locate ECSchema " FUNCTIONAL_DOMAIN_NAME ".");
        }
    else
        {
        //read context is owner of the schema, so just return raw pointer
        m_baseSchemaCache[FUNCTIONAL_DOMAIN_NAME] = functionalSchema.get();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
BECN::ECClassP BisClassConverter::SchemaConversionContext::GetInputClass(Utf8CP schemaName, Utf8CP className) const
    {
    auto it = m_inputSchemaMap.find(schemaName);
    BeAssert(it != m_inputSchemaMap.end());
    BECN::ECSchemaP inputSchema = it->second;
    BECN::ECClassP inputClass = inputSchema->GetClassP(className);
    BeAssert(inputClass != nullptr);
    return inputClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BECN::ECEntityClassP BisClassConverter::SchemaConversionContext::GetDefaultConstraintClass() const
    {
    if (nullptr == m_defaultConstraintClass)
        {
        ECClassCP temp = m_converter.GetDgnDb().Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Element);
        m_defaultConstraintClass = const_cast<ECClassP>(temp)->GetEntityClassP();
        }

    return m_defaultConstraintClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     04/2015
//---------------------------------------------------------------------------------------
BECN::ECClassCP BisClassConverter::SchemaConversionContext::GetBaseClass(ECClassName className) const
    {
    auto it = m_baseSchemaCache.find(className.GetSchemaName());
    if (it == m_baseSchemaCache.end())
        {
        BeAssert(false && "ECSchema not found in cache.");
        return nullptr;
        }

    return it->second->GetClassCP(className.GetClassName());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     04/2015
//---------------------------------------------------------------------------------------
BECN::ECRelationshipClassCP BisClassConverter::SchemaConversionContext::GetDomainRelationshipBaseClass(BECN::ECRelationshipClassR inputClass) const
    {
    if (m_domainRelationshipBaseClass == nullptr)
        {
        ECClassCP base = GetBaseClass(ECClassName(BIS_ECSCHEMA_NAME, BIS_REL_ElementRefersToElements))->GetRelationshipClassCP();
        if (base == nullptr)
            {
            BeAssert(base != nullptr);
            ReportIssue(Converter::IssueSeverity::Fatal, "Could not find ECClass " BIS_ECSCHEMA_NAME ":" BIS_REL_ElementRefersToElements);
            }
        m_domainRelationshipBaseClass = base->GetRelationshipClassCP();
        }

    if (m_aspectRelationshipBaseClass == nullptr)
        {
        ECClassCP base = GetBaseClass(ECClassName(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsMultiAspects));
        if (base == nullptr)
            {
            BeAssert(base != nullptr);
            ReportIssue(Converter::IssueSeverity::Fatal, "Could not find ECClass " BIS_ECSCHEMA_NAME ":" BIS_REL_ElementOwnsMultiAspects);
            }
        m_aspectRelationshipBaseClass = base->GetRelationshipClassCP();
        }

    // It is possible that the source is bis-ified as an Aspect (this is BAD, but there are schemas/dgns that do this).  In that case, the relationship cannot be bisified.
    for (auto constraintClass : inputClass.GetSource().GetConstraintClasses())
        {
        if (!constraintClass->Is(GetDefaultConstraintClass()))
            return nullptr;
        }

    ECClassCP abstractConstraint = inputClass.GetTarget().GetAbstractConstraint();
    if (nullptr == abstractConstraint || abstractConstraint->Is(GetDefaultConstraintClass()))
        return m_domainRelationshipBaseClass;
    return m_aspectRelationshipBaseClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus BisClassConverter::SchemaConversionContext::AddClassMapping(BECN::ECClassCR inputClass, BECN::ECClassR aspectClass, bool isAspectOnly, BECN::ECClassCR aspectBisBaseClass)
    {
    BeAssert(m_aspectMappingIndexMap.find(&inputClass) == m_aspectMappingIndexMap.end());

    const size_t newIndex = m_aspectMappings.size();

    std::pair<BECN::ECClassCP, ElementAspectDefinition> pair {&inputClass, ElementAspectDefinition(aspectClass, isAspectOnly, aspectBisBaseClass)};
    m_aspectMappings.push_back(std::move(pair));
    m_aspectMappingIndexMap[&inputClass] = newIndex;

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     04/2015
//---------------------------------------------------------------------------------------
bool BisClassConverter::SchemaConversionContext::IsAlreadyConverted(BECN::ECClassCR inputClass) const
    {
    return m_aspectMappingIndexMap.find(&inputClass) != m_aspectMappingIndexMap.end();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     06/2015
//---------------------------------------------------------------------------------------
BisClassConverter::SchemaConversionContext::ElementAspectDefinition const* BisClassConverter::SchemaConversionContext::TryGetAspectMapping(ECN::ECClassCR inputClass) const
    {
    auto indexIt = m_aspectMappingIndexMap.find(&inputClass);
    if (indexIt == m_aspectMappingIndexMap.end())
        return nullptr;

    const size_t index = indexIt->second;

    if (index >= m_aspectMappings.size())
        {
        BeAssert(false);
        return nullptr;
        }

    BeAssert(m_aspectMappings[index].first == &inputClass);
    return &m_aspectMappings[index].second;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     12/2015
//---------------------------------------------------------------------------------------
//static
bool BisClassConverter::SchemaConversionContext::ExcludeSchemaFromBisification(ECN::ECSchemaCR schema)
    {
    return schema.IsStandardSchema() || schema.IsSystemSchema() || schema.IsSupplementalSchema() ||
        schema.GetName().EqualsI(DGNDBSYNCV8_ECSCHEMA_NAME) || schema.GetName().EqualsI(BIS_ECSCHEMA_NAME) || schema.GetName().StartsWithI("ecdb_");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
void BisClassConverter::SchemaConversionContext::ReportIssue(Converter::IssueSeverity severity, Utf8CP fmt, ...) const
    {
    va_list args;
    va_start(args, fmt);
    ReportIssueV(severity, fmt, args);
    va_end(args);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
void BisClassConverter::SchemaConversionContext::ReportIssueV(Converter::IssueSeverity severity, Utf8CP fmt, va_list args) const
    {
    Utf8String message;
    message.VSprintf(fmt, args);
    GetConverter().ReportIssue(severity, Converter::IssueCategory::Sync(), Converter::Issue::Message(), message.c_str());
    }

//****************************************************************************************
// BisClassConverter::ECClassRemovalContext
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
BisClassConverter::ECClassRemovalContext::ECClassRemovalContext(SchemaConversionContext& schemaConversionContext)
    : m_schemaConversionContext(schemaConversionContext)
    {
    for (bpair<Utf8String, BECN::ECSchemaP> const& kvpair : schemaConversionContext.GetSchemas())
        {
        BECN::ECSchemaP schema = kvpair.second;
        m_schemas.insert(schema);
        for (BECN::ECClassP ecclass : schema->GetClasses())
            {
            m_candidateClasses.insert(ecclass);
            }
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
BentleyStatus BisClassConverter::ECClassRemovalContext::AddClassToRemove(BECN::ECClassR ecclass, bool includeSubclasses)
    {
    if (BSISUCCESS != DoAddClassToRemove(ecclass))
        return BSIERROR;

    if (includeSubclasses)
        {
        for (BECN::ECClassP subclass : ecclass.GetDerivedClasses())
            {
            if (BSISUCCESS != AddClassToRemove(*subclass, includeSubclasses))
                return BSIERROR;
            }
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
BentleyStatus BisClassConverter::ECClassRemovalContext::DoAddClassToRemove(BECN::ECClassR ecclass)
    {
    if (m_classesToRemove.find(&ecclass) != m_classesToRemove.end())
        return BSISUCCESS;

    m_classesToRemove.insert(&ecclass);
    ECClassName className(ecclass);
    if (BSISUCCESS != V8ECClassInfo::InsertOrUpdate(m_schemaConversionContext.GetConverter(), className, BisConversionRule::Ignored))
        return BSIERROR;

    return FindClassReferences(ecclass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     04/2015
//---------------------------------------------------------------------------------------
BentleyStatus BisClassConverter::ECClassRemovalContext::FindClassReferences(BECN::ECClassCR ecclass)
    {
    for (BECN::ECSchemaP schema : m_schemas)
        {
        RemoveCustomAttribute(*schema, ecclass);
        }

    for (BECN::ECClassP candidateECClass : m_candidateClasses)
        {
        if (candidateECClass == &ecclass || m_classesToRemove.find(candidateECClass) != m_classesToRemove.end())
            continue;

        RemoveCustomAttribute(*candidateECClass, ecclass);

        for (BECN::ECPropertyP prop : candidateECClass->GetProperties(false))
            {
            RemoveCustomAttribute(*prop, ecclass);

            BECN::ECClassCP structType = nullptr;
            BECN::StructECPropertyCP structProp = prop->GetAsStructProperty();
            if (structProp != nullptr)
                structType = &structProp->GetType();
            else
                {
                BECN::StructArrayECPropertyCP arrayProp = prop->GetAsStructArrayProperty();
                if (arrayProp != nullptr)
                    structType = &arrayProp->GetStructElementType();
                }

            if (structType != nullptr && structType == &ecclass)
                {
                m_schemaConversionContext.ReportIssue(Converter::IssueSeverity::Warning,
                                                      "ECClass '%s' is skipped from Bisification because its ECProperty %s is of the skipped type '%s'.",
                                                      Utf8String(candidateECClass->GetFullName()).c_str(),
                                                      Utf8String(prop->GetName().c_str()).c_str(),
                                                      Utf8String(ecclass.GetFullName()).c_str());

                AddClassToRemove(*candidateECClass);
                break;
                }
            }

        BECN::ECRelationshipClassCP relClass = candidateECClass->GetRelationshipClassCP();
        if (relClass == nullptr)
            continue;

        RemoveCustomAttribute(relClass->GetSource(), ecclass);
        RemoveCustomAttribute(relClass->GetTarget(), ecclass);

        const bool relIsAbstract = ECClassModifier::Abstract == relClass->GetClassModifier() || (relClass->GetSource().GetConstraintClasses().empty() || relClass->GetTarget().GetConstraintClasses().empty());
        ECEntityClassCP ecClassAsEntity = ecclass.GetEntityClassCP();
        if (nullptr != ecClassAsEntity)
            {
            relClass->GetSource().RemoveClass(*ecClassAsEntity);
            relClass->GetTarget().RemoveClass(*ecClassAsEntity);
            }

        if (!relIsAbstract && (relClass->GetSource().GetConstraintClasses().empty() || relClass->GetTarget().GetConstraintClasses().empty()))
            //if the relationship class was abstract before already, we will not delete it
            {
            m_schemaConversionContext.ReportIssue(Converter::IssueSeverity::Warning, "ECRelationshipClass '%s' is skipped from Bisification because one of its constraints only consists of the skipped ECClass '%s', making the ECRelationshipClass incomplete.",
                                                  candidateECClass->GetFullName(), ecclass.GetFullName());
            AddClassToRemove(*candidateECClass);
            }
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
//static
bool BisClassConverter::ECClassRemovalContext::RemoveCustomAttribute(ECN::IECCustomAttributeContainerR container, ECN::ECClassCR customAttributeClass)
    {
    bool found = container.RemoveCustomAttribute(customAttributeClass);
    //return true if one of the two methods succeeded (because a CA can live in both or in only one of the two)
    return container.RemoveSupplementedCustomAttribute(customAttributeClass) || found;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     07/2015
//---------------------------------------------------------------------------------------
BentleyStatus BisClassConverter::ECClassRemovalContext::FixClassHierarchies()
    {
    for (BECN::ECClassP classToRemove : m_classesToRemove)
        {
        BECN::ECDerivedClassesList derivedClasses(classToRemove->GetDerivedClasses());
        if (derivedClasses.empty())
            continue;

        BECN::ECClassCP newBaseClass = nullptr;
        if (classToRemove->HasBaseClasses())
            {
            for (BECN::ECClassP baseClass : classToRemove->GetBaseClasses())
                {
                if (m_classesToRemove.find(baseClass) == m_classesToRemove.end())
                    {
                    newBaseClass = baseClass;
                    break;
                    }
                }
            }

        for (BECN::ECClassP derivedClass : derivedClasses)
            {
            if (BECN::ECObjectsStatus::Success != derivedClass->RemoveBaseClass(*classToRemove))
                return BSISUCCESS;

            if (newBaseClass != nullptr)
                {
                if (BECN::ECObjectsStatus::Success != derivedClass->AddBaseClass(*newBaseClass))
                    return BSISUCCESS;
                }
            }
        }

    return BSISUCCESS;
    }

//****************************************************************************************
// ElementConverter
//****************************************************************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ElementConverter::ConvertToElementItem(ElementConversionResults& results, ECObjectsV8::IECInstance const* v8Instance, BisConversionRule const* primaryInstanceConversionRule) const
    {
    if (v8Instance == nullptr)
        return BSISUCCESS;

    BeAssert(primaryInstanceConversionRule != nullptr);

    BECN::ECClassCP dgnDbClass = GetDgnDbClass(*v8Instance, nullptr);
    if (dgnDbClass == nullptr)
        {
        Utf8String error;
        error.Sprintf("Inserting properties for %s failed. Could not find target ECClass in the DgnDb file.", ToInstanceLabel(*v8Instance).c_str());
        m_converter.ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Error(), error.c_str());
        return BSIERROR;
        }

    ECN::IECInstancePtr targetInstance = Transform(*v8Instance, *dgnDbClass);
    if (targetInstance == nullptr)
        {
        Utf8String error;
        error.Sprintf("Inserting properties for %s failed. Transforming v8 ECInstance to ECInstance failed.", ToInstanceLabel(*v8Instance).c_str());
        m_converter.ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Error(), error.c_str());
        return BSIERROR;
        }
    results.m_v8PrimaryInstance = V8ECInstanceKey(ECClassName(v8Instance->GetClass()), v8Instance->GetInstanceId().c_str());
    return (DgnDbStatus::Success == results.m_element->SetPropertyValues(*targetInstance))? BSISUCCESS: BSIERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
BECN::ECInstanceReadContextPtr ElementConverter::LocateInstanceReadContext(BECN::ECSchemaCR schema) const
    {
    auto it = m_instanceReadContextCache.find(schema.GetName().c_str());
    if (it != m_instanceReadContextCache.end())
        return it->second;

    BECN::ECInstanceReadContextPtr context = BECN::ECInstanceReadContext::CreateContext(schema);
    m_instanceReadContextCache[schema.GetName().c_str()] = context;
    return context;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
BECN::IECInstancePtr ElementConverter::Transform(ECObjectsV8::IECInstance const& v8Instance, BECN::ECClassCR dgnDbClass, bool transformAsAspect) const
    {
    Bentley::Utf8String ecInstanceXml;
    const ECObjectsV8::InstanceWriteStatus writeStat = const_cast<ECObjectsV8::IECInstance&> (v8Instance).WriteToXmlString(ecInstanceXml, true,
                                                                                                                           false); //don't bring over v8 instance id, to make clear to DgnElement that it should not attempt to update the instance and instead insert right away
    if (writeStat != ECObjectsV8::INSTANCE_WRITE_STATUS_Success)
        return nullptr;

    BECN::ECInstanceReadContextPtr context = LocateInstanceReadContext(dgnDbClass.GetSchema());
    context->SetUnitResolver(&m_unitResolver);
    context->SetSchemaRemapper(&m_schemaRemapper);
    m_schemaRemapper.SetRemapAsAspect(transformAsAspect);

    BECN::IECInstancePtr dgnDbECInstance = nullptr;
    const BECN::InstanceReadStatus readStat = BECN::IECInstance::ReadFromXmlString(dgnDbECInstance, (Utf8CP) ecInstanceXml.c_str(), *context);
    if (readStat != BECN::InstanceReadStatus::Success)
        return nullptr;

    return dgnDbECInstance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
BECN::ECClassCP ElementConverter::GetDgnDbClass(ECObjectsV8::IECInstance const& v8Instance, Utf8CP aspectClassSuffix) const
    {
    ECObjectsV8::ECClassCR aspectClass = v8Instance.GetClass();
    Utf8String schemaName(aspectClass.GetSchema().GetName().c_str());
    Utf8String className(aspectClass.GetName().c_str());
    if (aspectClassSuffix != nullptr)
        className.append(aspectClassSuffix);

    return m_converter.GetDgnDb().Schemas().GetClass(schemaName.c_str(), className.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
//static
Utf8String ElementConverter::ToInstanceLabel(ECObjectsV8::IECInstance const& v8Instance)
    {
    Utf8String label;
    label.Sprintf("v8 '%s' instance '%s'",
                  Bentley::Utf8String(v8Instance.GetClass().GetFullName()).c_str(),
                  Bentley::Utf8String(v8Instance.GetInstanceId()).c_str());

    return label;
    }

//****************************************************************************************
// ElementConverter::SchemaRemapper
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
bool ElementConverter::SchemaRemapper::_ResolveClassName(Utf8StringR serializedClassName, BECN::ECSchemaCR ecSchema) const
    {
    BisConversionRule conversionRule;
    bool hasSecondary;
    if (!V8ECClassInfo::TryFind(conversionRule, m_converter.GetDgnDb(),
                                ECClassName(ecSchema.GetName().c_str(), serializedClassName.c_str()), hasSecondary))
        {
        BeAssert(false);
        return false;
        }

    if (hasSecondary && m_remapAsAspect)
        conversionRule = BisConversionRule::ToAspectOnly;
    Utf8CP suffix = BisClassConverter::GetAspectClassSuffix(conversionRule);
    if (!Utf8String::IsNullOrEmpty(suffix))
        serializedClassName.append(suffix);
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool ElementConverter::SchemaRemapper::_ResolvePropertyName(Utf8StringR serializedPropertyName, ECN::ECClassCR ecClass) const
    {
    if (!m_convSchema.IsValid())
        {
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        SchemaKey key("ECv3ConversionAttributes", 1, 0);
        m_convSchema = ECSchema::LocateSchema(key, *context);
        if (!m_convSchema.IsValid())
            {
            BeAssert(false);
            return false;
            }
        }

    if (ECSchema::IsSchemaReferenced(ecClass.GetSchema(), *m_convSchema))
        {
        T_propertyNameMappings properties;
        T_ClassPropertiesMap::iterator mappedClassIter = m_renamedClassProperties.find(ecClass.GetFullName());
        if (mappedClassIter == m_renamedClassProperties.end())
            {
            IECInstancePtr renameInstance = ecClass.GetCustomAttributeLocal("ECv3ConversionAttributes", "RenamedPropertiesMapping");
            if (renameInstance.IsValid())
                {
                ECValue v;
                renameInstance->GetValue(v, "PropertyMapping");

                bvector<Utf8String> components;
                BeStringUtilities::Split(v.GetUtf8CP(), ";", components);
                for (Utf8String mapping : components)
                    {
                    bvector<Utf8String> components2;
                    BeStringUtilities::Split(mapping.c_str(), "|", components2);
                    bpair<Utf8String, Utf8String> pair(components2[0], components2[1]);
                    properties.insert(pair);
                    }
                }
            bpair<Utf8String, T_propertyNameMappings> pair2(Utf8String(ecClass.GetFullName()), properties);
            m_renamedClassProperties.insert(pair2);
            }
        else
            properties = mappedClassIter->second;

        T_propertyNameMappings::iterator mappedPropertiesIterator = properties.find(serializedPropertyName);
        if (mappedPropertiesIterator != properties.end())
            {
            serializedPropertyName = mappedPropertiesIterator->second;
            return true;
            }
        }

    for (ECClassP baseClass : ecClass.GetBaseClasses())
        {
        if (_ResolvePropertyName(serializedPropertyName, *baseClass))
            return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2017
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String ElementConverter::UnitResolver::_ResolveUnitName(ECPropertyCR ecProperty) const
    {
    if (!m_convSchema.IsValid())
        {
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        SchemaKey key("ECv3ConversionAttributes", 1, 0);
        m_convSchema = ECSchema::LocateSchema(key, *context);
        if (!m_convSchema.IsValid())
            {
            BeAssert(false);
            return "";
            }
        }

    if (ECSchema::IsSchemaReferenced(ecProperty.GetClass().GetSchema(), *m_convSchema))
        {
        IECInstancePtr instance = ecProperty.GetCustomAttribute("ECv3ConversionAttributes", "OldPersistenceUnit");
        if (instance.IsValid())
            {
            ECValue unitName;
            instance->GetValue(unitName, "Name");

            if (!unitName.IsNull() && unitName.IsUtf8())
                return unitName.GetUtf8CP();
            }
        }
    
    return "";
    }

//****************************************************************************************
// ElementAspectConverter
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus ElementAspectConverter::ConvertToAspects(ElementConversionResults& results,
                                                       std::vector<std::pair<ECObjectsV8::IECInstancePtr, BisConversionRule>> const& secondaryInstances) const
    {
    for (std::pair<ECObjectsV8::IECInstancePtr, BisConversionRule> const& v8SecondaryInstance : secondaryInstances)
        {
        if (BSISUCCESS != ConvertToAspect(results, *v8SecondaryInstance.first, BisClassConverter::GetAspectClassSuffix(v8SecondaryInstance.second)))
            return BSIERROR;
        }

    return BSISUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
BentleyStatus ElementAspectConverter::ConvertToAspect(ElementConversionResults& results, ECObjectsV8::IECInstance const& v8Instance, Utf8CP aspectClassSuffix) const
    {
    BECN::ECClassCP aspectClass = GetDgnDbClass(v8Instance, aspectClassSuffix);
    if (aspectClass == nullptr)
        {
        Utf8String error;
        error.Sprintf("Inserting aspect for %s failed. Could not find target aspect ECClass in the DgnDb file.", ToInstanceLabel(v8Instance).c_str());
        m_converter.ReportIssue(Converter::IssueSeverity::Fatal, Converter::IssueCategory::Sync(), Converter::Issue::Error(), error.c_str());
        return BSIERROR;
        }

    ECN::IECInstancePtr targetInstance = Transform(v8Instance, *aspectClass, true);
    if (targetInstance == nullptr)
        {
        Utf8String error;
        error.Sprintf("Inserting aspect for %s failed. Transforming v8 ECInstance to aspect ECInstance failed.", ToInstanceLabel(v8Instance).c_str());
        m_converter.ReportIssue(Converter::IssueSeverity::Fatal, Converter::IssueCategory::Sync(), Converter::Issue::Error(), error.c_str());
        return BSIERROR;
        }

    DgnElement::GenericMultiAspect::AddAspect(*results.m_element, *targetInstance);

    results.m_v8SecondaryInstanceMappings.push_back(bpair<V8ECInstanceKey, BECN::IECInstancePtr>(
        V8ECInstanceKey(ECClassName(v8Instance.GetClass()), v8Instance.GetInstanceId().c_str()),
        targetInstance));

    return BSISUCCESS;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE
