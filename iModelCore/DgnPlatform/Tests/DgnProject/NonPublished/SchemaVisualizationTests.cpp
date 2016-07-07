/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/NonPublished/SchemaVisualizationTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#ifdef NOT_NOW_ANDROID_STATIC_BB_1_ISSUE

#ifdef BENTLEY_WIN32

#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/GenericDomain.h>
#include <ECObjects/ECObjectsAPI.h>

#include <GraphViz/gvc.h>
#include <GraphViz/gvplugin_render.h>
#include <GraphViz/gvplugin_device.h>
#include <GraphViz/gvplugin_textlayout.h>

//[SVT_META]
#define METASCHEMA_XML "<?xml version='1.0' encoding='utf-8'?>"\
"<ECSchema schemaName='MetaSchema' nameSpacePrefix='ms' version='3.1' xmlns='http://www.bentley.com/schemas/Bentley.ECXML.2.0'>"\
"    <ECSchemaReference name='Bentley_Standard_CustomAttributes' version='01.00' prefix='bsca' />"\
"    <ECSchemaReference name='EditorCustomAttributes' version='01.00' prefix='beca' />"\
"    <ECSchemaReference name='Bentley_Standard_Classes' version='01.00' prefix='bsm' />"\
"    <ECClass typeName='ECSchemaDef'>"\
"        <ECProperty propertyName='Name' typeName='string'></ECProperty>"\
"        <ECProperty propertyName='DisplayLabel' typeName='string'></ECProperty>"\
"        <ECProperty propertyName='Description' typeName='string'></ECProperty>"\
"        <ECProperty propertyName='NameSpacePrefix' typeName='string'></ECProperty>"\
"        <ECProperty propertyName='VersionMajor' typeName='int'></ECProperty>"\
"        <ECProperty propertyName='VersionMinor' typeName='int'></ECProperty>"\
"    </ECClass>"\
"    <ECClass typeName='ECClassDef'>"\
"        <ECProperty propertyName='Name' typeName='string'></ECProperty>"\
"        <ECProperty propertyName='DisplayLabel' typeName='string'></ECProperty>"\
"        <ECProperty propertyName='Description' typeName='string'></ECProperty>"\
"        <ECProperty propertyName='Schema' typeName='string'></ECProperty>"\
"        <ECProperty propertyName='IsDomainClass' typeName='boolean'></ECProperty>"\
"        <ECProperty propertyName='IsCustomAttributeClass' typeName='boolean'></ECProperty>"\
"        <ECProperty propertyName='IsStruct' typeName='boolean'></ECProperty>"\
"        <ECProperty propertyName='IsRelationshipClass' typeName='boolean'></ECProperty>"\
"        <ECProperty propertyName='HasBaseClasses' typeName='boolean'></ECProperty>"\
"    </ECClass>"\
"    <ECClass typeName='ECRelationshipClassDef'>"\
"        <BaseClass>ECClassDef</BaseClass>"\
"        <ECProperty propertyName='IsExplicit' typeName='boolean'></ECProperty>"\
"        <ECProperty propertyName='StrengthDirection' typeName='int'>"\
"            <ECCustomAttributes>"\
"                <StandardValues xmlns='EditorCustomAttributes.01.00'>"\
"                    <MustBeFromList>True</MustBeFromList>"\
"                    <ValueMap>"\
"                        <ValueMap>"\
"                            <DisplayString>Forward</DisplayString>"\
"                            <Value>1</Value>"\
"                        </ValueMap>"\
"                        <ValueMap>"\
"                            <DisplayString>Backward</DisplayString>"\
"                            <Value>2</Value>"\
"                        </ValueMap>"\
"                    </ValueMap>"\
"                </StandardValues>"\
"            </ECCustomAttributes>"\
"        </ECProperty>"\
"        <ECProperty propertyName='Strength' typeName='int'>"\
"            <ECCustomAttributes>"\
"                <StandardValues xmlns='EditorCustomAttributes.01.00'>"\
"                    <MustBeFromList>True</MustBeFromList>"\
"                    <ValueMap>"\
"                        <ValueMap>"\
"                            <DisplayString>Referencing</DisplayString>"\
"                            <Value>0</Value>"\
"                        </ValueMap>"\
"                        <ValueMap>"\
"                            <DisplayString>Holding</DisplayString>"\
"                            <Value>1</Value>"\
"                        </ValueMap>"\
"                        <ValueMap>"\
"                            <DisplayString>Embedding</DisplayString>"\
"                            <Value>2</Value>"\
"                        </ValueMap>"\
"                    </ValueMap>"\
"                </StandardValues>"\
"            </ECCustomAttributes>"\
"        </ECProperty>"\
"    </ECClass>"\
"    <ECClass typeName='ECPropertyDef'>"\
"        <ECProperty propertyName='Name' typeName='string'></ECProperty>"\
"        <ECProperty propertyName='DisplayLabel' typeName='string'></ECProperty>"\
"        <ECProperty propertyName='Description' typeName='string'></ECProperty>"\
"        <ECProperty propertyName='TypeName' typeName='string'></ECProperty>"\
"        <ECProperty propertyName='OriginClass' typeName='string'></ECProperty>"\
"        <ECProperty propertyName='IsArray' typeName='boolean'></ECProperty>"\
"        <ECProperty propertyName='MinOccurs' typeName='string'></ECProperty>"\
"        <ECProperty propertyName='MaxOccurs' typeName='string'></ECProperty>"\
"    </ECClass>"\
"    <ECRelationshipClass typeName='SchemaHasClass' isDomainClass='True' strength='embedding' strengthDirection='forward'>"\
"        <Source cardinality='(1,1)' roleLabel='Properties' polymorphic='True'>"\
"            <Class class='ECSchemaDef' />"\
"        </Source>"\
"        <Target cardinality='(0,N)' roleLabel='Is From' polymorphic='True'>"\
"            <Class class='ECClassDef' />"\
"        </Target>"\
"    </ECRelationshipClass>"\
"    <ECRelationshipClass typeName='SchemaHasSchemaReference' isDomainClass='True' strength='embedding' strengthDirection='forward'>"\
"        <Source cardinality='(1,1)' roleLabel='References' polymorphic='True'>"\
"            <Class class='ECSchemaDef' />"\
"        </Source>"\
"        <Target cardinality='(0,N)' roleLabel='Is Referenced By' polymorphic='True'>"\
"            <Class class='ECSchemaDef' />"\
"        </Target>"\
"    </ECRelationshipClass>"\
"    <ECRelationshipClass typeName='ClassHasBaseClass' isDomainClass='True' strength='embedding' strengthDirection='forward'>"\
"        <Source cardinality='(0,N)' roleLabel='Parents' polymorphic='True'>"\
"            <Class class='ECClassDef' />"\
"        </Source>"\
"        <Target cardinality='(0,N)' roleLabel='Children' polymorphic='True'>"\
"            <Class class='ECClassDef' />"\
"        </Target>"\
"    </ECRelationshipClass>"\
"    <ECRelationshipClass typeName='ClassHasProperty' isDomainClass='True' strength='embedding' strengthDirection='forward'>"\
"        <Source cardinality='(1,1)' roleLabel='Properties' polymorphic='True'>"\
"            <Class class='ECClassDef' />"\
"        </Source>"\
"        <Target cardinality='(0,N)' roleLabel='Is From' polymorphic='True'>"\
"            <Class class='ECPropertyDef' />"\
"        </Target>"\
"    </ECRelationshipClass>"\
"    <ECRelationshipClass typeName='ClassHasLocalProperty' isDomainClass='True' strength='embedding' strengthDirection='forward'>"\
"        <Source cardinality='(1,1)' roleLabel='Properties' polymorphic='True'>"\
"            <Class class='ECClassDef' />"\
"        </Source>"\
"        <Target cardinality='(0,N)' roleLabel='Is From' polymorphic='True'>"\
"            <Class class='ECPropertyDef' />"\
"        </Target>"\
"    </ECRelationshipClass>"\
"    <ECRelationshipClass typeName='RelationshipHasSourceConstraintClass' isDomainClass='True' strength='referencing' strengthDirection='forward'>"\
"        <Source cardinality='(0,N)' roleLabel='Source Constraints' polymorphic='True'>"\
"            <Class class='ECRelationshipClassDef' />"\
"        </Source>"\
"        <Target cardinality='(0,N)' roleLabel='Source Constraint For' polymorphic='False'>"\
"            <Class class='ECClassDef' />"\
"        </Target>"\
"    </ECRelationshipClass>"\
"    <ECRelationshipClass typeName='RelationshipHasTargetConstraintClass' isDomainClass='True' strength='referencing' strengthDirection='forward'>"\
"        <Source cardinality='(0,N)' roleLabel='Target Constraints' polymorphic='True'>"\
"            <Class class='ECRelationshipClassDef' />"\
"        </Source>"\
"        <Target cardinality='(0,N)' roleLabel='Target Constraint For' polymorphic='False'>"\
"            <Class class='ECClassDef' />"\
"        </Target>"\
"    </ECRelationshipClass>"\
"</ECSchema>"\

#define METASCHEMA_CLASS_SchemaDef                                  "ECSchemaDef"
#define METASCHEMA_PROPERTY_SchemaDef_Name                          "Name"
#define METASCHEMA_PROPERTY_SchemaDef_DisplayLabel                  "DisplayLabel"
#define METASCHEMA_PROPERTY_SchemaDef_NameSpacePrefix               "NameSpacePrefix"
#define METASCHEMA_PROPERTY_SchemaDef_Description                   "Description"
#define METASCHEMA_PROPERTY_SchemaDef_VersionMajor                  "VersionMajor"
#define METASCHEMA_PROPERTY_SchemaDef_VersionMinor                  "VersionMinor"

#define METASCHEMA_CLASS_ClassDef                                   "ECClassDef"
#define METASCHEMA_PROPERTY_ClassDef_Name                           "Name"
#define METASCHEMA_PROPERTY_ClassDef_DisplayLabel                   "DisplayLabel"
#define METASCHEMA_PROPERTY_ClassDef_Schema                         "Schema"
#define METASCHEMA_PROPERTY_ClassDef_Description                    "Description"
#define METASCHEMA_PROPERTY_ClassDef_IsStruct                       "IsStruct"
#define METASCHEMA_PROPERTY_ClassDef_IsCustomAttributeClass         "IsCustomAttributeClass"
#define METASCHEMA_PROPERTY_ClassDef_IsDomainClass                  "IsDomainClass"
#define METASCHEMA_PROPERTY_ClassDef_HasBaseClasses                 "HasBaseClasses"
#define METASCHEMA_PROPERTY_ClassDef_IsRelationshipClass            "IsRelationshipClass"
#define METASCHEMA_PROPERTY_ClassDef_BaseClasses                    "BaseClasses"

#define METASCHEMA_CLASS_RelClassDef                                "ECRelationshipClassDef"
#define METASCHEMA_PROPERTY_RelClassDef_IsExplicit                  "IsExplicit"

#define METASCHEMA_CLASS_PropertyDef                                "ECPropertyDef"
#define METASCHEMA_PROPERTY_PropertyDef_Name                        "Name"
#define METASCHEMA_PROPERTY_PropertyDef_OriginClass                 "OriginClass"
#define METASCHEMA_PROPERTY_PropertyDef_OriginClassName             "OriginClassName"
#define METASCHEMA_PROPERTY_PropertyDef_BaseProperty                "BaseProperty"
#define METASCHEMA_PROPERTY_PropertyDef_Overrides                   "Overrides"
#define METASCHEMA_PROPERTY_PropertyDef_DisplayLabel                "DisplayLabel"
#define METASCHEMA_PROPERTY_PropertyDef_TypeName                    "TypeName"
#define METASCHEMA_PROPERTY_PropertyDef_Description                 "Description"
#define METASCHEMA_PROPERTY_PropertyDef_IsArray                     "IsArray"
#define METASCHEMA_PROPERTY_PropertyDef_MinOccurs                   "MinOccurs"
#define METASCHEMA_PROPERTY_PropertyDef_MaxOccurs                   "MaxOccurs"
#define METASCHEMA_PROPERTY_PropertyDef_ReadOnly                    "ReadOnly"
#define METASCHEMA_PROPERTY_PropertyDef_Priority                    "Priority"
#define METASCHEMA_PROPERTY_PropertyDef_IsTransient                 "IsTransient"

