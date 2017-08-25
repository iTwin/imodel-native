using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.EC.Persistence.Query;
using Bentley.ECObjects.Json;
using Bentley.ECSystem.Configuration;
using IndexECPlugin.Source.Helpers;
using IndexECPlugin.Source.QueryProviders;
using IndexECPlugin.Tests.Common;
using NUnit.Framework;

namespace IndexECPlugin.Tests.Tests.Helpers
    {
    //[TestFixture]
    //class UsgsEEApiQueryProviderTests
    //    {
    //    [Test]
    //    public void DeleteThisTestAfter()
    //        {
    //        ConfigurationRoot.SetAppSetting("RECPUserNameEE", "christian.tye-gingras");
    //        ConfigurationRoot.SetAppSetting("RECPPasswordEE", "BobbyWatson47");

    //        var schema = SetupHelpers.PrepareSchema();
    //        ECQuery query = new ECQuery(schema.GetClass("Server"));
    //        //query.ExtendedDataValueSetter.Add("polygon", "{ \"points\" : [[-90.1111928012935,41.32950370684],[-89.9874229095346,41.32950370684],[-89.9874229095346,41.4227313251356],[-90.1111928012935,41.4227313251356],[-90.1111928012935,41.32950370684]], \"coordinate_system\" : \"4326\" }");
    //        query.WhereClause = new WhereCriteria(new ECInstanceIdExpression("USGSEE__3411__3841618_L5C2"));

    //        var fetcher = new UsgsEEDataFetcher(query, new GenericHttpResponseGetter("bob"));

    //        var provider = new UsgsEEApiQueryProvider(query, null, null, schema);

    //        var instanceList = provider.CreateInstanceList();

    //        string json = instanceList.First().ToJson();
    //        }
    //    }
    }
