using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Instance;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Source.QueryProviders;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests
    {
    [TestFixture]
    class SqlQueryProviderTests
        {
        MockRepository m_mock;
        IECSchema m_schema;
        private IDbQuerier m_querierMock;


        [SetUp]
        public void setup()
            {
            m_mock = new MockRepository();
            m_querierMock = (IDbQuerier) m_mock.StrictMock(typeof(IDbQuerier));
            m_schema = SetupHelpers.PrepareSchema();
            }

        [Test]
        public void SqlQueryProviderBaseTest ()
            {
            IECClass seClass = m_schema.GetClass("SpatialEntity");
            var instance = SetupHelpers.CreateIndexSE(m_schema);

            List<IECInstance> dbInstanceList = new List<IECInstance>() { instance };
            using ( m_mock.Record())
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Equal(seClass),
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(dbInstanceList);

                }
            using ( m_mock.Playback() )
                {

                ECQuery query = new ECQuery(seClass);
                query.SelectClause.SelectAllProperties = true;

                var provider = new SqlQueryProvider(query, new ECQuerySettings(), m_querierMock, m_schema);

                var resultInstances = provider.CreateInstanceList();

                Assert.AreEqual(1, resultInstances.Count(), "There was not exactly one instance returned.");
                Assert.AreEqual(instance.InstanceId, resultInstances.First().InstanceId, "The returned instance's properties are incorrect.");
                }
            }

        [Test]
        public void SqlQueryProviderRelatedQueryTest ()
            {
            IECClass seClass = m_schema.GetClass("SpatialEntity");
            IECClass metadataClass = m_schema.GetClass("Metadata");
            var instance = SetupHelpers.CreateIndexSE(m_schema);
            var relInst = SetupHelpers.CreateIndexMetadata(m_schema);
            relInst.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("relInstID", instance.InstanceId));
            List<IECInstance> dbInstanceList = new List<IECInstance>() { instance };
            List<IECInstance> dbRelInstanceList = new List<IECInstance>() { relInst };

            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Equal(seClass),
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(dbInstanceList);
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Equal(metadataClass),
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(dbRelInstanceList);
                }
            using ( m_mock.Playback() )
                {

                ECQuery query = new ECQuery(seClass);

                IECRelationshipClass metadataRelClass = m_schema.GetClass("SpatialEntityToMetadata") as IECRelationshipClass;
                RelatedInstanceSelectCriteria metadataRelCrit = new RelatedInstanceSelectCriteria(new QueryRelatedClassSpecifier(metadataRelClass, RelatedInstanceDirection.Forward, metadataClass), true);
                metadataRelCrit.SelectAllProperties = true;

                query.SelectClause.SelectAllProperties = true;
                query.SelectClause.SelectedRelatedInstances.Add(metadataRelCrit);

                var provider = new SqlQueryProvider(query, new ECQuerySettings(), m_querierMock, m_schema);

                var resultInstances = provider.CreateInstanceList();

                Assert.AreEqual(1, resultInstances.Count(), "There was not exactly one instance returned.");
                Assert.AreEqual(1, resultInstances.First().GetRelationshipInstances().Count());

                var resultRelInst = resultInstances.First().GetRelationshipInstances().First().Target;

                Assert.AreEqual(relInst.InstanceId, resultRelInst.InstanceId);
                Assert.AreEqual(instance.InstanceId, resultInstances.First().InstanceId, "The returned instance's properties are incorrect.");
                }
            }

        [Test]
        public void SqlQueryProviderConstructorErrorTest()
            {
            IECClass seClass = m_schema.GetClass("SpatialEntity");
            ECQuery query = new ECQuery(seClass);
            query.SelectClause.SelectAllProperties = true;

            SqlQueryProvider provider1;
            SqlQueryProvider provider2;
            SqlQueryProvider provider3;
            SqlQueryProvider provider4;

            Assert.Throws<ArgumentNullException>(() => provider1 = new SqlQueryProvider(null, new ECQuerySettings(), m_querierMock, m_schema), "The constructor did not throw an exception after having been passed invalid parameters.");
            Assert.Throws<ArgumentNullException>(() => provider2 = new SqlQueryProvider(query, new ECQuerySettings(), null, m_schema), "The constructor did not throw an exception after having been passed invalid parameters.");
            Assert.Throws<ArgumentNullException>(() => provider3 = new SqlQueryProvider(query, new ECQuerySettings(), m_querierMock, null), "The constructor did not throw an exception after having been passed invalid parameters.");

            query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("polygon", "invalid value"));

            Assert.Throws<UserFriendlyException>(() => provider4 = new SqlQueryProvider(query, new ECQuerySettings(), m_querierMock, m_schema));

            }

        [Test]
        public void SqlQueryProviderSpatialTest ()
            {
            IECClass seClass = m_schema.GetClass("SpatialEntity");
            var instance = SetupHelpers.CreateIndexSE(m_schema);

            List<IECInstance> dbInstanceList = new List<IECInstance>() { instance };
            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Equal(seClass),
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(dbInstanceList);
                }
            using ( m_mock.Playback() )
                {

                ECQuery query = new ECQuery(seClass);
                query.SelectClause.SelectAllProperties = false;
                query.SelectClause.SelectedProperties.Add(seClass["Id"]);

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }"));

                var provider = new SqlQueryProvider(query, new ECQuerySettings(), m_querierMock, m_schema);

                var resultInstances = provider.CreateInstanceList();

                Assert.AreEqual(1, resultInstances.Count(), "There was not exactly one instance returned.");
                Assert.AreEqual(instance.InstanceId, resultInstances.First().InstanceId, "The returned instance's properties are incorrect.");
                //Assert.IsTrue(instance.First(propVal => propVal.Property.IsSpatial()).IsNull, "The spatial property should not be present in the result");
                }
            }

        //This test should be removed when the "bounding box" filtering is removed.
        [Test]
        public void SqlQueryProviderSpatialFilteringTemporaryTest ()
            {
            IECClass seClass = m_schema.GetClass("SpatialEntity");
            var instance = SetupHelpers.CreateIndexSE(m_schema);
            instance["Footprint"].StringValue = "{ \"points\" : [[-95,41.32950370684],[-94,41.32950370684],[-94,41.4227313251356],[-95,41.4227313251356],[-95,41.32950370684]], \"coordinate_system\" : \"4326\" }";

            List<IECInstance> dbInstanceList = new List<IECInstance>() { instance };
            using ( m_mock.Record() )
                {
                Expect.Call(m_querierMock.QueryDbForInstances(Arg<string>.Is.Anything,
                                                              Arg<DataReadingHelper>.Is.Anything,
                                                              Arg<IParamNameValueMap>.Is.Anything,
                                                              Arg<IECClass>.Is.Equal(seClass),
                                                              Arg<IEnumerable<IECProperty>>.Is.Anything,
                                                              Arg<IEnumerable<string>>.Is.Anything,
                                                              Arg<bool>.Is.Anything)).Repeat.Once().Return(dbInstanceList);
                }
            using ( m_mock.Playback() )
                {

                ECQuery query = new ECQuery(seClass);
                query.SelectClause.SelectAllProperties = false;
                query.SelectClause.SelectedProperties.Add(seClass["Id"]);

                query.ExtendedDataValueSetter.Add(new KeyValuePair<string, object>("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }"));

                var provider = new SqlQueryProvider(query, new ECQuerySettings(), m_querierMock, m_schema);

                var resultInstances = provider.CreateInstanceList();

                Assert.AreEqual(0, resultInstances.Count(), "An instance was returned, none was expected.");
                }
            }
        }
    }
