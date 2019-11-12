/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/
export declare function RunBridge(jsonOptions: string): number;

export declare class BridgeJob implements IDisposable {

    constructor();

    public runSync(jobInfo: string): int;

    public runAsync(jobInfo: string, callback: (err, result: int) => void): void;
}