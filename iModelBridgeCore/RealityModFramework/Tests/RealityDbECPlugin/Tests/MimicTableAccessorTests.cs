using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.ECObjects.XML;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;

namespace IndexECPlugin.Tests
    {
    [TestFixture]
    class MimicTableAccessorTests
        {
        IECSchema m_schema;
        MimicTableAccessor m_mimicTableAccessor;

        [SetUp]
        public void SetUp ()
            {
            m_schema = SetupHelpers.PrepareSchema();

            m_mimicTableAccessor = new MimicTableWriter(true, "CacheTableName", "CacheColumnName", "CacheJoinTableName", null);
            }

        [Test]
        public void SEBQuery()
            {

            IECClass sebClass = m_schema.GetClass("SpatialEntityBase");
            List<string> IdsList = new List<string>(){"1","2","3"};

            DataReadingHelper dataReadingHelper;
            IParamNameValueMap paramNameValueMap;

            string tableString = sebClass.GetCustomAttributes("SQLEntity").GetString("CacheTableName");
            string idColumnString = sebClass["Id"].GetCustomAttributes("MimicDBColumn").GetString("CacheColumnName");

            //SELECT tab0.IdStr, tab0.Footprint.STAsText(), tab0.Footprint.STSrid, tab0.Name, tab0.Keywords, tab0.AssociateFile, tab0.ProcessingDescription, tab0.DataSourceTypesAvailable, tab0.AccuracyResolutionDensity, tab0.Date, tab0.Classification FROM dbo.CacheSpatialEntityBases tab0  WHERE tab0.IdStr IN (@param0,@param1,@param2) AND tab0.SubAPI =@param3  ;
            string sqlQuery = m_mimicTableAccessor.CreateMimicSQLQuery(DataSource.USGS, IdsList, sebClass, sebClass, out dataReadingHelper, out paramNameValueMap);

            GenericParamNameValueMap genericMap = paramNameValueMap as GenericParamNameValueMap;

            Assert.IsNotNull(genericMap, "The ParamNameValueMap was not a GenericParamNameValueMap.");

            Regex reg = new Regex(@".*SELECT .*" + @".* FROM .*" + Regex.Escape(tableString) + @".*WHERE.*" + Regex.Escape(idColumnString) + @".*SubAPI.*");
            Assert.IsTrue(reg.IsMatch(sqlQuery), "The query does not have the required form.");
            foreach(IECProperty prop in sebClass)
                {
                if ( prop.GetCustomAttributes("MimicDBColumn") != null && prop.GetCustomAttributes("MimicDBColumn")["CacheColumnName"] != null )
                    {
                    string propColumnName = prop.GetCustomAttributes("MimicDBColumn").GetString("CacheColumnName");
                    Assert.IsTrue(sqlQuery.Contains(propColumnName), "The query does not contain the " + propColumnName + " property name.");
                    }
                }


            Assert.IsTrue(genericMap.Any(p => (string) p.Value.Item1 == "1"), "The map does not contain all of the queried IDs.");
            Assert.IsTrue(genericMap.Any(p => (string) p.Value.Item1 == "2"), "The map does not contain all of the queried IDs.");
            Assert.IsTrue(genericMap.Any(p => (string) p.Value.Item1 == "3"), "The map does not contain all of the queried IDs.");
            Assert.IsTrue(genericMap.Any(p => (string) p.Value.Item1 == "usgsapi"), "The source was not included in the map.");
            }

        [Test]
        public void SEBSpatialQuery ()
            {

            IECClass sebClass = m_schema.GetClass("SpatialEntityBase");

            DataReadingHelper dataReadingHelper;
            IParamNameValueMap paramNameValueMap;

            string tableString = sebClass.GetCustomAttributes("SQLEntity").GetString("CacheTableName");
            string idColumnString = sebClass["Id"].GetCustomAttributes("MimicDBColumn").GetString("CacheColumnName");

            PolygonDescriptor polygonDescriptor = new PolygonDescriptor();
            polygonDescriptor.WKT = "POLYGON ((30 10, 40 40, 20 40, 10 20, 30 10))";
            polygonDescriptor.SRID = 4326;

            string sqlQuery = m_mimicTableAccessor.CreateMimicSQLSpatialQuery(DataSource.USGS, polygonDescriptor, sebClass, sebClass, out dataReadingHelper, out paramNameValueMap);

            GenericParamNameValueMap genericMap = paramNameValueMap as GenericParamNameValueMap;

            Assert.IsNotNull(genericMap, "The ParamNameValueMap was not a GenericParamNameValueMap.");

            Regex reg = new Regex(@".*SELECT .*" + @".* FROM .*" + Regex.Escape(tableString) +  @".*WHERE.*STIntersects.*\('POLYGON \(\(30 10, 40 40, 20 40, 10 20, 30 10\)\)',4326\).*" + @".*SubAPI.*");
            Assert.IsTrue(reg.IsMatch(sqlQuery), "The query does not have the required form.");
            foreach ( IECProperty prop in sebClass )
                {
                if ( prop.GetCustomAttributes("MimicDBColumn") != null && prop.GetCustomAttributes("MimicDBColumn")["CacheColumnName"] != null )
                    {
                    string propColumnName = prop.GetCustomAttributes("MimicDBColumn").GetString("CacheColumnName");
                    Assert.IsTrue(sqlQuery.Contains(propColumnName), "The query does not contain the " + propColumnName + " property name.");
                    }
                }


            Assert.IsTrue(genericMap.Any(p => (string) p.Value.Item1 == "usgsapi"), "The source was not included in the map.");
            }

        [Test]
        public void SEBNonInstanceDataQuery ()
            {

            IECClass sebClass = m_schema.GetClass("SpatialEntityBase");
            List<string> IdsList = new List<string>() { "1" };

            DataReadingHelper dataReadingHelper;
            IParamNameValueMap paramNameValueMap;

            string tableString = sebClass.GetCustomAttributes("SQLEntity").GetString("CacheTableName");
            string idColumnString = sebClass["Id"].GetCustomAttributes("MimicDBColumn").GetString("CacheColumnName");

            List<IECProperty> emptyList = new List<IECProperty>();
            List<string> nonInstanceColumnList = new List<string>(){"testCol1234"};

            //SELECT tab0.IdStr, tab0.Footprint.STAsText(), tab0.Footprint.STSrid, tab0.Name, tab0.Keywords, tab0.AssociateFile, tab0.ProcessingDescription, tab0.DataSourceTypesAvailable, tab0.AccuracyResolutionDensity, tab0.Date, tab0.Classification FROM dbo.CacheSpatialEntityBases tab0  WHERE tab0.IdStr IN (@param0,@param1,@param2) AND tab0.SubAPI =@param3  ;
            string sqlQuery = m_mimicTableAccessor.CreateMimicSQLQuery(DataSource.USGS, IdsList, sebClass, emptyList, out dataReadingHelper, out paramNameValueMap, nonInstanceColumnList);

            Assert.IsTrue(sqlQuery.Contains("testCol1234"), "The additionnal column was not added in the query.");
            }

        [Test]
        public void SEBSpatialNonInstanceDataQuery ()
            {

            IECClass sebClass = m_schema.GetClass("SpatialEntityBase");

            DataReadingHelper dataReadingHelper;
            IParamNameValueMap paramNameValueMap;

            string tableString = sebClass.GetCustomAttributes("SQLEntity").GetString("CacheTableName");
            string idColumnString = sebClass["Id"].GetCustomAttributes("MimicDBColumn").GetString("CacheColumnName");

            List<IECProperty> emptyList = new List<IECProperty>();
            List<string> nonInstanceColumnList = new List<string>() { "testCol1234" };

            PolygonDescriptor polygonDescriptor = new PolygonDescriptor();
            polygonDescriptor.WKT = "POLYGON ((30 10, 40 40, 20 40, 10 20, 30 10))";
            polygonDescriptor.SRID = 4326;

            string sqlQuery = m_mimicTableAccessor.CreateMimicSQLSpatialQuery(DataSource.USGS, polygonDescriptor, sebClass, emptyList, out dataReadingHelper, out paramNameValueMap, nonInstanceColumnList);

            Assert.IsTrue(sqlQuery.Contains("testCol1234"), "The additionnal column was not added in the query.");
            }

        [Test]
        public void SEBSpatialWhereCriteriaQuery ()
            {

            IECClass sebClass = m_schema.GetClass("SpatialEntityBase");

            DataReadingHelper dataReadingHelper;
            IParamNameValueMap paramNameValueMap;

            string tableString = sebClass.GetCustomAttributes("SQLEntity").GetString("CacheTableName");
            string idColumnString = sebClass["Id"].GetCustomAttributes("MimicDBColumn").GetString("CacheColumnName");

            List<IECProperty> emptyList = new List<IECProperty>();
            SingleWhereCriteriaHolder criteria = new SingleWhereCriteriaHolder();
            criteria.Property = sebClass["Id"];
            criteria.Operator = Bentley.EC.Persistence.Query.RelationalOperator.EQ;
            criteria.Value = "testValue12345";
            List<SingleWhereCriteriaHolder> criteriaList = new List<SingleWhereCriteriaHolder>(){criteria};

            PolygonDescriptor polygonDescriptor = new PolygonDescriptor();
            polygonDescriptor.WKT = "POLYGON ((30 10, 40 40, 20 40, 10 20, 30 10))";
            polygonDescriptor.SRID = 4326;

            string sqlQuery = m_mimicTableAccessor.CreateMimicSQLSpatialQuery(DataSource.USGS, polygonDescriptor, sebClass, emptyList, out dataReadingHelper, out paramNameValueMap, null, criteriaList);

            GenericParamNameValueMap genericMap = paramNameValueMap as GenericParamNameValueMap;

            Assert.IsNotNull(genericMap, "The ParamNameValueMap was not a GenericParamNameValueMap.");

            Assert.IsTrue(sqlQuery.Contains(idColumnString), "The query does not contain the criteria on the ID");
            Assert.IsTrue(genericMap.Any(p => (string) p.Value.Item1 == "testValue12345"), "The map does not contain the value set in the criteria");
            }
        }
    }
