/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "../NonPublished/ECDbPublishedTests.h"

BEGIN_ECDBUNITTESTS_NAMESPACE

//=======================================================================================
// Rich ECSchema designed to maximize ECSQL parser/preparer code path coverage.
// Includes: class hierarchy (abstract + TPH), structs (flat/nested/deep), all primitive
// types, arrays (bounded/unbounded), navigation properties, relationships (1:N, N:N with
// props, 1:1), enums, KindOfQuantity, mixin, DateTime variants, and PropertyCategories.
// @bsiclass
//=======================================================================================
struct ECSqlFuzzSchema
{
static SchemaItem Get()
    {
    return SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="FuzzSchema" alias="fz" version="01.00.00" description="ECSQL fuzz testing schema"
          xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
    <ECSchemaReference name="CoreCustomAttributes" version="01.00.00" alias="CoreCA" />
    <ECSchemaReference name="ECDbMap" version="02.00.00" alias="ecdbmap" />

    <ECCustomAttributes>
        <SchemaMap xmlns="ECDbMap.02.00.00">
            <TablePrefix>fuzz</TablePrefix>
        </SchemaMap>
    </ECCustomAttributes>

    <KindOfQuantity typeName="FLength" description="Fuzz Length" displayLabel="Fuzz Length"
                    persistenceUnit="CM" relativeError="1E-3" presentationUnits="FT;IN" />

    <PropertyCategory typeName="FMisc" description="Miscellaneous" displayLabel="Miscellaneous" />
    <PropertyCategory typeName="FCore" description="Core" displayLabel="Core" priority="100" />

    <ECEnumeration typeName="FColor" backingTypeName="int" isStrict="true">
        <ECEnumerator name="Red" value="0" displayLabel="Red" />
        <ECEnumerator name="Green" value="1" displayLabel="Green" />
        <ECEnumerator name="Blue" value="2" displayLabel="Blue" />
    </ECEnumeration>

    <!-- Flat struct with all primitive types -->
    <ECStructClass typeName="FSimpleStruct">
        <ECProperty propertyName="sb" typeName="boolean" />
        <ECProperty propertyName="si" typeName="int" />
        <ECProperty propertyName="sl" typeName="long" />
        <ECProperty propertyName="sd" typeName="double" />
        <ECProperty propertyName="ss" typeName="string" />
        <ECProperty propertyName="sdt" typeName="dateTime" />
        <ECProperty propertyName="sp2d" typeName="point2d" />
        <ECProperty propertyName="sp3d" typeName="point3d" />
    </ECStructClass>

    <!-- Nested struct containing FSimpleStruct and an array of FSimpleStruct -->
    <ECStructClass typeName="FNestedStruct">
        <ECProperty propertyName="ns" typeName="string" />
        <ECStructProperty propertyName="inner" typeName="FSimpleStruct" />
        <ECStructArrayProperty propertyName="innerArray" typeName="FSimpleStruct" minOccurs="0" maxOccurs="unbounded" />
    </ECStructClass>

    <!-- Deep struct: three levels of nesting -->
    <ECStructClass typeName="FDeepStruct">
        <ECProperty propertyName="ds" typeName="string" />
        <ECStructProperty propertyName="nested" typeName="FNestedStruct" />
    </ECStructClass>

    <!-- Abstract base with TablePerHierarchy, all primitive types -->
    <ECEntityClass typeName="FBase" modifier="Abstract">
        <ECCustomAttributes>
            <ClassMap xmlns="ECDbMap.02.00.00">
                <MapStrategy>TablePerHierarchy</MapStrategy>
            </ClassMap>
        </ECCustomAttributes>
        <ECProperty propertyName="B" typeName="boolean" category="FCore" />
        <ECProperty propertyName="I" typeName="int" category="FCore" />
        <ECProperty propertyName="L" typeName="long" category="FCore" />
        <ECProperty propertyName="D" typeName="double" category="FCore" />
        <ECProperty propertyName="S" typeName="string" category="FCore" />
        <ECProperty propertyName="Bi" typeName="binary" />
        <ECProperty propertyName="Dt" typeName="dateTime" />
        <ECProperty propertyName="DtUtc" typeName="dateTime">
            <ECCustomAttributes>
                <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                    <DateTimeKind>Utc</DateTimeKind>
                </DateTimeInfo>
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="P2D" typeName="point2d" />
        <ECProperty propertyName="P3D" typeName="point3d" />
        <ECProperty propertyName="Color" typeName="FColor" />
    </ECEntityClass>

    <!-- FSub1: struct property + navigation property -->
    <ECEntityClass typeName="FSub1">
        <BaseClass>FBase</BaseClass>
        <ECProperty propertyName="Sub1Name" typeName="string" />
        <ECStructProperty propertyName="SimpleStruct" typeName="FSimpleStruct" category="FMisc" />
        <ECStructProperty propertyName="DeepStruct" typeName="FDeepStruct" />
        <ECNavigationProperty propertyName="Parent" relationshipName="FBaseHasSub1" direction="Backward">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.02.00.00" />
            </ECCustomAttributes>
        </ECNavigationProperty>
    </ECEntityClass>

    <!-- FSub2: array properties + mixin -->
    <ECEntityClass typeName="FSub2">
        <BaseClass>FBase</BaseClass>
        <BaseClass>FMixin</BaseClass>
        <ECProperty propertyName="Sub2Code" typeName="int" />
        <ECArrayProperty propertyName="IntArray" typeName="int" minOccurs="0" maxOccurs="unbounded" />
        <ECArrayProperty propertyName="StrArray" typeName="string" minOccurs="0" maxOccurs="unbounded" />
        <ECArrayProperty propertyName="DblArray" typeName="double" minOccurs="0" maxOccurs="unbounded" />
        <ECArrayProperty propertyName="BoundedArray" typeName="int" minOccurs="2" maxOccurs="10" />
        <ECStructArrayProperty propertyName="StructArray" typeName="FSimpleStruct" minOccurs="0" maxOccurs="unbounded" />
        <ECStructArrayProperty propertyName="NestedStructArray" typeName="FNestedStruct" minOccurs="0" maxOccurs="unbounded" />
    </ECEntityClass>

    <!-- FSub3: third-level inheritance -->
    <ECEntityClass typeName="FSub3">
        <BaseClass>FSub1</BaseClass>
        <ECProperty propertyName="Sub3Tag" typeName="string" />
        <ECProperty propertyName="Sub3Val" typeName="double" kindOfQuantity="FLength" />
    </ECEntityClass>

    <!-- Standalone entity with all property types for simple queries -->
    <ECEntityClass typeName="FStandalone">
        <ECProperty propertyName="Name" typeName="string" />
        <ECProperty propertyName="Code" typeName="int" />
        <ECProperty propertyName="Amount" typeName="double" />
        <ECProperty propertyName="Flag" typeName="boolean" />
        <ECProperty propertyName="Created" typeName="dateTime">
            <ECCustomAttributes>
                <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                    <DateTimeKind>Utc</DateTimeKind>
                </DateTimeInfo>
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="DateOnly" typeName="dateTime">
            <ECCustomAttributes>
                <DateTimeInfo xmlns="CoreCustomAttributes.01.00.00">
                    <DateTimeComponent>Date</DateTimeComponent>
                </DateTimeInfo>
            </ECCustomAttributes>
        </ECProperty>
        <ECProperty propertyName="Pt2" typeName="point2d" />
        <ECProperty propertyName="Pt3" typeName="point3d" />
        <ECStructProperty propertyName="Details" typeName="FSimpleStruct" />
        <ECArrayProperty propertyName="Tags" typeName="string" minOccurs="0" maxOccurs="unbounded" />
        <ECProperty propertyName="Color" typeName="FColor" />
        <ECNavigationProperty propertyName="Owner" relationshipName="FBaseOwnsStandalone" direction="Backward">
            <ECCustomAttributes>
                <ForeignKeyConstraint xmlns="ECDbMap.02.00.00" />
            </ECCustomAttributes>
        </ECNavigationProperty>
    </ECEntityClass>

    <!-- Mixin class applied to FSub2 -->
    <ECEntityClass typeName="FMixin" modifier="Abstract">
        <ECCustomAttributes>
            <IsMixin xmlns="CoreCustomAttributes.01.00.00">
                <AppliesToEntityClass>FBase</AppliesToEntityClass>
            </IsMixin>
        </ECCustomAttributes>
        <ECProperty propertyName="MixinLabel" typeName="string" />
    </ECEntityClass>

    <!-- Relationship: 1:N embedding, FBase owns FStandalone -->
    <ECRelationshipClass typeName="FBaseOwnsStandalone" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" polymorphic="true" roleLabel="owns">
            <Class class="FBase" />
        </Source>
        <Target multiplicity="(0..*)" polymorphic="true" roleLabel="owned by">
            <Class class="FStandalone" />
        </Target>
    </ECRelationshipClass>

    <!-- Relationship: N:N with properties (link table) -->
    <ECRelationshipClass typeName="FStandaloneRefsFBase" strength="referencing" modifier="Sealed">
        <ECProperty propertyName="RefD" typeName="double" />
        <ECProperty propertyName="RefI" typeName="int" />
        <ECProperty propertyName="RefS" typeName="string" />
        <Source multiplicity="(0..*)" polymorphic="true" roleLabel="references">
            <Class class="FStandalone" />
        </Source>
        <Target multiplicity="(0..*)" polymorphic="true" roleLabel="referenced by">
            <Class class="FBase" />
        </Target>
    </ECRelationshipClass>

    <!-- Relationship: 1:N for FSub1 children -->
    <ECRelationshipClass typeName="FBaseHasSub1" strength="embedding" modifier="Sealed">
        <Source multiplicity="(0..1)" polymorphic="true" roleLabel="has">
            <Class class="FBase" />
        </Source>
        <Target multiplicity="(0..*)" polymorphic="true" roleLabel="belongs to">
            <Class class="FSub1" />
        </Target>
    </ECRelationshipClass>

    <!-- Relationship: 1:1 between FSub1 and FSub2 -->
    <ECRelationshipClass typeName="FSub1HasSub2" strength="referencing" modifier="Sealed">
        <Source multiplicity="(0..1)" polymorphic="true" roleLabel="paired with">
            <Class class="FSub1" />
        </Source>
        <Target multiplicity="(0..1)" polymorphic="true" roleLabel="paired with">
            <Class class="FSub2" />
        </Target>
    </ECRelationshipClass>

</ECSchema>)xml");
    }