#define METASCHEMA_RELCLASS_SchemaHasClass                          "SchemaHasClass"
#define METASCHEMA_RELCLASS_SchemaHasSchemaReference                "SchemaHasSchemaReference"
#define METASCHEMA_RELCLASS_ClassHasBaseClass                       "ClassHasBaseClass"
#define METASCHEMA_RELCLASS_ClassHasProperty                        "ClassHasProperty"
#define METASCHEMA_RELCLASS_ClassHasLocalProperty                   "ClassHasLocalProperty"
#define METASCHEMA_RELCLASS_RelationshipHasSourceConstraintClass    "RelationshipHasSourceConstraintClass"
#define METASCHEMA_RELCLASS_RelationshipHasTargetConstraintClass    "RelationshipHasTargetConstraintClass"

using namespace BentleyApi::ECN;

//=======================================================================================
// @bsistruct                                                   Mike.Embick     08/15
//=======================================================================================
struct SchemaVisualizationTests : testing::Test
    {
    ECSchemaPtr m_metaSchema;
    ScopedDgnHost host;
    ECSchemaCR GetMetaSchema() { return *m_metaSchema; }

    SchemaVisualizationTests()
        {
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext();
        SchemaReadStatus status = ECSchema::ReadFromXmlString(m_metaSchema, METASCHEMA_XML, *schemaContext);
        EXPECT_EQ(SchemaReadStatus::Success, status);
        }
    };

