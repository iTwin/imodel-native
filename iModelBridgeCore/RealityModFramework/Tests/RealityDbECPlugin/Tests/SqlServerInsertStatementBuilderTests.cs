using System;
using System.Collections.Generic;
using System.Data;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using IndexECPlugin.Source;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;

namespace IndexECPlugin.Tests.Tests
    {
    [TestFixture]
    class SqlServerInsertStatementBuilderTests
        {

        IECSchema m_schema;

        [SetUp]
        public void SetUp ()
            {
            m_schema = SetupHelpers.PrepareSchema();
            }
        
        [Test]
        public void SimpleCreateSqlStatementTest()
            {
            IECClass spatialEntityClass = m_schema.GetClass("SpatialEntity");
            IECType stringType = spatialEntityClass["Id"].Type;

            var statementBuilder = new SQLServerInsertStatementBuilder();

            string TableName = "TestTableName";
            string ColumnName1 = "TestColumnName1";
            string ColumnName2 = "TestColumnName2";
            string ColumnName3 = "TestColumnName3";
            string ColumnName4 = "TestBinaryColumnName";
            string ColumnName5 = "TestSpatialColumnName";

            statementBuilder.SetInsertIntoTableName(TableName);
            statementBuilder.AddColumnName(ColumnName1, SqlDbType.Int);
            statementBuilder.AddColumnName(ColumnName2, stringType);
            statementBuilder.AddColumnName(ColumnName3, stringType);

            statementBuilder.AddBinaryColumnName(ColumnName4);
            statementBuilder.AddSpatialColumnName(ColumnName5);

            statementBuilder.EndSettingColumnNames();

            Dictionary<string, object> rowValues = new Dictionary<string, object>();

            int intVal = 1;
            string stringVal = "1234";
            string nullStringVal = null;

            rowValues.Add(ColumnName1, intVal);
            rowValues.Add(ColumnName2, stringVal);
            rowValues.Add(ColumnName3, nullStringVal);

            byte[] binaryData = Encoding.ASCII.GetBytes("TestBinaryData");

            rowValues.Add(ColumnName4, binaryData);

            rowValues.Add(ColumnName5, "{points:[[0,0],[0,1],[1,1],[1,0],[0,0]],coordinate_system:'4326'}");

            statementBuilder.AddRow(rowValues, null);

            IParamNameValueMap paramNameValueMap;

            string sqlStatement = statementBuilder.CreateSqlStatement(out paramNameValueMap);

            Assert.IsTrue(sqlStatement.Contains(TableName), "The SQL statement doesn't contain the table name.");
            Assert.IsTrue(sqlStatement.Contains(ColumnName1), "The SQL statement doesn't contain the column names.");
            Assert.IsTrue(sqlStatement.Contains(ColumnName2), "The SQL statement doesn't contain the column names.");
            Assert.IsTrue(sqlStatement.Contains(ColumnName3), "The SQL statement doesn't contain the column names.");
            Assert.IsTrue(sqlStatement.Contains(ColumnName4), "The SQL statement doesn't contain the column names.");
            Assert.IsTrue(sqlStatement.Contains(ColumnName5), "The SQL statement doesn't contain the column names.");

            Assert.IsTrue(paramNameValueMap is SqlServerParamNameValueMap, "The ParamNameValueMap should be of type SqlServerParamNameValueMap.");

            var sqlServerParamNameValueMap = paramNameValueMap as SqlServerParamNameValueMap;

            Assert.IsTrue(sqlServerParamNameValueMap.Any(row => row.Value.Item1 is int && intVal == (int) row.Value.Item1));
            Assert.IsTrue(sqlServerParamNameValueMap.Any(row => row.Value.Item1 is string && stringVal == (string) row.Value.Item1));
            Assert.IsTrue(sqlServerParamNameValueMap.Any(row => row.Value.Item1 is byte[] && binaryData == (byte[]) row.Value.Item1));
            //Spatial data should not be in the map
            }

        [Test]
        public void CreateSqlStatementWithDeleteTest ()
            {
            IECClass spatialEntityClass = m_schema.GetClass("SpatialEntity");
            IECType stringType = spatialEntityClass["Id"].Type;

            var statementBuilder = new SQLServerInsertStatementBuilder();

            string TableName = "TestTableName";
            string ColumnName1 = "TestColumnName1";
            string ColumnName2 = "TestColumnName2";
            string ColumnName3 = "TestColumnName3";
            string ColumnName4 = "TestBinaryColumnName";
            string ColumnName5 = "TestSpatialColumnName";

            statementBuilder.SetInsertIntoTableName(TableName);
            statementBuilder.AddColumnName(ColumnName1, SqlDbType.Int);
            statementBuilder.AddColumnName(ColumnName2, stringType);
            statementBuilder.AddColumnName(ColumnName3, stringType);

            statementBuilder.AddBinaryColumnName(ColumnName4);
            statementBuilder.AddSpatialColumnName(ColumnName5);

            statementBuilder.ActivateDeleteBeforeInsert();
            string whereStatementParamVal = "helloWorld";
            var whereStatementManager = new WhereStatementManager();
            whereStatementManager.WhereStatement = "TestCol = @TestVal@";
            whereStatementManager.AddParameter("@TestVal@", stringType, "helloWorld");

            statementBuilder.EndSettingColumnNames();

            Dictionary<string, object> rowValues = new Dictionary<string, object>();

            int intVal = 1;
            string stringVal = "1234";
            string nullStringVal = null;

            rowValues.Add(ColumnName1, intVal);
            rowValues.Add(ColumnName2, stringVal);
            rowValues.Add(ColumnName3, nullStringVal);

            byte[] binaryData = Encoding.ASCII.GetBytes("TestBinaryData");

            rowValues.Add(ColumnName4, binaryData);

            rowValues.Add(ColumnName5, "{points:[[0,0],[0,1],[1,1],[1,0],[0,0]],coordinate_system:'4326'}");

            statementBuilder.AddRow(rowValues, whereStatementManager);
            statementBuilder.AddRow(rowValues, whereStatementManager);

            IParamNameValueMap paramNameValueMap;

            string sqlStatement = statementBuilder.CreateSqlStatement(out paramNameValueMap);

            Assert.IsTrue(sqlStatement.Contains(TableName), "The SQL statement doesn't contain the table name.");
            Assert.IsTrue(sqlStatement.Contains(ColumnName1), "The SQL statement doesn't contain the column names.");
            Assert.IsTrue(sqlStatement.Contains(ColumnName2), "The SQL statement doesn't contain the column names.");
            Assert.IsTrue(sqlStatement.Contains(ColumnName3), "The SQL statement doesn't contain the column names.");
            Assert.IsTrue(sqlStatement.Contains(ColumnName4), "The SQL statement doesn't contain the column names.");
            Assert.IsTrue(sqlStatement.Contains(ColumnName5), "The SQL statement doesn't contain the column names.");

            Assert.IsTrue(paramNameValueMap is SqlServerParamNameValueMap, "The ParamNameValueMap should be of type SqlServerParamNameValueMap.");

            var sqlServerParamNameValueMap = paramNameValueMap as SqlServerParamNameValueMap;

            Assert.IsTrue(sqlServerParamNameValueMap.Any(row => row.Value.Item1 is int && intVal == (int) row.Value.Item1));
            Assert.IsTrue(sqlServerParamNameValueMap.Any(row => row.Value.Item1 is string && stringVal == (string) row.Value.Item1));
            Assert.IsTrue(sqlServerParamNameValueMap.Any(row => row.Value.Item1 is byte[] && binaryData == (byte[]) row.Value.Item1));

            Assert.IsTrue(sqlServerParamNameValueMap.Any(row => row.Value.Item1 is string && whereStatementParamVal == (string) row.Value.Item1));

            //Spatial data should not be in the map
            }

        [Test]
        public void AddColumnError ()
            {
            var statementBuilder = new SQLServerInsertStatementBuilder();

            IECClass spatialEntityClass = m_schema.GetClass("SpatialEntity");
            IECType stringType = spatialEntityClass["Id"].Type;

            string ColumnName1 = "TestColumnName1";

            statementBuilder.EndSettingColumnNames();

            Assert.Throws<ProgrammerException>(() => statementBuilder.AddColumnName(ColumnName1, SqlDbType.Int), "AddColumnName should not allow to be called after EndSettingColumnNames.");
            Assert.Throws<ProgrammerException>(() => statementBuilder.AddColumnName(ColumnName1, stringType), "AddColumnName should not allow to be called after EndSettingColumnNames.");
            Assert.Throws<ProgrammerException>(() => statementBuilder.AddSpatialColumnName(ColumnName1), "AddSpatialColumnName should not allow to be called after EndSettingColumnNames.");
            Assert.Throws<ProgrammerException>(() => statementBuilder.AddBinaryColumnName(ColumnName1), "AddBinaryColumnName should not allow to be called after EndSettingColumnNames.");
            Assert.Throws<ProgrammerException>(() => statementBuilder.ActivateDeleteBeforeInsert(), "ActivateDeleteBeforeInsert should not allow to be called after EndSettingColumnNames.");
            
            }

        [Test]
        public void AddRowError ()
            {
            var statementBuilder = new SQLServerInsertStatementBuilder();
            Dictionary<string, object> rowValues = new Dictionary<string, object>();

            statementBuilder.SetInsertIntoTableName("TestTableName");
            rowValues.Add("TestColumnName1", 1);

            Assert.Throws<ProgrammerException>(() => statementBuilder.AddRow(rowValues, null), "AddRow should not allow to be called before EndSettingColumnNames.");
            }

        [Test]
        public void CreateSqlStatementBeforeEndError ()
            {
            var statementBuilder = new SQLServerInsertStatementBuilder();

            IParamNameValueMap paramNameValueMap;
            statementBuilder.SetInsertIntoTableName("TestTableName");
            statementBuilder.AddColumnName("TestColumnName1", SqlDbType.Int);

            string sqlStatement;

            Assert.Throws<ProgrammerException>(() => sqlStatement = statementBuilder.CreateSqlStatement(out paramNameValueMap), "CreateSqlStatement should not allow to be called before EndSettingColumnNames.");
            }

        [Test]
        public void CreateSqlStatementNoAddedRowError ()
            {
            var statementBuilder = new SQLServerInsertStatementBuilder();

            IParamNameValueMap paramNameValueMap;
            statementBuilder.SetInsertIntoTableName("TestTableName");
            statementBuilder.AddColumnName("TestColumnName1", SqlDbType.Int);
            statementBuilder.EndSettingColumnNames();

            string sqlStatement;

            Assert.Throws<ProgrammerException>(() => sqlStatement = statementBuilder.CreateSqlStatement(out paramNameValueMap), "CreateSqlStatement should not allow to be called before at least one AddRow");
            }

        [Test]
        public void CreateSqlStatementBeforeSetInsertIntoTableNameError ()
            {
            var statementBuilder = new SQLServerInsertStatementBuilder();
            IParamNameValueMap paramNameValueMap;

            string ColumnName1 = "TestColumnName1";
            statementBuilder.AddColumnName(ColumnName1, SqlDbType.Int);
            statementBuilder.EndSettingColumnNames();

            Dictionary<string, object> rowValues = new Dictionary<string, object>();

            rowValues.Add(ColumnName1, 1);
            statementBuilder.AddRow(rowValues, null);

            string sqlStatement;

            Assert.Throws<ProgrammerException>(() => sqlStatement = statementBuilder.CreateSqlStatement(out paramNameValueMap), "AddRow should not allow to be called before SetInsertIntoTableName.");
            }

        [Test]
        public void CreateSqlStatementTestInsertTableError ()
            {
            var statementBuilder = new SQLServerInsertStatementBuilder();

            string TableName = "";

            statementBuilder.EndSettingColumnNames();

            Assert.Throws<ProgrammerException>(() => statementBuilder.SetInsertIntoTableName(TableName), "SetInsertIntoTableName should not allow empty table names");
            Assert.Throws<ProgrammerException>(() => statementBuilder.SetInsertIntoTableName(null), "SetInsertIntoTableName should not allow null table names");

            }

        }
    }
