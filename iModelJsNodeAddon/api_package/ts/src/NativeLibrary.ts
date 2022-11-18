/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import * as os from "os";
import * as path from "path";

import type { NativeCloudSqlite } from "./NativeCloudSqlite";

/**
 * @note This package may only have **dev** dependencies on @itwin packages, so they are *not* available at runtime. Therefore we can only import **types** from them.
 * @note You cannot use mapped types @href https://www.typescriptlang.org/docs/handbook/2/mapped-types.html from itwin imports here due to how they are resolved downstream
 */

import type {
  BentleyStatus, DbOpcode, DbResult, GuidString, Id64Array, Id64String, IDisposable, IModelStatus, Logger, OpenMode, RepositoryStatus,
  StatusCodeWithMessage,
} from "@itwin/core-bentley";
import type  {
  ChangesetIndexAndId, CreateEmptyStandaloneIModelProps, DbRequest, DbResponse, ElementAspectProps, ElementGraphicsRequestProps, ElementLoadProps, ElementProps,
  FilePropertyProps, FontMapProps, GeoCoordinatesRequestProps, GeoCoordinatesResponseProps, GeographicCRSInterpretRequestProps,
  GeographicCRSInterpretResponseProps, GeometryContainmentResponseProps, IModelCoordinatesRequestProps,
  IModelCoordinatesResponseProps, IModelProps, LocalDirName, LocalFileName, MassPropertiesResponseProps, ModelLoadProps,
  ModelProps, QueryQuota, RelationshipProps, SnapshotOpenOptions, TextureData, TextureLoadProps, TileVersionInfo, UpgradeOptions,
} from "@itwin/core-common";
import type { Range3dProps } from "@itwin/core-geometry";

// ###TODO import from core-common after merge with master
export type ElementMeshRequestProps = any;

// cspell:ignore  blocksize cachesize polltime bentleyjs imodeljs ecsql pollable polyface txns lzma uncompress changesets ruleset ulas oidc keychain libsecret rulesets struct
/* eslint-disable @bentley/prefer-get, no-restricted-syntax */

/** Logger categories used by the native addon
 * @internal
 */
export const NativeLoggerCategory = {
  BeSQLite: "BeSQLite",
  BRepCore: "BRepCore",
  Changeset: "Changeset",
  CloudSqlite: "CloudSqlite",
  DgnCore: "DgnCore",
  ECDb: "ECDb",
  ECObjectsNative: "ECObjectsNative",
  SQLite: "SQLite",
  UnitsNative: "UnitsNative",
} as const;

/** Find and load the native node-addon library */
export class NativeLibrary {
  public static get archName() {
    // We make sure we are running in a known platform.
    if (typeof process === "undefined" || process.platform === undefined || process.arch === undefined)
      throw new Error("Error - unknown process");
    return `imodeljs-${process.platform}-${process.arch}`;
  }
  public static get nodeAddonName() { return "imodeljs.node"; }
  public static get defaultLocalDir(): string {
    const homedir = os.homedir();
    const platform = os.platform() as NodeJS.Platform | "ios"; // we add "ios"
    switch (platform) {
      case "win32":
        return path.join(homedir, "AppData", "Local");
      case "darwin":
      case "ios":
        return path.join(homedir, "Library", "Caches");
      case "linux":
      case "android":
        return path.join(homedir, ".cache");
      default:
        throw new Error("Error - unknown platform");
    }
  }

  // This returns true if you used `linkNativePlatform.bat` to install your local addon build.
  public static get isDevBuild(): boolean {
    try {
      require("./devbuild.json");
      return true;
    } catch (_e) {
      return false
    };
  }

  public static get defaultCacheDir(): string { return path.join(this.defaultLocalDir, "iModelJs"); }
  private static _nativeLib?: typeof IModelJsNative;
  public static get nativeLib() { return this.load(); }
  public static load() {
    if (!this._nativeLib) {
      this._nativeLib = require(`./${NativeLibrary.archName}/${NativeLibrary.nodeAddonName}`) as typeof IModelJsNative; // eslint-disable-line @typescript-eslint/no-var-requires
      if (this.isDevBuild)
        console.log("\x1b[36m", `using dev build from ${__dirname}\n`, "\x1b[0m");
    }
    return this._nativeLib;
  }
}

/** Possible outcomes of generateElementGraphics.
* Must be kept in sync with Dgn::Tile::Graphics::TileGraphicsStatus.
* @internal
*/
export const enum ElementGraphicsStatus {
  Success = 0, // Non-null content as Uint8Array.
  Canceled = 1, // e.g., front-end explicitly canceled, iModel was closed while processing request, etc.
  NoGeometry = 2, // Not an error per se
  InvalidJson = 3,
  UnknownMajorFormatVersion = 4,
  ElementNotFound = 5,
  DuplicateRequestId = 6,
}

/** Possible outcomes of exportSchema[s] (and generally writing schemas)
* Must be kept in sync with ECN::SchemaWriteStatus
* @internal
*/
export const enum SchemaWriteStatus {
  Success = 0,
  FailedToSaveXml = 1,
  FailedToCreateXml = 2,
  FailedToCreateJson = 3,
  FailedToWriteFile = 4,
}

/** Module that declares the IModelJs native code.
 * @internal
 */
export declare namespace IModelJsNative {
  interface NameValuePair {
    name: string;
    value: string;
  }

  /** any type of Db that supports EC */
  type AnyECDb = DgnDb | ECDb;
  /** any type of Db */
  type AnyDb = AnyECDb | SQLiteDb;

  /** A string that identifies a Txn. */
  type TxnIdString = string;

  interface NativeCrashReportingConfig {
    /** The directory to which *.dmp and/or iModelJsNativeCrash*.properties.txt files are written. */
    crashDir: string;
    /** Write .dmp files to crashDir? The default is false. Even if writeDumpsToCrashDir is false, the iModelJsNativeCrash*.properties.txt file will be written to crashDir. */
    enableCrashDumps?: boolean;
    /** max # .dmp files that may exist in crashDir */
    maxDumpsInDir?: number;
    /** If writeDumpsToCrashDir is true, do you want a full-memory dump? Defaults to false. */
    wantFullMemory?: boolean;
    /** Additional name, value pairs to add as meta data to a crash report. */
    params?: NameValuePair[];
  }

  const version: string;
  let logger: Logger;
  function setMaxTileCacheSize(maxBytes: number): void;
  function flushLog():void;
  function getTileVersionInfo(): TileVersionInfo;
  function setCrashReporting(cfg: NativeCrashReportingConfig): void;
  function setCrashReportProperty(name: string, value: string | undefined): void;
  function getCrashReportProperties(): NameValuePair[];
  function storeObjectInVault(obj: any, id: string): void;
  function getObjectFromVault(id: string): any;
  function dropObjectFromVault(id: string): void;
  function addReferenceToObjectInVault(id: string): void;
  function getObjectRefCountFromVault(id: string): number;
  function clearLogLevelCache(): void;
  function addFontWorkspace(fileName: LocalFileName, container?: CloudContainer): boolean;
  function addGcsWorkspaceDb(dbNames: string, container?: CloudContainer, priority?: number): boolean;
  function enableLocalGcsFiles(yesNo: boolean): void;
  function queryConcurrency(pool: "io" | "cpu"): number;