//=======================================================================================
// @bsistruct                                                   Mike.Embick     09/15
//=======================================================================================
struct MetaSchemaInstanceGenerator //[SVT_MSIG]
    {
    ECSchemaCR m_metaSchema;
    bvector<ECSchemaCP> & m_schemaScope;
    MetaSchemaInstanceGenerator(ECSchemaCR metaSchema, bvector<ECSchemaCP> & schemaScope) : m_metaSchema(metaSchema), m_schemaScope(schemaScope) {};

    bool FindSchemaInScope(Utf8CP schemaName);
    bool GetSchemaFromScope(ECSchemaCP & placeSchemaHere, Utf8CP schemaName);
    void GenerateInstances(bvector<IECInstancePtr> & instances, bvector<ECClassCP> & queryClasses);
    void GenerateRelatedInstances(bvector<IECInstancePtr> & relatedInstances, IECInstanceCR sourceInstance, ECRelationshipClassCR queryRelationship);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool MetaSchemaInstanceGenerator::FindSchemaInScope(Utf8CP schemaName)
    {
    for (ECSchemaCP const& schemaInScope : m_schemaScope)
        {
        if (schemaName == schemaInScope->GetName().c_str())
            {
            return true;
            }
        }
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool MetaSchemaInstanceGenerator::GetSchemaFromScope(ECSchemaCP & placeSchemaHere, Utf8CP schemaName)
    {
    for (ECSchemaCP const& schemaInScope : m_schemaScope)
        {
        if (schemaName == schemaInScope->GetName())
            {
            placeSchemaHere = schemaInScope;
            return true;
            }
        }
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void MetaSchemaInstanceGenerator::GenerateInstances(bvector<IECInstancePtr> & instances, bvector<ECClassCP> & queryClasses)
    {
    ECValue v;

    for (ECClassCP const& qClass : queryClasses)
        {
        if (METASCHEMA_CLASS_SchemaDef == qClass->GetName())
            {
            for (ECSchemaCP const& ecSchema : m_schemaScope)
                {
                IECInstancePtr schemaInstance = m_metaSchema.GetClassCP(METASCHEMA_CLASS_SchemaDef)->GetDefaultStandaloneEnabler()->CreateInstance();

                v.SetUtf8CP(ecSchema->GetName().c_str());
                schemaInstance->SetValue(METASCHEMA_PROPERTY_SchemaDef_Name, v);
                v.SetUtf8CP(ecSchema->GetDisplayLabel().c_str());
                schemaInstance->SetValue(METASCHEMA_PROPERTY_SchemaDef_DisplayLabel, v);
                v.SetUtf8CP(ecSchema->GetNamespacePrefix().c_str());
                schemaInstance->SetValue(METASCHEMA_PROPERTY_SchemaDef_NameSpacePrefix, v);
                v.SetUtf8CP(ecSchema->GetDescription().c_str());
                schemaInstance->SetValue(METASCHEMA_PROPERTY_SchemaDef_Description, v);
                v.SetInteger(ecSchema->GetVersionMajor());
                schemaInstance->SetValue(METASCHEMA_PROPERTY_SchemaDef_VersionMajor, v);
                v.SetInteger(ecSchema->GetVersionMinor());
                schemaInstance->SetValue(METASCHEMA_PROPERTY_SchemaDef_VersionMinor, v);

                instances.push_back(schemaInstance);
                }
            continue;
            }
        if (METASCHEMA_CLASS_ClassDef == qClass->GetName())
            {
            for (ECSchemaCP const& ecSchema : m_schemaScope)
                {
                for (ECClassCP const& ecClass : ecSchema->GetClasses())
                    {
                    if (nullptr == ecClass->GetRelationshipClassCP())
                        {
                        IECInstancePtr classInstance = m_metaSchema.GetClassCP(METASCHEMA_CLASS_ClassDef)->GetDefaultStandaloneEnabler()->CreateInstance();

                        v.SetUtf8CP(ecClass->GetName().c_str());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Name, v);
                        v.SetUtf8CP(ecClass->GetDisplayLabel().c_str());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_DisplayLabel, v);
                        v.SetUtf8CP(ecSchema->GetName().c_str());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Schema, v);
                        v.SetUtf8CP(ecClass->GetDescription().c_str());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Description, v);
                        v.SetBoolean(ecClass->IsStructClass());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsStruct, v);
                        v.SetBoolean(ecClass->IsCustomAttributeClass());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsCustomAttributeClass, v);
                        v.SetBoolean(ecClass->IsEntityClass());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsDomainClass, v);
                        v.SetBoolean(ecClass->HasBaseClasses());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_HasBaseClasses, v);
                        v.SetBoolean(ecClass->IsRelationshipClass());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsRelationshipClass, v);

                        instances.push_back(classInstance);
                        }
                    }
                }
            continue;
            }
        if (METASCHEMA_CLASS_RelClassDef == qClass->GetName())
            {
            for (ECSchemaCP const& ecSchema : m_schemaScope)
                {
                for (ECClassCP const& ecClass : ecSchema->GetClasses())
                    {
                    if (ecClass->IsRelationshipClass())
                        {
                        IECInstancePtr classInstance = m_metaSchema.GetClassCP(METASCHEMA_CLASS_RelClassDef)->GetDefaultStandaloneEnabler()->CreateInstance();

                        v.SetUtf8CP(ecClass->GetName().c_str());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Name, v);
                        v.SetUtf8CP(ecClass->GetDisplayLabel().c_str());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_DisplayLabel, v);
                        v.SetUtf8CP(ecSchema->GetName().c_str());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Schema, v);
                        v.SetUtf8CP(ecClass->GetDescription().c_str());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Description, v);
                        v.SetBoolean(ecClass->IsStructClass());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsStruct, v);
                        v.SetBoolean(ecClass->IsCustomAttributeClass());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsCustomAttributeClass, v);
                        v.SetBoolean(ecClass->IsEntityClass());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsDomainClass, v);
                        v.SetBoolean(ecClass->HasBaseClasses());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_HasBaseClasses, v);
                        v.SetBoolean(ecClass->IsRelationshipClass());
                        classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsRelationshipClass, v);

                        //need to initialize relclass properties here
                        //v.SetBoolean(ecClass->GetRelationshipClassCP()->GetIsExplicit());
                        //classInstance->SetValue(METASCHEMA_PROPERTY_RelClassDef_IsExplicit, v);

                        instances.push_back(classInstance);
                        }
                    }
                }
            continue;
            }
        if (METASCHEMA_CLASS_PropertyDef == qClass->GetName())
            {
            for (ECSchemaCP const& ecSchema : m_schemaScope)
                {
                for (ECClassCP const& ecClass : ecSchema->GetClasses())
                    {
                    for (ECPropertyCP const& ecProp : ecClass->GetProperties())
                        {
                        IECInstancePtr propInstance = m_metaSchema.GetClassCP(METASCHEMA_CLASS_PropertyDef)->GetDefaultStandaloneEnabler()->CreateInstance();

                        v.SetUtf8CP(ecProp->GetName().c_str());
                        propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_Name, v);
                        v.SetUtf8CP(ecProp->GetClass().GetName().c_str());
                        propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_OriginClass, v);
                        v.SetUtf8CP(ecProp->GetDisplayLabel().c_str());
                        propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_DisplayLabel, v);
                        v.SetUtf8CP(ecProp->GetTypeName().c_str());
                        propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_TypeName, v);
                        v.SetUtf8CP(ecProp->GetDescription().c_str());
                        propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_Description, v);
                        v.SetBoolean(ecProp->GetIsArray());
                        propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_IsArray, v);
                        if (ecProp->GetIsArray())
                            {
                            Utf8PrintfString min ("%" PRIu32, ecProp->GetAsArrayProperty()->GetMinOccurs());
                            v.SetUtf8CP(min.c_str());
                            propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_MinOccurs, v);
                            if (INT_MAX == ecProp->GetAsArrayProperty()->GetStoredMaxOccurs())
                                {
                                v.SetUtf8CP("unbounded");
                                }
                            else
                                {
                                Utf8PrintfString max ("%" PRIu32, ecProp->GetAsArrayProperty()->GetStoredMaxOccurs());
                                v.SetUtf8CP(max.c_str());
                                }
                            propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_MaxOccurs, v);
                            }
                        else
                            {
                            v.SetUtf8CP("1");
                            propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_MinOccurs, v);
                            v.SetUtf8CP("1");
                            propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_MaxOccurs, v);
                            }

                        instances.push_back(propInstance);
                        }
                    }
                }
            continue;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void MetaSchemaInstanceGenerator::GenerateRelatedInstances(bvector<IECInstancePtr> & relatedInstances, IECInstanceCR sourceInstance, ECRelationshipClassCR queryRelationship)
    {
    ECValue v;
    if (METASCHEMA_CLASS_SchemaDef == sourceInstance.GetClass().GetName())
        {
        if (METASCHEMA_RELCLASS_SchemaHasSchemaReference == queryRelationship.GetName())
            {
            ECSchemaCP schemaFromInstance;
            sourceInstance.GetValue(v, METASCHEMA_PROPERTY_SchemaDef_Name);
            bool foundReferencingSchema = GetSchemaFromScope(schemaFromInstance, v.GetUtf8CP());
            if (foundReferencingSchema)
                {
                ECSchemaReferenceListCR refs = schemaFromInstance->GetReferencedSchemas();
                for (auto const& member : refs)
                    {
                    ECSchemaPtr const& refSchema = member.second;
                    bool foundReferencedSchema = FindSchemaInScope(refSchema->GetName().c_str());
                    if (foundReferencedSchema)
                        {
                        IECInstancePtr ecInstance = m_metaSchema.GetClassCP(METASCHEMA_CLASS_SchemaDef)->GetDefaultStandaloneEnabler()->CreateInstance();

                        v.SetUtf8CP(refSchema->GetName().c_str());
                        ecInstance->SetValue(METASCHEMA_PROPERTY_SchemaDef_Name, v);
                        v.SetUtf8CP(refSchema->GetDisplayLabel().c_str());
                        ecInstance->SetValue(METASCHEMA_PROPERTY_SchemaDef_DisplayLabel, v);
                        v.SetUtf8CP(refSchema->GetNamespacePrefix().c_str());
                        ecInstance->SetValue(METASCHEMA_PROPERTY_SchemaDef_NameSpacePrefix, v);
                        v.SetUtf8CP(refSchema->GetDescription().c_str());
                        ecInstance->SetValue(METASCHEMA_PROPERTY_SchemaDef_Description, v);
                        v.SetInteger(refSchema->GetVersionMajor());
                        ecInstance->SetValue(METASCHEMA_PROPERTY_SchemaDef_VersionMajor, v);
                        v.SetInteger(refSchema->GetVersionMinor());
                        ecInstance->SetValue(METASCHEMA_PROPERTY_SchemaDef_VersionMinor, v);

                        relatedInstances.push_back(ecInstance);
                        }
                    else
                        {
                        //place "related instance is out of scope" logic here
                        }
                    }
                }
            return;
            }
        if (METASCHEMA_RELCLASS_SchemaHasClass == queryRelationship.GetName())
            {
            ECSchemaCP schemaFromInstance;
            sourceInstance.GetValue(v, METASCHEMA_PROPERTY_SchemaDef_Name);
            bool foundContainingSchema = GetSchemaFromScope(schemaFromInstance, v.GetUtf8CP());
            if (foundContainingSchema)
                {
                for (ECClassCP const& ecClass : schemaFromInstance->GetClasses())
                    {
                    IECInstancePtr classInstance;
                    if (ecClass->IsRelationshipClass())
                        {
                        classInstance = m_metaSchema.GetClassCP(METASCHEMA_CLASS_RelClassDef)->GetDefaultStandaloneEnabler()->CreateInstance();
                        }
                    else
                        {
                        classInstance = m_metaSchema.GetClassCP(METASCHEMA_CLASS_ClassDef)->GetDefaultStandaloneEnabler()->CreateInstance();
                        }

                    v.SetUtf8CP(ecClass->GetName().c_str());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Name, v);
                    v.SetUtf8CP(ecClass->GetDisplayLabel().c_str());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_DisplayLabel, v);
                    v.SetUtf8CP(schemaFromInstance->GetName().c_str());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Schema, v);
                    v.SetUtf8CP(ecClass->GetDescription().c_str());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Description, v);
                    v.SetBoolean(ecClass->IsStructClass());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsStruct, v);
                    v.SetBoolean(ecClass->IsCustomAttributeClass());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsCustomAttributeClass, v);
                    v.SetBoolean(ecClass->IsEntityClass());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsDomainClass, v);
                    v.SetBoolean(ecClass->HasBaseClasses());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_HasBaseClasses, v);
                    v.SetBoolean(ecClass->IsRelationshipClass());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsRelationshipClass, v);

                    //need to initialize relclass properties here
                    if (ecClass->IsRelationshipClass())
                        {

                        }

                    relatedInstances.push_back(classInstance);
                    }
                }
            return;
            }
        return;
        }
    if (METASCHEMA_CLASS_ClassDef == sourceInstance.GetClass().GetName() ||
        METASCHEMA_CLASS_RelClassDef == sourceInstance.GetClass().GetName())
        {
        if (METASCHEMA_RELCLASS_ClassHasBaseClass == queryRelationship.GetName())
            {
            ECSchemaCP schemaFromInstance;
            sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Schema);
            bool foundContainingSchema = GetSchemaFromScope(schemaFromInstance, v.GetUtf8CP());
            if (foundContainingSchema)
                {
                sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
                ECClassCP classFromInstance = schemaFromInstance->GetClassCP(v.GetUtf8CP());
                if (classFromInstance->HasBaseClasses())
                    {
                    for (ECClassCP const& ecClass : classFromInstance->GetBaseClasses())
                        {
                        bool foundRelatedContainingSchema = FindSchemaInScope(ecClass->GetSchema().GetName().c_str());
                        if (foundRelatedContainingSchema)
                            {
                            IECInstancePtr classInstance;
                            if (ecClass->IsRelationshipClass())
                                {
                                classInstance = m_metaSchema.GetClassCP(METASCHEMA_CLASS_RelClassDef)->GetDefaultStandaloneEnabler()->CreateInstance();
                                }
                            else
                                {
                                classInstance = m_metaSchema.GetClassCP(METASCHEMA_CLASS_ClassDef)->GetDefaultStandaloneEnabler()->CreateInstance();
                                }

                            v.SetUtf8CP(ecClass->GetName().c_str());
                            classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Name, v);
                            v.SetUtf8CP(ecClass->GetDisplayLabel().c_str());
                            classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_DisplayLabel, v);
                            v.SetUtf8CP(schemaFromInstance->GetName().c_str());
                            classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Schema, v);
                            v.SetUtf8CP(ecClass->GetDescription().c_str());
                            classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Description, v);
                            v.SetBoolean(ecClass->IsStructClass());
                            classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsStruct, v);
                            v.SetBoolean(ecClass->IsCustomAttributeClass());
                            classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsCustomAttributeClass, v);
                            v.SetBoolean(ecClass->IsEntityClass());
                            classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsDomainClass, v);
                            v.SetBoolean(ecClass->HasBaseClasses());
                            classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_HasBaseClasses, v);
                            v.SetBoolean(ecClass->IsRelationshipClass());
                            classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsRelationshipClass, v);

                            //need to initialize relclass properties here
                            if (ecClass->IsRelationshipClass())
                                {

                                }

                            relatedInstances.push_back(classInstance);
                            }
                        }
                    }
                }
            return;
            }
        if (METASCHEMA_RELCLASS_ClassHasProperty == queryRelationship.GetName())
            {
            ECSchemaCP schemaFromInstance;
            sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Schema);
            bool foundContainingSchema = GetSchemaFromScope(schemaFromInstance, v.GetUtf8CP());
            if (foundContainingSchema)
                {
                sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
                ECClassCP classFromInstance = schemaFromInstance->GetClassCP(v.GetUtf8CP());
                for (ECPropertyCP const& ecProp : classFromInstance->GetProperties())
                    {
                    IECInstancePtr propInstance = m_metaSchema.GetClassCP(METASCHEMA_CLASS_PropertyDef)->GetDefaultStandaloneEnabler()->CreateInstance();

                    v.SetUtf8CP(ecProp->GetName().c_str());
                    propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_Name, v);
                    v.SetUtf8CP(ecProp->GetClass().GetName().c_str());
                    propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_OriginClass, v);
                    v.SetUtf8CP(ecProp->GetDisplayLabel().c_str());
                    propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_DisplayLabel, v);
                    v.SetUtf8CP(ecProp->GetTypeName().c_str());
                    propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_TypeName, v);
                    v.SetUtf8CP(ecProp->GetDescription().c_str());
                    propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_Description, v);
                    v.SetBoolean(ecProp->GetIsArray());
                    propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_IsArray, v);
                    if (ecProp->GetIsArray())
                        {
                        Utf8PrintfString min ("%" PRIu32, ecProp->GetAsArrayProperty()->GetMinOccurs());
                        v.SetUtf8CP(min.c_str());
                        propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_MinOccurs, v);
                        if (INT_MAX == ecProp->GetAsArrayProperty()->GetStoredMaxOccurs())
                            {
                            v.SetUtf8CP("unbounded");
                            }
                        else
                            {
                            Utf8PrintfString max ("%" PRIu32, ecProp->GetAsArrayProperty()->GetStoredMaxOccurs());
                            v.SetUtf8CP(max.c_str());
                            }
                        propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_MaxOccurs, v);
                        }
                    else
                        {
                        v.SetUtf8CP("1");
                        propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_MinOccurs, v);
                        v.SetUtf8CP("1");
                        propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_MaxOccurs, v);
                        }

                    relatedInstances.push_back(propInstance);
                    }
                }
            return;
            }
        if (METASCHEMA_RELCLASS_ClassHasLocalProperty == queryRelationship.GetName())
            {
            ECSchemaCP schemaFromInstance;
            sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Schema);
            bool foundContainingSchema = GetSchemaFromScope(schemaFromInstance, v.GetUtf8CP());
            if (foundContainingSchema)
                {
                sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
                ECClassCP classFromInstance = schemaFromInstance->GetClassCP(v.GetUtf8CP());
                for (ECPropertyCP const& ecProp : classFromInstance->GetProperties(false))
                    {
                    IECInstancePtr propInstance = m_metaSchema.GetClassCP(METASCHEMA_CLASS_PropertyDef)->GetDefaultStandaloneEnabler()->CreateInstance();

                    v.SetUtf8CP(ecProp->GetName().c_str());
                    propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_Name, v);
                    v.SetUtf8CP(ecProp->GetClass().GetName().c_str());
                    propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_OriginClass, v);
                    v.SetUtf8CP(ecProp->GetDisplayLabel().c_str());
                    propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_DisplayLabel, v);
                    v.SetUtf8CP(ecProp->GetTypeName().c_str());
                    propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_TypeName, v);
                    v.SetUtf8CP(ecProp->GetDescription().c_str());
                    propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_Description, v);
                    v.SetBoolean(ecProp->GetIsArray());
                    propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_IsArray, v);
                    if (ecProp->GetIsArray())
                        {
                        Utf8PrintfString min ("%" PRIu32, ecProp->GetAsArrayProperty()->GetMinOccurs());
                        v.SetUtf8CP(min.c_str());
                        propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_MinOccurs, v);
                        if (INT_MAX == ecProp->GetAsArrayProperty()->GetStoredMaxOccurs())
                            {
                            v.SetUtf8CP("unbounded");
                            }
                        else
                            {
                            Utf8PrintfString max ("%" PRIu32, ecProp->GetAsArrayProperty()->GetStoredMaxOccurs());
                            v.SetUtf8CP(max.c_str());
                            }
                        propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_MaxOccurs, v);
                        }
                    else
                        {
                        v.SetUtf8CP("1");
                        propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_MinOccurs, v);
                        v.SetUtf8CP("1");
                        propInstance->SetValue(METASCHEMA_PROPERTY_PropertyDef_MaxOccurs, v);
                        }

                    relatedInstances.push_back(propInstance);
                    }
                }
            return;
            }
        if (METASCHEMA_CLASS_RelClassDef == sourceInstance.GetClass().GetName() &&
            METASCHEMA_RELCLASS_RelationshipHasSourceConstraintClass == queryRelationship.GetName())
            {
            ECSchemaCP schemaFromInstance;
            sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Schema);
            bool foundContainingSchema = GetSchemaFromScope(schemaFromInstance, v.GetUtf8CP());
            if (foundContainingSchema)
                {
                sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
                ECRelationshipClassCP relClassFromInstance = schemaFromInstance->GetClassCP(v.GetUtf8CP())->GetRelationshipClassCP();
                for (ECClassCP const& ecClass : relClassFromInstance->GetSource().GetClasses())
                    {
                    IECInstancePtr classInstance;
                    if (ecClass->IsRelationshipClass())
                        {
                        classInstance = m_metaSchema.GetClassCP(METASCHEMA_CLASS_RelClassDef)->GetDefaultStandaloneEnabler()->CreateInstance();
                        }
                    else
                        {
                        classInstance = m_metaSchema.GetClassCP(METASCHEMA_CLASS_ClassDef)->GetDefaultStandaloneEnabler()->CreateInstance();
                        }

                    v.SetUtf8CP(ecClass->GetName().c_str());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Name, v);
                    v.SetUtf8CP(ecClass->GetDisplayLabel().c_str());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_DisplayLabel, v);
                    v.SetUtf8CP(schemaFromInstance->GetName().c_str());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Schema, v);
                    v.SetUtf8CP(ecClass->GetDescription().c_str());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Description, v);
                    v.SetBoolean(ecClass->IsStructClass());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsStruct, v);
                    v.SetBoolean(ecClass->IsCustomAttributeClass());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsCustomAttributeClass, v);
                    v.SetBoolean(ecClass->IsEntityClass());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsDomainClass, v);
                    v.SetBoolean(ecClass->HasBaseClasses());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_HasBaseClasses, v);
                    v.SetBoolean(ecClass->IsRelationshipClass());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsRelationshipClass, v);

                    //need to initialize relclass properties here
                    if (ecClass->IsRelationshipClass())
                        {

                        }

                    relatedInstances.push_back(classInstance);
                    }
                }
            return;
            }
        if (METASCHEMA_CLASS_RelClassDef == sourceInstance.GetClass().GetName() &&
            METASCHEMA_RELCLASS_RelationshipHasTargetConstraintClass == queryRelationship.GetName())
            {
            ECSchemaCP schemaFromInstance;
            sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Schema);
            bool foundContainingSchema = GetSchemaFromScope(schemaFromInstance, v.GetUtf8CP());
            if (foundContainingSchema)
                {
                sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
                ECRelationshipClassCP relClassFromInstance = schemaFromInstance->GetClassCP(v.GetUtf8CP())->GetRelationshipClassCP();
                for (ECClassCP const& ecClass : relClassFromInstance->GetTarget().GetClasses())
                    {
                    IECInstancePtr classInstance;
                    if (ecClass->IsRelationshipClass())
                        {
                        classInstance = m_metaSchema.GetClassCP(METASCHEMA_CLASS_RelClassDef)->GetDefaultStandaloneEnabler()->CreateInstance();
                        }
                    else
                        {
                        classInstance = m_metaSchema.GetClassCP(METASCHEMA_CLASS_ClassDef)->GetDefaultStandaloneEnabler()->CreateInstance();
                        }

                    v.SetUtf8CP(ecClass->GetName().c_str());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Name, v);
                    v.SetUtf8CP(ecClass->GetDisplayLabel().c_str());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_DisplayLabel, v);
                    v.SetUtf8CP(schemaFromInstance->GetName().c_str());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Schema, v);
                    v.SetUtf8CP(ecClass->GetDescription().c_str());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_Description, v);
                    v.SetBoolean(ecClass->IsStructClass());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsStruct, v);
                    v.SetBoolean(ecClass->IsCustomAttributeClass());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsCustomAttributeClass, v);
                    v.SetBoolean(ecClass->IsEntityClass());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsDomainClass, v);
                    v.SetBoolean(ecClass->HasBaseClasses());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_HasBaseClasses, v);
                    v.SetBoolean(ecClass->IsRelationshipClass());
                    classInstance->SetValue(METASCHEMA_PROPERTY_ClassDef_IsRelationshipClass, v);

                    //need to initialize relclass properties here
                    if (ecClass->IsRelationshipClass())
                        {

                        }

                    relatedInstances.push_back(classInstance);
                    }
                }
            return;
            }
        return;
        }
    }

