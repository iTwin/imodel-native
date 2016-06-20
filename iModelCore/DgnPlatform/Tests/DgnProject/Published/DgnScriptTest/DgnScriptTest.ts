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

    //---------------------------------------------------------------------------------------
    // *** NB: Keep this consistent with {DgnScriptContext_Test.cpp}TEST_F(DgnScriptTest, RunScripts)
    // @bsimethod                                   
    //---------------------------------------------------------------------------------------
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
    function fmtDPoint3d(pt: be.DPoint3d) {
        return "(" + pt.X + "," + pt.Y + "," + pt.Z + ")";
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   
    //---------------------------------------------------------------------------------------
    function testFile(filename: string) {
        var file = be.File.Fopen(filename, "r");
        while (true) {
            var line = file.ReadLine();
            if (file.Feof())
                break;
            be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, line);
        }

        file.Close();
     }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   
    //---------------------------------------------------------------------------------------
    function testEcSql_selectLastInstanceOf(db: be.DgnDb, cls: be.ECClass, minGenericObjectsExpected: number): be.DgnObjectId {
        var lastelid: be.DgnObjectId;

        //  0             1      2    3      4 
        var stmt = db.GetPreparedECSqlSelectStatement("ECInstanceId, Origin, Yaw, Pitch, Roll, BBoxLow, BBoxHigh FROM " + cls.ECSqlName);
        var count: number = 0;
        while (be.BeSQLiteDbResult.BE_SQLITE_ROW == stmt.Step()) {
            var elemid: be.DgnObjectId = stmt.GetValueId(0);
            var origin: be.DPoint3d = stmt.GetValueDPoint3d(1);
            var yaw: number = stmt.GetValueInt(2);
            var pitch: number = stmt.GetValueDouble(3);
            var roll: number = stmt.GetValueDouble(4);
            var bboxLow: be.DPoint3d = stmt.GetValueDPoint3d(5);
            var bboxHigh: be.DPoint3d = stmt.GetValueDPoint3d(6);

            // The above code shows how to extract multiple columns from a statement. 
            // In real code, we would do something with those values.

            be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, "eid: " + elemid.ToString() + " bbox:{" + fmtDPoint3d(bboxLow) + "," + fmtDPoint3d(bboxHigh));

            //  For purposes of this test, just remember the last element that we saw.
            lastelid = elemid;
            ++count;
        }

        if (count < minGenericObjectsExpected) {
            be.Script.ReportError('SELECT should have found at least ' + minGenericObjectsExpected + ' elements. It actually found ' + count);
            return null;
        }

        return lastelid;
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   
    //---------------------------------------------------------------------------------------
    function getElementRange(db: be.DgnDb, eid: be.DgnObjectId): be.DRange3d {
        var el: be.DgnElement = db.Elements.GetElement(eid);
        if (el == null) {
            be.Script.ReportError('db.Elements.Get(lastelid) failed');
            return null;
        }

        var pel: be.GeometrySource3d = el.ToGeometrySource3d();
        if (pel == null) {
            be.Script.ReportError('ToGeometrySource3d failed');
            return null;
        }
        return pel.Placement.CalculateRange();
    }
 
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Paul.Connelly   06/16
    +---------------+---------------+---------------+---------------+---------------+------*/
    function expectCondition(expected: boolean, actual: boolean, message: string): boolean {
        if (expected != actual) {
            be.Script.ReportError(message + ": expected: " + expected);

        return expected == actual;
        }
    }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Paul.Connelly   06/16
    +---------------+---------------+---------------+---------------+---------------+------*/
    function expectTrue(actual: boolean, message: string): boolean { return expectCondition(true, actual, message); }

    function expectFalse(actual: boolean, message: string): boolean { return expectCondition(false, actual, message); }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                    Paul.Connelly   06/16
    +---------------+---------------+---------------+---------------+---------------+------*/
    function testBriefcaseManagement(db: be.DgnDb, element: be.DgnElement, model: be.DgnModel) {
        var elementLockableId = be.LockableId.FromElement(element);
        expectTrue(elementLockableId.IsValid, "elementLockableId.IsValid");
        expectTrue(elementLockableId.Id.Equals(element.ElementId), "elementLockableId.Id.Equals(element.ElementId)");

        var operation: string = "test request";
        var request: be.RepositoryRequest = be.RepositoryRequest.Create(db, operation);
        expectTrue(null != request, "null != request");
        expectTrue(operation == request.Operation, "operation == request.Operation");
        expectTrue(db == request.Briefcase, "db == request.Briefcase");

        request.AddElement(element);
        expectTrue(be.RepositoryStatus.Success == request.FastQueryAvailability(), "FastQueryAvailability");
        expectTrue(be.RepositoryStatus.Success == request.QueryAvailability(), "QueryAvailability");
        expectTrue(be.RepositoryStatus.Success == request.Acquire(), "Acquire");

        var modelLockableId = be.LockableId.FromModel(model);
        expectTrue(modelLockableId.IsValid, "modelLockableId.IsValid");
        expectTrue(modelLockableId.Id.Equals(model.ModelId), "modelLockableId.Id.Equals(model.ModelId)");

        request = be.RepositoryRequest.Create(db, operation);
        request.AddModel(element.Model, be.LockLevel.Exclusive);
        expectTrue(be.RepositoryStatus.Success == request.FastQueryAvailability(), "FastQueryAvailability");
        expectTrue(be.RepositoryStatus.Success == request.QueryAvailability(), "QueryAvailability");
        expectTrue(be.RepositoryStatus.Success == request.Acquire(), "Acquire");

        var dbLockableId = be.LockableId.FromDgnDb(db);
        expectTrue(dbLockableId.IsValid, "dbLockableId.IsValid");

        request = be.RepositoryRequest.Create(db, operation);
        request.AddBriefcase(be.LockLevel.Exclusive);
        expectTrue(be.RepositoryStatus.Success == request.FastQueryAvailability(), "FastQueryAvailability");
        expectTrue(be.RepositoryStatus.Success == request.QueryAvailability(), "QueryAvailability");
        expectTrue(be.RepositoryStatus.Success == request.Acquire(), "Acquire");
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   
    //---------------------------------------------------------------------------------------
    function testEcSql(db: be.DgnDb, minGenericObjectsExpected: number) {

        var physObjClass = db.Schemas.GetECClass(be.GENERIC_ECSCHEMA_NAME, be.GENERIC_CLASSNAME_PhysicalObject);

        // Demonstrate how to do a normal SELECT
        var lastelid: be.DgnObjectId = testEcSql_selectLastInstanceOf(db, physObjClass, minGenericObjectsExpected);

        var lastelrange = getElementRange(db, lastelid);

        be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, "lastelid: " + lastelid.ToString() + " bbox:{" + fmtDPoint3d(lastelrange.Low) + "," + fmtDPoint3d(lastelrange.High));

        // Test spatial query. There are lots of ways to further refine the results. In this example,
        // I specify the ECClass of the elements that I am interested in. (Of course, in a real app, I would
        // be testing for valves and girders and so on.) I could also be testing for the properties aspects,
        // or codes, or various other things.
        var spatialQuery = db.GetPreparedECSqlSelectStatement(
            "SELECT rt.ECInstanceId FROM dgn.SpatialIndex rt, " + physObjClass.ECSqlName + 
            " WHERE rt.ECInstanceId MATCH DGN_spatial_overlap_aabb(:bbox)"
        );

        // *** TBD: AxisAlignedBoundingBox aka Range
        spatialQuery.BindDRange3d(spatialQuery.GetParameterIndex("bbox"), lastelrange);
        var foundIt: boolean = false;
        while ((be.BeSQLiteDbResult.BE_SQLITE_ROW == spatialQuery.Step()) && !foundIt) {
            var foundelemid: be.DgnObjectId = spatialQuery.GetValueId(0);
            var foundelemrange: be.DRange3d = getElementRange(db, foundelemid);
            be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, "foundelemid: " + foundelemid.ToString() + " bbox:{" + fmtDPoint3d(foundelemrange.Low) + "," + fmtDPoint3d(foundelemrange.High));
            foundIt = foundelemid.Equals(lastelid);
        }

        if (!foundIt) {
            be.Script.ReportError('spatial query failed to find element overlapping aabbox');
            return;
        }
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   
    //---------------------------------------------------------------------------------------
    function testInvalidArg()
    {
        var badEle: be.DgnElement = null;
        try
            {
            if (badEle.ElementId.IsValid())
                be.Script.ReportError('should not be here 1');
            be.Script.ReportError('should not be here 2');
            }
        catch (e)
            {
            be.Logging.Message('testInvalidArg1', be.LoggingSeverity.Info, JSON.stringify(e));
            }

        /* *** BeJsContext is not quite ready to return values while JsRT is in an exception state

        try
        {
            var builder = new be.GeometryBuilder(badEle, new be.DPoint3d(0, 0, 0), new be.YawPitchRollAngles(0,0,0));
            be.Script.ReportError('should not be here 3');
        }
        catch (e)
        {
            be.Logging.Message('testInvalidArg2', be.LoggingSeverity.Info, JSON.stringify(e));
        }

        */

        be.Logging.Message('DgnScriptTest', be.LoggingSeverity.Info, 'testInvalidArg - this should work');


    }

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
    function makeGeomPart(db: be.DgnDb, geoms: Array<be.Geometry>): be.DgnGeometryPart
    {
        var builder = be.GeometryBuilder.CreateGeometryPart(db, true);
        for (var i = 0; i < geoms.length; ++i)
            builder.AppendGeometry(geoms[i]);
        var geompart = be.DgnGeometryPart.Create(db);
        builder.SetGeometryStream(geompart);
        geompart.Insert();
        return geompart;
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      02/16
    //---------------------------------------------------------------------------------------
    function testGeomParts(model: be.DgnModel, catid: be.DgnObjectId): void
    {
        var cone = be.DgnCone.CreateCircularCone(new be.DPoint3d(0, 0, 0), new be.DPoint3d(1, 1, 0), 0.5, 0.3, true);
        var sphere = be.DgnSphere.CreateSphere(new be.DPoint3d(0, 0, 0), 1.0);
        var geoms = new Array<be.Geometry>();
        geoms.push(cone);
        geoms.push(sphere);
        var geompart: be.DgnGeometryPart = makeGeomPart(model.DgnDb, geoms);

        var ele: be.GeometricElement3d = be.GeometricElement3d.CreateGeometricElement3d(model, catid, be.GENERIC_ECSCHEMA_NAME + "." + be.GENERIC_CLASSNAME_PhysicalObject);

        var builder = new be.GeometryBuilder(ele, new be.DPoint3d(0, 0, 0), new be.YawPitchRollAngles(0, 0, 0));

        var gparams = builder.GeometryParams;
        gparams.GeometryClass = be.RenderDgnGeometryClass.Construction;

        gparams.LineColor = new be.ColorDef(1, 0, 0, 0);
        builder.AppendRenderGeometryParams(gparams);
        builder.AppendGeometryPart(geompart, new be.Placement3d(new be.DPoint3d(1, 0, 0), new be.YawPitchRollAngles(0, 0, 0)));

        gparams.LineColor = new be.ColorDef(2, 0, 0, 0);
        builder.AppendRenderGeometryParams(gparams);
        builder.AppendGeometryPart(geompart, new be.Placement3d(new be.DPoint3d(2, 0, 0), new be.YawPitchRollAngles(0, 0, 0)));

        gparams.LineColor = new be.ColorDef(3, 0, 0, 0);
        builder.AppendRenderGeometryParams(gparams);
        builder.AppendGeometryPart(geompart, new be.Placement3d(new be.DPoint3d(3, 0, 0), new be.YawPitchRollAngles(0, 0, 0)));

        builder.SetGeometryStreamAndPlacement(ele);
        ele.Insert();

        var geomcollection = ele.Geometry;
        var geomcollectionIter = geomcollection.Begin();
        var igeom = 0;
        while (geomcollection.IsValid(geomcollectionIter))
        {
            ++igeom;

            var geomParams = geomcollection.GetGeometryParams(geomcollectionIter);
            var lineColor = geomParams.LineColor;

            if (lineColor.Red != igeom)
                be.Script.ReportError('expected sequential colors');

            if (!geomcollection.GetGeometryPart(geomcollectionIter))
                be.Script.ReportError('expected to find geomparts');

            var parttrans = geomcollection.GetGeometryToWorld(geomcollectionIter);

            var xlat = parttrans.GetTranslation();
            if (xlat.X != igeom)
                be.Script.ReportError('expected sequential offsets');

            geomcollection.ToNext(geomcollectionIter);
        }

        if (igeom != 3)
            be.Script.ReportError('expected 3 part instances');
    }
    
    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      02/16
    //---------------------------------------------------------------------------------------
    function makeBuilderWithGeom(model: be.DgnModel, catid: be.DgnObjectId, geom: be.Geometry, color: be.ColorDef, geomClass: be.RenderDgnGeometryClass)
    {
        // Always use 0,0,0 for the placement of the part. We will transform it into place when we copy it into a destination assembly.
        var builder = be.GeometryBuilder.CreateForModel(model, catid, new be.DPoint3d(0, 0, 0), new be.YawPitchRollAngles(0, 0, 0));
        var gparams = builder.GeometryParams;
        gparams.LineColor = color;
        gparams.GeometryClass = geomClass;
        builder.AppendRenderGeometryParams(gparams);
        builder.AppendGeometry(geom);
        return builder;
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      02/16
    //---------------------------------------------------------------------------------------
    function testGeomBuilderAggregation(model: be.DgnModel, catid: be.DgnObjectId): void
    {
        var ele = be.GeometricElement3d.CreateGeometricElement3d(model, catid, be.GENERIC_ECSCHEMA_NAME + "." + be.GENERIC_CLASSNAME_PhysicalObject);

        // Create separate builders
        var constructionClass = be.RenderDgnGeometryClass.Construction;
        var geoms = new Array<be.Geometry>();
        geoms.push(be.DgnCone.CreateCircularCone(new be.DPoint3d(0, 0, 0), new be.DPoint3d(1, 1, 0), 0.5, 0.3, true));
        geoms.push(be.DgnSphere.CreateSphere(new be.DPoint3d(0, 0, 0), 1.0));
        var builders = new Array<be.GeometryBuilder>();
        for (var i = 0; i < geoms.length; ++i)
            builders.push(makeBuilderWithGeom(model, catid, geoms[i], new be.ColorDef(i, 0, 0, 0), constructionClass));

        // Copy contents of separate builders into the element's builder
        var builder = new be.GeometryBuilder(ele, new be.DPoint3d(0, 0, 0), new be.YawPitchRollAngles(0, 0, 0));
        for (var i = 0; i < builders.length; ++i)
            builder.AppendCopyOfGeometry(builders[i], new be.Placement3d(new be.DPoint3d(i, 0, 0), new be.YawPitchRollAngles(0, 0, 0)));

        //  Write the element to the Db
        builder.SetGeometryStreamAndPlacement(ele);
        ele.Insert();

        //  Verify that the element's geometry is a concatenation of the input builders
        var geomcollection = ele.Geometry;
        var geomcollectionIter = geomcollection.Begin();
        var igeom = 0;
        while (geomcollection.IsValid(geomcollectionIter))
        {
            var geomParams = geomcollection.GetGeometryParams(geomcollectionIter);
            var lineColor = geomParams.LineColor;
         
            // *** NEEDS WORK: Not getting the lineColors that I expect
            //if (lineColor.Red != igeom)
            //    be.Script.ReportError('expected sequential colors');

            var parttrans = geomcollection.GetGeometryToWorld(geomcollectionIter);

            var geom = geomcollection.GetGeometry(geomcollectionIter).Geometry;
            if (typeof geom != typeof geoms[igeom])
                be.Script.ReportError('unexpected type');

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
        var elementClass: be.ECClass = schemas.GetECClass(be.DGN_ECSCHEMA_NAME, be.DGN_CLASSNAME_PhysicalElement);
        if (!elementClass)
            be.Script.ReportError('SchemaManager.GetECClass could not find ' + be.DGN_ECSCHEMA_NAME + '.' + be.DGN_CLASSNAME_PhysicalElement);

        // -----------------------------------------------
        // Test GetProperties and ECPropertyCollection
        // -----------------------------------------------
        var peprops: be.ECPropertyCollection = elementClass.Properties;
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
            be.Script.ReportError('ECPropertyCollection must have failed -- there are more than 2 properties on GeometricElement3d');

        // -----------------------------------------------
        // Test GetProperty and PrimitiveECProperty
        // -----------------------------------------------
        var codeProp = elementClass.GetProperty('Code');
        if (!codeProp)
            be.Script.ReportError('GetProperty failed to find Code property');
        else
        {
            var primProp = codeProp.GetAsPrimitiveProperty();
            if (primProp)
                be.Script.ReportError('Code property is NOT a primitive property');
        }

        var LastModProp = elementClass.GetProperty('LastMod');
        if (!LastModProp)
            be.Script.ReportError('GetProperty failed to find LastMod property');
        else
        {
            var primProp = LastModProp.GetAsPrimitiveProperty();
            if (!primProp)
                be.Script.ReportError('LastModProp property is a primitive property');
            if (primProp.Type != be.ECPropertyPrimitiveType.DateTime)
                be.Script.ReportError('LastModProp property type should be DateTime');
        }

        // -----------------------------------------------
        // Test Base/DerivecClasses and ECClassCollection
        // -----------------------------------------------
        var baseclasses: be.ECClassCollection = elementClass.BaseClasses;
        var foundSpatialElement: boolean = false;
        var baseCount: number = 0;
        for (var clsiter = baseclasses.Begin(); baseclasses.IsValid(clsiter); baseclasses.ToNext(clsiter)) {
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
        for (var clsiter = derivedclasses.Begin(); derivedclasses.IsValid(clsiter); derivedclasses.ToNext(clsiter)) {
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
    function testUserProperties(el: be.DgnElement)
    {
        if (el.ContainsUserProperty('stuff'))
            be.Script.ReportError('no user props expected initially');

        if (true)
        {
            var stuff_userprop = el.GetUserProperty('stuff');
            stuff_userprop.ValueEC = be.ECValue.FromString('foo');
            stuff_userprop.Units = 'm';
            stuff_userprop.ReadOnly = true;
            // *** TRICKY: stuff_userprop is magically connected to el. Setting stuff_userprop's value actually
            //              updates data that is stored on el. That's why we don't need a "SetUserProperty" method.
        }

        if (!el.ContainsUserProperty('stuff'))
            be.Script.ReportError('stuff userprop should be there now');

        var stuff_userprop = el.GetUserProperty('stuff');
        if (stuff_userprop.ValueEC.GetString() != 'foo' || stuff_userprop.Units != 'm' || !stuff_userprop.ReadOnly)
            be.Script.ReportError('stuff userprop value should have been set to foo with units m');

        el.Update();

        if (!el.ContainsUserProperty('stuff'))
            be.Script.ReportError('stuff userprop should have been saved');

        el.GetUserProperty('nonsense').ValueEC = be.ECValue.FromString('bar');
        var nonsenseValue = el.GetUserProperty('nonsense').ValueEC;
        if (nonsenseValue.GetString() != 'bar')
            be.Script.ReportError('nonsense userprop should have been set to bar');

        el.RemoveUserProperty('nonsense');
        if (el.ContainsUserProperty('nonsense'))
            be.Script.ReportError('nonsense userprop should have been removed');
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      02/16
    //---------------------------------------------------------------------------------------
    function testProperties(el: be.DgnElement)
    {
        if (el.GetProperty('StringProperty'))
            be.Script.ReportError('StringProperty should not be set at the outset');

        if (0 != el.SetProperty('StringProperty', be.ECValue.FromString('stuff')))
            be.Script.ReportError('SetProperty failed');

        el.Update();

        var prop = el.GetProperty('StringProperty');
        if (!prop || prop.GetString() != 'stuff')
            be.Script.ReportError('SetProperty failed - changed value not verified');
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
        var ele = be.GeometricElement3d.CreateGeometricElement3d(model, catid, 'DgnPlatformTest.TestElementWithNoHandler');

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
        testGeomBuilderAggregation(model, catid);
        testGeomParts(model, catid);

        //  Test EC API
        testEC(db);

        //  DgnElement Properties and UserProperties
        testProperties(ele);
        testUserProperties(ele);

        //  Test argument validation
        testInvalidArg();

        //  Test ECSql API
        testEcSql(db, 2);

        //testFile("d:/tmp/xx.txt");

        //  Test briefcase management API
        testBriefcaseManagement(db, ele, model);

        return 0;
    }

    //---------------------------------------------------------------------------------------
    // @bsimethod                                   Sam.Wilson                      02/16
    //---------------------------------------------------------------------------------------
    be.RegisterDgnDbScript('DgnScriptTests.TestDgnDbScript', TestDgnDbScript);
}

