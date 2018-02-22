/*---------------------------------------------------------------------------------------------
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 *--------------------------------------------------------------------------------------------*/
import { IModelStatus, StatusCodeWithMessage, RepositoryStatus } from "@bentley/bentleyjs-core/lib/BentleyError";
import { BentleyStatus, ChangeSetProcessOption } from "@bentley/bentleyjs-core/lib/Bentley";
import { DbResult, DbOpcode } from "@bentley/bentleyjs-core/lib/BeSQLite";
import { OpenMode } from "@bentley/bentleyjs-core/lib/BeSQLite";
import { IDisposable } from "@bentley/bentleyjs-core/lib/Disposable";

/* The primary key for the DGN_TABLE_Txns table. */
interface AddonTxnId {
    readonly _id: string;
}

/* The signature of a callback that takes two arguments, the first being the error that describes a failed outcome and the second being the data
returned in a successful outcome. */
interface IModelJsAddonCallback<ERROR_TYPE, SUCCESS_TYPE> {
  /**
   * The signature of a callback.
   * @param error A description of th error, in case of failure.
   * @param result The result of the operation, in case of success.
   */
  (error: ERROR_TYPE, result: SUCCESS_TYPE): void;
}

/* The signature of a callback that expects a single argument, a status code. */
interface IModelJsAddonStatusOnlyCallback<STATUS_TYPE> {
  /**
   * The signature of a callback.
   * @param error A description of th error, in case of failure.
   */
  (error: STATUS_TYPE): void;
}

/**
 * The return type of synchronous functions that may return an error or a successful result.
 */
interface ErrorStatusOrResult<ErrorCodeType, ResultType> {
    /** Error from the operation. This property is defined if and only if the operation failed. */
    error?: StatusCodeWithMessage<ErrorCodeType>;

    /** Result of the operation. This property is defined if the operation completed successfully */
    result?: ResultType;
}

/**
 * A request to send on to iModelHub.
 */
declare class AddonBriefcaseManagerResourcesRequest {

    /**
     * Forget the requests.
     */
    reset(): void;

    /** Contains no requests? */
    isEmpty(): boolean;

    /** Get the request in JSON format */
    toJSON(): string;
}

/** How to handle a conflict 
export const enum AddonBriefcaseManagerOnConflict {
    // Reject the incoming change
    RejectIncomingChange = 0,
    // Accept the incoming change
    AcceptIncomingChange = 1,
}
*/

/** The options for how conflicts are to be handled during change-merging in an OptimisticConcurrencyControlPolicy.
 * The scenario is that the caller has made some changes to the *local* briefcase. Now, the caller is attempting to
 * merge in changes from iModelHub. The properties of this policy specify how to handle the *incoming* changes from iModelHub.
 */
export interface AddonBriefcaseManagerOnConflictPolicy {
    /** What to do with the incoming change in the case where the same entity was updated locally and also would be updated by the incoming change. */
    updateVsUpdate: /*AddonBriefcaseManagerOnConflict*/number;
    /** What to do with the incoming change in the case where an entity was updated locally and would be deleted by the incoming change. */
    updateVsDelete: /*AddonBriefcaseManagerOnConflict*/number;
    /** What to do with the incoming change in the case where an entity was deleted locally and would be updated by the incoming change. */
    deleteVsUpdate: /*AddonBriefcaseManagerOnConflict*/number;
}

/**
 * The AddonDgnDb class that is projected by the iModelJs node addon. 
 */
declare class AddonDgnDb {
  constructor();

  /**
   * Get information on all briefcases cached on disk
   * @param cachePath Path to the root of the disk cache
   */
  getCachedBriefcaseInfos(cachePath: string): ErrorStatusOrResult<DbResult, string>;

  /** Get the IModelProps of this iModel. */
  getIModelProps(): string;

  /** 
   * Create a local iModel. 
   * @param dbName The full path to the iModel in the local file system
   * @param rootSubjectName Name of the root subject
   * param rootSubjectDescription Description of the root subject
   */
  createDgnDb(dbName: string, rootSubjectName: string, rootSubjectDescription?: string): DbResult;
 
