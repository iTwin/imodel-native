/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/FileFormatCompatibilityTests.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPublishedTests.h"
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeTextFile.h>
#include <Bentley/BeNumerical.h>
#include <BeSQLite/BeBriefcaseBasedIdSequence.h>
#include <GeomSerialization/GeomSerializationApi.h>

USING_NAMESPACE_BENTLEY_EC

BEGIN_ECDBUNITTESTS_NAMESPACE

#define PROFILETABLE_SELECT_Schema "SELECT Name,DisplayLabel,Description,Alias,VersionDigit1,VersionDigit2,VersionDigit3 FROM ec_Schema ORDER BY Name"
#define PROFILETABLE_SELECT_SchemaReference "SELECT s.Name,ref.Name FROM ec_SchemaReference sr JOIN ec_Schema s ON sr.SchemaId=s.Id JOIN ec_Schema ref ON sr.ReferencedSchemaId=ref.Id ORDER BY s.Name,ref.Name"
#define PROFILETABLE_SELECT_Class "SELECT s.Name,c.Name,c.DisplayLabel,c.Description,c.Type,c.Modifier,c.RelationshipStrength,c.RelationshipStrengthDirection,c.CustomAttributeContainertype FROM ec_Class c JOIN ec_Schema s ON s.Id=c.SchemaId ORDER BY s.Name, c.Name"
#define PROFILETABLE_SELECT_ClassHasBaseClasses "SELECT s.Name, c.Name, baseS.Name, baseC.Name FROM ec_ClassHasBaseClasses bc JOIN ec_Class c ON bc.ClassId=c.Id JOIN ec_Schema s ON s.Id=c.SchemaId " \
                                                "JOIN ec_Class baseC ON bc.BaseClassId=baseC.Id JOIN ec_Schema baseS ON baseS.Id=baseC.SchemaId " \
                                                "ORDER BY s.Name, c.Name, baseS.Name, baseC.Name, bc.Ordinal"
#define PROFILETABLE_SELECT_Enumeration "SELECT s.Name, e.Name, e.DisplayLabel,e.Description,e.UnderlyingPrimitiveType,e.IsStrict,e.EnumValues FROM ec_Enumeration e JOIN ec_Schema s ON s.Id=e.SchemaId ORDER BY s.Name,e.Name"
#define PROFILETABLE_SELECT_KindOfQunatity "SELECT s.Name, koq.Name, koq.DisplayLabel,koq.Description,koq.PersistenceUnit,koq.RelativeError,koq.PresentationUnits FROM ec_KindOfQuantity koq JOIN ec_Schema s ON s.Id=koq.SchemaId ORDER BY s.Name,koq.Name"
#define PROFILETABLE_SELECT_PropertyCategory "SELECT s.Name, pc.Name, pc.DisplayLabel,pc.Description,pc.Priority FROM ec_PropertyCategory pc JOIN ec_Schema s ON s.Id=pc.SchemaId ORDER BY s.Name,pc.Name"
#define PROFILETABLE_SELECT_Property "SELECT s.Name, p.Name, p.DisplayLabel,p.Description,p.IsReadonly,p.Priority,p.Ordinal,p.Kind,p.PrimitiveType,p.PrimitiveTypeMinLength,p.PrimitiveTypeMaxLength,p.PrimitiveTypeMinValue,p.PrimitiveTypeMaxValue, " \
                                    "enumS.Name, enum.Name, structS.Name,struct.Name, p.ExtendedTypeName, koqS.Name,koq.Name,catS.Name,cat.Name,p.ArrayMinOccurs,p.ArrayMaxOccurs, " \
                                    "navRelS.Name,navRel.Name, p.NavigationDirection FROM ec_Property p " \
                                    "JOIN ec_Class c ON p.ClassId=c.Id JOIN ec_Schema s ON s.Id=c.SchemaId " \
                                    "JOIN ec_Enumeration enum ON enum.Id=p.EnumerationId JOIN ec_Schema enumS ON enumS.Id=enum.SchemaId " \
                                    "JOIN ec_Class struct ON struct.Id=p.StructClassId JOIN ec_Schema structS ON structS.Id=struct.SchemaId " \
                                    "JOIN ec_KindOfQuantity koq ON koq.Id=p.KindOfQuantityId JOIN ec_Schema koqS ON koqS.Id=koq.SchemaId " \
                                    "JOIN ec_PropertyCategory cat ON cat.Id=p.CategoryId JOIN ec_Schema catS ON catS.Id=cat.SchemaId " \
                                    "JOIN ec_Class navRel ON navRel.Id=p.NavigationRelationshipClassId JOIN ec_Schema navRelS ON navRelS.Id=navRel.SchemaId " \
                                    "ORDER BY s.Name,c.Name,p.Name"
#define PROFILETABLE_SELECT_RelationshipConstraint "SELECT relS.Name, rel.Name, rc.RelationshipEnd,rc.MultiplicityLowerLimit,rc.MultiplicityUpperLimit,rc.IsPolymorphic,rc.RoleLabel,abstractConstraintS.Name, abstractConstraint.Name FROM ec_RelationshipConstraint rc " \
                                    "JOIN ec_Class rel ON rel.Id=rc.RelationshipClassId JOIN ec_Schema relS ON relS.Id=rel.SchemaId " \
                                    "JOIN ec_Class abstractConstraint ON rc.AbstractConstraintClassId=abstractConstraint.Id JOIN ec_Schema abstractConstraintS ON abstractConstraintS.Id=abstractConstraint.SchemaId " \
                                    "ORDER BY relS.Name, rel.Name, rc.RelationshipEnd"
#define PROFILETABLE_SELECT_RelationshipConstraintClass "SELECT relS.Name,rel.Name,rc.RelationshipEnd,constraintS.Name,constraintC.Name FROM ec_RelationshipConstraintClass rcc " \
                                    "JOIN ec_RelationshipConstraint rc ON rc.Id=rcc.ConstraintId JOIN ec_Class rel ON rel.Id=rc.RelationshipClassId JOIN ec_Schema relS ON relS.Id=rel.SchemaId " \
                                    "JOIN ec_Class constraintC ON rcc.ClassId=constraintC.Id JOIN ec_Schema constraintS ON constraintS.Id=constraintC.SchemaId " \
                                    "ORDER BY relS.Name,rel.Name,rc.RelationshipEnd,constraintS.Name,constraintC.Name"
#define PROFILETABLE_SELECT_CustomAttribute "SELECT Instance,Ordinal,ContainerType FROM ec_CustomAttribute ORDER BY ClassId,ContainerId,ContainerType,Ordinal"
#define PROFILETABLE_SELECT_ClassMap "SELECT s.Name, c.Name, cm.MapStrategy,cm.ShareColumnsMode,cm.MaxSharedColumnsBeforeOverflow,cm.JoinedTableInfo FROM ec_ClassMap cm JOIN ec_Class c ON cm.ClassId=c.Id JOIN ec_Schema s ON s.Id=c.SchemaId ORDER BY s.Name,c.Name"
#define PROFILETABLE_SELECT_PropertyPath "SELECT rootPropS.Name, rootPropC.Name, rootProp.Name, pp.AccessString FROM ec_PropertyPath pp " \
                                    "JOIN ec_Property rootProp ON rootProp.Id=pp.RootPropertyId JOIN ec_Class rootPropC ON rootPropC.Id=rootProp.ClassId JOIN ec_Schema rootPropS ON rootPropS.Id=rootPropC.SchemaId " \
                                    "ORDER BY rootPropS.Name,rootPropC.Name, rootProp.Name,pp.AccessString"
#define PROFILETABLE_SELECT_PropertyMap "SELECT s.Name, c.Name, rootPropS.Name,rootPropC.Name,rootProp.Name,t.Name,col.Name FROM ec_PropertyMap pm " \
                                    "JOIN ec_Class c ON c.Id=pm.ClassId JOIN ec_Schema s ON s.Id=c.SchemaId " \
                                    "JOIN ec_PropertyPath pp ON pp.Id=pm.PropertyPathId JOIN ec_Property rootProp ON rootProp.Id=pp.RootPropertyId JOIN ec_Class rootPropC ON rootPropC.Id=rootProp.ClassId JOIN ec_Schema rootPropS ON rootPropS.Id=rootPropC.SchemaId " \
                                    "JOIN ec_Column col ON col.Id=pm.ColumnId JOIN ec_Table t ON t.Id=col.TableId " \
                                    "ORDER BY s.Name,c.Name,rootPropS.Name,rootPropC.Name,rootProp.Name,t.Name,col.Name"
#define PROFILETABLE_SELECT_Table "SELECT parentT.Name, t.Name, t.Type, exclusiveRootClassS.Name, exclusiveRootClass.Name, t.UpdatableViewName FROM ec_table t " \
                                    "JOIN ec_Table parentT ON parentT.Id=t.ParentTableId JOIN ec_Class exclusiveRootClass ON exclusiveRootClass.Id=t.ExclusiveRootClassId JOIN ec_Schema exclusiveRootClassS ON exclusiveRootClassS.Id=exclusiveRootClass.SchemaId " \
                                    "ORDER BY t.Name"
#define PROFILETABLE_SELECT_Column "SELECT t.Name, c.Name,c.Type,c.IsVirtual,c.Ordinal,c.NotNullConstraint,c.UniqueConstraint,c.CheckConstraint,c.DefaultConstraint,c.CollationConstraint,c.OrdinalInPrimaryKey,c.ColumnKind FROM ec_Column c " \
                                    "JOIN ec_Table t ON t.Id=c.TableId ORDER BY t.Name, c.Name"
