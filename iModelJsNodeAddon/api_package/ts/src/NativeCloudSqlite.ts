/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/
/** @packageDocumentation
 * @module iModels
 */

import * as child_process from "node:child_process";
import * as fs from "node:fs";
import * as os from "node:os";
import * as path from "node:path";
import { NativeLibrary } from "./NativeLibrary";

/**
 * @note This package may only have **dev** dependencies on @itwin packages, so they are *not* available at runtime. Therefore we can only import **types** from them.
 */
import type { LocalFileName } from "@itwin/core-common";

// cspell:ignore polltime cachesize notimestamps deletetime gctime retrytime nwrite ndelete prefetch httptimeout
/* eslint-disable no-restricted-syntax */

export namespace NativeCloudSqlite {

  export const enum LogMask {
    LOG_HTTP = 0x0001, LOG_UPLOAD = 0x0002, LOG_CLEANUP = 0x0004, LOG_EVENT = 0x0008
  }

  /** Properties of a CloudContainer. */
  export interface ContainerProps {
    /** blob storage provider */
    storageType: "azure" | "google" | "s3";
    /** base URI for container. */
    baseUri: string;
    /** the name of the container. */
    containerId: string;
    /** an alias for the container. Defaults to `containerId` */
    alias?: string;
    /** token that grants access to the container. For sas=1 `storageType`s, this is the sasToken. For sas=0, this is the account key */
    accessToken: string;
    /** if true, container is attached with write permissions, and accessToken must provide write access to the cloud container. */
    writeable?: boolean;
    /** if true, container is attached in "secure" mode (blocks are encrypted). Only supported in daemon mode. */
    secure?: boolean;
    /** true if the container is public (doesn't require authentication) */
    isPublic?: boolean;
    /** string attached to log messages from CloudSQLite. This is most useful for identifying usage from daemon mode. */
    logId?: string;
  }

  /** Returned from `CloudContainer.queryDatabase` describing one database in the container */
  export interface CachedDbProps {
    /** The total of (4Mb) blocks in the database. */
    readonly totalBlocks: number;
    /** the number of blocks of the database that have been downloaded into the CloudCache */
    readonly localBlocks: number;
    /** the number of blocks from this database that have been modified in the CloudCache and need to be uploaded. */
    readonly dirtyBlocks: number;
    /** If true, the database currently has transactions in the WAL file and may not be uploaded until they have been checkPointed. */
    readonly transactions: boolean;
    /** the state of this database. Indicates whether the database is new or deleted since last upload */
    readonly state: "" | "copied" | "deleted";
    /** current number of clients that have this database open. */
    readonly nClient: number;
    /** current number of ongoing prefetches on this database. */
    readonly nPrefetch: number;
  }

  /** Returned from 'CloudContainer.queryHttpLog' describing a row in the bcv_http_log table. */
  export interface BcvHttpLog {
    /** Unique, monotonically increasing id value */
    readonly id: number;
    /** Time request was made, as iso-8601 */
    readonly startTime: string;
    /** Time reply received, as iso-8601 (or NULL) */
    readonly endTime: string | undefined;
    /** "PUT", "GET", etc. */
    readonly method: string;
    /**  String configured by 'logId' in ContainerProps. Will be "prefetch" if request was triggered by a prefetch. */
    readonly logId: string;
    /** Log message associated with request */
    readonly logmsg: string;
    /** URI of request */
    readonly uri: string;
    /** HTTP response code (e.g. 200) */
    readonly httpcode: number;
  }

  /** Returned from 'CloudContainer.queryBcvStats' describing the rows in the bcv_stat table.
   *  Also gathers additional statistics using the other virtual tables bcv_container, bcv_database
   *  such as totalClients, ongoingPrefetches, activeClients and attachedContainers.
   */
  export interface BcvStats {
    /** The total number of cache slots that are currently in use or 'locked' by ongoing client read transactions. In daemonless mode, this value is always 0.
     *  A locked cache slot implies that it is not eligible for eviction in the event of a full cachefile.
    */
    readonly lockedCacheslots: number;
    /** The current number of slots with data in them in the cache. */
    readonly populatedCacheslots: number;
    /** The configured size of the cache, in number of slots. */
    readonly totalCacheslots: number;
    /** The total number of clients opened on this cache */
    readonly totalClients?: number;
    /** The total number of ongoing prefetches on this cache */
    readonly ongoingPrefetches?: number;
    /** The total number of active clients on this cache. An active client is one which has an open read txn. */
    readonly activeClients?: number;
    /** The total number of attached containers on this cache. */
    readonly attachedContainers?: number;
    /** The total amount of memory used by sqlite, in bytes. */
    readonly memoryUsed?: number;
    /** The maximum value of memoryUsed since high-water mark was last reset, in bytes. */
    readonly memoryHighwater?: number;
  }

  /** Properties for accessing a CloudContainer */
  export type ContainerAccessProps = ContainerProps & {
    /** Duration for holding write lock, in seconds. After this time the write lock expires if not refreshed. Default is one hour. */
    lockExpireSeconds?: number;
  };

  /** The name of a CloudSqlite database within a CloudContainer. */
  export interface DbNameProp {
    /** the name of the database within the CloudContainer.
     * @note names of databases within a CloudContainer are always **case sensitive** on all platforms.*/
    dbName: string;
  }

  /** Properties for accessing a database within a CloudContainer */
  export interface DbProps extends DbNameProp {
    /** the name of the local file to access the database. */
    localFileName: LocalFileName;
  }

