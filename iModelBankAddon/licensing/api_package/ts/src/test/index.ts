/*---------------------------------------------------------------------------------------------
* Copyright (c) 2019 Bentley Systems, Incorporated. All rights reserved.
* Licensed under the MIT License. See LICENSE.md in the project root for license terms.
*--------------------------------------------------------------------------------------------*/
import { logTest, loadAddon, it, tests } from "./utils";
import { assert } from "chai";
import { Logger } from "@bentley/bentleyjs-core";
import * as dbTest from "./dbTest";
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
    try {
        (addon as any).checkEntitlement();
        assert.fail();
    } catch (_err) {
        // expected
    }
    try {
        (addon as any).checkEntitlement(1);
        assert.fail();
    } catch (_err) {
        // expected
    }

    addon.checkEntitlement("/tmp/licensefile");
});

for (const test of tests) {
    test();
}

dbTest.intialize();

logTest("Finished imodel-bank-licensing tests");
