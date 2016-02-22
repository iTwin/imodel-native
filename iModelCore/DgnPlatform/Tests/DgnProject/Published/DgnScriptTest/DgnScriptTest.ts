/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/DgnScriptTest/DgnScriptTest.ts $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// Script that is executed by one of the unit tests in DgnScriptContext_Test.cpp
module DgnScriptTests {

    import be = Bentley.Dgn;

    // *** NB: Keep this consistent with {DgnScriptContext_Test.cpp}TEST_F(DgnScriptTest, RunScripts)
    class Params
    {
        modelName: string;
        categoryName: string;
    };

    var shiftXSize = 5.0;
    var shiftYsize = 2.5;

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   
    //---------------------------------------------------------------------------------------
    function Shift(g: be.Geometry, dx: number, dy: number)
        {
        g.TryTransformInPlace (be.Transform.CreateTranslationXYZ (dx*shiftXSize, dy*shiftYsize, 0.0));
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   
    //---------------------------------------------------------------------------------------
    function ShowPoint (builder : be.GeometryBuilder, point: be.DPoint3d)
        {
        var arc = be.EllipticArc.CreateCircleXY (point, 0.05);
        builder.AppendGeometry (arc);
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   
    //---------------------------------------------------------------------------------------
    function ShowArc (builder: be.GeometryBuilder, arc: be.EllipticArc )
        {
        builder.AppendGeometry (arc);
        ShowPoint (builder, arc.PointAtFraction (0.0));
        var basis = arc.GetBasisPlane ();
        builder.AppendGeometry (new be.LineSegment (basis.Evaluate (0,0), basis.Evaluate(1.4,0)));
        builder.AppendGeometry (new be.LineSegment (basis.Evaluate (0,0), basis.Evaluate(0,0.95)));
        var points = new be.DPoint3dArray ();
            points.Add (basis.Evaluate (1,-1));
            points.Add (basis.Evaluate (1,1));
            points.Add (basis.Evaluate (-1,1));
            points.Add (basis.Evaluate (-1,-1));
            points.Add (basis.Evaluate (1,-1));
        builder.AppendGeometry (new be.LineString (points));
        }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      02/16
    //---------------------------------------------------------------------------------------
    function makeRedConeInBuilder(ele: be.PhysicalElement)
    {
        var builder = new be.GeometryBuilder(ele, new be.DPoint3d(0, 0, 0), new be.YawPitchRollAngles(0, 0, 0));
        var cone = be.DgnCone.CreateCircularCone(
            new be.DPoint3d(0, 0, 0),
            new be.DPoint3d(1, 1, 0),
            0.5, 0.3, true);
        var gparams = builder.GeometryParams;
        gparams.LineColor = new be.ColorDef(255, 0, 0, 0);
        gparams.GeometryClass = be.RenderDgnGeometryClass.Construction;
        builder.AppendRenderGeometryParams(gparams);
        builder.AppendGeometry(cone);
        return builder;
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      02/16
    //---------------------------------------------------------------------------------------
    function makeGreenSphereInBuilder(ele: be.PhysicalElement)
    {
        var builder = new be.GeometryBuilder(ele, new be.DPoint3d(0, 0, 0), new be.YawPitchRollAngles(0, 0, 0));
        var sphere = be.DgnSphere.CreateSphere(new be.DPoint3d(0, 0, 0), 1.0);
        var gparams = builder.GeometryParams;
        gparams.LineColor = new be.ColorDef(0, 255, 0, 0);
        gparams.GeometryClass = be.RenderDgnGeometryClass.Construction;
        builder.AppendRenderGeometryParams(gparams);
        builder.AppendGeometry(sphere);
        return builder;
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      02/16
    //---------------------------------------------------------------------------------------
    function testGeomBuilders(db: be.DgnDb, model: be.DgnModel, catid: be.DgnObjectId): void
    {
        var ele = be.PhysicalElement.Create(model, catid, '');

        /* First test that we can create separate builders and then combine them */
        var b1: be.GeometryBuilder = makeRedConeInBuilder(ele);
        var b2: be.GeometryBuilder = makeGreenSphereInBuilder(ele);

        var builder = new be.GeometryBuilder(ele, new be.DPoint3d(0, 0, 0), new be.YawPitchRollAngles(0, 0, 0));
        builder.AppendCopyOfGeometry(b1, null);
        builder.AppendCopyOfGeometry(b2, new be.Placement3d(new be.DPoint3d(1, 0, 0), new be.YawPitchRollAngles(0, 0, 0)));
        builder.SetGeometryStreamAndPlacement(ele);
        ele.Insert();

        var geomcollection = ele.Geometry;
        var geomcollectionIter = geomcollection.Begin();
        var igeom = 0;
        while (geomcollection.IsValid(geomcollectionIter))
        {
            var geomParams = geomcollection.GetGeometryParams(geomcollectionIter);
            var lineColor = geomParams.LineColor;
         
            var geom = geomcollection.GetGeometry(geomcollectionIter).Geometry;
            if (igeom == 0)
            {
                if (!(geom instanceof be.DgnCone) || lineColor.Red != 255 || lineColor.Blue != 0 || lineColor.Green != 0)
                    be.Script.ReportError('first item should be a red cone');
            }
            else if (igeom == 1)
            {
                if (!(geom instanceof be.DgnSphere) || lineColor.Red != 0 || lineColor.Blue != 0 || lineColor.Green != 255)
                    be.Script.ReportError('second item should be a green sphere');
            }

            ++igeom;
            geomcollection.ToNext(geomcollectionIter);
        }

    if (igeom != 2)
        be.Script.ReportError('expected only 2 geoms');
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      02/16
    //---------------------------------------------------------------------------------------
    function testEC(db: be.DgnDb): void
    {
        //  EC API
        var schemas: be.SchemaManager = db.Schemas;
        var pe: be.ECClass = schemas.GetECClass(be.DGN_ECSCHEMA_NAME, be.DGN_CLASSNAME_PhysicalElement);
        if (!pe)
            be.Script.ReportError('SchemaManager.GetECClass could not find ' + be.DGN_ECSCHEMA_NAME + '.' + be.DGN_CLASSNAME_PhysicalElement);

        var peprops: be.ECPropertyCollection = pe.Properties;
        var foundCode: boolean = false;
        var propertyCount: number = 0;
        for (var propiter = peprops.Begin(); peprops.IsValid(propiter); peprops.ToNext(propiter))
        {
            var peprop: be.ECProperty = peprops.GetECProperty(propiter);
            be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, peprop.Name);
            if (peprop.Name == 'Code')
                foundCode = true;
            ++propertyCount;
        }
        if (!foundCode)
            be.Script.ReportError('ECPropertyCollection must have failed -- the Code property was not found');
        if (propertyCount <= 2)
            be.Script.ReportError('ECPropertyCollection must have failed -- there are more than 2 properties on PhysicalElement');


        var baseclasses: be.ECClassCollection = pe.BaseClasses;
        var foundSpatialElement: boolean = false;
        var baseCount: number = 0;
        for (var clsiter = baseclasses.Begin(); baseclasses.IsValid(clsiter); baseclasses.ToNext(clsiter))
        {
            var cls: be.ECClass = baseclasses.GetECClass(clsiter);
            be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, cls.Name);
            if (cls.Name == 'SpatialElement')
                foundSpatialElement = true;
            ++baseCount;
        }
        if (!foundSpatialElement)
            be.Script.ReportError('BaseClasses ECClassCollection must have failed -- I assume that PhysicalElement is derived from SpatialElement, but that base class was not found');
        if (baseCount != 1)
            be.Script.ReportError('BaseClasses ECClassCollection must have failed -- there should be 1 but I got ' + JSON.stringify(baseCount));

        var se: be.ECClass = schemas.GetECClass(be.DGN_ECSCHEMA_NAME, be.DGN_CLASSNAME_SpatialElement);
        var derivedclasses: be.ECClassCollection = se.DerivedClasses;
        var derivedCount: number = 0;
        for (var clsiter = derivedclasses.Begin(); derivedclasses.IsValid(clsiter); derivedclasses.ToNext(clsiter))
        {
            var cls: be.ECClass = derivedclasses.GetECClass(clsiter);
            be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, cls.Name);
            ++derivedCount;
        }
        if (derivedCount == 0)
            be.Script.ReportError('DerivedClasses ECClassCollection must have failed -- there should be at least 1');

    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      02/16
    //---------------------------------------------------------------------------------------
    function TestDgnDbScript(db: be.DgnDb, params: Params): number
    {
        var zz = 0.1;
        // Try out the new categories iterator
        var catid: be.DgnObjectId = null;
        var categories: be.DgnObjectIdSet = be.DgnCategory.QueryCategories(db);
        for (var catiter = categories.Begin(); categories.IsValid(catiter); categories.ToNext(catiter))
        {
            var cat = be.DgnCategory.QueryCategory(categories.GetId(catiter), db);
            if (cat.CategoryName == params.categoryName)
                catid = cat.CategoryId;
        }
        if (!catid)
            be.Script.ReportError('failed to find category ' + params.categoryName + ' using category iterator');

        //  Verify that we can find it the easy way, too.
        if (!catid.Equals(be.DgnCategory.QueryCategoryId(params.categoryName, db)))
            be.Script.ReportError('QueryCategoryId failed?');

        //  Create element
        var model = db.Models.GetModel(db.Models.QueryModelId(be.DgnModel.CreateModelCode(params.modelName)));
        var ele = be.PhysicalElement.Create(model, catid, '');

        //  Try out GeometryBuilder
        var builder = new be.GeometryBuilder(ele, new be.DPoint3d(0, 0, 0), new be.YawPitchRollAngles(0, 0, 0));

        //  1, add RenderGeometryParams. We will look for this in the collection test below
        var gparams = builder.GeometryParams;
        gparams.LineColor = new be.ColorDef(255, 0, 0, 0);
        gparams.GeometryClass = be.RenderDgnGeometryClass.Construction;
        builder.AppendRenderGeometryParams(gparams);

        var shiftCount = 0;

        //  2, add DgnSphere. We will look for this in the collection test below
        var sphere = be.DgnSphere.CreateSphere(new be.DPoint3d(0, 0, 0), 1.0);
        Shift (sphere, shiftCount, 0);
        builder.AppendGeometry(sphere);

        //  3, add DgnCone. We will look for this in the collection test below
        var cone = be.DgnCone.CreateCircularCone (
                new be.DPoint3d (0,0,0),
                new be.DPoint3d (1,1,0),
                0.5, 0.3, true);
        Shift (cone, shiftCount, 1);
        builder.AppendGeometry(cone);
        shiftCount++;
        var center = new be.DPoint3d (0,0,zz);
        var vector0 = new be.DVector3d (2,1,0).Scale (0.3);
        var vector90 = new be.DVector3d (1,4,0).Scale(0.3);
        var arc = new be.EllipticArc (center, vector0, vector90,
                    be.Angle.CreateDegrees (-10.0),
                    be.Angle.CreateDegrees (95.0)
                    );

        var arcPerp = arc.CloneWithPerpendicularAxes ();
        Shift (arc, shiftCount, 0);
        Shift (arcPerp, shiftCount, 1);

        ShowArc (builder, arc);
        ShowArc (builder, arcPerp);


        var centerToCenterVector = arc.GetCenter ().VectorTo (arcPerp.GetCenter ());
        var centerToCenterLine  = new be.LineSegment (arc.GetCenter (), arc.GetCenter().Plus (centerToCenterVector));
        var startTestLine = new be.LineSegment (arc.PointAtFraction (0.0), arc.PointAtFraction (0.0).Plus (centerToCenterVector));
        var endTestLine   = new be.LineSegment (arc.PointAtFraction (1.0), arc.PointAtFraction (1.0).Plus (centerToCenterVector));
        builder.AppendGeometry (centerToCenterLine);
        builder.AppendGeometry (startTestLine);
        builder.AppendGeometry (endTestLine);
        var line = new be.LineSegment (new be.DPoint3d (0,0,zz), new be.DPoint3d(0,4,zz));
        builder.AppendGeometry (line);
        shiftCount++;

        var points = new be.DPoint3dArray ();
            points.Add (new be.DPoint3d (0,0,zz));
            points.Add (new be.DPoint3d (1,0,zz));
            points.Add (new be.DPoint3d (1,1,zz));
            points.Add (new be.DPoint3d (0,1,zz));
            points.Add (new be.DPoint3d (0,0.4,zz));
            points.Add (new be.DPoint3d (0.5,0.7,zz));
        var linestring = new be.LineString (points);

        Shift(linestring, shiftCount, 0);
        shiftCount++;
        builder.AppendGeometry (linestring);
        var catenary = be.CatenaryCurve.CreateFromCoefficientAndXLimits (
                    new be.DPoint3d (0,0,zz),
                    new be.DVector3d (1,0,0),
                    new be.DVector3d (0,1,0),
                     1.0,
                    -0.5,
                     1.0
                    );
        Shift (catenary, shiftCount, 0);
        shiftCount++;
        builder.AppendGeometry (catenary);

        var arc3 = be.EllipticArc.CreateCircleStartMidEnd (new be.DPoint3d (1,0,0), new be.DPoint3d (0,1,0), new be.DPoint3d (-1,0,0));
        Shift(arc3, shiftCount,0);
        shiftCount++;
        builder.AppendGeometry (arc3);

        var bspline2 = be.BsplineCurve.CreateFromPoles (points, 2);
        Shift (bspline2, shiftCount,0);      builder.AppendGeometry (bspline2);
        var bspline3 = be.BsplineCurve.CreateFromPoles (points, 3);
        Shift (bspline3, shiftCount, 2);     builder.AppendGeometry (bspline3);
        var bspline4 = be.BsplineCurve.CreateFromPoles (points, 4);
        Shift (bspline4, shiftCount, 4);     builder.AppendGeometry (bspline4);
        var bspline5 = be.BsplineCurve.CreateFromPoles (points, 5);
        Shift (bspline5, shiftCount, 6);     builder.AppendGeometry (bspline5);


        if (0 != builder.SetGeometryStreamAndPlacement(ele))
            be.Script.ReportError('SetGeometryStreamAndPlacement failed');

        ele.Insert();

        // Verify that my DgnCategoryId was used
        if (!catid.Equals(ele.CategoryId))
            be.Script.ReportError('set DgnCategoryId failed');

        //  Test GeometryCollection 
        var geomcollection = ele.Geometry;
        var geomcollectionIter = geomcollection.Begin();
        var igeom = 0;
        while (geomcollection.IsValid(geomcollectionIter))
        {
            var geomParams = geomcollection.GetGeometryParams(geomcollectionIter);
            var lineColor = geomParams.LineColor;
            if (lineColor.Red != 255 || lineColor.Blue != 0 || lineColor.Green != 0)
                be.Script.ReportError('append GeometryParams failed - LineColor');

            if (geomParams.GeometryClass != be.RenderDgnGeometryClass.Construction)
                be.Script.ReportError('append GeometryParams failed - RenderDgnGeometryClass');

            var subcat = geomParams.SubCategoryId;
            // If you use special subcategories for things like centerlines, you can test that here.

            var geomPrim = geomcollection.GetGeometry(geomcollectionIter);
            if (igeom == 0)
            {
                if (!geomPrim)                
                    be.Script.ReportError('first item should be geometry!');
                var geom = geomPrim.Geometry;
                if (!(geom instanceof be.DgnSphere))
                    be.Script.ReportError('first item should be a sphere');
                var sphere: be.DgnSphere = <be.DgnSphere>geom;
                if (!sphere)
                    be.Script.ReportError('first item should be a sphere');
            }
            else if (igeom == 1)
            {
                if (!geomPrim)
                    be.Script.ReportError('second item should be geometry!');
                var geom = geomPrim.Geometry;
                if (!(geom instanceof be.DgnCone))
                    be.Script.ReportError('second item should be a cone');
                var cone: be.DgnCone = <be.DgnCone>geom;
                if (!cone)
                    be.Script.ReportError('second item should be a cone');
            }

            ++igeom;
            geomcollection.ToNext(geomcollectionIter);
        }

        //  Test Element.Transform
        if (ele.Placement.Angles.YawDegrees != 0.0)
            be.Script.ReportError('Element should have been unrotated initially');

        var rotateTransform = be.Transform.CreateRotationAroundRay(new be.DRay3d(new be.DPoint3d(0, 0, 0), new be.DVector3d(0, 0, 1)), be.Angle.CreateDegrees(45.0));
        ele.Transform(rotateTransform);
        ele.Update();

        if (ele.Placement.Angles.YawDegrees != 45.0)
            be.Script.ReportError('Element.Transform failed');

        //  Test GeometryBuilders
        testGeomBuilders(db, model, catid);
        
        //  Test EC API
        testEC(db);

        return 0;
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      02/16
    //---------------------------------------------------------------------------------------
    be.RegisterDgnDbScript('DgnScriptTests.TestDgnDbScript', TestDgnDbScript);
}

