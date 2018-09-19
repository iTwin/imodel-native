/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/FileFormatCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
            DoNotComparePk = 1,
            IgnoreDescriptions = 2
            };

    private:
        static ProfileVersion const* s_initialBim2ProfileVersion;

        BentleyStatus CreateFakeBimFile(Utf8CP fileName, BeFileNameCR bisSchemaFolder);
        BentleyStatus ImportSchemasFromFolder(BeFileName const& schemaFolder);

    protected:
        BentleyStatus SetupTestFile(Utf8CP fileName);

        static DbResult IncrementProfileVersion(DbR);

        static bool CompareTable(DbCR benchmark, DbCR actual, Utf8CP tableName, Utf8CP selectSql, CompareOptions options = CompareOptions::None);

        static void AssertMetaSchemaEnumeration(ECDbCR, Utf8CP schemaName, Utf8CP enumName);
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
// The profile upgrade uses ECEnumerator::DetermineName to update pre EC3.2 enumerators
// This test is a safe-guard to ensure that the logic of that method does not change
// @bsiclass                                     Krischan.Eberle                  01/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, ECEnumUpgrade)
    {
    int32_t val = 123;

    EXPECT_STREQ("myvalue", ECEnumerator::DetermineName("myenum", "myvalue", nullptr).c_str());
    EXPECT_STREQ("myValue", ECEnumerator::DetermineName("myEnum", "myValue", nullptr).c_str());
    EXPECT_STREQ("my__x0020__Value", ECEnumerator::DetermineName("myEnum", "my Value", nullptr).c_str());
    EXPECT_STREQ("__x0031__", ECEnumerator::DetermineName("myenum", "1", nullptr).c_str());

    EXPECT_STREQ("myvalue", ECEnumerator::DetermineName("myenum", "myvalue", &val).c_str());
    EXPECT_STREQ("myValue", ECEnumerator::DetermineName("myEnum", "myValue", &val).c_str());
    EXPECT_STREQ("my__x0020__Value", ECEnumerator::DetermineName("myEnum", "my Value", &val).c_str());
    EXPECT_STREQ("__x0031__", ECEnumerator::DetermineName("myenum", "1", &val).c_str());

    EXPECT_STREQ("myenum123", ECEnumerator::DetermineName("myenum", nullptr, &val).c_str());
    EXPECT_STREQ("myEnum123", ECEnumerator::DetermineName("myEnum", nullptr, &val).c_str());
    val = -1;
    EXPECT_STREQ("myEnum__x002D__1", ECEnumerator::DetermineName("myEnum", nullptr, &val).c_str());

    val = 123;
    EXPECT_STREQ("my__x0020__enum123", ECEnumerator::DetermineName("my enum", nullptr, &val).c_str());
    EXPECT_STREQ("my__x0020__Enum123", ECEnumerator::DetermineName("my Enum", nullptr, &val).c_str());
    val = -1;
    EXPECT_STREQ("my__x0020__Enum__x002D__1", ECEnumerator::DetermineName("my Enum", nullptr, &val).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, PreEC32Enums)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("PreEC32Enums.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEnumeration typeName="IntEnumNoDisplayLabel" description="1" backingTypeName="int" >
                    <ECEnumerator value="0" />
                    <ECEnumerator value="1" />
                </ECEnumeration>
                <ECEnumeration typeName="IntEnumWithDisplayLabel" description="2" backingTypeName="int" >
                    <ECEnumerator value="0" displayLabel="Turn On"/>
                    <ECEnumerator value="1" displayLabel="Turn Off"/>
                </ECEnumeration>
                <ECEnumeration typeName="StringEnumNoDisplayLabel" description="3" backingTypeName="string" >
                    <ECEnumerator value="On" />
                    <ECEnumerator value="Off" />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnumWithDisplayLabel" description="4" backingTypeName="string" >
                    <ECEnumerator value="On" displayLabel="Turn On" />
                    <ECEnumerator value="Off" displayLabel="Turn Off" />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnumNonECNameValueNoDisplayLabel" description="5" backingTypeName="string" >
                    <ECEnumerator value="Turn On"  />
                    <ECEnumerator value="Turn Off"  />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnumNonECNameValueWithDisplayLabel" description="6" backingTypeName="string" >
                    <ECEnumerator value="Turn On"  displayLabel="Turn Me On"/>
                    <ECEnumerator value="Turn Off" displayLabel="Turn Me Off" />
                </ECEnumeration>
        </ECSchema>)xml")));

    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT e.Name,e.EnumValues FROM ec_Enumeration e JOIN ec_Schema s ON e.SchemaId=s.Id WHERE s.Name='TestSchema' ORDER BY e.Description"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(JsonValue(R"json([{"Name":"IntEnumNoDisplayLabel0","IntValue":0},{"Name":"IntEnumNoDisplayLabel1","IntValue":1}])json"), JsonValue(stmt.GetValueText(1))) << stmt.GetValueText(0);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(JsonValue(R"json([{"Name":"IntEnumWithDisplayLabel0", "IntValue":0, "DisplayLabel":"Turn On"},{"Name":"IntEnumWithDisplayLabel1", "IntValue":1, "DisplayLabel":"Turn Off"}])json"), JsonValue(stmt.GetValueText(1))) << stmt.GetValueText(0);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(JsonValue(R"json([{"Name":"On", "StringValue":"On"},{"Name":"Off","StringValue":"Off"}])json"), JsonValue(stmt.GetValueText(1))) << stmt.GetValueText(0);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(JsonValue(R"json([{"Name":"On", "StringValue":"On", "DisplayLabel":"Turn On"},{"Name":"Off", "StringValue":"Off", "DisplayLabel":"Turn Off"}])json"), JsonValue(stmt.GetValueText(1))) << stmt.GetValueText(0);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(JsonValue(R"json([{"Name":"Turn__x0020__On", "StringValue":"Turn On"},{"Name":"Turn__x0020__Off","StringValue":"Turn Off"}])json"), JsonValue(stmt.GetValueText(1))) << stmt.GetValueText(0);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(JsonValue(R"json([{"Name":"Turn__x0020__On", "StringValue":"Turn On", "DisplayLabel":"Turn Me On"},{"Name":"Turn__x0020__Off", "StringValue":"Turn Off", "DisplayLabel":"Turn Me Off"}])json"), JsonValue(stmt.GetValueText(1))) << stmt.GetValueText(0);
    }

    {
    ECEnumerationCP ecenum = m_ecdb.Schemas().GetEnumeration("TestSchema", "IntEnumNoDisplayLabel");
    ASSERT_TRUE(ecenum != nullptr);
    ECEnumeratorCP enumValue = ecenum->FindEnumerator(0);
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("IntEnumNoDisplayLabel0", enumValue->GetName().c_str());
    ASSERT_EQ(0, enumValue->GetInteger());
    ASSERT_FALSE(enumValue->GetIsDisplayLabelDefined());
    enumValue = ecenum->FindEnumerator(1);
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_EQ(1, enumValue->GetInteger());
    ASSERT_STREQ("IntEnumNoDisplayLabel1", enumValue->GetName().c_str());
    ASSERT_FALSE(enumValue->GetIsDisplayLabelDefined());

    AssertMetaSchemaEnumeration(m_ecdb,"TestSchema", "IntEnumNoDisplayLabel");
    }

    {
    ECEnumerationCP ecenum = m_ecdb.Schemas().GetEnumeration("TestSchema", "IntEnumWithDisplayLabel");
    ASSERT_TRUE(ecenum != nullptr);
    ECEnumeratorCP enumValue = ecenum->FindEnumerator(0);
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("IntEnumWithDisplayLabel0", enumValue->GetName().c_str());
    ASSERT_EQ(0, enumValue->GetInteger());
    ASSERT_TRUE(enumValue->GetIsDisplayLabelDefined());
    ASSERT_STREQ("Turn On", enumValue->GetDisplayLabel().c_str());
    enumValue = ecenum->FindEnumerator(1);
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("IntEnumWithDisplayLabel1", enumValue->GetName().c_str());
    ASSERT_EQ(1, enumValue->GetInteger());
    ASSERT_TRUE(enumValue->GetIsDisplayLabelDefined());
    ASSERT_STREQ("Turn Off", enumValue->GetDisplayLabel().c_str());

    AssertMetaSchemaEnumeration(m_ecdb, "TestSchema", "IntEnumWithDisplayLabel");

    }

    {
    ECEnumerationCP ecenum = m_ecdb.Schemas().GetEnumeration("TestSchema", "StringEnumNoDisplayLabel");
    ASSERT_TRUE(ecenum != nullptr);
    ECEnumeratorCP enumValue = ecenum->FindEnumerator("On");
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("On", enumValue->GetName().c_str());
    ASSERT_STREQ("On", enumValue->GetString().c_str());
    ASSERT_FALSE(enumValue->GetIsDisplayLabelDefined());
    enumValue = ecenum->FindEnumerator("Off");
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("Off", enumValue->GetName().c_str());
    ASSERT_STREQ("Off", enumValue->GetString().c_str());
    ASSERT_FALSE(enumValue->GetIsDisplayLabelDefined());

    AssertMetaSchemaEnumeration(m_ecdb, "TestSchema", "StringEnumNoDisplayLabel");

    }

    {
    ECEnumerationCP ecenum = m_ecdb.Schemas().GetEnumeration("TestSchema", "StringEnumWithDisplayLabel");
    ASSERT_TRUE(ecenum != nullptr);
    ECEnumeratorCP enumValue = ecenum->FindEnumerator("On");
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("On", enumValue->GetName().c_str());
    ASSERT_STREQ("On", enumValue->GetString().c_str());
    ASSERT_TRUE(enumValue->GetIsDisplayLabelDefined());
    ASSERT_STREQ("Turn On", enumValue->GetDisplayLabel().c_str());
    enumValue = ecenum->FindEnumerator("Off");
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("Off", enumValue->GetName().c_str());
    ASSERT_STREQ("Off", enumValue->GetString().c_str());
    ASSERT_TRUE(enumValue->GetIsDisplayLabelDefined());
    ASSERT_STREQ("Turn Off", enumValue->GetDisplayLabel().c_str());

    AssertMetaSchemaEnumeration(m_ecdb, "TestSchema", "StringEnumWithDisplayLabel");

    }

    {
    ECEnumerationCP ecenum = m_ecdb.Schemas().GetEnumeration("TestSchema", "StringEnumNonECNameValueNoDisplayLabel");
    ASSERT_TRUE(ecenum != nullptr);
    ECEnumeratorCP enumValue = ecenum->FindEnumerator("Turn On");
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("Turn__x0020__On", enumValue->GetName().c_str());
    ASSERT_STREQ("Turn On", enumValue->GetString().c_str());
    ASSERT_FALSE(enumValue->GetIsDisplayLabelDefined());
    enumValue = ecenum->FindEnumerator("Turn Off");
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("Turn__x0020__Off", enumValue->GetName().c_str());
    ASSERT_STREQ("Turn Off", enumValue->GetString().c_str());
    ASSERT_FALSE(enumValue->GetIsDisplayLabelDefined());

    AssertMetaSchemaEnumeration(m_ecdb, "TestSchema", "StringEnumNonECNameValueNoDisplayLabel");
    }

    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, PreEC32EnumsWithSchemaUpgrade)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("PreEC32EnumsWithSchemaUpgrade.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                    <ECEnumerator value="0" />
                    <ECEnumerator value="1" />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                    <ECEnumerator value="On" />
                    <ECEnumerator value="Off" />
                </ECEnumeration>
                </ECSchema>)xml")));

    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT e.Name,e.EnumValues FROM ec_Enumeration e JOIN ec_Schema s ON e.SchemaId=s.Id WHERE s.Name='TestSchema' ORDER BY e.Name"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(JsonValue(R"json([{"Name":"IntEnum0","IntValue":0},{"Name":"IntEnum1","IntValue":1}])json"), JsonValue(stmt.GetValueText(1))) << stmt.GetValueText(0);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(JsonValue(R"json([{"Name":"On", "StringValue":"On"},{"Name":"Off","StringValue":"Off"}])json"), JsonValue(stmt.GetValueText(1))) << stmt.GetValueText(0);
    }

    {
    ECEnumerationCP ecenum = m_ecdb.Schemas().GetEnumeration("TestSchema", "IntEnum");
    ASSERT_TRUE(ecenum != nullptr);
    ECEnumeratorCP enumValue = ecenum->FindEnumerator(0);
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("IntEnum0", enumValue->GetName().c_str());
    ASSERT_EQ(0, enumValue->GetInteger());
    enumValue = ecenum->FindEnumerator(1);
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_EQ(1, enumValue->GetInteger());
    ASSERT_STREQ("IntEnum1", enumValue->GetName().c_str());

    AssertMetaSchemaEnumeration(m_ecdb, "TestSchema", "IntEnum");
    }

    {
    ECEnumerationCP ecenum = m_ecdb.Schemas().GetEnumeration("TestSchema", "StringEnum");
    ASSERT_TRUE(ecenum != nullptr);
    ECEnumeratorCP enumValue = ecenum->FindEnumerator("On");
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("On", enumValue->GetName().c_str());
    ASSERT_STREQ("On", enumValue->GetString().c_str());
    enumValue = ecenum->FindEnumerator("Off");
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("Off", enumValue->GetName().c_str());
    ASSERT_STREQ("Off", enumValue->GetString().c_str());

    AssertMetaSchemaEnumeration(m_ecdb, "TestSchema", "StringEnum");

    }

    // now run schema upgrade that modifies the names
    ASSERT_EQ(SUCCESS, GetHelper().ImportSchema(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8" ?>
              <ECSchema schemaName="TestSchema" alias="ts" version="1.0.1" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECEnumeration typeName="IntEnum" backingTypeName="int" >
                    <ECEnumerator name="On" value="0" />
                    <ECEnumerator name="Off" value="1" />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnum" backingTypeName="string" >
                    <ECEnumerator name="On" value="On" />
                    <ECEnumerator name="Off" value="Off" />
                </ECEnumeration>
                </ECSchema>)xml")));

    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT e.Name,e.EnumValues FROM ec_Enumeration e JOIN ec_Schema s ON e.SchemaId=s.Id WHERE s.Name='TestSchema' ORDER BY e.Name"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(JsonValue(R"json([{"Name":"On","IntValue":0},{"Name":"Off","IntValue":1}])json"), JsonValue(stmt.GetValueText(1))) << stmt.GetValueText(0);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    ASSERT_EQ(JsonValue(R"json([{"Name":"On", "StringValue":"On"},{"Name":"Off","StringValue":"Off"}])json"), JsonValue(stmt.GetValueText(1))) << stmt.GetValueText(0);
    }

    {
    ECEnumerationCP ecenum = m_ecdb.Schemas().GetEnumeration("TestSchema", "IntEnum");
    ASSERT_TRUE(ecenum != nullptr);
    ECEnumeratorCP enumValue = ecenum->FindEnumerator(0);
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("On", enumValue->GetName().c_str());
    ASSERT_EQ(0, enumValue->GetInteger());
    enumValue = ecenum->FindEnumerator(1);
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_EQ(1, enumValue->GetInteger());
    ASSERT_STREQ("Off", enumValue->GetName().c_str());

    AssertMetaSchemaEnumeration(m_ecdb, "TestSchema", "IntEnum");
    }

    {
    ECEnumerationCP ecenum = m_ecdb.Schemas().GetEnumeration("TestSchema", "StringEnum");
    ASSERT_TRUE(ecenum != nullptr);
    ECEnumeratorCP enumValue = ecenum->FindEnumerator("On");
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("On", enumValue->GetName().c_str());
    ASSERT_STREQ("On", enumValue->GetString().c_str());
    enumValue = ecenum->FindEnumerator("Off");
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("Off", enumValue->GetName().c_str());
    ASSERT_STREQ("Off", enumValue->GetString().c_str());

    AssertMetaSchemaEnumeration(m_ecdb, "TestSchema", "StringEnum");

    }
    }

