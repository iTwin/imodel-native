using System.Collections.Generic;
using System.Data;
using Bentley.Exceptions;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using NUnit.Framework;

namespace IndexECPlugin.Tests.Tests
    {

    [TestFixture]
    class SQLServerMergeUpsertStatementBuilderTests
        {
        private const string tableName = "AwesomeTable";
        private const string firstColumnName = "Description";
        private const string secondColumnName = "SpatialData";
        private const string thirdColumnName = "BinaryData";
        private const string firstRowDescription = "first row description";
        private const string secondRowDescription = "second row description";
        private const string thirdRowDescription = "third row description";
        private const string binaryData = "insert binary data here";
        private const string jsonPolygonModel = "{ \"points\" : [[-50.24,-10.5],[28.17,-5.6],[-13.42,10.77]], \"coordinate_system\" : \"1234\" }";
        private const string onStatement = "t.Description = s.Description";
        private const string matchedStatement = "t.SpatialData <> NULL";
        private const string notMatchedStatement = "s.SpatialData = NUL";
        private string STGeomFromText;

        private Dictionary<string, object> firstRow;
        private Dictionary<string, object> secondRow;
        private Dictionary<string, object> thirdRow;

        private SQLServerMergeUpsertStatementBuilder builder;
        private SqlServerParamNameValueMap expectedParamNameValueMap;
        private IParamNameValueMap actualParamNameValueMap;

        private string firstPartOfExpectedSqlStatement;
        private string secondPartOfSqlStatement;
        private string thirdPartOfSqlStatement;

        [TestFixtureSetUp]
        public void SetUpFixture ()
            {
            firstRow = new Dictionary<string, object>() { { firstColumnName, firstRowDescription },
                                                          { secondColumnName, jsonPolygonModel },
                                                          { thirdColumnName, binaryData } };

            //data for second column missing, value should be set to NULL in the paramNameValueMap
            secondRow = new Dictionary<string, object>() { { firstColumnName, secondRowDescription },
                                                           { thirdColumnName, binaryData } };

            //data for second column missing, value should be set to NULL in the paramNameValueMap
            thirdRow = new Dictionary<string, object>() { { firstColumnName, thirdRowDescription },
                                                          { thirdColumnName, binaryData },
                                                          { "extra column", "shouldn't be considered"}};

            STGeomFromText = DbGeometryHelpers.CreateSTGeomFromTextStringFromJson(jsonPolygonModel);

            expectedParamNameValueMap = new SqlServerParamNameValueMap();
            expectedParamNameValueMap.AddParamNameValue("@p1", firstRowDescription, SqlDbType.NVarChar);
            expectedParamNameValueMap.AddParamNameValue("@p3", binaryData, SqlDbType.VarBinary);

            expectedParamNameValueMap.AddParamNameValue("@p4", "second row description", SqlDbType.NVarChar);
            expectedParamNameValueMap.AddParamNameValue("@p5", null, SqlDbType.Udt);
            expectedParamNameValueMap.AddParamNameValue("@p6", binaryData, SqlDbType.VarBinary);

            expectedParamNameValueMap.AddParamNameValue("@p7", "third row description", SqlDbType.NVarChar);
            expectedParamNameValueMap.AddParamNameValue("@p8", null, SqlDbType.Udt);
            expectedParamNameValueMap.AddParamNameValue("@p9", binaryData, SqlDbType.VarBinary);
            }

        [SetUp]
        public void SetUp ()
            {
            builder = new SQLServerMergeUpsertStatementBuilder();
            actualParamNameValueMap = null;
            firstPartOfExpectedSqlStatement = "MERGE " + tableName + " AS " + builder.TargetTableAlias + " USING (VALUES (@p1," + STGeomFromText +
                                              ",@p3),(@p4,@p5,@p6),(@p7,@p8,@p9)) AS " + builder.SourceTableAlias + " (" + firstColumnName + "," +
                                              secondColumnName + "," + thirdColumnName + ") ON " + onStatement + " WHEN MATCHED ";
            secondPartOfSqlStatement = "THEN UPDATE SET " + firstColumnName + "=" + builder.SourceTableAlias + "." + firstColumnName + "," +
                                       secondColumnName + "=" + builder.SourceTableAlias + "." + secondColumnName + "," + thirdColumnName + "=" +
                                       builder.SourceTableAlias + "." + thirdColumnName + " WHEN NOT MATCHED ";
            thirdPartOfSqlStatement = "THEN INSERT (" + firstColumnName + "," + secondColumnName + "," + thirdColumnName + ") VALUES (" +
                                      builder.SourceTableAlias + "." + firstColumnName + "," + builder.SourceTableAlias + "." + secondColumnName + "," +
                                      builder.SourceTableAlias + "." + thirdColumnName + ");";
            }

        [Test]
        public void SetMergeTableNameExceptionTest ()
            {
            Assert.That(() => builder.SetMergeTableName(" "), Throws.TypeOf<ProgrammerException>());
            Assert.That(() => builder.SetMergeTableName(null), Throws.TypeOf<ProgrammerException>());
            }

        [Test]
        public void AddColumnNameExceptionTest ()
            {
            builder.EndSettingColumnNames();
            Assert.That(() => builder.AddColumnName(firstColumnName, Bentley.ECObjects.ECObjects.StringType), Throws.TypeOf<ProgrammerException>());
            }

        [Test]
        public void AddSpatialColumnNameExceptionTest ()
            {
            builder.EndSettingColumnNames();
            Assert.That(() => builder.AddSpatialColumnName(firstColumnName), Throws.TypeOf<ProgrammerException>());
            }

        [Test]
        public void AddBinaryColumnNameExceptionTest ()
            {
            builder.EndSettingColumnNames();
            Assert.That(() => builder.AddBinaryColumnName(firstColumnName), Throws.TypeOf<ProgrammerException>());
            }

        [Test]
        public void AddRowExceptionTest ()
            {
            Dictionary<string, object> rows = new Dictionary<string, object>() { { "key", "value" } };

            Assert.That(() => builder.AddRow(rows), Throws.TypeOf<ProgrammerException>());
            }

        [Test]
        public void CreateSqlStatementEndSettingColumnNamesNotCalledExceptionTest ()
            {
            Assert.That(() => builder.CreateSqlStatement(out actualParamNameValueMap), Throws.TypeOf<ProgrammerException>().
                With.Message.EqualTo("CreateSqlStatement cannot be called before EndSettingColumnNames has been called."));
            }

        [Test]
        public void CreateSqlStatementInvalidTargetTableNameExceptionTest ()
            {
            builder.EndSettingColumnNames();

            Assert.That(() => builder.CreateSqlStatement(out actualParamNameValueMap), Throws.TypeOf<ProgrammerException>().
                With.Message.EqualTo("The merge statement cannot be created before the table name has been set."));
            }

        [Test]
        public void CreateSqlStatementWithOutRowsExceptionTest ()
            {
            builder.EndSettingColumnNames();
            builder.SetMergeTableName(tableName);

            Assert.That(() => builder.CreateSqlStatement(out actualParamNameValueMap), Throws.TypeOf<ProgrammerException>().
                With.Message.EqualTo("The merge statement cannot be created if there are no rows added."));
            }

        [Test]
        public void CreateSqlStatementInvalidOnStatementExceptionTest ()
            {
            Dictionary<string, object> rows = new Dictionary<string, object>() { { "column name", "value" } };
            builder.AddColumnName("column name", Bentley.ECObjects.ECObjects.StringType);
            builder.EndSettingColumnNames();
            builder.SetMergeTableName(tableName);
            builder.AddRow(rows);

            Assert.That(() => builder.CreateSqlStatement(out actualParamNameValueMap), Throws.TypeOf<ProgrammerException>().
                With.Message.EqualTo("The merge statement cannot be created if there is no On statement set."));
            }

        [Test]
        public void CreateSqlStatementNoMatchedStatementNoNotMatchedStatementTest ()
            {
            SetUpBuilder();

            Assert.That(builder.CreateSqlStatement(out actualParamNameValueMap), Is.EqualTo(firstPartOfExpectedSqlStatement + secondPartOfSqlStatement +
                                                                                            thirdPartOfSqlStatement));
            CompareParamNameValueMap(actualParamNameValueMap as SqlServerParamNameValueMap, expectedParamNameValueMap);
            }

        [Test]
        public void CreateSqlStatementWithMatchedStatementAndNotMatchedStatementTest ()
            {
            SetUpBuilder();
            builder.SetMatchedStatement(matchedStatement);
            builder.SetNotMatchedStatement(notMatchedStatement);

            Assert.That(builder.CreateSqlStatement(out actualParamNameValueMap), Is.EqualTo(firstPartOfExpectedSqlStatement + "AND (" + matchedStatement +
                ") " + secondPartOfSqlStatement + "AND (" + notMatchedStatement + ") " + thirdPartOfSqlStatement));

            CompareParamNameValueMap(actualParamNameValueMap as SqlServerParamNameValueMap, expectedParamNameValueMap);
            }

        private void SetUpBuilder ()
            {
            builder.AddColumnName(firstColumnName, Bentley.ECObjects.ECObjects.StringType);
            builder.AddSpatialColumnName(secondColumnName);
            builder.AddBinaryColumnName(thirdColumnName);
            builder.EndSettingColumnNames();
            builder.SetMergeTableName(tableName);
            builder.AddRow(firstRow);
            builder.AddRow(secondRow);
            builder.AddRow(thirdRow);
            builder.SetOnStatement(onStatement);
            }

        private static void CompareParamNameValueMap (SqlServerParamNameValueMap actual, SqlServerParamNameValueMap expected)
            {
            Assert.That(actual.Count == expected.Count);
            foreach ( string key in expected.Keys )
                {
                Assert.That(expected[key].Equals(actual[key]));
                }
            }
        }
    }