  /** Get the SHA1 hash of a Schema XML file, possibly including its referenced Schemas */
  function computeSchemaChecksum(arg: {
    /** the full path to the root schema XML file */
    schemaXmlPath: string,
    /** A list of directories to find referenced schemas */
    referencePaths: string[],
    /** If true, the returned SHA1 includes the hash of all referenced schemas */
    exactMatch?: boolean
  }): string;

  /** The return type of synchronous functions that may return an error or a successful result. */
  type ErrorStatusOrResult<ErrorCodeType, ResultType> = {
    /** Error from the operation. This property is defined if and only if the operation failed. */
    error: StatusCodeWithMessage<ErrorCodeType>;
    result?: never;
    } | {
    error?:never
    /** Result of the operation. This property is defined if the operation completed successfully */
    result: ResultType;
    }

  namespace ConcurrentQuery {
    /**
     * @internal
     */
    type OnResponse = (response: DbResponse)=>void;
    /** Configuration for concurrent query manager
     * @internal
     */
  }
  interface IConcurrentQueryManager {
    concurrentQueryExecute(request: DbRequest, onResponse: ConcurrentQuery.OnResponse): void;
    concurrentQueryResetConfig(config?: QueryConfig): QueryConfig;
    concurrentQueryShutdown(): void;
  }

/** Concurrent query config which should be set before making first call to concurrent query manager.
 * @internal
 */
  export interface QueryConfig {
    globalQuota?: QueryQuota,
    ignoreDelay?: boolean
    ignorePriority?: boolean,
    requestQueueSize?: number,
    workerThreads?: number,
  }

  interface TileContent {
    content: Uint8Array;
    elapsedSeconds: number;
  }
  /** Represents the current state of a pollable tile content request.
   * Note: lack of a "completed" state because polling a completed request returns the content as a Uint8Array.
   * @internal
   */
  const enum TileContentState {
    New = 0, // Request was just created and enqueued.
    Pending = 1, // Request is enqueued but not yet being processed.
    Loading = 2, // Request is being actively processed.
  }

  /** Result of generateElementGraphics conveying graphics as Uint8Array.
   * @internal
   */
  export interface ElementGraphicsContent {
    status: ElementGraphicsStatus.Success;
    content: Uint8Array;
  }

  /** Result of generateElementGraphics that produced no graphics.
   * @internal
   */
  export interface ElementGraphicsError {
    status: Exclude<ElementGraphicsStatus, ElementGraphicsStatus.Success>;
  }

  /** The result of generateElementGraphics.
   * @note generateElementGraphics produces a Promise<ElementGraphicsResult>. It only ever rejects if some unforeseen error like "iModel not open" or "type error" occurs.
   * @internal
   */
  export type ElementGraphicsResult = ElementGraphicsContent | ElementGraphicsError;

  /** Information returned by DgnDb.queryDefinitionElementUsage. */
  interface DefinitionElementUsageInfo {
    /** The subset of input Ids that are SpatialCategory definitions. */
    spatialCategoryIds?: Id64String[];
    /** The subset of input Ids that are DrawingCategory definitions. */
    drawingCategoryIds?: Id64String[];
    /** The subset of input Ids that are SubCategory definitions. */
    subCategoryIds?: Id64String[];
    /** The subset of input Ids that are CategorySelectors. */
    categorySelectorIds?: Id64String[];
    /** The subset of input Ids that are ModelSelectors. */
    modelSelectorIds?: Id64String[];
    /** The subset of input Ids that are DisplayStyles. */
    displayStyleIds?: Id64String[];
    /** The subset of input Ids that are ViewDefinitions. */
    viewDefinitionIds?: Id64String[];
    /** The subset of input Ids that are GeometryParts. */
    geometryPartIds?: Id64String[];
    /** The subset of input Ids that are RenderMaterials. */
    renderMaterialIds?: Id64String[];
    /** The subset of input Ids that are LineStyles. */
    lineStyleIds?: Id64String[];
    /** The subset of input Ids that are Textures. */
    textureIds?: Id64String[];
    /** Ids of *other* input DefinitionElements that were not checked for usage. */
    otherDefinitionElementIds?: Id64String[];
    /** Ids of input DefinitionElements where usage was detected. */
    usedIds?: Id64String[];
  }

  /**
   * Represents all of the valid EC Specification Versions.
   */
  const enum ECVersion {
    V2_0 = (0x02 << 16),
    V3_1 = (0x03 << 16 | 0x01),
    V3_2 = (0x03 << 16 | 0x02),
    Latest = V3_2,
  }

  interface ChangesetFileProps {
    index: number;
    id: string;
    parentId: string;
    changesType: number;
    description: string;
    briefcaseId: number;
    pushDate: string;
    userCreated: string;
    size?: number;
    pathname: string;
  }

  interface EmbeddedFileProps {
    name: string,
    localFileName: string,
  }

  interface EmbedFileArg extends EmbeddedFileProps {
    date: number,
    fileExt?: string,
    compress?: boolean;
  }

  interface EmbedFileQuery {
    size: number,
    date: number,
    fileExt: string
  }

  interface FontEncodingProps {
    codePage?: number;
    degree?: number;
    plusMinus?: number;
    diameter?: number;
  }

  enum FontType { TrueType = 1, Rsc = 2, Shx = 3 }

  interface FontFaceProps {
    faceName: "regular" | "italic" | "bold" | "bolditalic";
    familyName: string;
    type: FontType;
    subId?: number;
    encoding?: FontEncodingProps;
  }

  interface EmbedFontDataProps {
    face: FontFaceProps;
    data: Uint8Array;
  }

  interface EmbedFontFileProps {
    fileName: LocalFileName;
  }

  interface EmbedSystemFontProps {
    systemFont: string;
  }

  type EmbedFontArg = EmbedFontDataProps | EmbedFontFileProps | EmbedSystemFontProps & { compress?: true };

  interface SQLiteOps {
    embedFile(arg: EmbedFileArg): void;
    embedFont(arg: EmbedFontArg): void;
    extractEmbeddedFile(arg: EmbeddedFileProps): void;
    getFilePath(): string;
    getLastInsertRowId(): number;
    getLastError(): string;
    isOpen(): boolean;
    isReadonly(): boolean;
    queryEmbeddedFile(name: string): EmbedFileQuery | undefined;
    queryFileProperty(props: FilePropertyProps, wantString: boolean): string | Uint8Array | undefined;
    queryNextAvailableFileProperty(props: FilePropertyProps): number;
    removeEmbeddedFile(name: string): void;
    replaceEmbeddedFile(arg: EmbedFileArg): void;
    restartDefaultTxn(): void;
    saveChanges(): void;
    saveFileProperty(props: FilePropertyProps, strValue: string | undefined, blobVal: Uint8Array | undefined): void;
    vacuum(arg?: { pageSize?: number, into?: LocalFileName }): void;
  }

  /** The result of DgnDb.inlineGeometryParts.
   * If numSingleRefParts, numRefsInlined, and numPartsDeleted are all the same, the operation was fully successful.
   * Otherwise, some errors occurred inlining and/or deleting one or more parts.
   * A part will not be deleted unless it is first successfully inlined.
   */
  interface InlineGeometryPartsResult {
    /** The number of parts that were determined to have exactly one reference, making them candidates for inlining. */
    numCandidateParts: number;
    /** The number of part references successfully inlined. */
    numRefsInlined: number;
    /** The number of candidate parts that were successfully deleted after inlining. */
    numPartsDeleted: number;
  }

