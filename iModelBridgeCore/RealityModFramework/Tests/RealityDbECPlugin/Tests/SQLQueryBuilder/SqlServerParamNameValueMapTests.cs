using System;
using System.Collections.Generic;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using IndexECPlugin.Source;
using NUnit.Framework;

namespace IndexECPlugin.Tests.Tests
    {

    [TestFixture]
    class SqlServerParamNameValueMapTests
        {
        private SqlServerParamNameValueMap sqlServerParamNameValueMap;

        [SetUp]
        public void SetUp ()
            {
            sqlServerParamNameValueMap = new SqlServerParamNameValueMap();
            }

        [Test]
        public void AddSqlDbTypeTest ()
            {
            sqlServerParamNameValueMap.AddParamNameValue("param1", true, SqlDbType.Bit);
            sqlServerParamNameValueMap.AddParamNameValue("param2", 5, SqlDbType.Int);
            sqlServerParamNameValueMap.AddParamNameValue("param3", "object", SqlDbType.NVarChar);

            Assert.That(sqlServerParamNameValueMap["param1"].Equals(new Tuple<object, SqlDbType>(true, SqlDbType.Bit)));
            Assert.That(sqlServerParamNameValueMap["param2"].Equals(new Tuple<object, SqlDbType>(5, SqlDbType.Int)));
            Assert.That(sqlServerParamNameValueMap["param3"].Equals(new Tuple<object, SqlDbType>("object", SqlDbType.NVarChar)));
            }

        [Test]
        public void AddECTypetest ()
            {
            sqlServerParamNameValueMap.AddParamNameValue("param1", true, Bentley.ECObjects.ECObjects.BooleanType);
            sqlServerParamNameValueMap.AddParamNameValue("param2", 5, Bentley.ECObjects.ECObjects.IntegerType);
            sqlServerParamNameValueMap.AddParamNameValue("param3", "object", Bentley.ECObjects.ECObjects.StringType);

            Assert.That(sqlServerParamNameValueMap["param1"].Equals(new Tuple<object, SqlDbType>(true, SqlDbType.Bit)));
            Assert.That(sqlServerParamNameValueMap["param2"].Equals(new Tuple<object, SqlDbType>(5, SqlDbType.Int)));
            Assert.That(sqlServerParamNameValueMap["param3"].Equals(new Tuple<object, SqlDbType>("object", SqlDbType.NVarChar)));
            }
        }
    }
