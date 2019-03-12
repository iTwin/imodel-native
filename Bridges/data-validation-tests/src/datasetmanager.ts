import { IModelInfo } from "@bentley/data-validation-api/lib/imodelinfo";
import * as path from "path";
import * as fs from "fs";

export class DatasetManager {
    public dataset: any[];
    public dataRoot: string;
    public latestRoot: string;
    public previousRoot: string;
    public baseRoot: string;
    public ver: string;
    public constructor(jsonFile: string) {
        const datasetStr = fs.readFileSync(path.join("./lib/config", jsonFile), "utf8");
        const datasetJson: any = JSON.parse(datasetStr);
        this.dataset = datasetJson.ds;
        this.dataRoot = datasetJson.root;
        this.latestRoot = path.join(this.dataRoot, datasetJson.latest);
        this.previousRoot = path.join(this.dataRoot, datasetJson.previous);
        this.baseRoot = path.join(this.dataRoot, datasetJson.base);
        this.ver = datasetJson.version;
    }
    public matchDatasetMulti(info: IModelInfo) {
        let ds = "";
        const dtsts = this.dataset;
        const partitions = info.getPartitionNames();
        // console.log(partitions);
        for (const dtst of dtsts) {
            let found = false;
            if (partitions.length === dtst.markerPartitions.length) {
                for (const marker of dtst.markerPartitions) {
                    found = false;
                    if (partitions.find((x) => x === marker))
                        found = true;
                }
            }
            if (found)
                ds = dtst.name;
        }
        return ds;
    }
    public matchDataset(info: IModelInfo) {
        let ds = "";
        const dtsts = this.dataset;
        const v8Files = info.getV8Names();
        for (const dtst of dtsts) {
            let found = false;
            const markerFiles = dtst.markerFiles;
            for (const markerFile of markerFiles) {
                found = false;
                let fName = path.parse(markerFile).name;
                if (fName.includes("_")) { // potentially has bridge version in the name
                    const nP = fName.split("_");
                    const toRemove = nP[nP.length - 1].length;
                    fName = fName.slice(0, fName.length - toRemove);
                }
                for (const v8File of v8Files) {
                    if (v8File.startsWith(fName))
                        found = true;
                }
            }
            if (found)
                ds = dtst.name;
        }
        return ds;
    }

}