  interface SchemaReferenceProps {
      readonly name: string;
      readonly version: string;
  }

  interface SchemaItemProps {
    readonly $schema?: string;
    readonly schema?: string;  // conditionally required
    readonly schemaVersion?: string;
    readonly name?: string;
    readonly schemaItemType?: string;
    readonly label?: string;
    readonly description?: string;
    readonly customAttributes?: Array<{ [value: string]: any }>;
  }

  interface SchemaProps {
    readonly $schema: string;
    readonly name: string;
    readonly version: string;
    readonly alias: string;
    readonly label?: string;
    readonly description?: string;
    readonly references?: SchemaReferenceProps[];
    readonly items?: { [name: string]: SchemaItemProps };
    readonly customAttributes?: Array<{ [value: string]: any }>;
  }

  // ###TODO import from core-common
  interface ModelExtentsResponseProps {
    id: Id64String;
    extents: Range3dProps;
    status: IModelStatus;
  }

  /** The native object for a Briefcase. */
  class DgnDb implements IConcurrentQueryManager, SQLiteOps {
    constructor();
    public readonly cloudContainer?: CloudContainer;
    public abandonChanges(): DbResult;
    public abandonCreateChangeset(): void;
    public addChildPropagatesChangesToParentRelationship(schemaName: string, relClassName: string): BentleyStatus;
    public addNewFont(arg: { type: FontType, name: string }): number;
    public applyChangeset(changeSet: ChangesetFileProps): void;
    public attachChangeCache(changeCachePath: string): DbResult;
    public beginMultiTxnOperation(): DbResult;
    public beginPurgeOperation(): IModelStatus;
    public cancelElementGraphicsRequests(requestIds: string[]): void;
    public cancelTileContentRequests(treeId: string, contentIds: string[]): void;
    public cancelTo(txnId: TxnIdString): IModelStatus;
    public classIdToName(idString: string): string;
    public classNameToId(className: string): Id64String;
    public closeIModel(): void;
    public completeCreateChangeset(arg: { index: number }): void;
    public computeProjectExtents(wantFullExtents: boolean, wantOutlierIds: boolean): { extents: Range3dProps, fullExtents?: Range3dProps, outliers?: Id64Array };
    public concurrentQueryExecute(request: DbRequest, onResponse: ConcurrentQuery.OnResponse): void;
    public concurrentQueryResetConfig(config?: QueryConfig): QueryConfig;
    public concurrentQueryShutdown(): void;
    public createBRepGeometry(createProps: any/* BRepGeometryCreate */): IModelStatus;
    public createChangeCache(changeCacheFile: ECDb, changeCachePath: string): DbResult;
    public createClassViewsInDb(): BentleyStatus;
    public createIModel(fileName: string, props: CreateEmptyStandaloneIModelProps): void;
    public deleteAllTxns(): void;
    public deleteElement(elemIdJson: string): void;
    public deleteElementAspect(aspectIdJson: string): void;
    public deleteLinkTableRelationship(props: RelationshipProps): DbResult;
    public deleteLocalValue(name: string): void;
    public deleteModel(modelIdJson: string): void;
    public detachChangeCache(): number;
    public dropSchema(schemaName: string): void;
    public dumpChangeset(changeSet: ChangesetFileProps): void;
    public elementGeometryCacheOperation(requestProps: any/* ElementGeometryCacheOperationRequestProps */): BentleyStatus;
    public embedFile(arg: EmbedFileArg): void;
    public embedFont(arg: EmbedFontArg): void;
    public enableChangesetSizeStats(enabled: boolean): DbResult;
    public enableTxnTesting(): void;
    public endMultiTxnOperation(): DbResult;
    public endPurgeOperation(): IModelStatus;
    public executeTest(testName: string, params: string): string;
    public exportGraphics(exportProps: any/* ExportGraphicsProps */): DbResult;
    public exportPartGraphics(exportProps: any/* ExportPartGraphicsProps */): DbResult;
    public exportSchema(schemaName: string, exportDirectory: string): SchemaWriteStatus;
    public exportSchemas(exportDirectory: string): SchemaWriteStatus;
    public extractChangedInstanceIdsFromChangeSets(changeSetFileNames: string[]): ErrorStatusOrResult<IModelStatus, ChangedInstanceIdsProps>;
    public extractChangeSummary(changeCacheFile: ECDb, changesetFilePath: string): ErrorStatusOrResult<DbResult, string>;
    public extractEmbeddedFile(arg: EmbeddedFileProps): void;
    public findGeometryPartReferences(partIds: Id64String[], is2d: boolean): Id64String[];
    public generateElementGraphics(request: ElementGraphicsRequestProps): Promise<ElementGraphicsResult>;
    public generateElementMeshes(request: ElementMeshRequestProps): Promise<Uint8Array>;
    public getBriefcaseId(): number;
    public getChangesetSize(): number;
    public getChangeTrackingMemoryUsed(): number;
    public getCurrentChangeset(): ChangesetIndexAndId;
    public getCurrentTxnId(): TxnIdString;
    public getECClassMetaData(schema: string, className: string): ErrorStatusOrResult<IModelStatus, string>;
    public getElement(opts: ElementLoadProps): ElementProps;
    public getFilePath(): string; // full path of the DgnDb file
    public getGeoCoordinatesFromIModelCoordinates(points: GeoCoordinatesRequestProps): GeoCoordinatesResponseProps;
    public getGeometryContainment(props: object): Promise<GeometryContainmentResponseProps>;
    public getIModelCoordinatesFromGeoCoordinates(points: IModelCoordinatesRequestProps): IModelCoordinatesResponseProps;
    public getIModelId(): GuidString;
    public getIModelProps(): IModelProps;
    public getITwinId(): GuidString;
    public getLastError(): string;
    public getLastInsertRowId(): number;
    public getMassProperties(props: object): Promise<MassPropertiesResponseProps>;
    public getModel(opts: ModelLoadProps): ModelProps;
    public getMultiTxnOperationDepth(): number;
    public getRedoString(): string;
    public getSchemaProps(name: string): SchemaProps;
    public getSchemaPropsAsync(name: string): Promise<SchemaProps>;
    public getSchemaItem(schemaName: string, itemName: string): ErrorStatusOrResult<IModelStatus, string>;
    public getTempFileBaseName(): string;
    public getTileContent(treeId: string, tileId: string, callback: (result: ErrorStatusOrResult<IModelStatus, Uint8Array>) => void): void;
    public getTileTree(id: string, callback: (result: ErrorStatusOrResult<IModelStatus, any>) => void): void;
    public getTxnDescription(txnId: TxnIdString): string;
    public getUndoString(): string;
    public hasFatalTxnError(): boolean;
    public hasPendingTxns(): boolean;
    public hasUnsavedChanges(): boolean;
    public importFunctionalSchema(): DbResult;
    public importSchemas(schemaFileNames: string[]): DbResult;
    public importXmlSchemas(serializedXmlSchemas: string[]): DbResult;
    public inBulkOperation(): boolean;
    public inlineGeometryPartReferences(): InlineGeometryPartsResult;
    public insertCodeSpec(name: string, jsonProperties:{spec: any, scopeSpec: any}): Id64String;
    public insertElement(elemProps: ElementProps, options?: { forceUseId: boolean }): Id64String;
    public insertElementAspect(aspectProps: ElementAspectProps): Id64String;
    public insertLinkTableRelationship(props: RelationshipProps): Id64String;
    public insertModel(modelProps: ModelProps): Id64String;
    public isChangeCacheAttached(): boolean;
    public isGeometricModelTrackingSupported(): boolean;
    public isIndirectChanges(): boolean;
    public isLinkTableRelationship(classFullName: string): boolean | undefined;
    public isOpen(): boolean;
    public isProfilerPaused(): boolean;
    public isProfilerRunning(): boolean;
    public isReadonly(): boolean;
    public isRedoPossible(): boolean;
    public isTxnIdValid(txnId: TxnIdString): boolean;
    public isUndoPossible(): boolean;
    public logTxnError(fatal: boolean): void;
    public openIModel(dbName: string, mode: OpenMode, upgradeOptions?: UpgradeOptions, props?: SnapshotOpenOptions, container?: CloudContainer): void;
    public pauseProfiler(): DbResult;
    public pollTileContent(treeId: string, tileId: string): ErrorStatusOrResult<IModelStatus, TileContentState | TileContent>;
    public processGeometryStream(requestProps: any/* ElementGeometryOptions */): IModelStatus;
    public purgeTileTrees(modelIds: Id64Array | undefined): void;
    public queryDefinitionElementUsage(definitionElementIds: Id64Array): DefinitionElementUsageInfo | undefined;
    public queryEmbeddedFile(name: string): EmbedFileQuery | undefined;
    public queryFileProperty(props: FilePropertyProps, wantString: boolean): string | Uint8Array | undefined;
    public queryFirstTxnId(): TxnIdString;
    public queryLocalValue(name: string): string | undefined;
    // ###TODO mark deprecated use queryModelExtentsAsync
    public queryModelExtents(options: {id: Id64String}):  { modelExtents: Range3dProps };
    public queryModelExtentsAsync(modelIds: Id64String[]): Promise<ModelExtentsResponseProps[]>;
    public queryNextAvailableFileProperty(props: FilePropertyProps): number;
    public queryNextTxnId(txnId: TxnIdString): TxnIdString;
    public queryPreviousTxnId(txnId: TxnIdString): TxnIdString;
    public queryTextureData(opts: TextureLoadProps): Promise<TextureData | undefined>;
    public readFontMap(): FontMapProps;
    public reinstateTxn(): IModelStatus;
    public removeEmbeddedFile( name: string): void;
    public replaceEmbeddedFile(arg: EmbedFileArg): void;
    public resetBriefcaseId(idValue: number): void;
    public restartDefaultTxn(): void;
    public restartTxnSession(): void;
    public resumeProfiler(): DbResult;
    public reverseAll(): IModelStatus;
    public reverseTo(txnId: TxnIdString): IModelStatus;
    public reverseTxns(numOperations: number): IModelStatus;
    public saveChanges(description?: string): DbResult;
    public saveFileProperty(props: FilePropertyProps, strValue: string | undefined, blobVal: Uint8Array | undefined): void;
    public saveLocalValue(name: string, value: string | undefined): void;
    public schemaToXmlString(schemaName: string, version?: ECVersion): string | undefined;
    public setGeometricModelTrackingEnabled(enabled: boolean): ErrorStatusOrResult<IModelStatus, boolean>;
    public setIModelDb(iModelDb?: any/* IModelDb */): void;
    public setIModelId(guid: GuidString): DbResult;
    public setITwinId(guid: GuidString): DbResult;
    public simplifyElementGeometry(simplifyArgs: any): DbResult;
    public startCreateChangeset(): ChangesetFileProps;
    public startProfiler(scopeName?: string, scenarioName?: string, overrideFile?: boolean, computeExecutionPlan?: boolean): DbResult;
    public stopProfiler(): { rc: DbResult, elapsedTime?: number, scopeId?: number, fileName?: string };
    public updateElement(elemProps: ElementProps): void;
    public updateElementAspect(aspectProps: ElementAspectProps): void;
    public updateElementGeometryCache(props: object): Promise<any>;
    public updateIModelProps(props: IModelProps): void;
    public updateLinkTableRelationship(props: RelationshipProps): DbResult;
    public updateModel(modelProps: ModelProps): void;
    public updateModelGeometryGuid(modelId: Id64String): IModelStatus;
    public updateProjectExtents(newExtentsJson: string): void;
    public writeAffectedElementDependencyGraphToFile(dotFileName: string, changedElems:Id64Array): BentleyStatus;
    public writeFullElementDependencyGraphToFile(dotFileName: string): BentleyStatus;
    public vacuum(arg?: { pageSize?: number, into?: LocalFileName }): void;

