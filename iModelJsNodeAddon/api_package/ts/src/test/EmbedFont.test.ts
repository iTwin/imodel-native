/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import * as path from "path";
import * as fs from "fs-extra";
import * as os from "os";
import { copyFile, dbFileName, getAssetsDir, getOutputDir, iModelJsNative } from "./utils";
import { expect } from "chai";
import { OpenMode } from "@itwin/core-bentley";
import { FontType } from "@itwin/core-common";
import { IModelJsNative } from "../NativeLibrary";

function getFontPath(fontFileName: string): string {
  return path.join(getAssetsDir(), "Fonts", fontFileName);
}

describe("embed fonts", () => {
  const shxFileName = getFontPath("Cdm.shx");
  const ttfFileName = getFontPath("Karla-Regular.ttf");
  const dummyRsc: IModelJsNative.EmbedFontArg = {
    face: {
      familyName: "dummy font",
      type: FontType.Rsc,
      faceName: "regular",
      encoding: {
        codePage: 100,
        degree: 21,
        diameter: 22,
        plusMinus: 23,
      },
    },
    data: new Uint8Array([100, 2, 233, 200]),
  };

  it("embed fonts in SQLiteDb", () => {
    const tempSQLiteDbName = path.join(getOutputDir(), "fontWs.db");
    const tempSQLiteDb = new iModelJsNative.SQLiteDb();

    if (fs.existsSync(tempSQLiteDbName))
      fs.removeSync(tempSQLiteDbName);
    tempSQLiteDb.createDb(tempSQLiteDbName);

    if (process.platform !== "darwin") {
      // This is failing on CI Macs, so disabling it for now.
      tempSQLiteDb.embedFont(dummyRsc);
    }
    tempSQLiteDb.embedFont({ fileName: shxFileName });
    tempSQLiteDb.embedFont({ fileName: ttfFileName });
    if (os.platform() === "win32") // Embedding system fonts is only supported on windows.
      tempSQLiteDb.embedFont({ systemFont: "Times New Roman" });

    // Currently, embedding rights are not enforced for workspace dbs.
    tempSQLiteDb.embedFont({
      fileName: getFontPath("Karla-Restricted.ttf"),
      compress: true,
    });

    tempSQLiteDb.embedFont({
      fileName: getFontPath("Karla-Preview-And-Print.ttf"),
      compress: true,
    });

    tempSQLiteDb.closeDb();
  });

  it("embed fonts in DgnDb", () => {
    const tempDgnDb = new iModelJsNative.DgnDb();
    const tempDgnDbName = copyFile("testEmbedFont.bim", dbFileName);
    tempDgnDb.openIModel(tempDgnDbName, OpenMode.ReadWrite);

    if (process.platform !== "darwin") {
      // This is failing on CI Macs, so disabling it for now.
      tempDgnDb.embedFont({ ...dummyRsc, compress: true });
    }
    tempDgnDb.embedFont({ fileName: shxFileName, compress: true });
    tempDgnDb.embedFont({ fileName: ttfFileName, compress: true });
    if (os.platform() === "win32") // Embedding system fonts is only supported on windows.
      tempDgnDb.embedFont({ systemFont: "times new roman", compress: true });

    // We prohibit embedding restricted and print-and-preview fonts in iModels.
    expect(tempDgnDb.embedFont({
      fileName: getFontPath("Karla-Restricted.ttf"),
      compress: true,
    })).to.throw("unable to embed font");

    expect(tempDgnDb.embedFont({
      fileName: getFontPath("Karla-Preview-And-Print.ttf"),
      compress: true,
    })).to.throw("unable to embed font");

    tempDgnDb.closeFile();
  });
});
