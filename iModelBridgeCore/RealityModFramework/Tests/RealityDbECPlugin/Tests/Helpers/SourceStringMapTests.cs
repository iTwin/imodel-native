using System;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using NUnit.Framework;

namespace IndexECPlugin.Tests.Tests.Helpers
    {
    [TestFixture]
    class SourceStringMapTests
        {

        [Test]
        public void IsValidIdTest ()
            {
            Assert.That(SourceStringMap.IsValidId(DataSource.Index, "10"), Is.True);
            Assert.That(SourceStringMap.IsValidId(DataSource.Index, "0"), Is.True);
            Assert.That(SourceStringMap.IsValidId(DataSource.Index, "-420"), Is.True);
            Assert.That(SourceStringMap.IsValidId(DataSource.Index, "360.1337"), Is.False);
            Assert.That(SourceStringMap.IsValidId(DataSource.Index, "not an int"), Is.False);
            Assert.That(SourceStringMap.IsValidId(DataSource.Index, ""), Is.False);

            Assert.That(SourceStringMap.IsValidId(DataSource.USGS, new String('a', IndexConstants.USGSIdLenght)), Is.True);
            Assert.That(SourceStringMap.IsValidId(DataSource.USGS, new String('a', IndexConstants.USGSIdLenght + 1)), Is.False);

            Assert.That(SourceStringMap.IsValidId(DataSource.RDS, new Guid().ToString()), Is.True);
            Assert.That(SourceStringMap.IsValidId(DataSource.RDS, "not a guid"), Is.False);

            Assert.That(SourceStringMap.IsValidId(DataSource.All, "foo"), Is.True);
            }

        [Test]
        public void IsValidIdExceptionTest ()
            {
            Assert.That(() => SourceStringMap.IsValidId(DataSource.All + 1, "foo"), Throws.TypeOf<NotImplementedException>());
            }

        [TestCase(DataSource.Index, "index")]
        [TestCase(DataSource.USGS, "usgsapi")]
        [TestCase(DataSource.RDS, "rds")]
        [TestCase(DataSource.All, "all")]
        public void SourceToStringTest (DataSource source, string sourceStr)
            {
            Assert.That(SourceStringMap.SourceToString(source), Is.EqualTo(sourceStr));
            }

        [Test]
        public void SourceToStringExceptionTest ()
            {
            Assert.That(() => SourceStringMap.SourceToString(DataSource.All + 1), Throws.TypeOf<NotImplementedException>());
            }

        [TestCase("index", DataSource.Index)]
        [TestCase("UsGsApI", DataSource.USGS)]
        [TestCase("RDS", DataSource.RDS)]
        [TestCase("all", DataSource.All)]
        public void StringToSourceTest (string sourceStr, DataSource dataSource)
            {
            Assert.That(SourceStringMap.StringToSource(sourceStr), Is.EqualTo(dataSource));
            }

        [Test]
        public void StringToSourceExceptionTest ()
            {
            Assert.That(() => SourceStringMap.StringToSource("this should throw an exception!"), Throws.TypeOf<NotImplementedException>());
            }

        [Test]
        public void GetAllSourceStringsTest ()
            {
            // TODO: change this test if the corresponding method is not hardcoded anymore
            Assert.That(SourceStringMap.GetAllSourceStrings(), Is.EqualTo("\"index\", \"usgsAPI\", \"rds\", \"usgsee\" and \"all\""));
            }
        }
    }
