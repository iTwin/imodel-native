/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "SchemaUpdateTestFixture.h"

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

namespace
{
struct BaselineSeed final
    {
    BeFileName m_filePath;
    Utf8String m_schemaName;
    };

struct InsertedInstance final
    {
    ECInstanceId m_id;
    ECClassCP m_class;
    };

//---------------------------------------------------------------------------------------
// Rejections are often expected, so their reasons are printed rather than reported as test failures.
//---------------------------------------------------------------------------------------
void PrintRejectionReasons(Utf8CP what, TestIssueListener const& issueListener)
    {
    printf("%s was rejected.\n", what);
    for (ReportedIssue const& issue : issueListener.m_issues)
        printf("    %s: %s\n", issue.id.m_issueId, issue.message.c_str());
    }

//---------------------------------------------------------------------------------------
// Baselines are created once per test run and shared by all tests using them.
//---------------------------------------------------------------------------------------
std::map<Utf8String, BaselineSeed>& GetBaselineSeeds()
    {
    static std::map<Utf8String, BaselineSeed> s_seeds;
    return s_seeds;
    }

// Selected right after ECInstanceId and ECClassId for relationship classes.
constexpr Utf8CP s_relationshipEndNames[] = {"SourceECInstanceId", "SourceECClassId", "TargetECInstanceId", "TargetECClassId"};

//---------------------------------------------------------------------------------------
// Relationship classes no navigation property points to. Their instances live in a link table
// and have their own ECInstanceIds. Only the schema itself is searched for navigation properties.
//---------------------------------------------------------------------------------------
bool IsLinkTableRelationshipClass(ECClassCR ecClass)
    {
    ECRelationshipClassCP relClass = ecClass.GetRelationshipClassCP();
    if (relClass == nullptr)
        return false;

    for (ECClassCP schemaClass : ecClass.GetSchema().GetClasses())
        {
        for (ECPropertyCP prop : schemaClass->GetProperties(false))
            {
            if (prop->GetIsNavigation() && relClass->Is(prop->GetAsNavigationProperty()->GetRelationshipClass()))
                return false;
            }
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool IsConcreteClass(ECClassCR ecClass)
    {
    return !ecClass.IsMixin() && ecClass.GetClassModifier() != ECClassModifier::Abstract;
    }

//---------------------------------------------------------------------------------------
// Entity classes and link table relationship classes, the classes the harness inserts and reads.
//---------------------------------------------------------------------------------------
bvector<ECClassCP> GetDataClasses(ECSchemaCR schema)
    {
    bvector<ECClassCP> classes;
    for (ECClassCP ecClass : schema.GetClasses())
        {
        if (ecClass->IsEntityClass() || IsLinkTableRelationshipClass(*ecClass))
            classes.push_back(ecClass);
        }

    return classes;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String BuildSelectECSql(ECClassCR ecClass, bool onlyThisClass)
    {
    Utf8String ecsql("SELECT ECInstanceId, ECClassId");
    if (ecClass.IsRelationshipClass())
        {
        for (Utf8CP endName : s_relationshipEndNames)
            ecsql.append(",").append(endName);
        }

    for (ECPropertyCP prop : ecClass.GetProperties(true))
        ecsql.append(",[").append(prop->GetName()).append("]");

    ecsql.append(onlyThisClass ? " FROM ONLY " : " FROM ").append(ecClass.GetECSqlName()).append(" ORDER BY ECInstanceId");
    return ecsql;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::map<Utf8String, Utf8String> ReadValues(ECSqlStatement const& stmt, JsonECSqlSelectAdapter const& adapter, ECClassCR ecClass)
    {
    std::map<Utf8String, Utf8String> values;
    BeJsDocument row;
    if (SUCCESS != adapter.GetRow(row))
        {
        ADD_FAILURE() << "Failed to read row of " << ecClass.GetFullName();
        return values;
        }

    if (ecClass.IsRelationshipClass())
        {
        int columnIndex = 2;
        for (Utf8CP endName : s_relationshipEndNames)
            values[endName] = Utf8PrintfString("%" PRId64, stmt.GetValueInt64(columnIndex++));
        }

    for (ECPropertyCP prop : ecClass.GetProperties(true))
        {
        BeJsConst value = row[prop->GetName().c_str()];
        if (!value.isNull())
            values[prop->GetName()] = value.Stringify();
        }

    return values;
    }

//---------------------------------------------------------------------------------------
// Keeps only the values of properties and relationship ends the class has.
//---------------------------------------------------------------------------------------
std::map<Utf8String, Utf8String> ProjectValues(std::map<Utf8String, Utf8String> const& values, ECClassCR ecClass)
    {
    std::map<Utf8String, Utf8String> projected;
    auto keepValue = [&](Utf8StringCR name)
        {
        auto it = values.find(name);
        if (it != values.end())
            projected.insert(*it);
        };

    if (ecClass.IsRelationshipClass())
        {
        for (Utf8CP endName : s_relationshipEndNames)
            keepValue(endName);
        }

    for (ECPropertyCP prop : ecClass.GetProperties(true))
        keepValue(prop->GetName());

    return projected;
    }

//---------------------------------------------------------------------------------------
// Reports one failure per property that differs.
//---------------------------------------------------------------------------------------
void CompareValues(Utf8CP context, std::map<Utf8String, Utf8String> const& expected, std::map<Utf8String, Utf8String> const& actual)
    {
    std::set<Utf8String> propertyNames;
    for (auto const& entry : expected)
        propertyNames.insert(entry.first);
    for (auto const& entry : actual)
        propertyNames.insert(entry.first);

    for (Utf8StringCR propertyName : propertyNames)
        {
        auto expectedIt = expected.find(propertyName);
        auto actualIt = actual.find(propertyName);
        Utf8String expectedValue = expectedIt != expected.end() ? expectedIt->second : Utf8String("NULL");
        Utf8String actualValue = actualIt != actual.end() ? actualIt->second : Utf8String("NULL");
        EXPECT_STREQ(expectedValue.c_str(), actualValue.c_str()) << context << ", property " << propertyName.c_str();
        }
    }

//=======================================================================================
// Generates a different value for every primitive it binds. Enumeration properties cycle
// through the enumerators instead.
//=======================================================================================
struct ValueGenerator final
    {
    private:
        uint64_t m_next = 1;
        std::map<ECEnumerationCP, size_t> m_nextEnumeratorIndexes;

        ECSqlStatus BindEnumerator(IECSqlBinder& binder, ECEnumerationCR enumeration)
            {
            if (enumeration.GetEnumeratorCount() == 0)
                {
                ADD_FAILURE() << "Enumeration " << enumeration.GetFullName().c_str() << " has no enumerators.";
                return ECSqlStatus::Error;
                }

            size_t& nextIndex = m_nextEnumeratorIndexes[&enumeration];
            auto it = enumeration.GetEnumerators().begin();
            std::advance(it, nextIndex++ % enumeration.GetEnumeratorCount());
            ECEnumeratorCP enumerator = *it;
            return enumerator->IsInteger() ? binder.BindInt(enumerator->GetInteger()) : binder.BindText(enumerator->GetString().c_str(), IECSqlBinder::MakeCopy::Yes);
            }

        ECSqlStatus BindPrimitive(IECSqlBinder& binder, PrimitiveType type)
            {
            const uint64_t value = m_next++;
            switch (type)
                {
                case PRIMITIVETYPE_Binary:
                    return binder.BindBlob(&value, (int) sizeof(value), IECSqlBinder::MakeCopy::Yes);
                case PRIMITIVETYPE_Boolean:
                    return binder.BindBoolean(value % 2 == 0);
                case PRIMITIVETYPE_DateTime:
                    return binder.BindDateTime(2451545.0 + (double) value, DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified));
                case PRIMITIVETYPE_Double:
                    return binder.BindDouble((double) value + 0.5);
                case PRIMITIVETYPE_IGeometry:
                    {
                    IGeometryPtr geometry = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.0, 0.0, 0.0, (double) value, (double) value + 0.25, (double) value + 0.5)));
                    return binder.BindGeometry(*geometry);
                    }
                case PRIMITIVETYPE_Integer:
                    return binder.BindInt((int) value);
                case PRIMITIVETYPE_Long:
                    return binder.BindInt64((int64_t) value);
                case PRIMITIVETYPE_Point2d:
                    return binder.BindPoint2d(DPoint2d::From((double) value, (double) value + 0.25));
                case PRIMITIVETYPE_Point3d:
                    return binder.BindPoint3d(DPoint3d::From((double) value, (double) value + 0.25, (double) value + 0.5));
                case PRIMITIVETYPE_String:
                    return binder.BindText(Utf8PrintfString("Value%" PRIu64, value).c_str(), IECSqlBinder::MakeCopy::Yes);
                default:
                    ADD_FAILURE() << "The value generator does not support primitive type " << (int) type << " yet.";
                    return ECSqlStatus::Error;
                }
            }

        ECSqlStatus BindStruct(IECSqlBinder& binder, ECStructClassCR structClass)
            {
            for (ECPropertyCP member : structClass.GetProperties(true))
                {
                ECSqlStatus status = Bind(binder[member->GetName().c_str()], *member);
                if (status != ECSqlStatus::Success)
                    return status;
                }

            return ECSqlStatus::Success;
            }

    public:
        ECSqlStatus Bind(IECSqlBinder& binder, ECPropertyCR prop)
            {
            if (prop.GetIsPrimitive())
                {
                PrimitiveECPropertyCP primitiveProp = prop.GetAsPrimitiveProperty();
                if (primitiveProp->GetEnumeration() != nullptr)
                    return BindEnumerator(binder, *primitiveProp->GetEnumeration());

                return BindPrimitive(binder, primitiveProp->GetType());
                }

            if (prop.GetIsStruct())
                return BindStruct(binder, prop.GetAsStructProperty()->GetType());

            if (prop.GetIsPrimitiveArray())
                {
                for (int i = 0; i < 2; i++)
                    {
                    ECSqlStatus status = BindPrimitive(binder.AddArrayElement(), prop.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType());
                    if (status != ECSqlStatus::Success)
                        return status;
                    }

                return ECSqlStatus::Success;
                }

            if (prop.GetIsStructArray())
                {
                for (int i = 0; i < 2; i++)
                    {
                    ECSqlStatus status = BindStruct(binder.AddArrayElement(), prop.GetAsStructArrayProperty()->GetStructElementType());
                    if (status != ECSqlStatus::Success)
                        return status;
                    }

                return ECSqlStatus::Success;
                }

            ADD_FAILURE() << "The value generator does not support property " << prop.GetName().c_str();
            return ECSqlStatus::Error;
            }
    };

//---------------------------------------------------------------------------------------
// Navigation properties are set in a second pass, so they can point to any inserted instance.
//---------------------------------------------------------------------------------------
BentleyStatus SetNavigationProperties(ECDbCR ecdb, bvector<InsertedInstance> const& inserted)
    {
    for (size_t i = 0; i < inserted.size(); i++)
        {
        ECClassCR ecClass = *inserted[i].m_class;
        for (ECPropertyCP prop : ecClass.GetProperties(true))
            {
            if (!prop->GetIsNavigation())
                continue;

            NavigationECPropertyCP navProp = prop->GetAsNavigationProperty();
            ECRelationshipClassCP relClass = navProp->GetRelationshipClass();
            ECRelationshipConstraintCR relatedEnd = navProp->GetDirection() == ECRelatedInstanceDirection::Forward ? relClass->GetTarget() : relClass->GetSource();
            if (relatedEnd.GetConstraintClasses().empty())
                continue;

            ECClassCP relatedClass = relatedEnd.GetConstraintClasses().front();
            bvector<ECInstanceId> candidates;
            for (InsertedInstance const& candidate : inserted)
                {
                if (candidate.m_class->Is(relatedClass))
                    candidates.push_back(candidate.m_id);
                }

            if (candidates.empty())
                continue;

            ECSqlStatement stmt;
            Utf8PrintfString ecsql("UPDATE %s SET [%s]=? WHERE ECInstanceId=?", ecClass.GetECSqlName().c_str(), prop->GetName().c_str());
            if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql.c_str()) ||
                ECSqlStatus::Success != stmt.GetBinder(1).BindNavigation(candidates[i % candidates.size()], relClass->GetId()) ||
                ECSqlStatus::Success != stmt.BindId(2, inserted[i].m_id) ||
                BE_SQLITE_DONE != stmt.Step())
                {
                ADD_FAILURE() << "Failed to set navigation property: " << ecsql.c_str();
                return ERROR;
                }
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bvector<InsertedInstance> FindInstancesFor(ECRelationshipConstraintCR constraint, bvector<InsertedInstance> const& inserted)
    {
    bvector<InsertedInstance> matches;
    for (InsertedInstance const& candidate : inserted)
        {
        for (ECClassCP constraintClass : constraint.GetConstraintClasses())
            {
            if (constraint.GetIsPolymorphic() ? candidate.m_class->Is(constraintClass) : candidate.m_class == constraintClass)
                {
                matches.push_back(candidate);
                break;
                }
            }
        }

    return matches;
    }

//---------------------------------------------------------------------------------------
// Relates the inserted entity instances through every concrete link table relationship class.
//---------------------------------------------------------------------------------------
BentleyStatus InsertLinkTableRelationships(ECDbCR ecdb, ECSchemaCR schema, int instancesPerClass, ValueGenerator& generator, bvector<InsertedInstance>& inserted)
    {
    bvector<InsertedInstance> insertedRelationships;
    for (ECClassCP ecClass : GetDataClasses(schema))
        {
        if (!ecClass->IsRelationshipClass() || !IsConcreteClass(*ecClass))
            continue;

        ECRelationshipClassCR relClass = *ecClass->GetRelationshipClassCP();
        bvector<InsertedInstance> sources = FindInstancesFor(relClass.GetSource(), inserted);
        bvector<InsertedInstance> targets = FindInstancesFor(relClass.GetTarget(), inserted);
        if (sources.empty() || targets.empty())
            {
            ADD_FAILURE() << "No instances to relate through " << ecClass->GetFullName();
            return ERROR;
            }

        bvector<ECPropertyCP> props;
        Utf8String propList, paramList;
        for (ECPropertyCP prop : ecClass->GetProperties(true))
            {
            propList.append(",[").append(prop->GetName()).append("]");
            paramList.append(",?");
            props.push_back(prop);
            }

        Utf8PrintfString ecsql("INSERT INTO %s (SourceECInstanceId,SourceECClassId,TargetECInstanceId,TargetECClassId%s) VALUES (?,?,?,?%s)",
            ecClass->GetECSqlName().c_str(), propList.c_str(), paramList.c_str());
        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql.c_str()))
            {
            ADD_FAILURE() << "Failed to prepare " << ecsql.c_str();
            return ERROR;
            }

        for (int instanceIndex = 0; instanceIndex < instancesPerClass; instanceIndex++)
            {
            InsertedInstance const& source = sources[instanceIndex % sources.size()];
            InsertedInstance const& target = targets[instanceIndex % targets.size()];
            if (ECSqlStatus::Success != stmt.BindId(1, source.m_id) ||
                ECSqlStatus::Success != stmt.BindId(2, source.m_class->GetId()) ||
                ECSqlStatus::Success != stmt.BindId(3, target.m_id) ||
                ECSqlStatus::Success != stmt.BindId(4, target.m_class->GetId()))
                {
                ADD_FAILURE() << "Failed to bind the relationship ends: " << ecsql.c_str();
                return ERROR;
                }

            for (size_t propIndex = 0; propIndex < props.size(); propIndex++)
                {
                if (ECSqlStatus::Success != generator.Bind(stmt.GetBinder((int) propIndex + 5), *props[propIndex]))
                    return ERROR;
                }

            ECInstanceKey key;
            if (BE_SQLITE_DONE != stmt.Step(key))
                {
                ADD_FAILURE() << "Failed to insert: " << ecsql.c_str() << " " << ecdb.GetLastError().c_str();
                return ERROR;
                }

            insertedRelationships.push_back({key.GetInstanceId(), ecClass});
            stmt.Reset();
            stmt.ClearBindings();
            }
        }

