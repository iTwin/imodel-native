//! Script that is executed by one of the unit tests in DgnScriptContext_Test.cpp
module DgnScriptChecker {
    function logMessage(msg: string): void {
        Bentley.Dgn.Logging.Message('TestRunner', Bentley.Dgn.LoggingSeverity.Info, msg);
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

        IsNearDPoint3d(a: Bentley.Dgn.DPoint3d, b: Bentley.Dgn.DPoint3d) {
            if (this.NearDouble(this.Abs(a.X - b.X), 0.0, false)
                && this.NearDouble(this.Abs(a.Y - b.Y), 0.0, false)
                && this.NearDouble(this.Abs(a.Z - b.Z), 0.0, false)
                )
                return true;
            logMessage('NearPoint');
            return false;
        }

        IsNearDPoint2d(a: Bentley.Dgn.DPoint2d, b: Bentley.Dgn.DPoint2d)
        {
            if (this.NearDouble(this.Abs(a.X - b.X), 0.0, false)
                && this.NearDouble(this.Abs(a.Y - b.Y), 0.0, false)
                )
                return true;
            logMessage('NearPoint');
            return false;
        }

        IsNearDVector3d(a: Bentley.Dgn.DVector3d, b: Bentley.Dgn.DVector3d)
        {
            if (this.NearDouble(this.Abs(a.X - b.X), 0.0, false)
                && this.NearDouble(this.Abs(a.Y - b.Y), 0.0, false)
                && this.NearDouble(this.Abs(a.Z - b.Z), 0.0, false)
                )
                return true;
            logMessage('NearVector');
            return false;
        }

        IsNearDVector2d(a: Bentley.Dgn.DVector2d, b: Bentley.Dgn.DVector2d)
        {
            if (this.NearDouble(this.Abs(a.X - b.X), 0.0, false)
                && this.NearDouble(this.Abs(a.Y - b.Y), 0.0, false)
                )
                return true;
            logMessage('NearVector');
            return false;
        }


        IsNearRotmatrix(b: Bentley.Dgn.RotMatrix, c: Bentley.Dgn.RotMatrix, reportError: boolean): boolean
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
            Bentley.Dgn.Script.ReportError("Not Equal");
            return false;
            }
    }


    logMessage('Checker B');
}