  export type TransferDirection = "upload" | "download";
  export interface TransferProgress {
    /** a user-supplied progress function called during the transfer operation. Return a non-0 value to abort the transfer. */
    onProgress?: (loaded: number, total: number) => number;
  }

  export interface CloudHttpProps {
    /** The number of simultaneous HTTP requests.  Default is 6. */
    nRequests?: number;
    /** The time in seconds to wait without a response before considering a http request as timed out. Default is 60 seconds. */
    httpTimeout?: number;
  }

  export interface PrefetchProps extends CloudHttpProps {
    /** timeout between requests, in milliseconds. Default is 100. */
    timeout?: number;
    /** The number of prefetch requests to issue while there is foreground activity. Default is 3. */
    minRequests?: number;
  }

  export interface CleanDeletedBlocksOptions {
    /**
     * Any block that was marked as unused before this number of seconds ago will be deleted. Specifying a non-zero
     * value gives a period of time for other clients to refresh their manifests and stop using the now-garbage blocks. Otherwise they may get
     * a 404 error. Default is 1 hour.
     */
    nSeconds?: number;
    /** if enabled, outputs verbose logs about the cleanup process. These would include outputting blocks which are determined as eligible for deletion.
     * @default false
    */
    debugLogging?: boolean;
    /** If true, iterates over all blobs in the cloud container to add blocks that are 'orphaned' to the delete list in the manifest.
     * Orphaned blocks are created when a client abruptly halts, is disconnected or encounters an error while uploading a change.
     * If false, the search for 'orphaned' blocks is skipped and only any blocks which are already on the delete list are deleted.
     * @default true
     */
    findOrphanedBlocks?: boolean;
    /**
     * a user-supplied progress function called during the cleanup operation. While the search for orphaned blocks occurs, nDeleted will be 0 and nTotalToDelete will be 1.
     * Once the search is complete and orphaned blocks begin being deleted, nDeleted will be the number of blocks deleted and nTotalToDelete will be the total number of blocks to delete.
     * If the return value is 1, the job will be cancelled. If one or more blocks have already been deleted, then a new manifest file is uploaded saving the progress of the delete job.
     * Return any other non-0 value to abort the transfer without saving progress.
     */
    onProgress?: (nDeleted: number, nTotalToDelete: number) => Promise<number>;
  }

  export type TransferDbProps = DbProps & TransferProgress & CloudHttpProps;

  /** Properties for creating a CloudCache. */
  export interface CacheProps extends CloudHttpProps {
    /** full path of directory for cache to store its files. Must be on a (preferably fast) local drive, and must be empty when the cache is first created. */
    rootDir: string;
    /** name of this cache. It is possible to have more than one CloudCache in the same session. */
    name: string;
    /** maximum cache Size. Must be a number followed by either M (for megabytes) or G (for gigabytes.) Default is 1G */
    cacheSize?: string;
    /** turn on diagnostics for `curl` (outputs to stderr) */
    curlDiagnostics?: boolean;
  }

  /** Properties for creating a new instance of a daemon process. */
  export interface DaemonProps {
    /** full path name of daemon.exe file. Default is to find "iTwinDaemon.exe" in the same directory as this library. */
    exePath?: string;
    /** daemon connection address. Default is "127.0.0.1" */
    addr?: string;
    /** port number. Default 22002 */
    portNumber?: number;
    /** maximum cache Size. Must be a number followed by either M (for megabytes) or G (for gigabytes.) Default is 1G */
    cacheSize?: string;
    /** logging options */
    log?: string;
    /** if true, don't include timestamps in log messages */
    noTimeStamps?: boolean;
    /** The amount of time, in seconds before an http request made to cloud storage by the daemon times out. Default 600 seconds. */
    httptimeout?: number;
    /** options for spawn */
    spawnOptions?: child_process.SpawnOptions;
  }
  export type DaemonCommandArg = DbNameProp & CacheProps & ContainerProps;

  export class Daemon {
    public static exeName(props: DaemonProps) {
      return props.exePath ?? path.join(__dirname, NativeLibrary.archName, os.platform() === "win32" ? "iTwinDaemon.exe" : "iTwinDaemon");
    }
    public static daemonDir(props: CacheProps) {
      return props.rootDir ?? path.join(NativeLibrary.defaultCacheDir, "itwin-daemon");
    }

    /** Start the Daemon process using the supplied properties. The process will be detached from the current process. */
    public static start(props: DaemonProps & CacheProps): child_process.ChildProcess {
      const dir = this.daemonDir(props);
      fs.mkdirSync(dir, { recursive: true }); // make sure the directory exists before starting the daemon

      const args = [`daemon`];
      if (props.addr !== undefined)
        args.push(`-addr`, `${props.addr}`);
      if (props.portNumber !== undefined)
        args.push(`-port`, `${props.portNumber}`);
      if (props.cacheSize)
        args.push(`-cachesize`, `${props.cacheSize}`);
      if (props.log)
        args.push(`-log`, `${props.log}`);
      if (props.noTimeStamps)
        args.push(`-notimestamps`);
      if (props.httptimeout !== undefined)
        args.push(`-httptimeout`, `${props.httptimeout}`);
      args.push(`${dir}`); // This MUST be the last arg when starting the daemon.
      return child_process.spawn(this.exeName(props), args, { ...props.spawnOptions, windowsVerbatimArguments: true });
    }
  }

}
