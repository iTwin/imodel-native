/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/ECConversion.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ConverterInternal.h"
#include <Bentley/NonCopyableClass.h>

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

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
    static BisConversionRule ConvertToBisConversionRule(V8ElementType v8ElementType, DgnModel *targetModel, const bool namedGroupOwnsMembersFlag, bool isSecondaryInstancesClass = false);
    //! Determines the conversion rule based on the v8 ECClass
    //! @remarks The effective rule is a combination of both the instance and class based rule and will be computed
    //! by the converter
    static BentleyStatus ConvertToBisConversionRule(BisConversionRule&, Converter&, ECN::ECClassCR);

    static BentleyStatus TryDetermineElementAspectKind(ElementAspectKind&, BisConversionRule);

    static bool IsSecondaryInstance(BisConversionRule);
    static bool IgnoreInstance(BisConversionRule);

    static bool ClassNeedsBisification(BisConversionRule conversionRule);
    static Utf8CP ToString(BisConversionRule);
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

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      02/2015
//+===============+===============+===============+===============+===============+======
struct V8ECClassInfo
    {
private:
    V8ECClassInfo ();
    ~V8ECClassInfo ();

    static BentleyStatus DoInsert(DgnDbR, ECClassName const& v8ClassName, BisConversionRule);

public:
    static BentleyStatus Insert(Converter&, DgnV8EhCR, ECClassName const&, bool namedGroupOwnsMembers, bool isSecondaryInstancesClass, DgnModel *targetModel);

    static BentleyStatus InsertOrUpdate(Converter&, ECClassName const&, BisConversionRule);
    static BentleyStatus Insert(Converter&, ECClassName const&, BisConversionRule);
    static BentleyStatus Update(Converter&, ECN::ECClassId v8ClassId, BisConversionRule, bool hasSecondary = false);

    static bool TryFind(ECN::ECClassId& v8classId, BisConversionRule&, DgnDbR, ECClassName const&, bool& hasSecondary);
    static bool TryFind(BisConversionRule& rule, DgnDbR dgndb, ECClassName const& className, bool& hasSecondary) { ECN::ECClassId id; return TryFind(id, rule, dgndb, className, hasSecondary); }

    static BentleyStatus CreateTable(DgnDbR db);
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      02/2015
//+===============+===============+===============+===============+===============+======
struct ECInstanceInfo : NonCopyableClass
    {
private:
    ECInstanceInfo ();
    ~ECInstanceInfo ();

public:
    static BeSQLite::EC::ECInstanceKey Find (bool& isElement, DgnDbR db, SyncInfo::V8FileSyncInfoId fileId, V8ECInstanceKey const& v8Key);
    static BentleyStatus Insert (DgnDbR db, SyncInfo::V8FileSyncInfoId fileId, V8ECInstanceKey const& v8Key, BeSQLite::EC::ECInstanceKey const& key, bool isElement);
    static BentleyStatus CreateTable (DgnDbR db);
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

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2015
//+===============+===============+===============+===============+===============+======
struct V8NamedGroupInfo : NonCopyableClass
    {
private:
    static bmap<SyncInfo::V8FileSyncInfoId, bset<DgnV8Api::ElementId>> s_namedGroupsWithOwnershipHint;

    V8NamedGroupInfo();
    ~V8NamedGroupInfo();

public:
    static void AddNamedGroupWithOwnershipHint(DgnV8EhCR);
    static bool TryGetNamedGroupsWithOwnershipHint(bset<DgnV8Api::ElementId> const*&, SyncInfo::V8FileSyncInfoId);
    static void Reset();
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      02/2015
//+===============+===============+===============+===============+===============+======
struct V8ECSchemaXmlInfo
    {
public:
    struct Iterable : BeSQLite::DbTableIterator
        {
        struct Entry : BeSQLite::DbTableIterator::Entry, std::iterator <std::input_iterator_tag, Entry const>
            {
        private:
            friend struct Iterable;
            Entry (BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry (sql, isValid) {}

        public:
            ECN::SchemaKey GetSchemaKey () const;
            Utf8CP GetSchemaXml () const;
            SyncInfo::ECSchemaMappingType GetMappingType () const;
            Entry const& operator* () const {return *this;}
            };

        explicit Iterable (DgnDbCR db) : BeSQLite::DbTableIterator ((BeSQLite::DbCR) db)
            {}

        typedef Entry const_iterator;
        const_iterator begin () const;
        const_iterator end () const { return Entry (nullptr, false); }
        };

private:
    V8ECSchemaXmlInfo ();
    ~V8ECSchemaXmlInfo ();

public:
    static BeSQLite::DbResult Insert (DgnDbR db, ECN::ECSchemaId schemaId, BentleyApi::Utf8CP schemaXml);
    static BeSQLite::DbResult CreateTable (DgnDbR db);
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      10/2014
//+===============+===============+===============+===============+===============+======
struct ECSchemaXmlDeserializer : public ECN::IECSchemaLocater
    {
private:
    bmap<Utf8String, bvector<bpair<ECN::SchemaKey, Utf8String>>> m_schemaXmlMap;
    ECN::ECSchemaCache m_schemaCache;
    Converter& m_converter;

    virtual ECN::ECSchemaPtr _LocateSchema (ECN::SchemaKeyR key, ECN::SchemaMatchType matchType, ECN::ECSchemaReadContextR schemaContext) override;

public:
    ECSchemaXmlDeserializer (Converter& converter) : m_converter(converter) {}

    void AddSchemaXml(Utf8CP schemaName, ECN::SchemaKeyCR key, Utf8CP xml);
    BentleyStatus DeserializeSchemas (ECN::ECSchemaReadContextR, ECN::SchemaMatchType, Converter&);
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
