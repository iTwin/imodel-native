import { assert } from "chai";
import { CompareIModels } from "@bentley/data-validation-api/lib/compareIModels";
import { IModelInfo } from "@bentley/data-validation-api/lib/imodelinfo";
import { Reporter } from "@bentley/data-validation-api/lib/reporter";
import * as path from "path";
import * as glob from "glob";
import dsJson = require("../../config/dgnv8converter.dataset.json");

describe("DataValidation-QuickConversion", () => {
    it("Bim0200", () => {
        let pass = true;
        const reporter = new Reporter("././lib");
        reporter.setHtmlReprtName("QuickConversionReport.html");
        // tslint:disable-next-line:no-console
        console.log("Running conversion tests");
        const latestRoot = dsJson.imodel02.latest;
        const prevRoot = dsJson.imodel02.previous;
        const lhsFiles: string[] = glob.sync("**/*.bim", { cwd: latestRoot });
        const rhsFiles: string[] = glob.sync("**/*.bim", { cwd:  prevRoot});

        for (const lf of lhsFiles) {
            for (const rf of rhsFiles) {
                if (lf === rf) {
                    // tslint:disable-next-line:no-console
                    console.log(lf);
                    const lhs: IModelInfo = new IModelInfo(path.join(latestRoot, lf));
                    const rhs: IModelInfo = new IModelInfo(path.join(prevRoot, rf));
                    const comparer = new CompareIModels(lhs, rhs);
                    const compare = comparer.doCompare();
                    reporter.addResult(lf, compare, comparer.getMismatches(), path.join(latestRoot, lf), path.join(prevRoot, rf));
                    if (!compare) {
                        // tslint:disable-next-line:no-console
                        console.log("*** Issues in comparison");
                        pass = false;
                    } else {
                        // tslint:disable-next-line:no-console
                        console.log("Comparison is passing.");
                    }
                    lhs.closeIModel();
                    rhs.closeIModel();
                }
            }
        }
        reporter.generateHtmlReport();
        assert.isTrue(pass);
        // tslint:disable-next-line:no-console
        console.log("Report is QuickConversionReport.html");
    });
});
