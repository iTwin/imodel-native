using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests
    {
    [TestFixture]
    class SqlQueryHelpersTests
        {
        MockRepository m_mock;
        IECSchema m_schema;

        [SetUp]
        public void SetUp ()
            {
            m_mock = new MockRepository();
            m_schema = SetupHelpers.PrepareSchema();
            }

        [Test]
        public void QueryDbForInstancesTests ()
            {
            IECInstance spatialEntityInst = SetupHelpers.CreateIndexSE(m_schema);

            //IECInstance serverInst = m_schema.GetClass("Server").CreateInstance();

            //serverInst.InitializePropertiesToNull();
            

            string sqlConnectionString = "TestConnectionString";
            string sqlCommandString = "TestCommandString";
            DataReadingHelper drh = new DataReadingHelper();
            List<IECProperty> propertyList = new List<IECProperty>();
            List<string> nonInstanceDataColumnList = new List<string>();

            IECProperty idProp = spatialEntityInst.ClassDefinition.First(p => p.Name == "Id");
            IECProperty spatialProp = spatialEntityInst.ClassDefinition.First(p => p.IsSpatial());
            drh.AddColumn(ColumnCategory.instanceData, idProp);
            drh.AddColumn(ColumnCategory.spatialInstanceData, spatialProp);
            drh.AddColumn(ColumnCategory.streamData, null);
            drh.AddColumn(ColumnCategory.relatedInstanceId, null);

            drh.AddNonPropertyDataColumn("TestColumnName");
            nonInstanceDataColumnList.Add("TestColumnName");

            propertyList.Add(idProp);
            propertyList.Add(spatialProp);

            IDbConnectionCreator connectionCreatorMock = (IDbConnectionCreator) m_mock.StrictMock(typeof(IDbConnectionCreator));
            IDbConnection dbConnectionMock = (IDbConnection) m_mock.StrictMock(typeof(IDbConnection));
            IDbCommand dbCommandMock = (IDbCommand) m_mock.StrictMock(typeof(IDbCommand));
            IDataReader dataReaderMock = (IDataReader) m_mock.DynamicMock(typeof(IDataReader));
            IDbDataParameter dbDataParameterMock = (IDbDataParameter) m_mock.StrictMock(typeof(IDbDataParameter));
            IDataParameterCollection dataParameterCollection = (IDataParameterCollection) m_mock.StrictMock(typeof(IDataParameterCollection));

            SetupResult.For(dbCommandMock.Parameters).Return(dataParameterCollection);
            DbQuerier dbQuerier = new DbQuerier(sqlConnectionString, connectionCreatorMock);
            GenericParamNameValueMap paramNameValueMap = new GenericParamNameValueMap();
            paramNameValueMap.AddParamNameValue("@test1", "testVal", DbType.String);
            using(m_mock.Record())
                {
                Expect.Call(connectionCreatorMock.CreateDbConnection(Arg<String>.Is.Equal(sqlConnectionString))).Repeat.Once().Return(dbConnectionMock);
                Expect.Call(delegate {dbConnectionMock.Open();}).Repeat.Once();
                Expect.Call(delegate {dbConnectionMock.Close();}).Repeat.Once();
                Expect.Call(dbConnectionMock.CreateCommand()).Repeat.Once().Return(dbCommandMock);
                //Expect.Call(dbCommandMock.CommandType).SetPropertyWithArgument(Arg<CommandType>.Is.Equal(CommandType.Text)).Repeat.Once();
                //Expect.Call(dbCommandMock.CommandText).SetPropertyWithArgument(Arg<String>.Is.Equal(sqlCommandString)).Repeat.Once();
                //Expect.Call(dbCommandMock.Connection).SetPropertyWithArgument(Arg<IDbConnection>.Is.Same(dbConnectionMock)).Repeat.Once();
                dbCommandMock.CommandType = CommandType.Text;
                dbCommandMock.CommandText = sqlCommandString;
                dbCommandMock.Connection = dbConnectionMock;
                Expect.Call(dbCommandMock.CreateParameter()).Repeat.Once().Return(dbDataParameterMock);
                Expect.Call(dbCommandMock.ExecuteReader()).Repeat.Once().Return(dataReaderMock);
                Expect.Call(dataParameterCollection.Add(Arg<IDbDataParameter>.Is.Same(dbDataParameterMock))).Repeat.Once().Return(0);
                //Expect.Call(dbDataParameterMock.DbType).SetPropertyWithArgument(Arg<DbType>.Is.Equal(DbType.String));
                //Expect.Call(dbDataParameterMock.ParameterName).SetPropertyWithArgument(Arg<String>.Is.Equal("@test1"));
                //Expect.Call(dbDataParameterMock.Value).SetPropertyWithArgument(Arg<string>.Is.Equal("testVal"));
                dbDataParameterMock.DbType = DbType.String;
                dbDataParameterMock.ParameterName = "@test1";
                dbDataParameterMock.Value = "testVal";

                //These two calls make the other dataReaderMock expectation calls fail... for unknown reason
                //Expect.Call(dataReaderMock[(Arg<int>.Is.Anything)]).Repeat.Once().Return("hello");
                //Expect.Call(dataReaderMock.GetValue(0)).Repeat.Once().Return(null);

                Expect.Call(dataReaderMock.IsDBNull(Arg<int>.Is.Equal(drh.getInstanceDataColumn(idProp).Value))).Repeat.Once().Return(spatialEntityInst[idProp.Name].IsNull);
                Expect.Call(dataReaderMock.IsDBNull(Arg<int>.Is.Equal(drh.getInstanceDataColumn(spatialProp).Value))).Repeat.Once().Return(false);
                Expect.Call(dataReaderMock.GetString(Arg<int>.Is.Equal(drh.getInstanceDataColumn(idProp).Value))).Repeat.Once().Return(spatialEntityInst[idProp.Name].StringValue);
                Expect.Call(dataReaderMock.GetString(Arg<int>.Is.Equal(drh.getInstanceDataColumn(spatialProp).Value))).Repeat.Once().Return("POLYGON((-180 -90, -180 90, 180 90, 180 -90, -180 -90))");
                Expect.Call(dataReaderMock.GetInt32(Arg<int>.Is.Equal(drh.getInstanceDataColumn(spatialProp).Value + 1))).Repeat.Once().Return(4326);

                Expect.Call(dataReaderMock.Read()).Repeat.Once().Return(true);
                Expect.Call(dataReaderMock.Read()).Repeat.Once().Return(false);


                Expect.Call(delegate {dbConnectionMock.Dispose();}).Repeat.Once();
                Expect.Call(delegate {dbCommandMock.Dispose();}).Repeat.Once();
                Expect.Call(delegate {dataReaderMock.Dispose();}).Repeat.Once();

                }
            using ( m_mock.Playback() )
                {
                List<IECInstance> resultList = dbQuerier.QueryDbForInstances(sqlCommandString, drh, paramNameValueMap, spatialEntityInst.ClassDefinition, propertyList, nonInstanceDataColumnList);
                
                Assert.AreEqual(1, resultList.Count);

                IECInstance resultInst = resultList.First();

                Assert.AreEqual(spatialEntityInst["Id"].StringValue, resultInst["Id"].StringValue);
                Assert.IsTrue(resultInst.ExtendedData.ContainsKey("relInstID"));
                Assert.IsNull(resultInst.ExtendedData["relInstID"]);
                Assert.AreEqual(spatialEntityInst["Footprint"].StringValue, resultInst["Footprint"].StringValue);

                StreamBackedDescriptor desc;
                Assert.IsTrue(StreamBackedDescriptorAccessor.TryGetFrom(resultInst, out desc));
                }
            }

        [Test]
        public void ExecuteNonQueryInDbTest()
            {
            string sqlConnectionString = "TestConnectionString";
            string sqlCommandString = "TestCommandString";

            IDbConnectionCreator connectionCreatorMock = (IDbConnectionCreator) m_mock.StrictMock(typeof(IDbConnectionCreator));
            IDbConnection dbConnectionMock = (IDbConnection) m_mock.StrictMock(typeof(IDbConnection));
            IDbCommand dbCommandMock = (IDbCommand) m_mock.StrictMock(typeof(IDbCommand));

            GenericParamNameValueMap paramNameValueMap = new GenericParamNameValueMap();


            DbQuerier dbQuerier = new DbQuerier(sqlConnectionString, connectionCreatorMock);

            using (m_mock.Record())
                {
                Expect.Call(connectionCreatorMock.CreateDbConnection(Arg<String>.Is.Equal(sqlConnectionString))).Repeat.Once().Return(dbConnectionMock);
                Expect.Call(delegate {dbConnectionMock.Open();}).Repeat.Once();
                Expect.Call(delegate {dbConnectionMock.Close();}).Repeat.Once();
                Expect.Call(dbConnectionMock.CreateCommand()).Repeat.Once().Return(dbCommandMock);
                
                dbCommandMock.CommandType = CommandType.Text;
                dbCommandMock.CommandText = sqlCommandString;
                dbCommandMock.Connection = dbConnectionMock;

                Expect.Call(dbCommandMock.ExecuteNonQuery()).Repeat.Once().Return(0);

                Expect.Call(delegate {dbConnectionMock.Dispose();}).Repeat.Once();
                Expect.Call(delegate {dbCommandMock.Dispose();}).Repeat.Once();
                }
            using (m_mock.Playback())
                {
                int result = dbQuerier.ExecuteNonQueryInDb(sqlCommandString, paramNameValueMap);

                Assert.AreEqual(0, result);
                }
            }
        }
    }
