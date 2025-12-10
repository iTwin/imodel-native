/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import * as os from "node:os";
import * as path from "node:path";

import type { NativeCloudSqlite } from "./NativeCloudSqlite";

/**
 * @note This package may only have **dev** dependencies on @itwin packages, so they are *not* available at runtime. Therefore we can only import **types** from them.
 * @note You cannot use mapped types @href https://www.typescriptlang.org/docs/handbook/2/mapped-types.html from itwin imports here due to how they are resolved downstream
 */

import type {
  BentleyStatus, DbOpcode, DbResult, GuidString, Id64Array, Id64String, IDisposable, IModelStatus, LogLevel, OpenMode
} from "@itwin/core-bentley";
import type {
  ChangesetIndexAndId, CodeProps, CodeSpecProperties, CreateEmptyStandaloneIModelProps, DbRequest, DbResponse, ElementAspectProps,
  ElementGeometryBuilderParams,
  ElementGeometryBuilderParamsForPart,
  ElementGraphicsRequestProps, ElementLoadOptions, ElementLoadProps, ElementMeshRequestProps, ElementProps,
  FilePropertyProps, FontId, FontMapProps, GeoCoordinatesRequestProps, GeoCoordinatesResponseProps, GeographicCRSInterpretRequestProps,
  GeographicCRSInterpretResponseProps, GeometryContainmentResponseProps, GeometryStreamProps, ImageBuffer, ImageBufferFormat, ImageSourceFormat, IModelCoordinatesRequestProps,
  IModelCoordinatesResponseProps, IModelProps, LocalDirName, LocalFileName, MassPropertiesResponseProps, ModelLoadProps,
  ModelProps, PlacementProps, QueryQuota, RelationshipProps, SnapshotOpenOptions, TextureData, TextureLoadProps, TileVersionInfo, UpgradeOptions
} from "@itwin/core-common";
import type { LowAndHighXYZProps, Range2dProps, Range3dProps } from "@itwin/core-geometry";

/* eslint-disable @typescript-eslint/naming-convention */
/* eslint-disable no-restricted-syntax */
/* eslint-disable @itwin/prefer-get */

// cspell:ignore  blocksize cachesize polltime bentleyjs imodeljs ecsql pollable polyface txns lzma uncompress changesets ruleset ulas oidc keychain libsecret rulesets struct

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

/** @internal */
export interface NativeLogger {
  readonly minLevel: LogLevel | undefined;
  readonly categoryFilter: Readonly<{ [categoryName: string]: LogLevel | undefined }>;
  logTrace: (category: string, message: string) => void;
  logInfo: (category: string, message: string) => void;
  logWarning: (category: string, message: string) => void;
  logError: (category: string, message: string) => void;
}

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
      require.resolve("./devbuild.json");
      return true;
    } catch {
      return false;
    }
  }

  public static get defaultCacheDir(): string { return path.join(this.defaultLocalDir, "iModelJs"); }
  private static _nativeLib?: typeof IModelJsNative;
  public static get nativeLib() { return this.load(); }
  public static load() {
    if (!this._nativeLib) {
      try {
        const platform = os.platform() as NodeJS.Platform | "ios"; // we add "ios"
        if (platform === "ios" || platform === "android") {
          this._nativeLib = (process as any)._linkedBinding("iModelJsNative") as typeof IModelJsNative;
        } else {
          this._nativeLib =
            require(`./${NativeLibrary.archName}/${NativeLibrary.nodeAddonName}`) as typeof IModelJsNative; // eslint-disable-line @typescript-eslint/no-require-imports
        }
      } catch (err: any) {
        err.message += "\nThis error may occur when trying to run an iTwin.js backend without"
          + " having installed the prerequisites. See the following link for all prerequisites:"
          + "\nhttps://www.itwinjs.org/learning/supportedplatforms/#backend-prerequisites";
        throw err;
      }
      if (this.isDevBuild)
        // eslint-disable-next-line no-console
        console.log("\x1b[36m", `using dev build from ${__dirname}\n`, "\x1b[0m");
    }
    return this._nativeLib;
  }
}

/** WAL checkpoint mode
 * @internal
 */
