module Bentley.Dgn
{
    // The dictionary of EGA functions
    var m_egaFunctions: any = {};
    // Get the dictionary of EGA functions
    export function GetEgaRegistry(): any { return m_egaFunctions; }
    // Set and entry in the dictionary of EGA functions
    export function RegisterEGA(egaName: string, egaFunction: any): void {m_egaFunctions[egaName] = egaFunction;}

    // The dictionary of model solver functions
    var m_modelSolverFunctions: any = {};
    // Get the dictionary of model solver functions
    export function GetModelSolverRegistry(): any { return m_modelSolverFunctions; }
    // Set an entry in the dictionary of model solver functions
    export function RegisterModelSolver(solverName: string, solverFunction: any): void {m_modelSolverFunctions[solverName] = solverFunction;}

    // The dictionary of DgnDbScript functions
    var m_dgnDbScriptFunctions: any = {};
    // Get the dictionary of DgnDbScript functions
    export function GetDgnDbScriptRegistry(): any { return m_dgnDbScriptFunctions; }
    // Set an entry in the dictionary of DgnDbScript functions
    export function RegisterDgnDbScript(scriptName: string, scriptFunction: any): void { m_dgnDbScriptFunctions[scriptName] = scriptFunction; }

    /** The alias of the BisCore schema namespace */
    export var BIS_ECSCHEMA_NAME = "BisCore";
    /** The alias of the Generic schema namespace */
    export var GENERIC_ECSCHEMA_NAME = "Generic";

    /** The name of the GeometricElement class in the dgn schema. */
    export var BIS_CLASS_GeometricElement = "GeometricElement";
    /** The name of the GeometricElement3d class in the dgn schema. */
    export var BIS_CLASS_GeometricElement3d = "GeometricElement3d";
    /** The name of the SpatialElement class in the dgn schema. */
    export var BIS_CLASS_SpatialElement = "SpatialElement";
    /** The name of the PhysicalElement class in the dgn schema. */
    export var BIS_CLASS_PhysicalElement = "PhysicalElement";
    /** The name of the PhysicalObject class in the Generic schema. */
    export var GENERIC_CLASSNAME_PhysicalObject = "PhysicalObject";

    /**
     * Options that are passed to a model solver.
     */
    export class ModelSolverOptions
    {
        /** The category to which harvestable elements should be assigned by a model solver */
        Category: string;
    }

    /**
     * Returns the fully qualified ECSQL name of the specified ECClass in the dgn schema. 
     */
    export function BIS_SCHEMA(name: string): string
    {
        return BIS_ECSCHEMA_NAME + "." + name;
    }

    /**
     * Returns the fully qualified ECSQL name of the specified ECClass in the generic schema. 
     */
    export function GENERIC_SCHEMA(name: string): string
    {
        return GENERIC_ECSCHEMA_NAME + "." + name;
    }

    /**
     * Throws an exeception. This is for the use of native code.
     */
    export function ThrowException(name: string, details: string): void 
    {
        if (details)
            throw new Error(name + " (" + details + ")");
        else
            throw new Error(name);
    }

} 