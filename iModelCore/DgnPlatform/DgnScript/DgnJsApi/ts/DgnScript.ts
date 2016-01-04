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

    /** The alias of the dgn schema namespace */
    export var DGN_ECSCHEMA_NAME = "dgn";

    /** The names of classes in the dgn schema. */
    export var DGN_CLASSNAME_SpatialElement = "SpatialElement";
    export var DGN_CLASSNAME_PhysicalElement = "PhysicalElement";

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
    export function DGN_SCHEMA(name: string): string
    {
        return DGN_ECSCHEMA_NAME + "." + name;
    }

} 