    public static enableSharedCache(enable: boolean): DbResult;
    public static getAssetsDir(): string;
  }

  /** The native object for GeoServices. */
  class GeoServices {
      constructor();
      public static getGeographicCRSInterpretation(props: GeographicCRSInterpretRequestProps): GeographicCRSInterpretResponseProps;

  }

  /**
   * RevisionUtility help with debugging and testing
   * @internal
   */
  class RevisionUtility {
    constructor();
    public static assembleRevision(targetFile: string, rawChangesetFile: string, prefixFile?: string, lzmaPropsJson?: string): BentleyStatus;
    public static computeStatistics(sourceFile: string, addPrefix: boolean): string;
    public static disassembleRevision(sourceFile: string, targetDir: string): BentleyStatus;
    public static dumpChangesetToDb(sourceFile: string, dbFile: string, includeCols: boolean): BentleyStatus;
    public static getUncompressSize(sourceFile: string): string;
    public static normalizeLzmaParams(lzmaPropsJson?: string): string;
    public static recompressRevision(sourceFile: string, targetFile: string, lzmaPropsJson?: string): BentleyStatus;
  }

  class ECDb implements IDisposable, IConcurrentQueryManager {
    constructor();
    public abandonChanges(): DbResult;
    public closeDb(): void;
    public createDb(dbName: string): DbResult;
    public dispose(): void;
    public dropSchema(schemaName: string): void;
    public getFilePath(): string;
    public importSchema(schemaPathName: string): DbResult;
    public isOpen(): boolean;
    public openDb(dbName: string, mode: OpenMode, upgradeProfiles?: boolean): DbResult;
    public saveChanges(changesetName?: string): DbResult;
    public getLastError(): string;
    public getLastInsertRowId(): number;
    public static enableSharedCache(enable: boolean): DbResult;
    public concurrentQueryExecute(request: DbRequest, onResponse: ConcurrentQuery.OnResponse): void;
    public concurrentQueryResetConfig(config?: QueryConfig): QueryConfig;
    public concurrentQueryShutdown(): void;
  }