  /**
   * Open a local iModel.
   * @param dbName The full path to the iModel in the local file system
   * @param mode The open mode
   * @return non-zero error status if operation failed.
   */
  openDgnDb(dbName: string, mode: OpenMode): DbResult;

  /** Close this iModel. */
  closeDgnDb(): void;

  /**
   * Process change sets
   * @param cachePath Path to the root of the disk cache
   */
  processChangeSets(changeSets: string, processOptions: ChangeSetProcessOption): DbResult;

  /**
   * Start creating a new change set with local changes
   */
  startCreateChangeSet(): ErrorStatusOrResult<DbResult, string>;

  /**
   * Start creating a new change set with local changes
   */
  finishCreateChangeSet(): DbResult;

  /** Creates an EC change cache for this iModel (but does not attach it). 
   * @param changeCacheFile The created change cache ECDb file
   * @param changeCachePath The full path to the EC change cache file in the local file system
   * @return non-zero error status if operation failed.
  */
  createChangeCache(changeCacheFile: AddonECDb, changeCachePath: string) : DbResult;

  /** Attaches an EC change cache file to this iModel. 
   * @param changeCachePath The full path to the EC change cache file in the local file system
   * @return non-zero error status if operation failed.
  */
  attachChangeCache(changeCachePath: string) : DbResult;

  /** Determines whether the EC Changes cache file is attached to this iModel. 
   * @return true if the changes cache is attached. false otherwise
  */
  isChangeCacheAttached() : boolean;

  /** Extracts a change summary from the specified Changeset file
   * @param changeCacheFile The change cache ECDb file where the extracted change summary will be persisted
   * @param changesetFilePath The full path to the SQLite changeset file in the local file system
   * @return The ChangeSummary ECInstanceId as hex string or error codes in case of failure
  */
  extractChangeSummary(changeCacheFile: AddonECDb, changesetFilePath: string): ErrorStatusOrResult<DbResult, string>;
  
  /**
   * Set the briefcase Id of this iModel.
   * @param idvalue The briefcase Id value.
   */
  setBriefcaseId(idvalue: number): DbResult;

  /** Get the briefcase Id of this iModel. */
  getBriefcaseId(): number;

  /**
   * Get the change set the iModel was reversed to
   * @return Returns the change set id if the iModel was reversed, or undefined if the iModel was not reversed.
   */
  getReversedChangeSetId(): string|undefined;

  /**
   * Get the Id of the last change set that was merged into or created from the Db. This is the parent for any new change sets that will be created from the iModel.
   * @return Returns an empty string if the iModel is in it's initial state (with no change sets), or if it's a standalone briefcase disconnected from the Hub.
   */
  getParentChangeSetId(): string;

  /* Get the GUID of this iModel */
  getDbGuid(): string;

  /* Set the GUID of this iModel */
  setDbGuid(guid: string): DbResult;

  /**
   * Sets up a briefcase
   * @param briefcaseToken Token identifying a briefcase
   */
  setupBriefcase(briefcaseToken: string): DbResult;

  /** 
   * Save any pending changes to this iModel.  
   * @param description optional description of changes
   * @return non-zero error status if save failed. 
   */
  saveChanges(description?: string): DbResult;

  /** Abandon changes
  * @return non-zero error status if operation failed.
  */
  abandonChanges(): DbResult;

  /**
   * Import an EC schema.
   * There are a number of restrictions when importing schemas into a briefcase. 
   * When importing into a briefcase, this function will acquire the schema lock. That means that that briefcase must be at the tip of the revision
   * history in iModelHub. If not, this function will return SchemaLockFailed.
   * Importing or upgrading a schema into a briefcase must be done in isolation from all other kinds of changes. That means two things:
   * there must be no pending local changes. All local changes must be pushed to iModelHub. This function will return SchemaImportFailed if that is not true.
   * Also, the caller must push the results of this function to iModelHub before making other changes to the briefcase.
   * @param schemaPathname The full path to the .xml file in the local file system.
   * @return non-zero error status if the operation failed, including SchemaImportFailed if the schema is invalid.
   */
  importSchema(schemaPathname: string): DbResult;

