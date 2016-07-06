/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMappingInfoHelper.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
void PropertyMapToString(Json::Value& propMapsJson, PropertyMap const& propMap)
    {
    Json::Value& json = propMapsJson.append(Json::Value(Json::objectValue));
    json["PropertyAccessString"] = propMap.GetPropertyAccessString();
    json["Type"] = PropertyMap::TypeToString(propMap.GetType());
    json["ECPropertyId"] = propMap.GetProperty().GetId().ToString().c_str();

    if (!propMap.GetChildren().IsEmpty())
        {
        Json::Value& childPropMaps = json["Children"];
        for (PropertyMapCP childPropertyMap : propMap.GetChildren())
            {
            PropertyMapToString(childPropMaps, *childPropertyMap);
            }

        return;
        }

    std::vector<DbColumn const*> columns;
    propMap.GetColumns(columns);
    if (columns.empty())
        return;

    Json::Value& columnsJson = json["Columns"];

    for (DbColumn const* column : columns)
        {
        Json::Value& columnJson = columnsJson.append(Json::Value(Json::objectValue));
        columnJson["Name"] = column->GetName().c_str();
        columnJson["Id"] = column->GetId().ToString().c_str();
        columnJson["Table"] = column->GetTable().GetName().c_str();
        columnJson["IsVirtual"] = column->GetPersistenceType() == PersistenceType::Virtual;
        columnJson["SqlType"] = DbColumn::TypeToSql(column->GetType());
        columnJson["Kind"] = DbColumn::KindToString(column->GetKind());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
void ClassMapToString(Json::Value& classMapJson, ClassMap const& classMap)
    {
    classMapJson["Name"] = classMap.GetClass().GetName().c_str();
    classMapJson["Id"] = classMap.GetId().ToString().c_str();
    classMapJson["Type"] = ClassMap::TypeToString(classMap.GetType());
    classMapJson["ECClassId"] = classMap.GetClass().GetId().ToString().c_str();
    classMapJson["MapStrategy"] = classMap.GetMapStrategy().ToString().c_str();

    Json::Value& tablesJson = classMapJson["Tables"];
    for (DbTable const* table : classMap.GetTables())
        {
        Json::Value tableJson = tablesJson.append(Json::objectValue);
        tableJson["Name"] = table->GetName().c_str();
        tableJson["Id"] = table->GetId().ToString().c_str();
        tableJson["IsVirtual"] = table->GetPersistenceType() == PersistenceType::Virtual;
        }

    Json::Value& propMapsJson = classMapJson["PropertyMaps"];
    for (PropertyMap const* propMap : classMap.GetPropertyMaps())
        {
        PropertyMapToString(propMapsJson, *propMap);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2016
//---------------------------------------------------------------------------------------
void SchemaToJson(Json::Value& schemaJson, ECDbMap const& ecdbMap, ECSchemaCR schema, bool skipUnmappedClasses)
    {
    schemaJson["FullName"] = schema.GetFullSchemaName().c_str();
    Json::Value& classMapsJson = schemaJson["ClassMaps"];

    for (ECClassCP ecclass : schema.GetClasses())
        {
        ClassMapCP classMap = ecdbMap.GetClassMap(*ecclass);
        if (classMap == nullptr)
            continue;

        if (classMap->GetMapStrategy().IsNotMapped() && skipUnmappedClasses)
            continue;

        Json::Value& classMapJson = classMapsJson.append(Json::Value(Json::objectValue));
        ClassMapToString(classMapJson, *classMap);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
//static
BentleyStatus ClassMappingInfoHelper::GetInfos(Json::Value& json, ECDbCR ecdb, bool skipUnmappedClasses)
    {
    bvector<ECSchemaCP> schemas = ecdb.Schemas().GetECSchemas();
    if (schemas.empty())
        return ERROR;

    ECDbMap const& ecdbMap = ecdb.GetECDbImplR().GetECDbMap();

    json = Json::Value(Json::objectValue);
    Json::Value& schemasJson = json["ECSchemas"];
    for (ECSchemaCP schema : schemas)
        {
        Json::Value& schemaJson = schemasJson.append(Json::Value(Json::objectValue));
        SchemaToJson(schemaJson, ecdbMap, *schema, skipUnmappedClasses);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
//static
BentleyStatus ClassMappingInfoHelper::GetInfos(Json::Value& json, ECDbCR ecdb, Utf8CP schemaName, bool skipUnmappedClasses)
    {
    ECSchemaCP schema = ecdb.Schemas().GetECSchema(schemaName);
    if (schema == nullptr)
        return ERROR;

    json = Json::Value(Json::objectValue);
    Json::Value& schemaJson = json["ECSchema"];
    SchemaToJson(schemaJson, ecdb.GetECDbImplR().GetECDbMap(), *schema, skipUnmappedClasses);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
//static
BentleyStatus ClassMappingInfoHelper::GetInfo(Json::Value& json, ECDbCR ecdb, Utf8CP schemaName, Utf8CP className)
    {
    ECClassCP ecClass = ecdb.Schemas().GetECClass(schemaName, className);
    if (ecClass == nullptr)
        return ERROR;

    ClassMap const* classMap = ecdb.GetECDbImplR().GetECDbMap().GetClassMap(ecClass->GetId());
    if (classMap == nullptr)
        return ERROR;

    json = Json::Value(Json::objectValue);
    Json::Value& classJson = json["ECClass"];
    ClassMapToString(classJson, *classMap);
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
