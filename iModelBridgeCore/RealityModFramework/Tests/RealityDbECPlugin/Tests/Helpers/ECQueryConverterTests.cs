#define BBOXQUERY

using System.Collections.Generic;
using System.Linq;
using System.Text.RegularExpressions;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
//using Microsoft.Data.Schema.ScriptDom;
//using Microsoft.Data.Schema.ScriptDom.Sql;


namespace IndexECPlugin.Tests.Tests.Helpers
    {
    [TestFixture]
    class ECQueryConverterTests
        {
        IECSchema m_schema;


        [SetUp]
        public void SetUp ()
            {
            m_schema = SetupHelpers.PrepareSchema();
            }

        [Test]
        public void SimpleQueryTest ()
            {
            IECClass SpatialEntityClass = m_schema.GetClass("SpatialEntity");
            ECQuery query = new ECQuery(SpatialEntityClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(SpatialEntityClass["Id"]);

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);            

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;
            List<IECProperty> propList;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap, out propList);

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            //SELECT tab0.IdStr FROM dbo.SpatialEntitys tab0  ;

            string idColumnString = SpatialEntityClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string tableString = SpatialEntityClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            //Assert.IsTrue(sqlCommand.Contains(idColumnString));
            //Assert.IsTrue(sqlCommand.Contains(tableString));

            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(tableString) + @".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            Assert.IsTrue(query.SelectClause.SelectedProperties.All(p => propList.Exists(p2 => p == p2)));
            }

        [Test]
        public void QueryNoInstanceIdProperty()
            {
            IECClass SpatialEntityClass = m_schema.GetClass("SpatialEntity");
            ECQuery query = new ECQuery(SpatialEntityClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(SpatialEntityClass["Name"]);

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;
            List<IECProperty> propList;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap, out propList);

            string idColumnString = SpatialEntityClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string tableString = SpatialEntityClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(tableString) + @".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            }

        [Test]
        public void QueryNonDatabaseProperty ()
            {
            IECClass spatialDataSource = m_schema.GetClass("SpatialDataSource");
            ECQuery query = new ECQuery(spatialDataSource);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(spatialDataSource["Metadata"]);

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;
            List<IECProperty> propList;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap, out propList);

            string idColumnString = spatialDataSource["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string tableString = spatialDataSource.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(tableString) + @".*");
            Assert.IsFalse(sqlCommand.Contains(spatialDataSource["Metadata"].Name), "A property which is not kept in the database should not be in the sql query.");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            }

        [Test]
        public void QueryUnqueriableClass()
            {
            IECClass PreparedPackageClass = m_schema.GetClass("PreparedPackage");
            ECQuery query = new ECQuery(PreparedPackageClass);

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;
            List<IECProperty> propList;

            Assert.Throws<UserFriendlyException>(() => ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap, out propList));
            }

        [Test]
        public void QueryDerivedClassWithBasePropertyWhereClause()
            {
            IECClass OsmSourceClass = m_schema.GetClass("OsmSource");
            IECClass SpatialDataSourceClass = m_schema.GetClass("SpatialDataSource");
            ECQuery query = new ECQuery(OsmSourceClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(OsmSourceClass["Id"]);
            query.WhereClause = new WhereCriteria(new PropertyExpression(RelationalOperator.EQ, OsmSourceClass["MainURL"], "Test"));
            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;
            List<IECProperty> propList;
            //SELECT TOP 1000 tab0.IdStr FROM dbo.OsmSources tab0 LEFT JOIN dbo.SpatialDataSources tab1 ON tab0.IdStr = tab1.IdStr  WHERE  ( tab1.MainURL =@param0  )  ;
            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap, out propList);

            string idColumnString = OsmSourceClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string tableString = OsmSourceClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");
            string joinedTableString = SpatialDataSourceClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            string firstKeyPoly = SpatialDataSourceClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
            string secondKeyPoly = OsmSourceClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
            string firstOrSecondPoly = "(" + firstKeyPoly + @".*=.*" + secondKeyPoly + "|" + secondKeyPoly + @".*=.*" + firstKeyPoly + ")";

            string mainUrlPropColumnString = OsmSourceClass["MainURL"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(tableString) + ".*LEFT JOIN.*" + Regex.Escape(joinedTableString) + ".*ON.*" + firstOrSecondPoly + @".* WHERE .*" + Regex.Escape(mainUrlPropColumnString) + @".*");

            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            }

        [Test]
        public void OrderByClauseTest()
            {
            IECClass SpatialDataSourceClass = m_schema.GetClass("SpatialDataSource");
            ECQuery query = new ECQuery(SpatialDataSourceClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(SpatialDataSourceClass["Id"]);
            query.OrderBy = new OrderByCriterion();
            query.OrderBy.Add(new OrderByItem(SpatialDataSourceClass["Id"].Name, true));
            query.OrderBy.Add(new OrderByItem(SpatialDataSourceClass["MainURL"].Name, false));
            query.OrderBy.Add(new OrderByItem(SpatialDataSourceClass["Metadata"].Name, false));
            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;
            List<IECProperty> propList;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap, out propList);

            string idColumnString = SpatialDataSourceClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string mainURLColumnString = SpatialDataSourceClass["MainURL"].GetCustomAttributes("DBColumn").GetString("ColumnName");

            Assert.IsTrue(sqlCommand.Contains("ORDER BY"));
            Assert.IsFalse(sqlCommand.Contains(SpatialDataSourceClass["Metadata"].Name), "A property which is not kept in the database should not be in the sql query.");
            Regex reg = new Regex(@".*" + idColumnString + @"\s*ASC.*");
            Regex reg2 = new Regex(@".*" + mainURLColumnString + @"\s*DESC.*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            Assert.IsTrue(reg2.IsMatch(sqlCommand), "The query does not have the required form.");
            }

        [Test]
        public void PropertyExpressionCriteriaQueryTest ()
            {
            IECClass SpatialEntityClass = m_schema.GetClass("SpatialEntity");
            ECQuery query = new ECQuery(SpatialEntityClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(SpatialEntityClass["Id"]);

            query.WhereClause = new WhereCriteria(new PropertyExpression(RelationalOperator.EQ, SpatialEntityClass["Name"], "Test"));

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;
            List<IECProperty> propList;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap, out propList);
            GenericParamNameValueMap genericParamNameValueMap = paramNameValueMap as GenericParamNameValueMap;

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            //SELECT tab0.IdStr FROM dbo.SpatialEntitys tab0  WHERE  ( tab0.Name =@param0  )  ;

            string idColumnString = SpatialEntityClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string nameColumnString = SpatialEntityClass["Name"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string tableString = SpatialEntityClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            Assert.IsNotNull(genericParamNameValueMap, "The ParamNameValueMap was not a GenericParamNameValueMap.");
            Assert.AreEqual(1, genericParamNameValueMap.Count(), "Invalid number of parameters in the map.");
            Assert.AreEqual("Test", genericParamNameValueMap.First().Value.Item1, "The map does not contain the value of the property requested in the criteria.");

            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(tableString) + @".* WHERE .*" + Regex.Escape(nameColumnString) + @".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            Assert.IsTrue(query.SelectClause.SelectedProperties.All(p => propList.Exists(p2 => p == p2)));
            }

        [Test]
        public void ECInstanceIdExpressionCriteriaQueryTest ()
            {
            IECClass SpatialEntityClass = m_schema.GetClass("SpatialEntity");
            ECQuery query = new ECQuery(SpatialEntityClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(SpatialEntityClass["Id"]);

            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("1"));

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;
            List<IECProperty> propList;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap, out propList);
            GenericParamNameValueMap genericParamNameValueMap = paramNameValueMap as GenericParamNameValueMap;
            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            //SELECT tab0.IdStr FROM dbo.SpatialEntitys tab0  WHERE  (  ( tab0.IdStr =@param0  )  )  ORDER BY tab0.IdStr ASC ;

            string idColumnString = SpatialEntityClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string tableString = SpatialEntityClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            Assert.IsNotNull(genericParamNameValueMap, "The ParamNameValueMap was not a GenericParamNameValueMap.");
            Assert.AreEqual(1, genericParamNameValueMap.Count(), "Invalid number of parameters in the map.");
            Assert.AreEqual("1", genericParamNameValueMap.First().Value.Item1, "The map does not contain the value of the ID requested.");

            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(tableString) + @".* WHERE .*" + Regex.Escape(idColumnString) + @".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            Assert.IsTrue(query.SelectClause.SelectedProperties.All(p => propList.Exists(p2 => p == p2)));
            }

        [Test]
        public void MultipleECInstanceIdExpressionCriteriaQueryTest ()
            {
            IECClass SpatialEntityClass = m_schema.GetClass("SpatialEntity");
            ECQuery query = new ECQuery(SpatialEntityClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(SpatialEntityClass["Id"]);

            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("1", "2"));

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;
            List<IECProperty> propList;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap, out propList);
            GenericParamNameValueMap genericParamNameValueMap = paramNameValueMap as GenericParamNameValueMap;
            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            //SELECT tab0.IdStr FROM dbo.SpatialEntitys tab0  WHERE  (  ( tab0.IdStr =@param0  )  )  ORDER BY tab0.IdStr ASC ;

            string idColumnString = SpatialEntityClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string tableString = SpatialEntityClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            Assert.IsNotNull(genericParamNameValueMap, "The ParamNameValueMap was not a GenericParamNameValueMap.");
            Assert.AreEqual(2, genericParamNameValueMap.Count(), "Invalid number of parameters in the map.");
            Assert.IsTrue(genericParamNameValueMap.Any(g => (string) g.Value.Item1 == "1"), "The map does not contain the value of the ID requested.");
            Assert.IsTrue(genericParamNameValueMap.Any(g => (string) g.Value.Item1 == "2"), "The map does not contain the value of the ID requested.");

            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(tableString) + @".* WHERE .*" + Regex.Escape(idColumnString) + @".* OR .*" + Regex.Escape(idColumnString) +@".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            Assert.IsTrue(query.SelectClause.SelectedProperties.All(p => propList.Exists(p2 => p == p2)));
            }

        [Test]
        public void ComplexCriteriaQueryTest ()
            {
            IECClass SpatialEntityClass = m_schema.GetClass("SpatialEntity");
            ECQuery query = new ECQuery(SpatialEntityClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(SpatialEntityClass["Id"]);

            // The where criteria will be equivalent to (Id == 1 || Id == 2) && (Id == 1) 
            WhereCriteria internalCriteria = new WhereCriteria(new ECInstanceIdExpression("1"));
            internalCriteria.Add(new ECInstanceIdExpression("2"));
            internalCriteria.SetLogicalOperatorAfter(0, LogicalOperator.OR);
            query.WhereClause = new WhereCriteria(internalCriteria);
            query.WhereClause.Add((new ECInstanceIdExpression("1")));
            query.WhereClause.SetLogicalOperatorAfter(0, LogicalOperator.AND);


            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;
            List<IECProperty> propList;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap, out propList);
            GenericParamNameValueMap genericParamNameValueMap = paramNameValueMap as GenericParamNameValueMap;
            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            //SELECT tab0.IdStr FROM dbo.SpatialEntitys tab0  WHERE  (  (  ( tab0.IdStr =@param0  ) OR  ( tab0.IdStr =@param1  )  ) AND  ( tab0.IdStr =@param2  )  )  ORDER BY tab0.IdStr ASC ;

            string idColumnString = SpatialEntityClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string tableString = SpatialEntityClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            Assert.IsNotNull(genericParamNameValueMap, "The ParamNameValueMap was not a GenericParamNameValueMap.");
            Assert.IsTrue(sqlCommand.Contains(idColumnString), "The query does not contain the name of the Id Column.");
            Assert.IsTrue(sqlCommand.Contains(tableString), "The query does not contain the name of the table queried.");
            Assert.AreEqual(3, genericParamNameValueMap.Count(), "Invalid number of parameters in the map.");

            Regex reg = new Regex(@".*SELECT.*" + Regex.Escape(idColumnString) + @".*FROM.*" + Regex.Escape(tableString) + @".*WHERE.*\(.*" + Regex.Escape(idColumnString) + @".*OR.*" + Regex.Escape(idColumnString) + @".*\).*AND.*" + Regex.Escape(idColumnString) + ".*");

            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            Assert.IsTrue(query.SelectClause.SelectedProperties.All(p => propList.Exists(p2 => p == p2)));
            }

        [Test]
        public void RelatedQueryTest ()
            {
            IECClass SpatialEntityClass = m_schema.GetClass("SpatialEntity");
            IECClass metadataClass = m_schema.GetClass("Metadata");
            IECRelationshipClass SEBToMetadataClass = m_schema.GetClass("SpatialEntityToMetadata") as IECRelationshipClass;
            ECQuery query = new ECQuery(SpatialEntityClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(SpatialEntityClass["Id"]);

            WhereCriteria relatedCriteria = new WhereCriteria(new RelatedCriterion(new QueryRelatedClassSpecifier(SEBToMetadataClass, RelatedInstanceDirection.Forward, metadataClass), new WhereCriteria(new ECInstanceIdExpression("1"))));
            query.WhereClause = relatedCriteria;

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;
            string idColumnString = SpatialEntityClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string metadataIdColumnString = metadataClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string fromTableString = SpatialEntityClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");
            string joinedTableString = metadataClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            string firstKey = SEBToMetadataClass.GetCustomAttributes("RelationshipKeys").GetString("ContainerKey");
            string secondKey = SEBToMetadataClass.GetCustomAttributes("RelationshipKeys").GetString("ContainedKey");
            string firstOrSecond = "(" + firstKey + @".*=.*" + secondKey + "|" + secondKey + @".*=.*" + firstKey + ")";

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;
            List<IECProperty> propList;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap, out propList);

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            // SELECT tab0.IdStr FROM dbo.SpatialEntitys tab0 LEFT JOIN dbo.Metadatas tab1 ON tab0.Metadata_Id = tab1.Id  WHERE  (  (  ( tab1.IdStr =@param0  )  )  )  ORDER BY tab1.IdStr ASC ;


            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(fromTableString) + ".*LEFT JOIN.*" + Regex.Escape(joinedTableString) + ".*ON.*" + firstOrSecond + @".* WHERE .*" + Regex.Escape(metadataIdColumnString) + @".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            Assert.IsTrue(query.SelectClause.SelectedProperties.All(p => propList.Exists(p2 => p == p2)));
            }

        [Test]
        public void RelatedQueryPolymorphicClassesTest ()
            {
            IECClass SpatialEntityClass = m_schema.GetClass("SpatialEntity");
            IECClass SpatialDataSourceClass = m_schema.GetClass("SpatialDataSource");
            IECClass OsmSourceClass = m_schema.GetClass("OsmSource");
            IECRelationshipClass SEBToSDSClass = m_schema.GetClass("SpatialEntityToSpatialDataSource") as IECRelationshipClass;
            ECQuery query = new ECQuery(SpatialEntityClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(SpatialEntityClass["Id"]);

            WhereCriteria relatedCriteria = new WhereCriteria(new RelatedCriterion(new QueryRelatedClassSpecifier(SEBToSDSClass, RelatedInstanceDirection.Forward, OsmSourceClass), new WhereCriteria(new ECInstanceIdExpression("1"))));
            query.WhereClause = relatedCriteria;

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;
            string idColumnString = SpatialEntityClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string osmIdColumnString = OsmSourceClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string fromTableString = SpatialEntityClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");
            string sdsTableString = SpatialDataSourceClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");
            string joinedTableString = OsmSourceClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            string firstKey = SEBToSDSClass.GetCustomAttributes("RelationshipKeys").GetString("ContainerKey");
            string secondKey = SEBToSDSClass.GetCustomAttributes("RelationshipKeys").GetString("ContainedKey");
            string firstOrSecond = "(" + firstKey + @".*=.*" + secondKey + "|" + secondKey + @".*=.*" + firstKey + ")";

            string firstKeyPoly = SpatialDataSourceClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;
            string secondKeyPoly = OsmSourceClass.GetCustomAttributes("SQLEntity").GetPropertyValue("InstanceIDProperty").StringValue;

            string firstOrSecondPoly = "(" + firstKeyPoly + @".*=.*" + secondKeyPoly + "|" + secondKeyPoly + @".*=.*" + firstKeyPoly + ")";

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;
            List<IECProperty> propList;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap, out propList);

            //SELECT TOP 1000 tab0.IdStr FROM dbo.SpatialEntities tab0 LEFT JOIN dbo.SpatialDataSources tab1 ON tab0.Id = tab1.SpatialEntity_Id LEFT JOIN dbo.OsmSources tab2 ON tab1.IdStr = tab2.IdStr  WHERE  (  (  ( tab2.IdStr =@param0  )  )  )  ORDER BY tab2.IdStr ASC ;

            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(fromTableString) + ".*LEFT JOIN.*" + Regex.Escape(sdsTableString) + ".*ON.*" + firstOrSecond + ".*LEFT JOIN.*" + Regex.Escape(joinedTableString) + ".*ON.*" + firstOrSecondPoly + @".* WHERE .*" + Regex.Escape(osmIdColumnString) + @".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            Assert.IsTrue(query.SelectClause.SelectedProperties.All(p => propList.Exists(p2 => p == p2)));
            }

        [Test]
        public void RelatedQueryTestWithRelatedID ()
            {
            IECClass SpatialEntityClass = m_schema.GetClass("SpatialEntity");
            IECClass metadataClass = m_schema.GetClass("Metadata");
            IECRelationshipClass SEBToMetadataClass = m_schema.GetClass("SpatialEntityToMetadata") as IECRelationshipClass;
            ECQuery query = new ECQuery(SpatialEntityClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            var criteria = new RelatedCriterion(new QueryRelatedClassSpecifier(SEBToMetadataClass, RelatedInstanceDirection.Forward, metadataClass), new WhereCriteria(new ECInstanceIdExpression("1")));
            criteria.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("RequestRelatedId", true));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(SpatialEntityClass["Id"]);

            WhereCriteria relatedCriteria = new WhereCriteria(criteria);
            query.WhereClause = relatedCriteria;

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;
            string idColumnString = SpatialEntityClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string metadataIdColumnString = metadataClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string fromTableString = SpatialEntityClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");
            string joinedTableString = metadataClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            string firstKey = SEBToMetadataClass.GetCustomAttributes("RelationshipKeys").GetString("ContainerKey");
            string secondKey = SEBToMetadataClass.GetCustomAttributes("RelationshipKeys").GetString("ContainedKey");
            string firstOrSecond = "(" + firstKey + @".*=.*" + secondKey + "|" + secondKey + @".*=.*" + firstKey + ")";

            string firstOrSecondIdColumn = "(" + Regex.Escape(idColumnString) + @".*,.*" + Regex.Escape(metadataIdColumnString) + "|" + Regex.Escape(metadataIdColumnString) + @".*,.*" + Regex.Escape(idColumnString) + ")";

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;
            List<IECProperty> propList;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap, out propList);

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            //SELECT tab0.IdStr, tab1.IdStr FROM dbo.SpatialEntitys tab0 LEFT JOIN dbo.Metadatas tab1 ON tab0.Metadata_Id = tab1.Id  WHERE  (  (  ( tab1.IdStr =@param0  )  )  )  ORDER BY tab1.IdStr ASC ;


            Regex reg = new Regex(@".*SELECT .*" + firstOrSecondIdColumn + @".* FROM .*" + Regex.Escape(fromTableString) + ".*LEFT JOIN.*" + Regex.Escape(joinedTableString) + ".*ON.*" + firstOrSecond + @".* WHERE .*" + Regex.Escape(metadataIdColumnString) + @".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            Assert.IsTrue(query.SelectClause.SelectedProperties.All(p => propList.Exists(p2 => p == p2)));
            }

        [Test]
        public void SpatialQueryTest ()
            {
            IECClass SpatialEntityClass = m_schema.GetClass("SpatialEntity");
            ECQuery query = new ECQuery(SpatialEntityClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(SpatialEntityClass["Id"]);

            PolygonDescriptor polygonDescriptor = new PolygonDescriptor();
            polygonDescriptor.WKT = "POLYGON ((30 10, 40 40, 20 40, 10 20, 30 10))";
            polygonDescriptor.SRID = 4326;

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, polygonDescriptor, m_schema, false);

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;
            List<IECProperty> propList;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap, out propList);

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            string idColumnString = SpatialEntityClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string fromTableString = SpatialEntityClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            //SELECT tab0.IdStr FROM dbo.SpatialEntitys tab0  WHERE tab0.Footprint.STIntersects(geometry::STGeomFromText('POLYGON ((30 10, 40 40, 20 40, 10 20, 30 10))',4326)) = 'true' ;
#if (BBOXQUERY)
            Regex reg = new Regex(@".*SELECT.*" + Regex.Escape(idColumnString) + @".*FROM.*" + Regex.Escape(fromTableString) + @".*WHERE(.*(MinX|MaxX|MinY|MaxY)){4}.*");
#else
            Regex reg = new Regex(@".*SELECT.*" + Regex.Escape(idColumnString) + @".*FROM.*" + Regex.Escape(fromTableString) + @".*WHERE.*STIntersects.*\('POLYGON \(\(30 10, 40 40, 20 40, 10 20, 30 10\)\)',4326\).*");
#endif
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            }

        [Test]
        public void PolymorphicClassQueryTest ()
            {
            IECClass multibandSource = m_schema.GetClass("MultibandSource");
            ECQuery query = new ECQuery(multibandSource);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(multibandSource["Id"]);
            query.SelectClause.SelectedProperties.Add(multibandSource["MainURL"]);
            query.SelectClause.SelectedProperties.Add(multibandSource["RedBandURL"]);

            //WhereCriteria internalCriteria = new WhereCriteria(new RelatedCriterion(new QueryRelatedClassSpecifier(SEBToMetadataClass, RelatedInstanceDirection.Forward, metadataClass), new WhereCriteria(new ECInstanceIdExpression())));

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;
            List<IECProperty> propList;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap, out propList);

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            string idColumnString = multibandSource["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string redBandUrlColumnString = multibandSource["RedBandURL"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string fromTableString = multibandSource.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            IECClass SpatialDataSource = m_schema.GetClass("SpatialDataSource");
            string leftJoinTableString = SpatialDataSource.GetCustomAttributes("SQLEntity").GetString("FromTableName");
            string baseIdColumnString = SpatialDataSource["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");

            string idOrRedBandUrl = "(" + idColumnString + @".*" + redBandUrlColumnString + "|" + redBandUrlColumnString + @".*" + idColumnString + ")";
            string idOrBaseId = "(" + idColumnString + @".*=.*" + baseIdColumnString + "|" + baseIdColumnString + @".*=.*" + idColumnString + ")";

            Regex reg = new Regex(@".*SELECT.*" + idOrRedBandUrl + @".* FROM .*" + Regex.Escape(fromTableString) + @".*LEFT JOIN.*" + leftJoinTableString + ".*ON.*" + idOrBaseId + ".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            //SELECT tab0.IdStr, tab1.Name, tab0.Processable FROM dbo.SpatialEntityDatasets tab0 LEFT JOIN dbo.SpatialEntityBases tab1 ON tab0.IdStr = tab1.IdStr  ;     
            Assert.IsTrue(query.SelectClause.SelectedProperties.All(p => propList.Exists(p2 => p == p2)));
            }
        }
    }