  /**
   * Get an element's properties
   * @param opts Identifies the element
   * @param In case of success, the result property of the returned object will be the element's properties in stringified JSON format.
   */
  getElement(opts: string): ErrorStatusOrResult<IModelStatus, string>;

  /**
   * Get the properties of a Model
   * @param opts Identifies the model
   * @param In case of success, the result property of the returned object will be the model's properties in stringified JSON format.
   */
  getModel(opts: string): ErrorStatusOrResult<IModelStatus, string>;

  /**
   * Insert an element.
   * @param elemProps The element's properties, in stringified JSON format.
   * @return In case of success, the result property of the returned object will be the element's ID (as a hex string)
   */
  insertElement(elemProps: string): ErrorStatusOrResult<IModelStatus, string>;

  /**
   * Update an element.
   * @param elemProps The element's properties, in stringified JSON format.
   * @return non-zero error status if the operation failed.
   */
  updateElement(elemProps: string): IModelStatus;

  /**
   * Delete an element from this iModel.
   * @param elemIdJson The element's Id, in stringified JSON format
   * @return non-zero error status if the operation failed.
   */
  deleteElement(elemIdJson: string): IModelStatus;

  /**
   * Insert a LinkTableRelationship.
   * @param props The linkTableRelationship's properties, in stringified JSON format.
   * @return In case of success, the result property of the returned object will be the ID of the new LinkTableRelationship instance (as a hex string)
   */
  insertLinkTableRelationship(props: string): ErrorStatusOrResult<DbResult, string>;

  /**
   * Update a LinkTableRelationship.
   * @param props The LinkTableRelationship's properties, in stringified JSON format.
   * @return non-zero error status if the operation failed.
   */
  updateLinkTableRelationship(props: string): DbResult;

  /**
   * Delete a LinkTableRelationship.
   * @param props The LinkTableRelationship's properties, in stringified JSON format. Only classFullName and id are required.
   * @return non-zero error status if the operation failed.
   */
  deleteLinkTableRelationship(props: string): DbResult;

  /**
   * Insert a new CodeSpec
   * @param name name of the CodeSpec
   * @param specType must be one of CodeScopeSpec::Type
   * @param scopeReq must be one of CodeScopeSpec::ScopeRequirement
   * @return In case of success, the result property of the returned object will be the ID of the new CodeSpec instance (as a hex string)
   */
  insertCodeSpec(name: string, specType: number, scopeReq: number): ErrorStatusOrResult<IModelStatus, string>;

  /**
   * Insert a model.
   * @param modelProps The model's properties, in stringified JSON format.
   * @return In case of success, the result property of the returned object will be the ID of the new Model (as a hex string)
   */
  insertModel(modelProps: string): ErrorStatusOrResult<IModelStatus, string>;

  /**
   * Update a model.
   * @param modelProps The model's properties, in stringified JSON format.
   * @return non-zero error status if the operation failed.
   */
  updateModel(modelProps: string): IModelStatus;

  /**
   * Delete a model.
   * @param modelIdJson The model's Id, in stringified JSON format
   * @return non-zero error status if the operation failed.
   */
  deleteModel(modelIdJson: string): IModelStatus;

  /**
   * Update the imodel project extents.
   * @param newExtentsJson The new project extents in stringified JSON format
   */
  updateProjectExtents(newExtentsJson: string): void;

  /**
   * Format an element's properties, suitable for display to the user.
   * @param id The element's Id, in stringified JSON format
   * @param on success, the result property of the returned object will be the object's properties, in stringified JSON format.
   */
  getElementPropertiesForDisplay(id: string): ErrorStatusOrResult<IModelStatus, string>;

