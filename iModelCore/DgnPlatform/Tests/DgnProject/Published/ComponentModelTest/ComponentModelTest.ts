/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ComponentModelTest/ComponentModelTest.ts $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/** 
 * ComponentModelTest
 */
module ComponentModelTest
{
    import be = Bentley.Dgn;

    // *** WARNING: Keep this consistent with ComponentModelTest.cpp
    var TEST_JS_NAMESPACE = "ComponentModelTest";
    var TEST_WIDGET_COMPONENT_NAME = "Widget";
    var TEST_GADGET_COMPONENT_NAME = "Gadget";
    var TEST_THING_COMPONENT_NAME = "Thing";

    //  Utility function that sets up the geometry of the specified element as a slab
    function makeBox(element: be.GeometricElement3d, origin: be.DPoint3d, angles: be.YawPitchRollAngles, xsize: number, ysize: number, zsize: number): void
    {
        var boxSize = new be.DVector3d (xsize, ysize, zsize);
        var box = be.DgnBox.CreateCenteredBox (new be.DPoint3d(0,0,0), boxSize, true); // NB: the *geometry* is always defined in an LCS w/ origin 0,0,0. The placement below puts where we want it.

        var builder = be.GeometryBuilder.CreateFor3dModel(element.Model, element.CategoryId, origin, angles);
        builder.AppendGeometry (box);
        builder.Finish(element);
    }

    //  Utility function that creates a new (non-persistent) GeometricElement3d object in memory and assigns it to the correct category 
    function makeElement(model: be.DgnModel, cdef: be.ComponentDef): be.GeometricElement3d
    {
        return be.GeometricElement3d.CreateGeometricElement3d(model, cdef.Category.CategoryId, be.GENERIC_SCHEMA(be.GENERIC_CLASSNAME_PhysicalObject));
    }

    /**
     * The 'Gadget' component. This is a simple case of just one element.
     */
    module Gadget
    {
        /** The Gadget component's logic */
        export function GenerateElements(componentModel: be.ComponentModel, destModel: be.DgnModel, instance: be.ECInstance, cdef: be.ComponentDef): number
        {
            var Q = instance.GetValue('Q').GetDouble();
            var W = instance.GetValue('W').GetDouble();
            var R = instance.GetValue('R').GetDouble();

            var element = makeElement(componentModel, cdef);

            var origin = new be.DPoint3d(0, 0, 0);
            var angles = new be.YawPitchRollAngles(0, 0, 45);

            makeBox(element, origin, angles, Q, W, R);

            element.Insert();

            return 0;
        }
    }

    /**
     * The 'Widget' component. This component makes two elements, one a child of the other.
     */
    module Widget
    {
        /** The Widget component's logic */
        export function GenerateElements(componentModel: be.ComponentModel, destModel: be.DgnModel, instance: be.ECInstance, cdef: be.ComponentDef): number
        {
            var X = instance.GetValue('X').GetDouble();
            var Y = instance.GetValue('Y').GetDouble();
            var Z = instance.GetValue('Z').GetDouble();

            var element = makeElement(componentModel, cdef);
            var origin = new be.DPoint3d(1, 2, 3);
            var angles = new be.YawPitchRollAngles(0, 0, 0);
            makeBox(element, origin, angles, X, Y, Z);
            element.Insert();

            var element2 = makeElement(componentModel, cdef);
            var origin2 = new be.DPoint3d(10, 12, 13);
            var angles2 = new be.YawPitchRollAngles(0, 0, 0);
            makeBox(element2, origin2, angles2, X, Y, Z);
            element2.Insert();

            element.SetParent(element2);
            element.Insert();


            /* WIP -- why am I calling Update on a new element?? That will always fail!
            var element3 = makeElement(componentModel, cdef);

            var builder = new be.GeometryBuilder(element, new be.DPoint3d(0,0,0), angles);

            builder.AppendGeometry(new be.LineSegment (new be.DPoint3d (0,0,0), new be.DPoint3d(1,0,0)));
            builder.Finish(element3);
            element3.Update ();
            */

            return 0;
        }
    }

    /**
     * The 'Thing' component. This component contains both a plain physical element and an instance of Gadget.
     */
    module Thing
    {
        /** The Thing component's logic */
        export function GenerateElements(componentModel: be.ComponentModel, destModel: be.DgnModel, instance: be.ECInstance, cdef: be.ComponentDef): number
        {
            var db = componentModel.DgnDb;

            var A = instance.GetValue('A').GetDouble();
            var B = instance.GetValue('B').GetDouble();
            var C = instance.GetValue('C').GetDouble();

            var element = makeElement(componentModel, cdef);
            var origin = new be.DPoint3d(2, 0, 0);
            var angles = new be.YawPitchRollAngles(45, 0, 0);
            makeBox(element, origin, angles, A, B, C);
            element.Insert();

            var gadgetComponentDef = be.ComponentDef.FindByName(db, TEST_JS_NAMESPACE + '.' + TEST_GADGET_COMPONENT_NAME);
            var gparams = gadgetComponentDef.MakeParameters();
            gparams.SetValue('Q', be.ECValue.FromDouble(A + 1));
            gparams.SetValue('W', be.ECValue.FromDouble(B + 1));
            gparams.SetValue('R', be.ECValue.FromDouble(C + 1));
            var element2 = gadgetComponentDef.MakeUniqueInstance(componentModel, gparams, null);

            // *** TBD:
            //var gorigin = origin.Plus(new be.DVector3d(1, 0, 0));
            //var gplacement = new be.Placement3d(gorigin, angles);
            //element2.SetPlacement(gplacement);

            element2.Insert();

            return 0;
        }
    }

    /* 
     * Register the component solvers
     */
    be.RegisterModelSolver(TEST_JS_NAMESPACE + "." + TEST_WIDGET_COMPONENT_NAME, Widget.GenerateElements);
    be.RegisterModelSolver(TEST_JS_NAMESPACE + "." + TEST_GADGET_COMPONENT_NAME, Gadget.GenerateElements);
    be.RegisterModelSolver(TEST_JS_NAMESPACE + "." + TEST_THING_COMPONENT_NAME, Thing.GenerateElements);
}