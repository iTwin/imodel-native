/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <DgnDbSync/DgnV8/Converter.h>

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

struct DynamicSchemaGenerator;

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      07/2015
//+===============+===============+===============+===============+===============+======
struct BisConversionRuleHelper
    {
public:
    enum class ElementAspectKind
        {
        ElementMultiAspect,
        ElementAspect
        };

private:
    BisConversionRuleHelper();
    ~BisConversionRuleHelper();

    static bool ContainsAnyClass(ECN::ECRelationshipConstraintClassList const&);

public:
    //! Determines the conversion rule based on characteristics of a given v8 ECInstance
    //! @remarks The effective rule is a combination of both the instance and class based rule and will be computed
    //! by the converter
    static BisConversionRule ConvertToBisConversionRule(V8ElementType v8ElementType, BisConversionTargetModelInfoCR targetModelInfo, const bool namedGroupOwnsMembersFlag, bool isSecondaryInstancesClass = false);
    //! Determines the conversion rule based on the v8 ECClass
    //! @remarks The effective rule is a combination of both the instance and class based rule and will be computed
    //! by the converter
    static BentleyStatus ConvertToBisConversionRule(BisConversionRule&, ECN::ECClassCR);

    static BentleyStatus TryDetermineElementAspectKind(ElementAspectKind&, BisConversionRule);

    static bool IsSecondaryInstance(BisConversionRule);
    static bool IgnoreInstance(BisConversionRule);

    static bool ClassNeedsBisification(BisConversionRule conversionRule);
    static Utf8CP ToString(BisConversionRule);

    static ECClassName GetElementBisBaseClassName(BisConversionRule, bool consider3dElementsAsGraphics);
    static ECClassName GetElementAspectBisBaseClassName(BisConversionRule);
    static Utf8CP GetAspectClassSuffix(BisConversionRule);
    };


//=======================================================================================
// @bsiclass                                                Krischan.Eberle      04/2015
//+===============+===============+===============+===============+===============+======
struct V8ElementTypeHelper
    {
private:
    V8ElementTypeHelper();
    ~V8ElementTypeHelper();

public:
    static V8ElementType GetType(DgnV8EhCR);
    static V8ElementType GetType(DgnV8Api::ElementRefBase const&);

    static Utf8CP ToString(V8ElementType);
    };

struct DgnV8Info
    {
    struct Spec : BeSQLite::PropertySpec
        {
        Spec(BentleyApi::Utf8CP nameSpace, BentleyApi::Utf8CP name, bool saveIfNull = false) : BeSQLite::PropertySpec(name, nameSpace, Mode::Normal, Compress::No, saveIfNull) {}
        };

    static Spec V8Class(BentleyApi::Utf8CP ecClass) { return Spec("dgn_V8Class", ecClass); }
    static Spec V8Schema(BentleyApi::Utf8CP ecSchema) { return Spec("dgn_V8Schema", ecSchema); }
    static Spec V8Expression(BentleyApi::Utf8CP fullClassName) { return Spec("dgn_V8Expression", fullClassName); }
    static Spec V8TagSet(BentleyApi::Utf8CP tagSetName) { return Spec("dgn_V8TagSet", tagSetName, true); }
    static Spec V8TagDefinition(BentleyApi::Utf8CP tagSetId) { return Spec("dgn_V8TagDef", tagSetId); }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      02/2015
//+===============+===============+===============+===============+===============+======
struct V8ECClassInfo
    {
private:
    V8ECClassInfo ();
    ~V8ECClassInfo ();

    static BentleyStatus Save(DgnDbR, ECClassName const& v8ClassName, BisConversionRule, bool hasSecondary = false);

public:
    static BentleyStatus InsertOrUpdate(DynamicSchemaGenerator&, DgnV8EhCR, ECClassName const&, bool namedGroupOwnsMembers, bool isSecondaryInstancesClass, BisConversionTargetModelInfoCR targetModelInfo);
    static BentleyStatus InsertOrUpdate(DynamicSchemaGenerator&, ECClassName const& className, BisConversionRule rule, bool hasSecondary = false);

    static bool TryFind(BisConversionRule& rule, bool& hasSecondary, DgnDbR dgndb, Utf8StringCR className);

    static BentleyStatus CreateTable(DgnDbR db);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
struct V8ElementSecondaryECClassInfo : NonCopyableClass
    {
private:
    V8ElementSecondaryECClassInfo();
    ~V8ElementSecondaryECClassInfo();

public:
    static BentleyStatus CreateTable(DgnDbR db);
    static BentleyStatus Insert(DgnDbR db, DgnV8EhCR el, ECClassName const&);
    static bool TryFind(DgnDbR db, DgnV8EhCR el, ECClassName const&);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2017
//---------------+---------------+---------------+---------------+---------------+-------
struct ElementClassToAspectClassMapping : NonCopyableClass
    {
private:
    ElementClassToAspectClassMapping();
    ~ElementClassToAspectClassMapping();

    static bool TryFind(DgnDbR db, DgnClassId elementId, ECN::ECClassId aspectId);

public:
    static BentleyStatus CreateTable(DgnDbR db);
    static BentleyStatus Insert(DgnDbR db, DgnClassId elementId, Utf8CP elementSchemaName, Utf8CP elementClassName, ECN::ECClassId aspectId, Utf8CP aspectSchemaName, Utf8CP aspectClassName);
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2015
//+===============+===============+===============+===============+===============+======
struct ECDiagnostics
    {
public:
    enum class Category
        {
        V8Instances,
        V8Relationships
        };
private:
    static const NativeLogging::SEVERITY s_severity = NativeLogging::LOG_DEBUG;
    static bmap<Category, NativeLogging::ILogger*> s_loggerMap;

    ECDiagnostics();
    ~ECDiagnostics();

    static Utf8CP ToBisTypeString(bool isConverted, bool isElement);

    static NativeLogging::ILogger& GetLogger(bool& isFirstCallForCategory, Category);

public:
    static void LogV8InstanceDiagnostics(DgnV8EhCR, V8ElementType, ECClassName const&, bool isSecondaryInstanceClass, BisConversionRule);
    static void LogV8RelationshipDiagnostics(DgnDbR, ECClassName const& v8RelClassName, V8ECInstanceKey const& sourceKey, bool v8SourceWasConverted, bool v8SourceConvertedToElement, V8ECInstanceKey const& targetKey, bool v8TargetWasConverted, bool v8TargetConvertedToElement);
    };

END_DGNDBSYNC_DGNV8_NAMESPACE
