/*---------------------------------------------------------------------------------------------
* Copyright (c) 2019 Bentley Systems, Incorporated. All rights reserved.
* Licensed under the MIT License. See LICENSE.md in the project root for license terms.
*--------------------------------------------------------------------------------------------*/
import * as path from "path";
import { logTest, loadAddon, it, tests } from "./utils";
import { assert } from "chai";
import { Logger } from "@bentley/bentleyjs-core";
import * as dbTest from "./dbTest";
import { KnownTestLocations } from "./KnownTestLocations";
import { IModelBankLicensingNative } from "../IModelBankLicensingNative";
import { IModelBankLicensingNativeHost } from "../IModelBankLicensingNativeHost";

// Run the tests
IModelBankLicensingNativeHost.addon = loadAddon();

const addon: typeof IModelBankLicensingNative = IModelBankLicensingNativeHost.addon;
addon.logger = Logger;

logTest("Start imodel-bank-licensing tests");

it("should log", () => {
    Logger.logInfo("cat", "message");
});

it("should verify addon API", () => {
    assert.isTrue(addon !== undefined);
    assert.isTrue(addon.hasOwnProperty("version"));
    assert.isString(addon.version);

    assert.isTrue(addon.hasOwnProperty("logger"));
    assert.strictEqual(addon.logger, Logger);

    assert.isTrue(addon.hasOwnProperty("checkEntitlement"));
    assert.isFunction(addon.checkEntitlement);
});

it("should call checkEntitlement", () => {
    const licensePath = path.join(__dirname, "../../../../", "License.belic");
    addon.setup(KnownTestLocations.outputDir, licensePath, "iModelBankTest");
    addon.checkEntitlement("iModelId", "activityId", "0");
});

for (const test of tests) {
    test();
}

dbTest.intialize();

logTest("Finished imodel-bank-licensing tests");