#define PROFILETABLE_SELECT_Index "SELECT i.Name, t.Name, i.IsUnique,i.AddNotNullWhereExp,i.IsAutoGenerated,s.Name,c.Name,i.AppliesToSubclassesIfPartial FROM ec_Index i " \
                                    "JOIN ec_Table t ON t.Id=i.TableId JOIN ec_Class c ON c.Id=i.ClassId JOIN ec_Schema s ON s.Id=c.SchemaId ORDER BY i.Name"
#define PROFILETABLE_SELECT_IndexColumn "SELECT i.Name, c.Name, t.Name, ic.Ordinal FROM ec_IndexColumn ic " \
                                    "JOIN ec_Index i ON i.Id=ic.IndexId JOIN ec_Column c ON c.Id=ic.ColumnId JOIN ec_Table t ON t.Id=c.TableId ORDER BY i.Name, ic.Ordinal"

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
struct FileFormatCompatibilityTests : ECDbTestFixture
    {
    protected:
        enum class CompareOptions
            {
            None = 0,
            DoNotComparePk = 1
            };

    private:
        static ProfileVersion const* s_initialBim2ProfileVersion;

        BentleyStatus CreateFakeBimFile(Utf8CP fileName, BeFileNameCR bisSchemaFolder);
        BentleyStatus ImportSchemasFromFolder(BeFileName const& schemaFolder);

    protected:
        BentleyStatus SetupTestFile(Utf8CP fileName);
        static bool CompareTable(DbCR benchmark, DbCR actual, Utf8CP tableName, Utf8CP selectSql, CompareOptions options = CompareOptions::None);

        static Utf8String GetPkColumnName(DbCR db, Utf8CP tableName);
        static BeFileName GetBenchmarkFileFolder(ProfileVersion const&);
        static BeFileName GetBenchmarkSchemaFolder();

        static ProfileVersion const& InitialBim2ProfileVersion() { return *s_initialBim2ProfileVersion; }
    };


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  09/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, PrimitiveDataTypeFormat)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("primitivedatatypesformatfileformatcompatibility.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                    <ECStructClass typeName="Types" modifier="Sealed">
                        <ECProperty propertyName="bl" typeName="binary" />
                        <ECProperty propertyName="b" typeName="boolean" />
                        <ECProperty propertyName="dt" typeName="datetime" />
                        <ECProperty propertyName="d" typeName="double" />
                        <ECProperty propertyName="g" typeName="Bentley.Geometry.Common.IGeometry" />
                        <ECProperty propertyName="i" typeName="int" />
                        <ECProperty propertyName="l" typeName="long" />
                        <ECProperty propertyName="pt2d" typeName="point2d" />
                        <ECProperty propertyName="pt3d" typeName="point3d" />
                        <ECProperty propertyName="s" typeName="string" />
                        <ECArrayProperty propertyName="bl_array" typeName="binary" />
                        <ECArrayProperty propertyName="b_array" typeName="boolean" />
                        <ECArrayProperty propertyName="dt_array" typeName="datetime" />
                        <ECArrayProperty propertyName="d_array" typeName="double" />
                        <ECArrayProperty propertyName="g_array" typeName="Bentley.Geometry.Common.IGeometry" />
                        <ECArrayProperty propertyName="i_array" typeName="int" />
                        <ECArrayProperty propertyName="l_array" typeName="long" />
                        <ECArrayProperty propertyName="pt2d_array" typeName="point2d" />
                        <ECArrayProperty propertyName="pt3d_array" typeName="point3d" />
                        <ECArrayProperty propertyName="s_array" typeName="string" />
                    </ECStructClass>
                    <ECEntityClass typeName="MyClass" modifier="Sealed">
                        <ECProperty propertyName="bl" typeName="binary" />
                        <ECProperty propertyName="b" typeName="boolean" />
                        <ECProperty propertyName="dt" typeName="datetime" />
                        <ECProperty propertyName="d" typeName="double" />
                        <ECProperty propertyName="g" typeName="Bentley.Geometry.Common.IGeometry" />
                        <ECProperty propertyName="i" typeName="int" />
                        <ECProperty propertyName="l" typeName="long" />
                        <ECProperty propertyName="pt2d" typeName="point2d" />
                        <ECProperty propertyName="pt3d" typeName="point3d" />
                        <ECProperty propertyName="s" typeName="string" />
                        <ECArrayProperty propertyName="bl_array" typeName="binary" />
                        <ECArrayProperty propertyName="b_array" typeName="boolean" />
                        <ECArrayProperty propertyName="dt_array" typeName="datetime" />
                        <ECArrayProperty propertyName="d_array" typeName="double" />
                        <ECArrayProperty propertyName="g_array" typeName="Bentley.Geometry.Common.IGeometry" />
                        <ECArrayProperty propertyName="i_array" typeName="int" />
                        <ECArrayProperty propertyName="l_array" typeName="long" />
                        <ECArrayProperty propertyName="pt2d_array" typeName="point2d" />
                        <ECArrayProperty propertyName="pt3d_array" typeName="point3d" />
                        <ECArrayProperty propertyName="s_array" typeName="string" />
                        <ECStructProperty propertyName="struct" typeName="Types"/>
                        <ECStructArrayProperty propertyName="struct_array" typeName="Types"/>
                    </ECEntityClass>
                </ECSchema>)xml")));

    const bool boolVal = true;
    const double doubleVal = -3.14151617;
    const DateTime dtVal = DateTime(DateTime::Kind::Unspecified, 2017, 9, 6, 12, 7);
    double jdVal = -1.0;
    ASSERT_EQ(SUCCESS, dtVal.ToJulianDay(jdVal));

    const IGeometryPtr geomVal = IGeometry::Create(ICurvePrimitive::CreateLine(DSegment3d::From(0.1324, 0.98432, 0.0, 231.453, 22.99, 1.0)));
    bvector<Byte> geometryBlobVec;
    BentleyGeometryFlatBuffer::GeometryToBytes(*geomVal, geometryBlobVec);
    ASSERT_FALSE(geometryBlobVec.empty());
    const void* geometryBlobVal = geometryBlobVec.data();
    const int geometryBlobSize = (int) geometryBlobVec.size();
    Utf8String geomBlobBase64Str;
    Base64Utilities::Encode(geomBlobBase64Str, static_cast<Byte const*> (geometryBlobVal), (size_t) geometryBlobSize);

    const int intVal = 314;
    const int64_t int64Val = INT64_C(12314234234);
    void const* blobVal = &int64Val;
    const int blobSize = (int) sizeof(int64Val);
    Utf8String blobBase64Str;
    Base64Utilities::Encode(blobBase64Str, static_cast<Byte const*> (blobVal), (size_t) blobSize);

    const Utf8CP stringVal = "Hello, world!!";
    const DPoint2d pt2dVal = DPoint2d::From(-3123.55435345, 1112.567);
    const DPoint3d pt3dVal = DPoint3d::From(1.0, -2.567, 14324.43223);

    {
    //insert row with NULLs
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MyClass(ECInstanceId) VALUES(NULL)"));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Reset();
    stmt.ClearBindings();

    Statement verifyStmt;
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.Prepare(m_ecdb, "SELECT * FROM ts_MyClass WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step());
    const int colCount = verifyStmt.GetColumnCount();
    ASSERT_EQ(48, colCount);

    for (int i = 0; i < colCount; i++)
        {
        if (BeStringUtilities::StricmpAscii("Id", verifyStmt.GetColumnName(i)) == 0)
            {
            ASSERT_EQ(key.GetInstanceId(), verifyStmt.GetValueId<ECInstanceId>(i));
            continue;
            }

        ASSERT_TRUE(verifyStmt.IsColumnNull(i)) << "Column is expected to be NULL when binding NULL to all parameters";
        }
    ASSERT_EQ(BE_SQLITE_DONE, verifyStmt.Step());
    }

    {
    //insert row with prim values
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MyClass(bl,b,dt,d,g,i,l,pt2d,pt3d,s) VALUES(?,?,?,?,?,?,?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(1, blobVal, blobSize, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(2, boolVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(3, dtVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(4, doubleVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(5, *geomVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(6, intVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(7, int64Val));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(8, pt2dVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(9, pt3dVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(10, stringVal, IECSqlBinder::MakeCopy::No));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Reset();
    stmt.ClearBindings();

    Statement verifyStmt;
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.Prepare(m_ecdb, "SELECT * FROM ts_MyClass WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step());
    const int colCount = verifyStmt.GetColumnCount();
    ASSERT_EQ(48, colCount);
    for (int i = 0; i < colCount; i++)
        {
        Utf8CP colName = verifyStmt.GetColumnName(i);
        if (BeStringUtilities::StricmpAscii("Id", colName) == 0)
            ASSERT_EQ(key.GetInstanceId(), verifyStmt.GetValueId<ECInstanceId>(i));
        else if (BeStringUtilities::StricmpAscii("bl", colName) == 0)
            ASSERT_EQ(0, memcmp(blobVal, verifyStmt.GetValueBlob(i), (size_t) blobSize)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("b", colName) == 0)
            ASSERT_EQ(boolVal, verifyStmt.GetValueBoolean(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("dt", colName) == 0)
            ASSERT_DOUBLE_EQ(jdVal, verifyStmt.GetValueDouble(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("d", colName) == 0)
            ASSERT_DOUBLE_EQ(doubleVal, verifyStmt.GetValueDouble(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("g", colName) == 0)
            ASSERT_EQ(0, memcmp(geometryBlobVal, verifyStmt.GetValueBlob(i), (size_t) geometryBlobSize)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("i", colName) == 0)
            ASSERT_EQ(intVal, verifyStmt.GetValueInt(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("l", colName) == 0)
            ASSERT_EQ(int64Val, verifyStmt.GetValueInt64(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("pt2d_x", colName) == 0)
            ASSERT_DOUBLE_EQ(pt2dVal.x, verifyStmt.GetValueDouble(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("pt2d_y", colName) == 0)
            ASSERT_DOUBLE_EQ(pt2dVal.y, verifyStmt.GetValueDouble(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("pt3d_x", colName) == 0)
            ASSERT_DOUBLE_EQ(pt3dVal.x, verifyStmt.GetValueDouble(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("pt3d_y", colName) == 0)
            ASSERT_DOUBLE_EQ(pt3dVal.y, verifyStmt.GetValueDouble(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("pt3d_z", colName) == 0)
            ASSERT_DOUBLE_EQ(pt3dVal.z, verifyStmt.GetValueDouble(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("s", colName) == 0)
            ASSERT_STREQ(stringVal, verifyStmt.GetValueText(i)) << "Column: " << colName;
        else
            ASSERT_TRUE(verifyStmt.IsColumnNull(i)) << "Column: " << colName;
        }
    ASSERT_EQ(BE_SQLITE_DONE, verifyStmt.Step());
    }

    {
    //insert row with prim array values
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MyClass(bl_array,b_array,dt_array,d_array,g_array,i_array,l_array,pt2d_array,pt3d_array,s_array) VALUES(?,?,?,?,?,?,?,?,?,?)"));
    IECSqlBinder& blArrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, blArrayBinder.AddArrayElement().BindBlob(blobVal, blobSize, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, blArrayBinder.AddArrayElement().BindBlob(blobVal, blobSize, IECSqlBinder::MakeCopy::No));

    IECSqlBinder& bArrayBinder = stmt.GetBinder(2);
    ASSERT_EQ(ECSqlStatus::Success, bArrayBinder.AddArrayElement().BindBoolean(boolVal));
    ASSERT_EQ(ECSqlStatus::Success, bArrayBinder.AddArrayElement().BindBoolean(boolVal));

    IECSqlBinder& dtArrayBinder = stmt.GetBinder(3);
    ASSERT_EQ(ECSqlStatus::Success, dtArrayBinder.AddArrayElement().BindDateTime(dtVal));
    ASSERT_EQ(ECSqlStatus::Success, dtArrayBinder.AddArrayElement().BindDateTime(dtVal));

    IECSqlBinder& dArrayBinder = stmt.GetBinder(4);
    ASSERT_EQ(ECSqlStatus::Success, dArrayBinder.AddArrayElement().BindDouble(doubleVal));
    ASSERT_EQ(ECSqlStatus::Success, dArrayBinder.AddArrayElement().BindDouble(doubleVal));

    IECSqlBinder& gArrayBinder = stmt.GetBinder(5);
    ASSERT_EQ(ECSqlStatus::Success, gArrayBinder.AddArrayElement().BindGeometry(*geomVal));
    ASSERT_EQ(ECSqlStatus::Success, gArrayBinder.AddArrayElement().BindGeometry(*geomVal));

    IECSqlBinder& iArrayBinder = stmt.GetBinder(6);
    ASSERT_EQ(ECSqlStatus::Success, iArrayBinder.AddArrayElement().BindInt(intVal));
    ASSERT_EQ(ECSqlStatus::Success, iArrayBinder.AddArrayElement().BindInt(intVal));

    IECSqlBinder& lArrayBinder = stmt.GetBinder(7);
    ASSERT_EQ(ECSqlStatus::Success, lArrayBinder.AddArrayElement().BindInt64(int64Val));
    ASSERT_EQ(ECSqlStatus::Success, lArrayBinder.AddArrayElement().BindInt64(int64Val));

    IECSqlBinder& pt2dArrayBinder = stmt.GetBinder(8);
    ASSERT_EQ(ECSqlStatus::Success, pt2dArrayBinder.AddArrayElement().BindPoint2d(pt2dVal));
    ASSERT_EQ(ECSqlStatus::Success, pt2dArrayBinder.AddArrayElement().BindPoint2d(pt2dVal));

    IECSqlBinder& pt3dArrayBinder = stmt.GetBinder(9);
    ASSERT_EQ(ECSqlStatus::Success, pt3dArrayBinder.AddArrayElement().BindPoint3d(pt3dVal));
    ASSERT_EQ(ECSqlStatus::Success, pt3dArrayBinder.AddArrayElement().BindPoint3d(pt3dVal));

    IECSqlBinder& sArrayBinder = stmt.GetBinder(10);
    ASSERT_EQ(ECSqlStatus::Success, sArrayBinder.AddArrayElement().BindText(stringVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, sArrayBinder.AddArrayElement().BindText(stringVal, IECSqlBinder::MakeCopy::No));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Reset();
    stmt.ClearBindings();

    Statement verifyStmt;
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.Prepare(m_ecdb, "SELECT * FROM ts_MyClass WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step());
    const int colCount = verifyStmt.GetColumnCount();
    ASSERT_EQ(48, colCount);
    for (int i = 0; i < colCount; i++)
        {
        Utf8CP colName = verifyStmt.GetColumnName(i);
        if (BeStringUtilities::StricmpAscii("Id", colName) == 0)
            ASSERT_EQ(key.GetInstanceId(), verifyStmt.GetValueId<ECInstanceId>(i));
        else if (BeStringUtilities::StricmpAscii("bl_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_STREQ(blobBase64Str.c_str(), arrayElementJson.GetString()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("b_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_EQ(boolVal, arrayElementJson.GetBool()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("dt_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_DOUBLE_EQ(jdVal, arrayElementJson.GetDouble()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("d_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_DOUBLE_EQ(doubleVal, arrayElementJson.GetDouble()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("g_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_STREQ(geomBlobBase64Str.c_str(), arrayElementJson.GetString()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("i_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_EQ(intVal, arrayElementJson.GetInt()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("l_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_EQ(int64Val, arrayElementJson.GetInt64()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("pt2d_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_TRUE(arrayElementJson.IsObject()) << "Column: " << colName;
                ASSERT_TRUE(arrayElementJson.HasMember("x")) << "Column: " << colName;
                ASSERT_DOUBLE_EQ(pt2dVal.x, arrayElementJson["x"].GetDouble()) << "Column: " << colName;
                ASSERT_TRUE(arrayElementJson.HasMember("y")) << "Column: " << colName;
                ASSERT_DOUBLE_EQ(pt2dVal.y, arrayElementJson["y"].GetDouble()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("pt3d_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_TRUE(arrayElementJson.IsObject()) << "Column: " << colName;
                ASSERT_TRUE(arrayElementJson.HasMember("x")) << "Column: " << colName;
                ASSERT_DOUBLE_EQ(pt3dVal.x, arrayElementJson["x"].GetDouble()) << "Column: " << colName;
                ASSERT_TRUE(arrayElementJson.HasMember("y")) << "Column: " << colName;
                ASSERT_DOUBLE_EQ(pt3dVal.y, arrayElementJson["y"].GetDouble()) << "Column: " << colName;
                ASSERT_TRUE(arrayElementJson.HasMember("z")) << "Column: " << colName;
                ASSERT_DOUBLE_EQ(pt3dVal.z, arrayElementJson["z"].GetDouble()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("s_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_STREQ(stringVal, arrayElementJson.GetString()) << "Column: " << colName;
                }
            }
        else
            ASSERT_TRUE(verifyStmt.IsColumnNull(i)) << "Column: " << colName;
        }
    ASSERT_EQ(BE_SQLITE_DONE, verifyStmt.Step());
    }

    {
    //insert row with prim values in struct
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MyClass(struct.bl,struct.b,struct.dt,struct.d,struct.g,struct.i,struct.l,struct.pt2d,struct.pt3d,struct.s) VALUES(?,?,?,?,?,?,?,?,?,?)"));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBlob(1, blobVal, blobSize, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindBoolean(2, boolVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDateTime(3, dtVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindDouble(4, doubleVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindGeometry(5, *geomVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt(6, intVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindInt64(7, int64Val));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint2d(8, pt2dVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindPoint3d(9, pt3dVal));
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(10, stringVal, IECSqlBinder::MakeCopy::No));
    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Reset();
    stmt.ClearBindings();

    Statement verifyStmt;
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.Prepare(m_ecdb, "SELECT * FROM ts_MyClass WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step());
    const int colCount = verifyStmt.GetColumnCount();
    ASSERT_EQ(48, colCount);
    for (int i = 0; i < colCount; i++)
        {
        Utf8CP colName = verifyStmt.GetColumnName(i);
        if (BeStringUtilities::StricmpAscii("Id", colName) == 0)
            ASSERT_EQ(key.GetInstanceId(), verifyStmt.GetValueId<ECInstanceId>(i));
        else if (BeStringUtilities::StricmpAscii("struct_bl", colName) == 0)
            ASSERT_EQ(0, memcmp(blobVal, verifyStmt.GetValueBlob(i), (size_t) blobSize)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("struct_b", colName) == 0)
            ASSERT_EQ(boolVal, verifyStmt.GetValueBoolean(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("struct_dt", colName) == 0)
            ASSERT_DOUBLE_EQ(jdVal, verifyStmt.GetValueDouble(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("struct_d", colName) == 0)
            ASSERT_DOUBLE_EQ(doubleVal, verifyStmt.GetValueDouble(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("struct_g", colName) == 0)
            ASSERT_EQ(0, memcmp(geometryBlobVal, verifyStmt.GetValueBlob(i), (size_t) geometryBlobSize)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("struct_i", colName) == 0)
            ASSERT_EQ(intVal, verifyStmt.GetValueInt(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("struct_l", colName) == 0)
            ASSERT_EQ(int64Val, verifyStmt.GetValueInt64(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("struct_pt2d_x", colName) == 0)
            ASSERT_DOUBLE_EQ(pt2dVal.x, verifyStmt.GetValueDouble(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("struct_pt2d_y", colName) == 0)
            ASSERT_DOUBLE_EQ(pt2dVal.y, verifyStmt.GetValueDouble(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("struct_pt3d_x", colName) == 0)
            ASSERT_DOUBLE_EQ(pt3dVal.x, verifyStmt.GetValueDouble(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("struct_pt3d_y", colName) == 0)
            ASSERT_DOUBLE_EQ(pt3dVal.y, verifyStmt.GetValueDouble(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("struct_pt3d_z", colName) == 0)
            ASSERT_DOUBLE_EQ(pt3dVal.z, verifyStmt.GetValueDouble(i)) << "Column: " << colName;
        else if (BeStringUtilities::StricmpAscii("struct_s", colName) == 0)
            ASSERT_STREQ(stringVal, verifyStmt.GetValueText(i)) << "Column: " << colName;
        else
            ASSERT_TRUE(verifyStmt.IsColumnNull(i)) << "Column: " << colName;
        }
    ASSERT_EQ(BE_SQLITE_DONE, verifyStmt.Step());
    }

    {
    //insert row with prim array values in struct
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MyClass(struct.bl_array,struct.b_array,struct.dt_array,struct.d_array,struct.g_array,struct.i_array,struct.l_array,struct.pt2d_array,struct.pt3d_array,struct.s_array) VALUES(?,?,?,?,?,?,?,?,?,?)"));
    IECSqlBinder& blArrayBinder = stmt.GetBinder(1);
    ASSERT_EQ(ECSqlStatus::Success, blArrayBinder.AddArrayElement().BindBlob(blobVal, blobSize, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, blArrayBinder.AddArrayElement().BindBlob(blobVal, blobSize, IECSqlBinder::MakeCopy::No));

    IECSqlBinder& bArrayBinder = stmt.GetBinder(2);
    ASSERT_EQ(ECSqlStatus::Success, bArrayBinder.AddArrayElement().BindBoolean(boolVal));
    ASSERT_EQ(ECSqlStatus::Success, bArrayBinder.AddArrayElement().BindBoolean(boolVal));

    IECSqlBinder& dtArrayBinder = stmt.GetBinder(3);
    ASSERT_EQ(ECSqlStatus::Success, dtArrayBinder.AddArrayElement().BindDateTime(dtVal));
    ASSERT_EQ(ECSqlStatus::Success, dtArrayBinder.AddArrayElement().BindDateTime(dtVal));

    IECSqlBinder& dArrayBinder = stmt.GetBinder(4);
    ASSERT_EQ(ECSqlStatus::Success, dArrayBinder.AddArrayElement().BindDouble(doubleVal));
    ASSERT_EQ(ECSqlStatus::Success, dArrayBinder.AddArrayElement().BindDouble(doubleVal));

    IECSqlBinder& gArrayBinder = stmt.GetBinder(5);
    ASSERT_EQ(ECSqlStatus::Success, gArrayBinder.AddArrayElement().BindGeometry(*geomVal));
    ASSERT_EQ(ECSqlStatus::Success, gArrayBinder.AddArrayElement().BindGeometry(*geomVal));

    IECSqlBinder& iArrayBinder = stmt.GetBinder(6);
    ASSERT_EQ(ECSqlStatus::Success, iArrayBinder.AddArrayElement().BindInt(intVal));
    ASSERT_EQ(ECSqlStatus::Success, iArrayBinder.AddArrayElement().BindInt(intVal));

    IECSqlBinder& lArrayBinder = stmt.GetBinder(7);
    ASSERT_EQ(ECSqlStatus::Success, lArrayBinder.AddArrayElement().BindInt64(int64Val));
    ASSERT_EQ(ECSqlStatus::Success, lArrayBinder.AddArrayElement().BindInt64(int64Val));

    IECSqlBinder& pt2dArrayBinder = stmt.GetBinder(8);
    ASSERT_EQ(ECSqlStatus::Success, pt2dArrayBinder.AddArrayElement().BindPoint2d(pt2dVal));
    ASSERT_EQ(ECSqlStatus::Success, pt2dArrayBinder.AddArrayElement().BindPoint2d(pt2dVal));

    IECSqlBinder& pt3dArrayBinder = stmt.GetBinder(9);
    ASSERT_EQ(ECSqlStatus::Success, pt3dArrayBinder.AddArrayElement().BindPoint3d(pt3dVal));
    ASSERT_EQ(ECSqlStatus::Success, pt3dArrayBinder.AddArrayElement().BindPoint3d(pt3dVal));

    IECSqlBinder& sArrayBinder = stmt.GetBinder(10);
    ASSERT_EQ(ECSqlStatus::Success, sArrayBinder.AddArrayElement().BindText(stringVal, IECSqlBinder::MakeCopy::No));
    ASSERT_EQ(ECSqlStatus::Success, sArrayBinder.AddArrayElement().BindText(stringVal, IECSqlBinder::MakeCopy::No));

    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Reset();
    stmt.ClearBindings();

    Statement verifyStmt;
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.Prepare(m_ecdb, "SELECT * FROM ts_MyClass WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step());
    const int colCount = verifyStmt.GetColumnCount();
    ASSERT_EQ(48, colCount);
    for (int i = 0; i < colCount; i++)
        {
        Utf8CP colName = verifyStmt.GetColumnName(i);
        if (BeStringUtilities::StricmpAscii("Id", colName) == 0)
            ASSERT_EQ(key.GetInstanceId(), verifyStmt.GetValueId<ECInstanceId>(i));
        else if (BeStringUtilities::StricmpAscii("struct_bl_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_STREQ(blobBase64Str.c_str(), arrayElementJson.GetString()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("struct_b_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_EQ(boolVal, arrayElementJson.GetBool()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("struct_dt_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_DOUBLE_EQ(jdVal, arrayElementJson.GetDouble()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("struct_d_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_DOUBLE_EQ(doubleVal, arrayElementJson.GetDouble()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("struct_g_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_STREQ(geomBlobBase64Str.c_str(), arrayElementJson.GetString()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("struct_i_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_EQ(intVal, arrayElementJson.GetInt()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("struct_l_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_EQ(int64Val, arrayElementJson.GetInt64()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("struct_pt2d_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_TRUE(arrayElementJson.IsObject()) << "Column: " << colName;
                ASSERT_TRUE(arrayElementJson.HasMember("x")) << "Column: " << colName;
                ASSERT_DOUBLE_EQ(pt2dVal.x, arrayElementJson["x"].GetDouble()) << "Column: " << colName;
                ASSERT_TRUE(arrayElementJson.HasMember("y")) << "Column: " << colName;
                ASSERT_DOUBLE_EQ(pt2dVal.y, arrayElementJson["y"].GetDouble()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("struct_pt3d_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_TRUE(arrayElementJson.IsObject()) << "Column: " << colName;
                ASSERT_TRUE(arrayElementJson.HasMember("x")) << "Column: " << colName;
                ASSERT_DOUBLE_EQ(pt3dVal.x, arrayElementJson["x"].GetDouble()) << "Column: " << colName;
                ASSERT_TRUE(arrayElementJson.HasMember("y")) << "Column: " << colName;
                ASSERT_DOUBLE_EQ(pt3dVal.y, arrayElementJson["y"].GetDouble()) << "Column: " << colName;
                ASSERT_TRUE(arrayElementJson.HasMember("z")) << "Column: " << colName;
                ASSERT_DOUBLE_EQ(pt3dVal.z, arrayElementJson["z"].GetDouble()) << "Column: " << colName;
                }
            }
        else if (BeStringUtilities::StricmpAscii("struct_s_array", colName) == 0)
            {
            rapidjson::Document actualJson;
            ASSERT_TRUE(!actualJson.Parse<0>(verifyStmt.GetValueText(i)).HasParseError()) << "Column: " << colName;
            ASSERT_TRUE(actualJson.IsArray()) << "Column: " << colName;
            ASSERT_EQ(2, (int) actualJson.GetArray().Size()) << "Column: " << colName;
            for (RapidJsonValueCR arrayElementJson : actualJson.GetArray())
                {
                ASSERT_TRUE(arrayElementJson.IsString()) << "Column: " << colName;
                ASSERT_STREQ(stringVal, arrayElementJson.GetString()) << "Column: " << colName;
                }
            }
        else
            ASSERT_TRUE(verifyStmt.IsColumnNull(i)) << "Column: " << colName;
        }
    ASSERT_EQ(BE_SQLITE_DONE, verifyStmt.Step());
    }

    {
    //insert row with struct array values
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "INSERT INTO ts.MyClass(struct_array) VALUES(?)"));
    IECSqlBinder& arrayBinder = stmt.GetBinder(1);
    for (int i = 0; i < 2; i++)
        {
        IECSqlBinder& structElementBinder = arrayBinder.AddArrayElement();
        ASSERT_EQ(ECSqlStatus::Success, structElementBinder["bl"].BindBlob(blobVal, blobSize, IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, structElementBinder["b"].BindBoolean(boolVal));
        ASSERT_EQ(ECSqlStatus::Success, structElementBinder["dt"].BindDateTime(dtVal));
        ASSERT_EQ(ECSqlStatus::Success, structElementBinder["d"].BindDouble(doubleVal));
        ASSERT_EQ(ECSqlStatus::Success, structElementBinder["g"].BindGeometry(*geomVal));
        ASSERT_EQ(ECSqlStatus::Success, structElementBinder["i"].BindInt(intVal));
        ASSERT_EQ(ECSqlStatus::Success, structElementBinder["l"].BindInt64(int64Val));
        ASSERT_EQ(ECSqlStatus::Success, structElementBinder["pt2d"].BindPoint2d(pt2dVal));
        ASSERT_EQ(ECSqlStatus::Success, structElementBinder["pt3d"].BindPoint3d(pt3dVal));
        ASSERT_EQ(ECSqlStatus::Success, structElementBinder["s"].BindText(stringVal, IECSqlBinder::MakeCopy::No));

        {
        IECSqlBinder& primArrayBinder = structElementBinder["bl_array"];
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindBlob(blobVal, blobSize, IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindBlob(blobVal, blobSize, IECSqlBinder::MakeCopy::No));
        }

        {
        IECSqlBinder& primArrayBinder = structElementBinder["b_array"];
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindBoolean(boolVal));
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindBoolean(boolVal));
        }

        {
        IECSqlBinder& primArrayBinder = structElementBinder["dt_array"];
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindDateTime(dtVal));
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindDateTime(dtVal));
        }

        {
        IECSqlBinder& primArrayBinder = structElementBinder["d_array"];
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindDouble(doubleVal));
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindDouble(doubleVal));
        }

        {
        IECSqlBinder& primArrayBinder = structElementBinder["g_array"];
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindGeometry(*geomVal));
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindGeometry(*geomVal));
        }

        {
        IECSqlBinder& primArrayBinder = structElementBinder["i_array"];
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindInt(intVal));
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindInt(intVal));
        }

        {
        IECSqlBinder& primArrayBinder = structElementBinder["l_array"];
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindInt64(int64Val));
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindInt64(int64Val));
        }

        {
        IECSqlBinder& primArrayBinder = structElementBinder["pt2d_array"];
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindPoint2d(pt2dVal));
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindPoint2d(pt2dVal));
        }

        {
        IECSqlBinder& primArrayBinder = structElementBinder["pt3d_array"];
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindPoint3d(pt3dVal));
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindPoint3d(pt3dVal));
        }

        {
        IECSqlBinder& primArrayBinder = structElementBinder["s_array"];
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindText(stringVal, IECSqlBinder::MakeCopy::No));
        ASSERT_EQ(ECSqlStatus::Success, primArrayBinder.AddArrayElement().BindText(stringVal, IECSqlBinder::MakeCopy::No));
        }

        }


    ECInstanceKey key;
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step(key));
    stmt.Reset();
    stmt.ClearBindings();

    Statement verifyStmt;
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.Prepare(m_ecdb, "SELECT struct_array FROM ts_MyClass WHERE Id=?"));
    ASSERT_EQ(BE_SQLITE_OK, verifyStmt.BindId(1, key.GetInstanceId()));
    ASSERT_EQ(BE_SQLITE_ROW, verifyStmt.Step());

    rapidjson::Document actualStructArrayJson;
    ASSERT_TRUE(!actualStructArrayJson.Parse<0>(verifyStmt.GetValueText(0)).HasParseError()) << "Column: struct_array";
    ASSERT_TRUE(actualStructArrayJson.IsArray()) << "Column: struct_array";
    ASSERT_EQ(2, (int) actualStructArrayJson.GetArray().Size()) << "Column: struct_array";
    for (RapidJsonValueCR structArrayElementJson : actualStructArrayJson.GetArray())
        {
        ASSERT_TRUE(structArrayElementJson.HasMember("bl"));
        ASSERT_STREQ(blobBase64Str.c_str(), structArrayElementJson["bl"].GetString());

        ASSERT_TRUE(structArrayElementJson.HasMember("b"));
        ASSERT_EQ(boolVal, structArrayElementJson["b"].GetBool());

        ASSERT_TRUE(structArrayElementJson.HasMember("dt"));
        ASSERT_DOUBLE_EQ(jdVal, structArrayElementJson["dt"].GetDouble());

        ASSERT_TRUE(structArrayElementJson.HasMember("d"));
        ASSERT_DOUBLE_EQ(doubleVal, structArrayElementJson["d"].GetDouble());

        ASSERT_TRUE(structArrayElementJson.HasMember("g"));
        ASSERT_STREQ(geomBlobBase64Str.c_str(), structArrayElementJson["g"].GetString());

        ASSERT_TRUE(structArrayElementJson.HasMember("i"));
        ASSERT_EQ(intVal, structArrayElementJson["i"].GetInt());

        ASSERT_TRUE(structArrayElementJson.HasMember("l"));
        ASSERT_EQ(int64Val, structArrayElementJson["l"].GetInt64());

        ASSERT_TRUE(structArrayElementJson.HasMember("pt2d"));
        ASSERT_TRUE(structArrayElementJson["pt2d"].IsObject());
        ASSERT_TRUE(structArrayElementJson["pt2d"].HasMember("x"));
        ASSERT_DOUBLE_EQ(pt2dVal.x, structArrayElementJson["pt2d"]["x"].GetDouble());
        ASSERT_TRUE(structArrayElementJson["pt2d"].HasMember("y"));
        ASSERT_DOUBLE_EQ(pt2dVal.y, structArrayElementJson["pt2d"]["y"].GetDouble());

        ASSERT_TRUE(structArrayElementJson.HasMember("pt3d"));
        ASSERT_TRUE(structArrayElementJson["pt3d"].IsObject());
        ASSERT_TRUE(structArrayElementJson["pt3d"].HasMember("x"));
        ASSERT_DOUBLE_EQ(pt3dVal.x, structArrayElementJson["pt3d"]["x"].GetDouble());
        ASSERT_TRUE(structArrayElementJson["pt3d"].HasMember("y"));
        ASSERT_DOUBLE_EQ(pt3dVal.y, structArrayElementJson["pt3d"]["y"].GetDouble());
        ASSERT_TRUE(structArrayElementJson["pt3d"].HasMember("z"));
        ASSERT_DOUBLE_EQ(pt3dVal.z, structArrayElementJson["pt3d"]["z"].GetDouble());

        ASSERT_TRUE(structArrayElementJson.HasMember("s"));
        ASSERT_STREQ(stringVal, structArrayElementJson["s"].GetString());

        ASSERT_TRUE(structArrayElementJson.HasMember("bl_array"));
        ASSERT_TRUE(structArrayElementJson["bl_array"].IsArray());
        ASSERT_EQ(2, (int) structArrayElementJson["bl_array"].GetArray().Size());
        for (RapidJsonValueCR primArrayElemJson : structArrayElementJson["bl_array"].GetArray())
            {
            ASSERT_TRUE(primArrayElemJson.IsString()) << "bl_array";
            ASSERT_STREQ(blobBase64Str.c_str(), primArrayElemJson.GetString()) << "bl_array";
            }

        ASSERT_TRUE(structArrayElementJson.HasMember("b_array"));
        ASSERT_TRUE(structArrayElementJson["b_array"].IsArray());
        ASSERT_EQ(2, (int) structArrayElementJson["b_array"].GetArray().Size());
        for (RapidJsonValueCR primArrayElemJson : structArrayElementJson["b_array"].GetArray())
            {
            ASSERT_TRUE(primArrayElemJson.IsBool()) << "b_array";
            ASSERT_EQ(boolVal, primArrayElemJson.GetBool()) << "b_array";
            }

        ASSERT_TRUE(structArrayElementJson.HasMember("dt_array"));
        ASSERT_TRUE(structArrayElementJson["dt_array"].IsArray());
        ASSERT_EQ(2, (int) structArrayElementJson["dt_array"].GetArray().Size());
        for (RapidJsonValueCR primArrayElemJson : structArrayElementJson["dt_array"].GetArray())
            {
            ASSERT_TRUE(primArrayElemJson.IsDouble()) << "dt_array";
            ASSERT_DOUBLE_EQ(jdVal, primArrayElemJson.GetDouble()) << "dt_array";
            }

        ASSERT_TRUE(structArrayElementJson.HasMember("d_array"));
        ASSERT_TRUE(structArrayElementJson["d_array"].IsArray());
        ASSERT_EQ(2, (int) structArrayElementJson["d_array"].GetArray().Size());
        for (RapidJsonValueCR primArrayElemJson : structArrayElementJson["d_array"].GetArray())
            {
            ASSERT_TRUE(primArrayElemJson.IsDouble()) << "d_array";
            ASSERT_DOUBLE_EQ(doubleVal, primArrayElemJson.GetDouble()) << "d_array";
            }

        ASSERT_TRUE(structArrayElementJson.HasMember("g_array"));
        ASSERT_TRUE(structArrayElementJson["g_array"].IsArray());
        ASSERT_EQ(2, (int) structArrayElementJson["g_array"].GetArray().Size());
        for (RapidJsonValueCR primArrayElemJson : structArrayElementJson["g_array"].GetArray())
            {
            ASSERT_TRUE(primArrayElemJson.IsString()) << "g_array";
            ASSERT_STREQ(geomBlobBase64Str.c_str(), primArrayElemJson.GetString()) << "g_array";
            }

        ASSERT_TRUE(structArrayElementJson.HasMember("i_array"));
        ASSERT_TRUE(structArrayElementJson["i_array"].IsArray());
        ASSERT_EQ(2, (int) structArrayElementJson["i_array"].GetArray().Size());
        for (RapidJsonValueCR primArrayElemJson : structArrayElementJson["i_array"].GetArray())
            {
            ASSERT_TRUE(primArrayElemJson.IsInt()) << "i_array";
            ASSERT_EQ(intVal, primArrayElemJson.GetInt()) << "i_array";
            }

        ASSERT_TRUE(structArrayElementJson.HasMember("l_array"));
        ASSERT_TRUE(structArrayElementJson["l_array"].IsArray());
        ASSERT_EQ(2, (int) structArrayElementJson["l_array"].GetArray().Size());
        for (RapidJsonValueCR primArrayElemJson : structArrayElementJson["l_array"].GetArray())
            {
            ASSERT_TRUE(primArrayElemJson.IsInt64()) << "l_array";
            ASSERT_EQ(int64Val, primArrayElemJson.GetInt64()) << "l_array";
            }

        ASSERT_TRUE(structArrayElementJson.HasMember("pt2d_array"));
        ASSERT_TRUE(structArrayElementJson["pt2d_array"].IsArray());
        ASSERT_EQ(2, (int) structArrayElementJson["pt2d_array"].GetArray().Size());
        for (RapidJsonValueCR primArrayElemJson : structArrayElementJson["pt2d_array"].GetArray())
            {
            ASSERT_TRUE(primArrayElemJson.IsObject()) << "pt2d_array";
            ASSERT_TRUE(primArrayElemJson.HasMember("x")) << "pt2d_array";
            ASSERT_DOUBLE_EQ(pt2dVal.x, primArrayElemJson["x"].GetDouble()) << "pt2d_array";
            ASSERT_TRUE(primArrayElemJson.HasMember("y")) << "pt2d_array";
            ASSERT_DOUBLE_EQ(pt2dVal.y, primArrayElemJson["y"].GetDouble()) << "pt2d_array";
            }

        ASSERT_TRUE(structArrayElementJson.HasMember("pt3d_array"));
        ASSERT_TRUE(structArrayElementJson["pt3d_array"].IsArray());
        ASSERT_EQ(2, (int) structArrayElementJson["pt3d_array"].GetArray().Size());
        for (RapidJsonValueCR primArrayElemJson : structArrayElementJson["pt3d_array"].GetArray())
            {
            ASSERT_TRUE(primArrayElemJson.IsObject()) << "pt3d_array";
            ASSERT_TRUE(primArrayElemJson.HasMember("x")) << "pt3d_array";
            ASSERT_DOUBLE_EQ(pt3dVal.x, primArrayElemJson["x"].GetDouble()) << "pt3d_array";
            ASSERT_TRUE(primArrayElemJson.HasMember("y")) << "pt3d_array";
            ASSERT_DOUBLE_EQ(pt3dVal.y, primArrayElemJson["y"].GetDouble()) << "pt3d_array";
            ASSERT_TRUE(primArrayElemJson.HasMember("z")) << "pt3d_array";
            ASSERT_DOUBLE_EQ(pt3dVal.z, primArrayElemJson["z"].GetDouble()) << "pt3d_array";
            }

        ASSERT_TRUE(structArrayElementJson.HasMember("s_array"));
        ASSERT_TRUE(structArrayElementJson["s_array"].IsArray());
        ASSERT_EQ(2, (int) structArrayElementJson["s_array"].GetArray().Size());
        for (RapidJsonValueCR primArrayElemJson : structArrayElementJson["s_array"].GetArray())
            {
            ASSERT_TRUE(primArrayElemJson.IsString()) << "s_array";
            ASSERT_STREQ(stringVal, primArrayElemJson.GetString()) << "s_array";
            }
        }
    ASSERT_EQ(BE_SQLITE_DONE, verifyStmt.Step());
    }
    }

//---------------------------------------------------------------------------------------
//* quick check whether schema import works for benchmark schemas.
//* Use this test to create a new benchmark file
// @bsiclass                                     Krischan.Eberle                  10/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, ImportSchemas)
    {
    ASSERT_EQ(SUCCESS, SetupTestFile("imodel2_4001.ecdb"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, CompareDdl_NewFile)
    {
    ASSERT_EQ(SUCCESS, SetupTestFile("imodel2fileformatcompatibilitytest.ecdb"));

    Db benchmarkFile;
    BeFileName benchmarkFilePath = GetBenchmarkFileFolder(ExpectedProfileVersion());
    benchmarkFilePath.AppendToPath(L"imodel2.ecdb");
    ASSERT_EQ(BE_SQLITE_OK, benchmarkFile.OpenBeSQLiteDb(benchmarkFilePath, Db::OpenParams(Db::OpenMode::Readonly))) << benchmarkFilePath.GetNameUtf8();

    BeFileName artefactOutDir;
    BeTest::GetHost().GetOutputRoot(artefactOutDir);

    BeFileStatus stat = BeFileStatus::Success;


    int benchmarkMasterTableRowCount = 0;
    {
    BeFileName benchmarkDdlDumpFilePath(artefactOutDir);
    benchmarkDdlDumpFilePath.AppendToPath(L"benchmarkddl.txt");

    BeTextFilePtr benchmarkDdlDumpFile = BeTextFile::Open(stat, benchmarkDdlDumpFilePath, TextFileOpenType::Write, TextFileOptions::KeepNewLine, TextFileEncoding::Utf8);
    ASSERT_EQ(BeFileStatus::Success, stat) << "Creating file " << benchmarkDdlDumpFilePath.GetNameUtf8();

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(benchmarkFile, "SELECT sql FROM sqlite_master ORDER BY name"));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        benchmarkMasterTableRowCount++;
        benchmarkDdlDumpFile->PutLine(WString(stmt.GetValueText(0), BentleyCharEncoding::Utf8).c_str(), true);
        }
    }


    BeFileName actualDdlDumpFilePath(artefactOutDir);
    actualDdlDumpFilePath.AppendToPath(L"actualddl.txt");
    BeTextFilePtr actualDdlDumpFile = BeTextFile::Open(stat, actualDdlDumpFilePath, TextFileOpenType::Write, TextFileOptions::KeepNewLine, TextFileEncoding::Utf8);
    ASSERT_EQ(BeFileStatus::Success, stat) << "Creating file " << actualDdlDumpFilePath.GetNameUtf8();

    Statement benchmarkDdlLookupStmt;
    ASSERT_EQ(BE_SQLITE_OK, benchmarkDdlLookupStmt.Prepare(benchmarkFile, "SELECT sql FROM sqlite_master WHERE name=?"));


    Statement actualDdlStmt;
    ASSERT_EQ(BE_SQLITE_OK, actualDdlStmt.Prepare(m_ecdb, "SELECT name, sql FROM sqlite_master ORDER BY name"));
    int actualMasterTableRowCount = 0;
    while (BE_SQLITE_ROW == actualDdlStmt.Step())
        {
        actualMasterTableRowCount++;
        Utf8CP actualName = actualDdlStmt.GetValueText(0);
        Utf8CP actualDdl = actualDdlStmt.GetValueText(1);

        actualDdlDumpFile->PutLine(WString(actualDdl, BentleyCharEncoding::Utf8).c_str(), true);

        benchmarkDdlLookupStmt.BindText(1, actualName, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW == benchmarkDdlLookupStmt.Step())
            {
            Utf8CP benchmarkDdl = benchmarkDdlLookupStmt.GetValueText(0);
            EXPECT_STREQ(benchmarkDdl, actualDdl) << "DB object in actual file has different DDL than in benchmark file: " << actualName;
            }
        else
            EXPECT_TRUE(false) << "DB object in benchmark file not found: " << actualName;

        benchmarkDdlLookupStmt.Reset();
        benchmarkDdlLookupStmt.ClearBindings();
        }

    ASSERT_EQ(benchmarkMasterTableRowCount, actualMasterTableRowCount) << benchmarkFilePath.GetNameUtf8();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, CompareDdl_UpgradedFile)
    {
    BeFileName benchmarkFilePath = GetBenchmarkFileFolder(InitialBim2ProfileVersion());
    benchmarkFilePath.AppendToPath(L"imodel2.ecdb");
    Db benchmarkFile;
    ASSERT_EQ(BE_SQLITE_OK, benchmarkFile.OpenBeSQLiteDb(benchmarkFilePath, Db::OpenParams(Db::OpenMode::Readonly))) << benchmarkFilePath.GetNameUtf8();

    BeFileName artefactOutDir;
    BeTest::GetHost().GetOutputRoot(artefactOutDir);
    if (!artefactOutDir.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(artefactOutDir));

    BeFileName upgradedFilePath(artefactOutDir);
    upgradedFilePath.AppendToPath(L"upgradedimodel2.ecdb");
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(benchmarkFilePath, upgradedFilePath));
    ECDb upgradedFile;
    ASSERT_EQ(BE_SQLITE_OK, upgradedFile.OpenBeSQLiteDb(upgradedFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));
    
    
    BeFileStatus stat = BeFileStatus::Success;
    
    int benchmarkMasterTableRowCount = 0;
    {
    BeFileName benchmarkDdlDumpFilePath(artefactOutDir);
    benchmarkDdlDumpFilePath.AppendToPath(L"benchmarkddl.txt");

    BeTextFilePtr benchmarkDdlDumpFile = BeTextFile::Open(stat, benchmarkDdlDumpFilePath, TextFileOpenType::Write, TextFileOptions::KeepNewLine, TextFileEncoding::Utf8);
    ASSERT_EQ(BeFileStatus::Success, stat) << "Creating file " << benchmarkDdlDumpFilePath.GetNameUtf8();

    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(benchmarkFile, "SELECT sql FROM sqlite_master ORDER BY name"));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        benchmarkMasterTableRowCount++;
        benchmarkDdlDumpFile->PutLine(WString(stmt.GetValueText(0), BentleyCharEncoding::Utf8).c_str(), true);
        }
    }


    BeFileName actualDdlDumpFilePath(artefactOutDir);
    actualDdlDumpFilePath.AppendToPath(L"upgradedfileddl.txt");
    BeTextFilePtr actualDdlDumpFile = BeTextFile::Open(stat, actualDdlDumpFilePath, TextFileOpenType::Write, TextFileOptions::KeepNewLine, TextFileEncoding::Utf8);
    ASSERT_EQ(BeFileStatus::Success, stat) << "Creating file " << actualDdlDumpFilePath.GetNameUtf8();

    Statement benchmarkDdlLookupStmt;
    ASSERT_EQ(BE_SQLITE_OK, benchmarkDdlLookupStmt.Prepare(benchmarkFile, "SELECT sql FROM sqlite_master WHERE name=?"));


    Statement actualDdlStmt;
    ASSERT_EQ(BE_SQLITE_OK, actualDdlStmt.Prepare(upgradedFile, "SELECT name, sql FROM sqlite_master ORDER BY name"));
    int actualMasterTableRowCount = 0;
    while (BE_SQLITE_ROW == actualDdlStmt.Step())
        {
        actualMasterTableRowCount++;
        Utf8CP actualName = actualDdlStmt.GetValueText(0);
        Utf8CP actualDdl = actualDdlStmt.GetValueText(1);

        actualDdlDumpFile->PutLine(WString(actualDdl, BentleyCharEncoding::Utf8).c_str(), true);

        benchmarkDdlLookupStmt.BindText(1, actualName, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW == benchmarkDdlLookupStmt.Step())
            {
            Utf8CP benchmarkDdl = benchmarkDdlLookupStmt.GetValueText(0);
            EXPECT_STREQ(benchmarkDdl, actualDdl) << "DB object in upgraded file has different DDL than in benchmark file: " << actualName;
            }
        else
            EXPECT_TRUE(false) << "DB object in upgraded file not found: " << actualName;

        benchmarkDdlLookupStmt.Reset();
        benchmarkDdlLookupStmt.ClearBindings();
        }

    ASSERT_EQ(benchmarkMasterTableRowCount, actualMasterTableRowCount) << benchmarkFilePath.GetNameUtf8();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, CompareProfileTables_NewFile)
    {
    ASSERT_EQ(SUCCESS, SetupTestFile("imodel2fileformatcompatibilitytest.ecdb"));

    Db benchmarkFile;
    BeFileName benchmarkFilePath = GetBenchmarkFileFolder(ExpectedProfileVersion());
    benchmarkFilePath.AppendToPath(L"imodel2.ecdb");
    ASSERT_EQ(BE_SQLITE_OK, benchmarkFile.OpenBeSQLiteDb(benchmarkFilePath, Db::OpenParams(Db::OpenMode::Readonly))) << benchmarkFilePath.GetNameUtf8();
    
    //profile table count check
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, R"sql(SELECT count(*) FROM sqlite_master WHERE name LIKE 'ec\_%' ESCAPE '\' ORDER BY name COLLATE NOCASE)sql"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetSql();
    ASSERT_EQ(20, stmt.GetValueInt(0)) << "ECDb profile table count";
    }

    //schema profile tables
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Schema", PROFILETABLE_SELECT_Schema));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_SchemaReference", PROFILETABLE_SELECT_SchemaReference));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Class", PROFILETABLE_SELECT_Class));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_ClassHasBaseClasses", PROFILETABLE_SELECT_ClassHasBaseClasses));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Enumeration", PROFILETABLE_SELECT_Enumeration));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_KindOfQuantity", PROFILETABLE_SELECT_KindOfQunatity));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_PropertyCategory", PROFILETABLE_SELECT_PropertyCategory));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Property", PROFILETABLE_SELECT_Property));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_RelationshipConstraint", PROFILETABLE_SELECT_RelationshipConstraint));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_RelationshipConstraintClass", PROFILETABLE_SELECT_RelationshipConstraintClass));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_CustomAttribute", PROFILETABLE_SELECT_CustomAttribute));

    //mapping profile tables
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_ClassMap", PROFILETABLE_SELECT_ClassMap));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_PropertyPath", PROFILETABLE_SELECT_PropertyPath));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_PropertyMap", PROFILETABLE_SELECT_PropertyMap));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Table", PROFILETABLE_SELECT_Table));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Column", PROFILETABLE_SELECT_Column));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Index", PROFILETABLE_SELECT_Index));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_IndexColumn", PROFILETABLE_SELECT_IndexColumn));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, CompareProfileTables_UpgradedFile)
    {
    BeFileName benchmarkFilePath = GetBenchmarkFileFolder(InitialBim2ProfileVersion());
    benchmarkFilePath.AppendToPath(L"imodel2.ecdb");
    Db benchmarkFile;
    ASSERT_EQ(BE_SQLITE_OK, benchmarkFile.OpenBeSQLiteDb(benchmarkFilePath, Db::OpenParams(Db::OpenMode::Readonly))) << benchmarkFilePath.GetNameUtf8();

    BeFileName artefactOutDir;
    BeTest::GetHost().GetOutputRoot(artefactOutDir);
    if (!artefactOutDir.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(artefactOutDir));

    BeFileName upgradedFilePath(artefactOutDir);
    upgradedFilePath.AppendToPath(L"upgradedimodel2.ecdb");
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(benchmarkFilePath, upgradedFilePath));
    ECDb upgradedFile;
    ASSERT_EQ(BE_SQLITE_OK, upgradedFile.OpenBeSQLiteDb(upgradedFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));

    //profile table count check
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(upgradedFile, R"sql(SELECT count(*) FROM sqlite_master WHERE name LIKE 'ec\_%' ESCAPE '\' ORDER BY name COLLATE NOCASE)sql"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetSql();
    ASSERT_EQ(20, stmt.GetValueInt(0)) << "ECDb profile table count";
    }

    //schema profile tables
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_Schema", PROFILETABLE_SELECT_Schema, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_SchemaReference", PROFILETABLE_SELECT_SchemaReference, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_Class", PROFILETABLE_SELECT_Class, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_ClassHasBaseClasses", PROFILETABLE_SELECT_ClassHasBaseClasses, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_Enumeration", PROFILETABLE_SELECT_Enumeration, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_KindOfQuantity", PROFILETABLE_SELECT_KindOfQunatity, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_PropertyCategory", PROFILETABLE_SELECT_PropertyCategory, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_Property", PROFILETABLE_SELECT_Property, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_RelationshipConstraint", PROFILETABLE_SELECT_RelationshipConstraint, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_RelationshipConstraintClass", PROFILETABLE_SELECT_RelationshipConstraintClass, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_CustomAttribute", PROFILETABLE_SELECT_CustomAttribute, CompareOptions::DoNotComparePk));

    //mapping profile tables
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_ClassMap", PROFILETABLE_SELECT_ClassMap, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_PropertyPath", PROFILETABLE_SELECT_PropertyPath, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_PropertyMap", PROFILETABLE_SELECT_PropertyMap, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_Table", PROFILETABLE_SELECT_Table, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_Column", PROFILETABLE_SELECT_Column, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_Index", PROFILETABLE_SELECT_Index, CompareOptions::DoNotComparePk));
    EXPECT_TRUE(CompareTable(benchmarkFile, upgradedFile, "ec_IndexColumn", PROFILETABLE_SELECT_IndexColumn, CompareOptions::DoNotComparePk));
    }

//*****************************************************************************************
// FileFormatCompatibilityTests
//*****************************************************************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                     Krischan.Eberle     10/2017
//+---------------+---------------+---------------+---------------+---------------+------
//no need to release a static non-POD variable (Bentley C++ coding standards)
//static
ProfileVersion const* FileFormatCompatibilityTests::s_initialBim2ProfileVersion = new ProfileVersion(4, 0, 0, 0);

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
//static
bool FileFormatCompatibilityTests::CompareTable(DbCR benchmarkFile, DbCR actualFile, Utf8CP tableName, Utf8CP selectSql, CompareOptions options)
    {
    //compare row count
            {
            Utf8String sql;
            sql.Sprintf("SELECT count(*) from %s", tableName);

            Statement benchmarkStmt;
            if (BE_SQLITE_OK != benchmarkStmt.Prepare(benchmarkFile, sql.c_str()) ||
                BE_SQLITE_ROW != benchmarkStmt.Step())
                {
                return false;
                }

            const int benchmarkRowCount = benchmarkStmt.GetValueInt(0);

            Statement actualStmt;
            if (BE_SQLITE_OK != actualStmt.Prepare(actualFile, sql.c_str()) ||
                BE_SQLITE_ROW != actualStmt.Step())
                {
                return false;
                }

            const int actualRowCount = actualStmt.GetValueInt(0);
            EXPECT_EQ(benchmarkRowCount, actualRowCount) << tableName;
            }

    Utf8String pkName = GetPkColumnName(benchmarkFile, tableName);
    EXPECT_FALSE(pkName.empty()) << "Tables in this test are expected to always have a PK";

    Statement benchmarkTableStmt, actualTableStmt;
    if (BE_SQLITE_OK != benchmarkTableStmt.Prepare(benchmarkFile, selectSql) ||
        BE_SQLITE_OK != actualTableStmt.Prepare(actualFile, selectSql))
        return false;

    int rowCount = 0;
    while (BE_SQLITE_ROW == benchmarkTableStmt.Step())
        {
        if (BE_SQLITE_ROW != actualTableStmt.Step())
            return false;

        rowCount++;

        const int colCount = benchmarkTableStmt.GetColumnCount();
        if (colCount != actualTableStmt.GetColumnCount())
            return false;

        for (int i = 0; i < colCount; i++)
            {
            Utf8CP colName = benchmarkTableStmt.GetColumnName(i);

            const DbValueType benchmarkColType = benchmarkTableStmt.GetColumnType(i);
            const DbValueType actualColType = actualTableStmt.GetColumnType(i);
            EXPECT_EQ(benchmarkColType, actualColType) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;

            if (options == CompareOptions::DoNotComparePk && pkName.EqualsIAscii(colName))
                continue;

            switch (benchmarkColType)
                {
                    case DbValueType::NullVal:
                    {
                    if (benchmarkTableStmt.IsColumnNull(i) != actualTableStmt.IsColumnNull(i))
                        {
                        EXPECT_EQ(benchmarkTableStmt.IsColumnNull(i), actualTableStmt.IsColumnNull(i)) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        return false;
                        }

                    break;
                    }

                    case DbValueType::BlobVal:
                    {
                    const int benchmarkBlobSize = benchmarkTableStmt.GetColumnBytes(i);
                    const int actualBlobSize = actualTableStmt.GetColumnBytes(i);
                    if (benchmarkBlobSize != actualBlobSize)
                        {
                        EXPECT_EQ(benchmarkBlobSize, actualBlobSize) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        return false;
                        }

                    void const* benchmarkValue = benchmarkTableStmt.GetValueBlob(i);
                    void const* actualValue = actualTableStmt.GetValueBlob(i);
                    if (0 != memcmp(benchmarkValue, actualValue, (size_t) benchmarkBlobSize))
                        {
                        EXPECT_EQ(0, memcmp(benchmarkValue, actualValue, (size_t) benchmarkBlobSize)) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        return false;
                        }
                    break;
                    }

                    case DbValueType::FloatVal:
                    {
                    double benchmarkValue = benchmarkTableStmt.GetValueDouble(i);
                    double actualValue = actualTableStmt.GetValueDouble(i);

                    if (fabs(benchmarkValue - actualValue) > BeNumerical::ComputeComparisonTolerance(benchmarkValue, actualValue))
                        {
                        EXPECT_DOUBLE_EQ(benchmarkValue, actualValue) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        return false;
                        }
                    break;
                    }

                    case DbValueType::IntegerVal:
                    {
                    int64_t benchmarkValue = benchmarkTableStmt.GetValueInt64(i);
                    int64_t actualValue = actualTableStmt.GetValueInt64(i);

                    if (benchmarkValue != actualValue)
                        {
                        EXPECT_EQ(benchmarkValue, actualValue) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        return false;
                        }
                    break;
                    }

                    case DbValueType::TextVal:
                    {
                    Utf8CP benchmarkValue = benchmarkTableStmt.GetValueText(i);
                    Utf8CP actualValue = actualTableStmt.GetValueText(i);

                    if (0 != strcmp(benchmarkValue, actualValue))
                        {
                        EXPECT_STREQ(benchmarkValue, actualValue) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                        return false;
                        }

                    break;
                    }

                    default:
                        return false;
                }
            }
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
BentleyStatus FileFormatCompatibilityTests::SetupTestFile(Utf8CP fileName)
    {
    BeFileName benchmarkSchemaFolder = GetBenchmarkSchemaFolder();

    BeFileName bisSchemaFolder(benchmarkSchemaFolder);
    bisSchemaFolder.AppendToPath(L"dgndb");

    BeFileName domainSchemaFolder(benchmarkSchemaFolder);
    domainSchemaFolder.AppendToPath(L"domains");

    if (SUCCESS != CreateFakeBimFile(fileName, bisSchemaFolder))
        return ERROR;

    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    if (BE_SQLITE_OK != OpenECDb(filePath))
        return ERROR;

    PERFLOG_START("ECDb ATP", "BIS domain schema import");
    const BentleyStatus stat = ImportSchemasFromFolder(domainSchemaFolder);
    PERFLOG_FINISH("ECDb ATP", "BIS domain schema import");

    if (SUCCESS != stat)
        return ERROR;

    if (BE_SQLITE_OK != ReopenECDb())
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
BentleyStatus FileFormatCompatibilityTests::CreateFakeBimFile(Utf8CP fileName, BeFileNameCR bisSchemaFolder)
    {
    if (BE_SQLITE_OK != SetupECDb(fileName))
        return ERROR;

    BeFileName filePath(m_ecdb.GetDbFileName());
    CloseECDb();

    if (BE_SQLITE_OK != OpenECDb(filePath))
        return ERROR;

    //BIS ECSchema needs this table to pre-exist
    if (BE_SQLITE_OK != m_ecdb.ExecuteSql("CREATE VIRTUAL TABLE dgn_SpatialIndex USING rtree(ElementId,MinX,MaxX,MinY,MaxY,MinZ,MaxZ)"))
        return ERROR;

    PERFLOG_START("ECDb ATP", "BIS schema import");
    const BentleyStatus stat = ImportSchemasFromFolder(bisSchemaFolder);
    PERFLOG_FINISH("ECDb ATP", "BIS schema import");
    return stat;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2017
//---------------------------------------------------------------------------------------
BentleyStatus FileFormatCompatibilityTests::ImportSchemasFromFolder(BeFileName const& schemaFolder)
    {
    ECSchemaReadContextPtr ctx = ECSchemaReadContext::CreateContext(false, true);
    ctx->AddSchemaLocater(m_ecdb.GetSchemaLocater());
    ctx->AddSchemaPath(schemaFolder);

    BeFileName ecdbSchemaSearchPath;
    BeTest::GetHost().GetDgnPlatformAssetsDirectory(ecdbSchemaSearchPath);
    ecdbSchemaSearchPath.AppendToPath(L"ECSchemas").AppendToPath(L"ECDb");
    ctx->AddSchemaPath(ecdbSchemaSearchPath);

    bvector<BeFileName> schemaPaths;
    BeDirectoryIterator::WalkDirsAndMatch(schemaPaths, schemaFolder, L"*.ecschema.xml", false);

    if (schemaPaths.empty())
        return ERROR;

    for (BeFileName const& schemaXmlFile : schemaPaths)
        {
        ECN::ECSchemaPtr ecSchema = nullptr;
        const SchemaReadStatus stat = ECN::ECSchema::ReadFromXmlFile(ecSchema, schemaXmlFile.GetName(), *ctx);
        //duplicate schema error is ok, as the ReadFromXmlFile reads schema references implicitly.
        if (SchemaReadStatus::Success != stat && SchemaReadStatus::DuplicateSchema != stat)
            return ERROR;
        }

    if (SUCCESS != m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas()))
        {
        m_ecdb.AbandonChanges();
        return ERROR;
        }

    m_ecdb.SaveChanges();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      10/2017
//---------------------------------------------------------------------------------------
//static
Utf8String FileFormatCompatibilityTests::GetPkColumnName(DbCR db, Utf8CP tableName)
    {
    CachedStatementPtr stmt = db.GetCachedStatement(Utf8PrintfString("pragma table_info('%s')", tableName).c_str());
    if (stmt == nullptr)
        {
        EXPECT_TRUE(stmt != nullptr);
        return Utf8String();
        }

    while (BE_SQLITE_ROW == stmt->Step())
        {
        EXPECT_EQ(0, BeStringUtilities::StricmpAscii(stmt->GetColumnName(5), "pk"));
        const int pkOrdinal = stmt->GetValueInt(5); //PK column ordinals returned by this pragma are 1-based as 0 indicates "not a PK col"

        if (pkOrdinal == 1) //1 indicates first PK column. This method expects ECDb tables always have a single column PK.
            {
            EXPECT_EQ(0, BeStringUtilities::StricmpAscii(stmt->GetColumnName(1), "name") );
            return stmt->GetValueText(1);
            }
        }

    return Utf8String();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      07/2017
//---------------------------------------------------------------------------------------
//static
BeFileName FileFormatCompatibilityTests::GetBenchmarkFileFolder(ProfileVersion const& profileVersion)
    {
    BeFileName benchmarkFilesDir;
    BeTest::GetHost().GetDocumentsRoot(benchmarkFilesDir);
    benchmarkFilesDir.AppendToPath(L"ECDb").AppendToPath(L"fileformatbenchmark");

    Utf8String versionFolderName;
    versionFolderName.Sprintf("%" PRIu16 "%" PRIu16 "%" PRIu16 "%" PRIu16, profileVersion.GetMajor(), profileVersion.GetMinor(),
                              profileVersion.GetSub1(), profileVersion.GetSub2());

    benchmarkFilesDir.AppendToPath(WString(versionFolderName.c_str(), BentleyCharEncoding::Utf8).c_str());
    return benchmarkFilesDir;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      10/2017
//---------------------------------------------------------------------------------------
//static
BeFileName FileFormatCompatibilityTests::GetBenchmarkSchemaFolder()
    {
    BeFileName dir;
    BeTest::GetHost().GetDocumentsRoot(dir);
    dir.AppendToPath(L"ECDb").AppendToPath(L"fileformatbenchmark").AppendToPath(L"schemas");
    return dir;
    }

END_ECDBUNITTESTS_NAMESPACE

