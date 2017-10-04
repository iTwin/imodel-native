/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/EntityConverter.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECConversion.h"

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

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
                Converter& m_converter;
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
                SchemaConversionContext(Converter&, ECN::ECSchemaReadContext& schemaReadContext, ECN::ECSchemaReadContext& syncReadContext, bool autoDetectMixinParams);

                BentleyStatus AddClassMapping(ECN::ECClassCR inputClass, ECN::ECClassR aspectClass, bool isAspectOnly, ECN::ECClassCR aspectBaseClass);
				BentleyStatus AddMixinAppliesToMapping(ECN::ECClassCP mixinClass, ECN::ECClassP appliesToClass);
				BentleyStatus AddMixinContextMapping(ECN::ECSchemaCP schema, MixinContext context);

                bool IsAlreadyConverted(ECN::ECClassCR inputClass) const;
                std::vector<std::pair<ECN::ECClassCP, ElementAspectDefinition>> const& GetAspectMappings() const { return m_aspectMappings; }
                ElementAspectDefinition const* TryGetAspectMapping(ECN::ECClassCR) const;

                Converter& GetConverter() const { return m_converter; }
                DgnDbR GetDgnDb() const { return m_converter.GetDgnDb(); }

                ECN::ECEntityClassP GetDefaultConstraintClass() const;
                ECN::ECClassCP GetBaseClass(ECClassName className) const;
                ECN::ECRelationshipClassCP GetDomainRelationshipBaseClass(ECN::ECRelationshipClassR inputClass) const;
                ECN::ECClassP GetInputClass(BentleyApi::Utf8CP schemaName, BentleyApi::Utf8CP className) const;
                bmap<Utf8String, ECN::ECSchemaP> const& GetSchemas() const { return m_inputSchemaMap; }
				bmap<ECN::ECClassCP, ECN::ECClassP> const& GetMixinAppliesToMapping() const { return m_mixinAppliesToMap; }
				MixinContext* GetMixinContext(ECN::ECSchemaCR schema);
                static bool ExcludeSchemaFromBisification(ECN::ECSchemaCR);

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

                bset<ECN::ECClassP> const& GetClasses() const { return m_classesToRemove; }
            };

    private:
        BisClassConverter();
        ~BisClassConverter();

        static BentleyStatus ConvertECClass(SchemaConversionContext&, ECClassName const& v8ClassName, BisConversionRule const* parentConversionRule);
        static BentleyStatus DoConvertECClass(SchemaConversionContext&, BisConversionRule, ECN::ECClassR inputClass, ECClassName const& v8ClassName, bool hasSecondary);

		static bool ShouldConvertECClassToMixin(ECN::ECSchemaR targetSchema, ECN::ECClassR inputClass, SchemaConversionContext& context);
		static BentleyStatus ConvertECClassToMixin(ECN::ECSchemaR targetSchema, ECN::ECClassR inputClass, ECN::ECClassCR appliesTo);
		static BentleyStatus CreateMixinContext(SchemaConversionContext::MixinContext& mixinContext, Converter& converter, ECN::ECSchemaReadContext& syncReadContext, ECN::ECSchemaP schema, bool autoDetect);
		static void FindCommonBaseClass(ECN::ECClassP& commonClass, ECN::ECClassP currentClass, ECN::ECBaseClassesList const& classes, const bvector<ECN::ECClassCP> propogationFilter);
        
		static void GetBisBaseClasses(ECN::ECClassCP& elementBaseClass, ECN::ECClassCP& elementAspectClass, SchemaConversionContext&, BisConversionRule);

        static ECClassName GetElementAspectBisBaseClassName(BisConversionRule);

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
        static BentleyStatus ConvertECRelationshipClass(SchemaConversionContext& context, ECN::ECRelationshipClassR inputClass, ECN::ECSchemaReadContextP syncContext);
        static void CheckForMixinConversion(SchemaConversionContext& context, ECN::ECClassR inputClass);
        static BentleyStatus FindAppliesToClass(ECN::ECClassP& appliesTo, SchemaConversionContext& context, ECN::ECSchemaR targetSchema, ECN::ECClassR mixinClass);
        static void ProcessConstraints(ECN::ECRelationshipClassR inputClass, ECN::ECEntityClassP defaultConstraintClass, SchemaConversionContext& context);
        static void ConvertECRelationshipConstraint(ECN::ECRelationshipConstraintR constraint, ECN::ECRelationshipClassR inputClass, ECN::ECEntityClassP defaultConstraintClass, SchemaConversionContext& context, bool isSource);

        static BentleyStatus PreprocessConversion(SchemaConversionContext&);

        //! Second step of the conversion process which is carried out once the first step is done for all ECClasses that 
        //! have ECInstances in the v8 file.
        static BentleyStatus FinalizeConversion(SchemaConversionContext&);

        static ECClassName GetElementBisBaseClassName(BisConversionRule);
        static Utf8CP GetAspectClassSuffix(BisConversionRule);
    };

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct ElementConverter
    {
    private:
        mutable bmap<Utf8CP, ECN::ECInstanceReadContextPtr> m_instanceReadContextCache;

        ECN::ECInstanceReadContextPtr LocateInstanceReadContext(ECN::ECSchemaCR schema) const;

    protected:
        ECN::IECInstancePtr Transform(ECObjectsV8::IECInstance const& v8Instance, ECN::ECClassCR dgnDbClass, bool transformAsAspect = false) const;
        //---------------------------------------------------------------------------------------
        // @bsiclass                                   Carole.MacDonald            01/2016
        //---------------+---------------+---------------+---------------+---------------+-------
        struct SchemaRemapper : ECN::IECSchemaRemapper
            {
            private:
                typedef bmap<Utf8String, Utf8String> T_propertyNameMappings;
                typedef bmap<Utf8String, T_propertyNameMappings> T_ClassPropertiesMap;
                Converter& m_converter;
                mutable ECN::ECSchemaPtr m_convSchema;
                bool m_remapAsAspect;
                mutable T_ClassPropertiesMap m_renamedClassProperties;

                virtual bool _ResolvePropertyName(Utf8StringR serializedPropertyName, ECN::ECClassCR ecClass) const override;
                virtual bool _ResolveClassName(Utf8StringR serializedClassName, ECN::ECSchemaCR ecSchema) const override;

            public:
                explicit SchemaRemapper(Converter& converter) : m_converter(converter), m_remapAsAspect(false) {}
                ~SchemaRemapper() {}
                void SetRemapAsAspect(bool remapAsAspect) { m_remapAsAspect = remapAsAspect; }
            };

        struct UnitResolver : ECN::ECInstanceReadContext::IUnitResolver
            {
            private:
                mutable ECN::ECSchemaPtr m_convSchema;

                virtual Utf8String _ResolveUnitName(ECN::ECPropertyCR ecProperty) const override;
            };

        Converter& m_converter;
        mutable SchemaRemapper m_schemaRemapper;
        mutable UnitResolver m_unitResolver;

        ECN::ECClassCP GetDgnDbClass(ECObjectsV8::IECInstance const& v8Instance, BentleyApi::Utf8CP aspectClassSuffix) const;
        static Utf8String ToInstanceLabel(ECObjectsV8::IECInstance const& v8Instance);

    public:
        explicit ElementConverter(Converter& converter) : m_converter(converter), m_schemaRemapper(converter) {}
        BentleyStatus ConvertToElementItem(ElementConversionResults&, ECObjectsV8::IECInstance const* v8PrimaryInstance, BisConversionRule const* primaryInstanceConversionRule) const;

    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      03/2015
//+===============+===============+===============+===============+===============+======
struct ElementAspectConverter : ElementConverter
    {
    private:
        BentleyStatus ConvertToAspect(ElementConversionResults&, ECObjectsV8::IECInstance const& v8Instance, BentleyApi::Utf8CP aspectClassSuffix) const;

    public:
        explicit ElementAspectConverter(Converter& converter) : ElementConverter(converter) {}
        BentleyStatus ConvertToAspects(ElementConversionResults&, std::vector<std::pair<ECObjectsV8::IECInstancePtr, BisConversionRule>> const& secondaryInstances) const;
    };

END_DGNDBSYNC_DGNV8_NAMESPACE
