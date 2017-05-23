using System;
using System.Collections.Generic;
using Bentley.Exceptions;
using IndexECPlugin.Source;
using IndexECPlugin.Source.Helpers;
using NUnit.Framework;

namespace IndexECPlugin.Tests.Tests.Helpers
    {
    [TestFixture]
    class DBGeometryHelpersTests
        {
        //Valid points enumeration
        private List<double[]> threePointsUnclosedEnumeration;
        private List<double[]> fourPointsUnclosedEnumeration;
        private List<double[]> fourPointsClosedEnumeration;
        private List<double[]> tenPointsClosedEnumeration;
        private List<double[]> thirtyonePointsUnclosedEnumeration;

        private string threePointsUnclosedEnumerationStr;
        private string fourPointsUnclosedEnumerationStr;
        private string tenPointsClosedEnumerationStr;
        private string thirtyonePointsUnclosedEnumerationStr;


        //Invalid points enumeration
        private List<Double[]> emptyEnumeration;
        private List<Double[]> onePointEnumeration;
        private List<Double[]> twoPointsEnumeration;
        private List<Double[]> threePointsClosedEnumeration;

        private PolygonModel model1;
        private string jsonModel1;

        [TestFixtureSetUp]
        public void FixtureSetUp ()
            {
            threePointsUnclosedEnumeration = new List<double[]> { new[] { -50.24, -10.5 }, new[] { 28.17, -5.6 }, new[] { -13.42, 10.77 } };
            threePointsUnclosedEnumerationStr = "POLYGON((-50.24 -10.5, 28.17 -5.6, -13.42 10.77, -50.24 -10.5))";

            fourPointsUnclosedEnumeration = new List<double[]> { new double[] { 0, 0 }, new double[] { 0, 1 }, new double[] { 1, 1 }, new double[] { 1, 0 } };
            fourPointsUnclosedEnumerationStr = "POLYGON((0 0, 0 1, 1 1, 1 0, 0 0))";

            fourPointsClosedEnumeration = new List<double[]> { new double[] { 0, 0 }, new double[] { 0, 1 }, new double[] { 1, 1 }, new double[] { 1, 0 }, new double[] { 0, 0 } };

            tenPointsClosedEnumeration = new List<double[]> {new [] {-40.2, -12.7}, new [] {-51.1, -16.1}, new [] {-59.2, -25.7 }, new [] {-61.7, -35.9},
                                                             new [] {-57.2, -49.1}, new [] {-42.7, -57.7}, new [] {-29.2, -55.5 }, new [] {-19.7, -43.1},
                                                             new [] {-19.0, -30.5}, new [] {-25.1, -19.1}, new [] {-40.2, -12.7}};
            tenPointsClosedEnumerationStr = "POLYGON((-40.2 -12.7, -51.1 -16.1, -59.2 -25.7, -61.7 -35.9, -57.2 -49.1, -42.7 -57.7, -29.2 -55.5, -19.7 -43.1," +
                                          " -19 -30.5, -25.1 -19.1, -40.2 -12.7))";

            thirtyonePointsUnclosedEnumeration = new List<double[]>();
            for ( int i = 0; i < 31; i++ )
                {
                thirtyonePointsUnclosedEnumeration.Add(new double[] { Math.Abs(15 - i), i });
                }
            thirtyonePointsUnclosedEnumerationStr = "POLYGON((15 0, 14 1, 13 2, 12 3, 11 4, 10 5, 9 6, 8 7, 7 8, 6 9, 5 10, 4 11, 3 12, 2 13, 1 14, 0 15, 1 16, " +
                                               "2 17, 3 18, 4 19, 5 20, 6 21, 7 22, 8 23, 9 24, 10 25, 11 26, 12 27, 13 28, 14 29, 15 30, 15 0))";

            emptyEnumeration = new List<double[]>();
            onePointEnumeration = new List<double[]> { new double[] { 1, 2 } };
            twoPointsEnumeration = new List<double[]> { new double[] { 1, 2 }, new double[] { 3, 4 } };
            threePointsClosedEnumeration = new List<double[]> { new double[] { 1, 2 }, new double[] { 3, 4 }, new double[] { 1, 2 } };

            model1 = new PolygonModel(threePointsUnclosedEnumeration)
            {
                coordinate_system = 1234
            };
            jsonModel1 = "{ \"points\" : [[-50.24,-10.5],[28.17,-5.6],[-13.42,10.77]], \"coordinate_system\" : \"1234\" }";
            }

        [Test]
        public void CreateWktPolygonStringTest ()
            {
            Assert.That(DbGeometryHelpers.CreateWktPolygonString(threePointsUnclosedEnumeration), Is.EqualTo(threePointsUnclosedEnumerationStr));
            Assert.That(DbGeometryHelpers.CreateWktPolygonString(fourPointsUnclosedEnumeration), Is.EqualTo(fourPointsUnclosedEnumerationStr));
            Assert.That(DbGeometryHelpers.CreateWktPolygonString(tenPointsClosedEnumeration), Is.EqualTo(tenPointsClosedEnumerationStr));
            Assert.That(DbGeometryHelpers.CreateWktPolygonString(thirtyonePointsUnclosedEnumeration), Is.EqualTo(thirtyonePointsUnclosedEnumerationStr));
            }

        [Test]
        public void CreateWktPolygonStringInvalidPolygonTest ()
            {
            Assert.That(() => DbGeometryHelpers.CreateWktPolygonString(emptyEnumeration), Throws.TypeOf<UserFriendlyException>());
            Assert.That(() => DbGeometryHelpers.CreateWktPolygonString(onePointEnumeration), Throws.TypeOf<UserFriendlyException>());
            Assert.That(() => DbGeometryHelpers.CreateWktPolygonString(twoPointsEnumeration), Throws.TypeOf<UserFriendlyException>());
            Assert.That(() => DbGeometryHelpers.CreateWktPolygonString(threePointsClosedEnumeration), Throws.TypeOf<UserFriendlyException>());
            }

        [Test]
        public void CreateSTGeomFromTextStringFromJsonTest ()
            {
            Assert.That(DbGeometryHelpers.CreateSTGeomFromTextStringFromJson(jsonModel1), Is.EqualTo("geometry::STGeomFromText('" + threePointsUnclosedEnumerationStr + "'," + model1.coordinate_system + ")"));
            }

        [Test]
        public void CreatePolygonModelFromJsonTest ()
            {
            PolygonModel polygonModel = DbGeometryHelpers.CreatePolygonModelFromJson(jsonModel1);
            Assert.That(polygonModel.Points, Is.EqualTo(model1.Points));
            Assert.That(polygonModel.coordinate_system, Is.EqualTo(model1.coordinate_system));
            }

        [Test]
        public void CreatePolygonModelFromJsonExceptionTest ()
            {
            Assert.That(() => DbGeometryHelpers.CreatePolygonModelFromJson("\"Invalid\" : \"polygon format\", \"Should pass\" : \"Not\"}"),
                Throws.TypeOf<UserFriendlyException>());
            }

        [Test]
        public void CreateFootprintStringTest ()
            {
            Assert.That(DbGeometryHelpers.CreateFootprintString("0", "-5", "100", "42.360", 12345),
                Is.EqualTo(@"{""points"":[[0,-5],[100,-5],[100,42.360],[0,42.360],[0,-5]],""coordinate_system"":""12345""}"));
            }

        [Test]
        public void CreateEsriPolygonFromPolyModelTest ()
            {
            Assert.That(DbGeometryHelpers.CreateEsriPolygonFromPolyModel(model1), Is.EqualTo("{\"rings\":[[[-50.24,-10.5],[28.17,-5.6],[-13.42,10.77],[-50.24,-10.5]]],\"spatialReference\":{\"wkid\":1234}}"));

            PolygonModel model2 = new PolygonModel(fourPointsClosedEnumeration)
                {
                    coordinate_system = 0
                };
            Assert.That(DbGeometryHelpers.CreateEsriPolygonFromPolyModel(model2), Is.EqualTo("{\"rings\":[[[0,0],[0,1],[1,1],[1,0],[0,0]]],\"spatialReference\":{\"wkid\":0}}"));
            }
        [Test]
        public void GetCoordinateArrayFromWKTPolygonTest ()
            {
            string[][] coordinateArray = DbGeometryHelpers.GetCoordinateArrayFromWKTPolygon(threePointsUnclosedEnumerationStr);
            Assert.That(coordinateArray, Is.EqualTo(new [] { new [] { "-50.24", "-10.5" }, new [] { "28.17", "-5.6" }, new [] { "-13.42", "10.77" }, new [] { "-50.24", "-10.5" } }));
            }

        [Test]
        public void GetCoordinateArrayFromWKTPolygonExceptionsTest ()
            {
            Assert.That(() => DbGeometryHelpers.GetCoordinateArrayFromWKTPolygon("Doesn't start with POLYGON"),
                Throws.TypeOf<ArgumentException>().With.Message.EqualTo("Only Polygon WKT are valid"));

            Assert.That(() => DbGeometryHelpers.GetCoordinateArrayFromWKTPolygon("POLYGON"),
                Throws.TypeOf<ArgumentException>().With.Message.EqualTo("Invalid WKT polygon"));

            Assert.That(() => DbGeometryHelpers.GetCoordinateArrayFromWKTPolygon("POLYGON(())"),
                Throws.TypeOf<ArgumentException>().With.Message.EqualTo("Invalid WKT polygon"));

            Assert.That(() => DbGeometryHelpers.GetCoordinateArrayFromWKTPolygon("POLYGON((-50.24 -10.5, 28.17 -5.6, -13.42 10.77))"),
                    Throws.TypeOf<ArgumentException>().With.Message.EqualTo("Invalid WKT polygon"));

            Assert.That(() => DbGeometryHelpers.GetCoordinateArrayFromWKTPolygon("POLYGON((-50.24 -10.5, 28.17_-5.6, -13.42 10.77, -50.24 -10.5))"),
                    Throws.TypeOf<ArgumentException>().With.Message.EqualTo("Invalid WKT polygon"));

            Assert.That(() => DbGeometryHelpers.GetCoordinateArrayFromWKTPolygon("POLYGON((-50.24 -10.5, 28.17 -5.6, -13.42 10.77 error, -50.24 -10.5))"),
                    Throws.TypeOf<ArgumentException>().With.Message.EqualTo("Invalid WKT polygon"));
            }

        [Test]
        public void ExtractOuterShellPointsFromWKTPolygonTest ()
            {
            Assert.That(DbGeometryHelpers.ExtractOuterShellPointsFromWKTPolygon(threePointsUnclosedEnumerationStr), Is.EqualTo("[[-50.24,-10.5],[28.17,-5.6],[-13.42,10.77],[-50.24,-10.5]]"));
            }

        [Test]
        public void ExtractBboxFromWKTPolygonTest ()
            {
            BBox bbox = DbGeometryHelpers.ExtractBboxFromWKTPolygon(threePointsUnclosedEnumerationStr);
            Assert.That(bbox.minX, Is.EqualTo(-50.24));
            Assert.That(bbox.minY, Is.EqualTo(-10.5));
            Assert.That(bbox.maxX, Is.EqualTo(28.17));
            Assert.That(bbox.maxY, Is.EqualTo(10.77));
            }

        [Test]
        public void ExtractBboxStringFromWKTPolygonTest ()
            {
            Assert.That(DbGeometryHelpers.ExtractBboxStringFromWKTPolygon(threePointsUnclosedEnumerationStr), Is.EqualTo("-50.24,-10.5,28.17,10.77"));
            }

        [TestCase(1800, 90, 0)]
        [TestCase(1800, -90, 0)]
        [TestCase(0, 0, 0)]
        [TestCase(1800, 0, 55566)]
        [TestCase(100, -45, 2182.839)]
        [TestCase(2345.67, 30, 62709.621)]
        [TestCase(3600, 60, 55566)]
        [TestCase(42.42, -72.87, 385.703)]
        [TestCase(-555.55, 0, -17149.829)]
        public void ConvertArcsecLatToMeterTest (double arcsec, double lat, double result)
            {
            Assert.That(DbGeometryHelpers.ConvertArcsecLatToMeter(arcsec, lat), Is.EqualTo(result).Within(0.01));
            }

        [Test]
        public void ConvertArcsecLatToMeterExceptionTest ()
            {
            Assert.That(() => DbGeometryHelpers.ConvertArcsecLatToMeter(30, -180), Throws.TypeOf<UserFriendlyException>());
            Assert.That(() => DbGeometryHelpers.ConvertArcsecLatToMeter(30, -90.01), Throws.TypeOf<UserFriendlyException>());
            Assert.That(() => DbGeometryHelpers.ConvertArcsecLatToMeter(30, 180), Throws.TypeOf<UserFriendlyException>());
            Assert.That(() => DbGeometryHelpers.ConvertArcsecLatToMeter(30, 90.01), Throws.TypeOf<UserFriendlyException>());
            }
        }
    }
