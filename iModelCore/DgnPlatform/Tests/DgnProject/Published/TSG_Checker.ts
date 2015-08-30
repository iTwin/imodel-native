//! Script that is executed by one of the unit tests in DgnScriptContext_Test.cpp
module DgnScriptChecker {
    BentleyApi.Dgn.JsUtils.ReportError(':Checker A');

    export class Checker {
        AbsTol: number;
        RelTol: number;

        constructor() {
            this.AbsTol = 1.0e-12;
            this.RelTol = 1.0e-12;
            BentleyApi.Dgn.JsUtils.ReportError(':Checker Constructor');

        }

        Abs(a: number) : number { return a >= 0 ? a : -a; }
        ConstructErrorString(a: number, b: number) : string 
            {
            var labelA = ":Value of A was: "
            var labelB = " Value of B was: "
            var fullA = labelA.concat(a.toString());
            var fullB = labelB.concat(b.toString());
            var fullString = fullA.concat(fullB);
            return fullString;
            }
        NearDouble(a: number, b: number, reportError : boolean) : boolean
        {
        var d = this.Abs(b - a);
        if (d < this.AbsTol)
            return true;
        if (reportError)
            var message = this.ConstructErrorString(a,b);
            BentleyApi.Dgn.JsUtils.ReportError(message);
            return false;
        }

        IsNearJsDPoint3d(a: BentleyApi.Dgn.JsDPoint3d, b: BentleyApi.Dgn.JsDPoint3d) {
            if (this.NearDouble(this.Abs(a.X - b.X), 0.0, false)
                && this.NearDouble(this.Abs(a.Y - b.Y), 0.0, false)
                && this.NearDouble(this.Abs(a.Z - b.Z), 0.0, false)
                )
                return true;
            BentleyApi.Dgn.JsUtils.ReportError(':NearPoint');
            return false;
        }
        IsNearJsDVector3d(a: BentleyApi.Dgn.JsDVector3d, b: BentleyApi.Dgn.JsDVector3d) {
            if (this.NearDouble(this.Abs(a.X - b.X), 0.0, false)
                && this.NearDouble(this.Abs(a.Y - b.Y), 0.0, false)
                && this.NearDouble(this.Abs(a.Z - b.Z), 0.0, false)
                )
                return true;
            BentleyApi.Dgn.JsUtils.ReportError(':NearVector');
            return false;
        }
        IsNearJsRotmatrix(b: BentleyApi.Dgn.JsRotMatrix, c: BentleyApi.Dgn.JsRotMatrix, reportError: boolean): boolean
            {
            var d = b.MaxDiff (c);
            var a = b.MaxAbs () + c.MaxAbs ();
            if(reportError)
            return(this.NearDouble(a,a+d,true));
            else
                BentleyApi.Dgn.JsUtils.ReportError(':NearRotmatrix');
            }

        CheckBool(a: boolean, b: boolean) : boolean
            {
            if(a==b)
                return true;
            BentleyApi.Dgn.JsUtils.ReportError(":Not Equal");
            return false;
            }
    }


    BentleyApi.Dgn.JsUtils.ReportError(':Checker B');
}