  /**
   * Get information about an ECClass
   * @param schema The name of the ECSchema
   * @param className The name of the ECClass
   * @param on success, the result property of the returned object will be an object containing the properties of the class, in stringified JSON format.
   */
  getECClassMetaData(schema: string, className: string): ErrorStatusOrResult<IModelStatus, string>;

    /**
    * Add the lock, code, and other resource request that would be needed in order to carry out the specified operation.
    * @param req The request object, which accumulates requests.
    * @param elemId The ID of an existing element or the {modelid, code} properties that specify a new element.
    * @param opcode The operation that will be performed on the element.
    */  
    buildBriefcaseManagerResourcesRequestForElement(req: AddonBriefcaseManagerResourcesRequest, elemId: string, opcode: DbOpcode): RepositoryStatus;

    /**
    * Add the lock, code, and other resource request that would be needed in order to carry out the specified operation.
    * @param req The request object, which accumulates requests.
    * @param modelId The ID of a model
    * @param opcode The operation that will be performed on the model.
    */
    buildBriefcaseManagerResourcesRequestForModel(req: AddonBriefcaseManagerResourcesRequest, modelId: string, opcode: DbOpcode): RepositoryStatus;

    /**
    * Add the resource request that would be needed in order to carry out the specified operation.
    * @param req The request object, which accumulates requests.
    * @param relKey Identifies a LinkTableRelationship: {classFullName, id}
    * @param opcode The operation that will be performed on the LinkTableRelationships.
    */
    buildBriefcaseManagerResourcesRequestForLinkTableRelationship(req: AddonBriefcaseManagerResourcesRequest, relKey: string, opcode: DbOpcode): RepositoryStatus;

    /**
     * Extract requests from the current bulk operation and append them to reqOut
     * @param req The pending requests.
     * @param locks Extract lock requests?
     * @param codes Extract Code requests?
     */
    extractBulkResourcesRequest(req: AddonBriefcaseManagerResourcesRequest, locks: boolean, codes: boolean): void;

    /**
     * Extract requests from reqIn and append them to reqOut
     * @param reqOut The output request
     * @param reqIn The input request
     * @param locks Extract lock requests?
     * @param codes Extract Code requests?
     */
    extractBriefcaseManagerResourcesRequest(reqOut: AddonBriefcaseManagerResourcesRequest, reqIn: AddonBriefcaseManagerResourcesRequest, locks: boolean, codes: boolean): void;

    /**
     * Append reqIn to reqOut
     * @param reqOut The request to be augmented
     * @param reqIn The request to read
     */
    appendBriefcaseManagerResourcesRequest(reqOut: AddonBriefcaseManagerResourcesRequest, reqIn: AddonBriefcaseManagerResourcesRequest): void;

    /** Start bulk update mode. Valid only with the pessimistic concurrency control policy */
    briefcaseManagerStartBulkOperation(): RepositoryStatus;

    /** End bulk update mode. This will wait for locks and codes. Valid only with the pessimistic concurrency control policy */
    briefcaseManagerEndBulkOperation(): RepositoryStatus;

    /** Check if there is a bulk operation in progress. */
    inBulkOperation(): boolean;

    /**
     *  The the pessimistic concurrency control policy.
     */
    setBriefcaseManagerPessimisticConcurrencyControlPolicy(): RepositoryStatus;

    /** Set the optimistic concurrency control policy.
     * @param policy The policy to used
     * @return non-zero if the policy could not be set
     */
    setBriefcaseManagerOptimisticConcurrencyControlPolicy(conflictPolicy: AddonBriefcaseManagerOnConflictPolicy): RepositoryStatus;
    