//---------------------------------------------------------------------------------------
//* quick check whether schema import works for benchmark schemas.
//* Use this test to create a new benchmark file
// @bsiclass                                     Krischan.Eberle                  10/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, ImportSchemas)
    {
    ASSERT_EQ(SUCCESS, SetupTestFile("imodel2.ecdb"));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, CompareDdl_NewFile)
    {
    ASSERT_EQ(SUCCESS, SetupTestFile("imodel2fileformatcompatibility_newfile_test.ecdb"));

    Db benchmarkFile;
    BeFileName benchmarkFilePath = GetBenchmarkFileFolder(ECDb::CurrentECDbProfileVersion());
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
            if (BeStringUtilities::StricmpAscii(actualName,"bis_Element_CurrentTimeStamp") == 0)
                EXPECT_STREQ("CREATE TRIGGER [bis_Element_CurrentTimeStamp] AFTER UPDATE ON [bis_Element] WHEN old.[LastMod]=new.[LastMod] AND old.[LastMod]!=julianday('now') BEGIN UPDATE [bis_Element] SET [LastMod]=julianday('now') WHERE [Id]=new.[Id]; END", actualDdl) << "DB object in actual file has different DDL than in benchmark file: " << actualName;
            else
                {
                Utf8CP benchmarkDdl = benchmarkDdlLookupStmt.GetValueText(0);
                EXPECT_STREQ(benchmarkDdl, actualDdl) << "DB object in actual file has different DDL than in benchmark file: " << actualName;
                }
            }
        else
            EXPECT_TRUE(false) << "DB object in benchmark file not found: " << actualName;

        benchmarkDdlLookupStmt.Reset();
        benchmarkDdlLookupStmt.ClearBindings();
        }

    ASSERT_EQ(benchmarkMasterTableRowCount, actualMasterTableRowCount) << benchmarkFilePath.GetNameUtf8();
    }