  class ChangedElementsECDb implements IDisposable {
    constructor();
    public dispose(): void;
    public createDb(db: DgnDb, dbName: string): DbResult;
    public openDb(dbName: string, mode: OpenMode, upgradeProfiles?: boolean): DbResult;
    public isOpen(): boolean;
    public closeDb(): void;
    public processChangesets(db: DgnDb, changesets: ChangesetFileProps[], rulesetId: string, filterSpatial?: boolean, wantParents?: boolean, wantPropertyChecksums?: boolean, rulesetDir?: string, tempDir?: string, wantChunkTraversal?: boolean): DbResult;
    public processChangesetsAndRoll(dbFilename: string, dbGuid: string, changesets: ChangesetFileProps[], rulesetId: string, filterSpatial?: boolean, wantParents?: boolean, wantPropertyChecksums?: boolean, rulesetDir?: string, tempDir?: string, wantRelationshipCaching?: boolean, relationshipCacheSize?: number, wantChunkTraversal?: boolean): DbResult;
    public getChangedElements(startChangesetId: string, endChangesetId: string): ErrorStatusOrResult<IModelStatus, any>;
    public isProcessed(changesetId: string): boolean;
    public cleanCaches(): void;
  }

  class ECSqlStatement implements IDisposable {
    constructor();
    public clearBindings(): DbResult;
    public dispose(): void;
    public getBinder(param: number | string): ECSqlBinder;
    public getColumnCount(): number;
    public getValue(columnIndex: number): ECSqlValue;
    public prepare(db: AnyECDb, ecsql: string, logErrors?: boolean): StatusCodeWithMessage<DbResult>;
    public reset(): DbResult;
    public step(): DbResult;
    public stepAsync(callback: (result: DbResult) => void): void;
    public stepForInsert(): { status: DbResult, id: string };
    public stepForInsertAsync(callback: (result: { status: DbResult, id: string }) => void): void;
    public getNativeSql(): string;
  }

  class ECSqlBinder {
    constructor();
    public addArrayElement(): ECSqlBinder;
    public bindBlob(base64String: string | Uint8Array | ArrayBuffer | SharedArrayBuffer): DbResult;
    public bindBoolean(val: boolean): DbResult;
    public bindDateTime(isoString: string): DbResult;
    public bindDouble(val: number): DbResult;
    public bindGuid(guidStr: GuidString): DbResult;
    public bindId(hexStr: Id64String): DbResult;
    public bindIdSet(hexVector: Id64String[]): DbResult;
    public bindInteger(val: number | string): DbResult;
    public bindMember(memberName: string): ECSqlBinder;
    public bindNavigation(navIdHexStr: Id64String, relClassName?: string, relClassTableSpace?: string): DbResult;
    public bindNull(): DbResult;
    public bindPoint2d(x: number, y: number): DbResult;
    public bindPoint3d(x: number, y: number, z: number): DbResult;
    public bindString(val: string): DbResult;
  }

  class ECSqlColumnInfo {
    constructor();
    public getAccessString(): string;
    public getPropertyName(): string;
    public getOriginPropertyName(): string;
    public getRootClassAlias(): string;
    public getRootClassName(): string;
    public getRootClassTableSpace(): string;
    public getType(): number;
    public isEnum(): boolean;
    public isGeneratedProperty(): boolean;
    public isSystemProperty(): boolean;
    public hasOriginProperty(): boolean;
  }

  class ECSqlValue {
    constructor();
    public getArrayIterator(): ECSqlValueIterator;
    public getBlob(): Uint8Array;
    public getBoolean(): boolean;
    public getClassNameForClassId(): string;
    public getColumnInfo(): ECSqlColumnInfo;
    public getDateTime(): string;
    public getDouble(): number;
    public getEnum(): Array<{ schema: string, name: string, key: string, value: number | string }> | undefined;
    public getGeometry(): string;
    public getGuid(): GuidString;
    public getId(): Id64String;
    public getInt(): number;
    public getInt64(): number;
    public getNavigation(): { id: Id64String, relClassName?: string };
    public getPoint2d(): { x: number, y: number };
    public getPoint3d(): { x: number, y: number, z: number };
    public getString(): string;
    public getStructIterator(): ECSqlValueIterator;
    public isNull(): boolean;
  }

  class ECSqlValueIterator {
    constructor();
    public getCurrent(): ECSqlValue;
    public moveNext(): boolean;
  }

  /** Default transaction mode for SQLiteDbs.
   * @see https://www.sqlite.org/lang_transaction.html
  */
  const enum DefaultTxnMode {
    /** no default transaction is started. You must use BEGIN/COMMIT or SQLite will use implicit transactions */
    None = 0,
    /** A deferred transaction is started when the file is first opened. This is the default. */
    Deferred = 1,
    /** An immediate transaction is started when the file is first opened. */
    Immediate = 2,
    /** An exclusive transaction is started when the file is first opened. */
    Exclusive = 3
  }

  /** parameters common to opening or creating a new SQLiteDb */
  interface SQLiteDbOpenOrCreateParams {
    /** If true, do not require that the `be_Prop` table exist */
    rawSQLite?: boolean;
    /** @see immutable option at https://www.sqlite.org/c3ref/open.html */
    immutable?: boolean;
    /** Do not attempt to verify that the file is a valid sQLite file before opening. */
    skipFileCheck?: boolean;
    /** the default transaction mode
     * @see [[SQLiteDb.DefaultTxnMode]]
    */
    defaultTxn?: 0 | 1 | 2 | 3;
    /** see query parameters from 'URI Filenames' in  https://www.sqlite.org/c3ref/open.html */
    queryParam?: string;
  }

  /** Parameters for opening an existing SQLiteDb */
  interface SQLiteDbOpenParams extends SQLiteDbOpenOrCreateParams {
    /** use OpenMode.ReadWrite to open the file with write access */
    openMode: OpenMode;
  }

  /** Size of a SQLiteDb page in bytes */
  interface PageSize {
    /** see https://www.sqlite.org/pragma.html#pragma_page_size */
    pageSize?: number;
  }

  /** Parameters for creating a new SQLiteDb */
  type SQLiteDbCreateParams = SQLiteDbOpenOrCreateParams & PageSize;

  class SQLiteDb implements SQLiteOps, IDisposable {
    constructor();
    public readonly cloudContainer?: CloudContainer;
    public abandonChanges(): void;
    public closeDb(): void;
    public createDb(dbName: string, container?: CloudContainer, params?: SQLiteDbCreateParams): void;
    public dispose(): void;
    public embedFile(arg: EmbedFileArg): void;
    public embedFont(arg: EmbedFontArg): void;
    public extractEmbeddedFile(arg: EmbeddedFileProps): void;
    public getFilePath(): string;
    public getLastError(): string;
    public getLastInsertRowId(): number;
    public isOpen(): boolean;
    public isReadonly(): boolean;
    public openDb(dbName: string, mode: OpenMode | SQLiteDbOpenParams, container?: CloudContainer): void;
    public queryEmbeddedFile(name: string): EmbedFileQuery | undefined;
    public queryFileProperty(props: FilePropertyProps, wantString: boolean): string | Uint8Array | undefined;
    public queryNextAvailableFileProperty(props: FilePropertyProps): number;
    public removeEmbeddedFile(name: string): void;
    public replaceEmbeddedFile(arg: EmbedFileArg): void;
    public restartDefaultTxn(): void;
    public saveChanges(): void;
    public saveFileProperty(props: FilePropertyProps, strValue: string | undefined, blobVal?: Uint8Array): void;
    public vacuum(arg?: { pageSize?: number, into?: LocalFileName }): void;
  }