    /** Query the ID of the first entry in the local Txn table, if any. */
    txnManagerQueryFirstTxnId(): AddonTxnId;
    /** Query the ID of the entry in the local Txn table that comes after the specified Txn, if any. */
    txnManagerQueryNextTxnId(txnId: AddonTxnId): AddonTxnId;
    /** Query the ID of the entry in the local Txn table that comes before the specified Txn, if any. */
    txnManagerQueryPreviousTxnId(txnId: AddonTxnId): AddonTxnId;
    /** Query the ID of the most recent entry in the local Txn table, if any. */
    txnManagerGetCurrentTxnId(): AddonTxnId;
    /** Get the description of the specified Txn. */
    txnManagerGetTxnDescription(txnId: AddonTxnId): string;
    /** Check if the specified TxnId is valid. The above query functions will return an invalid ID to indicate failure. */
    txnManagerIsTxnIdValid(txnId: AddonTxnId): boolean;
    /** Check if there are un-saved changes in memory. */
    txnManagerHasUnsavedChanges(): boolean;

  /**
   * Execute a test by name
   * @param testName The name of the test to execute
   * @param params A JSON string with the parameters for the test
   */
  executeTest(testName: string, params: string): any;
}

/* The AddonECDb class that is projected by the iModelJs node addon. */
declare class AddonECDb implements IDisposable {
    constructor();
     /**
     * Create a new ECDb.
     * @param dbname The full path to the ECDb in the local file system
     * @return non-zero error status if operation failed.
     */
    createDb(dbname : string): DbResult;

     /** Open a existing ECDb.
     * @param dbname The full path to the ECDb in the local file system
     * @param mode The open mode
     * @return non-zero error status if operation failed.
     */
    openDb(dbname : string, mode:OpenMode): DbResult;

     /** Check to see if connection to ECDb is open or not.
     * @return true if connection is open otherwise false.
     */
    isOpen(): boolean;
    
     /** Check to see if connection to ECDb is open or not.
     * @return true if connection is open otherwise false.
     */
    closeDb(): void;

    /** Dispose of the native ECDb object. */
    dispose(): void;

     /** Save changes to ecdb
     * @param changesetName The name of the operation that generated these changes. If transaction tracking is enabled.
     * @return non-zero error status if operation failed.
     */
    saveChanges(changesetName?:string): DbResult;

     /** Abandon changes
     * @return non-zero error status if operation failed.
     */
    abandonChanges(): DbResult;

     /** Import ECSchema into ECDb
     * @param schemaPathName Path to ECSchema file on disk. All reference schema should also be present on same path. 
     * @return non-zero error status if operation failed.
     */
    importSchema(schemaPathName:string): DbResult;
}

/* The AddonECSqlStatement class that is projected by the iModelJs node addon. */
declare class AddonECSqlStatement implements IDisposable {
    constructor();

    /**
     * Prepare an ECSQL statement.
     * @param db The native DgnDb object
     * @param ecsql The ECSQL to prepare
     * @return Returns the Zero status in case of success. Non-zero error status in case of failure. The error's message property will contain additional information.
     */
    prepare(db: AddonDgnDb | AddonECDb, ecsql: string): StatusCodeWithMessage<DbResult>;

    /** Reset the statement to just before the first row.
     * @return Returns non-zero error status in case of failure.
     */
    reset(): DbResult;

    /** Dispose of the native ECSqlStatement object - call this when finished stepping a statement, but only if the statement is not shared. */
    dispose(): void;

    /**
     * Gets a binder for the specified parameter. It can be used to bind any type of values to the parameter.
     * @param param Index (1-based) or name (without leading colon) of the parameter.
     * @return Returns the binder for the specified parameter
     */
    getBinder(param: number | string): AddonECSqlBinder;

    /** Clear the bindings of this statement. See bindValues.
     * @return Returns a non-zero error status in case of failure.
     */
    clearBindings(): DbResult;

    /** Step this statement to move to the next row.
     * @return Returns BE_SQLITE_ROW if the step moved to a new row. Returns BE_SQLITE_DONE if the step failed because there is no next row. Another non-zero error status if step failed because of an error.
    */
    step(): DbResult;

    /** Step this INSERT statement and returns the status along with the ECInstanceId of the newly inserted row.
     * @return Returns BE_SQLITE_DONE if the insert was successful. Returns another non-zero error status if step failed because of an error.
    */
    stepForInsert(): { status: DbResult, id: string };

