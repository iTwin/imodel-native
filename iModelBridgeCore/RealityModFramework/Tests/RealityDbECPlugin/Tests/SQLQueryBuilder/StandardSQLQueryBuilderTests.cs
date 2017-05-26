using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using IndexECPlugin.Source;
using NUnit.Framework;

namespace IndexECPlugin.Tests.Tests
    {

    [TestFixture]
    class StandardSQLQueryBuilderTests
        {
        private StandardSQLQueryBuilder standardSQLQueryBuilder;

        [SetUp]
        public void SetUp ()
            {
            standardSQLQueryBuilder = new StandardSQLQueryBuilder();
            }

        [Test]
        [Ignore]
        public void BuildQueryTest ()
            {
            Assert.Fail();
            //todo
            }
        }
    }
