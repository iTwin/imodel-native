/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "../ECSchemaMappings.h"

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

struct DynamicSchemaGenerator;

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
            ECN::SchemaKey GetSchemaKey() const;
            Utf8CP GetSchemaXml () const;
            SyncInfo::ECSchemaMappingType GetMappingType() const;
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
    bool m_skipECContent = true;
    bool m_needReimportSchemas = false;
    bool m_ecConversionFailed = false;
    bool m_ecConversionFailedDueToLockingError = false;
    bool m_anyImported = false;
    bool m_hasECContent = false;
    Converter& m_converter;
    ECN::ECSchemaReadContextPtr m_schemaReadContext;
    ECN::ECSchemaReadContextPtr m_syncReadContext;
    bmap<Utf8String, ECN::ECSchemaPtr> m_flattenedRefs;
    bmap<Utf8String, ECObjectsV8::ECSchemaPtr> m_v8Schemas;
    bvector<Utf8String> m_skippedSchemas;

    void CheckECSchemasForModel(DgnV8ModelR, bmap<Utf8String, uint32_t>& syncInfoChecksums);
    BentleyStatus RetrieveV8ECSchemas(DgnV8ModelR v8rootModel);
    BentleyStatus RetrieveV8ECSchemas(DgnV8ModelR v8rootModel, DgnV8Api::ECSchemaPersistence persistence);
    BentleyStatus ProcessSchemaXml(const ECObjectsV8::SchemaKey& schemaKey, Utf8CP schemaXml, bool isDynamicSchema, DgnV8ModelR v8Model);
    BentleyStatus ProcessReferenceSchemasFromExternal(ECObjectsV8::ECSchemaCR schema, DgnV8ModelR v8Model);
    static bool IsDynamicSchema(ECObjectsV8::ECSchemaCR schema);
    static bool IsWellKnownDynamicSchema(Bentley::Utf8StringCR schemaName);
    static bool IsDynamicSchema(Bentley::Utf8StringCR schemaName, Bentley::Utf8StringCR schemaXml);
    void ProcessSP3DSchema(ECN::ECSchemaP schema, ECN::ECClassCP baseInterface, ECN::ECClassCP baseObject);
    BentleyStatus CopyFlatConstraint(ECN::ECRelationshipConstraintR toRelationshipConstraint, ECN::ECRelationshipConstraintCR fromRelationshipConstraint);
    BentleyStatus CopyFlatCustomAttributes(ECN::IECCustomAttributeContainerR targetContainer, ECN::IECCustomAttributeContainerCR sourceContainer);
    BentleyStatus CreateFlatClass(ECN::ECClassP& targetClass, ECN::ECSchemaP flatSchema, ECN::ECClassCP sourceClass);
    BentleyStatus CopyFlatClass(ECN::ECClassP& targetClass, ECN::ECSchemaP flatSchema, ECN::ECClassCP sourceClass);
    BentleyStatus CopyFlattenedProperty(ECN::ECClassP targetClass, ECN::ECPropertyCP sourceProperty);
    BentleyStatus FlattenSchemas(ECN::ECSchemaP ecSchema);
    //BentleyStatus LastResortSchemaImport();

    BentleyStatus ConsolidateV8ECSchemas();
    BentleyStatus MergeV8ECSchemas(struct ECSchemaXmlDeserializer& deserializer, bmap<Utf8String, bvector<bpair<ECN::SchemaKey, Utf8String>>> const& schemaXmlMap) const;
    void SwizzleOpenPlantSupplementals(bvector<BECN::ECSchemaPtr>& tmpSupplementals, BECN::ECSchemaP primarySchema, bvector<BECN::ECSchemaP> supplementalSchemas);
    BentleyStatus SupplementV8ECSchemas();
    BentleyStatus ConvertToBisBasedECSchemas();
    BentleyStatus ImportTargetECSchemas();
    void ValidateSchemas(bvector<BECN::ECSchemaCP>& importedSchemas);
    void AnalyzeECContent(DgnV8ModelR, BisConversionTargetModelInfoCR);
    BentleyStatus Analyze(DgnV8Api::ElementHandle const&, BisConversionTargetModelInfoCR);
    BentleyStatus DoAnalyze(DgnV8Api::ElementHandle const&, BisConversionTargetModelInfoCR);
    void InitializeECSchemaConversion();
    void FinalizeECSchemaConversion();

    SyncInfo& GetSyncInfo() {return m_converter.GetSyncInfo();}
    bool IsUpdating() const {return m_converter.IsUpdating();}

    void CheckNoECSchemaChanges(bvector<DgnV8ModelP> const&);
    void BisifyV8Schemas(bvector<DgnV8FileP> const&, bvector<DgnV8ModelP> const&);
    void RemoveDgnV8CustomAttributes(ECN::IECCustomAttributeContainerR container);

    ECN::ECSchemaPtr CreateDgnV8TagSetDefinitionSchema(T_TagSetDefToClassNameMap& tagSetDefToClassMap, bvector<DgnV8FileP> const& uniqueFiles, bvector<DgnV8ModelP> const& uniqueModels);

    public:

    bool DidEcConversionFailDueToLockingError() const {return m_ecConversionFailedDueToLockingError;}
    bool WasAborted() const {return m_converter.WasAborted();}
    void ReportProgress() const {m_converter.ReportProgress();}
    template<typename ...Args> void SetStepName(Converter::ProgressMessage::StringId a, Args... args) const {m_converter.SetStepName(a, args...);}
    void AddTasks(int32_t n) const {m_converter.AddTasks(n);}
    template<typename ...Args> void SetTaskName(Converter::ProgressMessage::StringId a, Args... args) const {m_converter.SetTaskName(a, args...);}
    void ReportError(Converter::IssueCategory::StringId a, Converter::Issue::StringId b, Utf8CP c) {m_converter.ReportError(a,b,c);}
    void ReportIssue(Converter::IssueSeverity a, Converter::IssueCategory::StringId b, Converter::Issue::StringId c, Utf8CP d, Utf8CP e = nullptr) {m_converter.ReportIssue(a,b,c,d,e);}
    template<typename ...Args> void ReportIssueV(Converter::IssueSeverity a, Converter::IssueCategory::StringId b, Converter::Issue::StringId c, Utf8CP d, Args... args) {m_converter.ReportIssueV(a,b,c,d,args...);}
    void ReportSyncInfoIssue(Converter::IssueSeverity a, Converter::IssueCategory::StringId b, Converter::Issue::StringId c, Utf8CP d) {m_converter.ReportSyncInfoIssue(a,b,c,d);}
    template<typename ...Args> BentleyStatus OnFatalError(Converter::IssueCategory::StringId a=Converter::IssueCategory::Unknown(), Converter::Issue::StringId b=Converter::Issue::FatalError(), Args... args) const {return m_converter.OnFatalError(a,b,args...);}
    DgnDbR GetDgnDb() {return m_converter.GetDgnDb();}
    iModelBridge::Params const& GetParams() const {return m_converter._GetParams();}
    Converter::Config const& GetConfig() const {return m_converter.GetConfig();}

    DynamicSchemaGenerator(Converter& c) : m_converter(c), m_skipECContent(c.SkipECContent()) {}

    void SetEcConversionFailed() {m_ecConversionFailed=true;}
    bool GetEcConversionFailed() const {return m_ecConversionFailed;}
    bool GetAnyImported() const {return m_anyImported;}

    void GenerateSchemas(bvector<DgnFileP> const&, bvector<DgnV8ModelP> const&);

    static bool ExcludeSchemaFromBisification(ECN::ECSchemaCR);
    static bool ExcludeSchemaFromBisification(BentleyApi::Utf8StringCR schemaName);
    static bool IsDgnV8DeliveredSchema(BentleyApi::Utf8StringCR schemaName);


    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      02/2015