//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  10/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, ProfileUpgrade)
    {
    BeFileName benchmarkFilePath = GetBenchmarkFileFolder(InitialBim2ProfileVersion());
    benchmarkFilePath.AppendToPath(L"imodel2.ecdb");

    BeFileName artefactOutDir;
    BeTest::GetHost().GetOutputRoot(artefactOutDir);
    if (!artefactOutDir.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(artefactOutDir));

    BeFileName upgradedFilePath(artefactOutDir);
    upgradedFilePath.AppendToPath(L"upgradedimodel2.ecdb");
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(benchmarkFilePath, upgradedFilePath));
    ECDb upgradedFile;
    ASSERT_EQ(BE_SQLITE_OK, upgradedFile.OpenBeSQLiteDb(upgradedFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::Upgrade)));

    //verify 4.0.0.1 upgrade
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(upgradedFile, "SELECT Name FROM " BEDB_TABLE_Local " ORDER BY Name"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "First row";
    ASSERT_STRCASEEQ("be_repositoryid", stmt.GetValueText(0)) << "First row";
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "Second row";
    ASSERT_STRCASEEQ("ec_instanceidsequence", stmt.GetValueText(0)) << "Second row";
    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << "Only two entries expected in " << BEDB_TABLE_Local;
    stmt.Finalize();
    
    //verify 4.0.0.2 upgrade
    Db benchmarkFile;
    ASSERT_EQ(BE_SQLITE_OK, benchmarkFile.OpenBeSQLiteDb(benchmarkFilePath, Db::OpenParams(Db::OpenMode::Readonly)));

    {
    //verify that ECDbMeta schema was upgraded to version 4.0.1
    //and ECDbSystem schema was upgraded to version 5.0.1
    ECSqlStatement ecsqlStmt;
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(upgradedFile, "SELECT Name,VersionMajor,VersionWrite,VersionMinor FROM meta.ECSchemaDef WHERE Name IN ('ECDbFileInfo','ECDbMeta','ECDbSystem') ORDER BY Name"));
    ASSERT_EQ(BE_SQLITE_ROW, ecsqlStmt.Step());
    EXPECT_STREQ("ECDbFileInfo", ecsqlStmt.GetValueText(0));
    EXPECT_EQ(2, ecsqlStmt.GetValueInt(1));
    EXPECT_EQ(0, ecsqlStmt.GetValueInt(2));
    EXPECT_EQ(1, ecsqlStmt.GetValueInt(3));
    ASSERT_EQ(BE_SQLITE_ROW, ecsqlStmt.Step());
    EXPECT_STREQ("ECDbMeta", ecsqlStmt.GetValueText(0));
    EXPECT_EQ(4, ecsqlStmt.GetValueInt(1));
    EXPECT_EQ(0, ecsqlStmt.GetValueInt(2));
    EXPECT_EQ(1, ecsqlStmt.GetValueInt(3));
    ASSERT_EQ(BE_SQLITE_ROW, ecsqlStmt.Step());
    EXPECT_STREQ("ECDbSystem", ecsqlStmt.GetValueText(0));
    EXPECT_EQ(5, ecsqlStmt.GetValueInt(1));
    EXPECT_EQ(0, ecsqlStmt.GetValueInt(2));
    EXPECT_EQ(1, ecsqlStmt.GetValueInt(3));
    ecsqlStmt.Finalize();

    //verify that extended types were added to ECDbSystem classes
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(upgradedFile, "SELECT p.ExtendedTypeName FROM meta.ECPropertyDef p JOIN meta.ECClassDef c ON c.ECInstanceId=p.Class.Id JOIN meta.ECSchemaDef s ON s.ECInstanceId=c.Schema.Id WHERE s.Name='ECDbSystem' AND p.PrimitiveType=?"));
    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.BindInt(1, PrimitiveType::PRIMITIVETYPE_Long));
    int rowCount = 0;
    while (BE_SQLITE_ROW == ecsqlStmt.Step())
        {
        rowCount++;
        ASSERT_STREQ("Id", ecsqlStmt.GetValueText(0)) << "Expected ExtendedTypeName";
        }
    ASSERT_EQ(8, rowCount) << "Expected number of id properties in ECDbSystem schema";
    ecsqlStmt.Finalize();

    ASSERT_EQ(ECSqlStatus::Success, ecsqlStmt.Prepare(upgradedFile, "SELECT p.ExtendedTypeName FROM meta.ECPropertyDef p JOIN meta.ECClassDef c ON c.ECInstanceId=p.Class.Id JOIN meta.ECSchemaDef s ON s.ECInstanceId=c.Schema.Id WHERE s.Name='ECDbFileInfo' AND c.Name='FileInfoOwnership'"));
    rowCount = 0;
    while (BE_SQLITE_ROW == ecsqlStmt.Step())
        {
        rowCount++;
        ASSERT_STREQ("Id", ecsqlStmt.GetValueText(0)) << "Expected ExtendedTypeName";
        }
    ASSERT_EQ(4, rowCount) << "Expected number of id properties in ECDbFileInfo.FileInfoOwnership class";
    }

    bset<ECSchemaId> ecdbEnumSchemas;
    {
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(benchmarkFile, "SELECT Id FROM ec_Schema WHERE Name IN ('ECDbChange','ECDbFileInfo','ECDbMeta')"));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        ecdbEnumSchemas.insert(stmt.GetValueId<ECSchemaId>(0));
        }
    stmt.Finalize();
    }

    Statement benchmarkEnumsStmt, upgradedEnumsStmt;
    ASSERT_EQ(BE_SQLITE_OK, benchmarkEnumsStmt.Prepare(benchmarkFile, "SELECT EnumValues,Name,SchemaId FROM ec_Enumeration ORDER BY Id"));
    ASSERT_EQ(BE_SQLITE_OK, upgradedEnumsStmt.Prepare(upgradedFile, "SELECT EnumValues,Name,SchemaId FROM ec_Enumeration ORDER BY Id"));

    while (BE_SQLITE_ROW == benchmarkEnumsStmt.Step())
        {
        ASSERT_EQ(BE_SQLITE_ROW, upgradedEnumsStmt.Step());

        Utf8CP enumName = benchmarkEnumsStmt.GetValueText(1);
        ASSERT_STREQ(enumName, upgradedEnumsStmt.GetValueText(1));
        ECSchemaId schemaId = benchmarkEnumsStmt.GetValueId<ECSchemaId>(2);
        ASSERT_EQ(schemaId, upgradedEnumsStmt.GetValueId<ECSchemaId>(2));

        const bool isECDbEnum = ecdbEnumSchemas.find(schemaId) != ecdbEnumSchemas.end();

        Json::Value benchmarkEnumValuesJson, upgradedEnumValuesJson;
        ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(benchmarkEnumValuesJson, benchmarkEnumsStmt.GetValueText(0)));
        ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(upgradedEnumValuesJson, upgradedEnumsStmt.GetValueText(0)));
        ASSERT_EQ(benchmarkEnumValuesJson.size(), upgradedEnumValuesJson.size());
        for (Json::ArrayIndex i = 0; i < benchmarkEnumValuesJson.size(); i++)
            {
            Json::Value const& benchmarkEnumValueJson = benchmarkEnumValuesJson[i];
            Json::Value const& upgradedEnumValueJson = upgradedEnumValuesJson[i];
            ASSERT_TRUE(upgradedEnumValueJson.isMember("Name"));
            Utf8CP actualName = upgradedEnumValueJson["Name"].asCString();

            if (isECDbEnum)
                {
                if (BeStringUtilities::StricmpAscii(benchmarkEnumValueJson["DisplayLabel"].asCString(), "Point2d") == 0)
                    {
                    EXPECT_STREQ("Point2d", actualName) << enumName << " (Schema: " << schemaId.ToString() << ") Point2d was lower-cased during the upgrade";
                    EXPECT_STREQ("Point2d", upgradedEnumValueJson["DisplayLabel"].asCString()) << enumName << " (Schema: " << schemaId.ToString() << ") Point3d was lower-cased during the upgrade";
                    }
                else if (BeStringUtilities::StricmpAscii(benchmarkEnumValueJson["DisplayLabel"].asCString(), "Point3d") == 0)
                    {
                    EXPECT_STREQ("Point3d", actualName) << enumName << " (Schema: " << schemaId.ToString() << ") For ECDb enums the name is the the display label";
                    EXPECT_STREQ("Point3d", upgradedEnumValueJson["DisplayLabel"].asCString()) << enumName << " (Schema: " << schemaId.ToString() << ") For ECDb enums the name is the the display label";
                    }
                else
                    {
                    EXPECT_STREQ(benchmarkEnumValueJson["DisplayLabel"].asCString(), actualName) << enumName << " (Schema: " << schemaId.ToString() << ") For ECDb enums the name is the the display label";
                    EXPECT_STREQ(benchmarkEnumValueJson["DisplayLabel"].asCString(), upgradedEnumValueJson["DisplayLabel"].asCString()) << enumName << " (Schema: " << schemaId.ToString();
                    }
                }
            else
                {
                if (benchmarkEnumValueJson.isMember("StringValue"))
                    EXPECT_STREQ(benchmarkEnumValueJson["StringValue"].asCString(), actualName);
                else if (benchmarkEnumValueJson.isMember("IntValue"))
                    EXPECT_STREQ(Utf8PrintfString("%s%d", enumName, (int) benchmarkEnumValueJson["IntValue"].asInt()).c_str(), actualName);
                else
                    FAIL();

                EXPECT_EQ(benchmarkEnumValueJson.isMember("DisplayLabel"), upgradedEnumValueJson.isMember("DisplayLabel")) << enumName << " (Schema: " << schemaId.ToString() << ") Benchmark: " << benchmarkEnumValueJson.ToString() << " Upgraded: " << upgradedEnumValueJson.ToString();
                EXPECT_STREQ(benchmarkEnumValueJson["DisplayLabel"].asCString(), upgradedEnumValueJson["DisplayLabel"].asCString()) << enumName << " (Schema: " << schemaId.ToString() << ") Benchmark: " << benchmarkEnumValueJson.ToString() << " Upgraded: " << upgradedEnumValueJson.ToString();
                }
            }
        }

    ASSERT_TRUE(upgradedFile.TableExists("ec_Unit"));
    ASSERT_TRUE(upgradedFile.TableExists("ec_UnitSystem"));
    ASSERT_TRUE(upgradedFile.TableExists("ec_Phenomenon"));
    ASSERT_TRUE(upgradedFile.TableExists("ec_Format"));
    }



