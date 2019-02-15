import { assert } from "chai";
import { CompareIModels } from "@bentley/data-validation-api/lib/compareIModels";
import { DatasetManager } from "../../datasetmanager";
import { IModelInfo } from "@bentley/data-validation-api/lib/imodelinfo";
import { Reporter } from "@bentley/data-validation-api/lib/reporter";
import * as path from "path";
import * as glob from "glob";

describe("DataValidationTests", () => {
    it("Multi Bridge Smoke compare", () => {
        let pass = true;
        const dsManager = new DatasetManager("multibridge.dataset.json");
        const reporter = new Reporter("././lib");
        const latestRoot = path.join(dsManager.dataRoot, "BridgeOutputLatest");
        const prevRoot = path.join(dsManager.dataRoot, "BridgeOutputPrevious");
        const baseRoot = path.join(dsManager.dataRoot, "BridgeOutputBaselines");
        const lhsFiles: string[] = glob.sync("**/*.bim", { cwd: latestRoot });
        const rhsFiles: string[] = glob.sync("**/*.bim", { cwd: prevRoot });
        const baseFiles: string[] = glob.sync("**/*.bim", { cwd: baseRoot });
        const dataset: any[] = [];
        let found = false;
        // tslint:disable-next-line:no-console
        console.log("Processing latest bim files.");
        for (const file of lhsFiles) {
            const fullPath = path.join(latestRoot, file);
            const info = new IModelInfo(fullPath);
            const ds = dsManager.matchDatasetMulti(info);
            if (ds === "All") {
                found = true;
                dataset.push({
                    name: ds,
                    lhsPath: fullPath,
                    rhsPath: "",
                    basePath: "",
                });
            }
            info.closeIModel();
            if (found)
                break;
        }
        found = false;
        // tslint:disable-next-line:no-console
        console.log("Processing previous bim files.");
        for (const file of rhsFiles) {
            const fullPath = path.join(prevRoot, file);
            const info = new IModelInfo(fullPath);
            const ds = dsManager.matchDatasetMulti(info);
            if (ds === "All") {
                found = true;
                const dt = dataset.find((x) => x.name === ds);
                if (dt)
                    dt.rhsPath = fullPath;
                info.closeIModel();
            }
            if (found)
                break;
        }
        found = false;
        // tslint:disable-next-line:no-console
        console.log("Processing base bim files.");
        for (const file of baseFiles) {
            const fullPath = path.join(baseRoot, file);
            const info = new IModelInfo(fullPath);
            const ds = dsManager.matchDatasetMulti(info);
            if (ds === "All") {
                found = true;
                const dt = dataset.find((x) => x.name === ds);
                if (dt)
                    dt.basePath = fullPath;
                info.closeIModel();
            }
            if (found)
                break;
        }
        // tslint:disable-next-line:no-console
        console.log("\n Comparing iModels...");
        for (const dt of dataset) {
            // tslint:disable-next-line:no-console
            console.log("\nChecking dataset: \"" + dt.name + "\"");
            if (dt.lhsPath && dt.rhsPath && dt.basePath) {
                const lhs: IModelInfo = new IModelInfo(dt.lhsPath);
                const rhs: IModelInfo = new IModelInfo(dt.rhsPath);
                const base: IModelInfo = new IModelInfo(dt.basePath);
                const comparer = new CompareIModels(lhs, rhs);
                const compare = comparer.doCompareWithBase(base);
                reporter.addResult(dt.name, compare, comparer.getMismatches(), dt.lhsPath, dt.rhsPath, dt.basePath);
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
