/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import * as path from "path";
import * as fs from "fs-extra";
import * as os from "os";
import { copyFile, dbFileName, getAssetsDir, getOutputDir, iModelJsNative } from "./utils";
import { OpenMode } from "@itwin/core-bentley";
import { FontType } from "@itwin/core-common";
import { IModelJsNative } from "../NativeLibrary";

describe("embed fonts", () => {
  const shxFileName = path.join(getAssetsDir(), "Fonts", "Cdm.shx");
  const ttfFileName = path.join(getAssetsDir(), "Fonts", "Karla-Regular.ttf");
  const dummyRsc: IModelJsNative.EmbedFontArg = {
    face: {
      familyName: "dummy font",
      type: FontType.Rsc,
      faceName: "regular",
      encoding: {
        codePage: 100,
        degree: 21,
        diameter: 22,
        plusMinus: 23
      }
    },
    data: new Uint8Array([100, 2, 233, 200])
  };

  it("embed fonts in SQLiteDb", () => {
    const tempSQLiteDbName = path.join(getOutputDir(), "fontWs.db");
    const tempSQLiteDb = new iModelJsNative.SQLiteDb();

    if (fs.existsSync(tempSQLiteDbName))
      fs.removeSync(tempSQLiteDbName);
    tempSQLiteDb.createDb(tempSQLiteDbName);

    tempSQLiteDb.embedFont(dummyRsc);
    tempSQLiteDb.embedFont({fileName: shxFileName});
    tempSQLiteDb.embedFont({fileName: ttfFileName});
    if (os.platform() === "win32") // Embedding system fonts is only supported on windows.
      tempSQLiteDb.embedFont({systemFont: "Times New Roman"});

    tempSQLiteDb.closeDb();
  });

  it("embed fonts in DgnDb", () => {
    const tempDgnDb = new iModelJsNative.DgnDb();
    const tempDgnDbName = copyFile("testEmbedFont.bim", dbFileName);
    tempDgnDb.openIModel(tempDgnDbName, OpenMode.ReadWrite);

    tempDgnDb.embedFont({...dummyRsc, compress:true});
    tempDgnDb.embedFont({fileName: shxFileName, compress: true});
    tempDgnDb.embedFont({fileName: ttfFileName, compress: true});
    if (os.platform() === "win32") // Embedding system fonts is only supported on windows.
      tempDgnDb.embedFont({systemFont: "times new roman", compress: true});
    
      tempDgnDb.closeIModel();
  });
});