    inserted.insert(inserted.end(), insertedRelationships.begin(), insertedRelationships.end());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// Inserts instances into every concrete entity class and link table relationship class of the
// schema, with a different value for every primitive, so swapped or shared columns cannot hide
// behind equal values.
//---------------------------------------------------------------------------------------
BentleyStatus InsertGeneratedInstances(ECDbCR ecdb, ECSchemaCR schema, int instancesPerClass, bvector<InsertedInstance>& inserted)
    {
    ValueGenerator generator;
    bvector<InsertedInstance> insertedNow;
    for (ECClassCP ecClass : GetDataClasses(schema))
        {
        if (!ecClass->IsEntityClass() || !IsConcreteClass(*ecClass))
            continue;

        bvector<ECPropertyCP> props;
        Utf8String propList, paramList;
        for (ECPropertyCP prop : ecClass->GetProperties(true))
            {
            if (prop->GetIsNavigation())
                continue;

            propList.append(props.empty() ? "[" : ",[").append(prop->GetName()).append("]");
            paramList.append(props.empty() ? "?" : ",?");
            props.push_back(prop);
            }

        Utf8String ecsql = props.empty()
            ? Utf8PrintfString("INSERT INTO %s (ECInstanceId) VALUES (NULL)", ecClass->GetECSqlName().c_str())
            : Utf8PrintfString("INSERT INTO %s (%s) VALUES (%s)", ecClass->GetECSqlName().c_str(), propList.c_str(), paramList.c_str());

        ECSqlStatement stmt;
        if (ECSqlStatus::Success != stmt.Prepare(ecdb, ecsql.c_str()))
            {
            ADD_FAILURE() << "Failed to prepare " << ecsql.c_str();
            return ERROR;
            }

        for (int instanceIndex = 0; instanceIndex < instancesPerClass; instanceIndex++)
            {
            for (size_t propIndex = 0; propIndex < props.size(); propIndex++)
                {
                if (ECSqlStatus::Success != generator.Bind(stmt.GetBinder((int) propIndex + 1), *props[propIndex]))
                    return ERROR;
                }

            ECInstanceKey key;
            if (BE_SQLITE_DONE != stmt.Step(key))
                {
                ADD_FAILURE() << "Failed to insert: " << ecsql.c_str() << " " << ecdb.GetLastError().c_str();
                return ERROR;
                }

            insertedNow.push_back({key.GetInstanceId(), ecClass});
            stmt.Reset();
            stmt.ClearBindings();
            }
        }

    if (SUCCESS != SetNavigationProperties(ecdb, insertedNow) ||
        SUCCESS != InsertLinkTableRelationships(ecdb, schema, instancesPerClass, generator, insertedNow))
        return ERROR;

    inserted.insert(inserted.end(), insertedNow.begin(), insertedNow.end());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String GetSchemaNameFromXml(Utf8StringCR schemaXml)
    {
    Utf8CP attribute = "schemaName=\"";
    size_t start = schemaXml.find(attribute);
    if (start == Utf8String::npos)
        return Utf8String();

    start += strlen(attribute);
    size_t end = schemaXml.find('"', start);
    return end == Utf8String::npos ? Utf8String() : schemaXml.substr(start, end - start);
    }

//---------------------------------------------------------------------------------------
// Finds the XML element of a class, from its start tag up to the end of its end tag.
//---------------------------------------------------------------------------------------
bool FindClassElement(size_t& elementStart, size_t& elementEnd, Utf8StringCR schemaXml, Utf8CP className)
    {
    // Matches <ECEntityClass typeName="X", <ECStructClass typeName="X" etc., but not properties of type X.
    Utf8PrintfString classMarker("Class typeName=\"%s\"", className);
    size_t markerPosition = schemaXml.find(classMarker);
    elementStart = markerPosition == Utf8String::npos ? Utf8String::npos : schemaXml.rfind('<', markerPosition);
    size_t startTagEnd = markerPosition == Utf8String::npos ? Utf8String::npos : schemaXml.find('>', markerPosition);
    if (elementStart == Utf8String::npos || startTagEnd == Utf8String::npos)
        {
        ADD_FAILURE() << "Class " << className << " not found in the schema XML.";
        return false;
        }

    if (schemaXml[startTagEnd - 1] == '/')
        {
        elementEnd = startTagEnd + 1;
        return true;
        }

    Utf8String elementName = schemaXml.substr(elementStart + 1, markerPosition + strlen("Class") - elementStart - 1);
    Utf8PrintfString endTag("</%s>", elementName.c_str());
    size_t endTagStart = schemaXml.find(endTag, markerPosition);
    if (endTagStart == Utf8String::npos)
        {
        ADD_FAILURE() << "End of class " << className << " not found in the schema XML.";
        return false;
        }

    elementEnd = endTagStart + endTag.size();
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus CreateBaselineSeed(BaselineSeed& seed, SchemaUpdateBaseline const& baseline)
    {
    seed.m_schemaName = GetSchemaNameFromXml(baseline.GetSchemaXml());
    if (seed.m_schemaName.empty())
        {
        ADD_FAILURE() << "Baseline " << baseline.GetName().c_str() << " has no schemaName attribute.";
        return ERROR;
        }

    ECDb seedDb;
    Utf8PrintfString fileName("SchemaUpdateBaseline_%s.ecdb", baseline.GetName().c_str());
    if (BE_SQLITE_OK != ECDbTestFixture::CreateECDb(seedDb, fileName.c_str()))
        return ERROR;

    TestIssueListener issueListener;
    seedDb.AddIssueListener(issueListener);
    const BentleyStatus importStatus = TestHelper(seedDb).ImportSchema(SchemaItem(baseline.GetSchemaXml()), SchemaManager::SchemaImportOptions::VerifyPersistedSchemas);
    seedDb.RemoveIssueListener();
    if (SUCCESS != importStatus)
        {
        PrintRejectionReasons("Import of the baseline schema", issueListener);
        ADD_FAILURE() << "Failed to import the schema of baseline " << baseline.GetName().c_str();
        return ERROR;
        }

    ECSchemaCP schema = seedDb.Schemas().GetSchema(seed.m_schemaName);
    bvector<InsertedInstance> inserted;
    if (schema == nullptr || SUCCESS != InsertGeneratedInstances(seedDb, *schema, baseline.GetInstancesPerClass(), inserted))
        return ERROR;

    if (BE_SQLITE_OK != seedDb.SaveChanges())
        return ERROR;

    seed.m_filePath.AssignUtf8(seedDb.GetDbFileName());
    seedDb.CloseDb();
    return SUCCESS;
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaUpdateTestFixture::SetupFromBaseline(SchemaUpdateBaseline const& baseline)
    {
    CloseECDb();
    auto& seeds = GetBaselineSeeds();
    auto seedIt = seeds.find(baseline.GetName());
    if (seedIt == seeds.end())
        {
        BaselineSeed seed;
        if (SUCCESS != CreateBaselineSeed(seed, baseline))
            return ERROR;

        seedIt = seeds.insert({baseline.GetName(), seed}).first;
        }

    m_baseline = &baseline;
    m_schemaName = seedIt->second.m_schemaName;
    m_lastUpdateFailed = false;

    Utf8PrintfString fileName("%s_%s.ecdb", BeTest::GetNameOfCurrentTestCase(), BeTest::GetNameOfCurrentTest());
    if (BE_SQLITE_OK != CloneECDb(fileName.c_str(), seedIt->second.m_filePath))
        return ERROR;

    if (!EnableECSqlExperimentalFeatures(m_ecdb, true))
        return ERROR;

    m_dataBeforeUpdate.clear();
    ReadData(m_dataBeforeUpdate);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SchemaUpdateTestFixture::UpdateSchema(Utf8StringCR schemaXml, SchemaVersionChange versionChange, SchemaManager::SchemaImportOptions options)
    {
    ECSchemaCP persistedSchema = m_ecdb.Schemas().GetSchema(m_schemaName);
    if (persistedSchema == nullptr)
        {
        ADD_FAILURE() << "Schema " << m_schemaName.c_str() << " not found.";
        return ERROR;
        }

    m_schemaXmlBeforeUpdate = GetPersistedSchemaXml();
    const uint32_t persistedRead = persistedSchema->GetVersionRead();
    const uint32_t persistedWrite = persistedSchema->GetVersionWrite();
    const uint32_t persistedMinor = persistedSchema->GetVersionMinor();

    ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
    BeFileName ecdbSchemaSearchPath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(ecdbSchemaSearchPath);
    ecdbSchemaSearchPath.AppendToPath(L"ECSchemas").AppendToPath(L"ECDb");
    context->AddSchemaPath(ecdbSchemaSearchPath);
    context->AddSchemaLocater(m_ecdb.GetSchemaLocater());

    ECSchemaPtr schema;
    if (SchemaReadStatus::Success != ECSchema::ReadFromXmlString(schema, schemaXml.c_str(), *context))
        {
        // An invalid test, not a rejected update.
        ADD_FAILURE() << "Failed to read the updated schema XML.";
        return ERROR;
        }

    switch (versionChange)
        {
        case SchemaVersionChange::None:
            schema->SetVersionRead(persistedRead);
            schema->SetVersionWrite(persistedWrite);
            schema->SetVersionMinor(persistedMinor);
            break;
        case SchemaVersionChange::Minor:
            schema->SetVersionRead(persistedRead);
            schema->SetVersionWrite(persistedWrite);
            schema->SetVersionMinor(persistedMinor + 1);
            break;
        case SchemaVersionChange::Major:
            schema->SetVersionRead(persistedRead + 1);
            schema->SetVersionWrite(0);
            schema->SetVersionMinor(0);
            break;
        }

    // ECDb takes one issue listener at a time. If the test has its own, that one gets the issues.
    TestIssueListener issueListener;
    const bool isListening = SUCCESS == m_ecdb.AddIssueListener(issueListener);
    Savepoint savepoint(m_ecdb, "SchemaUpdate");
    const bool isImported = SUCCESS == m_ecdb.Schemas().ImportSchemas(context->GetCache().GetSchemas(), options | SchemaManager::SchemaImportOptions::VerifyPersistedSchemas);
    if (isListening)
        m_ecdb.RemoveIssueListener();

    m_lastUpdateIssues = issueListener.m_issues;
    m_isRejectionReasonVerified = false;
    if (!isImported)
        {
        savepoint.Cancel();
        m_lastUpdateFailed = true;
        PrintRejectionReasons("Schema update", issueListener);
        return ERROR;
        }

    savepoint.Commit();
    m_lastUpdateFailed = false;
    return BE_SQLITE_OK == m_ecdb.SaveChanges() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaUpdateTestFixture::ExpectRejectionReason(Utf8CP issueId)
    {
    m_isRejectionReasonVerified = true;
    if (!m_lastUpdateFailed)
        {
        ADD_FAILURE() << "The schema update was accepted, expected a rejection with " << issueId << ".";
        return;
        }

    Utf8String reportedIssues;
    for (ReportedIssue const& issue : m_lastUpdateIssues)
        {
        if (0 == strcmp(issueId, issue.id.m_issueId))
            return;

        reportedIssues.append("\n    ").append(issue.id.m_issueId).append(": ").append(issue.message);
        }

    ADD_FAILURE() << "The schema update was not rejected with " << issueId << ". Reported issues:" << reportedIssues.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaUpdateTestFixture::VerifyData(ExpectedDataChanges const& expectedChanges)
    {
    if (m_lastUpdateFailed && !m_isRejectionReasonVerified)
        ADD_FAILURE() << "The schema update was rejected. Call ExpectRejectionReason before VerifyData to state why.";

    // Reopening drops all caches, like an application that opens the file after the update.
    ASSERT_EQ(BE_SQLITE_OK, ReopenWithExperimentalFeatures());

    if (m_lastUpdateFailed)
        EXPECT_STREQ(m_schemaXmlBeforeUpdate.c_str(), GetPersistedSchemaXml().c_str()) << "A rejected update must not change the persisted schema.";

    DataSnapshot dataAfterUpdate;
    ReadData(dataAfterUpdate);

    for (auto const& [instanceId, before] : m_dataBeforeUpdate)
        {
        Utf8PrintfString context("%s %s", before.m_className.c_str(), instanceId.ToHexStr().c_str());
        auto afterIt = dataAfterUpdate.find(instanceId);
        if (expectedChanges.AreInstancesDeleted(before.m_className))
            {
            EXPECT_TRUE(afterIt == dataAfterUpdate.end()) << context.c_str() << " was expected to be deleted.";
            continue;
            }

        if (afterIt == dataAfterUpdate.end())
            {
            ADD_FAILURE() << context.c_str() << " is missing after the update.";
            continue;
            }

        InstanceData const& after = afterIt->second;
        EXPECT_EQ(before.m_classId, after.m_classId) << context.c_str() << " changed its class to " << after.m_className.c_str();
        ECClassCP classAfterUpdate = m_ecdb.Schemas().GetClass(after.m_classId);
        if (classAfterUpdate == nullptr)
            {
            ADD_FAILURE() << context.c_str() << " has a class that cannot be loaded.";
            continue;
            }

        CompareValues(context.c_str(), ProjectValues(before.m_values, *classAfterUpdate), after.m_values);
        }

    for (auto const& [instanceId, after] : dataAfterUpdate)
        {
        if (m_dataBeforeUpdate.find(instanceId) == m_dataBeforeUpdate.end())
            ADD_FAILURE() << after.m_className.c_str() << " " << instanceId.ToHexStr().c_str() << " appeared after the update.";
        }

    VerifyIntegrity();
    VerifyWriteProbe();
    }

//---------------------------------------------------------------------------------------
// Reads all instances of the schema with ONLY queries into the snapshot, then checks that
// polymorphic queries of every data class return exactly the matching instances and values.
//---------------------------------------------------------------------------------------
void SchemaUpdateTestFixture::ReadData(DataSnapshot& snapshot) const
    {
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema(m_schemaName);
    ASSERT_TRUE(schema != nullptr) << "Schema " << m_schemaName.c_str() << " not found.";
    const JsonECSqlSelectAdapter::FormatOptions formatOptions(JsonECSqlSelectAdapter::MemberNameCasing::KeepOriginal, ECJsonInt64Format::AsNumber);
    bvector<ECClassCP> dataClasses = GetDataClasses(*schema);

    for (ECClassCP ecClass : dataClasses)
        {
        if (!IsConcreteClass(*ecClass))
            continue;

        Utf8String ecsql = BuildSelectECSql(*ecClass, true);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        JsonECSqlSelectAdapter adapter(stmt, formatOptions);
        while (BE_SQLITE_ROW == stmt.Step())
            {
            const ECInstanceId instanceId = stmt.GetValueId<ECInstanceId>(0);
            const ECClassId classId = stmt.GetValueId<ECClassId>(1);
            EXPECT_EQ(ecClass->GetId(), classId) << ecsql.c_str() << " returned an instance of another class: " << instanceId.ToHexStr().c_str();
            EXPECT_TRUE(snapshot.find(instanceId) == snapshot.end()) << "Instance " << instanceId.ToHexStr().c_str() << " was returned by more than one ONLY query.";
            snapshot[instanceId] = InstanceData{classId, ecClass->GetFullName(), ReadValues(stmt, adapter, *ecClass)};
            }
        }

    for (ECClassCP ecClass : dataClasses)
        {
        Utf8String ecsql = BuildSelectECSql(*ecClass, false);
        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql.c_str())) << ecsql.c_str();
        JsonECSqlSelectAdapter adapter(stmt, formatOptions);
        size_t rowCount = 0;
        while (BE_SQLITE_ROW == stmt.Step())
            {
            rowCount++;
            const ECInstanceId instanceId = stmt.GetValueId<ECInstanceId>(0);
            Utf8PrintfString context("%s, instance %s", ecsql.c_str(), instanceId.ToHexStr().c_str());
            auto instanceIt = snapshot.find(instanceId);
            if (instanceIt == snapshot.end())
                {
                ADD_FAILURE() << context.c_str() << " was not returned by any ONLY query.";
                continue;
                }

            EXPECT_EQ(instanceIt->second.m_classId, stmt.GetValueId<ECClassId>(1)) << context.c_str();
            CompareValues(context.c_str(), ProjectValues(instanceIt->second.m_values, *ecClass), ReadValues(stmt, adapter, *ecClass));
            }

        size_t expectedRowCount = 0;
        for (auto const& entry : snapshot)
            {
            ECClassCP instanceClass = m_ecdb.Schemas().GetClass(entry.second.m_classId);
            if (instanceClass != nullptr && instanceClass->Is(ecClass))
                expectedRowCount++;
            }

        EXPECT_EQ(expectedRowCount, rowCount) << ecsql.c_str();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SchemaUpdateTestFixture::GetPersistedSchemaXml() const
    {
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema(m_schemaName);
    Utf8String xml;
    if (schema == nullptr || SchemaWriteStatus::Success != schema->WriteToXmlString(xml))
        ADD_FAILURE() << "Failed to serialize persisted schema " << m_schemaName.c_str();

    return xml;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaUpdateTestFixture::VerifyIntegrity() const
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "PRAGMA integrity_check"));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        if (stmt.GetValueBoolean(2))
            continue;

        Utf8String check = stmt.GetValueText(1);
        ADD_FAILURE() << "Integrity check " << check.c_str() << " failed.";
        ExpectNoRows(Utf8PrintfString("PRAGMA integrity_check(%s)", check.c_str()).c_str());
        }

    // check_missing_child_rows is left out: it only looks at bis_Element.
    // validate_persisted_mappings reloads the table definitions, which class maps already in the cache still point to.
    m_ecdb.ClearECDbCache();
    ExpectNoRows("PRAGMA validate_persisted_mappings");
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaUpdateTestFixture::ExpectNoRows(Utf8CP ecsql) const
    {
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, ecsql)) << ecsql;
    JsonValue rows = GetHelper().ExecutePreparedECSql(stmt);
    ASSERT_TRUE(rows.m_value.isArray()) << "Failed to execute " << ecsql;
    EXPECT_EQ(0u, rows.m_value.size()) << ecsql << " returned " << rows.ToString().c_str();
    }

//---------------------------------------------------------------------------------------
// Inserts one new instance per concrete class and reads it back. Every primitive got a
// different value, so two properties reading the same value share a column.
//---------------------------------------------------------------------------------------
void SchemaUpdateTestFixture::VerifyWriteProbe()
    {
    // The probe instances are rolled back, so the file keeps the state after the update.
    Savepoint savepoint(m_ecdb, "WriteProbe");
    VerifyWriteProbeInstances();
    savepoint.Cancel();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaUpdateTestFixture::VerifyWriteProbeInstances()
    {
    ECSchemaCP schema = m_ecdb.Schemas().GetSchema(m_schemaName);
    ASSERT_TRUE(schema != nullptr);
    bvector<InsertedInstance> inserted;
    ASSERT_EQ(SUCCESS, InsertGeneratedInstances(m_ecdb, *schema, 1, inserted)) << "Write probe failed to insert.";

    DataSnapshot data;
    ReadData(data);
    for (InsertedInstance const& instance : inserted)
        {
        Utf8PrintfString context("Write probe %s %s", instance.m_class->GetFullName(), instance.m_id.ToHexStr().c_str());
        auto instanceIt = data.find(instance.m_id);
        if (instanceIt == data.end())
            {
            ADD_FAILURE() << context.c_str() << " cannot be read back.";
            continue;
            }

        std::map<Utf8String, Utf8String> propertyNameByValue;
        for (ECPropertyCP prop : instance.m_class->GetProperties(true))
            {
            if (prop->GetIsNavigation())
                continue;

            auto valueIt = instanceIt->second.m_values.find(prop->GetName());
            if (valueIt == instanceIt->second.m_values.end())
                {
                ADD_FAILURE() << context.c_str() << ", property " << prop->GetName().c_str() << " reads NULL.";
                continue;
                }

            // Booleans and enumerators repeat by design.
            const bool isBoolean = (prop->GetIsPrimitive() && prop->GetAsPrimitiveProperty()->GetType() == PRIMITIVETYPE_Boolean) ||
                (prop->GetIsPrimitiveArray() && prop->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType() == PRIMITIVETYPE_Boolean);
            const bool isEnumeration = prop->GetIsPrimitive() && prop->GetAsPrimitiveProperty()->GetEnumeration() != nullptr;
            if (isBoolean || isEnumeration)
                continue;

            auto insertResult = propertyNameByValue.insert({valueIt->second, prop->GetName()});
            EXPECT_TRUE(insertResult.second) << context.c_str() << ", properties " << insertResult.first->second.c_str() << " and " << prop->GetName().c_str() << " read the same value " << valueIt->second.c_str();
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult SchemaUpdateTestFixture::ReopenWithExperimentalFeatures()
    {
    DbResult result = ReopenECDb();
    if (result != BE_SQLITE_OK)
        return result;

    return EnableECSqlExperimentalFeatures(m_ecdb, true) ? BE_SQLITE_OK : BE_SQLITE_ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaUpdateTestFixture::ReplaceInClass(Utf8StringR schemaXml, Utf8CP className, Utf8CP oldText, Utf8CP newText)
    {
    size_t elementStart, elementEnd;
    if (!FindClassElement(elementStart, elementEnd, schemaXml, className))
        return;

    size_t position = schemaXml.find(oldText, elementStart);
    if (position == Utf8String::npos || position >= elementEnd || schemaXml.find(oldText, position + 1) < elementEnd)
        {
        ADD_FAILURE() << "Expected exactly one occurrence of '" << oldText << "' in class " << className << ".";
        return;
        }

    schemaXml.replace(position, strlen(oldText), newText);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaUpdateTestFixture::RemoveClass(Utf8StringR schemaXml, Utf8CP className)
    {
    size_t elementStart, elementEnd;
    if (FindClassElement(elementStart, elementEnd, schemaXml, className))
        schemaXml.erase(elementStart, elementEnd - elementStart);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SchemaUpdateTestFixture::AppendToSchema(Utf8StringR schemaXml, Utf8CP schemaItemsXml)
    {
    size_t position = schemaXml.rfind("</ECSchema>");
    if (position == Utf8String::npos)
        {
        ADD_FAILURE() << "End of the schema not found in the schema XML.";
        return;
        }

    schemaXml.insert(position, schemaItemsXml);
    }

END_ECDBUNITTESTS_NAMESPACE
