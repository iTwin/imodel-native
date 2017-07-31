using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence.Operations;
using Bentley.EC.Persistence.Query;
using Bentley.ECSystem.Configuration;
using Bentley.Exceptions;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Tests.Common;
using Newtonsoft.Json.Linq;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests.Helpers
    {
    [TestFixture]
    class RDSDataFetcherTests
        {
        IHttpResponseGetter m_responseGetterMock;
        IBUDDIWrapper m_buddiWrapperMock;

        MockRepository m_mock;

        string buddiGetURLString = "http://testString.com/";
        string buddiGetURLCodeString = "http://testStringCode.com/";
        string rdsResponseString = "{instances:[{\"instanceId\": \"7d192080-e680-4581-8032-05b4992f05b7\",\"eTag\": \"etag\"}]}";

        [SetUp]
        public void Setup()
            {
            m_mock = new MockRepository();
            m_responseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));
            m_buddiWrapperMock = (IBUDDIWrapper) m_mock.StrictMock(typeof(IBUDDIWrapper));
            }

        [Test]
        public void CtorTestBuddyRegionCode()
            {
            ConfigurationRoot.SetAppSetting("RECPBuddiRegionCode", "103");

            //The constructor should call GetUrl with the regional code
            using(m_mock.Record())
                {
                Expect.Call(m_buddiWrapperMock.GetUrl(Arg<string>.Is.Anything, Arg<int>.Matches<int>(c => c == 103))).Repeat.Once().Return(buddiGetURLCodeString);
                }
            using(m_mock.Playback())
                {
                var rdsDataFetcher = new RDSDataFetcher(m_responseGetterMock, m_buddiWrapperMock);
                Assert.IsTrue(rdsDataFetcher.RdsUrlBase.Contains(buddiGetURLCodeString));
                }
            }

        [Test]
        public void CtorTestBuddyFailure ()
            {
            string RECPRdsUrlBase = "HelloTesting";
            ConfigurationRoot.SetAppSetting("RECPBuddiRegionCode", "103");
            ConfigurationRoot.SetAppSetting("RECPRdsUrlBase", RECPRdsUrlBase);

            //The constructor should call GetUrl with the regional code
            using ( m_mock.Record() )
                {
                Expect.Call(m_buddiWrapperMock.GetUrl(Arg<string>.Is.Anything, Arg<int>.Matches<int>(c => c == 103))).Repeat.Once().Throw(new Exception("Test Exception"));
                }
            using ( m_mock.Playback() )
                {
                var rdsDataFetcher = new RDSDataFetcher(m_responseGetterMock, m_buddiWrapperMock);
                Assert.IsTrue(rdsDataFetcher.RdsUrlBase.Contains(RECPRdsUrlBase));
                }
            }

        [Test]
        public void CtorTestBuddyFailureBadSettings ()
            {
            ConfigurationRoot.SetAppSetting("RECPBuddiRegionCode", "103");
            ConfigurationRoot.SetAppSetting("RECPRdsUrlBase", "");

            //The constructor should call GetUrl with the regional code
            using ( m_mock.Record() )
                {
                Expect.Call(m_buddiWrapperMock.GetUrl(Arg<string>.Is.Anything, Arg<int>.Matches<int>(c => c == 103))).Repeat.Once().Throw(new Exception("Test Exception"));
                }
            using ( m_mock.Playback() )
                {
                Assert.Throws<OperationFailedException>(() => new RDSDataFetcher(m_responseGetterMock, m_buddiWrapperMock));
                }
            }

        [Test]
        public void CtorTestNoRegionCode()
            {
            ConfigurationRoot.SetAppSetting("RECPBuddiRegionCode", "");

            using ( m_mock.Record() )
                {
                Expect.Call(m_buddiWrapperMock.GetUrl(Arg<string>.Is.Anything)).Repeat.Once().Return(buddiGetURLString);
                }
            using ( m_mock.Playback() )
                {
                var rdsDataFetcher = new RDSDataFetcher(m_responseGetterMock, m_buddiWrapperMock);
                Assert.IsTrue(rdsDataFetcher.RdsUrlBase.Contains(buddiGetURLString));
                }
            }

        [Test]
        public void GetSingleDataTest()
            {
            ConfigurationRoot.SetAppSetting("RECPBuddiRegionCode", "103");
            string entityId = "1234abc";
            using(m_mock.Record())
                {
                Expect.Call(m_responseGetterMock.GetHttpResponse(Arg<string>.Matches<string>(str => str.Contains(buddiGetURLCodeString) && str.Contains(entityId) && str.Contains("RealityData")))).Repeat.Once().Return(rdsResponseString);
                Expect.Call(m_buddiWrapperMock.GetUrl(Arg<string>.Is.Anything, Arg<int>.Matches<int>(c => c == 103))).Repeat.Once().Return(buddiGetURLCodeString);
                }
            using ( m_mock.Playback() )
                {
                var rdsDataFetcher = new RDSDataFetcher(m_responseGetterMock, m_buddiWrapperMock);
                JObject inst = rdsDataFetcher.GetSingleData(entityId);

                Assert.IsTrue(inst.Value<string>("instanceId") == "7d192080-e680-4581-8032-05b4992f05b7", "RDSDataFetcher did not return the first instance");
                }
            }
        [Test]
        public void GetReadAccessAzureTokenTest()
            {
            ConfigurationRoot.SetAppSetting("RECPBuddiRegionCode", "103");
            string entityId = "1234abc";
            string fileAccessKey = "{ \"instances\": [{\"instanceId\": \".AzureBlobSasUrl.Read.S3MX.RealityData.73d09423-28c3-4fdb-ab4a-03a47a5b04f8\",\"schemaName\": \"FileAccess\",\"className\": \"FileAccessKey\",\"properties\": {\"Url\": \"https://realityblobdeveussa01.blob.core.windows.net/73d09423-28c3-4fdb-ab4a-03a47a5b04f8?sv=2015-04-05&sr=c&sig=xWMA6R%2Bxaq84C6ZdfpiPqrJkjB%2FmLtSlkyTDX4251xs%3D&se=2017-07-28T20%3A55%3A08Z&sp=rl\",\"Type\": \"AzureBlobSasUrl\",\"Permissions\": \"Read\",\"RequiresConfirmation\": false},\"eTag\": \"etag\"}]}";
            using ( m_mock.Record() )
                {
                Expect.Call(m_responseGetterMock.GetHttpResponse(Arg<string>.Matches<string>(str => str.Contains(buddiGetURLCodeString) && str.Contains(entityId) && str.Contains("RealityData")))).Repeat.Once().Return(fileAccessKey);
                Expect.Call(m_buddiWrapperMock.GetUrl(Arg<string>.Is.Anything, Arg<int>.Matches<int>(c => c == 103))).Repeat.Once().Return(buddiGetURLCodeString);
                }
            using ( m_mock.Playback() )
                {
                var rdsDataFetcher = new RDSDataFetcher(m_responseGetterMock, m_buddiWrapperMock);
                JObject inst = rdsDataFetcher.GetReadAccessAzureToken(entityId);

                Assert.IsTrue(inst.Value<string>("instanceId") == ".AzureBlobSasUrl.Read.S3MX.RealityData.73d09423-28c3-4fdb-ab4a-03a47a5b04f8", "RDSDataFetcher did not return the first instance");
                }
            }

        [Test]
        public void GetDataBySpatialQueryTest()
            {
            ConfigurationRoot.SetAppSetting("RECPBuddiRegionCode", "103");
            string polygon = "{\"points\":[[0,0],[0,1],[1,1],[1,0],[0,0]],\"coordinate_system\":\"4326\"}";
            using(m_mock.Record())
                {
                Expect.Call(m_responseGetterMock.GetHttpResponse(Arg<string>.Matches<string>(u => u.Contains("PUBLIC") && u.Contains("ENTERPRISE") && u.Contains(polygon)))).Repeat.Once().Return(rdsResponseString);
                Expect.Call(m_buddiWrapperMock.GetUrl(Arg<string>.Is.Anything, Arg<int>.Matches<int>(c => c == 103))).Repeat.Once().Return(buddiGetURLCodeString);
                }
            using ( m_mock.Playback() )
                {
                
                List<SingleWhereCriteriaHolder> criteriaList = new List<SingleWhereCriteriaHolder>();
                var rdsDataFetcher = new RDSDataFetcher(m_responseGetterMock, m_buddiWrapperMock);
                var array = rdsDataFetcher.GetDataBySpatialQuery(polygon,criteriaList);

                Assert.AreEqual(1, array.Count, "There should have been only one instance.");
                Assert.IsTrue(array.First.Value<string>("instanceId") == "7d192080-e680-4581-8032-05b4992f05b7", "RDSDataFetcher did not return the first instance");
                }
            }

        [Test]
        public void GetDataBySpatialQueryTestClassificationCriteriaEq ()
            {
            ConfigurationRoot.SetAppSetting("RECPBuddiRegionCode", "103");
            string polygon = "{\"points\":[[0,0],[0,1],[1,1],[1,0],[0,0]],\"coordinate_system\":\"4326\"}";
            using ( m_mock.Record() )
                {
                Expect.Call(m_responseGetterMock.GetHttpResponse(Arg<string>.Matches<string>(u => u.Contains("PUBLIC") && u.Contains("ENTERPRISE") && u.Contains(polygon) && u.Contains("Classification+eq+'testing'")))).Repeat.Once().Return(rdsResponseString);
                Expect.Call(m_buddiWrapperMock.GetUrl(Arg<string>.Is.Anything, Arg<int>.Matches<int>(c => c == 103))).Repeat.Once().Return(buddiGetURLCodeString);
                }
            using ( m_mock.Playback() )
                {

                List<SingleWhereCriteriaHolder> criteriaList = new List<SingleWhereCriteriaHolder>();
                SingleWhereCriteriaHolder criteria = new SingleWhereCriteriaHolder(){Property = SetupHelpers.PrepareSchema()["SpatialEntityWithDetailsView"]["Classification"], Operator = RelationalOperator.EQ, Value = "'testing'"};
                criteriaList.Add(criteria);
                var rdsDataFetcher = new RDSDataFetcher(m_responseGetterMock, m_buddiWrapperMock);
                var array = rdsDataFetcher.GetDataBySpatialQuery(polygon, criteriaList);

                Assert.AreEqual(1, array.Count, "There should have been only one instance.");
                Assert.IsTrue(array.First.Value<string>("instanceId") == "7d192080-e680-4581-8032-05b4992f05b7", "RDSDataFetcher did not return the first instance");
                }
            }

        [Test]
        public void GetDataBySpatialQueryTestClassificationCriteriaLt ()
            {
            ConfigurationRoot.SetAppSetting("RECPBuddiRegionCode", "103");
            string polygon = "{\"points\":[[0,0],[0,1],[1,1],[1,0],[0,0]],\"coordinate_system\":\"4326\"}";
            using ( m_mock.Record() )
                {
                Expect.Call(m_responseGetterMock.GetHttpResponse(Arg<string>.Matches<string>(u => u.Contains("PUBLIC") && u.Contains("ENTERPRISE") && u.Contains(polygon) && u.Contains("Classification+lt+'testing'")))).Repeat.Once().Return(rdsResponseString);
                Expect.Call(m_buddiWrapperMock.GetUrl(Arg<string>.Is.Anything, Arg<int>.Matches<int>(c => c == 103))).Repeat.Once().Return(buddiGetURLCodeString);
                }
            using ( m_mock.Playback() )
                {

                List<SingleWhereCriteriaHolder> criteriaList = new List<SingleWhereCriteriaHolder>();
                SingleWhereCriteriaHolder criteria = new SingleWhereCriteriaHolder()
                {
                    Property = SetupHelpers.PrepareSchema()["SpatialEntityWithDetailsView"]["Classification"], Operator = RelationalOperator.LT, Value = "'testing'"
                };
                criteriaList.Add(criteria);
                var rdsDataFetcher = new RDSDataFetcher(m_responseGetterMock, m_buddiWrapperMock);
                var array = rdsDataFetcher.GetDataBySpatialQuery(polygon, criteriaList);

                Assert.AreEqual(1, array.Count, "There should have been only one instance.");
                Assert.IsTrue(array.First.Value<string>("instanceId") == "7d192080-e680-4581-8032-05b4992f05b7", "RDSDataFetcher did not return the first instance");
                }
            }

        [Test]
        public void GetDataBySpatialQueryTestClassificationCriteriaLe ()
            {
            ConfigurationRoot.SetAppSetting("RECPBuddiRegionCode", "103");
            string polygon = "{\"points\":[[0,0],[0,1],[1,1],[1,0],[0,0]],\"coordinate_system\":\"4326\"}";
            using ( m_mock.Record() )
                {
                Expect.Call(m_responseGetterMock.GetHttpResponse(Arg<string>.Matches<string>(u => u.Contains("PUBLIC") && u.Contains("ENTERPRISE") && u.Contains(polygon) && u.Contains("Classification+le+'testing'")))).Repeat.Once().Return(rdsResponseString);
                Expect.Call(m_buddiWrapperMock.GetUrl(Arg<string>.Is.Anything, Arg<int>.Matches<int>(c => c == 103))).Repeat.Once().Return(buddiGetURLCodeString);
                }
            using ( m_mock.Playback() )
                {

                List<SingleWhereCriteriaHolder> criteriaList = new List<SingleWhereCriteriaHolder>();
                SingleWhereCriteriaHolder criteria = new SingleWhereCriteriaHolder()
                {
                    Property = SetupHelpers.PrepareSchema()["SpatialEntityWithDetailsView"]["Classification"], Operator = RelationalOperator.LTEQ, Value = "'testing'"
                };
                criteriaList.Add(criteria);
                var rdsDataFetcher = new RDSDataFetcher(m_responseGetterMock, m_buddiWrapperMock);
                var array = rdsDataFetcher.GetDataBySpatialQuery(polygon, criteriaList);

                Assert.AreEqual(1, array.Count, "There should have been only one instance.");
                Assert.IsTrue(array.First.Value<string>("instanceId") == "7d192080-e680-4581-8032-05b4992f05b7", "RDSDataFetcher did not return the first instance");
                }
            }

        [Test]
        public void GetDataBySpatialQueryTestClassificationCriteriaGt ()
            {
            ConfigurationRoot.SetAppSetting("RECPBuddiRegionCode", "103");
            string polygon = "{\"points\":[[0,0],[0,1],[1,1],[1,0],[0,0]],\"coordinate_system\":\"4326\"}";
            using ( m_mock.Record() )
                {
                Expect.Call(m_responseGetterMock.GetHttpResponse(Arg<string>.Matches<string>(u => u.Contains("PUBLIC") && u.Contains("ENTERPRISE") && u.Contains(polygon) && u.Contains("Classification+gt+'testing'")))).Repeat.Once().Return(rdsResponseString);
                Expect.Call(m_buddiWrapperMock.GetUrl(Arg<string>.Is.Anything, Arg<int>.Matches<int>(c => c == 103))).Repeat.Once().Return(buddiGetURLCodeString);
                }
            using ( m_mock.Playback() )
                {

                List<SingleWhereCriteriaHolder> criteriaList = new List<SingleWhereCriteriaHolder>();
                SingleWhereCriteriaHolder criteria = new SingleWhereCriteriaHolder()
                {
                    Property = SetupHelpers.PrepareSchema()["SpatialEntityWithDetailsView"]["Classification"], Operator = RelationalOperator.GT, Value = "'testing'"
                };
                criteriaList.Add(criteria);
                var rdsDataFetcher = new RDSDataFetcher(m_responseGetterMock, m_buddiWrapperMock);
                var array = rdsDataFetcher.GetDataBySpatialQuery(polygon, criteriaList);

                Assert.AreEqual(1, array.Count, "There should have been only one instance.");
                Assert.IsTrue(array.First.Value<string>("instanceId") == "7d192080-e680-4581-8032-05b4992f05b7", "RDSDataFetcher did not return the first instance");
                }
            }

        [Test]
        public void GetDataBySpatialQueryTestClassificationCriteriaGe ()
            {
            ConfigurationRoot.SetAppSetting("RECPBuddiRegionCode", "103");
            string polygon = "{\"points\":[[0,0],[0,1],[1,1],[1,0],[0,0]],\"coordinate_system\":\"4326\"}";
            using ( m_mock.Record() )
                {
                Expect.Call(m_responseGetterMock.GetHttpResponse(Arg<string>.Matches<string>(u => u.Contains("PUBLIC") && u.Contains("ENTERPRISE") && u.Contains(polygon) && u.Contains("Classification+ge+'testing'")))).Repeat.Once().Return(rdsResponseString);
                Expect.Call(m_buddiWrapperMock.GetUrl(Arg<string>.Is.Anything, Arg<int>.Matches<int>(c => c == 103))).Repeat.Once().Return(buddiGetURLCodeString);
                }
            using ( m_mock.Playback() )
                {

                List<SingleWhereCriteriaHolder> criteriaList = new List<SingleWhereCriteriaHolder>();
                SingleWhereCriteriaHolder criteria = new SingleWhereCriteriaHolder()
                {
                    Property = SetupHelpers.PrepareSchema()["SpatialEntityWithDetailsView"]["Classification"], Operator = RelationalOperator.GTEQ, Value = "'testing'"
                };
                criteriaList.Add(criteria);
                var rdsDataFetcher = new RDSDataFetcher(m_responseGetterMock, m_buddiWrapperMock);
                var array = rdsDataFetcher.GetDataBySpatialQuery(polygon, criteriaList);

                Assert.AreEqual(1, array.Count, "There should have been only one instance.");
                Assert.IsTrue(array.First.Value<string>("instanceId") == "7d192080-e680-4581-8032-05b4992f05b7", "RDSDataFetcher did not return the first instance");
                }
            }

        [Test]
        public void GetDataBySpatialQueryTestClassificationCriteriaNe ()
            {
            ConfigurationRoot.SetAppSetting("RECPBuddiRegionCode", "103");
            string polygon = "{\"points\":[[0,0],[0,1],[1,1],[1,0],[0,0]],\"coordinate_system\":\"4326\"}";
            using ( m_mock.Record() )
                {
                Expect.Call(m_responseGetterMock.GetHttpResponse(Arg<string>.Matches<string>(u => u.Contains("PUBLIC") && u.Contains("ENTERPRISE") && u.Contains(polygon) && u.Contains("Classification+ne+'testing'")))).Repeat.Once().Return(rdsResponseString);
                Expect.Call(m_buddiWrapperMock.GetUrl(Arg<string>.Is.Anything, Arg<int>.Matches<int>(c => c == 103))).Repeat.Once().Return(buddiGetURLCodeString);
                }
            using ( m_mock.Playback() )
                {

                List<SingleWhereCriteriaHolder> criteriaList = new List<SingleWhereCriteriaHolder>();
                SingleWhereCriteriaHolder criteria = new SingleWhereCriteriaHolder()
                {
                    Property = SetupHelpers.PrepareSchema()["SpatialEntityWithDetailsView"]["Classification"], Operator = RelationalOperator.NE, Value = "'testing'"
                };
                criteriaList.Add(criteria);
                var rdsDataFetcher = new RDSDataFetcher(m_responseGetterMock, m_buddiWrapperMock);
                var array = rdsDataFetcher.GetDataBySpatialQuery(polygon, criteriaList);

                Assert.AreEqual(1, array.Count, "There should have been only one instance.");
                Assert.IsTrue(array.First.Value<string>("instanceId") == "7d192080-e680-4581-8032-05b4992f05b7", "RDSDataFetcher did not return the first instance");
                }
            }

        [Test]
        public void GetDataBySpatialQueryTestClassificationCriteriaNotImplemented ()
            {
            ConfigurationRoot.SetAppSetting("RECPBuddiRegionCode", "103");
            string polygon = "{\"points\":[[0,0],[0,1],[1,1],[1,0],[0,0]],\"coordinate_system\":\"4326\"}";
            using ( m_mock.Record() )
                {
                Expect.Call(m_buddiWrapperMock.GetUrl(Arg<string>.Is.Anything, Arg<int>.Matches<int>(c => c == 103))).Repeat.Once().Return(buddiGetURLCodeString);
                }
            using ( m_mock.Playback() )
                {

                List<SingleWhereCriteriaHolder> criteriaList = new List<SingleWhereCriteriaHolder>();
                SingleWhereCriteriaHolder criteria = new SingleWhereCriteriaHolder()
                {
                    Property = SetupHelpers.PrepareSchema()["SpatialEntityWithDetailsView"]["Classification"], Operator = RelationalOperator.CONTAINSWHOLEWORD, Value = "'testing'"
                };
                criteriaList.Add(criteria);
                var rdsDataFetcher = new RDSDataFetcher(m_responseGetterMock, m_buddiWrapperMock);
                Assert.Throws<UserFriendlyException>( () => rdsDataFetcher.GetDataBySpatialQuery(polygon, criteriaList));

                }
            }
        }
    }