    /** 
    * Get the value of the specified column for the current row
    * @param columnIndex Index (0-based) of the column in the ECSQL SELECT clause for which the value is to be retrieved.
    * @return Returns the ECSQL value of the specified column for the current row
    */
    getValue(columnIndex: number): AddonECSqlValue;

    /** 
    * Get the number of ECSQL columns in the result set after calling step on a SELECT statement.
    * @return Returns the ECSQL value of the specified column for the current row
    */
    getColumnCount(): number;
}

/* The AddonECSqlBinder class that is projected by the iModelJs node addon. */
declare class AddonECSqlBinder implements IDisposable {
    constructor();

    /** Dispose of the AddonECSqlBinder object */
    dispose(): void;

    /** Binds null to the parameter represented by this binder
     * @return non-zero error status in case of failure.
     */
    bindNull(): DbResult;

    /** Binds a BLOB, formatted as Base64 string, to the parameter represented by this binder
     * @return non-zero error status in case of failure.
     */
    bindBlob(base64String: string): DbResult;

    /** Binds a Boolean to the parameter represented by this binder
     * @return non-zero error status in case of failure.
     */
    bindBoolean(val: boolean): DbResult;

    /** Binds a DateTime, formatted as ISO string, to the parameter represented by this binder
     * @return non-zero error status in case of failure.
     */
    bindDateTime(isoString: string): DbResult;

    /** Binds a double to the parameter represented by this binder
     * @return non-zero error status in case of failure.
     */
    bindDouble(val: number): DbResult;

    /** Binds an Id, formatted as hexadecimal string, to the parameter represented by this binder
     * @return non-zero error status in case of failure.
     */
    bindId(hexStr: string): DbResult;

    /** Binds an int to the parameter represented by this binder
     * @param val Integral value, either as number or as decimal or hexadecimal string (for the case
     * where the integer is larger than the JS accuracy threshold)
     * @return non-zero error status in case of failure.
     */
    bindInteger(val: number | string): DbResult;

    /** Binds a Point2d to the parameter represented by this binder.
     * @return non-zero error status in case of failure.
     */
    bindPoint2d(x: number, y: number): DbResult;

    /** Binds a Point3d to the parameter represented by this binder.
     * @return non-zero error status in case of failure.
     */
    bindPoint3d(x: number, y: number, z: number): DbResult;

    /** Binds a string to the parameter represented by this binder.
     * @return non-zero error status in case of failure.
     */
    bindString(val: string): DbResult;

    /** Binds a Navigation property value to the parameter represented by this binder.
     * @param navIdHexStr Id of the related instance represented by the navigation property (formatted as hexadecimal string)
     * @param relClassName Name of the relationship class of the navigation property (can be undefined if it is not mandatory)
     * @param relClassTableSpace In case the relationship of the navigation property is persisted in an attached ECDb file, specify the table space.
                                 If undefined, ECDb will first look in the primary file and then in the attached ones.
     * @return non-zero error status in case of failure.
     */
    bindNavigation(navIdHexStr: string, relClassName?: string, relClassTableSpace?: string): DbResult;

     /** Gets a binder for the specified member of a struct parameter
     * @return Struct member binder.
     */
    bindMember(memberName: string): AddonECSqlBinder;

     /** Adds a new array element to the array parameter and returns the binder for the new array element
     * @return Binder for the new array element.
     */
    addArrayElement(): AddonECSqlBinder;
}


/* The AddonECSqlColumnInfo class that is projected by the iModelJs node addon. 
   @remarks No need to dispose this is its native counterpart is owned by the IECSqlValue. */
declare class AddonECSqlColumnInfo {
    constructor();

    /** Gets the data type of the column. 
     *  @returns one of the values of the enum ECSqlValueType values, defined in imodeljs-core/common.
     *  (enums cannot be defined in the addon)
     */
    getType(): number;

    /** Gets the name of the property backing the column.
     * @remarks If this column is backed by a generated property, i.e. it represents ECSQL expression,
     * the access string consists of the name of the generated property.
     */
    getPropertyName(): string;

