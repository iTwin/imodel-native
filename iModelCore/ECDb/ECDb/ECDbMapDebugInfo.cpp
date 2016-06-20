/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbMapDebugInfo.cpp $
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
ECDbMapDebugInfo::ECDbMapDebugInfo()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
ECDbMapDebugInfo::~ECDbMapDebugInfo()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMapDebugInfo::GetMapInfoForSchema(Utf8StringR info, ECDbCR ecdb, Utf8CP ecSchemaName, bool skipUnmappedClasses)
    {
    ECDbMap const& map = ecdb.GetECDbImplR().GetECDbMap();
    ECDbSchemaManagerCR sm = ecdb.Schemas();
    ECSchemaCP schema = sm.GetECSchema(ecSchemaName);
    if (schema == nullptr)
        return ERROR;

    DebugWriter writer;
    writer.AppendLine("ECSchema : %s", schema->GetFullSchemaName().c_str());
    if (auto s0 = writer.CreateIndentBlock())
        {
        for (auto ecClass : schema->GetClasses())
            {
            auto classMap = map.GetClassMap(*ecClass);
            if (classMap == nullptr)
                continue;

            if (classMap->GetMapStrategy().IsNotMapped() && skipUnmappedClasses)
                continue;

            classMap->WriteDebugInfo(writer);
            }
        }

    info = writer.ToString();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMapDebugInfo::GetMapInfoForClass(Utf8StringR info, ECDbCR ecdb, Utf8CP qualifiedClassName)
    {
    ECDbMap const& map = ecdb.GetECDbImplR().GetECDbMap();
    //ECDbSchemaManagerCR sm = ecdb.Schemas();
    Utf8String qClassName = qualifiedClassName;
    auto n = qClassName.find('.');
    BeAssert(n != Utf8String::npos);
    if (n == Utf8String::npos)
        {
        return ERROR;
        }

    Utf8String schemaName = qClassName.substr(0, n);
    Utf8String className = qClassName.substr(n + 1);
    ECClassId classId = ECDbSchemaPersistenceHelper::GetECClassId(ecdb, schemaName.c_str(), className.c_str(), ResolveSchema::AutoDetect);
    if (!classId.IsValid())
        return ERROR;

    DebugWriter writer;
    writer.AppendLine("ECDb : [BriefcaseId=%s], [File=%s]",
                      ecdb.GetBriefcaseId().GetValue(),
                      ecdb.GetDbFileName());

    auto classMap = map.GetClassMap(classId);
    if (classMap == nullptr)
        return ERROR;

    classMap->WriteDebugInfo(writer);
    info = writer.ToString();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      03/2013
//---------------------------------------------------------------------------------------
BentleyStatus ECDbMapDebugInfo::GetMapInfoForAllClasses(Utf8StringR info, ECDbCR ecdb, bool skipUnmappedClasses)
    {
    ECDbMap const& map = ecdb.GetECDbImplR().GetECDbMap();
    ECDbSchemaManagerCR sm = ecdb.Schemas();

    DebugWriter writer;
    bvector<ECN::ECSchemaCP> schemas;
    sm.GetECSchemas(schemas);
    writer.AppendLine("ECDb : [BriefcaseId=%s], [File=%s]",
                      ecdb.GetBriefcaseId().GetValue(),
                      ecdb.GetDbFileName());


    writer.AppendLine("ECSchemas [Count=%d]", schemas.size());
    for (auto schema : schemas)
        {
        writer.AppendLine("ECSchema : %s", schema->GetFullSchemaName().c_str());
        writer.Indent();
        for (auto ecclass : schema->GetClasses())
            {
            ClassMapCP classMap = map.GetClassMap(*ecclass);
            if (classMap == nullptr)
                continue;

            if (classMap->GetMapStrategy().IsNotMapped() && skipUnmappedClasses)
                continue;

            classMap->WriteDebugInfo(writer);
            }

        writer.UnIndent();
        }

    info = writer.ToString();
    return SUCCESS;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
