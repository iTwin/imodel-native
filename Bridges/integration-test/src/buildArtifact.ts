// A sample showing how to list VSTS build artifacts, and how to download a zip of a VSTS build artifact.
import * as cm from "./common";
import * as vm from "azure-devops-node-api";
import * as fs from "fs";

import * as ba from "azure-devops-node-api/BuildApi";
import * as bi from "azure-devops-node-api/interfaces/BuildInterfaces";

export module VSTSHelper {
    export async function downloadBuild(buildDefNumber: number, artifactToDownload: string, zipFileName: string): Promise<boolean> {
        try {
            const vsts: vm.WebApi = await cm.getWebApi();
            const vstsBuild: ba.IBuildApi = await vsts.getBuildApi();

            cm.banner("Downloading MSI files");
            const project = cm.getProject();
            console.log("project", project);

            // get the latest successful build.
            cm.heading(`Get latest successful build for ${project} project`);
            const builds: bi.Build[] = await vstsBuild.getBuilds(
                project,
                [buildDefNumber],                       // definitions: number[]
                undefined,                       // queues: number[]
                undefined,                       // buildNumber
                undefined, //new Date(2016, 1, 1),       // minFinishTime
                undefined,                       // maxFinishTime
                undefined,                       // requestedFor: string
                bi.BuildReason.All,         // reason
                bi.BuildStatus.Completed,
                bi.BuildResult.Succeeded,
                undefined,                       // tagFilters: string[]
                undefined,                       // properties: string[]
                //bi.DefinitionType.Build,
                1                           // top: number
            );

            if (builds.length > 0) {
                const latestBuild: bi.Build = builds[0];
                console.log(`build ${latestBuild.id}`);

                // Retrieve the list of artifacts for the latest build.
                cm.heading(`Artifacts for build ${latestBuild.id}, ${project} project`);
                const artifacts: bi.BuildArtifact[] = await vstsBuild.getArtifacts(latestBuild.id as number, project);

                let downloadableArtifactName = '';
                for (const artifact of artifacts) {
                    if (!artifact.resource)
                        continue;

                    let additionalInfo = "";
                    if (artifact.resource.type === "FilePath") {
                        additionalInfo = `UNC Path: ${artifact.resource.data}`;
                    }
                    else if (artifact.resource.type === "Container") {
                        // As of June 2018, only `Container` artifacts can be downloaded as a zip.
                        additionalInfo = `Downloadable: true.`;
                        if (artifact.name == artifactToDownload) {
                            downloadableArtifactName = artifact.name;
                            break;
                        }
                    }
                    console.log(`Artifact: '${artifact.name}'. Type: ${artifact.resource.type}. Id: ${artifact.id}. ${additionalInfo}`);
                }

                // Download an artifact.
                if (latestBuild.id) {
                    cm.heading(`Download zip of artifact '${downloadableArtifactName}' for build ${latestBuild.id}, ${project} project`);
                    const artifactStream: NodeJS.ReadableStream = await vstsBuild.getArtifactContentZip(latestBuild.id, downloadableArtifactName, project);

                    //const path = downloadDir + `${downloadableArtifact.name}.zip`;
                    const fileStream = fs.createWriteStream(zipFileName);
                    artifactStream.pipe(fileStream);
                    return new Promise<boolean>(function (resolve, reject) {
                        fileStream.on("close", () => {
                            console.log(`Artifact '${downloadableArtifactName}' downloaded to ${zipFileName}`);
                            resolve(true);
                            fileStream.on("error", () => {
                                reject();
                            });
                        });
                    });
                }
                else {
                    console.log("No downloadable artifact found.");
                    throw Error("No downloadable artifact found.");
                }
            }
            else {
                console.log("No successful builds found.");
                throw Error("No successful builds found.");
            }
            throw Error("Error downloading the file");
        }
        catch (err) {
            console.error(`Error: ${err.stack}`);
            throw err;
        }
    }
}