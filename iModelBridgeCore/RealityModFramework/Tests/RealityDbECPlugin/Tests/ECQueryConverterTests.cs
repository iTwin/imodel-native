using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.ECObjects.XML;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
//using Microsoft.Data.Schema.ScriptDom;
//using Microsoft.Data.Schema.ScriptDom.Sql;
using NUnit.Framework;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text.RegularExpressions;
using System.Data;


namespace IndexECPlugin.Tests
    {
    [TestFixture]
    class ECQueryConverterTests
        {
        IECSchema m_schema;


        [SetUp]
        public void SetUp ()
            {
            ECSchemaXmlStreamReader schemaReader = new ECSchemaXmlStreamReader(Assembly.GetAssembly(typeof(IndexECPlugin.Source.IndexECPlugin)).GetManifestResourceStream("ECSchemaDB.xml"));
            m_schema = schemaReader.Deserialize();
            }

        [Test]
        public void SimpleQueryTest ()
            {
            IECClass spatialEntityBaseClass = m_schema.GetClass("SpatialEntityBase");
            ECQuery query = new ECQuery(spatialEntityBaseClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(spatialEntityBaseClass["Id"]);

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap);

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            //SELECT tab0.IdStr FROM dbo.SpatialEntityBases tab0  ;

            string idColumnString = spatialEntityBaseClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string tableString = spatialEntityBaseClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            //Assert.IsTrue(sqlCommand.Contains(idColumnString));
            //Assert.IsTrue(sqlCommand.Contains(tableString));

            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(tableString) + @".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");


            }

        [Test]
        public void PropertyExpressionCriteriaQueryTest ()
            {
            IECClass spatialEntityBaseClass = m_schema.GetClass("SpatialEntityBase");
            ECQuery query = new ECQuery(spatialEntityBaseClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(spatialEntityBaseClass["Id"]);

            query.WhereClause = new WhereCriteria(new PropertyExpression(RelationalOperator.EQ, spatialEntityBaseClass["Name"], "Test"));

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap);
            GenericParamNameValueMap genericParamNameValueMap = paramNameValueMap as GenericParamNameValueMap;

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            //SELECT tab0.IdStr FROM dbo.SpatialEntityBases tab0  WHERE  ( tab0.Name =@param0  )  ;

            string idColumnString = spatialEntityBaseClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string nameColumnString = spatialEntityBaseClass["Name"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string tableString = spatialEntityBaseClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            Assert.IsNotNull(genericParamNameValueMap, "The ParamNameValueMap was not a GenericParamNameValueMap.");
            Assert.AreEqual(1, genericParamNameValueMap.Count(), "Invalid number of parameters in the map.");
            Assert.AreEqual("Test", genericParamNameValueMap.First().Value.Item1, "The map does not contain the value of the property requested in the criteria.");

            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(tableString) + @".* WHERE .*" + Regex.Escape(nameColumnString) + @".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            }

        [Test]
        public void ECInstanceIdExpressionCriteriaQueryTest ()
            {
            IECClass spatialEntityBaseClass = m_schema.GetClass("SpatialEntityBase");
            ECQuery query = new ECQuery(spatialEntityBaseClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(spatialEntityBaseClass["Id"]);

            query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("1"));

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;


            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap);
            GenericParamNameValueMap genericParamNameValueMap = paramNameValueMap as GenericParamNameValueMap;
            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            //SELECT tab0.IdStr FROM dbo.SpatialEntityBases tab0  WHERE  (  ( tab0.IdStr =@param0  )  )  ORDER BY tab0.IdStr ASC ;

            string idColumnString = spatialEntityBaseClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string tableString = spatialEntityBaseClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            Assert.IsNotNull(genericParamNameValueMap, "The ParamNameValueMap was not a GenericParamNameValueMap.");
            Assert.AreEqual(1, genericParamNameValueMap.Count(), "Invalid number of parameters in the map.");
            Assert.AreEqual("1", genericParamNameValueMap.First().Value.Item1, "The map does not contain the value of the ID requested.");

            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(tableString) + @".* WHERE .*" + Regex.Escape(idColumnString) + @".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            }

        [Test]
        public void ComplexCriteriaQueryTest ()
            {
            IECClass spatialEntityBaseClass = m_schema.GetClass("SpatialEntityBase");
            ECQuery query = new ECQuery(spatialEntityBaseClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(spatialEntityBaseClass["Id"]);

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

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap);
            GenericParamNameValueMap genericParamNameValueMap = paramNameValueMap as GenericParamNameValueMap;
            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            //SELECT tab0.IdStr FROM dbo.SpatialEntityBases tab0  WHERE  (  (  ( tab0.IdStr =@param0  ) OR  ( tab0.IdStr =@param1  )  ) AND  ( tab0.IdStr =@param2  )  )  ORDER BY tab0.IdStr ASC ;

            string idColumnString = spatialEntityBaseClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string tableString = spatialEntityBaseClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            Assert.IsNotNull(genericParamNameValueMap, "The ParamNameValueMap was not a GenericParamNameValueMap.");
            Assert.IsTrue(sqlCommand.Contains(idColumnString), "The query does not contain the name of the Id Column.");
            Assert.IsTrue(sqlCommand.Contains(tableString), "The query does not contain the name of the table queried.");
            Assert.AreEqual(3, genericParamNameValueMap.Count(), "Invalid number of parameters in the map.");

            Regex reg = new Regex(@".*SELECT.*" + Regex.Escape(idColumnString) + @".*FROM.*" + Regex.Escape(tableString) + @".*WHERE.*\(.*" + Regex.Escape(idColumnString) + @".*OR.*" + Regex.Escape(idColumnString) + @".*\).*AND.*" + Regex.Escape(idColumnString) + ".*");

            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");

            }

        [Test]
        public void RelatedQueryTest ()
            {
            IECClass spatialEntityBaseClass = m_schema.GetClass("SpatialEntityBase");
            IECClass metadataClass = m_schema.GetClass("Metadata");
            IECRelationshipClass SEBToMetadataClass = m_schema.GetClass("SpatialEntityBaseToMetadata") as IECRelationshipClass;
            ECQuery query = new ECQuery(spatialEntityBaseClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(spatialEntityBaseClass["Id"]);

            WhereCriteria relatedCriteria = new WhereCriteria(new RelatedCriterion(new QueryRelatedClassSpecifier(SEBToMetadataClass, RelatedInstanceDirection.Forward, metadataClass), new WhereCriteria(new ECInstanceIdExpression("1"))));
            query.WhereClause = relatedCriteria;

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;
            string idColumnString = spatialEntityBaseClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string metadataIdColumnString = metadataClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string fromTableString = spatialEntityBaseClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");
            string joinedTableString = metadataClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            string firstKey = SEBToMetadataClass.GetCustomAttributes("RelationshipKeys").GetString("ContainerKey");
            string secondKey = SEBToMetadataClass.GetCustomAttributes("RelationshipKeys").GetString("ContainedKey");
            string firstOrSecond = "(" + firstKey + @".*=.*" + secondKey + "|" + secondKey + @".*=.*" + firstKey + ")";

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap);

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            // SELECT tab0.IdStr FROM dbo.SpatialEntityBases tab0 LEFT JOIN dbo.Metadatas tab1 ON tab0.Metadata_Id = tab1.Id  WHERE  (  (  ( tab1.IdStr =@param0  )  )  )  ORDER BY tab1.IdStr ASC ;


            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(fromTableString) + ".*LEFT JOIN.*" + Regex.Escape(joinedTableString) + ".*ON.*" + firstOrSecond + @".* WHERE .*" + Regex.Escape(metadataIdColumnString) + @".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");

            }

        [Test]
        public void RelatedQueryTestWithRelatedID ()
            {
            IECClass spatialEntityBaseClass = m_schema.GetClass("SpatialEntityBase");
            IECClass metadataClass = m_schema.GetClass("Metadata");
            IECRelationshipClass SEBToMetadataClass = m_schema.GetClass("SpatialEntityBaseToMetadata") as IECRelationshipClass;
            ECQuery query = new ECQuery(spatialEntityBaseClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            var criteria = new RelatedCriterion(new QueryRelatedClassSpecifier(SEBToMetadataClass, RelatedInstanceDirection.Forward, metadataClass), new WhereCriteria(new ECInstanceIdExpression("1")));
            criteria.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("RequestRelatedId", true));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(spatialEntityBaseClass["Id"]);

            WhereCriteria relatedCriteria = new WhereCriteria(criteria);
            query.WhereClause = relatedCriteria;

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;
            string idColumnString = spatialEntityBaseClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string metadataIdColumnString = metadataClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string fromTableString = spatialEntityBaseClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");
            string joinedTableString = metadataClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            string firstKey = SEBToMetadataClass.GetCustomAttributes("RelationshipKeys").GetString("ContainerKey");
            string secondKey = SEBToMetadataClass.GetCustomAttributes("RelationshipKeys").GetString("ContainedKey");
            string firstOrSecond = "(" + firstKey + @".*=.*" + secondKey + "|" + secondKey + @".*=.*" + firstKey + ")";

            string firstOrSecondIdColumn = "(" + Regex.Escape(idColumnString) + @".*,.*" + Regex.Escape(metadataIdColumnString) + "|" + Regex.Escape(metadataIdColumnString) + @".*,.*" + Regex.Escape(idColumnString) + ")";

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap);

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            //SELECT tab0.IdStr, tab1.IdStr FROM dbo.SpatialEntityBases tab0 LEFT JOIN dbo.Metadatas tab1 ON tab0.Metadata_Id = tab1.Id  WHERE  (  (  ( tab1.IdStr =@param0  )  )  )  ORDER BY tab1.IdStr ASC ;


            Regex reg = new Regex(@".*SELECT .*" + firstOrSecondIdColumn + @".* FROM .*" + Regex.Escape(fromTableString) + ".*LEFT JOIN.*" + Regex.Escape(joinedTableString) + ".*ON.*" + firstOrSecond + @".* WHERE .*" + Regex.Escape(metadataIdColumnString) + @".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");

            }

        [Test]
        public void SpatialQueryTest ()
            {
            IECClass spatialEntityBaseClass = m_schema.GetClass("SpatialEntityBase");
            ECQuery query = new ECQuery(spatialEntityBaseClass);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(spatialEntityBaseClass["Id"]);

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

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap);

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            string idColumnString = spatialEntityBaseClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string fromTableString = spatialEntityBaseClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            //SELECT tab0.IdStr FROM dbo.SpatialEntityBases tab0  WHERE tab0.Footprint.STIntersects(geometry::STGeomFromText('POLYGON ((30 10, 40 40, 20 40, 10 20, 30 10))',4326)) = 'true' ;

            Regex reg = new Regex(@".*SELECT.*" + Regex.Escape(idColumnString) + @".*FROM.*" + Regex.Escape(fromTableString) + @".*WHERE.*STIntersects.*\('POLYGON \(\(30 10, 40 40, 20 40, 10 20, 30 10\)\)',4326\).*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            }

        [Test]
        public void PolymorphicClassQueryTest ()
            {
            IECClass spatialEntityDataset = m_schema.GetClass("SpatialEntityDataset");
            ECQuery query = new ECQuery(spatialEntityDataset);
            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

            query.SelectClause.SelectAllProperties = false;
            query.SelectClause.SelectedProperties.Add(spatialEntityDataset["Id"]);
            query.SelectClause.SelectedProperties.Add(spatialEntityDataset["Name"]);
            query.SelectClause.SelectedProperties.Add(spatialEntityDataset["Processable"]);

            //WhereCriteria internalCriteria = new WhereCriteria(new RelatedCriterion(new QueryRelatedClassSpecifier(SEBToMetadataClass, RelatedInstanceDirection.Forward, metadataClass), new WhereCriteria(new ECInstanceIdExpression())));

            ECQuerySettings querySettings = new ECQuerySettings();

            SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

            ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

            string sqlCommand;
            string sqlCount;

            DataReadingHelper helper;
            IParamNameValueMap paramNameValueMap;

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount, out helper, out paramNameValueMap);

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            string idColumnString = spatialEntityDataset["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string nameColumnString = spatialEntityDataset["Name"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string fromTableString = spatialEntityDataset.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            IECClass spatialEntityBase = m_schema.GetClass("SpatialEntityBase");
            string leftJoinTableString = spatialEntityBase.GetCustomAttributes("SQLEntity").GetString("FromTableName");
            string baseIdColumnString = spatialEntityBase["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");

            string idOrName = "(" + idColumnString + @".*" + nameColumnString + "|" + nameColumnString + @".*" + idColumnString + ")";
            string idOrBaseId = "(" + idColumnString + @".*=.*" + baseIdColumnString + "|" + baseIdColumnString + @".*=.*" + idColumnString + ")";

            Regex reg = new Regex(@".*SELECT.*" + idOrName + @".* FROM .*" + Regex.Escape(fromTableString) + @".*LEFT JOIN.*" + leftJoinTableString + ".*ON.*" + idOrBaseId + ".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand), "The query does not have the required form.");
            //SELECT tab0.IdStr, tab1.Name, tab0.Processable FROM dbo.SpatialEntityDatasets tab0 LEFT JOIN dbo.SpatialEntityBases tab1 ON tab0.IdStr = tab1.IdStr  ;        
            }
        }
    }