//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  06/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, OpenOldFileWithDifferentOptions)
    {
    BeFileName benchmarkFilePath = GetBenchmarkFileFolder(InitialBim2ProfileVersion());
    benchmarkFilePath.AppendToPath(L"imodel2.ecdb");

    BeFileName artefactOutDir;
    BeTest::GetHost().GetOutputRoot(artefactOutDir);
    if (!artefactOutDir.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(artefactOutDir));

    BeFileName oldFilePath(artefactOutDir);
    oldFilePath.AppendToPath(L"oldimodel2.ecdb");
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(benchmarkFilePath, oldFilePath));


    ECDb oldFile;
    {
    ScopedDisableFailOnAssertion disableAssertion;
    ASSERT_EQ((int) BE_SQLITE_READONLY, (int) oldFile.OpenBeSQLiteDb(oldFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly, ECDb::ProfileUpgradeOptions::Upgrade))) << "ProfileUpgradeOptions::Upgrade requires OpenMode::ReadWrite";
    }
    ASSERT_EQ((int) BE_SQLITE_OK, (int) oldFile.OpenBeSQLiteDb(oldFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite))) << "Can open readwrite";
    EXPECT_EQ(ProfileVersion(4, 0, 0, 0), oldFile.GetECDbProfileVersion()) << "Open without upgrade";
    oldFile.CloseDb();
    ASSERT_EQ((int) BE_SQLITE_OK, (int) oldFile.OpenBeSQLiteDb(oldFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::None))) << "Can open readwrite";
    EXPECT_EQ(ProfileVersion(4, 0, 0, 0), oldFile.GetECDbProfileVersion()) << "Open without upgrade";
    oldFile.CloseDb();
    ASSERT_EQ((int) BE_SQLITE_OK, (int) oldFile.OpenBeSQLiteDb(oldFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly))) << "Can open readonly";
    EXPECT_EQ(ProfileVersion(4, 0, 0, 0), oldFile.GetECDbProfileVersion()) << "Open without upgrade";
    oldFile.CloseDb();
    ASSERT_EQ((int) BE_SQLITE_OK, (int) oldFile.OpenBeSQLiteDb(oldFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::Upgrade))) << "Open with upgrade";
    EXPECT_EQ(ECDb::CurrentECDbProfileVersion(), oldFile.GetECDbProfileVersion()) << "Open with upgrade";
    oldFile.CloseDb();
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
    ASSERT_EQ(BE_SQLITE_OK, upgradedFile.OpenBeSQLiteDb(upgradedFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::Upgrade)));
    
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(upgradedFile, R"sql(SELECT count(*) FROM sqlite_master WHERE name LIKE 'ec\_%' ESCAPE '\' ORDER BY name COLLATE NOCASE)sql"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetSql();
    ASSERT_EQ(25, stmt.GetValueInt(0)) << "ECDb profile table count";
    }

    
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
    ASSERT_EQ(BE_SQLITE_OK, benchmarkDdlLookupStmt.Prepare(benchmarkFile, "SELECT name,sql FROM sqlite_master ORDER BY name"));


    Statement actualDdlStmt;
    ASSERT_EQ(BE_SQLITE_OK, actualDdlStmt.Prepare(upgradedFile, "SELECT sql FROM sqlite_master WHERE name=?"));
    int benchmarkRowCount = 0;
    while (BE_SQLITE_ROW == benchmarkDdlLookupStmt.Step())
        {
        benchmarkRowCount++;
        Utf8CP benchmarkName = benchmarkDdlLookupStmt.GetValueText(0);
        Utf8CP benchmarkDdl = benchmarkDdlLookupStmt.GetValueText(1);

        actualDdlStmt.BindText(1, benchmarkName, Statement::MakeCopy::No);
        if (BE_SQLITE_ROW == actualDdlStmt.Step())
            {
            Utf8CP actualDdl = actualDdlStmt.GetValueText(0);

            if (BeStringUtilities::StricmpAscii(benchmarkName,"ec_Schema") == 0)
                EXPECT_STREQ("CREATE TABLE ec_Schema(Id INTEGER PRIMARY KEY,Name TEXT UNIQUE NOT NULL COLLATE NOCASE,DisplayLabel TEXT,Description TEXT,Alias TEXT UNIQUE NOT NULL COLLATE NOCASE,VersionDigit1 INTEGER NOT NULL,VersionDigit2 INTEGER NOT NULL,VersionDigit3 INTEGER NOT NULL, OriginalECXmlVersionMajor INTEGER, OriginalECXmlVersionMinor INTEGER)",
                             actualDdl);
            else
                EXPECT_STREQ(benchmarkDdl, actualDdl) << "DB object in upgraded file has different DDL than in benchmark file: " << benchmarkName;

            actualDdlDumpFile->PutLine(WString(actualDdl, BentleyCharEncoding::Utf8).c_str(), true);
            }
        else
            EXPECT_TRUE(false) << "DB object in upgraded file not found: " << benchmarkName;

        actualDdlStmt.Reset();
        actualDdlStmt.ClearBindings();
        }
    actualDdlStmt.Finalize();
    ASSERT_EQ(BE_SQLITE_OK, actualDdlStmt.Prepare(upgradedFile, "SELECT count(*) FROM sqlite_master"));
    ASSERT_EQ(BE_SQLITE_ROW, actualDdlStmt.Step());
    ASSERT_EQ(benchmarkMasterTableRowCount + 18, actualDdlStmt.GetValueInt(0)) << " 18 sqlite_master entries are added in the upgrade " << benchmarkFilePath.GetNameUtf8();
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  01/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, ProfileUpgrade_Enums)
    {
    BeFileName benchmarkFilePath = GetBenchmarkFileFolder(InitialBim2ProfileVersion());
    benchmarkFilePath.AppendToPath(L"imodel2.ecdb");

    BeFileName artefactOutDir;
    BeTest::GetHost().GetOutputRoot(artefactOutDir);
    if (!artefactOutDir.DoesPathExist())
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(artefactOutDir));

    BeFileName upgradedFilePath(artefactOutDir);
    upgradedFilePath.AppendToPath(L"upgradedimodel2.ecdb");
    ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(benchmarkFilePath, upgradedFilePath));
    {
    //upgrade and close again
    ECDb upgradedFile;
    ASSERT_EQ(BE_SQLITE_OK, upgradedFile.OpenBeSQLiteDb(upgradedFilePath, ECDb::OpenParams(ECDb::OpenMode::ReadWrite, ECDb::ProfileUpgradeOptions::Upgrade)));
    }

    ECDb upgradedFile;
    ASSERT_EQ(BE_SQLITE_OK, upgradedFile.OpenBeSQLiteDb(upgradedFilePath, ECDb::OpenParams(ECDb::OpenMode::Readonly)));

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(upgradedFile, "SELECT s.Name, e.Name FROM meta.ECEnumerationDef e JOIN meta.ECSchemaDef s ON e.Schema.Id=s.ECInstanceId"));
    while (BE_SQLITE_ROW == stmt.Step())
        {
        AssertMetaSchemaEnumeration(upgradedFile, stmt.GetValueText(0), stmt.GetValueText(1));
        }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  07/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, CompareProfileTables_NewFile)
    {
    ASSERT_EQ(SUCCESS, SetupTestFile("imodel2fileformatcompatibility_newfile_test.ecdb"));

    Db benchmarkFile;
    BeFileName benchmarkFilePath = GetBenchmarkFileFolder(ECDb::CurrentECDbProfileVersion());
    benchmarkFilePath.AppendToPath(L"imodel2.ecdb");
    ASSERT_EQ(BE_SQLITE_OK, benchmarkFile.OpenBeSQLiteDb(benchmarkFilePath, Db::OpenParams(Db::OpenMode::Readonly))) << benchmarkFilePath.GetNameUtf8();
    
    //profile table count check
    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, R"sql(SELECT count(*) FROM sqlite_master WHERE name LIKE 'ec\_%' ESCAPE '\' ORDER BY name COLLATE NOCASE)sql"));
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetSql();
    ASSERT_EQ(25, stmt.GetValueInt(0)) << "ECDb profile table count";
    }

    //schema profile tables
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Schema", PROFILETABLE_SELECT_Schema));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_SchemaReference", PROFILETABLE_SELECT_SchemaReference));
    EXPECT_TRUE(CompareTable(benchmarkFile, m_ecdb, "ec_Class", PROFILETABLE_SELECT_Class, CompareOptions::IgnoreDescriptions));
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
// @bsiclass                                     Krischan.Eberle                  11/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, ForwardCompatibilitySafeguards)
    {
    ASSERT_EQ(SUCCESS, SetupECDb("ForwardCompatibilitySafeguards.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
              <ECSchemaReference name="ECDbMap" version="02.00" alias="ecdbmap" />
                <ECEntityClass typeName="A">
                    <ECProperty propertyName="Prop1" typeName="string" />
                    <ECProperty propertyName="Prop2" typeName="int" />
                </ECEntityClass>
                <ECEntityClass typeName="B">
                   <ECCustomAttributes>
                        <ClassMap xmlns="ECDbMap.02.00">
                             <MapStrategy>TablePerHierarchy</MapStrategy>
                        </ClassMap>
                        <ShareColumns xmlns="ECDbMap.02.00">
                            <MaxSharedColumnsBeforeOverflow>100</MaxSharedColumnsBeforeOverflow>
                            <ApplyToSubclassesOnly>True</ApplyToSubclassesOnly>
                        </ShareColumns>
                        <JoinedTablePerDirectSubclass xmlns="ECDbMap.02.00" />
                    </ECCustomAttributes>
                    <ECProperty propertyName="Prop1" typeName="string" />
                </ECEntityClass>
                <ECEntityClass typeName="SubB">
                    <BaseClass>B</BaseClass>
                </ECEntityClass>
                <ECRelationshipClass typeName="AHasB" strength="Referencing" modifier="None" strengthDirection="Backward">
                  <Source multiplicity="(0..*)" polymorphic="False" roleLabel="A">
                      <Class class ="A" />
                  </Source>
                  <Target multiplicity="(0..*)" polymorphic="False" roleLabel="B">
                      <Class class ="B" />
                  </Target>
               </ECRelationshipClass>
         </ECSchema>)xml")));

    ECClassId aClassId = m_ecdb.Schemas().GetClassId("TestSchema", "A");
    ASSERT_TRUE(aClassId.IsValid());
    ECClassId bClassId = m_ecdb.Schemas().GetClassId("TestSchema", "B");
    ASSERT_TRUE(bClassId.IsValid());
    ECClassId relClassId = m_ecdb.Schemas().GetClassId("TestSchema", "AHasB");
    ASSERT_TRUE(relClassId.IsValid());

    //Now modify some entries in the ec tables that mimick the scenario where new ECClass types, ECProperty types, MapStrategies would
    //get added in future versions of ECDb.

    //Missing table
    {
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("DROP TABLE ts_A"));
    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "A");
    ASSERT_TRUE(testClass != nullptr) << "GetClass should be possible for missing table";

    //ECSQL
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * FROM ts.A")) << "Preparing ECSQL against class with missing table is expected to fail";

    //ECClass views
    Savepoint sp(m_ecdb, "");
    bvector<ECClassId> classIds;
    classIds.push_back(testClass->GetId());
    EXPECT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb(classIds)) << "expected to succeed but view would not work because of missing table";
    sp.Cancel();

    //Schema import
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
                <ECEntityClass typeName="SubA">
                    <BaseClass>ts.A</BaseClass>
                    <ECProperty propertyName="NewProp" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml")));
    sp.Cancel();

    m_ecdb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }

    //Missing joined table
    {
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("DROP TABLE ts_SubB"));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "B");
    ASSERT_TRUE(testClass != nullptr) << "GetClass should be possible for missing table";

    testClass = m_ecdb.Schemas().GetClass("TestSchema", "SubB");
    ASSERT_TRUE(testClass != nullptr) << "GetClass should be possible for missing table";

    //ECSQL
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT * FROM ts.B")) << "Preparing ECSQL against class with missing table is expected to fail";
    stmt.Finalize();
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT * FROM ts.SubB")) << "Preparing ECSQL against class with missing table is expected to fail";

    //ECClass views
    Savepoint sp(m_ecdb, "");
    bvector<ECClassId> classIds;
    classIds.push_back(testClass->GetId());
    EXPECT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb(classIds)) << "expected to succeed but view would not work because of missing table";
    sp.Cancel();

    //Schema import
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
                <ECEntityClass typeName="SubA">
                    <BaseClass>ts.A</BaseClass>
                    <ECProperty propertyName="NewProp" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml")));
    sp.Cancel();

    m_ecdb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }

    //Unknown MapStrategy
    {
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql(Utf8PrintfString("UPDATE ec_ClassMap SET MapStrategy=5 WHERE ClassId=%s", aClassId.ToString().c_str()).c_str()));
    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "A");
    ASSERT_TRUE(testClass != nullptr) << "GetClass should be possible for unknown MapStrategy";

    //ECSQL
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NULL FROM ts.A")) << "Preparing ECSQL against class with unknown MapStrategy is expected to fail";
    stmt.Finalize();

    //ECClass views
    Savepoint sp(m_ecdb, "");
    bvector<ECClassId> classIds;
    classIds.push_back(testClass->GetId());
    EXPECT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb(classIds));
    sp.Cancel();

    //Schema import
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
               <ECEntityClass typeName="SubB2">
                    <BaseClass>ts.B</BaseClass>
                    <ECProperty propertyName="NewProp" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml")));
    sp.Cancel();

    m_ecdb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }

    {
    //Unknown ShareColumnsMode
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql(Utf8PrintfString("UPDATE ec_ClassMap SET ShareColumnsMode=100 WHERE ClassId=%s", bClassId.ToString().c_str()).c_str()));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "B");
    ASSERT_TRUE(testClass != nullptr) << "GetClass should be possible for unknown ShareColumnsMode";

    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NULL FROM ts.B")) << "Preparing ECSQL against class with unknown ShareColumnsMode is expected to fail";
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NULL FROM ts.SubB")) << "Preparing ECSQL against class with unknown ShareColumnsMode is expected to fail";

    //ECClass views
    Savepoint sp(m_ecdb, "");
    bvector<ECClassId> classIds;
    classIds.push_back(testClass->GetId());
    EXPECT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb(classIds));
    sp.Cancel();

    //Schema import
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
                <ECEntityClass typeName="SubB2">
                    <BaseClass>ts.B</BaseClass>
                    <ECProperty propertyName="NewProp" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml")));
    sp.Cancel();

    m_ecdb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }

    {
    //Unknown JoinedTableInfo
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql(Utf8PrintfString("UPDATE ec_ClassMap SET JoinedTableInfo=100 WHERE ClassId=%s", bClassId.ToString().c_str()).c_str()));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "B");
    ASSERT_TRUE(testClass != nullptr) << "GetClass should be possible for unknown JoinedTableInfo";

    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NULL FROM ts.B")) << "Preparing ECSQL against class with unknown JoinedTableInfo is expected to fail";
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NULL FROM ts.SubB")) << "Preparing ECSQL against class with unknown JoinedTableInfo is expected to fail";

    //ECClass views
    Savepoint sp(m_ecdb, "");
    bvector<ECClassId> classIds;
    classIds.push_back(testClass->GetId());
    EXPECT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb(classIds));
    sp.Cancel();

    //Schema import
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
                <ECEntityClass typeName="SubB2">
                    <BaseClass>ts.B</BaseClass>
                    <ECProperty propertyName="NewProp" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml")));
    sp.Cancel();

    m_ecdb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }

    {
    //Unknown TableType
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("UPDATE ec_Table SET Type=100 WHERE Name='ts_B'"));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "B");
    ASSERT_TRUE(testClass != nullptr) << "GetClass should be possible for unknown TableType";

    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NULL FROM ts.B")) << "Preparing ECSQL against class with unknown TableType is expected to fail";
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NULL FROM ts.SubB")) << "Preparing ECSQL against class with unknown TableType is expected to fail";

    //ECClass views
    Savepoint sp(m_ecdb, "");
    bvector<ECClassId> classIds;
    classIds.push_back(testClass->GetId());
    EXPECT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb(classIds));
    sp.Cancel();

    //Schema import
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
                <ECEntityClass typeName="SubB2">
                    <BaseClass>ts.B</BaseClass>
                    <ECProperty propertyName="NewProp" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml")));
    sp.Cancel();

    m_ecdb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }

    {
    //Unknown ColumnKind
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("UPDATE ec_Column SET ColumnKind=-1 WHERE Name='Prop2'"));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "A");
    ASSERT_TRUE(testClass != nullptr) << "GetClass should be possible for unknown ColumnKind";

    //ECSQL
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NULL FROM ts.A")) << "Preparing ECSQL against class with unknown ColumnKind is expected to fail";

    //ECClass views
    Savepoint sp(m_ecdb, "");
    bvector<ECClassId> classIds;
    classIds.push_back(testClass->GetId());
    EXPECT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb(classIds));
    sp.Cancel();

    //Schema import
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
                <ECEntityClass typeName="SubA">
                    <BaseClass>ts.A</BaseClass>
                    <ECProperty propertyName="NewProp" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml")));
    sp.Cancel();

    m_ecdb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }

    {
    //Unknown column data type
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("UPDATE ec_Column SET Type=1000 WHERE Name='Prop2'"));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "A");
    ASSERT_TRUE(testClass != nullptr) << "GetClass should be possible for unknown column data type";

    //ECSQL
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NULL FROM ts.A")) << "Preparing ECSQL against class with unknown column data type is expected to fail";

    //ECClass views
    Savepoint sp(m_ecdb, "");
    bvector<ECClassId> classIds;
    classIds.push_back(testClass->GetId());
    EXPECT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb(classIds));
    sp.Cancel();

    //Schema import
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
                <ECEntityClass typeName="SubA">
                    <BaseClass>ts.A</BaseClass>
                    <ECProperty propertyName="NewProp" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml")));
    sp.Cancel();

    m_ecdb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }

    {
    //Unknown column collation
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("UPDATE ec_Column SET CollationConstraint=1000 WHERE Name='Prop2'"));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "A");
    ASSERT_TRUE(testClass != nullptr) << "GetClass should be possible for unknown collation";

    //ECSQL
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NULL FROM ts.A")) << "Preparing ECSQL against class with unknown collation is expected to fail";

    //ECClass views
    Savepoint sp(m_ecdb, "");
    bvector<ECClassId> classIds;
    classIds.push_back(testClass->GetId());
    EXPECT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb(classIds));
    sp.Cancel();

    //Schema import
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
                <ECEntityClass typeName="SubA">
                    <BaseClass>ts.A</BaseClass>
                    <ECProperty propertyName="NewProp" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml")));
    sp.Cancel();

    m_ecdb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }

    {
    //Unknown ECClassType
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("UPDATE ec_Class SET Type=100 WHERE Name='A'"));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "A");
    EXPECT_TRUE(testClass == nullptr) << "GetClass should not be possible for unknown ECClassType";

    //ECSQL
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NULL FROM ts.A")) << "Preparing ECSQL against class with unknown ECClassType is expected to fail";

    //ECClass views
    Savepoint sp(m_ecdb, "");
    EXPECT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb());
    sp.Cancel();

    //Schema import
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
                <ECEntityClass typeName="SubA">
                    <BaseClass>ts.A</BaseClass>
                    <ECProperty propertyName="NewProp" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml")));
    sp.Cancel();

    m_ecdb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }

    {
    //Unknown ECClassModifier
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("UPDATE ec_Class SET Modifier=100 WHERE Name='A'"));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "A");
    EXPECT_TRUE(testClass == nullptr) << "GetClass should not be possible for unknown ECClassModifier";

    //ECSQL
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NULL FROM ts.A")) << "Preparing ECSQL against class with unknown ECClassModifier is expected to fail";

    //ECClass views
    Savepoint sp(m_ecdb, "");
    EXPECT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb());
    sp.Cancel();

    //Schema import
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
                <ECEntityClass typeName="SubA">
                    <BaseClass>ts.A</BaseClass>
                    <ECProperty propertyName="NewProp" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml")));
    sp.Cancel();

    m_ecdb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }

    {
    //Unknown relationship strength
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("UPDATE ec_Class SET RelationshipStrength=100 WHERE Name='AHasB'"));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "AHasB");
    EXPECT_TRUE(testClass == nullptr) << "GetClass should not be possible for unknown relationship strength";

    //ECSQL
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NULL FROM ts.AHasB")) << "Preparing ECSQL against class with unknown relationship strength is expected to fail";

    //ECClass views
    Savepoint sp(m_ecdb, "");
    EXPECT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb());
    sp.Cancel();

    //Schema import
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
                <ECRelationshipClass typeName="SubAHasB" strength="Referencing" modifier="None" strengthDirection="Backward">
                    <BaseClass>ts.AHasB</BaseClass>
                  <Source multiplicity="(0..*)" polymorphic="False" roleLabel="A">
                      <Class class ="A" />
                  </Source>
                  <Target multiplicity="(0..*)" polymorphic="False" roleLabel="B">
                      <Class class ="B" />
                  </Target>
               </ECRelationshipClass>
                </ECSchema>)xml")));
    sp.Cancel();

    m_ecdb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }

    {
    //Unknown relationship strength direction
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("UPDATE ec_Class SET RelationshipStrengthDirection=100 WHERE Name='AHasB'"));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "AHasB");
    EXPECT_TRUE(testClass == nullptr) << "GetClass should not be possible for unknown relationship strength direction";

    //ECSQL
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT NULL FROM ts.AHasB")) << "Preparing ECSQL against class with unknown relationship strength direction is expected to fail";

    //ECClass views
    Savepoint sp(m_ecdb, "");
    EXPECT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb());
    sp.Cancel();

    //Schema import
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
                <ECRelationshipClass typeName="SubAHasB" strength="Referencing" modifier="None" strengthDirection="Backward">
                    <BaseClass>ts.AHasB</BaseClass>
                  <Source multiplicity="(0..*)" polymorphic="False" roleLabel="A">
                      <Class class ="A" />
                  </Source>
                  <Target multiplicity="(0..*)" polymorphic="False" roleLabel="B">
                      <Class class ="B" />
                  </Target>
               </ECRelationshipClass>
                </ECSchema>)xml")));
    sp.Cancel();

    m_ecdb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }

    {
    //Unknown property kind type
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("UPDATE ec_Property SET Kind=1000 WHERE Name='Prop2'"));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "A");
    EXPECT_TRUE(testClass == nullptr) << "GetClass should not be possible for unknown property kind";

    //ECSQL
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT Prop2 FROM ts.A")) << "Preparing ECSQL against class with unknown property kind is expected to fail";

    //ECClass views
    Savepoint sp(m_ecdb, "");
    EXPECT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb());
    sp.Cancel();

    //Schema import
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
                <ECEntityClass typeName="SubA">
                    <BaseClass>ts.A</BaseClass>
                    <ECProperty propertyName="NewProp" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml")));
    sp.Cancel();

    m_ecdb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }

    {
    //Unknown primitive type
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.ExecuteSql("UPDATE ec_Property SET PrimitiveType=-1 WHERE Name='Prop2'"));

    ECClassCP testClass = m_ecdb.Schemas().GetClass("TestSchema", "A");
    EXPECT_TRUE(testClass == nullptr) << "GetClass should not be possible for unknown property primitive type";

    //ECSQL
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::InvalidECSql, stmt.Prepare(m_ecdb, "SELECT Prop2 FROM ts.A")) << "Preparing ECSQL against class with unknown property primitive type is expected to fail";

    //ECClass views
    Savepoint sp(m_ecdb, "");
    EXPECT_EQ(ERROR, m_ecdb.Schemas().CreateClassViewsInDb());
    sp.Cancel();

    //Schema import
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
                <ECSchema schemaName="TestSchema2" alias="ts2" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECSchemaReference name="TestSchema" version="01.00" alias="ts" />
                <ECEntityClass typeName="SubA">
                    <BaseClass>ts.A</BaseClass>
                    <ECProperty propertyName="NewProp" typeName="string" />
                </ECEntityClass>
                </ECSchema>)xml")));
    sp.Cancel();

    m_ecdb.AbandonChanges();
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  11/17
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, ForwardCompatibilitySafeguards_ECEnums)
    {
    //Future EC3.2 ECEnumerator property (If this code has already EC3.2 we don't need to execute the test)
    if (ECN::ECVersion::Latest > ECN::ECVersion::V3_1)
        return;

    ASSERT_EQ(SUCCESS, SetupECDb("ForwardCompatibilitySafeguards_ECEnums.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEnumeration typeName="IntEnumNoDisplayLabel" backingTypeName="int" >
                    <ECEnumerator value="0" />
                    <ECEnumerator value="1" />
                </ECEnumeration>
                <ECEnumeration typeName="IntEnumDisplayLabel" backingTypeName="int" >
                    <ECEnumerator value="0" displayLabel="Turn On"/>
                    <ECEnumerator value="1" displayLabel="Turn Off"/>
                </ECEnumeration>
                <ECEnumeration typeName="StringEnumNoDisplayLabel" backingTypeName="string" >
                    <ECEnumerator value="On" />
                    <ECEnumerator value="Off" />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnumDisplayLabel" backingTypeName="string" >
                    <ECEnumerator value="On" displayLabel="Turn On" />
                    <ECEnumerator value="Off" displayLabel="Turn Off" />
                </ECEnumeration>                
                <ECEntityClass typeName="Foo">
                    <ECProperty propertyName="Prop1" typeName="IntEnumNoDisplayLabel" />
                    <ECProperty propertyName="Prop2" typeName="IntEnumDisplayLabel" />
                    <ECProperty propertyName="Prop3" typeName="StringEnumNoDisplayLabel" />
                    <ECProperty propertyName="Prop4" typeName="StringEnumDisplayLabel" />
                </ECEntityClass>
         </ECSchema>)xml")));

    {
    Statement stmt;
    ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "SELECT Name, Id, EnumValues FROM ec_Enumeration ORDER BY Name"));
    bmap<BeInt64Id, Json::Value> enumValues;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8CP enumName = stmt.GetValueText(0);
        Json::Value& json = enumValues[stmt.GetValueId<BeInt64Id>(1)];
        ASSERT_EQ(SUCCESS, TestUtilities::ParseJson(json, stmt.GetValueText(2)));

        for (Json::Value& enumValue : json)
            {
            if (enumValue.isMember("StringValue"))
                enumValue["Name"] = ECNameValidation::EncodeToValidName(enumValue["StringValue"].asCString());
            else if (enumValue.isMember("IntValue"))
                {
                Utf8String name;
                name.Sprintf("%s%d", enumName, enumValue["IntValue"].asInt());
                enumValue["Name"] = ECNameValidation::EncodeToValidName(name);
                }

            if (enumValue.isMember("DisplayLabel"))
                enumValue["Description"] = enumValue["DisplayLabel"].asCString();
            }
        stmt.Finalize();
        ASSERT_EQ(BE_SQLITE_OK, stmt.Prepare(m_ecdb, "UPDATE ec_Enumeration SET EnumValues=? WHERE Id=?"));
        for (bpair<BeInt64Id, Json::Value> const& kvPair : enumValues)
            {
            ASSERT_EQ(BE_SQLITE_OK, stmt.BindText(1, kvPair.second.ToString(), Statement::MakeCopy::Yes));
            ASSERT_EQ(BE_SQLITE_OK, stmt.BindId(2, kvPair.first));
            ASSERT_EQ(BE_SQLITE_DONE, stmt.Step());
            stmt.Reset();
            stmt.ClearBindings();
            }
        stmt.Finalize();
        }

    //bump up profile version (which is expected if the file format changes)
    ASSERT_EQ(BE_SQLITE_OK, IncrementProfileVersion(m_ecdb));
    ASSERT_EQ(BE_SQLITE_OK, m_ecdb.SaveChanges());
    ASSERT_EQ(BE_SQLITE_OK, ReopenECDb());
    }

    {
    ECEnumerationCP ecenum = m_ecdb.Schemas().GetEnumeration("TestSchema", "IntEnumNoDisplayLabel");
    ASSERT_TRUE(ecenum != nullptr);
    ECEnumeratorCP enumValue = ecenum->FindEnumerator(0);
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_EQ(0, enumValue->GetInteger());
    ASSERT_FALSE(enumValue->GetIsDisplayLabelDefined());
    enumValue = ecenum->FindEnumerator(1);
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_EQ(1, enumValue->GetInteger());
    ASSERT_FALSE(enumValue->GetIsDisplayLabelDefined());
    }

    {
    ECEnumerationCP ecenum = m_ecdb.Schemas().GetEnumeration("TestSchema", "IntEnumDisplayLabel");
    ASSERT_TRUE(ecenum != nullptr);
    ECEnumeratorCP enumValue = ecenum->FindEnumerator(0);
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_EQ(0, enumValue->GetInteger());
    ASSERT_TRUE(enumValue->GetIsDisplayLabelDefined());
    ASSERT_STREQ("Turn On", enumValue->GetDisplayLabel().c_str());
    enumValue = ecenum->FindEnumerator(1);
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_EQ(1, enumValue->GetInteger());
    ASSERT_TRUE(enumValue->GetIsDisplayLabelDefined());
    ASSERT_STREQ("Turn Off", enumValue->GetDisplayLabel().c_str());
    }

    {
    ECEnumerationCP ecenum = m_ecdb.Schemas().GetEnumeration("TestSchema", "StringEnumNoDisplayLabel");
    ASSERT_TRUE(ecenum != nullptr);
    ECEnumeratorCP enumValue = ecenum->FindEnumerator("On");
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("On", enumValue->GetString().c_str());
    ASSERT_FALSE(enumValue->GetIsDisplayLabelDefined());
    enumValue = ecenum->FindEnumerator("Off");
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("Off", enumValue->GetString().c_str());
    ASSERT_FALSE(enumValue->GetIsDisplayLabelDefined());
    }

    {
    ECEnumerationCP ecenum = m_ecdb.Schemas().GetEnumeration("TestSchema", "StringEnumDisplayLabel");
    ASSERT_TRUE(ecenum != nullptr);
    ECEnumeratorCP enumValue = ecenum->FindEnumerator("On");
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("On", enumValue->GetString().c_str());
    ASSERT_TRUE(enumValue->GetIsDisplayLabelDefined());
    ASSERT_STREQ("Turn On", enumValue->GetDisplayLabel().c_str());
    enumValue = ecenum->FindEnumerator("Off");
    ASSERT_TRUE(enumValue != nullptr);
    ASSERT_STREQ("Off", enumValue->GetString().c_str());
    ASSERT_TRUE(enumValue->GetIsDisplayLabelDefined());
    ASSERT_STREQ("Turn Off", enumValue->GetDisplayLabel().c_str());
    }

    //ECSQL
    ECSqlStatement stmt;
    EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(m_ecdb, "SELECT Prop1 FROM ts.Foo")) << "Preparing ECSQL against class with unknown property primitive type is expected to fail";

    //ECClass views
    Savepoint sp(m_ecdb, "");
    EXPECT_EQ(SUCCESS, m_ecdb.Schemas().CreateClassViewsInDb());
    sp.Cancel();

    //Schema upgrade
    sp.Begin();
    EXPECT_EQ(ERROR, GetHelper().ImportSchema(SchemaItem(R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
                <ECEnumeration typeName="IntEnumNoDisplayLabel" backingTypeName="int" >
                    <ECEnumerator value="0" />
                    <ECEnumerator value="1" />
                    <ECEnumerator value="2" />
                </ECEnumeration>
                <ECEnumeration typeName="IntEnumDisplayLabel" backingTypeName="int" >
                    <ECEnumerator value="0" displayLabel="Turn On"/>
                    <ECEnumerator value="1" displayLabel="Turn Off"/>
                    <ECEnumerator value="2" displayLabel="Toggle"/>
                </ECEnumeration>
                <ECEnumeration typeName="StringEnumNoDisplayLabel" backingTypeName="string" >
                    <ECEnumerator value="On" />
                    <ECEnumerator value="Off" />
                    <ECEnumerator value="Toggle" />
                </ECEnumeration>
                <ECEnumeration typeName="StringEnumDisplayLabel" backingTypeName="string" >
                    <ECEnumerator value="On" displayLabel="Turn On" />
                    <ECEnumerator value="Off" displayLabel="Turn Off" />
                    <ECEnumerator value="Toggle" displayLabel="Toggle me" />
                </ECEnumeration>                
                <ECEntityClass typeName="Foo">
                    <ECProperty propertyName="Prop1" typeName="IntEnumNoDisplayLabel" />
                    <ECProperty propertyName="Prop2" typeName="IntEnumDisplayLabel" />
                    <ECProperty propertyName="Prop3" typeName="StringEnumNoDisplayLabel" />
                    <ECProperty propertyName="Prop4" typeName="StringEnumDisplayLabel" />
                </ECEntityClass>
         </ECSchema>)xml"))) << "Profile newer than software";
    sp.Cancel();

    m_ecdb.AbandonChanges();
    }

//---------------------------------------------------------------------------------------
// Tests that Units which are supported in EC3.2 are not supported yet in EC3.1 schemas.
// @bsiclass                                     Krischan.Eberle                  03/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, EC31KOQs)
    {
    //garbage units
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="garbage" presentationUnits="FT;IN;M" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="garbage(myformat)" presentationUnits="FT;IN;M" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="CM" presentationUnits="FT;Garbage;M" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="CM" presentationUnits="FT;Garbage(myFormat);M" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml")));

    //EC-compatible names (only possible in EC3.2, but not yet in EC3.1)
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="KM_PER_SEC" presentationUnits="M/SEC" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "EC3.1 cannot handle ECnamed units";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="u:KM_PER_SEC" presentationUnits="M/SEC" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "EC3.1 cannot handle ECnamed units";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="u:KM_PER_SEC(DefaultReal)" presentationUnits="M/SEC" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "EC3.1 cannot handle ECnamed units";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="KM/SEC" presentationUnits="M_PER_SEC" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "EC3.1 cannot handle ECnamed units";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="KM/SEC" presentationUnits="M_PER_SEC(DefaultReal)" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "EC3.1 cannot handle ECnamed units";

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="KM/SEC" presentationUnits="u:M_PER_SEC(DefaultReal)" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml"))) << "EC3.1 cannot handle ECnamed units";

    //User defined units (only possible in EC3.2, but not yet in EC3.1)
    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="MyUnit(defaultReal)" presentationUnits="M" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="myalias:MyUnit(defaultReal)" presentationUnits="M" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="MyUnit" presentationUnits="M" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="M" presentationUnits="MyUnit" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="M" presentationUnits="MyUnit(DefaultReal)" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml")));

    ASSERT_EQ(ERROR, TestHelper::RunSchemaImport(SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="M" presentationUnits="myalias:MyUnit(DefaultReal)" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml")));
    }

//---------------------------------------------------------------------------------------
// @bsiclass                                     Krischan.Eberle                  03/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(FileFormatCompatibilityTests, ForwardCompatibilitySafeguards_KOQs)
    {
    //Future EC3.2 units (If this code has already EC3.2 we don't need to execute the test)
    if (ECN::ECVersion::Latest > ECN::ECVersion::V3_1)
        return;

    ASSERT_EQ(SUCCESS, SetupECDb("ForwardCompatibilitySafeguards_KOQs.ecdb", SchemaItem(
        R"xml(<?xml version="1.0" encoding="utf-8"?>
        <ECSchema schemaName="TestSchema" alias="ts" version="1.0" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.1">
            <KindOfQuantity typeName="MyKoq" persistenceUnit="CM" presentationUnits="FT;IN;M" relativeError=".5"/>
            <ECEntityClass typeName="Foo">
               <ECProperty propertyName="Prop" typeName="int" kindOfQuantity="MyKoq" />
            </ECEntityClass>
        </ECSchema>)xml")));

    KindOfQuantityId koqId;
    { 
    KindOfQuantityCP koq = m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "MyKoq");
    ASSERT_TRUE(koq != nullptr);
    koqId = koq->GetId();
    ASSERT_TRUE(koqId.IsValid());
    }


    auto setEC32 = [&koqId, this] (Utf8CP persistenceUnitStr, Utf8CP presentationFormatStr)
        {
        BeFileName ecdbPath(m_ecdb.GetDbFileName());
        CloseECDb();

        //bump up profile version (which is expected if the file format changes)
        Db ecdb;
        if (BE_SQLITE_OK != ecdb.OpenBeSQLiteDb(ecdbPath, Db::OpenParams(Db::OpenMode::ReadWrite)))
            return ERROR;

        if (BE_SQLITE_OK != IncrementProfileVersion(ecdb))
            return ERROR;

        CachedStatementPtr stmt = ecdb.GetCachedStatement("UPDATE ec_KindOfQuantity SET PersistenceUnit=?, PresentationUnits=? WHERE Id=?");
        if (stmt == nullptr)
            return ERROR;

        stmt->BindText(1, persistenceUnitStr, Statement::MakeCopy::No);
        if (!Utf8String::IsNullOrEmpty(presentationFormatStr))
            stmt->BindText(2, presentationFormatStr, Statement::MakeCopy::No);

        stmt->BindId(3, koqId);
        if (BE_SQLITE_DONE != stmt->Step())
            return ERROR;

        stmt = nullptr;
        if (BE_SQLITE_OK != ecdb.SaveChanges())
            return ERROR;

        ecdb.CloseDb();

        if (BE_SQLITE_OK != OpenECDb(ecdbPath, ECDb::OpenParams(ECDb::OpenMode::Readonly)))
            return ERROR;

        return SUCCESS;
        };

    auto getKoq = [this] () { return m_ecdb.Schemas().GetKindOfQuantity("TestSchema", "MyKoq");};

    auto getPropertyKoq = [this] ()
        {
        ECClassCP ecClass = m_ecdb.Schemas().GetClass("TestSchema", "Foo");
        if (ecClass == nullptr)
            return (KindOfQuantityCP) nullptr;

        ECPropertyCP prop = ecClass->GetPropertyP("Prop");
        if (prop == nullptr)
            return (KindOfQuantityCP) nullptr;

        return prop->GetKindOfQuantity();
        };

    auto assertUnit = [] (ECN::ECUnitCR unit, bool expectedIsValidUnit, Utf8CP expectedUnitName)
        {
        EXPECT_EQ(expectedIsValidUnit, unit.IsValid()) << unit.GetFullName();
        EXPECT_STRCASEEQ(expectedUnitName, unit.GetFullName().c_str());
        };

    //garbage units
    {
    ASSERT_EQ(SUCCESS, setEC32("garbage(unknownFormat)", nullptr));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), false, "garbage");
    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("garbage", nullptr));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), false, "garbage");
    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("garbage(unknownFormat)", R"json(["CM","M"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), false, "garbage");
    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("garbage", R"json(["CM","M"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), false, "garbage");
    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("KM", R"json(["CM","garbage(unknownFormat)"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), true, "u:KM");
    ASSERT_EQ(1, koq->GetPresentationFormats().size());
    ASSERT_STRCASEEQ(koq->GetPresentationFormats().at(0).GetName().c_str(), "f:defaultReal");
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("KM", R"json(["CM","garbage"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), true, "u:KM");
    ASSERT_EQ(1, koq->GetPresentationFormats().size());
    ASSERT_STRCASEEQ(koq->GetPresentationFormats().at(0).GetName().c_str(), "f:defaultReal");
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("KM", R"json(["garbage(unknownFormat)","CM"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), true, "u:KM");
    ASSERT_EQ(1, koq->GetPresentationFormats().size());
    ASSERT_STRCASEEQ(koq->GetPresentationFormats().at(0).GetName().c_str(), "f:defaultReal");
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("KM", R"json(["garbage","CM"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), true, "u:KM");
    ASSERT_EQ(1, koq->GetPresentationFormats().size());
    ASSERT_STRCASEEQ(koq->GetPresentationFormats().at(0).GetName().c_str(), "f:defaultReal");
    }

    //User defined EC3.2 units
    {
    ASSERT_EQ(SUCCESS, setEC32("myalias:myunit(myformat)", R"json(["myalias:mydisplayunit1(myformat)","myalias:mydisplayunit2(myotherformat)"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), false, "myalias:myunit");
    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("myunit(myformat)", R"json(["mydisplayunit1(myformat)","mydisplayunit2(myotherformat)"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), false, "myunit");
    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("myalias:myunit", R"json(["myalias:mydisplayunit1","myalias:mydisplayunit2"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), false, "myalias:myunit");
    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("CM", R"json(["myalias:myunit(myformat)","M"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), true, "u:CM");
    ASSERT_EQ(1, koq->GetPresentationFormats().size());
    ASSERT_STRCASEEQ(koq->GetPresentationFormats().at(0).GetName().c_str(), "f:defaultReal");
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("CM", R"json(["myalias:myunit","M"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), true, "u:CM");
    ASSERT_EQ(1, koq->GetPresentationFormats().size());
    ASSERT_STRCASEEQ(koq->GetPresentationFormats().at(0).GetName().c_str(), "f:defaultReal");
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("CM", R"json(["KM","myalias:myunit(myformat)","M"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), true, "u:CM");
    ASSERT_EQ(2, koq->GetPresentationFormats().size());
    ASSERT_STRCASEEQ(koq->GetPresentationFormats().at(0).GetName().c_str(), "f:defaultReal");
    ASSERT_STRCASEEQ(koq->GetPresentationFormats().at(1).GetName().c_str(), "f:defaultReal");
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("CM", R"json(["KM","myalias:myunit","M"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), true, "u:CM");
    ASSERT_EQ(2, koq->GetPresentationFormats().size());
    ASSERT_STRCASEEQ(koq->GetPresentationFormats().at(0).GetName().c_str(), "f:defaultReal");
    ASSERT_STRCASEEQ(koq->GetPresentationFormats().at(1).GetName().c_str(), "f:defaultReal");
    }

    //standard units
    {
    ASSERT_EQ(SUCCESS, setEC32("KM", nullptr));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), true, "u:KM");
    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("u:KM", nullptr));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), true, "u:KM");
    EXPECT_TRUE(koq->GetPresentationFormats().empty());
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("KM", R"json(["CM","M"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), true, "u:KM");
    ASSERT_EQ(2, koq->GetPresentationFormats().size());
    ASSERT_STRCASEEQ(koq->GetPresentationFormats().at(0).GetName().c_str(), "f:defaultReal");
    ASSERT_STRCASEEQ(koq->GetPresentationFormats().at(1).GetName().c_str(), "f:defaultReal");
    }

    {
    ASSERT_EQ(SUCCESS, setEC32("u:KM", R"json(["u:CM","u:M"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), true, "u:KM");
    ASSERT_EQ(2, koq->GetPresentationFormats().size());
    ASSERT_STRCASEEQ(koq->GetPresentationFormats().at(0).GetName().c_str(), "f:defaultReal");
    ASSERT_STRCASEEQ(koq->GetPresentationFormats().at(1).GetName().c_str(), "f:defaultReal");
    }

    //standard units where ECName differs from unit name
    {
    ASSERT_EQ(SUCCESS, setEC32("u:CM_PER_SEC", R"json(["u:M_PER_SEC"])json"));
    KindOfQuantityCP koq = getKoq();
    ASSERT_TRUE(koq != nullptr);
    EXPECT_EQ(koq, getPropertyKoq());
    assertUnit(*koq->GetPersistenceUnit(), true, "u:CM_PER_SEC");
    ASSERT_EQ(1, koq->GetPresentationFormats().size());
    ASSERT_STRCASEEQ(koq->GetPresentationFormats().at(0).GetName().c_str(), "f:defaultReal");
    }

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
// @bsimethod                                                Krischan.Eberle      01/2018
//---------------------------------------------------------------------------------------
//static
DbResult FileFormatCompatibilityTests::IncrementProfileVersion(DbR db)
    {
    //bump up profile version (which is expected if the file format changes)
    PropertySpec profileVersionSpec("SchemaVersion", "ec_Db");
    Utf8String profileVersionStr;
    DbResult stat = db.QueryProperty(profileVersionStr, profileVersionSpec);
    if (BE_SQLITE_ROW != stat)
        return stat;

    ProfileVersion profileVersion(profileVersionStr.c_str());
    profileVersion = ProfileVersion(profileVersion.GetMajor(), profileVersion.GetMinor(), profileVersion.GetSub1(), profileVersion.GetSub2() + 1);
    return db.SavePropertyString(profileVersionSpec, profileVersion.ToJson());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2018
//---------------------------------------------------------------------------------------
//static
void FileFormatCompatibilityTests::AssertMetaSchemaEnumeration(ECDbCR ecdb, Utf8CP schemaName, Utf8CP enumName)
    {
    ECEnumerationCP expectedEnum = ecdb.Schemas().GetEnumeration(schemaName, enumName);
    ASSERT_TRUE(expectedEnum != nullptr) << schemaName << "." << enumName;

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(ecdb, "SELECT e.Name, e.EnumValues FROM meta.ECEnumerationDef e JOIN meta.ECSchemaDef s ON s.ECInstanceId=e.Schema.Id WHERE s.Name=? AND e.Name=?")) << schemaName << "." << enumName;
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(1, schemaName, IECSqlBinder::MakeCopy::No)) << schemaName << "." << enumName;
    ASSERT_EQ(ECSqlStatus::Success, stmt.BindText(2, enumName, IECSqlBinder::MakeCopy::No)) << schemaName << "." << enumName;

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << schemaName << "." << enumName;

    ASSERT_STREQ(enumName, stmt.GetValueText(0)) << schemaName << "." << enumName;

    IECSqlValue const& enumValues = stmt.GetValue(1);
    ASSERT_EQ((int) expectedEnum->GetEnumeratorCount(), enumValues.GetArrayLength()) << schemaName << "." << enumName;
    auto expectedEnumeratorIt = expectedEnum->GetEnumerators().begin();
    for (IECSqlValue const& enumValue : enumValues.GetArrayIterable())
        {
        ECEnumeratorCP expectedEnumerator = *expectedEnumeratorIt;

        ASSERT_STREQ(expectedEnumerator->GetName().c_str(), enumValue["Name"].GetText()) << schemaName << "." << enumName;
        if (expectedEnumerator->IsInteger())
            ASSERT_EQ(expectedEnumerator->GetInteger(), enumValue["IntValue"].GetInt()) << schemaName << "." << enumName;
        else
            ASSERT_STREQ(expectedEnumerator->GetString().c_str(), enumValue["StringValue"].GetText()) << schemaName << "." << enumName;

        if (expectedEnumerator->GetIsDisplayLabelDefined())
            ASSERT_STREQ(expectedEnumerator->GetInvariantDisplayLabel().c_str(), enumValue["DisplayLabel"].GetText()) << schemaName << "." << enumName;
        else
            ASSERT_TRUE(enumValue["DisplayLabel"].IsNull()) << schemaName << "." << enumName;

        if (!expectedEnumerator->GetInvariantDescription().empty())
            ASSERT_STREQ(expectedEnumerator->GetInvariantDescription().c_str(), enumValue["Description"].GetText()) << schemaName << "." << enumName;
        else
            ASSERT_TRUE(enumValue["Description"].IsNull()) << schemaName << "." << enumName;

        ++expectedEnumeratorIt;
        }

    ASSERT_EQ(BE_SQLITE_DONE, stmt.Step()) << schemaName << "." << enumName;
    }

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
            if (options == CompareOptions::IgnoreDescriptions && BeStringUtilities::StricmpAscii("description", colName) == 0)
                {
                //if col types differ only by one being null the other being text, the test will ignore and continue.
                //if the col types differ otherwise, it is still considered a major difference and false is returned.
                if (benchmarkColType != actualColType)
                    {
                    if ((benchmarkColType == DbValueType::NullVal || benchmarkColType == DbValueType::TextVal) &&
                        (actualColType == DbValueType::NullVal || actualColType == DbValueType::TextVal))
                        continue;
                    }
                }

            if (benchmarkColType != actualColType)
                {
                EXPECT_EQ(benchmarkColType, actualColType) << "Table: " << tableName << " Col: " << colName << " Row: " << rowCount;
                return false;
                }

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

    if (SUCCESS != m_ecdb.Schemas().ImportSchemas(ctx->GetCache().GetSchemas(), SchemaManager::SchemaImportOptions::DisallowMajorSchemaUpgrade))
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