//+===============+===============+===============+===============+===============+======
struct BisClassConverter
    {
    public:
        struct SchemaConversionContext : NonCopyableClass
            {
            public:
                struct ElementAspectDefinition
                    {
                    ECN::ECClassP m_aspectClass;
                    bool m_isAspectOnly;
                    ECN::ECClassCP m_aspectBisBaseClass;
                    ElementAspectDefinition() : m_aspectClass(nullptr), m_isAspectOnly(false), m_aspectBisBaseClass(nullptr) {}
                    ElementAspectDefinition(ECN::ECClassR aspectClass, bool isAspectOnly, ECN::ECClassCR aspectBisBaseClass)
                        : m_aspectClass(&aspectClass), m_isAspectOnly(isAspectOnly), m_aspectBisBaseClass(&aspectBisBaseClass)
                        {
                        }
                    };

                typedef Bentley::bpair<ECN::ECClassCP, ECN::ECClassCP> MixinContext;

            private:
                DynamicSchemaGenerator& m_converter;
                mutable ECN::ECEntityClassP m_defaultConstraintClass = nullptr;
                bmap<Utf8String, ECN::ECSchemaP> m_inputSchemaMap;

                struct Utf8CPKeyComparer
                    {
                    bool operator() (Utf8CP lhs, Utf8CP rhs) const { return strcmp(lhs, rhs) < 0; }
                    };

                bmap<Utf8CP, ECN::ECSchemaCP, Utf8CPKeyComparer> m_baseSchemaCache;
                mutable ECN::ECRelationshipClassCP m_domainRelationshipBaseClass = nullptr;
                mutable ECN::ECRelationshipClassCP m_aspectRelationshipBaseClass = nullptr;

                std::vector<std::pair<ECN::ECClassCP, ElementAspectDefinition>> m_aspectMappings;
                bmap<ECN::ECClassCP, size_t> m_aspectMappingIndexMap;

                bmap<ECN::ECClassCP, ECN::ECClassP> m_mixinAppliesToMap;
                bmap<ECN::ECSchemaCP, MixinContext>  m_mixinContextCache;

                void ReportIssueV(Converter::IssueSeverity severity, BentleyApi::Utf8CP message, va_list args) const;

            public:
                SchemaConversionContext(DynamicSchemaGenerator&, ECN::ECSchemaReadContext& schemaReadContext, ECN::ECSchemaReadContext& syncReadContext, bool autoDetectMixinParams);

                BentleyStatus AddClassMapping(ECN::ECClassCR inputClass, ECN::ECClassR aspectClass, bool isAspectOnly, ECN::ECClassCR aspectBaseClass);
                BentleyStatus AddMixinAppliesToMapping(ECN::ECClassCP mixinClass, ECN::ECClassP appliesToClass);
                BentleyStatus AddMixinContextMapping(ECN::ECSchemaCP schema, MixinContext context);

                bool IsAlreadyConverted(ECN::ECClassCR inputClass) const;
                std::vector<std::pair<ECN::ECClassCP, ElementAspectDefinition>> const& GetAspectMappings() const { return m_aspectMappings; }
                ElementAspectDefinition const* TryGetAspectMapping(ECN::ECClassCR) const;

                DynamicSchemaGenerator& GetConverter() const { return m_converter; }
                DgnDbR GetDgnDb() const { return m_converter.GetDgnDb(); }

                ECN::ECEntityClassP GetDefaultConstraintClass() const;
                ECN::ECClassCP GetBaseClass(ECClassName className) const;
                ECN::ECRelationshipClassCP GetDomainRelationshipBaseClass(ECN::ECRelationshipClassR inputClass) const;
                ECN::ECClassP GetInputClass(BentleyApi::Utf8CP schemaName, BentleyApi::Utf8CP className) const;
                bmap<Utf8String, ECN::ECSchemaP> const& GetSchemas() const { return m_inputSchemaMap; }
                bmap<ECN::ECClassCP, ECN::ECClassP> const& GetMixinAppliesToMapping() const { return m_mixinAppliesToMap; }
                MixinContext* GetMixinContext(ECN::ECSchemaCR schema);
                void ReportIssue(Converter::IssueSeverity severity, BentleyApi::Utf8CP message, ...) const;
            };

        struct ECClassRemovalContext
            {
            private:
                SchemaConversionContext& m_schemaConversionContext;
                bset<ECN::ECSchemaP> m_schemas;
                bset<ECN::ECClassP> m_candidateClasses;
                bset<ECN::ECClassP> m_classesToRemove;

                BentleyStatus DoAddClassToRemove(ECN::ECClassR);
                BentleyStatus FindClassReferences(ECN::ECClassCR);

                static bool RemoveCustomAttribute(ECN::IECCustomAttributeContainerR, ECN::ECClassCR caClass);

            public:
                explicit ECClassRemovalContext(SchemaConversionContext&);
                BentleyStatus AddClassToRemove(ECN::ECClassR, bool includeSubclasses = false);
                BentleyStatus FixClassHierarchies();
                SchemaConversionContext& SchemaContext() { return m_schemaConversionContext; }

                bset<ECN::ECClassP> const& GetClasses() const { return m_classesToRemove; }
            };

    private:
        BisClassConverter();
        ~BisClassConverter();

        static BentleyStatus ConvertECClass(SchemaConversionContext&, ECClassName const& v8ClassName, BisConversionRule const* parentConversionRule);
        static BentleyStatus DoConvertECClass(SchemaConversionContext&, BisConversionRule, ECN::ECClassR inputClass, ECClassName const& v8ClassName, bool hasSecondary);

        static bool ShouldConvertECClassToMixin(ECN::ECSchemaR targetSchema, ECN::ECClassR inputClass, SchemaConversionContext& context);
        static BentleyStatus ConvertECClassToMixin(ECN::ECSchemaR targetSchema, ECN::ECClassR inputClass, ECN::ECClassCR appliesTo);
        static BentleyStatus CreateMixinContext(SchemaConversionContext::MixinContext& mixinContext, DynamicSchemaGenerator& converter, ECN::ECSchemaReadContext& syncReadContext, ECN::ECSchemaP schema, bool autoDetect);
        static void FindCommonBaseClass(ECN::ECClassP& commonClass, ECN::ECClassP currentClass, ECN::ECBaseClassesList const& classes, const bvector<ECN::ECClassCP> propogationFilter);
        
        static void GetBisBaseClasses(ECN::ECClassCP& elementBaseClass, ECN::ECClassCP& elementAspectClass, SchemaConversionContext&, BisConversionRule);

        //! Injecting a BIS base class might lead to collisions between ECDbMap custom attributes used on the BIS base class and the domain class.
        //! The BIS base class custom attributes should be preferred, therefore the hint on the domain class is deleted.
        static BentleyStatus RemoveDuplicateClassMapCustomAttributes(ECN::ECClassR);

        static BentleyStatus ValidateClassProperties(SchemaConversionContext&, ECN::ECClassR);

        static BentleyStatus AddBaseClass(ECN::ECClassR targetClass, ECN::ECClassCR baseClass);
        static BentleyStatus MoveProperties(ECN::ECClassR toClass, ECN::ECClassR fromClass);

        static bool IsAbstractClass(ECN::ECClassCR ecClass) { return ECN::ECClassModifier::Abstract == ecClass.GetClassModifier(); }
        static BentleyStatus CheckBaseAndDerivedClassesForBisification(SchemaConversionContext& context, ECN::ECClassCP childClass, BisConversionRule childRule, bvector<ECN::ECClassP> classes, bool isBaseClassCheck);
        static BentleyStatus EnsureBaseClassesAndDerivedClassesAreSet(SchemaConversionContext& context, bool isBaseClassCheck);


    public:
        //! Converts the given v8 ECClass and its direct derived ECClasses
        static BentleyStatus ConvertECClass(SchemaConversionContext&, ECClassName const& v8ClassName);
        static BentleyStatus ConvertECRelationshipClass(ECClassRemovalContext& context, ECN::ECRelationshipClassR inputClass, ECN::ECSchemaReadContextP syncContext);
        static void CheckForMixinConversion(SchemaConversionContext& context, ECN::ECClassR inputClass);
        static BentleyStatus FindAppliesToClass(ECN::ECClassP& appliesTo, SchemaConversionContext& context, ECN::ECSchemaR targetSchema, ECN::ECClassR mixinClass);
        static void ProcessConstraints(ECN::ECRelationshipClassR inputClass, ECN::ECEntityClassP defaultConstraintClass, SchemaConversionContext& context);
        static void ConvertECRelationshipConstraint(ECN::ECRelationshipConstraintR constraint, ECN::ECRelationshipClassR inputClass, ECN::ECEntityClassP defaultConstraintClass, SchemaConversionContext& context, bool isSource);
        static void AddDroppedDerivedClass(ECN::ECClassP baseClass, ECN::ECClassP derivedClass);

        static BentleyStatus PreprocessConversion(SchemaConversionContext&);

        //! Second step of the conversion process which is carried out once the first step is done for all ECClasses that 
        //! have ECInstances in the v8 file.
        static BentleyStatus FinalizeConversion(SchemaConversionContext&);
    };
END_DGNDBSYNC_DGNV8_NAMESPACE
