/*---------------------------------------------------------------------------------------------
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 *--------------------------------------------------------------------------------------------*/
import { IModelBankNative } from "./IModelBankNative";

export type MakeDbError = (errno: number, msg: string) => typeof Error;

export class IModelBankNativeHost {
    public static addon: typeof IModelBankNative;
    public static makeDbError: MakeDbError;
}
