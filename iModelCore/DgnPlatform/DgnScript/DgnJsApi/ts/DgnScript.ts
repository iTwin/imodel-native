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
} 