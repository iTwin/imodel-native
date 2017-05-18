using System;
using System.Collections.Generic;
using IndexECPlugin.Source;
using NUnit.Framework;

namespace IndexECPlugin.Tests.Tests
    {
    [TestFixture]
    class PolygonModelTests
        {
        [Test]
        public void ConstructorShouldSucceedTest ()
            {
            List<Double[]> validPointEnumeration = new List<double[]> { new[] { -50.24, -10.5 }, new[] { 28.17, -5.6 }, new[] { -13.42, 10.77 } };
            PolygonModel model = new PolygonModel(validPointEnumeration);
            Assert.That(model.Points, Is.EqualTo(validPointEnumeration));

            List<Double[]> validPointEnumeration2 = new List<double[]> { new double[] { 0, 0 }, new double[] { 0, 1 }, new double[] { 1, 1 }, new double[] { 1, 0 } };
            PolygonModel model2 = new PolygonModel(validPointEnumeration2);
            Assert.That(model2.Points, Is.EqualTo(validPointEnumeration2));
            }

        [Test]
        public void ConstructorShouldThrowExceptionTest ()
            {
            List<Double[]> emptyEnumeration = new List<double[]>();
            Assert.That(() => new PolygonModel(emptyEnumeration), Throws.TypeOf<ArgumentException>());

            List<Double[]> onePointEnumeration = new List<double[]> { new double[] { 1, 2 } };
            Assert.That(() => new PolygonModel(onePointEnumeration), Throws.TypeOf<ArgumentException>());

            List<Double[]> twoPointsEnumeration = new List<double[]> { new double[] { 1, 2 }, new double[] { 3, 4 } };
            Assert.That(() => new PolygonModel(twoPointsEnumeration), Throws.TypeOf<ArgumentException>());

            List<Double[]> threePointsClosedEnumeration = new List<double[]> { new double[] { 1, 2 }, new double[] { 3, 4 }, new double[] { 1, 2 } };
            Assert.That(() => new PolygonModel(threePointsClosedEnumeration), Throws.TypeOf<ArgumentException>());
            }
        }
    }
