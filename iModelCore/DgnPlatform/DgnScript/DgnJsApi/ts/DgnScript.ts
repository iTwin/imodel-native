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

    /** The alias of the dgn schema namespace */
    export var DGN_ECSCHEMA_NAME = "dgn";

    /** The name of the PhysicalElement class in the dgn schema. */
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