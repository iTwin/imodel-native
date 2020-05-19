/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

export declare namespace ECSchemaOpsNative {

  /* The native ECSchemaOps class that is projected by the addon. */
  class ECSchemaOps {
    constructor();
    /**
     * Generates a SHA1 hash for a given schema XML file.
     * @param schemaPath The full path to the schema XML file.
     * @param defaultTxn The list of reference paths use to locate schema references.
     * @return The generated SHA1 hash.
     */
    public computeChecksum(schemaPath: string, referencePaths: string[]): string;
  }
}