//! Populates the ECDb with seed data for fuzz testing.
//! Inserts instances with varied data (NULLs, boundary values, etc.) to exercise Step() paths.
static void PopulateSeedData(ECDbR ecdb, TestHelper const& helper)
    {
    // FSub1 instances
    for (int i = 0; i < 10; i++)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO fz.FSub1(B,I,L,D,S,Sub1Name) VALUES(%s,%d,%d,%f,'sub1_%d','name_%d')",
            (i % 2 == 0) ? "true" : "false", i, (int64_t)i * 1000, i * 1.5, i, i);
        helper.ExecuteECSql(ecsql.c_str());
        }

    // FSub2 instances
    for (int i = 0; i < 10; i++)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO fz.FSub2(B,I,L,D,S,Sub2Code,MixinLabel) VALUES(%s,%d,%d,%f,'sub2_%d',%d,'mixin_%d')",
            (i % 3 == 0) ? "true" : "false", i + 100, (int64_t)(i + 100) * 1000, (i + 100) * 2.5, i, i * 7, i);
        helper.ExecuteECSql(ecsql.c_str());
        }

    // FSub3 instances
    for (int i = 0; i < 5; i++)
        {
        Utf8String ecsql;
        ecsql.Sprintf("INSERT INTO fz.FSub3(B,I,L,D,S,Sub1Name,Sub3Tag,Sub3Val) VALUES(%s,%d,%d,%f,'sub3_%d','n3_%d','tag_%d',%f)",
            (i % 2 == 0) ? "true" : "false", i + 200, (int64_t)(i + 200) * 1000, (i + 200) * 3.0, i, i, i, i * 99.9);
        helper.ExecuteECSql(ecsql.c_str());
        }

    // FStandalone instances (some with NULLs for boundary testing)
    for (int i = 0; i < 15; i++)
        {
        Utf8String ecsql;
        if (i % 5 == 0)
            ecsql.Sprintf("INSERT INTO fz.FStandalone(Name,Code,Amount,Flag) VALUES(NULL,%d,NULL,%s)", i, (i % 2) ? "true" : "false");
        else
            ecsql.Sprintf("INSERT INTO fz.FStandalone(Name,Code,Amount,Flag) VALUES('standalone_%d',%d,%f,%s)", i, i, i * 12.34, (i % 2) ? "true" : "false");
        helper.ExecuteECSql(ecsql.c_str());
        }

    ecdb.SaveChanges();
    }
};

END_ECDBUNITTESTS_NAMESPACE
