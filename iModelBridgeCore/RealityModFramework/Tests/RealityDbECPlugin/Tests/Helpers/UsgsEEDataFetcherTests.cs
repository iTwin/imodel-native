using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence.Operations;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Schema;
using Bentley.ECSystem.Configuration;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;
using Rhino.Mocks;

namespace IndexECPlugin.Tests.Tests.Helpers
    {
    [TestFixture]
    class UsgsEEDataFetcherTests
        {

        MockRepository m_mock;
        IECSchema m_schema;

        string m_userName = "hello.world";
        string m_password = "password";

        [SetUp]
        public void setup()
            {
            m_mock = new MockRepository();
            m_schema = SetupHelpers.PrepareSchema();

            ConfigurationRoot.SetAppSetting("RECPUserNameEE", m_userName);
            ConfigurationRoot.SetAppSetting("RECPPasswordEE", m_password);
            }

        [Test]
        public void SpatialQueryTest()
            {

            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));

            using (m_mock.Record())
                {
                string jsonLoginResponseString;
                using ( StreamReader jsonResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerLoginResponse.txt")) )
                    {
                    jsonLoginResponseString = jsonResponseStreamReader.ReadToEnd();
                    }

                string jsonQueryResponseString;
                using ( StreamReader jsonQueryResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerSpatialQueryResponse.txt")) )
                    {
                    jsonQueryResponseString = jsonQueryResponseStreamReader.ReadToEnd();
                    }
                string jsonLogoutResponseString;
                using ( StreamReader jsonLogoutResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerLogoutResponse.txt")) )
                    {
                    jsonLogoutResponseString = jsonLogoutResponseStreamReader.ReadToEnd(); 
                    }
                string jsonLoginRequest = "{\"username\":\"" + m_userName + "\",\"password\":\"" + m_password + "\",\"authType\":\"EROS\",\"catalogId\":\"EE\"}";
                string jsonLogoutRequest = "{\"apiKey\":\"b819d10e84564320921cd9269ebdd337\"}";

                string urlLogin = "https://earthexplorer.usgs.gov/inventory/json/v/1.4.0/" + "login?jsonRequest=" + Uri.EscapeUriString(jsonLoginRequest);
                string urlLogout = "https://earthexplorer.usgs.gov/inventory/json/v/1.4.0/" + "logout?jsonRequest=" + Uri.EscapeUriString(jsonLogoutRequest);

                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Equal(urlLogin))).Repeat.Once().Return(jsonLoginResponseString);
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.NotEqual(urlLogin))).Repeat.Once().Return(jsonQueryResponseString);
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Equal(urlLogout))).Repeat.Once().Return(jsonLogoutResponseString);
                }
            using (m_mock.Playback())
                {
                ECQuery query = new ECQuery();
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");

                UsgsEEDataFetcher fetcher = new UsgsEEDataFetcher(query, httpResponseGetterMock);
                var list = fetcher.SpatialQuery();

                Assert.AreEqual(1, list.Count, "There is not exactly one result to the spatial request.");

                foreach ( var result in list )
                    {
                    Assert.IsTrue(fetcher.DatasetList.Any(d => d.DatasetId == result.Dataset.DatasetId), "The dataset returned should be present in the dataset list");
                    }
                }
            }

        [Test]
        public void SpatialQueryErrorTest()
            {

            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));

            using (m_mock.Record())
                {
                string jsonLoginResponseString;
                using ( StreamReader jsonResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerLoginResponse.txt")) )
                    {
                    jsonLoginResponseString = jsonResponseStreamReader.ReadToEnd();
                    }

                string jsonQueryResponseString;
                using ( StreamReader jsonQueryResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerSpatialQueryErrorResponse.txt")) )
                    {
                    jsonQueryResponseString = jsonQueryResponseStreamReader.ReadToEnd();
                    }
                string jsonLogoutResponseString;
                using ( StreamReader jsonLogoutResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerLogoutResponse.txt")) )
                    {
                    jsonLogoutResponseString = jsonLogoutResponseStreamReader.ReadToEnd(); 
                    }
                string jsonLoginRequest = "{\"username\":\"" + m_userName + "\",\"password\":\"" + m_password + "\",\"authType\":\"EROS\",\"catalogId\":\"EE\"}";
                string jsonLogoutRequest = "{\"apiKey\":\"b819d10e84564320921cd9269ebdd337\"}";

                string urlLogin = "https://earthexplorer.usgs.gov/inventory/json/v/1.4.0/" + "login?jsonRequest=" + Uri.EscapeUriString(jsonLoginRequest);
                string urlLogout = "https://earthexplorer.usgs.gov/inventory/json/v/1.4.0/" + "logout?jsonRequest=" + Uri.EscapeUriString(jsonLogoutRequest);

                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Equal(urlLogin))).Repeat.Once().Return(jsonLoginResponseString);
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.NotEqual(urlLogin))).Repeat.Once().Return(jsonQueryResponseString);
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Equal(urlLogout))).Repeat.Once().Return(jsonLogoutResponseString);
                }
            using (m_mock.Playback())
                {
                ECQuery query = new ECQuery();
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");

                UsgsEEDataFetcher fetcher = new UsgsEEDataFetcher(query, httpResponseGetterMock);
                var list = fetcher.SpatialQuery();

                Assert.AreEqual(0, list.Count, "There is a result to the spatial request, there should have been none due to an error.");
                }
            }

        [Test]
        public void SpatialQueryTimeoutTest ()
            {

            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));

            using ( m_mock.Record() )
                {
                string jsonLoginResponseString;
                using ( StreamReader jsonResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerLoginResponse.txt")) )
                    {
                    jsonLoginResponseString = jsonResponseStreamReader.ReadToEnd();
                    }

                string jsonLogoutResponseString;
                using ( StreamReader jsonLogoutResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerLogoutResponse.txt")) )
                    {
                    jsonLogoutResponseString = jsonLogoutResponseStreamReader.ReadToEnd();
                    }
                string jsonLoginRequest = "{\"username\":\"" + m_userName + "\",\"password\":\"" + m_password + "\",\"authType\":\"EROS\",\"catalogId\":\"EE\"}";
                string jsonLogoutRequest = "{\"apiKey\":\"b819d10e84564320921cd9269ebdd337\"}";

                string urlLogin = "https://earthexplorer.usgs.gov/inventory/json/v/1.4.0/" + "login?jsonRequest=" + Uri.EscapeUriString(jsonLoginRequest);
                string urlLogout = "https://earthexplorer.usgs.gov/inventory/json/v/1.4.0/" + "logout?jsonRequest=" + Uri.EscapeUriString(jsonLogoutRequest);

                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Equal(urlLogin))).Repeat.Once().Return(jsonLoginResponseString);
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.NotEqual(urlLogin))).Repeat.Once().Throw(new TaskCanceledException());
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Equal(urlLogout))).Repeat.Once().Return(jsonLogoutResponseString);
                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery();
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");

                UsgsEEDataFetcher fetcher = new UsgsEEDataFetcher(query, httpResponseGetterMock);

                Assert.Throws<Bentley.Exceptions.EnvironmentalException>( () => fetcher.SpatialQuery(), "An environmental exception should have been thrown when a timeout happens.");
                }
            }

        [Test]
        public void SpatialQueryTimeoutTest2 ()
            {

            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));

            using ( m_mock.Record() )
                {
                string jsonLoginResponseString;
                using ( StreamReader jsonResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerLoginResponse.txt")) )
                    {
                    jsonLoginResponseString = jsonResponseStreamReader.ReadToEnd();
                    }

                string jsonLogoutResponseString;
                using ( StreamReader jsonLogoutResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerLogoutResponse.txt")) )
                    {
                    jsonLogoutResponseString = jsonLogoutResponseStreamReader.ReadToEnd();
                    }
                string jsonLoginRequest = "{\"username\":\"" + m_userName + "\",\"password\":\"" + m_password + "\",\"authType\":\"EROS\",\"catalogId\":\"EE\"}";
                string jsonLogoutRequest = "{\"apiKey\":\"b819d10e84564320921cd9269ebdd337\"}";

                string urlLogin = "https://earthexplorer.usgs.gov/inventory/json/v/1.4.0/" + "login?jsonRequest=" + Uri.EscapeUriString(jsonLoginRequest);
                string urlLogout = "https://earthexplorer.usgs.gov/inventory/json/v/1.4.0/" + "logout?jsonRequest=" + Uri.EscapeUriString(jsonLogoutRequest);

                List<Exception> innerExceptions = new List<Exception>() { new TaskCanceledException() };
                AggregateException ex = new AggregateException(innerExceptions);

                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Equal(urlLogin))).Repeat.Once().Return(jsonLoginResponseString);
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.NotEqual(urlLogin))).Repeat.Once().Throw(ex);
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Equal(urlLogout))).Repeat.Once().Return(jsonLogoutResponseString);
                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery();
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");

                UsgsEEDataFetcher fetcher = new UsgsEEDataFetcher(query, httpResponseGetterMock);

                Assert.Throws<Bentley.Exceptions.EnvironmentalException>(() => fetcher.SpatialQuery(), "An environmental exception should have been thrown when a timeout happens.");
                }
            }

        [Test]
        public void SpatialQueryOtherErrorTest ()
            {

            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));

            using ( m_mock.Record() )
                {
                string jsonLoginResponseString;
                using ( StreamReader jsonResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerLoginResponse.txt")) )
                    {
                    jsonLoginResponseString = jsonResponseStreamReader.ReadToEnd();
                    }

                string jsonLogoutResponseString;
                using ( StreamReader jsonLogoutResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerLogoutResponse.txt")) )
                    {
                    jsonLogoutResponseString = jsonLogoutResponseStreamReader.ReadToEnd();
                    }
                string jsonLoginRequest = "{\"username\":\"" + m_userName + "\",\"password\":\"" + m_password + "\",\"authType\":\"EROS\",\"catalogId\":\"EE\"}";
                string jsonLogoutRequest = "{\"apiKey\":\"b819d10e84564320921cd9269ebdd337\"}";

                string urlLogin = "https://earthexplorer.usgs.gov/inventory/json/v/1.4.0/" + "login?jsonRequest=" + Uri.EscapeUriString(jsonLoginRequest);
                string urlLogout = "https://earthexplorer.usgs.gov/inventory/json/v/1.4.0/" + "logout?jsonRequest=" + Uri.EscapeUriString(jsonLogoutRequest);

                List<Exception> innerExceptions = new List<Exception>() { new AccessViolationException() };
                AggregateException ex = new AggregateException(innerExceptions);

                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Equal(urlLogin))).Repeat.Once().Return(jsonLoginResponseString);
                //This exception should not occur normally. We test that it is rethrown
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.NotEqual(urlLogin))).Repeat.Once().Throw(ex);
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Equal(urlLogout))).Repeat.Once().Return(jsonLogoutResponseString);
                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery();
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");

                UsgsEEDataFetcher fetcher = new UsgsEEDataFetcher(query, httpResponseGetterMock);

                var resEx = Assert.Throws<AggregateException>(() => fetcher.SpatialQuery(), "This type of exception should be rethrown.");
                Assert.IsTrue(resEx.InnerExceptions.Any(e => e is AccessViolationException), "The exception was not rethrown as is.");
                }
            }

        [Test]
        public void LoginSettingsErrorTest ()
            {
            ConfigurationRoot.SetAppSetting("RECPUserNameEE", "");
            ConfigurationRoot.SetAppSetting("RECPPasswordEE", "");

            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));

            ECQuery query = new ECQuery();
            query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");

            UsgsEEDataFetcher fetcher = new UsgsEEDataFetcher(query, httpResponseGetterMock);

            Assert.Throws<OperationFailedException>(() => fetcher.SpatialQuery(), "An OperationFailedException should be thrown when settings are incorrect.");

            }

        [Test]
        public void LoginErrorCodeTest ()
            {

            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));

            using ( m_mock.Record() )
                {
                string jsonLoginResponseString;
                using ( StreamReader jsonResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerLoginErrorResponse.txt")) )
                    {
                    jsonLoginResponseString = jsonResponseStreamReader.ReadToEnd();
                    }
                string jsonLoginRequest = "{\"username\":\"" + m_userName + "\",\"password\":\"" + m_password + "\",\"authType\":\"EROS\",\"catalogId\":\"EE\"}";

                string urlLogin = "https://earthexplorer.usgs.gov/inventory/json/v/1.4.0/" + "login?jsonRequest=" + Uri.EscapeUriString(jsonLoginRequest);

                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Equal(urlLogin))).Repeat.Once().Return(jsonLoginResponseString);
                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery();
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");

                UsgsEEDataFetcher fetcher = new UsgsEEDataFetcher(query, httpResponseGetterMock);

                Assert.Throws<OperationFailedException>(() => fetcher.SpatialQuery());
                }
            }

        [Test]
        public void SpatialQueryErrorLogout ()
            {

            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));

            using ( m_mock.Record() )
                {
                string jsonLoginResponseString;
                using ( StreamReader jsonResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerLoginResponse.txt")) )
                    {
                    jsonLoginResponseString = jsonResponseStreamReader.ReadToEnd();
                    }

                string jsonQueryResponseString;
                using ( StreamReader jsonQueryResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerSpatialQueryResponse.txt")) )
                    {
                    jsonQueryResponseString = jsonQueryResponseStreamReader.ReadToEnd();
                    }
                string jsonLoginRequest = "{\"username\":\"" + m_userName + "\",\"password\":\"" + m_password + "\",\"authType\":\"EROS\",\"catalogId\":\"EE\"}";
                string jsonLogoutRequest = "{\"apiKey\":\"b819d10e84564320921cd9269ebdd337\"}";

                string urlLogin = "https://earthexplorer.usgs.gov/inventory/json/v/1.4.0/" + "login?jsonRequest=" + Uri.EscapeUriString(jsonLoginRequest);
                string urlLogout = "https://earthexplorer.usgs.gov/inventory/json/v/1.4.0/" + "logout?jsonRequest=" + Uri.EscapeUriString(jsonLogoutRequest);

                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Equal(urlLogin))).Repeat.Once().Return(jsonLoginResponseString);
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.NotEqual(urlLogin))).Repeat.Once().Return(jsonQueryResponseString);
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Equal(urlLogout))).Repeat.Once().Throw(new AccessViolationException());
                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery();
                query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");

                UsgsEEDataFetcher fetcher = new UsgsEEDataFetcher(query, httpResponseGetterMock);

                Assert.Throws<AccessViolationException>(() => fetcher.SpatialQuery(), "The error in the logout was not thrown as is.");
                }
            }

        [Test]
        public void GetXmlDocTest()
            {
            IHttpResponseGetter httpResponseGetterMock = (IHttpResponseGetter) m_mock.StrictMock(typeof(IHttpResponseGetter));

            using (m_mock.Record())
                {

                string xmlResponseString;
                using ( StreamReader xmlResponseStreamReader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("EarthExplorerXmlDocResponse.txt")) )
                    {
                    xmlResponseString = xmlResponseStreamReader.ReadToEnd();
                    }
                Expect.Call(httpResponseGetterMock.GetHttpResponse(Arg<string>.Is.Anything)).Repeat.Once().Return(xmlResponseString);
                }
            using ( m_mock.Playback() )
                {
                ECQuery query = new ECQuery();
                UsgsEEDataFetcher fetcher = new UsgsEEDataFetcher(query, httpResponseGetterMock);
                var result = fetcher.GetXmlDocForInstance("1234", "5678");
                }
            }
        }
    }
