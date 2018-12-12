/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/DynamicSchemaGenerator/EntityConverter.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include "ECConversion.h"

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
ECClassName BisConversionRuleHelper::GetElementBisBaseClassName(BisConversionRule conversionRule)
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
ECClassName BisConversionRuleHelper::GetElementAspectBisBaseClassName(BisConversionRule conversionRule)
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
Utf8CP BisConversionRuleHelper::GetAspectClassSuffix(BisConversionRule conversionRule)
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
BentleyStatus BisClassConverter::CheckBaseAndDerivedClassesForBisification(SchemaConversionContext& context, ECN::ECClassCP incomingClass, BisConversionRule incomingRule, bvector<ECClassP> classes, bool isBaseClassCheck)
    {
    DynamicSchemaGenerator& converter = context.GetConverter();
    for (BECN::ECClassP ecClass : classes)
        {
        ECClassName v8ClassName(*ecClass);
        BisConversionRule existingRule;
        BECN::ECClassId existingV8ClassId;
        bool hasSecondary;
        if (V8ECClassInfo::TryFind(existingV8ClassId, existingRule, context.GetDgnDb(), v8ClassName, hasSecondary))
            {
            if (BisConversionRule::ToDefaultBisBaseClass == existingRule && BisConversionRule::ToDefaultBisBaseClass != incomingRule)
                {
                if (BSISUCCESS != V8ECClassInfo::Update(converter, existingV8ClassId, incomingRule))
                    return BSIERROR;
                if (BisConversionRule::ToPhysicalElement != incomingRule)
                    if (BSISUCCESS != CheckBaseAndDerivedClassesForBisification(context, ecClass, incomingRule, isBaseClassCheck ? ecClass->GetDerivedClasses() : ecClass->GetBaseClasses(), !isBaseClassCheck))
                        return BSIERROR;
                }
            // If a class is using the ToAspectOnly rule, all of its children must also become aspects
            else if (!isBaseClassCheck && BisConversionRule::ToAspectOnly == incomingRule)
                {
                if (BSISUCCESS != V8ECClassInfo::Update(converter, existingV8ClassId, incomingRule))
                    return BSIERROR;
                }
            // If the base is physical and the derived class is drawing, then make the derived class Physical.  We will try to turn any instances into aspects
            else if (BisConversionRule::ToDrawingGraphic == existingRule && BisConversionRule::ToPhysicalElement == incomingRule)
                {
                if (BSISUCCESS != V8ECClassInfo::Update(converter, existingV8ClassId, BisConversionRule::ToPhysicalElement, true))
                    return BSIERROR;
                }
            else if (BisConversionRule::ToGroup == incomingRule && (BisConversionRule::ToPhysicalElement == existingRule || BisConversionRule::ToDrawingGraphic == existingRule))
                {
                BisConversionRule tempRule;
                BECN::ECClassId incomingV8ClassId;
                ECClassName incomingV8ClassName(*incomingClass);
                if (V8ECClassInfo::TryFind(incomingV8ClassId, tempRule, context.GetDgnDb(), incomingV8ClassName, hasSecondary))
                    {
                    if (BSISUCCESS != V8ECClassInfo::Update(converter, incomingV8ClassId, existingRule))
                        return BSIERROR;
                    incomingRule = existingRule;
                    if (BSISUCCESS != CheckBaseAndDerivedClassesForBisification(context, ecClass, incomingRule, ecClass->GetDerivedClasses(), false))
                        return BSIERROR;
                    }
                }
            else if (BisConversionRule::ToGroup == existingRule && (BisConversionRule::ToPhysicalElement == incomingRule || BisConversionRule::ToDrawingGraphic == incomingRule))
                {
                if (BSISUCCESS != V8ECClassInfo::Update(converter, existingV8ClassId, incomingRule))
                    return BSIERROR;
                if (BSISUCCESS != CheckBaseAndDerivedClassesForBisification(context, ecClass, incomingRule, ecClass->GetDerivedClasses(), false))
                    return BSIERROR;
                }
            }
        else if (BSISUCCESS != V8ECClassInfo::Insert(converter, v8ClassName, incomingRule))
            return BSIERROR;
        if (BSISUCCESS != CheckBaseAndDerivedClassesForBisification(context, ecClass, incomingRule, isBaseClassCheck ? ecClass->GetBaseClasses() : ecClass->GetDerivedClasses(), isBaseClassCheck))
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
    DynamicSchemaGenerator& converter = context.GetConverter();
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
            if (BSISUCCESS != BisConversionRuleHelper::ConvertToBisConversionRule(rule, *v8Class))
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
// @bsimethod                                   Carole.MacDonald            09/2017
//---------------+---------------+---------------+---------------+---------------+-------
void BisClassConverter::CheckForMixinConversion(SchemaConversionContext& context, BECN::ECClassR inputClass)
    {
    BECN::ECSchemaR targetSchema = inputClass.GetSchemaR();
    if (ShouldConvertECClassToMixin(targetSchema, inputClass, context))
        {
        ECClassP appliesTo;
        if (BSISUCCESS == FindAppliesToClass(appliesTo, context, targetSchema, inputClass))
            ConvertECClassToMixin(targetSchema, inputClass, *appliesTo);
        }

    for (BECN::ECClassP childClass : inputClass.GetDerivedClasses())
        {
        CheckForMixinConversion(context, *childClass);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Simi.Hartstein      04/2017
//---------------------------------------------------------------------------------------
//static
bool BisClassConverter::ShouldConvertECClassToMixin(BECN::ECSchemaR targetSchema, BECN::ECClassR inputClass, SchemaConversionContext& context)
    {
    if (inputClass.GetClassModifier() != ECClassModifier::Abstract)
        return false;

    SchemaConversionContext::MixinContext* mixinContext = context.GetMixinContext(targetSchema);
    if (mixinContext == nullptr)
        return false;

    BECN::ECClassCP baseInterface = mixinContext->first;
    BECN::ECClassCP baseObject = mixinContext->second;
    if (baseInterface == nullptr || baseObject == nullptr)
        return false;

    // check base class
    for (auto baseClass : inputClass.GetBaseClasses())
        {
        if (baseClass->GetSchemaR().GetName().EqualsI(BIS_ECSCHEMA_NAME))
            continue;
        if (baseClass->IsEntityClass())
            {
            auto baseEntityClass = baseClass->GetEntityClassP();
            if (!baseEntityClass->IsMixin() && !ShouldConvertECClassToMixin(baseEntityClass->GetSchemaR(), *baseEntityClass, context))
                return false;
            }
        else
            return false;
        }

    if (!inputClass.Is(baseInterface) || inputClass.Is(baseObject))
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Simi.Hartstein      04/2017
//---------------------------------------------------------------------------------------
//static
void BisClassConverter::FindCommonBaseClass(ECClassP &commonClass, ECClassP currentClass, ECBaseClassesList const& classes, const bvector<ECClassCP> propagationFilter)
    {
    ECClassP tempCommonClass = currentClass;
    for (const auto &secondConstraint : classes)
        {
        ECClassCP secondClass = secondConstraint;
        ECEntityClassCP asEntity = secondClass->GetEntityClassCP();
        if (nullptr != asEntity && asEntity->IsMixin() && asEntity->GetAppliesToClass()->Is(tempCommonClass->GetEntityClassCP()))
            continue;
        if (secondClass->Is(tempCommonClass))
            continue;

        for (const auto baseClass : tempCommonClass->GetBaseClasses())
            {
            bool shouldPropagate = false;
            for (const auto filterClass : propagationFilter)
                {
                if (baseClass->Is(filterClass))
                    {
                    shouldPropagate = true;
                    break;
                    }
                }
            if (!shouldPropagate)
                continue;
            
            FindCommonBaseClass(commonClass, baseClass->GetEntityClassP(), classes, propagationFilter);
            if (commonClass != nullptr)
                return;
            }

        tempCommonClass = nullptr;
        break;
        }

    if (nullptr != tempCommonClass)
        commonClass = tempCommonClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Simi.Hartstein      04/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisClassConverter::FindAppliesToClass(BECN::ECClassP& appliesTo, SchemaConversionContext& context, BECN::ECSchemaR targetSchema, BECN::ECClassR mixinClass)
    {
    auto appliesToMap = context.GetMixinAppliesToMapping();
    if (appliesToMap.find(mixinClass.GetEntityClassCP()) != appliesToMap.end())
        {
        appliesTo = appliesToMap[mixinClass.GetEntityClassP()];
        return BSISUCCESS;
        }

    SchemaConversionContext::MixinContext* mixinContext = context.GetMixinContext(targetSchema);
    if (nullptr == mixinContext)
        return BSIERROR;

    BECN::ECClassCP baseObject = mixinContext->second;
    if (baseObject == nullptr)
        return BSIERROR;

    bvector<ECClassCP> propagationFilter;
    propagationFilter.push_back(baseObject);

    BECN::ECDerivedClassesList derivedClasses = mixinClass.GetDerivedClasses();
    BECN::ECDerivedClassesList searchClasses;
    for (auto derived : derivedClasses)
        {
        BECN::ECClassP derivedAppliesTo;
        // if concrete class
        if (derived->Is(baseObject))
            searchClasses.push_back(derived);
        else if (BSISUCCESS == FindAppliesToClass(derivedAppliesTo, context, derived->GetSchemaR(), *derived))
            searchClasses.push_back(derivedAppliesTo);
        else
            return BSIERROR;
        }
        
    if (searchClasses.empty())
        appliesTo = targetSchema.GetClassP(baseObject->GetName().c_str());
    else
        FindCommonBaseClass(appliesTo, searchClasses.front(), searchClasses, propagationFilter);
    
    if (appliesTo == nullptr)
        return BSIERROR;
    
    return context.AddMixinAppliesToMapping(mixinClass.GetEntityClassCP(), appliesTo);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Simi.Hartstein      04/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisClassConverter::ConvertECClassToMixin(BECN::ECSchemaR targetSchema, BECN::ECClassR inputClass, BECN::ECClassCR appliesTo)
    {
    if (ECClassModifier::Abstract != inputClass.GetClassModifier())
        return BSIERROR;

    IECInstancePtr mixinInstance = CoreCustomAttributeHelper::CreateCustomAttributeInstance("IsMixin");
    if (!mixinInstance.IsValid())
        return BSIERROR;

    auto& coreCA = mixinInstance->GetClass().GetSchema();
    if (!BECN::ECSchema::IsSchemaReferenced(targetSchema, coreCA))
        targetSchema.AddReferencedSchema(const_cast<ECSchemaR>(coreCA));
    ECValue appliesToClass(BECN::ECClass::GetQualifiedClassName(targetSchema, appliesTo).c_str());
    
    BECN::ECObjectsStatus status;
    
    status = mixinInstance->SetValue("AppliesToEntityClass", appliesToClass);
    if (BECN::ECObjectsStatus::Success != status)
        return BSIERROR;

    status = inputClass.SetCustomAttribute(*mixinInstance);
    if (BECN::ECObjectsStatus::Success != status)
        return BSIERROR;

    for (auto baseClass : inputClass.GetBaseClasses())
        {
        if (baseClass->GetSchemaR().GetName().EqualsI(BIS_ECSCHEMA_NAME))
            inputClass.RemoveBaseClass(*baseClass);
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
        Utf8CP aspectSuffix = BisConversionRuleHelper::GetAspectClassSuffix(BisConversionRule::ToAspectOnly);
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
BentleyStatus BisClassConverter::ConvertECRelationshipClass(ECClassRemovalContext& removeContext, ECN::ECRelationshipClassR inputClass, ECN::ECSchemaReadContextP syncContext)
    {
    RemoveDuplicateClassMapCustomAttributes(inputClass);

    SchemaConversionContext& context = removeContext.SchemaContext();
    ProcessConstraints(inputClass, context.GetDefaultConstraintClass(), context);

    bool ignoreBisBase = false;
    ECN::SchemaKey conversionKey(Utf8String(inputClass.GetSchema().GetName().c_str()).append("_DgnDbSync").c_str(), inputClass.GetSchema().GetVersionRead(), inputClass.GetSchema().GetVersionMinor());
    ECN::ECSchemaPtr conversionSchema = syncContext->LocateSchema(conversionKey, ECN::SchemaMatchType::LatestReadCompatible);
    if (!conversionSchema.IsValid())
        conversionSchema = syncContext->LocateSchema(conversionKey, ECN::SchemaMatchType::Latest);

    // Since abstractness was ignored in V8, remove all abstract flags
    if (inputClass.GetClassModifier() == ECClassModifier::Abstract)
        inputClass.SetClassModifier(ECClassModifier::None);

    //if (conversionSchema.IsValid())
    //    {
    //    ECN::ECClassCP ecClass = conversionSchema->GetClassCP(inputClass.GetName().c_str());
    //    if (nullptr != ecClass)
    //        ignoreBisBase = ecClass->GetCustomAttribute("ForceIgnoreRelationshipBISBaseClass") != nullptr;
    //    }

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

                if (ECObjectsStatus::Success != inputClass.SetStrength(newStrength))
                    {
                    if (inputClass.HasBaseClasses())
                        {
                        inputClass.RemoveBaseClasses();
                        inputClass.SetStrength(newStrength);
                        }
                    }
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

    // If there is no BisCore base class, then ECDb reports this error: It violates against the 'No additional link tables' policy which means that relationship classes with 'Link table' mapping must subclass from relationship classes defined in the ECSchema BisCore
    // The solution is: The only way it can be converted to a Logical FK relationship is to add a navigation property to this relationship on Target side. Navigation property is prerequisite for EndTable relationship.
    if (inputClass.GetBaseClasses().size() == 0)
        {
        // 1:N->nav prop on target side
        // N:1->nav prop on source side
        // 1:1 and StrengthDirection == Forward->nav prop on target side
        // 1:1 and StrengthDirection == Backward->nav prop on source side
        if (inputClass.GetSource().GetMultiplicity().GetUpperLimit() == 1)
            {
            if (inputClass.GetTarget().GetMultiplicity().IsUpperLimitUnbounded() || (inputClass.GetTarget().GetMultiplicity().GetUpperLimit() == 1 && inputClass.GetStrengthDirection() == ECRelatedInstanceDirection::Forward))
                {
                bvector<ECEntityClassP> toRemove;
                for (ECClassCP constraintClass : inputClass.GetTarget().GetConstraintClasses())
                    {
                    ECEntityClassP target = const_cast<ECEntityClassP>(constraintClass->GetEntityClassCP());
                    NavigationECPropertyP navProp = nullptr;
                    if (ECObjectsStatus::Success != target->CreateNavigationProperty(navProp, inputClass.GetName(), inputClass, ECRelatedInstanceDirection::Backward, false))
                        toRemove.push_back(target);
                    }
                for (ECEntityClassP constraintClass : toRemove)
                    inputClass.GetTarget().RemoveClass(*constraintClass);
                if (inputClass.GetTarget().GetConstraintClasses().size() == 0)
                    removeContext.AddClassToRemove(inputClass);
                }
            }
        else if (inputClass.GetTarget().GetMultiplicity().GetUpperLimit() == 1)
            {
            if (inputClass.GetSource().GetMultiplicity().IsUpperLimitUnbounded() || (inputClass.GetSource().GetMultiplicity().GetUpperLimit() == 1 && inputClass.GetStrengthDirection() == ECRelatedInstanceDirection::Backward))
                {
                bvector<ECEntityClassP> toRemove;
                for (ECClassCP constraintClass : inputClass.GetSource().GetConstraintClasses())
                    {
                    ECEntityClassP source = const_cast<ECEntityClassP>(constraintClass->GetEntityClassCP());
                    NavigationECPropertyP navProp = nullptr;
                    if (ECObjectsStatus::Success != source->CreateNavigationProperty(navProp, inputClass.GetName(), inputClass, ECRelatedInstanceDirection::Forward, false))
                        toRemove.push_back(source);
                    }
                for (ECEntityClassP constraintClass : toRemove)
                    inputClass.GetSource().RemoveClass(*constraintClass);
                if (inputClass.GetSource().GetConstraintClasses().size() == 0)
                    removeContext.AddClassToRemove(inputClass);
                }
            }
        else
            {
            inputClass.GetSource().SetMultiplicity(BECN::RelationshipMultiplicity::OneOne());
            inputClass.GetTarget().SetMultiplicity(BECN::RelationshipMultiplicity::OneMany());
            bvector<ECEntityClassP> toRemove;
            for (ECClassCP constraintClass : inputClass.GetTarget().GetConstraintClasses())
                {
                ECEntityClassP target = const_cast<ECEntityClassP>(constraintClass->GetEntityClassCP());
                NavigationECPropertyP navProp = nullptr;
                if (ECObjectsStatus::Success != target->CreateNavigationProperty(navProp, inputClass.GetName(), inputClass, ECRelatedInstanceDirection::Backward, false))
                    toRemove.push_back(target);
                }
            for (ECEntityClassP constraintClass : toRemove)
                inputClass.GetTarget().RemoveClass(*constraintClass);
            if (inputClass.GetTarget().GetConstraintClasses().size() == 0)
                removeContext.AddClassToRemove(inputClass);
            }
        }

    // if the relationship class is a link table, it cannot have any properties on it
    if (inputClass.GetBaseClasses().size() == 0 || inputClass.Is(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsMultiAspects))
        {
        bvector<Utf8CP> propertyNames;
        for (BECN::ECPropertyP prop : inputClass.GetProperties(false))
            propertyNames.push_back(prop->GetName().c_str());

        for (Utf8CP propName : propertyNames)
            inputClass.RemoveProperty(propName);
        }

    bvector<ECClassP> derivedClasses(inputClass.GetDerivedClasses());
    for (BECN::ECClassP childClass : derivedClasses)
        {
        BECN::ECRelationshipClassP relClass = childClass->GetRelationshipClassP();
        ECClassName childClassName(*childClass);
        if (BSISUCCESS != ConvertECRelationshipClass(removeContext, *relClass, syncContext))
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
        {
        IECInstancePtr overwriteCA = constraint.GetCustomAttribute("OverwriteCardinality");
        if (overwriteCA.IsValid())
            {
            ECValue cardinality;
            if (ECObjectsStatus::Success == overwriteCA->GetValue(cardinality, "Cardinality"))
                {
                constraint.SetCardinality(cardinality.GetUtf8CP());
                }
            else
                constraint.SetMultiplicity(BECN::RelationshipMultiplicity::ZeroMany());
            }
        else if (relClass.GetSchema().GetName().EqualsIAscii("CivilSchema_iModel"))
            constraint.SetMultiplicity(BECN::RelationshipMultiplicity::ZeroOne());
        else
            constraint.SetMultiplicity(BECN::RelationshipMultiplicity::ZeroMany());
        }

    if (ContainsAnyClass(constraint.GetConstraintClasses()))
        {
        constraint.RemoveConstraintClasses();
        Utf8String msg;
        msg.Sprintf("ECRelationshipClass '%s' contains 'AnyClass' as a constraint.  AnyClass is no longer supported and will be replaced with '%s.'",
                    relClass.GetFullName(), defaultConstraintClass->GetName().c_str());
        context.ReportIssue(Converter::IssueSeverity::Info, msg.c_str());
        }
    
    // If the constraint is the source, replace all aspect constraints with the element class it modifies
    // If the constraint is the target, and the first constraint class is an element, replace any subsequent aspect class with the corresponding element class
    bool haveFirstType = false;
    bool firstIsElement = true;
    bvector<ECClassCP> constraintsToRemove;
    for (auto constraintClass : constraint.GetConstraintClasses())
        {
        if (!haveFirstType)
            {
            haveFirstType = true;
            firstIsElement = constraintClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_Element);
            }
        if (isSource)
            {
            if (!constraintClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_Element))
                constraintsToRemove.push_back(constraintClass);
            }
        else if (firstIsElement != constraintClass->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_Element))
            constraintsToRemove.push_back(constraintClass);
        }

    for (ECClassCP ecClass : constraintsToRemove)
        constraint.RemoveClass(*ecClass->GetEntityClassCP());

    // If we removed any aspect classes as a constraint, then we need to add the base Element class as a constraint.  This is because we are 
    // using the element that owns the aspect as the relationship constraint but we have no way of knowing what class that is.  Therefore, we
    // can only use the most generic class as the constraint.
    if (constraintsToRemove.size() > 0)
        {
        constraint.AddClass(*defaultConstraintClass);
        constraint.SetIsPolymorphic(true);
        }

    constraintsToRemove.clear();
    bvector<ECClassCP> constraintsToAdd;

    if (constraint.GetIsPolymorphic())
        {
        bmap<Utf8String, ECN::ECSchemaP> const& contextSchemas = context.GetSchemas();
        // In the case of flattened schemas, we have broken polymorphism so need to fix that
        for (auto constraintClass : constraint.GetConstraintClasses())
            {
            ECN::ECClassP nonConstConstraint = const_cast<ECN::ECClassP>(constraintClass);
            // Need to find a common base class between this constraint and any derived classes that were removed
            ECN::IECInstancePtr droppedInstance = constraintClass->GetCustomAttributeLocal("ECv3ConversionAttributes", "OldDerivedClasses");
            if (droppedInstance.IsValid())
                {
                ECN::ECValue v;
                droppedInstance->GetValue(v, "Classes");
                if (!v.IsNull())
                    {
                    bvector<Utf8String> classNames;
                    BeStringUtilities::Split(v.GetUtf8CP(), ";", classNames);
                    bvector<ECN::ECClassCP> searchClasses;
                    for (Utf8String className : classNames)
                        {
                        bvector<Utf8String> components;
                        BeStringUtilities::Split(className.c_str(), ":", components);
                        if (components.size() != 2)
                            continue;
                        bmap<Utf8String, ECN::ECSchemaP>::const_iterator iter = contextSchemas.find(components[0]);
                        if (contextSchemas.end() != iter)
                            {
                            ECN::ECSchemaP droppedSchema = iter->second;
                            ECN::ECClassP dropped = droppedSchema->GetClassP(components[1].c_str());
                            if (nullptr != dropped)
                                {
                                if (dropped->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementAspect))
                                    {
                                    searchClasses.push_back(defaultConstraintClass);
                                    }
                                else
                                    {
                                    ECN::ECClassP nonConst = const_cast<ECN::ECClassP>(dropped);
                                    searchClasses.push_back(nonConst);
                                    }
                                }
                            }
                        }
                    ECN::ECEntityClassCP commonClass = nullptr;
                    ECN::ECClass::FindCommonBaseClass(commonClass, constraintClass->GetEntityClassCP(), searchClasses);
                    if (commonClass != nullptr)
                        {
                        constraintsToRemove.push_back(nonConstConstraint);
                        constraintsToAdd.push_back(commonClass);
                        }
                    }
                }
            }
        for (ECClassCP toRemove : constraintsToRemove)
            constraint.RemoveClass(*toRemove->GetEntityClassCP());
        for (ECClassCP toAdd : constraintsToAdd)
            constraint.AddClass(*toAdd->GetEntityClassCP());
        }

    if (0 == constraint.GetConstraintClasses().size())
        {
        if (relClass.HasBaseClasses())
            {
            ECRelationshipClassCP baseClass = relClass.GetBaseClasses()[0]->GetRelationshipClassCP();
            ECRelationshipConstraintR baseConstraint = (isSource) ? baseClass->GetSource() : baseClass->GetTarget();
            constraint.AddClass(*(baseConstraint.GetConstraintClasses()[0]->GetEntityClassCP()));
            }
        else if (ECObjectsStatus::SchemaNotFound == constraint.AddClass(*defaultConstraintClass))
            {
            relClass.GetSchemaR().AddReferencedSchema(defaultConstraintClass->GetSchemaR());
            constraint.AddClass(*defaultConstraintClass);
            }
        }
    else if (relClass.HasBaseClasses())
        {
        ECRelationshipClassCP baseClass = relClass.GetBaseClasses()[0]->GetRelationshipClassCP();
        ECRelationshipConstraintR baseConstraint = (isSource) ? baseClass->GetSource() : baseClass->GetTarget();

        bool removed = false;
        for (auto constraintClass : constraint.GetConstraintClasses())
            {
            for (auto baseConstraintClass : baseConstraint.GetConstraintClasses())
                {
                if (!constraintClass->Is(baseConstraintClass))
                    {
                    removed = true;
                    relClass.RemoveBaseClass(*baseClass);
                    break;
                    }
                }
            if (removed)
                break;
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

    // Need to create a PresentationRuleSet for all aspect classes.
    bmap<Utf8String, bvector<Utf8String>> aspectClassNames;
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

        auto perSchema = aspectClassNames.find(aspectClass->GetSchema().GetName());
        if (perSchema == aspectClassNames.end())
            {
            bvector<Utf8String> names;
            names.push_back(aspectClass->GetName());
            aspectClassNames[aspectClass->GetSchema().GetName()] = names;
            }
        else
            perSchema->second.push_back(aspectClass->GetName());

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

                ECN::ECClassP nonConstBase = const_cast<ECN::ECClassP>(baseClass);
                AddDroppedDerivedClass(nonConstBase, aspectOnlyClass);
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
        elementBaseClass = context.GetBaseClass(BisConversionRuleHelper::GetElementBisBaseClassName(conversionRule));
        BeAssert(elementBaseClass != nullptr);
        }
    elementAspectBaseClass = context.GetBaseClass(BisConversionRuleHelper::GetElementAspectBisBaseClassName(BisConversionRule::ToAspectOnly));

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

// Create the pseudo polymorphic hierarchy by keeping track of any derived class that was lost.
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
void BisClassConverter::AddDroppedDerivedClass(ECN::ECClassP baseClass, ECN::ECClassP derivedClass)
    {
    ECN::IECInstancePtr droppedInstance = baseClass->GetCustomAttributeLocal("ECv3ConversionAttributes", "OldDerivedClasses");
    if (!droppedInstance.IsValid())
        droppedInstance = ECN::ConversionCustomAttributeHelper::CreateCustomAttributeInstance("OldDerivedClasses");
    if (!droppedInstance.IsValid())
        {
        LOG.warningv("Failed to create 'OldDerivedClasses' custom attribute for ECClass '%s'", baseClass->GetFullName());
        return;
        }
    ECN::ECValue v;
    droppedInstance->GetValue(v, "Classes");
    Utf8String classes("");
    if (!v.IsNull())
        classes = Utf8String(v.GetUtf8CP()).append(";");

    classes.append(derivedClass->GetFullName());

    v.SetUtf8CP(classes.c_str());
    if (ECN::ECObjectsStatus::Success != droppedInstance->SetValue("Classes", v))
        {
        LOG.warningv("Failed to create 'OldDerivedClasses' custom attribute for the ECClass '%s' with 'Classes' set to '%s'.", baseClass->GetFullName(), classes.c_str());
        return;
        }

    if (!ECN::ECSchema::IsSchemaReferenced(baseClass->GetSchemaR(), droppedInstance->GetClass().GetSchema()))
        {
        ECN::ECClassP nonConstClass = const_cast<ECN::ECClassP>(&droppedInstance->GetClass());
        if (ECN::ECObjectsStatus::Success != baseClass->GetSchemaR().AddReferencedSchema(nonConstClass->GetSchemaR()))
            {
            LOG.warningv("Failed to add %s as a referenced schema to %s.", droppedInstance->GetClass().GetSchema().GetName().c_str(), baseClass->GetSchemaR().GetName().c_str());
            LOG.warningv("Failed to add 'OldDerivedClasses' custom attribute to ECClass '%s'.", baseClass->GetFullName());
            return;
            }
        }

    if (ECN::ECObjectsStatus::Success != baseClass->SetCustomAttribute(*droppedInstance))
        {
        LOG.warningv("Failed to add 'OldDerivedClasses' custom attribute, with 'PropertyMapping' set to '%s', to ECClass '%s'.", classes.c_str(), baseClass->GetFullName());
        return;
        }

    LOG.debugv("Successfully added OldDerivedClasses custom attribute to ECClass '%s'", baseClass->GetFullName());
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
            ECN::ECPropertyP renamedProperty = nullptr;
            ecClass.RenameConflictProperty(prop, true, renamedProperty);
            prop = renamedProperty;
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2017
//---------------+---------------+---------------+---------------+---------------+-------
void findBase(ECClassCP &inputClass)
    {
    ECClassCP test = inputClass;
    while (true)
        {
        if (!test->HasBaseClasses())
            break;
        ECClassCP t2 = *test->GetBaseClasses().begin();
        if (t2->GetName().Equals(inputClass->GetName()))
            test = t2;
        else
            break;
        }
    inputClass = test;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Simi.Hartstein      06/2017
//---------------------------------------------------------------------------------------
//static
BentleyStatus BisClassConverter::CreateMixinContext(SchemaConversionContext::MixinContext& mixinContext, DynamicSchemaGenerator& converter, ECSchemaReadContext& syncReadContext, ECSchemaP schema, bool autoDetect)
    {
    // autodetect SmartPlant schema
    if (autoDetect && schema->GetName().StartsWithI("SP3D"))
        {
        ECClassCP baseInterface = schema->GetClassCP("BaseInterface");
        ECClassCP baseObject = schema->GetClassCP("BaseObject");
        if (nullptr != baseInterface)
            findBase(baseInterface);

        if (nullptr != baseObject)
            findBase(baseObject);

        if (baseInterface != nullptr && baseObject != nullptr)
            {
            mixinContext = SchemaConversionContext::MixinContext(baseInterface, baseObject);
            return BSISUCCESS;
            }
        }

    ECN::SchemaKey conversionKey(Utf8String(schema->GetName()).append("_DgnDbSync").c_str(), 1, 0);
    ECN::ECSchemaPtr conversionSchema = syncReadContext.LocateSchema(conversionKey, ECN::SchemaMatchType::Latest);
    if (conversionSchema == nullptr)
        return BSIERROR;

    ECN::IECInstancePtr mixinAttr = conversionSchema->GetCustomAttribute("DgnDbSyncV8", "MixinConversionContext");
    if (mixinAttr == nullptr)
        return BSIERROR;

    auto reportMixinContextError = [](Utf8CP action, Utf8CP propVal, ECSchemaPtr schema, DynamicSchemaGenerator& converter)
        {
        Utf8String error;
        error.Sprintf("Could not %s '%s' specified in mixin context of ECSchema '%s'", action, propVal, schema->GetFullSchemaName().c_str());
        converter.ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Error(), error.c_str());
        return BSIERROR;
        };

    ECN::ECValue baseInterfaceValue, baseObjectValue;
    ECN::ECObjectsStatus status = mixinAttr->GetValue(baseInterfaceValue, "BaseInterfaceClassName");
    if (ECN::ECObjectsStatus::Success != status)
        return reportMixinContextError("get value of property", "BaseInterfaceClassName", conversionSchema, converter);

    status = mixinAttr->GetValue(baseObjectValue, "BaseObjectClassName");
    if (ECN::ECObjectsStatus::Success != status)
        return reportMixinContextError("get value of property", "BaseObjectClassName", conversionSchema, converter);

    Utf8String alias, className;
    status = ECClass::ParseClassName(alias, className, baseInterfaceValue.GetUtf8CP());
    ECSchemaCP targetSchema = alias.empty() ? schema : schema->GetSchemaByAliasP(alias);
    if (ECObjectsStatus::Success != status || nullptr == targetSchema)
        return reportMixinContextError("find BaseInterface class", baseInterfaceValue.GetUtf8CP(), conversionSchema, converter);

    BECN::ECClassCP baseInterface = targetSchema->GetClassCP(className.c_str());
    if (nullptr == baseInterface)
        return reportMixinContextError("find BaseInterface class", baseInterfaceValue.GetUtf8CP(), conversionSchema, converter);

    status = ECClass::ParseClassName(alias, className, baseObjectValue.GetUtf8CP());
    targetSchema = alias.empty() ? schema : schema->GetSchemaByAliasP(alias);
    if (ECObjectsStatus::Success != status || nullptr == targetSchema)
        return reportMixinContextError("find BaseObject class", baseObjectValue.GetUtf8CP(), conversionSchema, converter);

    ECClassCP baseObject = targetSchema->GetClassCP(className.c_str());
    if (nullptr == baseObject)
        return reportMixinContextError("find BaseObject class", baseObjectValue.GetUtf8CP(), conversionSchema, converter);

    mixinContext = SchemaConversionContext::MixinContext(baseInterface, baseObject);
    return BSISUCCESS;
    }

//****************************************************************************************
// ElementConverter::SchemaConversionContext
//****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     03/2015
//---------------------------------------------------------------------------------------
BisClassConverter::SchemaConversionContext::SchemaConversionContext(DynamicSchemaGenerator& converter, BECN::ECSchemaReadContext& schemaReadContext, BECN::ECSchemaReadContext& syncReadContext, bool autoDetectMixinParams)
    : m_converter(converter), m_domainRelationshipBaseClass(nullptr), m_defaultConstraintClass(nullptr)
    {
    //need a schema map keyed on name only. SchemaCache doesn't support that
    bvector<BECN::ECSchemaP> schemaCache;
    schemaReadContext.GetCache().GetSchemas(schemaCache);

    for (auto schema : schemaCache)
        m_inputSchemaMap[Utf8String(schema->GetName())] = schema;


    BECN::SchemaKey dgnSchemaKey(BIS_ECSCHEMA_NAME, 1, 0);
    BECN::ECSchemaPtr dgnSchema = schemaReadContext.LocateSchema(dgnSchemaKey, BECN::SchemaMatchType::Latest);
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
    BECN::ECSchemaPtr genericSchema = schemaReadContext.LocateSchema(genericSchemaKey, BECN::SchemaMatchType::Latest);
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
    BECN::ECSchemaPtr functionalSchema = schemaReadContext.LocateSchema(functionalSchemaKey, BECN::SchemaMatchType::Latest);
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

    auto reportMixinContextError = [](Utf8CP action, Utf8CP propVal, ECSchemaPtr schema, DynamicSchemaGenerator& converter)
        {
        Utf8String error;
        error.Sprintf("Could not %s '%s' specified in mixin context of ECSchema '%s'", action, propVal, schema->GetFullSchemaName().c_str());
        converter.ReportIssue(Converter::IssueSeverity::Error, Converter::IssueCategory::Sync(), Converter::Issue::Error(), error.c_str());
        };

    for (auto schema : schemaCache)
        {
        MixinContext mixinContext;
        if (BSISUCCESS == CreateMixinContext(mixinContext, converter, syncReadContext, schema, autoDetectMixinParams))
            m_mixinContextCache[schema] = mixinContext;
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
        ECEntityClassCP asEntity = constraintClass->GetEntityClassCP();
        if (asEntity != nullptr && asEntity->IsMixin())
            {
            if (!asEntity->GetAppliesToClass()->Is(GetDefaultConstraintClass()))
                return nullptr;
            }
        else if (!constraintClass->Is(GetDefaultConstraintClass()))
            return nullptr;
        }

    ECClassCP abstractConstraint = inputClass.GetTarget().GetAbstractConstraint();
    if (nullptr == abstractConstraint)
        {
        if (inputClass.GetTarget().GetConstraintClasses().size() > 0)
            {
            if (inputClass.GetTarget().GetConstraintClasses()[0]->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_Element))
                return m_domainRelationshipBaseClass;
            else
                return m_aspectRelationshipBaseClass;
            }
        return m_domainRelationshipBaseClass;
        }
    else if (abstractConstraint->Is(GetDefaultConstraintClass()) || (abstractConstraint->IsEntityClass() && abstractConstraint->GetEntityClassCP()->IsMixin()))
        return m_domainRelationshipBaseClass;
    return m_aspectRelationshipBaseClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Simi.Hartstein      05/2017
//---------------------------------------------------------------------------------------
BisClassConverter::SchemaConversionContext::MixinContext* BisClassConverter::SchemaConversionContext::GetMixinContext(BECN::ECSchemaCR schema)
    {
    auto indexIt = m_mixinContextCache.find(&schema);
    return (indexIt == m_mixinContextCache.end() ? nullptr : &(indexIt->second));
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
// @bsimethod                                                 Simi.Hartstein      04/2017
//---------------------------------------------------------------------------------------
BentleyStatus BisClassConverter::SchemaConversionContext::AddMixinAppliesToMapping(BECN::ECClassCP mixinClass, BECN::ECClassP appliesToClass)
    {
    BeAssert(m_mixinAppliesToMap.find(mixinClass) == m_mixinAppliesToMap.end());
    m_mixinAppliesToMap.Insert(mixinClass, appliesToClass);
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Simi.Hartstein      05/2017
//---------------------------------------------------------------------------------------
BentleyStatus BisClassConverter::SchemaConversionContext::AddMixinContextMapping(BECN::ECSchemaCP schema, BisClassConverter::SchemaConversionContext::MixinContext context)
    {
    BeAssert(m_mixinContextCache.find(schema) == m_mixinContextCache.end());
    m_mixinContextCache.Insert(schema, context);
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
        schema.GetName().EqualsI(DGNDBSYNCV8_ECSCHEMA_NAME) || schema.GetName().EqualsI(BIS_ECSCHEMA_NAME) || 
        schema.GetName().EqualsIAscii("Generic") || schema.GetName().EqualsIAscii("Functional") || 
        schema.GetName().StartsWithI("ecdb") || schema.GetName().EqualsIAscii("ECv3ConversionAttributes");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool BisClassConverter::SchemaConversionContext::ExcludeSchemaFromBisification(Utf8StringCR schemaName)
    {
    return ECN::ECSchema::IsStandardSchema(schemaName) ||
        schemaName.EqualsI(DGNDBSYNCV8_ECSCHEMA_NAME) || schemaName.EqualsI(BIS_ECSCHEMA_NAME) ||
        schemaName.EqualsIAscii("Generic") || schemaName.EqualsIAscii("Functional") ||
        schemaName.StartsWithI("ecdb") || schemaName.EqualsIAscii("ECv3ConversionAttributes");
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

#if defined(NEEDSWORK_PERFORMANCE)
#endif
END_DGNDBSYNC_DGNV8_NAMESPACE
