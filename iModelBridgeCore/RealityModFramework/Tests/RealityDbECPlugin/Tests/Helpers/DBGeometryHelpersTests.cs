using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Bentley.Exceptions;
using IndexECPlugin.Source.Helpers;
using NUnit.Framework;

namespace IndexECPlugin.Tests.Tests.Helpers
    {
    [TestFixture]
    class DBGeometryHelpersTests
        {
        [SetUp]
        public void SetUp ()
            {

            }

        [TearDown]
        public void TearDown ()
            {

            }

        [Test]
        public void CreateWktPolygonStringTest ()
            {
            List<Double[]> enumerationOfPoints = new List<double[]>();
            enumerationOfPoints.Add(new double[2] { 0, 0 });
            enumerationOfPoints.Add(new double[2] { 0, 1 });
            enumerationOfPoints.Add(new double[2] { 1, 1 });
            enumerationOfPoints.Add(new double[2] { 1, 0 });
            Console.WriteLine(DbGeometryHelpers.CreateWktPolygonString(enumerationOfPoints));
            }

        [Test]
        public void CreateWktPolygonStringInvalidPolygonTest ()
            {
            List<Double[]> emptyEnumeration = new List<double[]>();
            Assert.That(() => DbGeometryHelpers.CreateWktPolygonString(emptyEnumeration), Throws.TypeOf<UserFriendlyException>());

            List<Double[]> onePointEnumeration = new List<double[]>();
            onePointEnumeration.Add(new double[2] { 1, 2 });
            Assert.That(() => DbGeometryHelpers.CreateWktPolygonString(onePointEnumeration), Throws.TypeOf<UserFriendlyException>());

            List<Double[]> twoPointsEnumeration = new List<double[]>();
            twoPointsEnumeration.Add(new double[2] { 1, 2 });
            twoPointsEnumeration.Add(new double[2] { 3, 4 });
            Assert.That(() => DbGeometryHelpers.CreateWktPolygonString(twoPointsEnumeration), Throws.TypeOf<UserFriendlyException>());
            }
        }
    }
