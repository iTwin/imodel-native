//! Script that is executed by one of the unit tests in DgnScriptContext_Test.cpp
module DgnScriptTests {
    function logMessage(msg: string): void {
        Bentley.Dgn.Logging.Message('TestRunner', Bentley.Dgn.LoggingSeverity.Info, msg);
    }

    logMessage('TSG_GeometryB A');
    var checker = new DgnScriptChecker.Checker();

    function t_pointVectorOps3d (origin: Bentley.Dgn.DPoint3d,
        vectorX: Bentley.Dgn.DVector3d,
        vectorY: Bentley.Dgn.DVector3d,
        vectorZ: Bentley.Dgn.DVector3d
        ): void
    {
        var transform = Bentley.Dgn.Transform.CreateOriginAndVectors(origin, vectorX, vectorY, vectorZ);
        var localA = new Bentley.Dgn.DPoint3d(5, 3, 7);
        var pointA1 = transform.MultiplyPoint(localA);
        var pointA2 = transform.MultiplyXYZ(localA.X, localA.Y, localA.Z);

        checker.IsNearDPoint3d(pointA1, pointA2);
        var pointA3 = origin.Plus3Scaled (vectorX, localA.X, vectorY, localA.Y, vectorZ, localA.Z);
        checker.IsNearDPoint3d(pointA1, pointA3);
        var pointA4 = origin.PlusScaled (vectorX, localA.X).PlusScaled (vectorY, localA.Y).PlusScaled (vectorZ, localA.Z);
        checker.IsNearDPoint3d(pointA1, pointA4);
        var pointA4 = origin.PlusScaled(vectorX, localA.X).Plus2Scaled(vectorY, localA.Y, vectorZ, localA.Z);
        checker.IsNearDPoint3d(pointA1, pointA4);

        var pointB1 = origin.Plus(vectorX);
        var vectorX1 = origin.VectorTo(pointB1);
        checker.IsNearDVector3d(vectorX, vectorX1);
        var pointB2 = origin.PlusScaled (vectorX, 1.0);
        var pointB3 = pointB2.Minus(vectorX);
        checker.IsNearDPoint3d(origin, pointB3);
        checker.IsNearDPoint3d(pointB1, pointB2);

        var a = 0.75;
        var pointC1 = origin.Interpolate(a, pointB1);
        var pointC2 = origin.PlusScaled(vectorX, a);
        checker.IsNearDPoint3d(pointC1, pointC2);
    }


    function t_vectorVectorOps2d(origin: Bentley.Dgn.DVector2d,
        vectorX: Bentley.Dgn.DVector2d,
        vectorY: Bentley.Dgn.DVector2d,
        vectorZ: Bentley.Dgn.DVector2d
        ): void
    {
    var localA = new Bentley.Dgn.DVector3d (2,5,7);
        var vectorA3 = origin.Plus3Scaled(vectorX, localA.X, vectorY, localA.Y, vectorZ, localA.Z);
        var vectorA4 = origin.PlusScaled(vectorX, localA.X).PlusScaled(vectorY, localA.Y).PlusScaled(vectorZ, localA.Z);
        checker.IsNearDVector2d(vectorA3, vectorA4);
        var vectorA5 = origin.PlusScaled(vectorX, localA.X).Plus2Scaled(vectorY, localA.Y, vectorZ, localA.Z);
        checker.IsNearDVector2d(vectorA3, vectorA5);

        var vectorB1 = origin.Plus(vectorX);
        var vectorX1 = origin.VectorTo(vectorB1);
        checker.IsNearDVector2d(vectorX, vectorX1);
        var vectorB2 = origin.PlusScaled(vectorX, 1.0);
        var vectorB3 = vectorB2.Minus(vectorX);
        checker.IsNearDVector2d(origin, vectorB3);
        checker.IsNearDVector2d(vectorB1, vectorB2);

        var a = 0.75;
        var vectorC1 = origin.Interpolate(a, vectorB1);
        var vectorC2 = origin.PlusScaled(vectorX, a);
        checker.IsNearDVector2d(vectorC1, vectorC2);
    }


