/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/ComponentModelTest.ts $
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

            var element = model.CreateElement(be.DGN_SCHEMA(be.DGN_CLASSNAME_PhysicalElement), options.Category);

            var origin = new be.DPoint3d(0, 0, 0);
            var angles = new be.YawPitchRollAngles(0, 0, 45);
            var builder = new be.ElementGeometryBuilder(element, origin, angles);
            builder.AppendBox(params.Q, params.W, params.R);
            builder.SetGeomStreamAndPlacement(element);

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

            var element = model.CreateElement(be.DGN_SCHEMA(be.DGN_CLASSNAME_PhysicalElement), options.Category);

            var origin = new be.DPoint3d(1, 2, 3);
            var angles = new be.YawPitchRollAngles(0, 0, 0);
            var builder = new be.ElementGeometryBuilder(element, origin, angles);
            builder.AppendBox(params.X, params.Y, params.Z);
            builder.SetGeomStreamAndPlacement(element);

            element.Insert();

            var element2 = model.CreateElement(be.DGN_SCHEMA(be.DGN_CLASSNAME_PhysicalElement), options.Category);

            var origin2 = new be.DPoint3d(10, 12, 13);
            var angles2 = new be.YawPitchRollAngles(0, 0, 0);
            var builder2 = new be.ElementGeometryBuilder(element2, origin2, angles2);
            builder2.AppendBox(params.X, params.Y, params.Z);
            builder2.SetGeomStreamAndPlacement(element2);

            element2.Insert();

            element.SetParent(element2);
            element.Update();

            return 0;
        }
    }

    /**
     * The 'Thing' component. This component contains both raw elements and an instance of Gadget.
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

            var element = model.CreateElement(be.DGN_SCHEMA(be.DGN_CLASSNAME_PhysicalElement), options.Category);

            var origin = new be.DPoint3d(2, 0, 0);
            var angles = new be.YawPitchRollAngles(45, 0, 0);
            var builder = new be.ElementGeometryBuilder(element, origin, angles);
            builder.AppendBox(params.A, params.B, params.C);
            builder.SetGeomStreamAndPlacement(element);

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