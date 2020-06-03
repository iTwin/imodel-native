/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
/** @packageDocumentation
 * @module iModels
 */

import * as child_process from "child_process";
import * as fs from "fs";
import * as path from "path";
import { DbResult } from "@bentley/bentleyjs-core";
import { NativeLibrary } from "./NativeLibrary";

// cspell:ignore polltime blocksize cachesize
// tslint:disable: no-const-enum

export const enum BlobStorageType {
  Azure = 1, AzureSas = 2, AzureEmulator = 3, AzureEmulatorSas = 4, Google = 5,
}

/** Error values for negative numbers returned by the BlobDaemon `attach` command. */
export const enum DaemonAttachError {
  INVALID_DAEMON_DIRECTORY = -1, DAEMON_NOT_ACTIVE = -2, SOCKET_ERROR = -3,
}

/** Possible values for `command` argument of `BlobDaemon.command` method. */
export type BlobDaemonCommand = "attach" | "detach" | "create" | "destroy" | "upload" | "download" | "delete" | "copy";

/** Properties for accessing BlockCache directory and cloud storage. */
export interface BlobCacheProps {
  /** blob store account name. For Azure storage emulator, use "127.0.0.1:10000". */
  account: string;
  /** full path of directory for daemon to store its files. Must be on fast local drive. */
  daemonDir?: string;
  /** blob storage type */
  storageType?: BlobStorageType;
  /** block size, in megabytes used by CREATE command. Default is 4MB. */
  blockSizeMb?: number;
}

/** Properties for creating a new instance of the BlockCache daemon process. */
export interface DaemonProps {
  /** full path name of daemon.exe file. Default is to find "BeBlobDaemon.exe" in the same directory as this library. */
  exePath?: string;
  /** port number. Default 22002 */
  portNumber?: number;
  /** maximum cache Size. Must be a number followed by either M (for megabytes) or G (for gigabytes.) Default is 1G */
  maxCacheSize?: string;
  /** How often cloud storage is polled for database changes made by other daemon processes, in seconds. Default is 60. */
  pollTime?: number;
  /** How long an overwritten block is allowed to persist before it is permanently deleted, in seconds. Default is 3600. */
  deleteTime?: number;
  /** How often each daemon process scans for and deletes such garbage files, in seconds. Default is 3600. */
  gcTime?: number;
  /** How long after a failed checkpoint-to-upload operation the daemon process waits before retrying, in seconds. Default=10. */
  retryTime?: number;
  /** The maximum number of concurrent uploads (PUT requests) the daemon will make when uploading a new version of a database to cloud storage. Default=10. */
  nWrites?: number;
  /** The maximum number of concurrent deletes (DELETE requests) the daemon will make when uploading a new version of a database. Default=10. */
  nDeletes?: number;
  /** If true, unmodified cache entries are retained across runs of the daemon. Default=false. */
  persistAcrossSessions?: boolean;
  /** logging options */
  log?: string;
  /** options for spawn */
  spawnOptions?: child_process.SpawnOptions;
}

/** The name and key for a blob container */
export interface BlobContainerProps {
  /** the name of the container. */
  container: string;
  /** SAS key that grants access to the container. */
  sasKey: string;
  /** if true, container is attached with write permissions. */
  writeable?: boolean;
}

/** Properties for uploading, downloading, copying, or deleting a database, by its alias, within a BlobContainer. */
export interface BlobDbProps {
  /** The alias of the database within the container */
  dbAlias: string;
  /** local file name. Applies only to `upload` and `download` commands. */
  localFile?: string;
  /** A new alias, used only for the `copy` command  */
  toAlias?: string;
  /** progress callback for `upload` and `download` commands. Return non-zero to abort operation. */
  onProgress?: (nDone: number, nTotal: number) => number;
}

export class BlobDaemon {
  private static exeName(props: DaemonProps) {
    return props.exePath ?? path.join(path.dirname(require.resolve(NativeLibrary.libraryName)), "BeBlobDaemon.exe");
  }
  public static daemonDir(props: BlobCacheProps) {
    const dir = props.daemonDir ?? path.join(NativeLibrary.defaultCacheDir, "blob-daemon");
    if (!fs.existsSync(dir))
      fs.mkdirSync(dir);
    return dir;
  }
  /** Start the BlobDaemon process using the supplied properties. The process will be detached from the current process. */
  public static start(props: DaemonProps & BlobCacheProps): child_process.ChildProcess {
    const args = ["daemon", `-account ${props.account}`, `-directory ${this.daemonDir(props)}`, `-polltime ${props.pollTime ?? 60}`];
    if (props.portNumber !== undefined)
      args.push(`-port ${props.portNumber}`);
    if (props.maxCacheSize)
      args.push(`-cachesize ${props.maxCacheSize}`);
    if (props.log)
      args.push(`-log ${props.log}`);
    if (props.storageType === BlobStorageType.AzureEmulator || props.storageType === BlobStorageType.AzureEmulatorSas)
      args.push(`-emulator ${props.account}`);
    if (props.deleteTime !== undefined)
      args.push(`-deletetime ${props.deleteTime}`);
    if (props.gcTime !== undefined)
      args.push(`-gctime ${props.gcTime}`);
    if (props.retryTime !== undefined)
      args.push(`-retrytime ${props.retryTime}`);
    if (props.nWrites !== undefined)
      args.push(`-nwrite ${props.nWrites}`);
    if (props.nDeletes !== undefined)
      args.push(`-ndelete ${props.nDeletes}`);
    if (props.persistAcrossSessions)
      args.push(`-persistent`);

    return child_process.spawn(this.exeName(props), args, { ...props.spawnOptions, windowsVerbatimArguments: true });
  }

  /** Get the full path to the local file that can be used to open a BlobDb. For the file to exist, you must first issue the "attach" command. */
  public static getDbFileName(props: BlobDbProps & BlobCacheProps & BlobContainerProps) {
    return path.join(this.daemonDir(props), props.container, props.dbAlias);
  }

  /** Perform one of the BlobDaemon commands.
   * @param command The BlobDaemon command to run.
   * @param args arguments to the command.
   * @return `Promise<{ result, errMsg }>` the meaning of result varies by command, but 0 always means success. For `attach`, negative values are `DaemonAttachError`, positive numbers below 400 are `SQLITE_xxx` errors, and
   * values 400 and above are HTTP errors. `errMsg` is blank on success, but an english message useful for diagnostics otherwise.
   */
  public static async command(command: BlobDaemonCommand, args: BlobDbProps & BlobCacheProps): Promise<{ result: DaemonAttachError | DbResult | number, errMsg: string }> {
    const fullArgs = { ...args };
    fullArgs.daemonDir = fullArgs.daemonDir ?? this.daemonDir(args);
    return NativeLibrary.nativeLib.runDaemonCommand(command, fullArgs);
  }
}
