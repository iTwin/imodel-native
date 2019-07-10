
import { expect } from "chai";
import * as fs from 'fs-extra';
import { CmdUtil } from "../src/cmd";
import * as child_process from 'child_process';

const MicrostationBuildNumber = 622;
const MicrostationBuidlName = "iModelBridgeMstnx64";
const RevitBuildNumber = 350;
const RevitBuildName = "iModelBridgeService-Revit";

function GetTempFolder () : string
    {
    let testRunDirectory = process.env.TestRunDirectory;
    if (testRunDirectory)
        return testRunDirectory as string;
        
    return fs.mkdtempSync("MixedBridgeTest");
    }

describe("Microstation and Revit Mixed Bridge Test", function () {
    // Download the msi files
    const tempFolder = GetTempFolder();
    const extractionFolder = tempFolder + "OutPutZip\\";
    const iModelBankDir = tempFolder + "iModelBank\\";
    const iModelTestDir = tempFolder + "TestData\\";

    const mstnBridgeDir = tempFolder + "OutputMsi\\Bentley\\iModelBridgeMstn\\";
    const revitBridgeDir = tempFolder + "OutputMsi\\Bentley\\RevitBridge\\";
    const stagingDir = tempFolder + "StagingDir\\"
    let iModelBankProcess: child_process.ChildProcess;
    after(async function () {
        if (iModelBankProcess && !iModelBankProcess.killed)
            await iModelBankProcess.kill();
    });
    context("Setting up the test environment", function () {
        it("Download the Microstation msi file", async () => {
            let downloadSuccess = await CmdUtil.downloadFile(tempFolder, extractionFolder, MicrostationBuildNumber, MicrostationBuidlName);
            expect(downloadSuccess);
        });

        it("Download the Revit msi file", async () => {
            let downloadSuccess = await CmdUtil.downloadFile(tempFolder, extractionFolder, RevitBuildNumber, RevitBuildName);
            expect(downloadSuccess);
        });
        it("Extract Microstation Msi files", (done) => {
            CmdUtil.extractMsi(extractionFolder + "iModelBridgeMstnx64\\iModelBridgeMstnx64.msi", tempFolder + "OutputMsi\\", "iModelBridgeMstn", done);
        });
        it("Extract Revit Msi files", (done) => {
            CmdUtil.extractMsi(extractionFolder + "iModelBridgeService-Revit\\RevitBridgex64.msi", tempFolder + "OutputMsi\\", "RevitBridge", done);
        });
        it("Boot strap a new iModel Bank FS", async function () {
            await CmdUtil.SetUpiModelBankFileSystem(iModelBankDir);
            let outputDirStatus = await fs.pathExists(iModelBankDir);
            expect(outputDirStatus);
        });
        it("Start iModelBank ", async function () {
            iModelBankProcess = await CmdUtil.startiModelBank(iModelBankDir);
            iModelBankProcess.stdout.pipe(process.stdout);
            expect(0 != iModelBankProcess.pid);
            expect(iModelBankProcess.connected);

        });
        it("Get Test Data ", async function () {
            const dgnTestUrl = "https://imodelbridgetestdata.file.core.windows.net/lofts/MixedBridgeTests/Microstation"
            await CmdUtil.downloadAzureDirectory(iModelTestDir + "Dgn\\", dgnTestUrl);

            const revitUrl = "https://imodelbridgetestdata.file.core.windows.net/lofts/MixedBridgeTests/Revit"
            await CmdUtil.downloadAzureDirectory(iModelTestDir + "Revit\\", revitUrl);
        });
    });

    class MstnJob implements CmdUtil.IArgDetails {
        GetArguments(): string[] {
            let args: string[] = new CmdUtil.BankConfig().GetArguments();
            const mstnInput = iModelTestDir + "Dgn\\Metrostation\\MetroStation\\DGN\\3DModel\\Master.dgn"
            args.push("--fwk-input=" + mstnInput)
            return args;
        }

    }
    class RevitJob implements CmdUtil.IArgDetails {
        GetArguments(): string[] {
            let args: string[] = new CmdUtil.BankConfig().GetArguments();
            const mstnInput = iModelTestDir + "Revit\\sample\\rac_basic_sample_project.rvt"
            args.push("--fwk-input=" + mstnInput)
            return args;
        }

    }
    context("Running the test ", function () {
        it("Run the bridge", async function () {

            let mstnBridgeProc = CmdUtil.RunBridge(mstnBridgeDir, "Dgnv8BridgeB02.dll", stagingDir, new MstnJob());
            let revitBridgeProc = CmdUtil.RunBridge(revitBridgeDir, "RevitBridge.dll", stagingDir, new RevitJob());
            let results = await Promise.all([mstnBridgeProc, revitBridgeProc]);
            expect(0 == results[0])
            expect(0 == results[1])
        });
        it("Validate the output", function () {
        });
    });
});

    // Start iModel Bank with 
    // Run a bridge
    // Validate

