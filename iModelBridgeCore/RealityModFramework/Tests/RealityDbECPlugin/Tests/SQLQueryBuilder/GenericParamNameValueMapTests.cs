using System;
using System.Data;
using IndexECPlugin.Source;
using NUnit.Framework;

namespace IndexECPlugin.Tests.Tests
    {

    [TestFixture]
    class GenericParamNameValueMapTests
        {
        private GenericParamNameValueMap genericParamNameValueMap;

        [SetUp]
        public void SetUp ()
            {
            genericParamNameValueMap = new GenericParamNameValueMap();
            }

        [Test]
        public void AddDbTypeTest ()
            {
            genericParamNameValueMap.AddParamNameValue("param1", true, DbType.Boolean);
            genericParamNameValueMap.AddParamNameValue("param2", 5, DbType.Int32);
            genericParamNameValueMap.AddParamNameValue("param3", "object", DbType.String);

            Assert.That(genericParamNameValueMap["param1"].Equals(new Tuple<object, DbType>(true, DbType.Boolean)));
            Assert.That(genericParamNameValueMap["param2"].Equals(new Tuple<object, DbType>(5, DbType.Int32)));
            Assert.That(genericParamNameValueMap["param3"].Equals(new Tuple<object, DbType>("object", DbType.String)));
            }

        [Test]
        public void AddECTypeTest ()
            {
            genericParamNameValueMap.AddParamNameValue("param1", true, Bentley.ECObjects.ECObjects.BooleanType);
            genericParamNameValueMap.AddParamNameValue("param2", 5, Bentley.ECObjects.ECObjects.IntegerType);
            genericParamNameValueMap.AddParamNameValue("param3", "object", Bentley.ECObjects.ECObjects.StringType);

            Assert.That(genericParamNameValueMap["param1"].Equals(new Tuple<object, DbType>(true, DbType.Boolean)));
            Assert.That(genericParamNameValueMap["param2"].Equals(new Tuple<object, DbType>(5, DbType.Int32)));
            Assert.That(genericParamNameValueMap["param3"].Equals(new Tuple<object, DbType>("object", DbType.String)));
            }
        }
    }
