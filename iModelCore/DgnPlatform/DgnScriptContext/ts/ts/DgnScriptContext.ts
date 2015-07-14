module BentleyApi.Dgn {
    var s_egaFunctions: any = {};
    export function GetEgaRegistry(): any { return s_egaFunctions; }
    export function RegisterEGA(egaName: string, egaFunction: any): void {
        s_egaFunctions[egaName] = egaFunction;
    }
} 