//=======================================================================================
// @bsistruct                                                   Mike.Embick     09/15
//=======================================================================================
struct SchemaDiagramStringBuilder
    {
    ECSchemaCR m_metaSchema;
    SchemaDiagramStringBuilder(ECSchemaCR metaSchema) : m_metaSchema(metaSchema) {};

    Utf8String InstanceToNodeString(IECInstanceCR instance);
    Utf8String InstancePairToEdgeString(IECInstanceCR sourceInstance, IECInstanceCR targetInstance);
    void BuildDiagram(bvector<Utf8String> & strings, bvector<ECSchemaCP> & scope);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SchemaDiagramStringBuilder::InstanceToNodeString(IECInstanceCR instance)
    {
    ECValue v;
    instance.GetValue(v, METASCHEMA_PROPERTY_SchemaDef_Name);
    return v.GetUtf8CP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SchemaDiagramStringBuilder::InstancePairToEdgeString(IECInstanceCR sourceInstance, IECInstanceCR targetInstance)
    {
    ECValue v;
    sourceInstance.GetValue(v, METASCHEMA_PROPERTY_SchemaDef_Name);
    Utf8String edgeString = v.GetUtf8CP();
    edgeString += " -> ";
    targetInstance.GetValue(v, METASCHEMA_PROPERTY_SchemaDef_Name);
    edgeString += v.GetUtf8CP();
    return edgeString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaDiagramStringBuilder::BuildDiagram(bvector<Utf8String> & strings, bvector<ECSchemaCP> & scope)
    {
    MetaSchemaInstanceGenerator gen(m_metaSchema, scope);

    bvector<ECClassCP> querySchemas;
    querySchemas.push_back(m_metaSchema.GetClassCP(METASCHEMA_CLASS_SchemaDef));
    ECRelationshipClassCP queryRelationship = m_metaSchema.GetClassCP(METASCHEMA_RELCLASS_SchemaHasSchemaReference)->GetRelationshipClassCP();

    strings.push_back("digraph SchemaDiagram {");

    bvector<IECInstancePtr> schemaInstanceVector;
    gen.GenerateInstances(schemaInstanceVector, querySchemas);
    for (IECInstancePtr const& schemaInstance : schemaInstanceVector)
        {
        strings.push_back(InstanceToNodeString(*schemaInstance));
        }
    for (IECInstancePtr const& schemaInstance : schemaInstanceVector)
        {
        bvector<IECInstancePtr> refSchemaInstanceVector;
        gen.GenerateRelatedInstances(refSchemaInstanceVector, *schemaInstance, *queryRelationship);
        for (IECInstancePtr const& refSchemaInstance : refSchemaInstanceVector)
            {
            strings.push_back(InstancePairToEdgeString(*schemaInstance, *refSchemaInstance));
            }
        }

    strings.push_back("}");
    }

//=======================================================================================
// @bsistruct                                                   Mike.Embick     09/15
//=======================================================================================
struct ClassDiagramStringBuilder
    {
    ECSchemaCR m_metaSchema;
    ClassDiagramStringBuilder(ECSchemaCR metaSchema) : m_metaSchema(metaSchema) {};

    Utf8String SchemaClusterString(IECInstanceCR instance);
    Utf8String ClassNodeString(IECInstanceCR instance);
    Utf8String BaseClassEdgeString(IECInstanceCR sourceInstance, IECInstanceCR targetInstance);
    Utf8String ClassPropsString(IECInstanceCR sourceInstance, bvector<IECInstancePtr> & targetInstanceVector);
    Utf8String RelSourceEdgeString(IECInstanceCR sourceInstance, IECInstanceCR targetInstance);
    Utf8String RelTargetEdgeString(IECInstanceCR sourceInstance, IECInstanceCR targetInstance);

    void BuildDiagram(bvector<Utf8String> & strings, bvector<ECSchemaCP> & scope);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClassDiagramStringBuilder::SchemaClusterString(IECInstanceCR instance)
    {
    ECValue v;
    Utf8String schemaString = "subgraph cluster_";
    instance.GetValue(v, METASCHEMA_PROPERTY_SchemaDef_Name);
    schemaString += v.GetUtf8CP();
    schemaString += " { label=";
    schemaString += v.GetUtf8CP();
    return schemaString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClassDiagramStringBuilder::ClassNodeString(IECInstanceCR instance)
    {
    ECValue v;
    Utf8String classString = "\"";
    instance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Schema);
    classString += v.GetUtf8CP();
    classString += "::";
    instance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
    classString += v.GetUtf8CP();
    classString += "\" [label=";
    classString += v.GetUtf8CP();
    if (METASCHEMA_CLASS_RelClassDef == instance.GetClass().GetName())
        {
        classString += " fillcolor=beige";
        }
    classString += "]";
    return classString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClassDiagramStringBuilder::BaseClassEdgeString(IECInstanceCR sourceInstance, IECInstanceCR targetInstance)
    {
    ECValue v;
    Utf8String baseClassString = "\"";
    targetInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Schema);
    baseClassString += v.GetUtf8CP();
    baseClassString += "::";
    targetInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
    baseClassString += v.GetUtf8CP();
    baseClassString += "\" -> \"";
    sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Schema);
    baseClassString += v.GetUtf8CP();
    baseClassString += "::";
    sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
    baseClassString += v.GetUtf8CP();
    baseClassString += "\" [label=\"\" dir=back arrowtail=onormal]";
    return baseClassString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClassDiagramStringBuilder::ClassPropsString(IECInstanceCR sourceInstance, bvector<IECInstancePtr> & targetInstanceVector)
    {
    ECValue v;
    Utf8String propsString = "\"";
    sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Schema);
    propsString += v.GetUtf8CP();
    propsString += "::";
    sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
    propsString += v.GetUtf8CP();
    propsString += "\" [shape=record label=\"{";
    propsString += v.GetUtf8CP();
    propsString += "|";
    for (IECInstancePtr const& targetInstance : targetInstanceVector)
        {
        targetInstance->GetValue(v, METASCHEMA_PROPERTY_PropertyDef_Name);
        propsString += v.GetUtf8CP();
        propsString += "\\n";
        }
    propsString += "}\"]";
    return propsString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClassDiagramStringBuilder::RelSourceEdgeString(IECInstanceCR sourceInstance, IECInstanceCR targetInstance)
    {
    ECValue v;
    Utf8String relSourceString = "\"";
    targetInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Schema);
    relSourceString += v.GetUtf8CP();
    relSourceString += "::";
    targetInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
    relSourceString += v.GetUtf8CP();
    relSourceString += "\" -> \"";
    sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Schema);
    relSourceString += v.GetUtf8CP();
    relSourceString += "::";
    sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
    relSourceString += v.GetUtf8CP();
    relSourceString += "\" [label=\"\" dir=back color=red]";
    return relSourceString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ClassDiagramStringBuilder::RelTargetEdgeString(IECInstanceCR sourceInstance, IECInstanceCR targetInstance)
    {
    ECValue v;
    Utf8String relTargetString = "\"";
    targetInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Schema);
    relTargetString += v.GetUtf8CP();
    relTargetString += "::";
    targetInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
    relTargetString += v.GetUtf8CP();
    relTargetString += "\" -> \"";
    sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Schema);
    relTargetString += v.GetUtf8CP();
    relTargetString += "::";
    sourceInstance.GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
    relTargetString += v.GetUtf8CP();
    relTargetString += "\" [label=\"\" dir=back color=blue]";
    return relTargetString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassDiagramStringBuilder::BuildDiagram(bvector<Utf8String> & strings, bvector<ECSchemaCP> & scope)
    {
    MetaSchemaInstanceGenerator gen(m_metaSchema, scope);

    bvector<ECClassCP> querySchemas;
    querySchemas.push_back(m_metaSchema.GetClassCP(METASCHEMA_CLASS_SchemaDef));
    ECRelationshipClassCP querySchemaHasClass = m_metaSchema.GetClassCP(METASCHEMA_RELCLASS_SchemaHasClass)->GetRelationshipClassCP();
    ECRelationshipClassCP queryClassHasBaseClass = m_metaSchema.GetClassCP(METASCHEMA_RELCLASS_ClassHasBaseClass)->GetRelationshipClassCP();
    ECRelationshipClassCP queryClassHasLocalProperty = m_metaSchema.GetClassCP(METASCHEMA_RELCLASS_ClassHasLocalProperty)->GetRelationshipClassCP();
    ECRelationshipClassCP queryRelationshipHasSource = m_metaSchema.GetClassCP(METASCHEMA_RELCLASS_RelationshipHasSourceConstraintClass)->GetRelationshipClassCP();
    ECRelationshipClassCP queryRelationshipHasTarget = m_metaSchema.GetClassCP(METASCHEMA_RELCLASS_RelationshipHasTargetConstraintClass)->GetRelationshipClassCP();

    strings.push_back("digraph ClassDiagram { splines=ortho");
    strings.push_back("node [shape=record style=filled fillcolor=gray95]");

    bvector<IECInstancePtr> schemaInstanceVector;
    gen.GenerateInstances(schemaInstanceVector, querySchemas);
    for (IECInstancePtr const& schemaInstance : schemaInstanceVector)
        {
        strings.push_back(SchemaClusterString(*schemaInstance));

        bvector<IECInstancePtr> classInstanceVector;
        gen.GenerateRelatedInstances(classInstanceVector, *schemaInstance, *querySchemaHasClass);
        for (IECInstancePtr const& classInstance : classInstanceVector)
            {
            strings.push_back(ClassNodeString(*classInstance));

            bvector<IECInstancePtr> baseClassInstanceVector;
            gen.GenerateRelatedInstances(baseClassInstanceVector, *classInstance, *queryClassHasBaseClass);
            for (IECInstancePtr const& baseClassInstance : baseClassInstanceVector)
                {
                strings.push_back(BaseClassEdgeString(*classInstance, *baseClassInstance));
                }

            bvector<IECInstancePtr> propInstanceVector;
            gen.GenerateRelatedInstances(propInstanceVector, *classInstance, *queryClassHasLocalProperty);
            strings.push_back(ClassPropsString(*classInstance, propInstanceVector));

            bvector<IECInstancePtr> relSourceInstanceVector;
            gen.GenerateRelatedInstances(relSourceInstanceVector, *classInstance, *queryRelationshipHasSource);
            for (IECInstancePtr const& relSourceInstance : relSourceInstanceVector)
                {
                strings.push_back(RelSourceEdgeString(*classInstance, *relSourceInstance));
                }

            bvector<IECInstancePtr> relTargetInstanceVector;
            gen.GenerateRelatedInstances(relTargetInstanceVector, *classInstance, *queryRelationshipHasTarget);
            for (IECInstancePtr const& relTargetInstance : relTargetInstanceVector)
                {
                strings.push_back(RelTargetEdgeString(*classInstance, *relTargetInstance));
                }
            }
        strings.push_back("}");
        }

    strings.push_back("}");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaVisualizationTests, TestSchemaDiagram)
    {
    BeFileName dbPath;
    BeTest::GetHost().GetOutputRoot(dbPath);
    dbPath.AppendToPath(L"SchemaVisualizationTest.ibim");

    CreateDgnDbParams dbCreateParams;
    dbCreateParams.SetOverwriteExisting(true);
    dbCreateParams.SetProjectName("SchemaVisualizationTest-TestSchemaDiagramFromDgnDb");
    dbCreateParams.SetProjectDescription("Created by unit test SchemaVisualizationTest.TestSchemaDiagramFromDgnDb");
    dbCreateParams.SetStartDefaultTxn(DefaultTxn::Exclusive);

    DbResult createStatus;
    DgnDbPtr db = DgnDb::CreateDgnDb(&createStatus, dbPath, dbCreateParams);
    ASSERT_TRUE(BE_SQLITE_OK == createStatus);
    ASSERT_TRUE(db.IsValid());

    ECDbSchemaManager const& manager = db->Schemas();
    bvector<ECSchemaCP> scope;
    scope.push_back(manager.GetECSchema(BIS_ECSCHEMA_NAME));
    scope.push_back(manager.GetECSchema("ECDbMap"));
    scope.push_back(manager.GetECSchema("ECDb_FileInfo"));
    scope.push_back(manager.GetECSchema("ECDb_System"));

    SchemaDiagramStringBuilder sdb(GetMetaSchema());
    bvector<Utf8String> instanceStrings;
    sdb.BuildDiagram(instanceStrings, scope);
    
    BeFileName fileName;
    BeTest::GetHost().GetOutputRoot(fileName);
    fileName.AppendToPath(L"SchemaVisualizationTests_SchemaDiagram.gv");
    if (BeFileName::DoesPathExist(fileName)) { BeFileName::BeDeleteFile(fileName); }

    BeFile newfile;
    newfile.Create(fileName);
    for (Utf8String const& curString : instanceStrings)
        {
        if (!(curString == ""))
            {
            newfile.Write(nullptr, curString.c_str(), uint32_t(curString.length()));
            newfile.Write(nullptr, "\n", 1);
            }
        }
    newfile.Close();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     09/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaVisualizationTests, TestClassDiagram)
    {
    BeFileName dbPath;
    BeTest::GetHost().GetOutputRoot(dbPath);
    dbPath.AppendToPath(L"SchemaVisualizationTest.ibim");

    CreateDgnDbParams dbCreateParams;
    dbCreateParams.SetOverwriteExisting(true);
    dbCreateParams.SetProjectName("SchemaVisualizationTest-TestClassDiagramFromDgnDb");
    dbCreateParams.SetProjectDescription("Created by unit test SchemaVisualizationTest.TestClassDiagramFromDgnDb");
    dbCreateParams.SetStartDefaultTxn(DefaultTxn::Exclusive);

    DbResult createStatus;
    DgnDbPtr db = DgnDb::CreateDgnDb(&createStatus, dbPath, dbCreateParams);
    ASSERT_TRUE(BE_SQLITE_OK == createStatus);
    ASSERT_TRUE(db.IsValid());

    ECDbSchemaManager const& manager = db->Schemas();
    bvector<ECSchemaCP> scope;
    scope.push_back(manager.GetECSchema(BIS_ECSCHEMA_NAME));
    scope.push_back(manager.GetECSchema("ECDbMap"));
    scope.push_back(manager.GetECSchema("ECDb_FileInfo"));
    scope.push_back(manager.GetECSchema("ECDb_System"));

    ClassDiagramStringBuilder cdb(GetMetaSchema());
    bvector<Utf8String> instanceStrings;
    cdb.BuildDiagram(instanceStrings, scope);

    BeFileName fileName;
    BeTest::GetHost().GetOutputRoot(fileName);
    fileName.AppendToPath(L"SchemaVisualizationTests_ClassDiagram.gv");
    if (BeFileName::DoesPathExist(fileName)) { BeFileName::BeDeleteFile(fileName); }

    BeFile newfile;
    newfile.Create(fileName);
    for (Utf8String const& curString : instanceStrings)
        {
        if (!(curString == ""))
            {
            newfile.Write(nullptr, curString.c_str(), uint32_t(curString.length()));
            newfile.Write(nullptr, "\n", 1);
            }
        }
    newfile.Close();
    }

//=======================================================================================
//                                                              Mike.Embick     10/15
//=======================================================================================
extern "C" //[SVT_EXTERN]
    {
    IMPORT_ATTRIBUTE gvplugin_library_t gvplugin_dot_layout_LTX_library;
    IMPORT_ATTRIBUTE gvplugin_library_t gvplugin_core_LTX_library;
    }

//=======================================================================================
// @bsistruct                                                   Mike.Embick     10/15
//=======================================================================================
struct GraphvizDiagram
    {
private:
    Agraph_t* m_graph;
public:
    GraphvizDiagram()
        {
        m_graph = agopen("g", Agdirected, NULL);
        }
    ~GraphvizDiagram()
        {
        agclose(m_graph);
        }
    Agraph_t* GetGraph() const { return m_graph; }
    };

//=======================================================================================
// @bsistruct                                                   Mike.Embick     10/15
//=======================================================================================
struct SchemaDiagramBuilder
    {
    ECSchemaCR m_metaSchema;
    SchemaDiagramBuilder(ECSchemaCR metaSchema) : m_metaSchema(metaSchema) {};
    void BuildDiagram(GraphvizDiagram& diagram, bvector<ECSchemaCP>& scope);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SchemaDiagramBuilder::BuildDiagram(GraphvizDiagram& diagram, bvector<ECSchemaCP>& scope)
    {
    ECValue v;
    Agraph_t* g = diagram.GetGraph();
    MetaSchemaInstanceGenerator gen(m_metaSchema, scope);

    bvector<ECClassCP> querySchemas;
    querySchemas.push_back(m_metaSchema.GetClassCP(METASCHEMA_CLASS_SchemaDef));
    ECRelationshipClassCP querySchemaHasReference = m_metaSchema.GetClassCP(METASCHEMA_RELCLASS_SchemaHasSchemaReference)->GetRelationshipClassCP();

    agattr(g, AGRAPH, "layout", "dot");
    agattr(g, AGRAPH, "splines", "ortho");
    agattr(g, AGRAPH, "label", "");
    agattr(g, AGNODE, "shape", "box");
    agattr(g, AGNODE, "label", "");
    agattr(g, AGEDGE, "style", "solid");

    bvector<IECInstancePtr> schemaInstanceVector;
    gen.GenerateInstances(schemaInstanceVector, querySchemas);
    for (IECInstancePtr const& schemaInstance : schemaInstanceVector)
        {
        schemaInstance->GetValue(v, METASCHEMA_PROPERTY_SchemaDef_Name);
        Utf8String schemaString = v.GetUtf8CP();
        Agnode_t* n = agnode(g, const_cast<char*>(schemaString.c_str()), TRUE);
        agset(n, "label", const_cast<char*>((schemaString + "\\l").c_str()));
        }

    for (IECInstancePtr const& schemaInstance : schemaInstanceVector)
        {
        schemaInstance->GetValue(v, METASCHEMA_PROPERTY_SchemaDef_Name);
        Utf8String schemaString = v.GetUtf8CP();
        Agnode_t* n = agnode(g, const_cast<char*>(schemaString.c_str()), FALSE);

        bvector<IECInstancePtr> refSchemaInstanceVector;
        gen.GenerateRelatedInstances(refSchemaInstanceVector, *schemaInstance, *querySchemaHasReference);
        for (IECInstancePtr const& refSchemaInstance : refSchemaInstanceVector)
            {
            refSchemaInstance->GetValue(v, METASCHEMA_PROPERTY_SchemaDef_Name);
            Utf8String refSchemaString = v.GetUtf8CP();
            Agnode_t* m = agnode(g, const_cast<char*>(refSchemaString.c_str()), FALSE);

            Utf8String edgeString = refSchemaString;
            edgeString += "->";
            edgeString += schemaString;

            Agedge_t* e = agedge(g, m, n, const_cast<char*>(edgeString.c_str()), TRUE);
            agsafeset(e, "dir", "back", "");
            agsafeset(e, "arrowtail", "onormal", "");
            }
        }
    }

//=======================================================================================
// @bsistruct                                                   Mike.Embick     10/15
//=======================================================================================
struct ClassDiagramBuilder
    {
    ECSchemaCR m_metaSchema;
    ClassDiagramBuilder(ECSchemaCR metaSchema) : m_metaSchema(metaSchema) {};
    void BuildDiagram(GraphvizDiagram& diagram, bvector<ECSchemaCP>& scope);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ClassDiagramBuilder::BuildDiagram(GraphvizDiagram& diagram, bvector<ECSchemaCP>& scope)
    {
    ECValue v;
    Agraph_t* g = diagram.GetGraph();
    MetaSchemaInstanceGenerator gen(m_metaSchema, scope);

    bvector<ECClassCP> querySchemas;
    querySchemas.push_back(m_metaSchema.GetClassCP(METASCHEMA_CLASS_SchemaDef));
    ECRelationshipClassCP querySchemaHasClass = m_metaSchema.GetClassCP(METASCHEMA_RELCLASS_SchemaHasClass)->GetRelationshipClassCP();
    ECRelationshipClassCP queryClassHasBaseClass = m_metaSchema.GetClassCP(METASCHEMA_RELCLASS_ClassHasBaseClass)->GetRelationshipClassCP();
    ECRelationshipClassCP queryClassHasLocalProperty = m_metaSchema.GetClassCP(METASCHEMA_RELCLASS_ClassHasLocalProperty)->GetRelationshipClassCP();
    ECRelationshipClassCP queryRelationshipHasSource = m_metaSchema.GetClassCP(METASCHEMA_RELCLASS_RelationshipHasSourceConstraintClass)->GetRelationshipClassCP();
    ECRelationshipClassCP queryRelationshipHasTarget = m_metaSchema.GetClassCP(METASCHEMA_RELCLASS_RelationshipHasTargetConstraintClass)->GetRelationshipClassCP();

    agattr(g, AGRAPH, "layout", "dot");
    agattr(g, AGRAPH, "splines", "true");
    agattr(g, AGRAPH, "label", "");
    agattr(g, AGRAPH, "rankdir", "LR");
    //agattr(g, AGRAPH, "labeljust", "l"); //this only left-justifies cluster graph labels
    //agattr(g, AGRAPH, "concentrate", "true");
    agattr(g, AGNODE, "shape", "record");
    agattr(g, AGNODE, "label", "");
    agattr(g, AGEDGE, "style", "solid");
    //agattr(g, AGEDGE, "decorate", "true");
    agattr(g, AGEDGE, "constraint", "true");

    bvector<IECInstancePtr> schemaInstanceVector;
    gen.GenerateInstances(schemaInstanceVector, querySchemas);
    for (IECInstancePtr const& schemaInstance : schemaInstanceVector)
        {
        schemaInstance->GetValue(v, METASCHEMA_PROPERTY_SchemaDef_Name);
        Utf8String schemaName = v.GetUtf8CP();
        Utf8String schemaString = "cluster_";
        schemaString += schemaName;
        Agraph_t* h = agsubg(g, const_cast<char*>(schemaString.c_str()), TRUE);
        agset(h, "label", const_cast<char*>((schemaName + "\\l").c_str()));

        bvector<IECInstancePtr> classInstanceVector;
        gen.GenerateRelatedInstances(classInstanceVector, *schemaInstance, *querySchemaHasClass);
        for (IECInstancePtr const& classInstance : classInstanceVector)
            {
            if (METASCHEMA_CLASS_ClassDef == classInstance->GetClass().GetName())
                {
                classInstance->GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
                Utf8String className = v.GetUtf8CP();
                Utf8String classString = schemaName;
                classString += "::";
                classString += className;
                Agnode_t* n = agnode(h, const_cast<char*>(classString.c_str()), TRUE);
                agset(n, "label", const_cast<char*>((className + "\\l").c_str()));

                bvector<IECInstancePtr> propInstanceVector;
                gen.GenerateRelatedInstances(propInstanceVector, *classInstance, *queryClassHasLocalProperty);
                Utf8String recordString = "";
                recordString += (className + "\\l");
                recordString += "|";
                for (IECInstancePtr const& propInstance : propInstanceVector)
                    {
                    propInstance->GetValue(v, METASCHEMA_PROPERTY_PropertyDef_Name);
                    recordString += v.GetUtf8CP();
                    recordString += "\\l";
                    }
                recordString += "";
                agset(n, "label", const_cast<char*>(recordString.c_str()));
                }
            else if (METASCHEMA_CLASS_RelClassDef == classInstance->GetClass().GetName())
                {
                //is there any init here for relclasses?
                }
            }
        }

    for (IECInstancePtr const& schemaInstance : schemaInstanceVector)
        {
        schemaInstance->GetValue(v, METASCHEMA_PROPERTY_SchemaDef_Name);
        Utf8String schemaName = v.GetUtf8CP();
        Utf8String schemaString = "cluster_";
        schemaString += schemaName;
        Agraph_t* h = agsubg(g, const_cast<char*>(schemaString.c_str()), FALSE);

        bvector<IECInstancePtr> classInstanceVector;
        gen.GenerateRelatedInstances(classInstanceVector, *schemaInstance, *querySchemaHasClass);
        for (IECInstancePtr const& classInstance : classInstanceVector)
            {
            if (METASCHEMA_CLASS_ClassDef == classInstance->GetClass().GetName())
                {
                classInstance->GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
                Utf8String className = v.GetUtf8CP();
                Utf8String classString = schemaName;
                classString += "::";
                classString += className;
                Agnode_t* n = agnode(h, const_cast<char*>(classString.c_str()), FALSE);

                bvector<IECInstancePtr> baseClassInstanceVector;
                gen.GenerateRelatedInstances(baseClassInstanceVector, *classInstance, *queryClassHasBaseClass);
                for (IECInstancePtr const& baseClassInstance : baseClassInstanceVector)
                    {
                    baseClassInstance->GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
                    Utf8String baseClassName = v.GetUtf8CP();
                    Utf8String baseClassString = schemaName;
                    baseClassString += "::";
                    baseClassString += baseClassName;
                    Agnode_t* m = agnode(h, const_cast<char*>(baseClassString.c_str()), FALSE);

                    Utf8String edgeString = baseClassString;
                    edgeString += "->";
                    edgeString += classString;

                    Agedge_t* e = agedge(h, m, n, const_cast<char*>(edgeString.c_str()), TRUE);
                    agsafeset(e, "dir", "back", "");
                    agsafeset(e, "arrowtail", "onormal", "");
                    }
                }
            else if (METASCHEMA_CLASS_RelClassDef == classInstance->GetClass().GetName())
                {
                bvector<IECInstancePtr> relSourceInstanceVector;
                gen.GenerateRelatedInstances(relSourceInstanceVector, *classInstance, *queryRelationshipHasSource);
                bvector<IECInstancePtr> relTargetInstanceVector;
                gen.GenerateRelatedInstances(relTargetInstanceVector, *classInstance, *queryRelationshipHasTarget);
                for (IECInstancePtr const& relSourceInstance : relSourceInstanceVector)
                    {
                    for (IECInstancePtr const& relTargetInstance : relTargetInstanceVector)
                        {
                        Utf8String sourceClassString = schemaName;
                        sourceClassString += "::";
                        relSourceInstance->GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
                        sourceClassString += v.GetUtf8CP();

                        Utf8String targetClassString = schemaName;
                        targetClassString += "::";
                        relTargetInstance->GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
                        targetClassString += v.GetUtf8CP();

                        Agnode_t* s = agnode(h, const_cast<char*>(sourceClassString.c_str()), FALSE);
                        Agnode_t* t = agnode(h, const_cast<char*>(targetClassString.c_str()), FALSE);
                        Agedge_t* e = agedge(h, s, t, const_cast<char*>((sourceClassString + "->" + targetClassString).c_str()), TRUE);
                        agsafeset(e, "arrowhead", "none", "normal");

                        classInstance->GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
                        Utf8String className = v.GetUtf8CP();
                        agsafeset(e, "label", const_cast<char*>((className + "\\l").c_str()), "");
                        //agset(e, "constraint", "false"); //always seems to produce an error
                        }
                    }
                }
            }
        }
    }

//=======================================================================================
// @bsistruct                                                   Mike.Embick     11/15
//=======================================================================================
struct RelationshipClassDiagramBuilder
    {
    ECSchemaCR m_metaSchema;
    RelationshipClassDiagramBuilder(ECSchemaCR metaSchema) : m_metaSchema(metaSchema) {};
    void BuildDiagram(GraphvizDiagram& diagram, bvector<ECSchemaCP>& scope);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void RelationshipClassDiagramBuilder::BuildDiagram(GraphvizDiagram& diagram, bvector<ECSchemaCP>& scope)
    {
    ECValue v;
    Agraph_t* g = diagram.GetGraph();
    MetaSchemaInstanceGenerator gen(m_metaSchema, scope);

    bvector<ECClassCP> querySchemas;
    querySchemas.push_back(m_metaSchema.GetClassCP(METASCHEMA_CLASS_SchemaDef));
    ECRelationshipClassCP querySchemaHasClass = m_metaSchema.GetClassCP(METASCHEMA_RELCLASS_SchemaHasClass)->GetRelationshipClassCP();
    ECRelationshipClassCP queryClassHasBaseClass = m_metaSchema.GetClassCP(METASCHEMA_RELCLASS_ClassHasBaseClass)->GetRelationshipClassCP();
    ECRelationshipClassCP queryClassHasLocalProperty = m_metaSchema.GetClassCP(METASCHEMA_RELCLASS_ClassHasLocalProperty)->GetRelationshipClassCP();

    agattr(g, AGRAPH, "layout", "dot");
    agattr(g, AGRAPH, "splines", "polyline");
    agattr(g, AGRAPH, "label", "");
    //agattr(g, AGRAPH, "labeljust", "l"); //this only left-justifies cluster graph labels
    //agattr(g, AGRAPH, "concentrate", "true");
    agattr(g, AGNODE, "shape", "record");
    agattr(g, AGNODE, "label", "");
    //agattr(g, AGNODE, "margin", "0.5,1.0");
    agattr(g, AGEDGE, "style", "solid");

    bvector<IECInstancePtr> schemaInstanceVector;
    gen.GenerateInstances(schemaInstanceVector, querySchemas);
    for (IECInstancePtr const& schemaInstance : schemaInstanceVector)
        {
        schemaInstance->GetValue(v, METASCHEMA_PROPERTY_SchemaDef_Name);
        Utf8String schemaName = v.GetUtf8CP();
        Utf8String schemaString = "cluster_";
        schemaString += schemaName;
        Agraph_t* h = agsubg(g, const_cast<char*>(schemaString.c_str()), TRUE);
        agset(h, "label", const_cast<char*>((schemaName + "\\l").c_str()));

        bvector<IECInstancePtr> classInstanceVector;
        gen.GenerateRelatedInstances(classInstanceVector, *schemaInstance, *querySchemaHasClass);
        for (IECInstancePtr const& classInstance : classInstanceVector)
            {
            if (METASCHEMA_CLASS_RelClassDef == classInstance->GetClass().GetName())
                {
                classInstance->GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
                Utf8String className = v.GetUtf8CP();
                Utf8String classString = schemaName;
                classString += "::";
                classString += className;
                Agnode_t* n = agnode(h, const_cast<char*>(classString.c_str()), TRUE);
                agset(n, "label", const_cast<char*>((className + "\\l").c_str()));

                bvector<IECInstancePtr> propInstanceVector;
                gen.GenerateRelatedInstances(propInstanceVector, *classInstance, *queryClassHasLocalProperty);
                Utf8String recordString = "{";
                recordString += (className + "\\l");
                recordString += "|";
                for (IECInstancePtr const& propInstance : propInstanceVector)
                    {
                    propInstance->GetValue(v, METASCHEMA_PROPERTY_PropertyDef_Name);
                    recordString += v.GetUtf8CP();
                    recordString += "\\l";
                    }
                recordString += "}";
                agset(n, "label", const_cast<char*>(recordString.c_str()));
                }
            }
        }

    for (IECInstancePtr const& schemaInstance : schemaInstanceVector)
        {
        schemaInstance->GetValue(v, METASCHEMA_PROPERTY_SchemaDef_Name);
        Utf8String schemaName = v.GetUtf8CP();
        Utf8String schemaString = "cluster_";
        schemaString += schemaName;
        Agraph_t* h = agsubg(g, const_cast<char*>(schemaString.c_str()), FALSE);

        bvector<IECInstancePtr> classInstanceVector;
        gen.GenerateRelatedInstances(classInstanceVector, *schemaInstance, *querySchemaHasClass);
        for (IECInstancePtr const& classInstance : classInstanceVector)
            {
            if (METASCHEMA_CLASS_RelClassDef == classInstance->GetClass().GetName())
                {
                classInstance->GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
                Utf8String className = v.GetUtf8CP();
                Utf8String classString = schemaName;
                classString += "::";
                classString += className;
                Agnode_t* n = agnode(h, const_cast<char*>(classString.c_str()), FALSE);

                bvector<IECInstancePtr> baseClassInstanceVector;
                gen.GenerateRelatedInstances(baseClassInstanceVector, *classInstance, *queryClassHasBaseClass);
                for (IECInstancePtr const& baseClassInstance : baseClassInstanceVector)
                    {
                    baseClassInstance->GetValue(v, METASCHEMA_PROPERTY_ClassDef_Name);
                    Utf8String baseClassName = v.GetUtf8CP();
                    Utf8String baseClassString = schemaName;
                    baseClassString += "::";
                    baseClassString += baseClassName;
                    Agnode_t* m = agnode(h, const_cast<char*>(baseClassString.c_str()), FALSE);

                    Utf8String edgeString = baseClassString;
                    edgeString += "->";
                    edgeString += classString;

                    Agedge_t* e = agedge(h, m, n, const_cast<char*>(edgeString.c_str()), TRUE);
                    agsafeset(e, "dir", "back", "");
                    agsafeset(e, "arrowtail", "onormal", "");
                    }
                }
            }
        }
    }

//=======================================================================================
// @bsistruct                                                   Mike.Embick     10/15
//=======================================================================================
struct BentleyGraphvizContext;
struct IDiagramRenderer
    {
    virtual void RenderDiagram(GraphvizDiagram& diagram, BentleyGraphvizContext& bgvc);
    //virtual void begin_job          (GVJ_t* job) = 0;
    //virtual void end_job            (GVJ_t* job) = 0;
    //virtual void begin_graph        (GVJ_t* job) = 0;
    //virtual void end_graph          (GVJ_t* job) = 0;
    ////virtual void begin_layer        (GVJ_t* job, char* layername, int layerNum, int numLayers) = 0;
    ////virtual void end_layer          (GVJ_t* job) = 0;
    //virtual void begin_page         (GVJ_t* job) = 0;
    //virtual void end_page           (GVJ_t* job) = 0;
    //virtual void begin_cluster      (GVJ_t* job) = 0;
    //virtual void end_cluster        (GVJ_t* job) = 0;
    ////virtual void begin_nodes        (GVJ_t* job) = 0;
    ////virtual void end_nodes          (GVJ_t* job) = 0;
    ////virtual void begin_edges        (GVJ_t* job) = 0;
    ////virtual void end_edges          (GVJ_t* job) = 0;
    //virtual void begin_node         (GVJ_t* job) = 0;
    //virtual void end_node           (GVJ_t* job) = 0;
    //virtual void begin_edge         (GVJ_t* job) = 0;
    //virtual void end_edge           (GVJ_t* job) = 0;
    ////virtual void begin_anchor       (GVJ_t* job, char* href, char* tooltip, char* target, char* id) = 0;
    ////virtual void end_anchor         (GVJ_t* job) = 0;
    //virtual void begin_label        (GVJ_t* job, label_type type) = 0;
    //virtual void end_label          (GVJ_t* job) = 0;
    virtual void textspan           (GVJ_t* job, pointf p, textspan_t* span) {}
    virtual void resolve_color      (GVJ_t* job, gvcolor_t* color) {}
    virtual void ellipse            (GVJ_t* job, pointf* A, int filled) {}
    virtual void polygon            (GVJ_t* job, pointf* A, int n, int filled) {}
    virtual void beziercurve        (GVJ_t* job, pointf* A, int n, int arrow_at_start, int arrow_at_end, int x) {}
    virtual void polyline           (GVJ_t* job, pointf* A, int n) {}
    virtual void comment            (GVJ_t* job, char* comment) {}
    ////virtual void library_shape      (GVJ_t* job, char* name, pointf* A, int n, int filled) = 0;
    };

//=======================================================================================
//                                                              Mike.Embick     10/15
//=======================================================================================
//[SVT_PLUGIN]
void mylib_begin_job(GVJ_t* job) {}
void mylib_end_job(GVJ_t* job) {}
void mylib_begin_graph(GVJ_t* job) {}
void mylib_end_graph(GVJ_t* job) {}

void mylib_begin_layer(GVJ_t* job, char* layername, int layerNum, int numLayers) {} //uncalled group
void mylib_end_layer(GVJ_t* job) {}

void mylib_begin_page(GVJ_t* job) {}
void mylib_end_page(GVJ_t* job) {}
void mylib_begin_cluster(GVJ_t* job) {}
void mylib_end_cluster(GVJ_t* job) {}

void mylib_begin_nodes(GVJ_t* job) {} //uncalled group
void mylib_end_nodes(GVJ_t* job) {}
void mylib_begin_edges(GVJ_t* job) {}
void mylib_end_edges(GVJ_t* job) {}

void mylib_begin_node(GVJ_t* job) {}
void mylib_end_node(GVJ_t* job) {}
void mylib_begin_edge(GVJ_t* job) {}
void mylib_end_edge(GVJ_t* job) {}

void mylib_begin_anchor(GVJ_t* job, char* href, char* tooltip, char* target, char* id) {} //uncalled group
void mylib_end_anchor(GVJ_t* job) {}

void mylib_begin_label(GVJ_t* job, label_type type) {}
void mylib_end_label(GVJ_t* job) {}

void mylib_textspan(GVJ_t* job, pointf p, textspan_t* span)
    {
    if (EXPECTED_CONDITION(nullptr != job->context))
        {
        ((IDiagramRenderer*)job->context)->textspan(job, p, span);
        }
    }

void mylib_resolve_color(GVJ_t* job, gvcolor_t* color)
    {
    if (EXPECTED_CONDITION(nullptr != job->context))
        {
        ((IDiagramRenderer*)job->context)->resolve_color(job, color);
        }
    }

void mylib_ellipse(GVJ_t* job, pointf* A, int filled)
    {
    if (EXPECTED_CONDITION(nullptr != job->context))
        {
        ((IDiagramRenderer*)job->context)->ellipse(job, A, filled);
        }
    }

void mylib_polygon(GVJ_t* job, pointf* A, int n, int filled)
    {
    if (EXPECTED_CONDITION(nullptr != job->context))
        {
        ((IDiagramRenderer*)job->context)->polygon(job, A, n, filled);
        }
    }

//the fourth int argument doesn't have a specified name in the library, I don't know what it does
void mylib_beziercurve(GVJ_t* job, pointf* A, int n, int arrow_at_start, int arrow_at_end, int x)
    {
    if (EXPECTED_CONDITION(nullptr != job->context))
        {
        ((IDiagramRenderer*)job->context)->beziercurve(job, A, n, arrow_at_start, arrow_at_end, x);
        }
    }

void mylib_polyline(GVJ_t* job, pointf* A, int n)
    {
    if (EXPECTED_CONDITION(nullptr != job->context))
        {
        ((IDiagramRenderer*)job->context)->polyline(job, A, n);
        }
    }

void mylib_comment(GVJ_t* job, char* comment)
    {
    if (EXPECTED_CONDITION(nullptr != job->context))
        {
        ((IDiagramRenderer*)job->context)->comment(job, comment);
        }
    }

void mylib_library_shape(GVJ_t* job, char* name, pointf* A, int n, int filled) {} //uncalled

gvrender_engine_t mylib_render_engine =
    {
    mylib_begin_job,
    mylib_end_job,
    mylib_begin_graph,
    mylib_end_graph,
    mylib_begin_layer,
    mylib_end_layer,
    mylib_begin_page,
    mylib_end_page,
    mylib_begin_cluster,
    mylib_end_cluster,
    mylib_begin_nodes,
    mylib_end_nodes,
    mylib_begin_edges,
    mylib_end_edges,
    mylib_begin_node,
    mylib_end_node,
    mylib_begin_edge,
    mylib_end_edge,
    mylib_begin_anchor,
    mylib_end_anchor,
    mylib_begin_label,
    mylib_end_label,
    mylib_textspan,
    mylib_resolve_color,
    mylib_ellipse,
    mylib_polygon,
    mylib_beziercurve,
    mylib_polyline,
    mylib_comment,
    mylib_library_shape,
    };

gvrender_features_t mylib_render_features =
    {
    0,                  //flags
    0.0,                //default_pad
    NULL,               //knowncolors
    0,                  //sz_knowncolors
    COLOR_STRING,       //color_type
    };

//gvdevice_engine_t mylib_device_engine =
//    {
//    0,
//    0,
//    0,
//    };

gvdevice_features_t mylib_device_features =
    {
    0,                  //flags
    { 0.,0. },          //default margin - points
    { 0.,0. },          //default page width, height - points
    { 72.,72. },        //default dpi
    };

boolean mylib_textlayout(textspan_t * span, char **fontpath)
    {
    TextStringStylePtr textspanStyle = TextStringStyle::Create();
    textspanStyle->SetFont(DgnFontManager::GetDecoratorFont());
    textspanStyle->SetSize(DPoint2d::From(span->font->size, span->font->size));

    TextStringPtr textspanText = TextString::Create();
    textspanText->SetText(span->str);
    textspanText->SetStyle(*textspanStyle);

    span->size.x = textspanText->GetRange().XLength();
    span->size.y = textspanText->GetRange().YLength() * 1.20;
    span->yoffset_layout = 0.0;
    span->yoffset_centerline = 0.1 * span->font->size;
    span->layout = NULL;
    span->free_layout = NULL;

    return TRUE;
    }

gvtextlayout_engine_t mylib_textlayout_engine =
    {
    mylib_textlayout,
    };

gvplugin_installed_t mylib_render_types[2] =
    {
        { 0, "mylib", 1, &mylib_render_engine, &mylib_render_features },
        { 0, NULL, 0, NULL, NULL }
    };

gvplugin_installed_t mylib_device_types[2] =
    {
        { 0, "mylib:mylib", 1, NULL, &mylib_device_features },
        { 0, NULL, 0, NULL, NULL }
    };

gvplugin_installed_t mylib_textlayout_types[2] =
    {
        { 0, "textlayout", 1, &mylib_textlayout_engine, NULL },
        { 0, NULL, 0, NULL, NULL }
    };

gvplugin_api_t mylib_apis[4] =
    {
        { API_device, mylib_device_types },
        { API_render, mylib_render_types },
        { API_textlayout, mylib_textlayout_types },
        { (api_t)0, 0 }
    };

gvplugin_library_t gvplugin_mylib_LTX_library = { "mylib", mylib_apis };

//=======================================================================================
// @bsistruct                                                   Mike.Embick     10/15
//=======================================================================================
struct BentleyGraphvizContext
    {
private:
    GVC_t* m_gvc;
    lt_symlist_t lt_preloaded_symbols[4];

public:
    BentleyGraphvizContext()
        {
        lt_preloaded_symbols[0] = { "gvplugin_dot_layout_LTX_library", (void*)(&gvplugin_dot_layout_LTX_library) };
        lt_preloaded_symbols[1] = { "gvplugin_core_LTX_library", (void*)(&gvplugin_core_LTX_library) };
        lt_preloaded_symbols[2] = { "gvplugin_mylib_LTX_library", (void*)(&gvplugin_mylib_LTX_library) };
        lt_preloaded_symbols[3] = { 0, 0 };
        m_gvc = gvContextPlugins(lt_preloaded_symbols, 0);
        }
    ~BentleyGraphvizContext()
        {
        gvFreeContext(m_gvc);
        }
    GVC_t* GetGVContext() const { return m_gvc; }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void IDiagramRenderer::RenderDiagram(GraphvizDiagram& diagram, BentleyGraphvizContext& bgvc)
    {
    gvLayout(bgvc.GetGVContext(), diagram.GetGraph(), "dot");
    gvRenderContext(bgvc.GetGVContext(), diagram.GetGraph(), "mylib", this);
    gvFreeLayout(bgvc.GetGVContext(), diagram.GetGraph());
    }

//=======================================================================================
// @bsistruct                                                   Mike.Embick     10/15
//=======================================================================================
struct CreateElementsDiagramRenderer : public IDiagramRenderer
    {
private:
    DgnDbPtr& db;
    DgnModelId& modelId;
    DgnCategoryId& categoryId;
    DgnClassId& classId;
    YawPitchRollAngles angles;
public:
    CreateElementsDiagramRenderer(DgnDbPtr& db, DgnModelId& modelId, DgnCategoryId& categoryId, DgnClassId& classId)
        : db(db), modelId(modelId), categoryId(categoryId), classId(classId) {}

    virtual void textspan           (GVJ_t* job, pointf p, textspan_t* span) override
        {
        GenericPhysicalObjectPtr textspanElement = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(*db, modelId, classId, categoryId));
        textspanElement->SetCode(DgnCode::CreateEmpty());

        DPoint3d origin = DPoint3d::From(p.x, p.y, 0.0);
        GeometryBuilderPtr builder = GeometryBuilder::Create(*textspanElement, origin, angles);

        TextStringStylePtr textspanStyle = TextStringStyle::Create();
        textspanStyle->SetFont(DgnFontManager::GetDecoratorFont());
        textspanStyle->SetSize(DPoint2d::From(span->font->size, span->font->size));

        TextStringPtr textspanText = TextString::Create();
        textspanText->SetText(span->str);
        textspanText->SetStyle(*textspanStyle);
        //textspanText->SetOrigin(DPoint3d::From(span->size.x / -2.0, 0.0, 0.0));
        textspanText->SetOrigin(DPoint3d::From(0.0, 0.0, 0.0));

        builder->Append(*textspanText);
        builder->SetGeometryStreamAndPlacement(*textspanElement);
        ASSERT_TRUE(db->Elements().Insert(*textspanElement).IsValid());
        }

    virtual void resolve_color      (GVJ_t* job, gvcolor_t* color) override
        {

        }

    virtual void ellipse            (GVJ_t* job, pointf* A, int filled) override
        {

        }

    virtual void polygon            (GVJ_t* job, pointf* A, int n, int filled) override
        {
        GenericPhysicalObjectPtr polygonElement = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(*db, modelId, classId, categoryId));
        polygonElement->SetCode(DgnCode::CreateEmpty());

        DPoint3d origin = DPoint3d::From(A[0].x, A[0].y, 0.0);
        GeometryBuilderPtr builder = GeometryBuilder::Create(*polygonElement, origin, angles);

        bvector<DPoint3d> points;
        for (int i = 0; i < n; i++)
            {
            points.push_back(DPoint3d::From(A[i].x - A[0].x, A[i].y - A[0].y, 0.0));
            }
        points.push_back(DPoint3d::From(0.0, 0.0, 0.0));

        ICurvePrimitivePtr polygonCurvePrimitive = ICurvePrimitive::CreateLineString(points);
        CurveVectorPtr polygonCurveVector = CurveVector::Create(polygonCurvePrimitive, CurveVector::BOUNDARY_TYPE_Open);

        builder->Append(*polygonCurveVector);
        builder->SetGeometryStreamAndPlacement(*polygonElement);
        ASSERT_TRUE(db->Elements().Insert(*polygonElement).IsValid());
        }

    virtual void beziercurve        (GVJ_t* job, pointf* A, int n, int arrow_at_start, int arrow_at_end, int x) override
        {
        GenericPhysicalObjectPtr bezierElement = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(*db, modelId, classId, categoryId));
        bezierElement->SetCode(DgnCode::CreateEmpty());

        DPoint3d origin = DPoint3d::From(A[0].x, A[0].y, 0.0);
        GeometryBuilderPtr builder = GeometryBuilder::Create(*bezierElement, origin, angles);

        bvector<DPoint3d> poles;
        for (int i = 0; i < n; i++)
            {
            poles.push_back(DPoint3d::From(A[i].x - A[0].x, A[i].y - A[0].y, 0.0));
            }

        bvector<double> knots;
        int s = (n-1)/3;
        double a = 1.0 / s;
        for (int i = 0; i < 4; i++) { knots.push_back(0.0); }
        for (int i = 1; i < s; i++)
            {
            for (int j = 0; j < 3; j++) { knots.push_back(i*a); }
            }
        for (int i = 0; i < 4; i++) { knots.push_back(1.0); }

        MSBsplineCurvePtr bezierSpline = MSBsplineCurve::CreateFromPolesAndOrder(poles, nullptr, &knots, 4, false, true);
        ICurvePrimitivePtr bezierCurvePrimitive = ICurvePrimitive::CreateBsplineCurve(bezierSpline);
        CurveVectorPtr bezierCurveVector = CurveVector::Create(bezierCurvePrimitive, CurveVector::BOUNDARY_TYPE_Open);

        builder->Append(*bezierCurveVector);
        builder->SetGeometryStreamAndPlacement(*bezierElement);
        ASSERT_TRUE(db->Elements().Insert(*bezierElement).IsValid());
        }

    virtual void polyline           (GVJ_t* job, pointf* A, int n) override
        {
        GenericPhysicalObjectPtr polylineElement = GenericPhysicalObject::Create(GenericPhysicalObject::CreateParams(*db, modelId, classId, categoryId));
        polylineElement->SetCode(DgnCode::CreateEmpty());

        DPoint3d origin = DPoint3d::From(A[0].x, A[0].y, 0.0);
        GeometryBuilderPtr builder = GeometryBuilder::Create(*polylineElement, origin, angles);

        bvector<DPoint3d> points;
        for (int i = 0; i < n; i++)
            {
            points.push_back(DPoint3d::From(A[i].x - A[0].x, A[i].y - A[0].y, 0.0));
            }

        ICurvePrimitivePtr polylineCurvePrimitive = ICurvePrimitive::CreateLineString(points);
        CurveVectorPtr polylineCurveVector = CurveVector::Create(polylineCurvePrimitive, CurveVector::BOUNDARY_TYPE_Open);

        builder->Append(*polylineCurveVector);
        builder->SetGeometryStreamAndPlacement(*polylineElement);
        ASSERT_TRUE(db->Elements().Insert(*polylineElement).IsValid());
        }

    virtual void comment            (GVJ_t* job, char* comment) override
        {

        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     11/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct GVFileDiagramRenderer : public IDiagramRenderer
    {
private:
    WCharCP m_filename;
public:
    GVFileDiagramRenderer(WCharCP filename) : m_filename(filename) {}

    virtual void RenderDiagram(GraphvizDiagram& diagram, BentleyGraphvizContext& bgvc) override
        {
        BeFileName fileName;
        BeTest::GetHost().GetOutputRoot(fileName);
        fileName.AppendToPath(m_filename);
        if (BeFileName::DoesPathExist(fileName)) { BeFileName::BeDeleteFile(fileName); }

        gvLayout(bgvc.GetGVContext(), diagram.GetGraph(), "dot");
        gvRenderFilename(bgvc.GetGVContext(), diagram.GetGraph(), "xdot", fileName.GetNameUtf8().c_str());
        gvFreeLayout(bgvc.GetGVContext(), diagram.GetGraph());
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mike.Embick     10/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(SchemaVisualizationTests, GraphvizDiagramTest)
    {
    //Make DgnDb, Schema, Scope, Category, Model
    BeFileName dbPath;
    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::GetTestDataOut(dbPath, L"3dMetricGeneral.ibim", L"GVDiagramTestOutput.bim", __FILE__));

    DbResult openStatus;
    DgnDbPtr db = DgnDb::OpenDgnDb(&openStatus, dbPath, DgnDb::OpenParams(Db::OpenMode::ReadWrite));
    ASSERT_TRUE(BE_SQLITE_OK == openStatus);
    ASSERT_TRUE(db.IsValid());

    ECDbSchemaManager const& manager = db->Schemas();
    ECSchemaCP targetSchema = manager.GetECSchema(BIS_ECSCHEMA_NAME);
    bvector<ECSchemaCP> scope;
    scope.push_back(targetSchema);

    DgnCategory category(DgnCategory::CreateParams(*db, "Physical Category", DgnCategory::Scope::Physical));
    DgnSubCategory::Appearance categoryAppearance;
    category.Insert(categoryAppearance);

    DgnCategoryId categoryId = category.GetCategoryId();
    ASSERT_TRUE(categoryId.IsValid());

    DgnModelPtr model = new SpatialModel(SpatialModel::CreateParams
        (
        *db,
        DgnClassId(db->Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_SpatialModel)),
        DgnModel::CreateModelCode("Physical Model")
        ));
    ASSERT_TRUE(DgnDbStatus::Success == model->Insert());

    DgnModelId modelId = model->GetModelId();
    ASSERT_TRUE(modelId.IsValid());

    DgnClassId pclassId = DgnClassId(db->Schemas().GetECClassId(GENERIC_DOMAIN_NAME, GENERIC_CLASSNAME_PhysicalObject));
    ASSERT_TRUE(pclassId.IsValid());

    //make diagram from scope
    GraphvizDiagram diagram;
    BentleyGraphvizContext bgvc;

    ClassDiagramBuilder cdb(GetMetaSchema());
    cdb.BuildDiagram(diagram, scope);

    //write diagram to file
    GVFileDiagramRenderer gvfdr(L"SchemaVisualizationTests_GraphvizDiagramTest.gv");
    gvfdr.RenderDiagram(diagram, bgvc);

    //draw diagram on model
    CreateElementsDiagramRenderer cedr(db, modelId, categoryId, pclassId);
    cedr.RenderDiagram(diagram, bgvc);

    //make a view & controller
    DrawingViewDefinition view(DrawingViewDefinition::CreateParams(*db, "SchemaVisualizationTests",
        ViewDefinition::Data(modelId, DgnViewSource::Generated)));
    EXPECT_TRUE(view.Insert().IsValid());

    DrawingViewController viewController(*db, view.GetViewId());
    viewController.SetStandardViewRotation(StandardView::Top);
    viewController.GetViewFlagsR().SetRenderMode(Render::RenderMode::Wireframe);
    viewController.ChangeCategoryDisplay(categoryId, true);
    viewController.ChangeModelDisplay(modelId, true);

    EXPECT_TRUE(BE_SQLITE_OK == viewController.Save());

    db->SaveSettings();
    }

/* 
Mike.Embick         12/15

This is the result of my research into programmatically generating vizualizations of ECSchemas.
It was thought that this would be a useful internal tool for developers in understanding schemas.
The file exists as a test and runs tests, but no code verification is happening.

Goals:
-Take a schema, either from an XML file or from a DgnDb, look at its structure, and automatically
  generate a graph showing inheritance relationships or RelationshipClass relationships between
  the ECClasses in it, essentially as a UML class diagram. (A diagram showing the referencing
  relationships between schemas was also considered, as well as a diagram showing inheritance
  among RelationshipClasses, which would otherwise be only arrows.)
-Render the diagram in a DgnDb so that something like SampleNavigator could view it.
-Make the diagram interactive, e.g. contract/expand nodes to hide their children, hover over a
  node to view its property list.

The first two of these goals were largely accomplished. No interactivity was added.

First, the ECObjects API was used to read schemas from files and to access the classes in a
given schema, or properties in a given class etc. Initially the target schemas were read from
XML files, but later, primarily from DgnDb files. In order to act on these EC objects the same
way, this was built into a MetaSchemaInstanceGenerator class. Search for [SVT_MSIG].

This uses a metaschema containing meta-ECClasses like ECSchemaDef, ECClassDef, ECPropertyDef
etc. to generate IECInstances of those classes to correspond to the real EC objects.
I.e. for the dgn schema:
an instance of ECSchemaDef representing the schema,
instances of ECClassDef representing Element, Item, Category, ...
for each class, instances of ECPropertyDef representing the class's properties, such as:
  Element.Label, .LastMod, .Descr ...; AnnotationTableCell.TextBlock, .FillKey, .Alignment ...

Instantiated with a metaschema and with a vector of schemas as its scope, the generator can make
instances of a named meta-class, or the instances of related to a given instance by a named
meta-relationship-class, based on what is in its scope.

The metaschema inlined in this file [SVT_META] is not representative of the current EC
metaschema, as it was made specifically for these tests. A better way to produce the instances
would be to use a metaschema with appropriate ECDb mapping custom attributes applied, imported
into an ECDb, so that the ec_ tables can be queried using an ECSQL query for the metaschema
classes, and then the query results can be turned into instances via ECInstanceECSqlSelectAdapter.
This was partially done in the ECDb test ECSqlMetadataQueryTest, but the metaschema there is also
unfinished as of this writing, and does not map every conceivable necessary meta-class.

Next, the Graphviz library was ported to x64 and its GPL'd dependencies cut out, in order to use
it for automatic graph layout. The IECInstances from the generator were corresponded to graph
elements to build a graph structure: initially they were turned into a sequence of strings in
the DOT graph description language and sent to a .GV file to be read by the library's editor
executable, gvedit; and later into graph objects using the library's graph types.

Classes created were graph type wrapper classes GraphvizDiagram and BentleyGraphvizContext, and
the builders ClassDiagramBuilder and SchemaDiagramBuilder, while the old versions using strings
and not Graphviz types were renamed to -StringBuilder.
The StringBuilders are tested in TestSchemaDiagram and TestClassDiagram.

Using Graphviz's "plugin" system, a renderer plugin [SVT_PLUGIN] was made solely to pass layout
information out of Graphviz. It required an extern [SVT_EXTERN]. The plugin is a group of
functions which are added as function pointers in a struct representing the render engine, and
the engine is itself in another struct array of "APIs", and that is included in another struct
representing the "library". In practice not all of the function pointers were called by
Graphviz, but they were all added for completeness.

IDiagramRenderer was created to connect to the plugin, and subclassed to implement different
renderings, such as generating Elements with geometry in a DgnDb, or sending the layout
information in DOT format to a .GV file. The Renderer takes a GraphvizDiagram and its context
and calls a Graphviz render method, passing itself as context, which calls the plugin, which
connects back to the Renderer by the passed context.
The two renderers are tested side by side, using ClassDiagramBuilder, in GraphvizDiagramTest.

Evidently some functional differences remained between Bentley's ported build of Graphviz and
the build on the site, because while receiving the same layout information, the two renderers
(DgnDb and gvedit) produced different results, including the minimization of edge crossings.
This point is where development stopped.

Generating elements on a DgnDb as in the CreateElementsDiagramRenderer was only a stepping stone
toward the actual goal of interactivity. We want to create non-persistent "GeometrySource"
objects which will be displayed as view decorations, rather than persisting elements with
geometry to the DgnDb. These will make it easier to provide the intended interactivity.
*/

#endif

#endif // NOT_NOW_ANDROID_STATIC_BB_1_ISSUE
