/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/BimImporter/lib/SchemaFlattener.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BIM_TELEPORTER_NAMESPACE
//=======================================================================================
// @bsiclass                                                Carole.MacDonald      12/2017
//+===============+===============+===============+===============+===============+======
struct SchemaFlattener
    {

    private:
    bmap<Utf8String, ECN::ECSchemaPtr> m_flattenedRefs;
    ECN::ECSchemaReadContextPtr m_schemaReadContext;

    BentleyStatus CopyFlatConstraint(ECN::ECRelationshipConstraintR toRelationshipConstraint, ECN::ECRelationshipConstraintCR fromRelationshipConstraint);
    BentleyStatus CopyFlatCustomAttributes(ECN::IECCustomAttributeContainerR targetContainer, ECN::IECCustomAttributeContainerCR sourceContainer);
    BentleyStatus CreateFlatClass(ECN::ECClassP& targetClass, ECN::ECSchemaP flatSchema, ECN::ECClassCP sourceClass);
    BentleyStatus CopyFlatClass(ECN::ECClassP& targetClass, ECN::ECSchemaP flatSchema, ECN::ECClassCP sourceClass);
    BentleyStatus CopyFlattenedProperty(ECN::ECClassP targetClass, ECN::ECPropertyCP sourceProperty);

    public:
        SchemaFlattener(ECN::ECSchemaReadContextPtr context) : m_schemaReadContext(context) {}

        void ProcessSP3DSchema(ECN::ECSchemaP schema, ECN::ECClassCP baseInterface, ECN::ECClassCP baseObject);
        BentleyStatus FlattenSchemas(ECN::ECSchemaP ecSchema);
    };

END_BIM_TELEPORTER_NAMESPACE
