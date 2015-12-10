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
using System.Data;
using System.Data.Common;
using System.Data.SqlClient;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;


namespace IndexECPlugin.Tests
{
    [TestFixture]
    class ECQueryConverterTests
    {
        IECSchema m_schema;


        [SetUp]
        public void SetUp()
        {
            ECSchemaXmlStreamReader schemaReader = new ECSchemaXmlStreamReader(Assembly.GetAssembly(typeof(IndexECPlugin.Source.IndexECPlugin)).GetManifestResourceStream("ECSchemaDB.xml"));
            m_schema = schemaReader.Deserialize();
        }

        [Test]
        public void SimpleQueryTest()
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

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount);

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
            Assert.IsTrue(reg.IsMatch(sqlCommand));


        }

        [Test]
        public void PropertyExpressionCriteriaQueryTest()
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

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount);

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            //SELECT tab0.IdStr FROM dbo.SpatialEntityBases tab0  WHERE  ( tab0.Name =@param0  )  ;

            string idColumnString = spatialEntityBaseClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string nameColumnString = spatialEntityBaseClass["Name"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string tableString = spatialEntityBaseClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            var paramNameValueMap = sqlQueryBuilder.paramNameValueMap;

            Assert.AreEqual(1, paramNameValueMap.Count);
            Assert.AreEqual("Test", paramNameValueMap.First().Value.Item1);

            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(tableString) + @".* WHERE .*" + Regex.Escape(nameColumnString) + @".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand));
        }

        [Test]
        public void ECInstanceIdExpressionCriteriaQueryTest()
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

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount);

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            //SELECT tab0.IdStr FROM dbo.SpatialEntityBases tab0  WHERE  (  ( tab0.IdStr =@param0  )  )  ORDER BY tab0.IdStr ASC ;

            string idColumnString = spatialEntityBaseClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string tableString = spatialEntityBaseClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            var paramNameValueMap = sqlQueryBuilder.paramNameValueMap;

            Assert.AreEqual(1, paramNameValueMap.Count);
            Assert.AreEqual("1", paramNameValueMap.First().Value.Item1);

            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(tableString) + @".* WHERE .*" + Regex.Escape(idColumnString) + @".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand));
        }

        [Test]
        public void ComplexCriteriaQueryTest()
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

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount);

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            //SELECT tab0.IdStr FROM dbo.SpatialEntityBases tab0  WHERE  (  (  ( tab0.IdStr =@param0  ) OR  ( tab0.IdStr =@param1  )  ) AND  ( tab0.IdStr =@param2  )  )  ORDER BY tab0.IdStr ASC ;

            string idColumnString = spatialEntityBaseClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string tableString = spatialEntityBaseClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            var paramNameValueMap = sqlQueryBuilder.paramNameValueMap;

            Assert.IsTrue(sqlCommand.Contains(idColumnString));
            Assert.IsTrue(sqlCommand.Contains(tableString));
            Assert.AreEqual(3, paramNameValueMap.Count);

            Regex reg = new Regex(@".*SELECT.*" + Regex.Escape(idColumnString) + @".*FROM.*" + Regex.Escape(tableString) + @".*WHERE.*\(.*" + Regex.Escape(idColumnString) + @".*OR.*" + Regex.Escape(idColumnString)+@".*\).*AND.*" + Regex.Escape(idColumnString) + ".*");

            Assert.IsTrue(reg.IsMatch(sqlCommand));

        }

        [Test]
        public void RelatedQueryTest()
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

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount);

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            // SELECT tab0.IdStr FROM dbo.SpatialEntityBases tab0 LEFT JOIN dbo.Metadatas tab1 ON tab0.Metadata_Id = tab1.Id  WHERE  (  (  ( tab1.IdStr =@param0  )  )  )  ORDER BY tab1.IdStr ASC ;
            

            Regex reg = new Regex(@".*SELECT .*" + Regex.Escape(idColumnString) + @".* FROM .*" + Regex.Escape(fromTableString) + ".*LEFT JOIN.*" + Regex.Escape(joinedTableString) + ".*ON.*" + firstOrSecond + @".* WHERE .*" + Regex.Escape(metadataIdColumnString) + @".*");
            Assert.IsTrue(reg.IsMatch(sqlCommand));

        }

        [Test]
        public void SpatialQueryTest()
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

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount);

            //TSql100Parser parser = new TSql100Parser(false);
            //IList<ParseError> errors;
            //parser.Parse(new StringReader(sqlCommand), out errors);

            ////This verifies that the sql query is valid
            //Assert.IsTrue(errors.Count == 0);

            string idColumnString = spatialEntityBaseClass["Id"].GetCustomAttributes("DBColumn").GetString("ColumnName");
            string fromTableString = spatialEntityBaseClass.GetCustomAttributes("SQLEntity").GetString("FromTableName");

            //SELECT tab0.IdStr FROM dbo.SpatialEntityBases tab0  WHERE tab0.Footprint.STIntersects(geometry::STGeomFromText('POLYGON ((30 10, 40 40, 20 40, 10 20, 30 10))',4326)) = 'true' ;

            Regex reg = new Regex(@".*SELECT.*" + Regex.Escape(idColumnString) + @".*FROM.*" + Regex.Escape(fromTableString) + @".*WHERE.*STIntersects.*\('POLYGON \(\(30 10, 40 40, 20 40, 10 20, 30 10\)\)',4326\).*");
            Assert.IsTrue(reg.IsMatch(sqlCommand));
        }

        [Test]
        public void PolymorphicClassQueryTest()
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

            ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount);

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
            Assert.IsTrue(reg.IsMatch(sqlCommand));
            //SELECT tab0.IdStr, tab1.Name, tab0.Processable FROM dbo.SpatialEntityDatasets tab0 LEFT JOIN dbo.SpatialEntityBases tab1 ON tab0.IdStr = tab1.IdStr  ;        
        }

        //[Test]
        //public void RelatedInstanceSelectTest()
        //{
        //    IECClass spatialEntityBaseClass = m_schema.GetClass("SpatialEntityBase");
        //    IECClass metadataClass = m_schema.GetClass("Metadata");
        //    IECRelationshipClass SEBToMetadataClass = m_schema.GetClass("SpatialEntityBaseToMetadata") as IECRelationshipClass;
        //    ECQuery query = new ECQuery(spatialEntityBaseClass);
        //    query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

        //    query.SelectClause.SelectAllProperties = false;
        //    query.SelectClause.SelectedProperties.Add(spatialEntityBaseClass["Id"]);

        //    var relatedCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(SEBToMetadataClass, RelatedInstanceDirection.Forward, metadataClass), true);
        //    relatedCrit.SelectAllProperties = false;
        //    relatedCrit.SelectedProperties.Add(metadataClass["Id"]);
        //    query.SelectClause.SelectedRelatedInstances.Add(relatedCrit);

        //    ECQuerySettings querySettings = new ECQuerySettings();

        //    SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

        //    ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

        //    string sqlCommand;
        //    string sqlCount;

        //    ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount);

        //    TSql100Parser parser = new TSql100Parser(false);
        //    IList<ParseError> errors;
        //    parser.Parse(new StringReader(sqlCommand), out errors);

        //    //This verifies that the sql query is valid
        //    Assert.IsTrue(errors.Count == 0);

        //    //SELECT tab0.IdStr FROM dbo.SpatialEntityBases tab0  ;

        //}

        //[Test]
        //public void PolymorphicRelatedInstanceSelectTest()
        //{
        //    IECClass WMSServerClass = m_schema.GetClass("WMSServer");
        //    IECClass WMSSourceClass = m_schema.GetClass("WMSSource");
        //    IECRelationshipClass SEBToMetadataClass = m_schema.GetClass("ServerToSpatialDataSource") as IECRelationshipClass;
        //    ECQuery query = new ECQuery(WMSServerClass);
        //    query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("source", "index"));

        //    query.SelectClause.SelectAllProperties = false;
        //    query.SelectClause.SelectedProperties.Add(WMSServerClass["Id"]);

        //    var relatedCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(SEBToMetadataClass, RelatedInstanceDirection.Forward, WMSSourceClass), true);
        //    relatedCrit.SelectAllProperties = false;
        //    relatedCrit.SelectedProperties.Add(WMSSourceClass["Id"]);
        //    relatedCrit.SelectedProperties.Add(WMSSourceClass["Title"]);
        //    query.SelectClause.SelectedRelatedInstances.Add(relatedCrit);

        //    ECQuerySettings querySettings = new ECQuerySettings();

        //    SQLQueryBuilder sqlQueryBuilder = new StandardSQLQueryBuilder();

        //    ECQueryConverter ecQueryConverter = new ECQueryConverter(query, querySettings, sqlQueryBuilder, null, m_schema, false);

        //    string sqlCommand;
        //    string sqlCount;

        //    ecQueryConverter.CreateSqlCommandStringFromQuery(out sqlCommand, out sqlCount);

        //    TSql100Parser parser = new TSql100Parser(false);
        //    IList<ParseError> errors;
        //    parser.Parse(new StringReader(sqlCommand), out errors);

        //    //This verifies that the sql query is valid
        //    Assert.IsTrue(errors.Count == 0);

        //    //SELECT tab0.IdStr FROM dbo.WMSServers tab0  ;
        //}

    }
}
