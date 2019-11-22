/*---------------------------------------------------------------------------------------------
 * Adapted from https://github.com/atom/node-keytar
 * See LICENSE.md in the libsrc/keytar folder for license information. 
 *--------------------------------------------------------------------------------------------*/
import { iModelJsNative } from "./utils";
import { assert } from "chai";

const keytar = iModelJsNative.KeyTar;

describe("keytar", () => {
  const service = "keytar tests";
  const service2 = "other tests";
  const account = "buster";
  const password = "secret";
  const account2 = "buster2";
  const password2 = "secret2";

  before(function () {
    // Run only on Windows or MacOS
    if (process.platform !== "win32" && process.platform !== "darwin")
      this.skip();
  });

  beforeEach(async ()  => {
    await keytar.deletePassword(service, account);
    await keytar.deletePassword(service, account2);
    await keytar.deletePassword(service2, account);
  });

  afterEach(async () => {
    await keytar.deletePassword(service, account);
    await keytar.deletePassword(service, account2);
    await keytar.deletePassword(service2, account);
  });

  describe("setPassword/getPassword(service, account)", () => {
    it("sets and yields the password for the service and account", async () => {
      await keytar.setPassword(service, account, password);
      assert.equal(await keytar.getPassword(service, account), password);
      await keytar.setPassword(service, account, password2);
      assert.equal(await keytar.getPassword(service, account), password2);
    });

    it("yields null when the password was not found", async () => {
      assert.equal(await keytar.getPassword(service, account), null);
    });

    describe("Unicode support", () => {
      const locService = "se®vi\u00C7e";
      const locAccount = "shi\u0191\u2020ke\u00A5";
      const locPassword = "p\u00E5ssw\u00D8®\u2202";

      it("handles unicode strings everywhere", async () => {
        await keytar.setPassword(locService, locAccount, locPassword);
        assert.equal(await keytar.getPassword(locService, locAccount), locPassword);
      });

      afterEach(async () => {
        await keytar.deletePassword(locService, locAccount)
      });
    });
  });

  describe("deletePassword(service, account)", () => {
    it("yields true when the password was deleted", async () => {
      await keytar.setPassword(service, account, password);
      assert.equal(await keytar.deletePassword(service, account), true);
    });

    it("yields false when the password didn't exist", async () => {
      assert.equal(await keytar.deletePassword(service, account), false);
    });
  });

  describe("findPassword(service)", () => {
    it("yields a password for the service", async () => {
      await keytar.setPassword(service, account, password);
      await keytar.setPassword(service, account2, password2);
      assert.include([password, password2], await keytar.findPassword(service));
    });

    it("yields null when no password can be found", async () => {
      assert.equal(await keytar.findPassword(service), null);
    });
  });

  describe("findCredentials(service)", () => {
    it("yields an array of the credentials", async () => {
      await keytar.setPassword(service, account, password);
      await keytar.setPassword(service, account2, password2);
      await keytar.setPassword(service2, account, password);

      const found = await keytar.findCredentials(service)
      const sorted = found.sort((a, b) => {
        return a.account.localeCompare(b.account);
      });

      assert.deepEqual([{account, password}, {account: account2, password: password2}], sorted);
    });

    it("returns an empty array when no credentials are found", async () => {
      const accounts = await keytar.findCredentials(service)
      assert.deepEqual([], accounts)
    });

    describe("Unicode support", () => {
      const locService = "se®vi\u00C7e";
      const locAccount = "shi\u0191\u2020ke\u00A5";
      const locPassword = "p\u00E5ssw\u00D8®\u2202";

      it("handles unicode strings everywhere", async () => {
        await keytar.setPassword(locService, locAccount, locPassword);
        await keytar.setPassword(locService, account2, password2);

        const found = await keytar.findCredentials(locService);
        const sorted = found.sort((a, b) => {
          return a.account.localeCompare(b.account);
        });

        assert.deepEqual([{account: account2, password: password2}, {account: locAccount, password: locPassword}], sorted);
      });

      afterEach(async () => {
        await keytar.deletePassword(locService, locAccount)
      });
    });
  });
});
