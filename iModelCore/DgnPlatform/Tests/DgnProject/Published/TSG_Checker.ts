//! Script that is executed by one of the unit tests in DgnScriptContext_Test.cpp
module DgnScriptChecker {
    function logMessage(msg: string): void {
        BentleyApi.Dgn.Logging.Message('TestRunner', BentleyApi.Dgn.LoggingSeverity.Info, msg);
    }

    logMessage('Checker A');

    export class Checker {
        AbsTol: number;
        RelTol: number;

        constructor() {
            this.AbsTol = 1.0e-12;
            this.RelTol = 1.0e-12;
            logMessage('Checker Constructor');

        }

        Abs(a: number) : number { return a >= 0 ? a : -a; }
        ConstructErrorString(a: number, b: number) : string 
            {
            var labelA = "Value of A was: "
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
            logMessage(message);
            return false;
        }

        IsNearJsDPoint3d(a: BentleyApi.Dgn.JsDPoint3d, b: BentleyApi.Dgn.JsDPoint3d) {
            if (this.NearDouble(this.Abs(a.X - b.X), 0.0, false)
                && this.NearDouble(this.Abs(a.Y - b.Y), 0.0, false)
                && this.NearDouble(this.Abs(a.Z - b.Z), 0.0, false)
                )
                return true;
            logMessage('NearPoint');
            return false;
        }

        IsNearJsDPoint2d(a: BentleyApi.Dgn.JsDPoint2d, b: BentleyApi.Dgn.JsDPoint2d)
        {
            if (this.NearDouble(this.Abs(a.X - b.X), 0.0, false)
                && this.NearDouble(this.Abs(a.Y - b.Y), 0.0, false)
                )
                return true;
            logMessage('NearPoint');
            return false;
        }

        IsNearJsDVector3d(a: BentleyApi.Dgn.JsDVector3d, b: BentleyApi.Dgn.JsDVector3d)
        {
            if (this.NearDouble(this.Abs(a.X - b.X), 0.0, false)
                && this.NearDouble(this.Abs(a.Y - b.Y), 0.0, false)
                && this.NearDouble(this.Abs(a.Z - b.Z), 0.0, false)
                )
                return true;
            logMessage('NearVector');
            return false;
        }

        IsNearJsDVector2d(a: BentleyApi.Dgn.JsDVector2d, b: BentleyApi.Dgn.JsDVector2d)
        {
            if (this.NearDouble(this.Abs(a.X - b.X), 0.0, false)
                && this.NearDouble(this.Abs(a.Y - b.Y), 0.0, false)
                )
                return true;
            logMessage('NearVector');
            return false;
        }


        IsNearJsRotmatrix(b: BentleyApi.Dgn.JsRotMatrix, c: BentleyApi.Dgn.JsRotMatrix, reportError: boolean): boolean
            {
            var d = b.MaxDiff (c);
            var a = b.MaxAbs () + c.MaxAbs ();
            if(reportError)
                return(this.NearDouble(a,a+d,true));

            logMessage('NearRotmatrix');
            }

        CheckBool(a: boolean, b: boolean) : boolean
            {
            if(a==b)
                return true;
            BentleyApi.Dgn.Script.ReportError("Not Equal");
            return false;
            }
    }


    logMessage('Checker B');
}

