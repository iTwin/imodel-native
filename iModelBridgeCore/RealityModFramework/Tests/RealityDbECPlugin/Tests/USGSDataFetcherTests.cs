using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using Bentley.EC.Persistence.Operations;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Schema;
using Bentley.Exceptions;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests
    {
    [TestFixture]
    class USGSDataFetcherTests
        {
        MockRepository m_mock;
        IECSchema m_schema;
        [SetUp]
        public void SetUp()
            {
            m_mock = new MockRepository();
            m_schema = SetupHelpers.PrepareSchema();
            }

        [Test]
        public void GetSciencebaseJsonTest()
            {
            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));
            const string url = "TestURL";
            string scienceBaseJsonString;
            using ( StreamReader scienceBaseJsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("ScienceBaseJsonNED1m.txt")) )
                {
                scienceBaseJsonString = scienceBaseJsonStreamReader.ReadToEnd();
                }
            using(m_mock.Record())
                {
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Once().Return(scienceBaseJsonString);
                }
            using(m_mock.Playback())
                {
                var dataFetcher = new USGSDataFetcher(new ECQuery(), httpResponseGetterMock);

                var jObject = dataFetcher.GetSciencebaseJson(url);

                Assert.AreEqual("581d23cee4b08da350d5761d", jObject.TryToGetString("id"), "The created JObject is invalid");
                }
            }

        [Test]
        public void GetXmlDocFromURLTest ()
            {
            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));
            const string url = "TestURL";
            string xmlDocString;
            using ( StreamReader metadataFileStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("USGS_NED_one_meter_x24y459_IL_12_County_HenryCO_2009_IMG_2015_meta.xml")) )
                {
                xmlDocString = metadataFileStreamReader.ReadToEnd();
                }

            using ( m_mock.Record() )
                {
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Once().Return(xmlDocString);
                }
            using ( m_mock.Playback() )
                {
                var dataFetcher = new USGSDataFetcher(new ECQuery(), httpResponseGetterMock);

                var xmlDoc = dataFetcher.GetXmlDocFromURL(url);

                Assert.AreEqual("USGS NED one meter x24y459 IL 12-County-HenryCO 2009 IMG 2015", xmlDoc["metadata"]["idinfo"]["citation"]["citeinfo"]["title"].InnerText, "The created XmlDoc is invalid");
                }
            }

        [Test]
        public void GetNonFormattedUSGSResultsTest ()
            {
            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));
            string tnmApiDatasetsJsonString;
            using ( StreamReader tnmApiDatasetsJsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiDatasets.txt")) )
                {
                tnmApiDatasetsJsonString = tnmApiDatasetsJsonStreamReader.ReadToEnd();
                }
            string tnmApiJsonString;
            using ( StreamReader tnmApiJsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiHRO.txt")) )
                {
                tnmApiJsonString = tnmApiJsonStreamReader.ReadToEnd();
                }
            using ( m_mock.Record() )
                {
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Once().Return(tnmApiDatasetsJsonString);
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Times(6).Return(tnmApiJsonString);
                }
            using ( m_mock.Playback() )
                {
                var query = new ECQuery();
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");
                var dataFetcher = new USGSDataFetcher(query, httpResponseGetterMock);

                var criteriaList = new List<SingleWhereCriteriaHolder>();
                var sewdvClass = m_schema.GetClass("SpatialEntityWithDetailsView");
                criteriaList.Add(new SingleWhereCriteriaHolder(){Operator = RelationalOperator.IN, Property = sewdvClass["Classification"], Value = "Imagery,Terrain"});

                IEnumerable<USGSRequestBundle> resultList = dataFetcher.GetNonFormattedUSGSResults(criteriaList);

                Assert.AreEqual(6, resultList.Count());
                foreach ( var result in resultList )
                    {
                    Assert.IsTrue(dataFetcher.CategoryTable.Any(category => category.SbDatasetTag == result.Dataset), "The dataset returned should be present in the category table");
                    }

                }
            }

        [Test]
        public void GetNonFormattedUSGSResultsTest2 ()
            {
            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));
            string tnmApiDatasetsJsonString;
            using ( StreamReader tnmApiDatasetsJsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiDatasets.txt")) )
                {
                tnmApiDatasetsJsonString = tnmApiDatasetsJsonStreamReader.ReadToEnd();
                }
            string tnmApiJsonString;
            using ( StreamReader tnmApiJsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiHRO.txt")) )
                {
                tnmApiJsonString = tnmApiJsonStreamReader.ReadToEnd();
                }
            using ( m_mock.Record() )
                {
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Once().Return(tnmApiDatasetsJsonString);
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Times(2).Return(tnmApiJsonString);
                }
            using ( m_mock.Playback() )
                {
                var query = new ECQuery();
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");
                var dataFetcher = new USGSDataFetcher(query, httpResponseGetterMock);

                var criteriaList = new List<SingleWhereCriteriaHolder>();
                var sewdvClass = m_schema.GetClass("SpatialEntityWithDetailsView");
                criteriaList.Add(new SingleWhereCriteriaHolder(){Operator = RelationalOperator.IN, Property = sewdvClass["Classification"], Value = "Imagery,Terrain"});
                criteriaList.Add(new SingleWhereCriteriaHolder(){Operator = RelationalOperator.EQ, Property = sewdvClass["Classification"], Value = "Imagery"});

                IEnumerable<USGSRequestBundle> resultList = dataFetcher.GetNonFormattedUSGSResults(criteriaList);

                Assert.AreEqual(2, resultList.Count());
                foreach ( var result in resultList )
                    {
                    Assert.IsTrue(dataFetcher.CategoryTable.Any(category => category.SbDatasetTag == result.Dataset), "The dataset returned should be present in the category table");
                    }
                }
            }

        [Test]
        public void GetNonFormattedUSGSResultsTest3 ()
            {
            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));
            string tnmApiDatasetsJsonString;
            using ( StreamReader tnmApiDatasetsJsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiDatasets.txt")) )
                {
                tnmApiDatasetsJsonString = tnmApiDatasetsJsonStreamReader.ReadToEnd();
                }
            string tnmApiJsonString;
            using ( StreamReader tnmApiJsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiHRO.txt")) )
                {
                tnmApiJsonString = tnmApiJsonStreamReader.ReadToEnd();
                }
            using ( m_mock.Record() )
                {
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Once().Return(tnmApiDatasetsJsonString);
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Once().Return(tnmApiJsonString);
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Once().Throw(new OperationFailedException());
                }
            using ( m_mock.Playback() )
                {
                var query = new ECQuery();
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");
                var dataFetcher = new USGSDataFetcher(query, httpResponseGetterMock);

                var criteriaList = new List<SingleWhereCriteriaHolder>();
                var sewdvClass = m_schema.GetClass("SpatialEntityWithDetailsView");
                criteriaList.Add(new SingleWhereCriteriaHolder()
                {
                    Operator = RelationalOperator.IN, Property = sewdvClass["Classification"], Value = "Imagery,Terrain"
                });
                criteriaList.Add(new SingleWhereCriteriaHolder()
                {
                    Operator = RelationalOperator.EQ, Property = sewdvClass["Classification"], Value = "Imagery"
                });

                IEnumerable<USGSRequestBundle> resultList = dataFetcher.GetNonFormattedUSGSResults(criteriaList);

                Assert.AreEqual(1, resultList.Count());
                foreach ( var result in resultList )
                    {
                    Assert.IsTrue(dataFetcher.CategoryTable.Any(category => category.SbDatasetTag == result.Dataset), "The dataset returned should be present in the category table");
                    }
                }
            }

        [Test]
        public void GetNonFormattedUSGSResultsErrorDatasetsTest ()
            {
            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));
            using ( m_mock.Record() )
                {
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Once().Throw(new TaskCanceledException());
                }
            using ( m_mock.Playback() )
                {
                var query = new ECQuery();
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");
                var dataFetcher = new USGSDataFetcher(query, httpResponseGetterMock);

                var criteriaList = new List<SingleWhereCriteriaHolder>();
                var sewdvClass = m_schema.GetClass("SpatialEntityWithDetailsView");
                criteriaList.Add(new SingleWhereCriteriaHolder()
                {
                    Operator = RelationalOperator.IN, Property = sewdvClass["Classification"], Value = "Imagery,Terrain"
                });

                IEnumerable<USGSRequestBundle> resultList;
                Assert.Throws<EnvironmentalException>(() => resultList = dataFetcher.GetNonFormattedUSGSResults(criteriaList), "The call to GetNonFormattedUSGSResults should have thrown an EnvironmentalException");

                }
            }

        [Test]
        public void GetNonFormattedUSGSResultsErrorDatasetsTest2 ()
            {
            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));
            using ( m_mock.Record() )
                {
                var ex = new AggregateException(new List<Exception>(){new TaskCanceledException()});
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Once().Throw(ex);
                }
            using ( m_mock.Playback() )
                {
                var query = new ECQuery();
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");
                var dataFetcher = new USGSDataFetcher(query, httpResponseGetterMock);

                var criteriaList = new List<SingleWhereCriteriaHolder>();
                var sewdvClass = m_schema.GetClass("SpatialEntityWithDetailsView");
                criteriaList.Add(new SingleWhereCriteriaHolder()
                {
                    Operator = RelationalOperator.IN, Property = sewdvClass["Classification"], Value = "Imagery,Terrain"
                });

                IEnumerable<USGSRequestBundle> resultList;
                Assert.Throws<EnvironmentalException>(() => resultList = dataFetcher.GetNonFormattedUSGSResults(criteriaList), "The call to GetNonFormattedUSGSResults should have thrown an EnvironmentalException");

                }
            }

        [Test]
        public void GetNonFormattedUSGSResultsErrorTest ()
            {
            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));
            string tnmApiDatasetsJsonString;
            using ( StreamReader tnmApiDatasetsJsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiDatasets.txt")) )
                {
                tnmApiDatasetsJsonString = tnmApiDatasetsJsonStreamReader.ReadToEnd();
                }
            using ( m_mock.Record() )
                {
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Once().Return(tnmApiDatasetsJsonString);
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Times(6).Throw(new TaskCanceledException());
                }
            using ( m_mock.Playback() )
                {
                var query = new ECQuery();
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");
                var dataFetcher = new USGSDataFetcher(query, httpResponseGetterMock);

                var criteriaList = new List<SingleWhereCriteriaHolder>();
                var sewdvClass = m_schema.GetClass("SpatialEntityWithDetailsView");
                criteriaList.Add(new SingleWhereCriteriaHolder()
                {
                    Operator = RelationalOperator.IN, Property = sewdvClass["Classification"], Value = "Imagery,Terrain"
                });

                IEnumerable<USGSRequestBundle> resultList;
                AggregateException ex = Assert.Throws<AggregateException>(() => resultList = dataFetcher.GetNonFormattedUSGSResults(criteriaList));
                Assert.IsTrue(ex.InnerExceptions.Count >= 1, "There should have been at least 1 exception returned, but " + ex.InnerExceptions.Count + " were returned instead.");
                Assert.IsTrue(ex.InnerExceptions.First() is EnvironmentalException, "The exceptions should have been of type EnvironmentalException.");

                }
            }

        [Test]
        public void GetNonFormattedUSGSResultsErrorTest2 ()
            {
            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));
            string tnmApiDatasetsJsonString;
            using ( StreamReader tnmApiDatasetsJsonStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("TnmApiDatasets.txt")) )
                {
                tnmApiDatasetsJsonString = tnmApiDatasetsJsonStreamReader.ReadToEnd();
                }
            using ( m_mock.Record() )
                {
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Once().Return(tnmApiDatasetsJsonString);
                var ex = new AggregateException(new List<Exception>() { new TaskCanceledException() });
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Times(6).Throw(ex);
                }
            using ( m_mock.Playback() )
                {
                var query = new ECQuery();
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");
                var dataFetcher = new USGSDataFetcher(query, httpResponseGetterMock);

                var criteriaList = new List<SingleWhereCriteriaHolder>();
                var sewdvClass = m_schema.GetClass("SpatialEntityWithDetailsView");
                criteriaList.Add(new SingleWhereCriteriaHolder()
                {
                    Operator = RelationalOperator.IN, Property = sewdvClass["Classification"], Value = "Imagery,Terrain"
                });

                IEnumerable<USGSRequestBundle> resultList;
                AggregateException ex = Assert.Throws<AggregateException>(() => resultList = dataFetcher.GetNonFormattedUSGSResults(criteriaList));
                Assert.IsTrue(ex.InnerExceptions.Count >= 1, "There should have been 1 exception returned, but " + ex.InnerExceptions.Count + " were returned instead.");
                Assert.IsTrue(ex.InnerExceptions.First() is EnvironmentalException, "The exceptions should have been of type EnvironmentalException.");

                }
            }
        }
    }
