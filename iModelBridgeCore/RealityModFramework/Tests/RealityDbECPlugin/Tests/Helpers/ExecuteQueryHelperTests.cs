using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Source.QueryProviders;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests.Helpers
    {
    [TestFixture]
    class ExecuteQueryHelperTests
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
        public void QueryMultipleSourcesTest()
            {
            IECQueryProvider provider1 = (IECQueryProvider) m_mock.StrictMock(typeof(IECQueryProvider));
            IECQueryProvider provider2 = (IECQueryProvider) m_mock.StrictMock(typeof(IECQueryProvider));
            List<IECInstance> list1;
            List<IECInstance> list2;
            List<Tuple<DataSource, IECQueryProvider>> inputList;
            using(m_mock.Record())
                {
                list1 = new List<IECInstance>(){SetupHelpers.CreateIndexSE(m_schema)};
                list1.First().InstanceId = "Test1";
                list2 = new List<IECInstance>(){SetupHelpers.CreateIndexSE(m_schema)};
                list2.First().InstanceId = "Test2";
                Expect.Call(provider1.CreateInstanceList()).Repeat.Once().Return(list1);
                Expect.Call(provider2.CreateInstanceList()).Repeat.Once().Return(list2);

                inputList = new List<Tuple<DataSource, IECQueryProvider>>()
                    {
                    new Tuple<DataSource, IECQueryProvider>(DataSource.Index, provider1), 
                    new Tuple<DataSource, IECQueryProvider>(DataSource.USGS, provider2)
                    };

                }
            using(m_mock.Playback())
                {
                var resultsList = ExecuteQueryHelper.QueryMultipleSources(inputList);

                Assert.IsTrue(resultsList.Any(i => i == list1.First()));
                Assert.IsTrue(resultsList.Any(i => i == list2.First()));
                }
            }
        [Test]
        public void QueryMultipleSourcesTestErrorPartialResults ()
            {
            IECQueryProvider provider1 = (IECQueryProvider) m_mock.StrictMock(typeof(IECQueryProvider));
            IECQueryProvider provider2 = (IECQueryProvider) m_mock.StrictMock(typeof(IECQueryProvider));
            List<IECInstance> list1;
            List<Tuple<DataSource, IECQueryProvider>> inputList;
            using ( m_mock.Record() )
                {
                list1 = new List<IECInstance>() { SetupHelpers.CreateIndexSE(m_schema) };
                list1.First().InstanceId = "Test1";
                Expect.Call(provider1.CreateInstanceList()).Repeat.Once().Return(list1);
                Expect.Call(provider2.CreateInstanceList()).Repeat.Once().Throw(new AggregateException(new List<Exception>(){new Exception("Test")}));

                inputList = new List<Tuple<DataSource, IECQueryProvider>>()
                    {
                    new Tuple<DataSource, IECQueryProvider>(DataSource.Index, provider1), 
                    new Tuple<DataSource, IECQueryProvider>(DataSource.USGS, provider2)
                    };

                }
            using ( m_mock.Playback() )
                {
                var resultsList = ExecuteQueryHelper.QueryMultipleSources(inputList);

                Assert.AreEqual(1, resultsList.Count());
                Assert.IsTrue(resultsList.Any(i => i == list1.First()));
                }
            }
        [Test]
        public void QueryMultipleSourcesTestErrorNoResult ()
            {
            IECQueryProvider provider1 = (IECQueryProvider) m_mock.StrictMock(typeof(IECQueryProvider));
            IECQueryProvider provider2 = (IECQueryProvider) m_mock.StrictMock(typeof(IECQueryProvider));
            List<Tuple<DataSource, IECQueryProvider>> inputList;
            var ex1 = new AggregateException(new List<Exception>() { new Exception("Test1") });
            var ex2 = new AggregateException(new List<Exception>() { new Exception("Test2") });
            using ( m_mock.Record() )
                {
                Expect.Call(provider1.CreateInstanceList()).Repeat.Once().Throw(ex1);
                Expect.Call(provider2.CreateInstanceList()).Repeat.Once().Throw(ex2);

                inputList = new List<Tuple<DataSource, IECQueryProvider>>()
                    {
                    new Tuple<DataSource, IECQueryProvider>(DataSource.Index, provider1), 
                    new Tuple<DataSource, IECQueryProvider>(DataSource.USGS, provider2)
                    };

                }
            using ( m_mock.Playback() )
                {
                var ex = Assert.Throws<AggregateException>(() => ExecuteQueryHelper.QueryMultipleSources(inputList));
                Assert.IsTrue(ex.Equals(ex1) || ex.Equals(ex2));
                }
            }
        [Test]
        public void QueryMultipleSourcesTestErrorPartialResultsUserFriendlyException ()
            {
            IECQueryProvider provider1 = (IECQueryProvider) m_mock.StrictMock(typeof(IECQueryProvider));
            IECQueryProvider provider2 = (IECQueryProvider) m_mock.StrictMock(typeof(IECQueryProvider));
            List<IECInstance> list1;
            List<Tuple<DataSource, IECQueryProvider>> inputList;
            var ex2 = new UserFriendlyException("Test1Ex");
            using ( m_mock.Record() )
                {
                list1 = new List<IECInstance>() { SetupHelpers.CreateIndexSE(m_schema) };
                list1.First().InstanceId = "Test1";
                Expect.Call(provider1.CreateInstanceList()).Repeat.Once().Return(list1);
                Expect.Call(provider2.CreateInstanceList()).Repeat.Once().Throw(ex2);

                inputList = new List<Tuple<DataSource, IECQueryProvider>>()
                    {
                    new Tuple<DataSource, IECQueryProvider>(DataSource.Index, provider1), 
                    new Tuple<DataSource, IECQueryProvider>(DataSource.USGS, provider2)
                    };

                }
            using ( m_mock.Playback() )
                {
                var ex = Assert.Throws<UserFriendlyException>(() => ExecuteQueryHelper.QueryMultipleSources(inputList));
                Assert.IsTrue(ex.Equals(ex2));
                }
            }
        }
    }
