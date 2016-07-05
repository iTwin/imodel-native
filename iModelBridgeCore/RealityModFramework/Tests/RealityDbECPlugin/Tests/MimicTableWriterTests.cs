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
    class MimicTableWriterTests
        {
        IECSchema m_schema;
        MimicTableWriter m_mimicTableWriter;

        [SetUp]
        public void SetUp ()
            {
            m_schema = SetupHelpers.PrepareSchema();

            m_mimicTableWriter = new MimicTableWriter(true, "CacheTableName", "CacheColumnName", "CacheJoinTableName", null);
            }

        private IECInstance GetInstanceFilledWithTestData (IECClass ecClass)
            {
            IECInstance instance = ecClass.CreateInstance();
            foreach ( IECProperty prop in ecClass )
                {
                if ( ECTypeHelper.IsString(prop.Type) )
                    {
                    if ( (prop.GetCustomAttributes("DBColumn") != null) &&
                         (prop.GetCustomAttributes("DBColumn")["IsSpatial"] != null) &&
                         (!prop.GetCustomAttributes("DBColumn")["IsSpatial"].IsNull) &&
                         (prop.GetCustomAttributes("DBColumn")["IsSpatial"].StringValue.ToLower() == "true") )
                        {
                        instance[prop.Name].StringValue = "{ \"points\" : [[-90,34],[-89,34],[-89,35],[-90,35],[-90,34]], \"coordinate_system\" : \"4326\" }";
                        }
                    else
                        {
                        instance[prop.Name].StringValue = prop.Name + "Test";
                        }
                    }
                else if ( ECTypeHelper.IsDouble(prop.Type) )
                    {
                    instance[prop.Name].DoubleValue = 123.45;
                    }
                else if ( ECTypeHelper.IsBoolean(prop.Type) )
                    {
                    instance[prop.Name].NativeValue = false;
                    }
                else if ( ECTypeHelper.IsInteger(prop.Type) )
                    {
                    instance[prop.Name].IntValue = 12345;
                    }
                else if ( ECTypeHelper.IsLong(prop.Type) )
                    {
                    instance[prop.Name].NativeValue = (long)12345;
                    }
                else if ( ECTypeHelper.IsDateTime(prop.Type) )
                    {
                    instance[prop.Name].NativeValue = new DateTime(2016, 5, 27);
                    }
                }
            return instance;
            }

        [Test]
        public void CreateMimicSEBTest()
            {

            IECClass sebClass = m_schema.GetClass("SpatialEntityBase");

            IECInstance spatialEntityBase = GetInstanceFilledWithTestData(sebClass);

            List<IECInstance> instanceList = new List<IECInstance>() { spatialEntityBase };
            IParamNameValueMap paramNameValueMap;

            string tableString = sebClass.GetCustomAttributes("SQLEntity").GetString("CacheTableName");

            // BEGIN TRY INSERT INTO dbo.CacheSpatialEntityBases (IdStr,Footprint,Name,Keywords,AssociateFile,ProcessingDescription,DataSourceTypesAvailable,AccuracyResolutionDensity,Date,Classification) VALUES (@p1,geometry::STGeomFromText('POLYGON((-90 34, -89 34, -89 35, -90 35, -90 34))',4326),@p3,@p4,@p5,@p6,@p7,@p8,@p9,@p10); END TRY BEGIN CATCH IF ERROR_NUMBER() <> 2627 THROW END CATCH;
            string sqlQuery = m_mimicTableWriter.CreateMimicSQLInsert(instanceList, sebClass, new SQLServerInsertStatementBuilder(), out paramNameValueMap);
            SqlServerParamNameValueMap sqlServerMap = paramNameValueMap as SqlServerParamNameValueMap;

            Assert.IsNotNull(sqlServerMap, "The ParamNameValueMap was not a SqlServerParamNameValueMap.");

            //Unfinished regex... to complete?
            Regex reg = new Regex(@".*BEGIN.*TRY.*INSERT.*INTO.*" + Regex.Escape(tableString) + @".*END.*TRY.*BEGIN.*CATCH.*IF.*ERROR_NUMBER().*<>.*2627.*THROW.*END.*CATCH.*;.*");
            Assert.IsTrue(reg.IsMatch(sqlQuery), "The statement does not have the required form.");
            int count = 0;
            foreach(IECProperty prop in sebClass)
                {
                if(prop.GetCustomAttributes("MimicDBColumn") != null && prop.GetCustomAttributes("MimicDBColumn")["CacheColumnName"] != null)
                    {
                    if ( (prop.GetCustomAttributes("DBColumn") == null) ||
                         (prop.GetCustomAttributes("DBColumn")["IsSpatial"] == null) ||
                         (prop.GetCustomAttributes("DBColumn")["IsSpatial"].IsNull) ||
                         (prop.GetCustomAttributes("DBColumn")["IsSpatial"].StringValue.ToLower() == "false") )
                        {
                        //Spatial parameters are not in the map.
                        count++;
                        }
                    string propColumnName = prop.GetCustomAttributes("MimicDBColumn").GetString("CacheColumnName");
                    Assert.IsTrue(sqlQuery.Contains(propColumnName), "The statement does not contain the " + propColumnName + " property name.");
                    }
                }
            Assert.IsTrue(sqlServerMap.Count == count, "The number of values mapped doesn't reflect the number of parameters in the statement");
            }

        [Test]
        public void CreateMimicSEBTestWithAddedColumn ()
            {

            IECClass sebClass = m_schema.GetClass("SpatialEntityBase");

            IECInstance spatialEntityBase = GetInstanceFilledWithTestData(sebClass);

            List<IECInstance> instanceList = new List<IECInstance>() { spatialEntityBase };
            IParamNameValueMap paramNameValueMap;

            string tableString = sebClass.GetCustomAttributes("SQLEntity").GetString("CacheTableName");

            List<Tuple<string, IECType, Func<IECInstance, string>>> additionalColumns = new List<Tuple<string, IECType, Func<IECInstance, string>>>();
            additionalColumns.Add(new Tuple<string, IECType, Func<IECInstance, string>>("TestColumn1234", Bentley.ECObjects.ECObjects.StringType, inst => "TestColVal1234"));

            // BEGIN TRY INSERT INTO dbo.CacheSpatialEntityBases (IdStr,Footprint,Name,Keywords,AssociateFile,ProcessingDescription,DataSourceTypesAvailable,AccuracyResolutionDensity,Date,Classification) VALUES (@p1,geometry::STGeomFromText('POLYGON((-90 34, -89 34, -89 35, -90 35, -90 34))',4326),@p3,@p4,@p5,@p6,@p7,@p8,@p9,@p10); END TRY BEGIN CATCH IF ERROR_NUMBER() <> 2627 THROW END CATCH;
            string sqlQuery = m_mimicTableWriter.CreateMimicSQLInsert(instanceList, sebClass, new SQLServerInsertStatementBuilder(), out paramNameValueMap, additionalColumns);
            SqlServerParamNameValueMap sqlServerMap = paramNameValueMap as SqlServerParamNameValueMap;

            Assert.IsNotNull(sqlServerMap, "The ParamNameValueMap was not a SqlServerParamNameValueMap.");

            Assert.IsTrue(sqlQuery.Contains("TestColumn1234"), "The statement does not contain the name of the added column.");
            Assert.IsTrue(sqlServerMap.Any(p => (string)p.Value.Item1 == "TestColVal1234"), "The map does not contain the value of the added column.");

            }

        [Test]
        public void CreateMimicSEBTestWithDelete ()
            {

            IECClass sebClass = m_schema.GetClass("SpatialEntityBase");

            IECInstance spatialEntityBase = GetInstanceFilledWithTestData(sebClass);

            List<IECInstance> instanceList = new List<IECInstance>() { spatialEntityBase };
            IParamNameValueMap paramNameValueMap;

            string tableString = sebClass.GetCustomAttributes("SQLEntity").GetString("CacheTableName");

            Func<IECInstance, WhereStatementManager> deleteStatementConstructor;

            deleteStatementConstructor = inst =>
            {
                WhereStatementManager deleteStatementManager;
                deleteStatementManager = new WhereStatementManager();
                deleteStatementManager.WhereStatement = "TestCol1234 = @testParam1234@; ";
                deleteStatementManager.AddParameter("@testParam1234@", Bentley.ECObjects.ECObjects.StringType, "TestVal4561");
                //    //If the data is not complete, we delete the cache only if it is outdated or is not complete
                //     return idColumnName + " = " + inst.InstanceId + " AND (Complete = 'false' OR DateCacheCreated <= '" + (DateTime.UtcNow.AddDays(m_daysCacheIsValid*-1) + "'); ");
                
            return deleteStatementManager;
            };

            // BEGIN TRY INSERT INTO dbo.CacheSpatialEntityBases (IdStr,Footprint,Name,Keywords,AssociateFile,ProcessingDescription,DataSourceTypesAvailable,AccuracyResolutionDensity,Date,Classification) VALUES (@p1,geometry::STGeomFromText('POLYGON((-90 34, -89 34, -89 35, -90 35, -90 34))',4326),@p3,@p4,@p5,@p6,@p7,@p8,@p9,@p10); END TRY BEGIN CATCH IF ERROR_NUMBER() <> 2627 THROW END CATCH;
            string sqlQuery = m_mimicTableWriter.CreateMimicSQLInsert(instanceList, sebClass, new SQLServerInsertStatementBuilder(), out paramNameValueMap, null, deleteStatementConstructor);
            SqlServerParamNameValueMap sqlServerMap = paramNameValueMap as SqlServerParamNameValueMap;

            Regex reg = new Regex(@".*DELETE.*FROM.*" + Regex.Escape(tableString) + @".*WHERE.*TestCol1234 =.*");
            Assert.IsTrue(reg.IsMatch(sqlQuery), "The statement does not have the required form.");

            Assert.IsNotNull(sqlServerMap, "The ParamNameValueMap was not a SqlServerParamNameValueMap.");

            Assert.IsTrue(sqlServerMap.Any(p => (string) p.Value.Item1 == "TestVal4561"), "The parameter from the delete statement was not added in the map.");

            }
        }
    }