  class SqliteStatement implements IDisposable {
    constructor();
    public bindBlob(param: number | string, val: Uint8Array | ArrayBuffer | SharedArrayBuffer): DbResult;
    public bindDouble(param: number | string, val: number): DbResult;
    public bindGuid(param: number | string, guidStr: GuidString): DbResult;
    public bindId(param: number | string, hexStr: Id64String): DbResult;
    public bindInteger(param: number | string, val: number | string): DbResult;
    public bindNull(param: number | string): DbResult;
    public bindString(param: number | string, val: string): DbResult;
    public clearBindings(): DbResult;
    public dispose(): void;
    public getColumnBytes(columnIndex: number): number;
    public getColumnCount(): number;
    public getColumnName(columnIndex: number): string;
    public getColumnType(columnIndex: number): number;
    public getValueBlob(columnIndex: number): Uint8Array;
    public getValueDouble(columnIndex: number): number;
    public getValueGuid(columnIndex: number): GuidString;
    public getValueId(columnIndex: number): Id64String;
    public getValueInteger(columnIndex: number): number;
    public getValueString(columnIndex: number): string;
    public isReadonly(): boolean;
    public isValueNull(columnIndex: number): boolean;
    public prepare(db: AnyDb, sql: string, logErrors?: boolean): void;
    public reset(): DbResult;
    public step(): DbResult;
    public stepAsync(callback: (result: DbResult) => void): void;
  }

  /** Incremental IO for blobs */
  class BlobIO {
    constructor();
    /** Close this BlobIO if it is opened.
     * @note this BlobIO *may* be reused after this call by calling `open` again.
    */
    public close(): void;
    /** get the total number of bytes in the blob */
    public getNumBytes(): number;
    /** @return true if this BlobIO was successfully opened and may be use to read or write the blob */
    public isValid(): boolean;
    /** Open this BlobIO against a table/row/column in a Db */
    public open(
      /** The database for the blob */
      db: AnyDb,
      args: {
        /** the name of the table for the blob*/
        tableName: string,
        /** the name of the column for the blob */
        columnName: string,
        /** The rowId of the blob */
        row: number,
        /** If true, open this BlobIO for write access */
        writeable?: boolean
      }): void;
    /** Read from a blob
     * @returns the contents of the requested byte range
     */
    public read(args: {
      /** The number of bytes to read */
      numBytes: number;
      /** starting offset within the blob to read */
      offset: number;
      /** If present and of sufficient size, use this ArrayBuffer for the value. */
      blob?: ArrayBuffer; }): Uint8Array;
    /** Reposition this BlobIO to a new rowId
     * @note this BlobIO must be valid when this methods is called.
     */
    public changeRow(row: number): void;
    /** Write to a blob */
    public write(args: {
      /** The number of bytes to write  */
      numBytes: number;
      /** starting offset within the blob to write */
      offset: number;
      /** the value to write */
      blob: ArrayBuffer; }): void;
  }

  /**
   * A cache for storing data from CloudSqlite databases. This object refers to a directory on a local filesystem
   * and is used to **connect** CloudContainers so they may be accessed. The contents of the cache directory are entirely
   * controlled by CloudSqlite and should be empty when the cache is first created and never modified directly. It maintains
   * the state of the local data across sessions.
   */
  class CloudCache {
    /** Create an instance of a CloudCache. */
    public constructor(props: NativeCloudSqlite.CacheProps);
    /** `true` if this CloudCache is connected to a daemon process */
    public get isDaemon(): boolean;
    /** The name for this CloudCache. */
    public get name(): string;
    /** The root directory of this CloudCache on a local drive. */
    public get rootDir(): LocalDirName;
    /** The guid for this CloudCache. Used for acquiring write lock. */
    public get guid(): GuidString;
    public setLogMask(mask: number): void;
    /** destroy this CloudCache. All CloudContainers should be detached before calling this. */
    public destroy(): void;
  }

  /** A CloudSqlite container that may be connected to a CloudCache. */
  class CloudContainer {
    public readonly cache?: CloudCache;
    /** Create a new instance of a CloudContainer. It must be connected to a CloudCache for most operations. */
    public constructor(props: NativeCloudSqlite.ContainerAccessProps);
    /** The ContainerId. */
    public get containerId(): string;
    /** The *alias* to identify this CloudContainer in a CloudCache. Usually just the ContainerId. */
    public get alias(): string;
    /** true if this CloudContainer is currently connected to a CloudCache via the `connect` method. */
    public get isConnected(): boolean;
    /** true if this CloudContainer was created with the `writeable` flag (and its `accessToken` supplies write access). */
    public get isWriteable(): boolean;
    /** true if this CloudContainer currently holds the write lock for its container in the cloud. */
    public get hasWriteLock(): boolean;
    /** true if this CloudContainer has local changes that have not be uploaded to its container in the cloud. */
    public get hasLocalChanges(): boolean;
    /** The current accessToken providing access to the cloud container */
    public get accessToken(): string;
    public set accessToken(val: string);
    /** Get the number of garbage blocks in this container that can be purged. */
    public get garbageBlocks(): number;
    /** The block size for this CloudContainer. */
    public get blockSize(): number;

    /**
     * initialize a cloud blob-store container to be used as a new Sqlite CloudContainer. This creates the manifest, and should be
     * performed on an empty container. If an existing manifest is present, it is destroyed and a new one is created (essentially emptying the container.)
     */
    public initializeContainer(opts?: { checksumBlockNames?: boolean, blockSize?: number }): void;

    /**
     * Attempt to acquire the write lock for this CloudContainer. For this to succeed:
     * 1. it must be connected to a `CloudCache`
     * 2. this CloudContainer must have been constructed with `writeable: true`
     * 3. the `accessToken` must authorize write access
     * 4. no other process may be holding an unexpired write lock
     * @throws exception if any of the above conditions fail
     * @note Write locks *expire* after the duration specified in the `durationSeconds` property of the constructor argument, in case a process
     * crashes or otherwise fails to release the lock. Calling `acquireWriteLock` with the lock already held resets the lock duration from the current time,
     * so long running processes should call this method periodically to ensure their lock doesn't expire (they should also make sure their accessToken is refreshed
     * before it expires.)
     * @note on success, the manifest is polled before the promise resolves.
     * @param user  An identifier of the process/user locking the CloudContainer. In the event of a write lock
     * collision, this string will be included in the exception string of the *other* process attempting to obtain a write lock.
     */
    public acquireWriteLock(user: string): void;

    /**
     * Release the write lock if it is currently held.
     * @note if there are local changes that have not been uploaded, they are automatically uploaded before the write lock is released.
     * @note if the write lock is not held, this method does nothing.
     */
    public releaseWriteLock(): void;

    /**
     * Destroy any currently valid write lock from this or any other process. This is obviously very dangerous and defeats the purpose of write locking.
     * This method exists only for administrator tools to clear a failed process without waiting for the expiration period. It can also be useful for tests.
     * For this to succeed, all of the conditions of `acquireWriteLock` must be true other than #4.
     */
    public clearWriteLock(): void;

    /**
     * Abandon any local changes in this container. If the write lock is currently held, it is released.
     * This function fails with BE_SQLITE_BUSY if one or more clients have open read or write transactions
     * on any database in the container.
     */
     public abandonChanges(): void;

