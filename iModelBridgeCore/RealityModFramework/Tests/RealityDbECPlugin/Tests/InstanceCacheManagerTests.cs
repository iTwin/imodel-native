using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
using Rhino.Mocks;
using Rhino.Mocks.Constraints;

namespace IndexECPlugin.Tests.Tests
    {
    [TestFixture]
    class InstanceCacheManagerTests
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
        public void QueryInstancesFromCacheTest()
            {
            InstanceCacheManager icm = new InstanceCacheManager(DataSource.USGS, 1, null, m_querierMock);
            IECInstance instance = SetupHelpers.CreateUsgsSDS(true, m_schema);
            instance.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("DateCacheCreated", DateTime.Now));
            List<IECInstance> instanceList = new List<IECInstance>(){instance};
            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                  Arg<DataReadingHelper>.Is.Anything,
                                  Arg<IParamNameValueMap>.Is.Anything,
                                  Arg<IECClass>.Is.Same(instance.ClassDefinition),
                                  Arg<IEnumerable<IECProperty>>.Is.Anything,
                                  Arg<IEnumerable<string>>.Is.Anything,
                                  Arg<bool>.Is.Anything)).Repeat.Once().Return(instanceList);
                }
            using (m_mock.Playback())
                {
                List<string> idList = new List<string>(){instance.InstanceId};
                var returnedList = icm.QueryInstancesFromCache(idList, instance.ClassDefinition, instance.ClassDefinition, new SelectCriteria(true));
                Assert.AreSame(instanceList.First(), returnedList.First(), "The entity should have been returned in the list.");
                }
            }

        [Test]
        public void QueryInstancesFromCacheTestDerivedType ()
            {
            InstanceCacheManager icm = new InstanceCacheManager(DataSource.USGS, 1, null, m_querierMock);
            IECInstance instance = SetupHelpers.CreateMultibandSource(m_schema);
            instance.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("Complete", true));
            instance.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("DateCacheCreated", DateTime.Now));
            var baseClass = m_schema.GetClass("SpatialDataSource");
            var derivedClass = m_schema.GetClass("MultibandSource");
            var removedProp = derivedClass.FindProperty("RedBandURL");
            var keptProp = derivedClass.FindProperty("MainURL");
            var selectCriteria = new SelectCriteria(new List<IECProperty>() { removedProp, keptProp });

            List<IECInstance> instanceList = new List<IECInstance>() { instance };
            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                  Arg<DataReadingHelper>.Is.Anything,
                                  Arg<IParamNameValueMap>.Is.Anything,
                                  Arg<IECClass>.Is.Same(instance.ClassDefinition),
                                  Arg<IEnumerable<IECProperty>>.Matches(new PredicateConstraint<IEnumerable<IECProperty>>(l => !l.Contains(removedProp) && l.Contains(keptProp))),
                                  Arg<IEnumerable<string>>.Is.Anything,
                                  Arg<bool>.Is.Anything)).Repeat.Once().Return(instanceList);
                }
            using ( m_mock.Playback() )
                {
                List<string> idList = new List<string>() { instance.InstanceId };
                var returnedList = icm.QueryInstancesFromCache(idList, derivedClass, baseClass, selectCriteria);
                Assert.AreSame(instanceList.First(), returnedList.First(), "The entity should have been returned in the list.");
                }
            }

        [Test]
        public void QueryOutdatedInstancesFromCacheTest ()
            {
            InstanceCacheManager icm = new InstanceCacheManager(DataSource.USGS, 1, null, m_querierMock);
            IECInstance instance = SetupHelpers.CreateUsgsSDS(true, m_schema);
            instance.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("DateCacheCreated", DateTime.Now.AddDays(-10)));
            List<IECInstance> instanceList = new List<IECInstance>() { instance };
            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                  Arg<DataReadingHelper>.Is.Anything,
                                  Arg<IParamNameValueMap>.Is.Anything,
                                  Arg<IECClass>.Is.Same(instance.ClassDefinition),
                                  Arg<IEnumerable<IECProperty>>.Is.Anything,
                                  Arg<IEnumerable<string>>.Is.Anything,
                                  Arg<bool>.Is.Anything)).Repeat.Once().Return(instanceList);
                }
            using ( m_mock.Playback() )
                {
                List<string> idList = new List<string>() { instance.InstanceId };
                var returnedList = icm.QueryInstancesFromCache(idList, instance.ClassDefinition, instance.ClassDefinition, new SelectCriteria(true));
                Assert.AreEqual(0, returnedList.Count, "The outdated entity should not have been added.");
                }
            }

        
        [Test]
        public void QuerySpatialInstancesFromCacheTest ()
            {
            InstanceCacheManager icm = new InstanceCacheManager(DataSource.USGS, 1, null, m_querierMock);
            IECInstance instance = SetupHelpers.CreateUsgsSEWDV(true, m_schema);
            IECInstance instance2 = SetupHelpers.CreateUsgsSEWDV(true, m_schema);
            IECInstance instance3 = SetupHelpers.CreateUsgsSEWDV(true, m_schema);
            instance.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("DateCacheCreated", DateTime.Now));
            instance.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("MetadataDateCreated", DateTime.Now));
            instance.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("SDSDateCreated", DateTime.Now));
            instance.ExtendedDataValueSetter.Add("MetadataComplete", false);
            instance.ExtendedDataValueSetter.Add("SDSComplete", true);

            instance2["Footprint"].StringValue = "{ \"points\" : [[-180,-90],[-180,-89],[-179,-89],[-179,-90],[-180,-90]], \"coordinate_system\" : \"4326\" }";
            instance2.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("DateCacheCreated", DateTime.Now));
            instance2.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("MetadataDateCreated", DateTime.Now));
            instance2.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("SDSDateCreated", DateTime.Now));
            instance2.ExtendedDataValueSetter.Add("MetadataComplete", true);
            instance2.ExtendedDataValueSetter.Add("SDSComplete", true);

            instance3.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("DateCacheCreated", DateTime.Now.AddDays(-5)));
            instance3.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("MetadataDateCreated", DateTime.Now));
            instance3.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("SDSDateCreated", DateTime.Now));
            instance3.ExtendedDataValueSetter.Add("MetadataComplete", false);
            instance3.ExtendedDataValueSetter.Add("SDSComplete", true);

            var baseClass = m_schema.GetClass("SpatialEntityWithDetailsView");

            var selectedProp = baseClass.FindProperty("Name");
            var selectCriteria = new SelectCriteria(new List<IECProperty>() { selectedProp });

            List<IECInstance> instanceList = new List<IECInstance>() { instance, instance2, instance3 };
            PolygonDescriptor polyDesc = new PolygonDescriptor(){ SRID = 4326, WKT = "POLYGON ((30 10, 40 40, 20 40, 10 20, 30 10))" };
            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                  Arg<DataReadingHelper>.Is.Anything,
                                  Arg<IParamNameValueMap>.Is.Anything,
                                  Arg<IECClass>.Is.Same(instance.ClassDefinition),
                                  Arg<IEnumerable<IECProperty>>.Is.Anything,
                                  Arg<IEnumerable<string>>.Is.Anything,
                                  Arg<bool>.Is.Anything)).Repeat.Once().Return(instanceList);
                }
            using ( m_mock.Playback() )
                {
                List<string> idList = new List<string>() { instance.InstanceId };
                var returnedList = icm.QuerySpatialInstancesFromCache(polyDesc, instance.ClassDefinition, instance.ClassDefinition, selectCriteria, new List<SingleWhereCriteriaHolder>());
                Assert.AreEqual(1, returnedList.Count());
                Assert.AreSame(instanceList.First(), returnedList.First(), "The entity should have been returned in the list.");
                Assert.AreEqual(false, instanceList.First().ExtendedData["Complete"]);
                }
            }

        [Test]
        public void QueryOutdatedSpatialInstancesFromCacheTest ()
            {
            InstanceCacheManager icm = new InstanceCacheManager(DataSource.USGS, 1, null, m_querierMock);
            IECInstance instance = SetupHelpers.CreateUsgsSEWDV(true, m_schema);
            instance.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("DateCacheCreated", DateTime.Now));
            instance.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("MetadataDateCreated", DateTime.Now.AddDays(-10)));
            instance.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("SDSDateCreated", DateTime.Now));
            instance.ExtendedDataValueSetter.Add("MetadataComplete", true);
            instance.ExtendedDataValueSetter.Add("SDSComplete", true);

            List<IECInstance> instanceList = new List<IECInstance>() { instance };
            PolygonDescriptor polyDesc = new PolygonDescriptor()
            {
                SRID = 4326, WKT = "POLYGON ((30 10, 40 40, 20 40, 10 20, 30 10))"
            };
            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                  Arg<DataReadingHelper>.Is.Anything,
                                  Arg<IParamNameValueMap>.Is.Anything,
                                  Arg<IECClass>.Is.Same(instance.ClassDefinition),
                                  Arg<IEnumerable<IECProperty>>.Is.Anything,
                                  Arg<IEnumerable<string>>.Is.Anything,
                                  Arg<bool>.Is.Anything)).Repeat.Once().Return(instanceList);
                }
            using ( m_mock.Playback() )
                {
                List<string> idList = new List<string>() { instance.InstanceId };
                var returnedList = icm.QuerySpatialInstancesFromCache(polyDesc, instance.ClassDefinition, instance.ClassDefinition, new SelectCriteria(true), new List<SingleWhereCriteriaHolder>());
                Assert.AreEqual(0, returnedList.Count, "The outdated entity should not have been added.");
                }
            }
        [Test]
        public void PrepareCacheStatementTest ()
            {
            InstanceCacheManager icm = new InstanceCacheManager(DataSource.USGS, 1, null, m_querierMock);
            IECInstance instance = SetupHelpers.CreateUsgsSEWDV(true, m_schema);
            instance.ExtendedDataValueSetter.Add("Test", "Test");
            List<Tuple<string, IECType, Func<IECInstance, string>>> additionalColumns = new List<Tuple<string, IECType, Func<IECInstance, string>>>();
            additionalColumns.Add(new Tuple<string, IECType, Func<IECInstance, string>>("Test", Bentley.ECObjects.ECObjects.StringType, inst => inst.ExtendedData["Test"].ToString()));
            List<IECInstance> instanceList = new List<IECInstance>() { instance };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.ExecuteNonQueryInDb(Arg<string>.Is.Anything, Arg<IParamNameValueMap>.Is.Anything)).Repeat.Once().Return(1);
                }
            using ( m_mock.Playback() )
                {
                icm.PrepareCacheInsertStatement(instanceList, instance.ClassDefinition, additionalColumns);
                icm.SendAllPreparedCacheInsertStatements();
                }
            }
        }
    }