    /** Gets the full access string to the corresponding ECSqlValue starting from the root class.
     * @remarks If this column is backed by a generated property, i.e. it represents ECSQL expression,
     * the access string consists of the ECSQL expression.
     */
    getAccessString(): string;

    /** Indicates whether the column refers to a system property (e.g. id, className) backing the column. */
    isSystemProperty(): boolean;

    /** Indicates whether the column is backed by a generated property or not. For SELECT clause items that are expressions other
     * than simply a reference to an ECProperty, a property is generated containing the expression name.
     */
    isGeneratedProperty(): boolean;

    /** Gets the table space in which this root class is persisted.
     * @remarks for classes in the primary file the table space is MAIN. For classes in attached
     * files, the table space is the name by which the file was attached (see BentleyApi::BeSQLite::Db::AttachDb)
     * For generated properties the table space is empty */
    getRootClassTableSpace(): string;

    /** Gets the fully qualified name of the ECClass of the top-level ECProperty backing this column. */
    getRootClassName(): string;

    /** Gets the class alias of the root class to which the column refers to.
     * @returns Returns the alias of root class the column refers to or an empty string if no class alias was specified in the select clause */
    getRootClassAlias(): string;
}

/* The AddonECSqlValue class that is projected by the iModelJs node addon. */
declare class AddonECSqlValue implements IDisposable {
    constructor();

    dispose(): void;

    /** Get information about the ECSQL SELECT clause column this value refers to. */
    getColumnInfo(): AddonECSqlColumnInfo;

    isNull(): boolean;
    /** Get value as a BLOB, formatted as Base64-encoded string. */
    getBlob(): string;
    /** Get value as boolean. */
    getBoolean(): boolean;
    /** Get value as date time, formatted as ISO8601 string. */
    getDateTime(): string;
    /** Get value as double. */
    getDouble(): number;
    /** Get value as IGeometry formatted as JSON. */
    getGeometry(): string;
    /** Get value as id, formatted as hexadecimal string. */
    getId(): string;
    /** If this ECSqlValue represents a class id, this method returns the fully qualified class name. */
    getClassNameForClassId(): string;
    /** Get value as int. */
    getInt(): number;
    /** Get value as int64. This method does not deal with JS accuracy issues of int64 values greater than 2^53. */
    getInt64(): number;
    /** Get value as Point2d. */
    getPoint2d(): { x: number, y: number };
    /** Get value as Point3d. */
    getPoint3d(): { x: number, y: number, z: number};
    /** Get value as string. */
    getString(): string;
    /** Get value as Navigation property value. */
    getNavigation(): { id: string, relClassName?: string };

    /** Get an iterator for iterating the struct members of this struct value. */
    getStructIterator(): AddonECSqlValueIterator;
    /** Get an iterator for iterating the array elements of this array value. */
    getArrayIterator(): AddonECSqlValueIterator;
}

/* The AddonECSqlValueIterator class that is projected by the iModelJs node addon. */
declare class AddonECSqlValueIterator implements IDisposable {
    constructor();
    dispose(): void;
    /**
     * Move the iterator to the next ECSqlValue.
     * @returns Returns true if the iterator now points to the next element. Returns false if the iterator reached the end.
     */
    moveNext(): boolean;
    /**
     * Get the ECSqlValue the iterator is currently pointing to.
     */
    getCurrent(): AddonECSqlValue;
}

/* The AddonECPresentationManager class that is projected by the iModelJs node addon. */
declare class AddonECPresentationManager {
    constructor();
    /**
     * Handles an ECPresentation manager request
     * @param db The db to run the request on
     * @param options Serialized JSON object that contains parameters for the request
     * @return Serialized JSON response
     */
    handleRequest(db: AddonDgnDb, options: string): string;
    /**
     * Sets up a ruleset locater that looks for rulesets in the specified directories
     * @param directories Ruleset locations
     */
    setupRulesetDirectories(directories: string[]): void;
}