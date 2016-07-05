using System;
using System.Collections.Generic;
using System.Data;
using System.Data.SqlClient;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests
    {
    [TestFixture]
    class InstanceComplementTests
        {

        MockRepository m_mock;
        IECSchema m_schema;
        IDbQuerier m_querierMock;
        [SetUp]
        public void SetUp ()
            {
            m_mock = new MockRepository();

            m_querierMock = (IDbQuerier) m_mock.StrictMock(typeof(IDbQuerier));

            m_schema = SetupHelpers.PrepareSchema();
            }

        [Test]
        public void SDSTest ()
            {

            InstanceComplement instanceComplement = new InstanceComplement(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();

            IECInstance originalInstance = SetupHelpers.CreateSDS(true, m_schema);
            List<IECInstance> instances = new List<IECInstance>() { originalInstance };
            //IECInstance complementInstance = SetupHelpers.CreateSDS(true, m_schema);

            IECInstance complementInstance = m_schema.GetClass("SpatialDataSource").CreateInstance();
            complementInstance.InstanceId = originalInstance.InstanceId;

            string originalURL = originalInstance["MainURL"].StringValue;
            string complementURL = "ComplementURL";

            complementInstance["MainURL"].StringValue = complementURL;

            List<IECInstance> complementInstances = new List<IECInstance>() { complementInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(complementInstances);
                }
            using ( m_mock.Playback() )
                {
                instanceComplement.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(originalURL + ", " + complementURL, instances.First()["MainURL"].StringValue);

                }
            }

        [Test]
        public void OSMSourceTest ()
            {

            InstanceComplement instanceComplement = new InstanceComplement(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();

            IECInstance originalInstance = SetupHelpers.CreateOsmSource(m_schema);
            List<IECInstance> instances = new List<IECInstance>() { originalInstance };
            //IECInstance complementInstance = SetupHelpers.CreateSDS(true, m_schema);

            IECInstance complementInstance = m_schema.GetClass("OsmSource").CreateInstance();
            complementInstance.InstanceId = originalInstance.InstanceId;

            string originalURL = originalInstance["MainURL"].StringValue;
            string complementURL = "ComplementURL";

            complementInstance["MainURL"].StringValue = complementURL;

            List<IECInstance> complementInstances = new List<IECInstance>() { complementInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(complementInstances);
                }
            using ( m_mock.Playback() )
                {
                instanceComplement.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(originalURL + ", " + complementURL, instances.First()["MainURL"].StringValue);

                }
            }

        [Test]
        public void WMSSourceTest ()
            {

            InstanceComplement instanceComplement = new InstanceComplement(m_querierMock);
            SqlConnection sqlConnection = new SqlConnection();

            IECInstance originalInstance = SetupHelpers.CreateWMSSource(m_schema);
            List<IECInstance> instances = new List<IECInstance>() { originalInstance };
            //IECInstance complementInstance = SetupHelpers.CreateSDS(true, m_schema);

            IECInstance complementInstance = m_schema.GetClass("WMSSource").CreateInstance();
            complementInstance.InstanceId = originalInstance.InstanceId;

            string originalURL = originalInstance["MainURL"].StringValue;
            string complementURL = "ComplementURL";

            complementInstance["MainURL"].StringValue = complementURL;

            List<IECInstance> complementInstances = new List<IECInstance>() { complementInstance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Anything,
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IDbConnection>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(complementInstances);
                }
            using ( m_mock.Playback() )
                {
                instanceComplement.Modify(instances, DataSource.USGS, sqlConnection);

                Assert.AreEqual(originalURL + ", " + complementURL, instances.First()["MainURL"].StringValue);

                }
            }
        }
    }