     /**
     * Connect this CloudContainer to a CloudCache for reading or writing its manifest, write lock, and databases.
     * @note A CloudCache is a local directory holding copies of information from the cloud. Its content is persistent across sessions,
     * but this method must be called each session to (re)establish the connection to the cache. If the CloudCache was previously populated,
     * this method may be called and will succeed *even when offline* or without a valid `accessToken`.
     * @note all operations that access the contents of databases or the manifest require this method be called (`isConnected === true`).
     */
    public connect(cache: CloudCache): void;

    /**
     * Disconnect this CloudContainer from its CloudCache. There must be no open databases from this container. Leaves the container attached to the
     * CloudCache so it is available for future sessions.
     */
    public disconnect(): void;

    /**
     * Permanently Detach and Disconnect this CloudContainer from its CloudCache. There must be no open databases from this container.
     */
     public detach(): void;

     /**
     * Poll cloud storage for changes from other processes. *No changes* made by other processes are visible to
     * this CloudContainer unless/until this method is called.
     * @note this is automatically called whenever the write lock is obtained to ensure all changes are against the latest version.
     */
    public checkForChanges(): void;

    /**
     * Upload any changed blocks from all databases in this CloudContainer.
     * @note this is called automatically from `releaseWriteLock` before the write lock is released. It is only necessary to call this directly if you
     * wish to upload changes while the write lock is still held.
     * @see hasLocalChanges
     */
    public uploadChanges(): Promise<void>;

    /**
     * Clean any unused deleted blocks from cloud storage. When a database is written, a subset of its blocks are replaced
     * by new versions, sometimes leaving the originals unused. In this case, they are not deleted immediately.
     * Instead, they are scheduled for deletion at some later time. Calling this method deletes all blocks in the cloud container
     * for which the scheduled deletion time has passed.
     * @param nSeconds Any block that was marked as unused before this number of seconds ago will be deleted. Specifying a non-zero
     * value gives a period of time for other clients to refresh their manifests and stop using the now-garbage blocks. Otherwise they may get
     * a 404 error. Default is 1 hour.
     */
    public cleanDeletedBlocks(nSeconds?: number): Promise<void>;

    /**
     * Create a copy of an existing database within this CloudContainer with a new name.
     * @note CloudSqlite uses copy-on-write semantics for this operation. That is, this method merely makes a
     * new entry in the manifest with the new name that *shares* all of its blocks with the original database.
     * If either database subsequently changes, the only modified blocks are not shared.
     */
    public copyDatabase(dbName: string, toAlias: string): Promise<void>;

    /** Remove a database from this CloudContainer.
     * @see cleanDeletedBlocks
     */
    public deleteDatabase(dbName: string): Promise<void>;

    /** Get the list of database names in this CloudContainer.
     * @param globArg if present, filter the results with SQLite [GLOB](https://www.sqlite.org/lang_expr.html#glob) operator.
     */
    public queryDatabases(globArg?: string): string[];

    /**
     * Get the status of a specific database in this CloudContainer.
     * @param dbName the name of the database of interest
     */
    public queryDatabase(dbName: string): NativeCloudSqlite.CachedDbProps | undefined;

    /**
     * Get the SHA1 hash of the content of a database.
     * @param dbName the name of the database of interest
     * @note the hash will be empty if the database does not exist
     */
    public queryDatabaseHash(dbName: string): string;
  }

  /**
   * Object to perform an "upload" or "download" of a database to/from a CloudContainer.
   * @note The transfer begins when the object is constructed, and the object remains alive during the upload/download operation.
   * It provides the Promise that is resolved when the operation completes or fails, and has methods to provide feedback for progress and to cancel the operation prematurely.
   */
  class CloudDbTransfer {
    /** create an instance of a transfer. The operation begins immediately when the object is created.
     * @param direction either "upload" or "download"
     * @param container the container holding the database. Does *not* require that the container be connected to a CloudCache.
     * @param args The properties for the source and target of the transfer.
     */
    constructor(direction: NativeCloudSqlite.TransferDirection, container: CloudContainer, args: NativeCloudSqlite.TransferDbProps);

    /** Cancel a currently pending transfer and cause the promise to be rejected with a Cancelled status.
     * @throws exception if the operation has already completed.
     */
    public cancelTransfer(): void;
    /** Get the current progress of the transfer.
     * @throws exception if the operation has already completed.
     */
    public getProgress(): { loaded: number, total: number };

    /** Promise that is resolved when the transfer completes, or is rejected if the transfer fails (or is cancelled.) */
    public promise: Promise<void>;
  }

  class CloudPrefetch {
    public readonly cloudContainer: CloudContainer;
    public readonly dbName: string;

    /** create an instance of a prefetch operation. The operation begins immediately when the object is created.
     * The prefetch will continue in the background until it either finishes, is canceled, or the CloudContainer is disconnected from its CloudCache.
     * @param container the container holding the database.
     * @param dbName the name of the database to prefetch
     */
    constructor(container: CloudContainer, dbName: string, args?: NativeCloudSqlite.PrefetchProps );

    /** Cancel a currently pending prefetch. The promise will be resolved immediately after this call. */
    public cancel(): void;

    /**
     * Promise that is resolved when the prefetch completes or is cancelled. Await this promise to ensure that the
     * database has been fully downloaded before going offline, for example.
     * @returns a Promise that resolves to `true` if the prefetch completed and the entire database is local, or `false` if it was aborted or failed.
     * @note the promise is *not* rejected on `cancel`. Some progress may (or may not) have been made by the request.
     * @note To monitor the progress being made during prefetch, call `CloudContainer.queryDatabase` periodically.
     */
    public promise: Promise<boolean>;
  }

  const enum ECPresentationStatus {
    Success = 0,
    Canceled = 1,
    Pending = 2,
    Error = 0x10000,
    InvalidArgument = Error + 1,
  }

  interface ECPresentationMemoryHierarchyCacheConfig {
    mode: "memory";
  }

  interface ECPresentationDiskHierarchyCacheConfig {
    mode: "disk";
    directory: string;
    memoryCacheSize?: number;
  }

  interface ECPresentationHybridHierarchyCacheConfig {
    mode: "hybrid";
    disk?: ECPresentationDiskHierarchyCacheConfig;
  }

  type ECPresentationHierarchyCacheConfig = ECPresentationMemoryHierarchyCacheConfig | ECPresentationDiskHierarchyCacheConfig | ECPresentationHybridHierarchyCacheConfig;

  type ECPresentationManagerResponse<TResult> = ErrorStatusOrResult<ECPresentationStatus, TResult> & {
    diagnostics?: any;
  }

  interface ECPresentationManagerProps {
    id: string;
    taskAllocationsMap: { [priority: number]: number };
    defaultFormats: {
      [phenomenon: string]: {
        unitSystems: string[];
        serializedFormat: string;
      };
    };
    isChangeTrackingEnabled: boolean;
    cacheConfig: ECPresentationHierarchyCacheConfig;
    contentCacheSize?: number;
    workerConnectionCacheSize?: number;
    useMmap?: boolean | number;
  }

