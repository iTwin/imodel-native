//! Script that is executed by one of the unit tests in DgnScriptContext_Test.cpp
module DgnScriptTests {
    function logMessage(msg: string): void {
        BentleyApi.Dgn.Logging.Message('TestRunner', BentleyApi.Dgn.LoggingSeverity.Info, msg);
    }

    logMessage('TSG_GeometryB A');
    var checker = new DgnScriptChecker.Checker();

    function t_pointVectorOps3d (origin: BentleyApi.Dgn.JsDPoint3d,
        vectorX: BentleyApi.Dgn.JsDVector3d,
        vectorY: BentleyApi.Dgn.JsDVector3d,
        vectorZ: BentleyApi.Dgn.JsDVector3d
        ): void
    {
        var transform = BentleyApi.Dgn.JsTransform.CreateOriginAndVectors(origin, vectorX, vectorY, vectorZ);
        var localA = new BentleyApi.Dgn.JsDPoint3d(5, 3, 7);
        var pointA1 = transform.MultiplyPoint(localA);
        var pointA2 = transform.MultiplyXYZ(localA.X, localA.Y, localA.Z);

        checker.IsNearJsDPoint3d(pointA1, pointA2);
        var pointA3 = origin.Plus3Scaled (vectorX, localA.X, vectorY, localA.Y, vectorZ, localA.Z);
        checker.IsNearJsDPoint3d(pointA1, pointA3);
        var pointA4 = origin.PlusScaled (vectorX, localA.X).PlusScaled (vectorY, localA.Y).PlusScaled (vectorZ, localA.Z);
        checker.IsNearJsDPoint3d(pointA1, pointA4);
        var pointA4 = origin.PlusScaled(vectorX, localA.X).Plus2Scaled(vectorY, localA.Y, vectorZ, localA.Z);
        checker.IsNearJsDPoint3d(pointA1, pointA4);

        var pointB1 = origin.Plus(vectorX);
        var vectorX1 = origin.VectorTo(pointB1);
        checker.IsNearJsDVector3d(vectorX, vectorX1);
        var pointB2 = origin.PlusScaled (vectorX, 1.0);
        var pointB3 = pointB2.Minus(vectorX);
        checker.IsNearJsDPoint3d(origin, pointB3);
        checker.IsNearJsDPoint3d(pointB1, pointB2);

        var a = 0.75;
        var pointC1 = origin.Interpolate(a, pointB1);
        var pointC2 = origin.PlusScaled(vectorX, a);
        checker.IsNearJsDPoint3d(pointC1, pointC2);
    }


    function t_vectorVectorOps2d(origin: BentleyApi.Dgn.JsDVector2d,
        vectorX: BentleyApi.Dgn.JsDVector2d,
        vectorY: BentleyApi.Dgn.JsDVector2d,
        vectorZ: BentleyApi.Dgn.JsDVector2d
        ): void
    {
    var localA = new BentleyApi.Dgn.JsDVector3d (2,5,7);
        var vectorA3 = origin.Plus3Scaled(vectorX, localA.X, vectorY, localA.Y, vectorZ, localA.Z);
        var vectorA4 = origin.PlusScaled(vectorX, localA.X).PlusScaled(vectorY, localA.Y).PlusScaled(vectorZ, localA.Z);
        checker.IsNearJsDVector2d(vectorA3, vectorA4);
        var vectorA5 = origin.PlusScaled(vectorX, localA.X).Plus2Scaled(vectorY, localA.Y, vectorZ, localA.Z);
        checker.IsNearJsDVector2d(vectorA3, vectorA5);

        var vectorB1 = origin.Plus(vectorX);
        var vectorX1 = origin.VectorTo(vectorB1);
        checker.IsNearJsDVector2d(vectorX, vectorX1);
        var vectorB2 = origin.PlusScaled(vectorX, 1.0);
        var vectorB3 = vectorB2.Minus(vectorX);
        checker.IsNearJsDVector2d(origin, vectorB3);
        checker.IsNearJsDVector2d(vectorB1, vectorB2);

        var a = 0.75;
        var vectorC1 = origin.Interpolate(a, vectorB1);
        var vectorC2 = origin.PlusScaled(vectorX, a);
        checker.IsNearJsDVector2d(vectorC1, vectorC2);
    }


