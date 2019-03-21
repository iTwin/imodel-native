import * as child_process from 'child_process';
import * as AdmZip from 'adm-zip';
import * as bd from "../src/buildArtifact"
import * as fs from "fs-extra";
import { SharedKeyCredential, FileURL, Aborter, DirectoryURL } from '@azure/storage-file';
import { FileItem, DirectoryItem } from '@azure/storage-file/typings/lib/generated/lib/models';

export module CmdUtil {
    /**
     * Creates a child process with script path
     * @param {string} lprocessPath Path of the process to execute
     * @param {Array} args Arguments to the command
     * @param {Object} env (optional) Environment variables
     */
    export async function createProcess(lprocessPath: string, args?: string[], opts?: child_process.ExecFileOptions) {
        // Ensure that path exists
        if (!lprocessPath) {
            throw new Error('Invalid process path');
        }

        return child_process.spawn(lprocessPath, args, opts);
    }

    /**
     * Extract a zip file to path
     * @param {string} zipFilePath Path of the process to execute
     * @param {string} targetDir Directory into which we need to extract the contents
     */
    export function extractZipFile(zipFilePath: string, targetDir: string) {
        var zip = new AdmZip(zipFilePath);
        zip.extractAllTo(targetDir);
    }

    const azAccount = "imodelbridgetestdata";
    const accountKey = "kDiqUEQbqwGw4Ur4XXhw7ocRQ+/iD0iwyuEsdO7HViBMntHmEk/kXkKWMZ996vebaVqYIJ1bt2z5gScxhM8sjQ==";
    const sharedKeyCredential = new SharedKeyCredential(azAccount, accountKey);
    /**
     * Download a file locally from azure storage.
     * @param localFileName Location on the file system where to store the downloaded file
     * @param inputUrl Url into the azure storage location
     */
    export async function downloadAzureFile(localFileName: string, inputUrl: string) {

        const pipeLine = FileURL.newPipeline(sharedKeyCredential);
        let url = new FileURL(inputUrl, pipeLine);
        let resp = await url.download(Aborter.none, 0);
        const fileStream = fs.createWriteStream(localFileName);
        if (resp.readableStreamBody)
            resp.readableStreamBody.pipe(fileStream);
        return new Promise<boolean>(function (resolve, reject) {
            fileStream.on("close", () => {
                //console.log(`Url '${inputUrl}' downloaded to ${localFileName}`);
                resolve(true);
                fileStream.on("error", () => {
                    reject();
                });
            });
        });
    }

    /**
     * Download the file storage directory into the local directory.
     * @param localDirName Directory into which to dowload the files
     * @param inputUrl Url for the input directory
     */
    export async function downloadAzureDirectory(localDirName: string, inputUrl: string) {
        const pipeLine = DirectoryURL.newPipeline(sharedKeyCredential);
        let url = new DirectoryURL(inputUrl, pipeLine);
        let marker = undefined;
        //let outerPromise  : Promise <boolean []> [] =  [];
        let dirPromise: Promise<any>[] = [];
        do {
            let response = await url.listFilesAndDirectoriesSegment(Aborter.none, marker);
            let filePromises: Promise<boolean>[] = [];
            response.segment.fileItems.forEach((value: FileItem) => {
                const fileName = value.name;
                filePromises.push(downloadAzureFile(localDirName + fileName, inputUrl + "\\" + fileName));
            });
            response.segment.directoryItems.forEach((value: DirectoryItem) => {
                const dirName = value.name;
                const newDirName = localDirName + dirName + "\\";
                fs.ensureDirSync(newDirName);
                let promiseList = downloadAzureDirectory(newDirName, inputUrl + "/" + dirName);
                dirPromise.push(promiseList)
            });
            // for (const item of response.segment.fileItems) {
            //     filePromises.push(downloadAzureFile (localDirName + item.name, inputUrl+"\\"+item.name));
            //}
            //marker = response.
        } while (marker)

        return Promise.all(dirPromise);
    }

