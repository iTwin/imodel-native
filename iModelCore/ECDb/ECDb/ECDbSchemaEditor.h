/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaEditor.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE


struct ECDbSchemaEditor
    {

    };
//struct ECDbSchemaChange
//    {
//    struct SchemaKey
//        {
//        public:
//            Utf8StringCR GetName();
//        };
//    struct ClassKey : SchemaKey
//        {
//        public:
//            Utf8StringCR GetName();
//        };
//    struct EnumerationKey : SchemaKey
//        {
//        public:
//            Utf8StringCR GetName();
//        };
//    struct PropertyKey : ClassKey
//        {
//        public:
//            Utf8StringCR GetName();
//        };
//    struct RelationshipConstraintKey : ClassKey
//        {
//        public:
//            Utf8StringCR GetName();
//        };
//    enum ConstraintType
//        {
//        Source,
//        Target
//        };
//
//    void CreateSchema(SchemaKey const& key, Utf8CP schemaPrefix);
//    void CreateEntityClass(Utf8CP name);
//    void CreateStructClass(Utf8CP name);
//    void CreateCustomAttributeClass(Utf8CP name);
//    void CreateRelationshipClass(Utf8CP name, Utf8CP displayLabel);
//    void CreateKindOfQuantity(Utf8CP name);
//
//    void CreateEnumeration(Utf8CP name, PrimitiveType type);
//
//    void SetDisplayLabel (SchemaKey const& schemakey, Utf8CP displayLabel);
//    void SetDisplayLabel (ClassKey const& classKey, Utf8CP displayLabel);
//    void SetDisplayLabel (PropertyKey const& propertyKey, Utf8CP displayLabel);
//
//    void Rename (SchemaKey const& schemakey, Utf8CP newSchemaName);
//    void Rename (ClassKey const& classKey, Utf8CP newClassName);
//    void Rename (PropertyKey const& propertyKey, Utf8CP newPropertyName);
//
//
//    void SetCustomAttribute (SchemaKey const& schemakey, IECInstancePtr ptr);
//    void SetCustomAttribute (ClassKey const& classKey, IECInstancePtr ptr);
//    void SetCustomAttribute (PropertyKey const& propertyKey, IECInstancePtr ptr);
//    void SetCustomAttribute (RelationshipConstraintKey const& relationshipConstraintKey, ECN::IECInstancePtr ptr);
//
//    };
END_BENTLEY_SQLITE_EC_NAMESPACE

