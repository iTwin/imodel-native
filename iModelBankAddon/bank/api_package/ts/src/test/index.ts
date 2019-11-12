/*---------------------------------------------------------------------------------------------
* Copyright (c) 2019 Bentley Systems, Incorporated. All rights reserved.
* Licensed under the MIT License. See LICENSE.md in the project root for license terms.
*--------------------------------------------------------------------------------------------*/
import { logTest, loadAddon } from "./utils";
import { assert } from "chai";
import { Logger } from "@bentley/bentleyjs-core";
import { runTests as runDbTests } from "./dbTest";
import { IModelBankNative } from "../IModelBankNative";
import { IModelBankNativeHost } from "../IModelBankNativeHost";

// Run the tests
IModelBankNativeHost.addon = loadAddon();

const addon: typeof IModelBankNative = IModelBankNativeHost.addon;
addon.logger = Logger;

logTest("Start imodel-bank tests");

assert.isTrue(addon !== undefined);
assert.isTrue(addon.hasOwnProperty("version"));
assert.isString(addon.version);

assert.isTrue(addon.hasOwnProperty("logger"));
assert.strictEqual(addon.logger, Logger);

assert.isTrue(addon.hasOwnProperty("doDeferredLogging"));
assert.isFunction(addon.doDeferredLogging);

runDbTests();

// Give the license check a chance to succeed.
setTimeout(() => {
    addon.doDeferredLogging();
    logTest("Finished imodel-bank tests");
}, 3000);
