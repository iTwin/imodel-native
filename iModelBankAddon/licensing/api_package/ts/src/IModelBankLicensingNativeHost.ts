/*---------------------------------------------------------------------------------------------
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 *--------------------------------------------------------------------------------------------*/
import { IModelBankLicensingNative } from "./IModelBankLicensingNative";

export type MakeDbError = (errno: number, msg: string) => typeof Error;

export class IModelBankLicensingNativeHost {
    public static addon: typeof IModelBankLicensingNative;
    public static makeDbError: MakeDbError;
}
