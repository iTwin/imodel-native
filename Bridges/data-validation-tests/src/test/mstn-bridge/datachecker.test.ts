import { assert } from "chai";
import { CompareIModels } from "@bentley/data-validation-api/lib/compareIModels";
import { DatasetManager } from "../../datasetmanager";
import { IModelInfo } from "@bentley/data-validation-api/lib/imodelinfo";
import { Reporter } from "@bentley/data-validation-api/lib/reporter";
import * as path from "path";
import * as glob from "glob";

describe("DataValidationTests", () => {
    it("MSTN Bridge Smoke compare", () => {
        let pass = true;
        const dsManager = new DatasetManager("mstnbridge.dataset.json");
        const ver = dsManager.ver;
        const reporter = new Reporter("././lib");
        const dataset: any[] = [];
        // tslint:disable-next-line:no-console
        console.log("Processing latest bim files.");
        const lhsFiles: string[] = glob.sync("**/*.bim", { cwd: dsManager.latestRoot });
        const rhsFiles: string[] = glob.sync("**/*.bim", { cwd: dsManager.previousRoot });
        const baseFiles: string[] = glob.sync("**/*.bim", { cwd: dsManager.baseRoot });

        for (const file of lhsFiles) {
            const fullPath = path.join(dsManager.latestRoot, file);
            const info = new IModelInfo(fullPath);
            const ds = dsManager.matchDataset(info);
            if (ds) {
                dataset.push({
                    name: ds,
                    lhsPath: fullPath,
                    rhsPath: "",
                    basePath: "",
                });
            }
            info.closeIModel();
        }
        // tslint:disable-next-line:no-console
        console.log("Processing previous bim files.");
        for (const file of rhsFiles) {
            const fullPath = path.join(dsManager.previousRoot, file);
            const info = new IModelInfo(fullPath);
            const ds = dsManager.matchDataset(info);
            if (ds) {
                for (const dt of dataset) {
                    if (dt.name === ds) {
                        dt.rhsPath = fullPath;
                    }
                }
            }
            info.closeIModel();
        }
        // tslint:disable-next-line:no-console
        console.log("Processing base bim files.");
        for (const file of baseFiles) {
            const fullPath = path.join(dsManager.baseRoot, file);
            const info = new IModelInfo(fullPath);
            const ds = dsManager.matchDataset(info);
            if (ds) {
                for (const dt of dataset) {
                    if (dt.name === ds) {
                        dt.basePath = fullPath;
                    }
                }
            }
            info.closeIModel();
        }
        // tslint:disable-next-line:no-console
        console.log("\n Comparing Latest files with Previous.");
        for (const dt of dataset) {
            // tslint:disable-next-line:no-console
            console.log("\nChecking dataset: \"" + dt.name + "\"");
            if (dt.lhsPath && dt.rhsPath && dt.basePath) {
                const lhs: IModelInfo = new IModelInfo(dt.lhsPath);
                const rhs: IModelInfo = new IModelInfo(dt.rhsPath);
                const base: IModelInfo = new IModelInfo(dt.basePath);
                const comparer = new CompareIModels(lhs, rhs);
                let compare = comparer.doCompareWithBase(base);
                const mms = comparer.getMismatches();
                let mml = mms.length;
                while (mml--) {
                    const mm = mms[mml];
                    if (mm.name.startsWith("ElementsInSubject") && mm.name.includes(ver)) {
                        mms.splice(mml, 1);
                    } 
                }        
                if (mms.length === 0)
                    compare = true;
                reporter.addResult(dt.name, compare, mms, dt.lhsPath, dt.rhsPath, dt.basePath);
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
                base.closeIModel();
            } else {
                // tslint:disable-next-line:no-console
                console.log("Matching iModels could not be found");
                reporter.addResult(dt.name, false, [""], dt.lhsPath, dt.rhsPath, dt.basePath);
                pass = false;
            }
        }
        reporter.generateHtmlReport();
        assert.isTrue(pass);
        // tslint:disable-next-line:no-console
        console.log("Report is data-validation-report.html");
    });
});
