/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import * as path from "path";
import * as fs from "fs-extra";
import { copyFile, dbFileName, getAssetsDir, getOutputDir, iModelJsNative } from "./utils";
import { expect } from "chai";
import { OpenMode } from "@itwin/core-bentley";
import { IModelJsNative } from "../NativeLibrary";
import { Range3d } from "@itwin/core-geometry";

describe("embedded files", () => {
  const embeddedName = "embedded file 1";
  const embeddedExt = "abc";
  const validateEmbedding = (db: IModelJsNative.DgnDb | IModelJsNative.SQLiteDb, originalFile: string) => {
    const statOrig = fs.statSync(originalFile);
    const info = db.queryEmbeddedFile(embeddedName)!;
    expect(info.date).equals(Math.trunc(statOrig.mtimeMs));
    expect(info.size).equals(statOrig.size);
    expect(info.fileExt).equals(embeddedExt);

    const localFileName = path.join(getOutputDir(), "extracted");
    if (fs.existsSync(localFileName))
      fs.removeSync(localFileName);
    db.extractEmbeddedFile({ localFileName, name: embeddedName });
    const statExtracted = fs.statSync(localFileName);
    expect(statOrig.size).equal(statExtracted.size);
    const f1 = fs.readFileSync(localFileName);
    const f2 = fs.readFileSync(originalFile);
    expect(f1).to.deep.equal(f2);
    fs.removeSync(localFileName);
  }

  const testEmbed = (db: IModelJsNative.DgnDb | IModelJsNative.SQLiteDb) => {
    let stat = fs.statSync(dbFileName);
    db.embedFile({ name: embeddedName, localFileName: dbFileName, date: stat.mtimeMs, fileExt: embeddedExt });
    validateEmbedding(db, dbFileName);

    const newName = path.join(getAssetsDir(), "test.text");
    stat = fs.statSync(newName);
    db.replaceEmbeddedFile({ name: embeddedName, localFileName: newName, date: stat.mtimeMs });
    validateEmbedding(db, newName);

    expect(() => db.extractEmbeddedFile({ localFileName: "none", name: "not there" })).to.throw("not found");
    db.removeEmbeddedFile(embeddedName);
    expect(db.queryEmbeddedFile(embeddedName)).to.be.undefined;
    expect(() => db.extractEmbeddedFile({ localFileName: "none", name: embeddedName })).to.throw("not found");

    stat = fs.statSync(dbFileName);
    db.embedFile({ name: embeddedName, localFileName: dbFileName, date: stat.mtimeMs, fileExt: embeddedExt, compress: false });
    validateEmbedding(db, dbFileName);
  }

  it("embed files in SQLiteDb", () => {
    const tempSQLiteDbName = path.join(getOutputDir(), "testEmbed.db");
    const tempSQLiteDb = new iModelJsNative.SQLiteDb();

    if (fs.existsSync(tempSQLiteDbName))
      fs.removeSync(tempSQLiteDbName);
    tempSQLiteDb.createDb(tempSQLiteDbName);
    testEmbed(tempSQLiteDb);
    tempSQLiteDb.closeDb();
  });

  it("embed files in DgnDb", () => {
    const tempDgnDb = new iModelJsNative.DgnDb();
    const tempDgnDbName = copyFile("testEmbed.bim", dbFileName);
    tempDgnDb.openIModel(tempDgnDbName, OpenMode.ReadWrite);
    testEmbed(tempDgnDb);
    expect(tempDgnDb.hasUnsavedChanges()).to.be.false;
    expect(tempDgnDb.hasPendingTxns()).to.be.false;
    tempDgnDb.closeIModel();
  });

  it("BlobIO", () => {
    const tempSQLiteDbName = path.join(getOutputDir(), "testBlobIO.db");
    const tempSQLiteDb = new iModelJsNative.SQLiteDb();

    if (fs.existsSync(tempSQLiteDbName))
      fs.removeSync(tempSQLiteDbName);
    tempSQLiteDb.createDb(tempSQLiteDbName);

    const stmt = new iModelJsNative.SqliteStatement();
    stmt.prepare(tempSQLiteDb, "CREATE TABLE blobs(id TEXT PRIMARY KEY NOT NULL,value BLOB)");
    stmt.step();
    stmt.dispose();

    const testRange = new Range3d(1.2, 2.3, 3.4, 4.5, 5.6, 6.7);
    const blobVal = new Uint8Array(testRange.toFloat64Array().buffer);
    const blob2 = new Int32Array([22,33,44,55,66]);
    stmt.prepare(tempSQLiteDb, "INSERT INTO blobs(id,value) VALUES(?,?)");
    stmt.bindString(1, "test1");
    stmt.bindBlob(2, blobVal);
    stmt.step();
    stmt.reset();
    stmt.bindString(1, "test2");
    stmt.bindBlob(2, blob2.buffer);
    stmt.step();
    stmt.dispose();
    tempSQLiteDb.saveChanges();

    const blobIo = new iModelJsNative.BlobIO();
    blobIo.open(tempSQLiteDb, {tableName: "blobs", columnName: "value", row: 1});
    const val = blobIo.read({numBytes: 8*3, offset: 0});
    let floats = new Float64Array(val.buffer);
    expect(floats[0]).eq(1.2);
    expect(floats[1]).eq(2.3);
    expect(floats[2]).eq(3.4);
    const val2 = blobIo.read({numBytes: 8*3, offset: 8*3, blob: val});
    floats = new Float64Array(val.buffer);
    expect(val2).eq(val)
    expect(floats[0]).eq(4.5);
    expect(floats[1]).eq(5.6);
    expect(floats[2]).eq(6.7);

    const val3 = blobIo.read({numBytes: 8*6, offset: 0, blob: val});
    expect(val3).not.eq(val);
    floats = new Float64Array(val3.buffer);
    expect(floats[0]).eq(1.2);
    expect(floats[1]).eq(2.3);
    expect(floats[2]).eq(3.4);
    expect(floats[3]).eq(4.5);
    expect(floats[4]).eq(5.6);
    expect(floats[5]).eq(6.7);

    floats[0] = 0;
    const val4 = new Float64Array(blobIo.read({numBytes: 8*3, offset: 8*3, blob: floats}).buffer);
    expect(val4[0]).eq(4.5);
    expect(val4[1]).eq(5.6);
    expect(val4[2]).eq(6.7);

    blobIo.changeRow(2);
    expect(blobIo.getNumBytes()).eq(5*4);
    expect(blobIo.isValid()).eq(true);
    const val5 = new Int32Array(blobIo.read({numBytes: 5*4, offset:0}).buffer);
    expect(val5).to.deep.eq(blob2);

    val5[0] = 100;
    val5[1] = 200;
    blobIo.close();
    blobIo.open(tempSQLiteDb, {tableName: "blobs", columnName: "value", row: 2, writeable: true});
    blobIo.write({numBytes: 2*4, offset: 2*4, blob: val5});
    blobIo.close();

    blobIo.open(tempSQLiteDb, {tableName: "blobs", columnName: "value", row: 2});
    const val6 =  new Int32Array(blobIo.read({numBytes: 5*4, offset:0}).buffer);
    expect(val6[0]).eq(blob2[0]);
    expect(val6[1]).eq(blob2[1]);
    expect(val6[2]).eq(val5[0]);
    expect(val6[3]).eq(val5[1]);
    expect(val6[4]).eq(blob2[4]);

    blobIo.close();
    expect(blobIo.isValid()).eq(false);

    tempSQLiteDb.closeDb();
  });
});
