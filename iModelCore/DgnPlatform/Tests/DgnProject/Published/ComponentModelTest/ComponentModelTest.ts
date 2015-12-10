/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ComponentModelTest/ComponentModelTest.ts $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    function makeBox(element: be.PhysicalElement, origin: be.DPoint3d, angles: be.YawPitchRollAngles, xsize: number, ysize: number, zsize: number): void
    {
        var boxSize = new be.DVector3d (xsize, ysize, zsize);
        var box = be.DgnBox.CreateBoxCentered (new be.DPoint3d(0,0,0), boxSize, true); // NB: the *geometry* is always defined in an LCS w/ origin 0,0,0. The placement below puts where we want it.

        var builder = new be.ElementGeometryBuilder(element, origin, angles);
        builder.Append(box);
        builder.SetGeomStreamAndPlacement(element);
    }

    //  Utility function that creates a new (non-persistent) PhysicalElement object in memory and assigns it to the correct category 
    function makeElement(model: be.DgnModel, options: be.ModelSolverOptions): be.PhysicalElement
    {
        return be.PhysicalElement.Create(model, be.DgnCategory.QueryCategoryId(options.Category, model.DgnDb), '');
    }

    /**
     * The 'Gadget' component. This is a simple case of just one element.
     */
    module Gadget
    {
        /** The parameters that the Gadget component requires as inputs */
        export class Parameters
        {
            // *** WARNING: Keep this consistent with ComponentModelTest.cpp
            Q: number;
            W: number;
            R: number;
        }

        /** The Gadget component's logic */
        export function GenerateElements(model: be.DgnModel, params: Parameters, options: be.ModelSolverOptions)
        {
            model.DeleteAllElements();

            var element = makeElement(model, options);

            var origin = new be.DPoint3d(0, 0, 0);
            var angles = new be.YawPitchRollAngles(0, 0, 45);

            makeBox(element, origin, angles, params.Q, params.W, params.R);

            element.Insert();

            return 0;
        }
    }

    /**
     * The 'Widget' component. This component makes two elements, one a child of the other.
     */
    module Widget
    {
        /** The parameters that the Widget component requires as inputs */
        export class Parameters
        {
            // *** WARNING: Keep this consistent with ComponentModelTest.cpp
            X: number;
            Y: number;
            Z: number;
        }

        /** The Widget component's logic */
        export function GenerateElements(model: be.DgnModel, params: Parameters, options: be.ModelSolverOptions): number
        {
            model.DeleteAllElements();

            var element = makeElement(model, options);
            var origin = new be.DPoint3d(1, 2, 3);
            var angles = new be.YawPitchRollAngles(0, 0, 0);
            makeBox(element, origin, angles, params.X, params.Y, params.Z);
            element.Insert();

            var element2 = makeElement(model, options);
            var origin2 = new be.DPoint3d(10, 12, 13);
            var angles2 = new be.YawPitchRollAngles(0, 0, 0);
            makeBox(element2, origin2, angles2, params.X, params.Y, params.Z);
            element2.Insert();

            element.SetParent(element2);
            element.Update();

            return 0;
        }
    }

    /**
     * The 'Thing' component. This component contains both a plain physical element and an instance of Gadget.
     */
    module Thing
    {
        /** The parameters that the 'Thing' component requires as inputs */
        export class Parameters
        {
            // *** WARNING: Keep this consistent with ComponentModelTest.cpp
            A: number;
            B: number;
            C: number;
        }

        /** The Thing component's logic */
        export function GenerateElements(model: be.DgnModel, params: Parameters, options: be.ModelSolverOptions)
        {
            var db = model.DgnDb;

            model.DeleteAllElements();

            var element = makeElement(model, options);
            var origin = new be.DPoint3d(2, 0, 0);
            var angles = new be.YawPitchRollAngles(45, 0, 0);
            makeBox(element, origin, angles, params.A, params.B, params.C);
            element.Insert();

            var gadgetComponentModel = be.ComponentModel.FindModelByName(db, TEST_GADGET_COMPONENT_NAME);
            var gparams: Gadget.Parameters = { Q: params.A + 1, W: params.B + 1, R: params.C + 1 };
            var element2 = gadgetComponentModel.MakeInstance(model, "", JSON.stringify(gparams), null);

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