  class ECPresentationManager implements IDisposable {
    constructor(props: ECPresentationManagerProps);
    public forceLoadSchemas(db: DgnDb): Promise<ECPresentationManagerResponse<void>>;
    public setupRulesetDirectories(directories: string[]): ECPresentationManagerResponse<void>;
    public setupSupplementalRulesetDirectories(directories: string[]): ECPresentationManagerResponse<void>;
    public setRulesetVariableValue(rulesetId: string, variableId: string, type: string, value: any): ECPresentationManagerResponse<void>;
    public unsetRulesetVariableValue(rulesetId: string, variableId: string): ECPresentationManagerResponse<void>;
    public getRulesetVariableValue(rulesetId: string, variableId: string, type: string): ECPresentationManagerResponse<any>;
    public getRulesets(rulesetId: string): ECPresentationManagerResponse<string>;
    public addRuleset(serializedRuleset: string): ECPresentationManagerResponse<string>;
    public removeRuleset(rulesetId: string, hash: string): ECPresentationManagerResponse<boolean>;
    public clearRulesets(): ECPresentationManagerResponse<void>;
    public handleRequest(db: DgnDb, options: string): { result: Promise<ECPresentationManagerResponse<string>>, cancel: () => void };
    public getUpdateInfo(): ECPresentationManagerResponse<any>;
    public updateHierarchyState(db: DgnDb, rulesetId: string, changeType: "nodesExpanded" | "nodesCollapsed", serializedKeys: string): ECPresentationManagerResponse<void>;
    public dispose(): void;
  }

  namespace ECSchemaXmlContext {
    interface SchemaKey {
      name: string;
      readVersion: number;
      writeVersion: number;
      minorVersion: number;
    }

    const enum SchemaMatchType {
      Identical = 0,               // Find exact VersionRead, VersionWrite, VersionMinor match as well as Data
      Exact = 1,                   // Find exact VersionRead, VersionWrite, VersionMinor match.
      LatestWriteCompatible = 2,   // Find latest version with matching VersionRead and VersionWrite
      Latest = 3,                  // Find latest version.
      LatestReadCompatible = 4,    // Find latest version with matching VersionRead
    }

    type SchemaLocaterCallback = (key: SchemaKey, matchType: SchemaMatchType) => string | undefined | void;
  }

  class ECSchemaXmlContext {
    constructor();
    public addSchemaPath(path: string): void;
    public setSchemaLocater(locater: ECSchemaXmlContext.SchemaLocaterCallback): void;
    public setFirstSchemaLocater(locater: ECSchemaXmlContext.SchemaLocaterCallback): void;
    public readSchemaFromXmlFile(filePath: string): ErrorStatusOrResult<BentleyStatus, string>;
  }

  class SnapRequest {
    constructor();
    public doSnap(db: DgnDb, request: any): Promise<any>;
    public cancelSnap(): void;
  }


  interface FeatureUserDataKeyValuePair {
    key: string;
    value: string;
  }

  interface NativeUlasClientFeatureEvent {
    featureId: string;
    versionStr: string;
    projectId?: string;
    startDateZ?: string;
    endDateZ?: string;
    featureUserData?: FeatureUserDataKeyValuePair[];
  }

  const enum DbChangeStage {
    Old = 0,
    New = 1,
  }

  const enum DbValueType {
    IntegerVal  = 1,
    FloatVal    = 2,
    TextVal     = 3,
    BlobVal     = 4,
    NullVal     = 5,
  }

  type ChangeValueType = Uint8Array | number| string | null | undefined;

  interface ChangedValue {
    new?: ChangeValueType;
    old?: ChangeValueType;
  }

  class ChangesetReader {
    close(): DbResult;
    getColumnCount(): number| undefined;
    getColumnValue(col: number, stage: DbChangeStage): ChangeValueType;
    getColumnValueType(col: number, stage: DbChangeStage): DbValueType | undefined;
    getFileName(): string | undefined;
    getOpCode(): DbOpcode | undefined;
    getRow(): ChangedValue[] | undefined;
    getDdlChanges(): string | undefined;
    getTableName(): string | undefined;
    isIndirectChange(): boolean | undefined;
    isPrimaryKeyColumn(col: number): boolean | undefined;
    open(fileName: string, invert: boolean): DbResult;
    reset(): DbResult;
    step(): DbResult;
  }

  class DisableNativeAssertions implements IDisposable {
    constructor();
    public dispose(): void;
  }

  class ImportContext implements IDisposable {
    constructor(sourceDb: DgnDb, targetDb: DgnDb);
    public dispose(): void;
    public dump(outputFileName: string): BentleyStatus;
    public addClass(sourceClassFullName: string, targetClassFullName: string): BentleyStatus;
    public addCodeSpecId(sourceId: Id64String, targetId: Id64String): BentleyStatus;
    public addElementId(sourceId: Id64String, targetId: Id64String): BentleyStatus;
    public removeElementId(sourceId: Id64String): BentleyStatus;
    public findCodeSpecId(sourceId: Id64String): Id64String;
    public findElementId(sourceId: Id64String): Id64String;
    public cloneElement(sourceId: Id64String, cloneOptions?: CloneElementOptions): ElementProps;
    public importCodeSpec(sourceId: Id64String): Id64String;
    public importFont(sourceId: number): number;
    public hasSubCategoryFilter(): boolean;
    public isSubCategoryFiltered(sourceId: Id64String): boolean;
    public filterSubCategoryId(sourceId: Id64String): BentleyStatus;
    public saveStateToDb(db: SQLiteDb): void;
    public loadStateFromDb(db: SQLiteDb): void;
  }

  interface CloneElementOptions {
    binaryGeometry?: boolean;
  }

  interface ChangedInstanceOpsProps {
    insert?: Id64String[];
    update?: Id64String[];
    delete?: Id64String[];
  }

  /** The *wire format* of the result returned by DgnDb.extractChangedInstanceIdsFromChangeSets */
  interface ChangedInstanceIdsProps {
    codeSpec?: ChangedInstanceOpsProps;
    model?: ChangedInstanceOpsProps;
    element?: ChangedInstanceOpsProps;
    aspect?: ChangedInstanceOpsProps;
    relationship?: ChangedInstanceOpsProps;
    font?: ChangedInstanceOpsProps;
  }

  /**
   * Temporary implementation to allow crashing the backend for testing purposes
   * @internal
   */
  class NativeDevTools {
    public static signal(signalType: number): boolean;
  }

  /**
   * Utilities to encode/decode names to convert them to/from names that comply with EC naming rules
   */
  class ECNameValidation {
    /**
     * Encodes names to comply with EC naming rules
     * @param name Name to be encoded
     * @returns Encoded name
     * @param name
     */
    public static encodeToValidName(name: string): string;

    /**
    * Decodes names that were encoded to comply with EC naming rules
    * @param encodedName Encoded name
    * @returns Decoded name
    */
    public static decodeFromValidName(encodedName: string): string;
  }

  const enum TestWorkerState {
    NotQueued = 0,
    Queued = 1,
    Skipped = 2,
    Running = 3,
    Ok = 4,
    Error = 5,
    Aborted = 6,
  }

  class TestWorker {
    public constructor(db: DgnDb);

    public queue(): Promise<void>;
    public cancel(): void;
    public setReady(): void;
    public setThrow(): void;
    public isCanceled(): boolean;
    public wasExecuted(): boolean;
    public getState(): TestWorkerState;
  }
}
