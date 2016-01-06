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

    function TestDgnDbScript(db: be.DgnDb, params: Params): number
    {
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

        //  Try out SolidPrimitive
        var builder = new be.ElementGeometryBuilder(ele, new be.DPoint3d(0, 0, 0), new be.YawPitchRollAngles(0, 0, 0));

        var sphere = be.DgnSphere.CreateSphere(new be.DPoint3d(0, 0, 0), 1.0);
        builder.AppendSolidPrimitive(sphere);

        var arc = new be.EllipticArc (
                    new be.DPoint3d (2,2,2),
                    new be.DVector3d (2,0,0),
                    new be.DVector3d (0,2,0),
                    be.Angle.CreateDegrees (0.0),
                    be.Angle.CreateDegrees (95.0)
                    );
         builder.Append (arc);


        var line = new be.LineSegment (new be.DPoint3d (0,0,0), new be.DPoint3d(0,4,0));
        builder.Append (line);

        var points = new be.DPoint3dArray ();
            points.Add (new be.DPoint3d (0,4,0));
            points.Add (new be.DPoint3d (1,4,0));
            points.Add (new be.DPoint3d (1,5,0));
            points.Add (new be.DPoint3d (0,5,0));
            points.Add (new be.DPoint3d (0,4.5,0));
            points.Add (new be.DPoint3d (0.5,4.5,0));
        var linestring = new be.LineString (points);
        builder.Append (linestring);
        var catenary = be.CatenaryCurve.CreateFromCoefficientAndXLimits (
                    new be.DPoint3d (0,0,0),
                    new be.DVector3d (1,0,0),
                    new be.DVector3d (0,1,0),
                     4.0,
                    -2.0,
                     6.0
                    );
        builder.Append (catenary);



        if (0 != builder.SetGeomStreamAndPlacement(ele))
            be.Script.ReportError('SetGeomStreamAndPlacement failed');

        ele.Insert();

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

        return 0;
    }

    be.RegisterDgnDbScript('DgnScriptTests.TestDgnDbScript', TestDgnDbScript);
}