export const enum WalCheckpointMode {
  Passive = 0,  /* Do as much as possible w/o blocking */
  Full = 1,     /* Wait for writers, then checkpoint */
  Restart = 2,  /* Like FULL but wait for for readers */
  Truncate = 3,  /* Like RESTART but also truncate WAL */
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

  let logger: NativeLogger;
  function setMaxTileCacheSize(maxBytes: number): void;
  function getTileVersionInfo(): TileVersionInfo;
  function setCrashReporting(cfg: NativeCrashReportingConfig): void;
  function setCrashReportProperty(name: string, value: string | undefined): void;
  function getCrashReportProperties(): NameValuePair[];
  function clearLogLevelCache(): void;
  function addFontWorkspace(fileName: LocalFileName, container?: CloudContainer): boolean;
  function addGcsWorkspaceDb(dbNames: string, container?: CloudContainer, priority?: number): boolean;
  function enableLocalGcsFiles(yesNo: boolean): void;
  function queryConcurrency(pool: "io" | "cpu"): number;

  interface TrueTypeFontMetadata {
    faces: FontFaceProps[];
    embeddable: boolean;
  }

  function getTrueTypeFontMetadata(fileName: LocalFileName): TrueTypeFontMetadata;
  function isRscFontData(blob: Uint8Array): boolean;

  function imageBufferFromImageSource(
    sourceFormat: ImageSourceFormat.Png | ImageSourceFormat.Jpeg,
    sourceData: Uint8Array,
    targetFormat: ImageBufferFormat.Rgb | ImageBufferFormat.Rgba | 255,
    flipVertically: boolean
  ): Pick<ImageBuffer, "data" | "format" | "width"> | undefined;

  function imageSourceFromImageBuffer(
    imageFormat: ImageBufferFormat.Rgb | ImageBufferFormat.Rgba,
    imageData: Uint8Array,
    imageWidth: number,
    imageHeight: number,
    targetFormat: ImageSourceFormat.Png | ImageSourceFormat.Jpeg | 255,
    flipVertically: boolean,
    jpegQuality: number
  ): { format: ImageSourceFormat.Jpeg | ImageSourceFormat.Png, data: Uint8Array } | undefined;

  /** Get the SHA1 hash of a Schema XML file, possibly including its referenced Schemas */
  function computeSchemaChecksum(arg: {
    /** the full path to the root schema XML file */
    schemaXmlPath: string;
    /** A list of directories to find referenced schemas */
    referencePaths: string[];
    /** If true, the returned SHA1 includes the hash of all referenced schemas */
    exactMatch?: boolean;
  }): string;

  /** When you want to associate an explanatory message with an error status value. */
  interface StatusCodeWithMessage<ErrorCodeType> {
    status: ErrorCodeType;
    message: string;
  }

  /** The return type of synchronous functions that may return an error or a successful result. */
  type ErrorStatusOrResult<ErrorCodeType, ResultType> = {
    /** Error from the operation. This property is defined if and only if the operation failed. */
    error: StatusCodeWithMessage<ErrorCodeType>;
    result?: never;
  } | {
    error?: never;
    /** Result of the operation. This property is defined if the operation completed successfully */
    result: ResultType;
  };

  namespace ConcurrentQuery {
    /**
     * @internal
     */
    type OnResponse = (response: DbResponse) => void;
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
    globalQuota?: QueryQuota;
    ignoreDelay?: boolean;
    ignorePriority?: boolean;
    requestQueueSize?: number;
    workerThreads?: number;
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

  /** Instance key for a change
   * @internal
   */
  export interface ChangeInstanceKey {
    id: Id64String;
    classFullName: string;
    changeType: "inserted" | "updated" | "deleted";
  }

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

  interface PerStatementHealthStats {
    sqlStatement: string;
    dbOperation: string;
    rowCount: number;
    elapsedMs: number;
    fullTableScans: number;
  }
  interface ChangesetHealthStats {
    changesetId: string;
    uncompressedSizeBytes: number;
    sha1ValidationTimeMs: number;
    insertedRows: number;
    updatedRows: number;
    deletedRows: number;
    totalElapsedMs: number;
    totalFullTableScans: number;
    perStatementStats: [PerStatementHealthStats];
  }
  interface ECSqlRowAdaptorOptions {
    abbreviateBlobs?: boolean;
    classIdsToClassNames?: boolean;
    useJsName?: boolean;
    doNotConvertClassIdsToClassNamesWhenAliased?: boolean; // backward compatibility
  }

  interface EmbeddedFileProps {
    name: string;
    localFileName: string;
  }

  interface EmbedFileArg extends EmbeddedFileProps {
    date: number;
    fileExt?: string;
    compress?: boolean;
  }

  interface EmbedFileQuery {
    size: number;
    date: number;
    fileExt: string;
  }

  interface FontEncodingProps {
    codePage?: number;
    degree?: number;
    plusMinus?: number;
    diameter?: number;
  }

  interface ResolveInstanceKeyArgs {
    partialKey?: { id: Id64String, baseClassName: string };
    federationGuid?: GuidString;
    code?: CodeProps;
  }

  interface ResolveInstanceKeyResult {
    id: Id64String;
    classFullName: string;
  }

  enum FontType { TrueType = 1, Rsc = 2, Shx = 3 }

  interface FontFaceProps {
    faceName: "regular" | "italic" | "bold" | "bolditalic";
    familyName: string;
    type: FontType;
    subId?: number;
    encoding?: FontEncodingProps;
  }

  interface SQLiteOps {
    embedFile(arg: EmbedFileArg): void;
    embedFontFile(id: number, faces: FontFaceProps[], data: Uint8Array, compress: boolean): void;
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
    enableWalMode(yesNo?: boolean): void;
    /** perform a checkpoint if this db is in WAL mode. Otherwise this function does nothing.
     * @param mode the checkpoint mode. Default is `Truncate`.
     */
    performCheckpoint(mode?: WalCheckpointMode): void;
    setAutoCheckpointThreshold(frames: number): void;
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

  interface SchemaImportOptions {
    readonly schemaLockHeld?: boolean;
    readonly schemaSyncDbUri?: string;
    readonly ecSchemaXmlContext?: ECSchemaXmlContext;
  }

  interface SchemaLocalDbInfo {
    readonly id: string;
    readonly dataVer: string;
    readonly lastModUtc: string;
  }

  interface SchemaSyncDbInfo extends SchemaLocalDbInfo {
    readonly projectId: string;
    readonly parentChangesetId: string;
    readonly parentChangesetIndex?: string;
  }

  export interface TxnProps {
    id: TxnIdString;
    sessionId: number;
    nextId?: TxnIdString;
    prevId?: TxnIdString;
    props: { description?: string; source?: string, appData: { [key: string]: any } };
    type: "Data" | "ECSchema" | "Ddl";
    reversed: boolean;
    grouped: boolean;
    timestamp: string; // ISO 8601 format
  }

  type GeometryOutputFormat = "BinaryStream" | "GeometryStreamProps";
  interface IGeometrySource {
    geom?: Uint8Array | GeometryStreamProps;
    builder?: ElementGeometryBuilderParams;
    placement?: PlacementProps;
    categoryId?: Id64String;
    is2d: boolean;
  }


  interface IGeometryPart {
    geom?: Uint8Array | GeometryStreamProps;
    builder?: ElementGeometryBuilderParamsForPart;
    is2d: boolean;
    bbox?: LowAndHighXYZProps;
  }




  // ###TODO import from core-common
  interface ModelExtentsResponseProps {
    id: Id64String;
    extents: Range3dProps;
    status: IModelStatus;
  }

  interface TextLayoutRangesProps {
    layout: Range2dProps;
    justification: Range2dProps;
  }

  enum TextEmphasis { None = 0, Bold = 1, Italic = 2, BoldItalic = Bold | Italic }

  type NoCaseCollation = "ASCII" | "Latin1";

  /** The native object for a Briefcase. */
  class DgnDb implements IConcurrentQueryManager, SQLiteOps {
    constructor();
    public readonly cloudContainer?: CloudContainer;
    public attachDb(filename: string, alias: string): void;
    public detachDb(alias: string): void;
    public getNoCaseCollation(): NoCaseCollation;
    public setNoCaseCollation(collation: NoCaseCollation): void;
    public schemaSyncSetDefaultUri(syncDbUri: string): void;
    public schemaSyncGetDefaultUri(): string;
    public schemaSyncInit(syncDbUri: string, containerId: string, overrideContainer: boolean): void;
    public schemaSyncPull(syncDbUri?: string): void;
    public schemaSyncPush(syncDbUri?: string): void;
    public schemaSyncEnabled(): boolean;
    public schemaSyncGetLocalDbInfo(): SchemaLocalDbInfo | undefined;
    public schemaSyncGetSyncDbInfo(syncDbUri: string): SchemaSyncDbInfo | undefined;
    public abandonChanges(): DbResult;
    public abandonCreateChangeset(): void;
    public addChildPropagatesChangesToParentRelationship(schemaName: string, relClassName: string): BentleyStatus;
    public invalidateFontMap(): void;
    public applyChangeset(changeSet: ChangesetFileProps, fastForward: boolean): void;
    public revertTimelineChanges(changeSet: ChangesetFileProps[], skipSchemaChanges: boolean): void;
    public attachChangeCache(changeCachePath: string): DbResult;
    public beginMultiTxnOperation(): DbResult;
    public beginPurgeOperation(): IModelStatus;
    public cancelElementGraphicsRequests(requestIds: string[]): void;
    public cancelTileContentRequests(treeId: string, contentIds: string[]): void;
    public cancelTo(txnId: TxnIdString): IModelStatus;
    public classIdToName(idString: string): string;
    public classNameToId(className: string): Id64String;
    public closeFile(): void;
    public completeCreateChangeset(arg: { index: number }): void;
    public computeProjectExtents(wantFullExtents: boolean, wantOutlierIds: boolean): { extents: Range3dProps, fullExtents?: Range3dProps, outliers?: Id64Array };
    public computeRangesForText(chars: string, fontId: FontId, bold: boolean, italic: boolean, widthFactor: number, height: number): TextLayoutRangesProps;
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
    public deleteLinkTableRelationships(props: ReadonlyArray<RelationshipProps>): DbResult;
    public deleteLocalValue(name: string): void;
    public deleteModel(modelIdJson: string): void;
    public detachChangeCache(): number;
    public dropSchemas(schemaNames: ReadonlyArray<string>): void;
    public dumpChangeset(changeSet: ChangesetFileProps): void;
    public elementGeometryCacheOperation(requestProps: any/* ElementGeometryCacheOperationRequestProps */): BentleyStatus;
    public embedFile(arg: EmbedFileArg): void;
    public embedFontFile(id: number, faces: FontFaceProps[], data: Uint8Array, compress: boolean): void;
    public enableChangesetSizeStats(enabled: boolean): DbResult;
    public enableTxnTesting(): void;
    public endMultiTxnOperation(): DbResult;
    public endPurgeOperation(): IModelStatus;
    public executeTest(testName: string, params: string): string;
    public exportGraphics(exportProps: any/* ExportGraphicsProps */): DbResult;
    public exportPartGraphics(exportProps: any/* ExportPartGraphicsProps */): DbResult;
    public exportSchema(schemaName: string, exportDirectory: string, outFileName?: string): SchemaWriteStatus;
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
    public getCodeValueBehavior(): "exact" | "trim-unicode-whitespace";
    public getCurrentChangeset(): ChangesetIndexAndId;
    public getCurrentTxnId(): TxnIdString;
    public getECClassMetaData(schema: string, className: string): ErrorStatusOrResult<IModelStatus, string>;
    public getElement(opts: ElementLoadProps): ElementProps;
    public executeSql(sql: string): DbResult;
    public getFilePath(): string; // full path of the DgnDb file
    public getGeoCoordinatesFromIModelCoordinates(points: GeoCoordinatesRequestProps): GeoCoordinatesResponseProps;
    public getGeometryContainment(props: object): Promise<GeometryContainmentResponseProps>;
    public getIModelCoordinatesFromGeoCoordinates(points: IModelCoordinatesRequestProps): IModelCoordinatesResponseProps;
    public getIModelId(): GuidString;
    public getIModelProps(when?: "pullMerge"): IModelProps;
    public resolveInstanceKey(args: ResolveInstanceKeyArgs): ResolveInstanceKeyResult;
    public readInstance(key: NodeJS.Dict<any>, args: NodeJS.Dict<any>): NodeJS.Dict<any>;
    public insertInstance(inst: NodeJS.Dict<any>, args: NodeJS.Dict<any>): Id64String;
    public updateInstance(inst: NodeJS.Dict<any>, args: NodeJS.Dict<any>): boolean;
    public deleteInstance(key: NodeJS.Dict<any>, args: NodeJS.Dict<any>): boolean;
    public patchJsonProperties(jsonProps: string): string;
    public newBeGuid(): GuidString;

    public clearECDbCache(): void;

    public convertOrUpdateGeometrySource(arg: IGeometrySource, outFmt: GeometryOutputFormat, opts: ElementLoadOptions): IGeometrySource;
    public convertOrUpdateGeometryPart(arg: IGeometryPart, outFmt: GeometryOutputFormat, opts: ElementLoadOptions): IGeometryPart;

    // when lets getIModelProps know that the extents may have been updated as the result of a pullChanges and should be read directly from the iModel as opposed to the cached extents.
    public getITwinId(): GuidString;
    public getLastError(): string;
    public getLastInsertRowId(): number;
    public getLocalChanges(rootClassFilter: string[], includeInMemoryChanges: boolean): ChangeInstanceKey[]
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
    public importSchemas(schemaFileNames: string[], options?: SchemaImportOptions): DbResult;
    public importXmlSchemas(serializedXmlSchemas: string[], options?: SchemaImportOptions): DbResult;
    public inBulkOperation(): boolean;
    public inlineGeometryPartReferences(): InlineGeometryPartsResult;
    public insertCodeSpec(name: string, jsonProperties: CodeSpecProperties): Id64String;
    public insertElement(elemProps: ElementProps, options?: { forceUseId?: boolean }): Id64String;
    public insertElementAspect(aspectProps: ElementAspectProps): Id64String;
    public insertLinkTableRelationship(props: RelationshipProps): Id64String;
    public insertModel(modelProps: ModelProps): Id64String;
    public isChangeCacheAttached(): boolean;
    public isGeometricModelTrackingSupported(): boolean;
    public isLinkTableRelationship(classFullName: string): boolean | undefined;
    public isOpen(): boolean;
    public isProfilerPaused(): boolean;
    public isProfilerRunning(): boolean;
    public isReadonly(): boolean;
    public isRedoPossible(): boolean;
    public isSubClassOf(childClassFullName: string, parentClassFullName: string): boolean;
    public isTxnIdValid(txnId: TxnIdString): boolean;
    public isUndoPossible(): boolean;
    public logTxnError(fatal: boolean): void;
    public moveElementToModel(elementId: Id64String, modelId: Id64String): IModelStatus;
    public openIModel(dbName: string, mode: OpenMode, upgradeOptions?: UpgradeOptions & SchemaImportOptions, props?: SnapshotOpenOptions, container?: CloudContainer, sqliteOptions?: { busyTimeout?: number }): void;
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
    public queryModelExtents(options: { id: Id64String }): { modelExtents: Range3dProps };
    public queryModelExtentsAsync(modelIds: Id64String[]): Promise<ModelExtentsResponseProps[]>;
    public queryNextAvailableFileProperty(props: FilePropertyProps): number;
    public queryNextTxnId(txnId: TxnIdString): TxnIdString;
    public queryPreviousTxnId(txnId: TxnIdString): TxnIdString;
    public queryTextureData(opts: TextureLoadProps): Promise<TextureData | undefined>;
    public readFontMap(): FontMapProps;
    public reinstateTxn(): IModelStatus;
    public removeEmbeddedFile(name: string): void;
    public replaceEmbeddedFile(arg: EmbedFileArg): void;
    public resetBriefcaseId(idValue: number): void;
    public restartDefaultTxn(): void;
    public restartTxnSession(): void;
    public currentTxnSessionId(): number;
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
    public setBusyTimeout(ms: number): void;
    public setCodeValueBehavior(newBehavior: "exact" | "trim-unicode-whitespace"): void;
    public simplifyElementGeometry(simplifyArgs: any): IModelStatus;
    public startCreateChangeset(): ChangesetFileProps;
    public startProfiler(scopeName?: string, scenarioName?: string, overrideFile?: boolean, computeExecutionPlan?: boolean): DbResult;
    public stopProfiler(): { rc: DbResult, elapsedTime?: number, scopeId?: number, fileName?: string };
    public enableChangesetStatsTracking(): void;
    public disableChangesetStatsTracking(): void;
    public getChangesetHealthData(changesetId: string): ChangesetHealthStats;
    public getAllChangesetHealthData(): ChangesetHealthStats[];
    public updateElement(elemProps: Partial<ElementProps>): void;
    public updateElementAspect(aspectProps: ElementAspectProps): void;
    public updateElementGeometryCache(props: object): Promise<any>;
    public updateIModelProps(props: IModelProps): void;
    public updateLinkTableRelationship(props: RelationshipProps): DbResult;
    public updateModel(modelProps: ModelProps): void;
    public updateModelGeometryGuid(modelId: Id64String): IModelStatus;
    public updateProjectExtents(newExtentsJson: string): void;
    public writeAffectedElementDependencyGraphToFile(dotFileName: string, changedElems: Id64Array): BentleyStatus;
    public writeFullElementDependencyGraphToFile(dotFileName: string): BentleyStatus;
    public vacuum(arg?: { pageSize?: number, into?: LocalFileName }): void;
    public enableWalMode(yesNo?: boolean): void;
    public performCheckpoint(mode?: WalCheckpointMode): void;
    public setAutoCheckpointThreshold(frames: number): void;

    public pullMergeGetStage(): "None" | "Merging" | "Rebasing";
    public pullMergeRebaseReinstateTxn(): void;
    public pullMergeRebaseUpdateTxn(): void;
    public pullMergeRebaseNext(): TxnIdString | undefined;
    public pullMergeRebaseAbortTxn(): void
    public pullMergeRebaseBegin(): TxnIdString[];
    public pullMergeRebaseEnd(): void;
    public pullMergeReverseLocalChanges(): TxnIdString[];
    public stashChanges(args: { stashRootDir: string, description: string, iModelId: string, resetBriefcase?: true}): any;
    public stashRestore(stashFile: string): void;
    public getPendingTxnsHash(includeReversedTxns: boolean): string;
    public hasPendingSchemaChanges(): boolean;
    public discardLocalChanges(): void;
    public getTxnProps(id: TxnIdString): TxnProps | undefined;
    public setTxnMode(mode: "direct" | "indirect"): void;
    public getTxnMode(): "direct" | "indirect";
    public static enableSharedCache(enable: boolean): DbResult;
    public static getAssetsDir(): string;
    public static zlibCompress(data: Uint8Array): Uint8Array;
    public static zlibDecompress(data: Uint8Array, actualSize: number): Uint8Array;
    public static computeChangesetId(args: Partial<ChangesetFileProps> & Required<Pick<ChangesetFileProps, "parentId" | "pathname">>): string;
  }

  /** The native object for GeoServices. */
  class GeoServices {
    constructor();
    public static getGeographicCRSInterpretation(props: GeographicCRSInterpretRequestProps): GeographicCRSInterpretResponseProps;
    public static getListOfCRS(extent?: Range2dProps, includeWorld?: boolean): Array<{ name: string, description: string, deprecated: boolean, crsExtent: Range2dProps }>;
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

  /**
   * The native object for SchemaUtility
   * @internal
   */
  class SchemaUtility {
    constructor();
    /** Converts given schemas and their reference schemas to EC3.2 schemas */
    public static convertCustomAttributes(xmlSchemas: string[], schemaContext?: ECSchemaXmlContext): string[];
    public static convertEC2XmlSchemas(ec2XmlSchemas: string[], schemaContext?: ECSchemaXmlContext): string[];
  }

  class ECDb implements IDisposable, IConcurrentQueryManager {
    constructor();
    public abandonChanges(): DbResult;
    public closeDb(): void;
    public createDb(dbName: string): DbResult;
    public dispose(): void;
    public dropSchemas(schemaNames: ReadonlyArray<string>): void;
    public schemaSyncSetDefaultUri(syncDbUri: string): void;
    public schemaSyncGetDefaultUri(): string;
    public schemaSyncInit(syncDbUri: string, containerId: string, overrideContainer: boolean): void;
    public schemaSyncPull(syncDbUri: string | undefined): void;
    public schemaSyncPush(syncDbUri: string | undefined): void;
    public schemaSyncEnabled(): boolean;
    public schemaSyncGetLocalDbInfo(): SchemaLocalDbInfo | undefined;
    public schemaSyncGetSyncDbInfo(): SchemaSyncDbInfo | undefined;
    public getFilePath(): string;
    public resolveInstanceKey(args: ResolveInstanceKeyArgs): ResolveInstanceKeyResult;
    public readInstance(key: NodeJS.Dict<any>, args: NodeJS.Dict<any>): NodeJS.Dict<any>;
    public insertInstance(inst: NodeJS.Dict<any>, args: NodeJS.Dict<any>): Id64String;
    public updateInstance(inst: NodeJS.Dict<any>, args: NodeJS.Dict<any>): boolean;
    public deleteInstance(key: NodeJS.Dict<any>, args: NodeJS.Dict<any>): boolean;
    public getSchemaProps(name: string): SchemaProps;
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
    public attachDb(filename: string, alias: string): void;
    public detachDb(alias: string): void;
  }

  class ChangedElementsECDb implements IDisposable {
    constructor();
    public dispose(): void;
    public createDb(db: DgnDb, dbName: string): DbResult;
    public openDb(dbName: string, mode: OpenMode, upgradeProfiles?: boolean): DbResult;
    public isOpen(): boolean;
    public closeDb(): void;
    public processChangesets(db: DgnDb, changesets: ChangesetFileProps[], rulesetId: string, filterSpatial?: boolean, wantParents?: boolean, wantPropertyChecksums?: boolean, rulesetDir?: string, tempDir?: string, wantChunkTraversal?: boolean): DbResult;
    public processChangesetsAndRoll(dbFilename: string, dbGuid: string, changesets: ChangesetFileProps[], rulesetId: string, filterSpatial?: boolean, wantParents?: boolean, wantPropertyChecksums?: boolean, rulesetDir?: string, tempDir?: string, wantRelationshipCaching?: boolean, relationshipCacheSize?: number, wantChunkTraversal?: boolean, wantBoundingBoxes?: boolean): DbResult;
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
    public toRow(arg: ECSqlRowAdaptorOptions): any;
    public getMetadata(): any;
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
    public getOriginPropertyName(): string | undefined;
    public getRootClassAlias(): string;
    public getRootClassName(): string;
    public getRootClassTableSpace(): string;
    public getType(): number;
    public isEnum(): boolean;
    public isGeneratedProperty(): boolean;
    public isSystemProperty(): boolean;
    public isDynamicProp(): boolean;
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
    public embedFontFile(id: number, faces: FontFaceProps[], data: Uint8Array, compress: boolean): void;
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
    public enableWalMode(yesNo?: boolean): void;
    public performCheckpoint(mode?: WalCheckpointMode): void;
    public setAutoCheckpointThreshold(frames: number): void;
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
        tableName: string;
        /** the name of the column for the blob */
        columnName: string;
        /** The rowId of the blob */
        row: number;
        /** If true, open this BlobIO for write access */
        writeable?: boolean;
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
      blob?: ArrayBuffer;
    }): Uint8Array;
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
      blob: ArrayBuffer;
    }): void;
  }

  /** Filter options passed to CloudContainer.queryHttpLog */
  interface BcvHttpLogFilterOptions {
    /** only return rows whose ID is >= the provided id */
    startFromId?: number;
    /** only return rows whose endTime is null OR >= the provided endTime. */
    finishedAtOrAfterTime?: string;
    /** only return rows with a non-null end_time. */
    showOnlyFinished?: boolean;
  }

  interface BcvStatsFilterOptions {
    /** if true, adds activeClients, totalClients, ongoingPrefetches, and attachedContainers to the result. */
    addClientInformation?: boolean;
  }

  /**
   * A cache for storing data from CloudSqlite databases. This object refers to a directory on a local filesystem
   * and is used to **connect** CloudContainers so they may be accessed. The contents of the cache directory are entirely
   * controlled by CloudSqlite and should be empty when the cache is first created and never modified directly. It maintains
   * the state of the local data across sessions.
   * @note All CloudContainers attached to a CloudCache must have the same block size, as determined by the first one connected.
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
    /** destroy this CloudCache. All CloudContainers currently connected are disconnected. */
    public destroy(): void;
  }

  /** A CloudSqlite container that may be connected to a CloudCache. */
  class CloudContainer {
    public onConnect?: (container: CloudContainer, cache: CloudCache) => void;
    public onConnected?: (container: CloudContainer) => void;
    public onDisconnect?: (container: CloudContainer, detach: boolean) => void;
    public onDisconnected?: (container: CloudContainer, detach: boolean) => void;

    public readonly cache?: CloudCache;
    /** Create a new instance of a CloudContainer. It must be connected to a CloudCache for most operations. */
    public constructor(props: NativeCloudSqlite.ContainerAccessProps);
    /** the baseUri of this container */
    public get baseUri(): string;
    /** the storageType of this container */
    public get storageType(): string;
    /** The ContainerId. */
    public get containerId(): string;
    /** The *alias* to identify this CloudContainer in a CloudCache. Usually just the ContainerId. */
    public get alias(): string;
    /** The logId. */
    public get logId(): string;
    /** The time that the write lock expires. Of the form 'YYYY-MM-DDTHH:MM:SS.000Z' in UTC.
     *  Returns empty string if write lock is not held.
     */
    public get writeLockExpires(): string;
    /** true if this CloudContainer is currently connected to a CloudCache via the `connect` method. */
    public get isConnected(): boolean;
    /** true if this CloudContainer was created with the `writeable` flag (and its `accessToken` supplies write access). */
    public get isWriteable(): boolean;
    /** true if this container is public (doesn't require authorization ). */
    public get isPublic(): boolean;
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
    public initializeContainer(opts: { checksumBlockNames?: boolean, blockSize: number }): void;

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
     * Disconnect this CloudContainer from its CloudCache. There must be no open databases from this container.
     */
    public disconnect(args?: {
      /** if true removes the container from the CloudCache, otherwise Leaves the container in the CloudCache so it is available for future sessions. */
      detach?: boolean;
    }): void;

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
     * Create a copy of an existing database within this CloudContainer with a new name.
     * @note CloudSqlite uses copy-on-write semantics for this operation. That is, this method merely makes a
     * new entry in the manifest with the new name that *shares* all of its blocks with the original database.
     * If either database subsequently changes, the only modified blocks are not shared.
     */
    public copyDatabase(dbName: string, toAlias: string): Promise<void>;

    /** Remove a database from this CloudContainer, moving all of its no longer used blocks to the delete list in the manifest.
     * @see [[CloudSqlite.CleanDeletedBlocksJob]] to actually delete the blocks from the delete list.
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
     * query the bcv_http_log table
     * @note the bcv_http_log table contains one row for each HTTP request made by the VFS or connected daemon.
     * @note Entries are automatically removed from the table on a FIFO basis. By default entries which are 1 hr old will be removed.
     */
    public queryHttpLog(filterOptions?: BcvHttpLogFilterOptions): NativeCloudSqlite.BcvHttpLog[];

    /**
     * query the bcv_stat table.
     * @internal
     */
    public queryBcvStats(filterOptions?: BcvStatsFilterOptions): NativeCloudSqlite.BcvStats;

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
  class CancellableCloudSqliteJob {
    /** create an instance of a transfer. The operation begins immediately when the object is created.
     * @param direction either "upload" or "download"
     * @param container the container holding the database. Does *not* require that the container be connected to a CloudCache.
     * @param args The properties for the source and target of the transfer.
     */
    constructor(direction: NativeCloudSqlite.TransferDirection | "cleanup", container: CloudContainer, args: NativeCloudSqlite.TransferDbProps | NativeCloudSqlite.CleanDeletedBlocksOptions);

    /** Cancel a currently pending transfer and cause the promise to be rejected with a Cancelled status.
     * @throws exception if the operation has already completed.
     */
    public cancelTransfer(): void;
    /** Get the current progress of the transfer.
     * @throws exception if the operation has already completed.
     */
    public getProgress(): { loaded: number, total: number };

    /**
     * Only applicable to cleanup jobs. Calling this in a download or upload job will also stop the job but without saving progress.
     * During a cleanup job, if any blocks have been deleted, the job will stop and upload the manifest reflecting which blocks have been deleted.
     */
    public stopAndSaveProgress(): void;

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
    constructor(container: CloudContainer, dbName: string, args?: NativeCloudSqlite.PrefetchProps);

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
    ResultSetTooLarge = Error + 2,
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
  };

  interface ECPresentationManagerProps {
    id: string;
    taskAllocationsMap: { [priority: number]: number };
    defaultFormats: {
      [phenomenon: string]: Array<{
        unitSystems: string[];
        serializedFormat: string;
      }>;
    };
    updateCallback: (updateInfo: any) => void;
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
    public registerSupplementalRuleset(serializedRuleset: string): ECPresentationManagerResponse<string>;
    public getRulesets(rulesetId: string): ECPresentationManagerResponse<string>;
    public addRuleset(serializedRuleset: string): ECPresentationManagerResponse<string>;
    public removeRuleset(rulesetId: string, hash: string): ECPresentationManagerResponse<boolean>;
    public clearRulesets(): ECPresentationManagerResponse<void>;
    public handleRequest(db: DgnDb, options: string): { result: Promise<ECPresentationManagerResponse<Buffer>>, cancel: () => void };
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
    public addSchemaPath(schemaPath: string): void;
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

  class ChangesetReader {
    public close(): void;
    public getColumnCount(): number;
    public getColumnValue(col: number, stage: number): Uint8Array | number | string | null | undefined;
    public getColumnValueBinary(col: number, stage: number): Uint8Array | undefined;
    public getColumnValueDouble(col: number, stage: number): number | undefined;
    public getColumnValueId(col: number, stage: number): string | undefined;
    public getColumnValueInteger(col: number, stage: number): number | undefined;
    public getColumnValueText(col: number, stage: number): string | undefined;
    public getColumnValueType(col: number, stage: number): number | undefined;
    public getDdlChanges(): string | undefined;
    public getOpCode(): DbOpcode;
    public getPrimaryKeys(): (Uint8Array | number | string | null | undefined)[];
    public getRow(stage: number): (Uint8Array | number | string | null | undefined)[];
    public getTableName(): string;
    public hasRow(): boolean;
    public isColumnValueNull(col: number, stage: number): boolean | undefined;
    public isIndirectChange(): boolean;
    public getPrimaryKeyColumnIndexes(): number[];
    public openFile(fileName: string, invert: boolean): void;
    public openGroup(fileName: string[], db: AnyECDb, invert: boolean): void;
    public openLocalChanges(db: DgnDb, includeInMemoryChanges: boolean, invert: boolean): void;
    public openInMemoryChanges(db: DgnDb, invert: boolean): void;
    public openTxn(db: DgnDb, txnId: Id64String, invert: boolean): void;
    public reset(): void;
    public step(): boolean;
    public writeToFile(fileName: string, containsSchemaChanges: boolean, overrideFile: boolean): void;
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
    public static emitLogs(count: number, category: string, severity: LogLevel, thread: "main" | "worker", onDone: () => void): void;
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
