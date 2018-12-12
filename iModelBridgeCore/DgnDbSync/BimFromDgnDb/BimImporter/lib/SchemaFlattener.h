/*--------------------------------------------------------------------------------------+
|
|     $Source: BimFromDgnDb/BimImporter/lib/SchemaFlattener.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BIM_FROM_DGNDB_NAMESPACE
//=======================================================================================
// @bsiclass                                                Carole.MacDonald      12/2017
//+===============+===============+===============+===============+===============+======
struct SchemaFlattener
    {

    private:
    bmap<Utf8String, ECN::ECSchemaPtr> m_flattenedRefs;
    ECN::ECSchemaReadContextPtr m_schemaReadContext;
    ECN::ECClassCP m_baseObject = nullptr;
    ECN::ECClassCP m_baseInterface = nullptr;
    bmap<ECN::ECClassCP, ECN::ECClassP> m_mixinAppliesToMap;

    BentleyStatus CopyFlatConstraint(ECN::ECRelationshipConstraintR toRelationshipConstraint, ECN::ECRelationshipConstraintCR fromRelationshipConstraint);
    BentleyStatus CopyFlatCustomAttributes(ECN::IECCustomAttributeContainerR targetContainer, ECN::IECCustomAttributeContainerCR sourceContainer);
    BentleyStatus CreateFlatClass(ECN::ECClassP& targetClass, ECN::ECSchemaP flatSchema, ECN::ECClassCP sourceClass);
    BentleyStatus CopyFlatClass(ECN::ECClassP& targetClass, ECN::ECSchemaP flatSchema, ECN::ECClassCP sourceClass);
    BentleyStatus CopyFlattenedProperty(ECN::ECClassP targetClass, ECN::ECPropertyCP sourceProperty);
    BentleyStatus FindBisBaseClass(ECN::ECClassP targetClass, ECN::ECClassCP sourceClass);
    BentleyStatus FindAppliesToClass(ECN::ECClassP& appliesTo, ECN::ECSchemaR targetSchema, ECN::ECClassR mixinClass);
    BentleyStatus ConvertECClassToMixin(ECN::ECSchemaR targetSchema, ECN::ECClassR inputClass, ECN::ECClassCR appliesTo);
    BentleyStatus AddMixinAppliesToMapping(ECN::ECClassCP mixinClass, ECN::ECClassP appliesToClass);
    bool ShouldConvertECClassToMixin(ECN::ECSchemaR targetSchema, ECN::ECClassR inputClass);
    static void FindCommonBaseClass(ECN::ECClassP& commonClass, ECN::ECClassP currentClass, ECN::ECBaseClassesList const& classes, const bvector<ECN::ECClassCP> propogationFilter);
    void ProcessConstraints(ECN::ECRelationshipConstraintR constraint, bool isSource, ECN::ECRelationshipClassR relClass, ECN::ECEntityClassP defaultConstraintClass);
    void CheckConstraintForDerivedClasses(ECN::ECRelationshipConstraintR constraint);

    public:
        SchemaFlattener(ECN::ECSchemaReadContextPtr context) : m_schemaReadContext(context) {}

        void ProcessSP3DSchema(ECN::ECSchemaP schema, ECN::ECClassCP baseInterface, ECN::ECClassCP baseObject, bset<ECN::ECClassP>& rootClasses, ECN::ECEntityClassP defaultConstraintClass);
        BentleyStatus FlattenSchemas(ECN::ECSchemaP ecSchema);
        void CheckForMixinConversion(ECN::ECClassR inputClass);
    };

END_BIM_FROM_DGNDB_NAMESPACE