    /**
     * Downloads the latest 
     * @param {string} tempFolder Directory into which we need to download the build artifact.
     * @param {string} extractionFolder Directory into which we need to unzip the build artifact.
     * @param {number} buildNumber The VSTS build number whose artifact we need to download
     */
    export async function downloadFile(tempFolder: string, extractionFolder: string, buildNumber: number, buildName: string) {
        let msiZipfileName = tempFolder + buildName + ".zip";
        try {
            if (!fs.existsSync(msiZipfileName)) {
                let result = await bd.VSTSHelper.downloadBuild(buildNumber, buildName, msiZipfileName);
                if (!result) {
                    console.error("Unable to download the file.");
                    return false;
                }
            }
            else
                console.log("Reusing already downloaded msi file.");
            extractZipFile(msiZipfileName, extractionFolder);
            return true;
        }
        catch (err) {
            console.error(`Error: ${err.stack}`);
            return false;
        }

    }
    /**
     * Starts a process the extracts the contents of the msi file to the given output folder
     * @param msiName Name of the msi file to extract
     * @param outputFolder Path to where we should extract the contents
     */
    export async function extractMsi(msiName: string, outputFolder: string, programFolderName: string, callback?: any) {
        let msiExtractProcessResult = await createProcess("msiexec", ["/a", msiName, "/qn", "TARGETDIR=" + outputFolder]);
        msiExtractProcessResult.stdout.pipe(process.stdout);
        msiExtractProcessResult.on('exit', async () => {
            //Copy the contents of system64 to Program folder
            await fs.copy(outputFolder + "System64\\", outputFolder + "Bentley\\" + programFolderName + "\\", { overwrite: true });
            //let ncpInstance = Ncp.ncp;
            //ncpInstance()
            callback();
        });
    }

    const iModelBankGuid = "233e1f55-561d-42a4-8e80-d6f91743863e";
    /**
     * Starts the imodel Bank listener
     * @param iModelBankDir The iModelFilesystem from which we will be running this test
     */
    export async function startiModelBank(iModelBankDir: string) {


        return createProcess('node', ["node_modules\\@bentley\\imodel-bank-server\\lib\\runWebServer.js", iModelBankDir + iModelBankGuid + "\\", "src\\config\\server.config.json", "src\\config\\logging.config.json"])
    }

    export async function SetUpiModelBankFileSystem(iModelBankDir: string) {
        const iModelBankSeedFileUrl = "https://imodelbridgetestdata.file.core.windows.net/lofts/MixedBridgeTests/iModelBankData/ReadOnlyTest.bim";
        let iModelBankSeedFile = iModelBankDir + iModelBankGuid + "\\ReadOnlyTest.bim";
        await fs.ensureDir(iModelBankDir + iModelBankGuid + "\\");
        let seedFileExist = fs.existsSync(iModelBankSeedFile)
        if (!seedFileExist) {
            await downloadAzureFile(iModelBankSeedFile, iModelBankSeedFileUrl);
        }
        return fs.copy("src\\config\\imodelfs", iModelBankDir);
    }

    export interface  IArgDetails {
        GetArguments(): string[];

    }
    export class BankConfig implements IArgDetails { 
        GetArguments(): string[] {
            let args: string[] = [];
            args.push("--imodel-bank-imodel-id=233e1f55-561d-42a4-8e80-d6f91743863e");
            args.push("--imodel-bank-access-token=eyJGb3JlaWduUHJvamVjdEFjY2Vzc1Rva2VuIjp7InVzZXJJbmZvIjp7ImlkIjoiam9iMSB1c2VyIn19fQ==");
            args.push("--imodel-bank-url=http://localhost:4000");
            return args;
        }
    }

    export class HubConfig implements IArgDetails {
        GetArguments(): string[] {
            let args: string[] = [];
            //--server-user=abeesh.basheer@bentley.com
            //--server-password=Tt2~PG[u
            //--server-repository="AB_Drexel_Calhoun"
            //--server-project="Mott_EAP_Test"
            //--server-environment=QA
            return args;
        }
    }
    export async function RunBridge(bridgeDir: string, bridgeDllName: string, stagingDir: string, jobDetails :IArgDetails) {
        const iModelBridgeFwkExe = bridgeDir + "iModelBridgeFwk.exe"
        let args: string[] = [];
        stagingDir = stagingDir +bridgeDllName+"\\";
        fs.ensureDirSync(stagingDir);
        args.push("--fwk-staging-dir=" + stagingDir);
        args.push("--fwk-bridge-library=" + bridgeDir + bridgeDllName);
        args.push("--fwk-skip-assignment-check");
        args = args.concat(jobDetails.GetArguments());
        let bridgeProc =  await createProcess(iModelBridgeFwkExe, args);
        //bridgeProcPromise.stdout.pipe(process.stdout);
        //bridgeProcPromise.stderr.pipe(process.stderr);
        return new Promise<number> (function(resolve, reject) {
            bridgeProc.on('exit', (code: number) => {
                if (0 != code)
                    reject(Error("Error code = "+code.toString()));
                else
                    resolve(code);
            });
            // bridgeProc.stdout.on('data', (data) => {
            //     console.log(data.toString())
            // });
            bridgeProc.stderr.on('data', (data) => {
                console.log(data.toString())
            });
        });
    }

}