    function t_pointVectorOps2d(origin: Bentley.Dgn.DPoint2d,
        vectorX: Bentley.Dgn.DVector2d,
        vectorY: Bentley.Dgn.DVector2d,
        vectorZ: Bentley.Dgn.DVector2d
        ): void
    {
        var localA = new Bentley.Dgn.DVector3d(2, 5, 7);
        var vectorA3 = origin.Plus3Scaled(vectorX, localA.X, vectorY, localA.Y, vectorZ, localA.Z);
        var vectorA4 = origin.PlusScaled(vectorX, localA.X).PlusScaled(vectorY, localA.Y).PlusScaled(vectorZ, localA.Z);
        checker.IsNearDPoint2d(vectorA3, vectorA4);
        var vectorA5 = origin.PlusScaled(vectorX, localA.X).Plus2Scaled(vectorY, localA.Y, vectorZ, localA.Z);
        checker.IsNearDPoint2d(vectorA3, vectorA5);

        var vectorB1 = origin.Plus(vectorX);
        var vectorX1 = origin.VectorTo(vectorB1);
        checker.IsNearDVector2d (vectorX, vectorX1);
        var vectorB2 = origin.PlusScaled(vectorX, 1.0);
        var vectorB3 = vectorB2.Minus(vectorX);
        checker.IsNearDPoint2d(origin, vectorB3);
        checker.IsNearDPoint2d(vectorB1, vectorB2);

        var a = 0.75;
        var vectorC1 = origin.Interpolate(a, vectorB1);
        var vectorC2 = origin.PlusScaled(vectorX, a);
        checker.IsNearDPoint2d(vectorC1, vectorC2);
    }



    function t_pointDistances(pointA: Bentley.Dgn.DPoint3d,
        vectorX: Bentley.Dgn.DVector3d)
    {
        var pointB = pointA.Plus (vectorX);
        checker.NearDouble (vectorX.Magnitude (), pointA.Distance (pointB), true);
        checker.NearDouble(vectorX.MagnitudeSquared(), pointA.DistanceSquared(pointB), true);
        for (var f = 0.0; f < 3.0; f += 0.2344987)
        {
            var pointB1 = pointA.Interpolate(f, pointB);
            var pointB2 = pointA.PlusScaled (vectorX, f);
            checker.IsNearDPoint3d(pointB1, pointB2);
            checker.NearDouble(pointA.Distance(pointB1), f * vectorX.Magnitude(), true);

        }
        }
    //debugger ;
    logMessage('TSG_GeometryB t_pointVectorOps3d');
    t_pointVectorOps3d(new Bentley.Dgn.DPoint3d(1, 2, 3),
        new Bentley.Dgn.DVector3d(0.2, 0.4, 0.8),
        new Bentley.Dgn.DVector3d(-0.4, 0.2, 0),
        new Bentley.Dgn.DVector3d(0, 0.4, -0.8));

    logMessage('TSG_GeometryB t_vectorVectorOps2d');

    t_vectorVectorOps2d(new Bentley.Dgn.DVector2d (1, 2),
        new Bentley.Dgn.DVector2d(0.2, 0.4),
        new Bentley.Dgn.DVector2d(-0.4, 0.2),
        new Bentley.Dgn.DVector2d(0, 0.4));
    logMessage('TSG_GeometryB t_pointVectorOps2d');

    t_pointVectorOps2d(new Bentley.Dgn.DPoint2d(1, 2),
        new Bentley.Dgn.DVector2d(0.2, 0.4),
        new Bentley.Dgn.DVector2d(-0.4, 0.2),
        new Bentley.Dgn.DVector2d(0, 0.4));

    t_pointDistances(new Bentley.Dgn.DPoint3d(1, 2, 3),
        new Bentley.Dgn.DVector3d(0.2, 0.4, 0.8));
    

    logMessage('TSG_GeometryB B Exit');
}

