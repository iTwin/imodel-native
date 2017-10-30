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
    static BisConversionRule ConvertToBisConversionRule(V8ElementType v8ElementType, DgnModel *targetModel, const bool namedGroupOwnsMembersFlag, bool isSecondaryInstancesClass = false);
    //! Determines the conversion rule based on the v8 ECClass
    //! @remarks The effective rule is a combination of both the instance and class based rule and will be computed
    //! by the converter
    static BentleyStatus ConvertToBisConversionRule(BisConversionRule&, ECN::ECClassCR);

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
    static BentleyStatus Insert(DynamicSchemaGenerator&, DgnV8EhCR, ECClassName const&, bool namedGroupOwnsMembers, bool isSecondaryInstancesClass, DgnModel *targetModel);

    static BentleyStatus InsertOrUpdate(DynamicSchemaGenerator&, ECClassName const&, BisConversionRule);
    static BentleyStatus Insert(DynamicSchemaGenerator&, ECClassName const&, BisConversionRule);
    static BentleyStatus Update(DynamicSchemaGenerator&, ECN::ECClassId v8ClassId, BisConversionRule, bool hasSecondary = false);

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
    DynamicSchemaGenerator& m_converter;

    virtual ECN::ECSchemaPtr _LocateSchema (ECN::SchemaKeyR key, ECN::SchemaMatchType matchType, ECN::ECSchemaReadContextR schemaContext) override;

public:
    ECSchemaXmlDeserializer (DynamicSchemaGenerator& converter) : m_converter(converter) {}

    void AddSchemaXml(Utf8CP schemaName, ECN::SchemaKeyCR key, Utf8CP xml);
    BentleyStatus DeserializeSchemas (ECN::ECSchemaReadContextR, ECN::SchemaMatchType);
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

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      05/2015
//+===============+===============+===============+===============+===============+======
struct DynamicSchemaGenerator
    {

#ifndef DOCUMENTATION_GENERATOR
    //=======================================================================================
    //!Convenience class that simplifies finalizing the schema conversion process. The destructor
    //! does all the work necessary to clean-up after schema conversion is done.
    // @bsiclass
    //=======================================================================================
    struct SchemaConversionScope : NonCopyableClass
        {
        private:
            DynamicSchemaGenerator& m_converter;
            bool m_succeeded;
        public:
            explicit SchemaConversionScope(DynamicSchemaGenerator&);
            ~SchemaConversionScope();
            void SetSucceeded() {m_succeeded = true;}
        };
#endif

    private:
    bool m_skipECContent = false;
    bool m_needReimportSchemas = false;
    bool m_ecConversionFailed = false;
    bool m_anyImported = false;
    Converter& m_converter;
    ECN::ECSchemaReadContextPtr m_schemaReadContext;
    ECN::ECSchemaReadContextPtr m_syncReadContext;

    void CheckECSchemasForModel(DgnV8ModelR, bmap<Utf8String, uint32_t>& syncInfoChecksums);
    BentleyStatus RetrieveV8ECSchemas(DgnV8ModelR v8rootModel);
    BentleyStatus RetrieveV8ECSchemas(DgnV8ModelR v8rootModel, DgnV8Api::ECSchemaPersistence persistence);
    static bool IsDynamicSchema(ECObjectsV8::ECSchemaCR schema);
    static bool IsWellKnownDynamicSchema(Bentley::Utf8StringCR schemaName);
    static bool IsDynamicSchema(Bentley::Utf8StringCR schemaName, Bentley::Utf8StringCR schemaXml);
    BentleyStatus ConsolidateV8ECSchemas();
    BentleyStatus MergeV8ECSchemas(struct ECSchemaXmlDeserializer& deserializer, bmap<Utf8String, bvector<bpair<ECN::SchemaKey, Utf8String>>> const& schemaXmlMap) const;
    BentleyStatus SupplementV8ECSchemas();
    BentleyStatus ConvertToBisBasedECSchemas();
    BentleyStatus ImportTargetECSchemas();
    void AnalyzeECContent(DgnV8ModelR, DgnModelP targetModel);
    BentleyStatus Analyze(DgnV8Api::ElementHandle const&, DgnModelP targetModel);
    BentleyStatus DoAnalyze(DgnV8Api::ElementHandle const&, DgnModelP targetModel);
    void InitializeECSchemaConversion();
    void FinalizeECSchemaConversion();
    static WCharCP GetV8TagSetDefinitionSchemaName() {return L"V8TagSetDefinitions";}

    SyncInfo& GetSyncInfo() {return m_converter.GetSyncInfo();}
    bool IsUpdating() const {return m_converter.IsUpdating();}

    void CheckNoECSchemaChanges(bset<DgnV8ModelP> const&);
    void BisifyV8Schemas(bset<DgnV8ModelP> const&);

    public:

    bool WasAborted() const {return m_converter.WasAborted();}
    void ReportProgress() const {m_converter.ReportProgress();}
    template<typename ...Args> void SetStepName(Converter::ProgressMessage::StringId a, Args... args) const {m_converter.SetStepName(a, args...);}
    void AddTasks(int32_t n) const {m_converter.AddTasks(n);}
    template<typename ...Args> void SetTaskName(Converter::ProgressMessage::StringId a, Args... args) const {m_converter.SetTaskName(a, args...);}
    void ReportError(Converter::IssueCategory::StringId a, Converter::Issue::StringId b, Utf8CP c) {m_converter.ReportError(a,b,c);}
    void ReportIssue(Converter::IssueSeverity a, Converter::IssueCategory::StringId b, Converter::Issue::StringId c, Utf8CP d, Utf8CP e = nullptr) {m_converter.ReportIssue(a,b,c,d,e);}
    template<typename ...Args> void ReportIssueV(Converter::IssueSeverity a, Converter::IssueCategory::StringId b, Converter::Issue::StringId c, Utf8CP d, Args... args) {m_converter.ReportIssueV(a,b,c,d,args...);}
    void ReportSyncInfoIssue(Converter::IssueSeverity a, Converter::IssueCategory::StringId b, Converter::Issue::StringId c, Utf8CP d) {m_converter.ReportSyncInfoIssue(a,b,c,d);}
    template<typename ...Args> BentleyStatus OnFatalError(Converter::IssueCategory::StringId a=Converter::IssueCategory::Unknown(), Converter::Issue::StringId b=Converter::Issue::FatalError(), Args... args) const {m_converter.OnFatalError(a,b,args...);}
    DgnDbR GetDgnDb() {return m_converter.GetDgnDb();}
    iModelBridge::Params const& GetParams() const {return m_converter._GetParams();}
    Converter::Config const& GetConfig() const {return m_converter.GetConfig();}

    DynamicSchemaGenerator(Converter& c) : m_converter(c) {}

    void SetEcConversionFailed() {m_ecConversionFailed=true;}
    bool GetEcConversionFailed() const {return m_ecConversionFailed;}
    bool GetAnyImported() const {return m_anyImported;}

    void GenerateSchemas(bset<DgnV8ModelP> const&);

    };

END_DGNDBSYNC_DGNV8_NAMESPACE