    function t_pointVectorOps2d(origin: BentleyApi.Dgn.JsDPoint2d,
        vectorX: BentleyApi.Dgn.JsDVector2d,
        vectorY: BentleyApi.Dgn.JsDVector2d,
        vectorZ: BentleyApi.Dgn.JsDVector2d
        ): void
    {
        var localA = new BentleyApi.Dgn.JsDVector3d(2, 5, 7);
        var vectorA3 = origin.Plus3Scaled(vectorX, localA.X, vectorY, localA.Y, vectorZ, localA.Z);
        var vectorA4 = origin.PlusScaled(vectorX, localA.X).PlusScaled(vectorY, localA.Y).PlusScaled(vectorZ, localA.Z);
        checker.IsNearJsDPoint2d(vectorA3, vectorA4);
        var vectorA5 = origin.PlusScaled(vectorX, localA.X).Plus2Scaled(vectorY, localA.Y, vectorZ, localA.Z);
        checker.IsNearJsDPoint2d(vectorA3, vectorA5);

        var vectorB1 = origin.Plus(vectorX);
        var vectorX1 = origin.VectorTo(vectorB1);
        checker.IsNearJsDVector2d (vectorX, vectorX1);
        var vectorB2 = origin.PlusScaled(vectorX, 1.0);
        var vectorB3 = vectorB2.Minus(vectorX);
        checker.IsNearJsDPoint2d(origin, vectorB3);
        checker.IsNearJsDPoint2d(vectorB1, vectorB2);

        var a = 0.75;
        var vectorC1 = origin.Interpolate(a, vectorB1);
        var vectorC2 = origin.PlusScaled(vectorX, a);
        checker.IsNearJsDPoint2d(vectorC1, vectorC2);
    }



    function t_pointDistances(pointA: BentleyApi.Dgn.JsDPoint3d,
        vectorX: BentleyApi.Dgn.JsDVector3d)
    {
        var pointB = pointA.Plus (vectorX);
        checker.NearDouble (vectorX.Magnitude (), pointA.Distance (pointB), true);
        checker.NearDouble(vectorX.MagnitudeSquared(), pointA.DistanceSquared(pointB), true);
        for (var f = 0.0; f < 3.0; f += 0.2344987)
        {
            var pointB1 = pointA.Interpolate(f, pointB);
            var pointB2 = pointA.PlusScaled (vectorX, f);
            checker.IsNearJsDPoint3d(pointB1, pointB2);
            checker.NearDouble(pointA.Distance(pointB1), f * vectorX.Magnitude(), true);

        }
        }
    //debugger ;
    logMessage('TSG_GeometryB t_pointVectorOps3d');
    t_pointVectorOps3d(new BentleyApi.Dgn.JsDPoint3d(1, 2, 3),
        new BentleyApi.Dgn.JsDVector3d(0.2, 0.4, 0.8),
        new BentleyApi.Dgn.JsDVector3d(-0.4, 0.2, 0),
        new BentleyApi.Dgn.JsDVector3d(0, 0.4, -0.8));

    logMessage('TSG_GeometryB t_vectorVectorOps2d');

    t_vectorVectorOps2d(new BentleyApi.Dgn.JsDVector2d (1, 2),
        new BentleyApi.Dgn.JsDVector2d(0.2, 0.4),
        new BentleyApi.Dgn.JsDVector2d(-0.4, 0.2),
        new BentleyApi.Dgn.JsDVector2d(0, 0.4));
    logMessage('TSG_GeometryB t_pointVectorOps2d');

    t_pointVectorOps2d(new BentleyApi.Dgn.JsDPoint2d(1, 2),
        new BentleyApi.Dgn.JsDVector2d(0.2, 0.4),
        new BentleyApi.Dgn.JsDVector2d(-0.4, 0.2),
        new BentleyApi.Dgn.JsDVector2d(0, 0.4));

    t_pointDistances(new BentleyApi.Dgn.JsDPoint3d(1, 2, 3),
        new BentleyApi.Dgn.JsDVector3d(0.2, 0.4, 0.8));
    

    logMessage('TSG_GeometryB B Exit');
}

