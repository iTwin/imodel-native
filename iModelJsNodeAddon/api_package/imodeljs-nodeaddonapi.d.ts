/*---------------------------------------------------------------------------------------------
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
 *--------------------------------------------------------------------------------------------*/
import { IModelStatus, StatusCodeWithMessage, RepositoryStatus } from "@bentley/bentleyjs-core/lib/BentleyError";
import { BentleyStatus } from "@bentley/bentleyjs-core/lib/Bentley";
import { DbResult, DbOpcode } from "@bentley/bentleyjs-core/lib/BeSQLite";
import { OpenMode } from "@bentley/bentleyjs-core/lib/BeSQLite";
import { IDisposable } from "@bentley/bentleyjs-core/lib/Disposable";

/* The signature of a callback that takes two arguments, the first being the error that describes a failed outcome and the second being the data
returned in a successful outcome. */
interface IModelJsNodeAddonCallback<ERROR_TYPE, SUCCESS_TYPE> {
  /**
   * The signature of a callback.
   * @param error A description of th error, in case of failure.
   * @param result The result of the operation, in case of success.
   */
  (error: ERROR_TYPE, result: SUCCESS_TYPE): void;
}

/* The signature of a callback that expects a single argument, a status code. */
interface IModelJsNodeAddonStatusOnlyCallback<STATUS_TYPE> {
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
declare class NodeAddonBriefcaseManagerResourcesRequest {

    /**
     * Forget the requests.
     */
    reset(): void;

    /** Contains no requests? */
    isEmpty(): boolean;

    /** Get the request in JSON format */
    toJSON(): string;
}

/** How to handle a conflict */
export const enum NodeAddonBriefcaseManagerConflictResolution {
    /** Reject the incoming change */
    Reject = 0,
    /** Accept the incoming change */
    Take = 1,
}

/** The options for how conflicts are to be handled during change-merging in an OptimisticConcurrencyControlPolicy.
 * The scenario is that the caller has made some changes to the *local* briefcase. Now, the caller is attempting to
 * merge in changes from iModelHub. The properties of this policy specify how to handle the *incoming* changes from iModelHub.
 */
export interface NodeAddonBriefcaseManagerConflictResolutionPolicy {
    /** What to do with the incoming change in the case where the same entity was updated locally and also would be updated by the incoming change. */
    updateVsUpdate: /*NodeAddonBriefcaseManagerConflictResolution*/number;
    /** What to do with the incoming change in the case where an entity was updated locally and would be deleted by the incoming change. */
    updateVsDelete: /*NodeAddonBriefcaseManagerConflictResolution*/number;
    /** What to do with the incoming change in the case where an entity was deleted locally and would be updated by the incoming change. */
    deleteVsUpdate: /*NodeAddonBriefcaseManagerConflictResolution*/number;
}

/**
 * The NodeAddonDgnDb class that is projected by the iModelJs node addon. 
 */
declare class NodeAddonDgnDb {
  constructor();

  /**
   * Get information on all briefcases cached on disk
   * @param cachePath Path to the root of the disk cache
   */
  getCachedBriefcaseInfos(cachePath: string): ErrorStatusOrResult<DbResult, string>;

  /** Get the IModelProps of this iModel. */
  getIModelProps(): string;

  /**
   * Open a local iModel.
   * @param dbname The full path to the iModel in the local file system
   * @param mode The open mode
   * @return non-zero error status if operation failed.
   */
  openDgnDb(dbname: string, mode: OpenMode): DbResult;

  /** Close this iModel. */
  closeDgnDb(): void;

  /** Creates an EC change cache for this iModel (but does not attach it). 
   * @param changeCacheFile The created change cache ECDb file
   * @param changeCachePath The full path to the EC change cache file in the local file system
   * @return non-zero error status if operation failed.
  */
  createChangeCache(changeCacheFile: NodeAddonECDb, changeCachePath: string) : DbResult;

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
  extractChangeSummary(changeCacheFile: NodeAddonECDb, changesetFilePath: string): ErrorStatusOrResult<DbResult, string>;
  
  /**
   * Set the briefcase Id of this iModel.
   * @param idvalue The briefcase Id value.
   */
  setBriefcaseId(idvalue: number): DbResult;

  /** Get the briefcase Id of this iModel. */
  getBriefcaseId(): number;

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
   * TBD
   * @param briefcaseToken TBD
   * @param changeSetTokens TBD
   * @param revisionUpgradeOptions TBD
   */
  openBriefcase(briefcaseToken: string, changeSetTokens: string, revisionUpgradeOptions?:number): DbResult;

  /** 
   * Save any pending changes to this iModel.  
   * @param description optional description of changes
   * @return non-zero error status if save failed. 
   */
  saveChanges(description?: string): DbResult;

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
    buildBriefcaseManagerResourcesRequestForElement(req: NodeAddonBriefcaseManagerResourcesRequest, elemId: string, opcode: DbOpcode): RepositoryStatus;

    /**
    * Add the lock, code, and other resource request that would be needed in order to carry out the specified operation.
    * @param req The request object, which accumulates requests.
    * @param modelId The ID of a model
    * @param opcode The operation that will be performed on the model.
    */
    buildBriefcaseManagerResourcesRequestForModel(req: NodeAddonBriefcaseManagerResourcesRequest, modelId: string, opcode: DbOpcode): RepositoryStatus;

    /**
    * Add the resource request that would be needed in order to carry out the specified operation.
    * @param req The request object, which accumulates requests.
    * @param codeSpecId The ID of an existing CodeSpec or {} for a new CodeSpec
    * @param opcode The operation that will be performed on the CodeSpec.
    */
    buildBriefcaseManagerResourcesRequestForCodeSpec(req: NodeAddonBriefcaseManagerResourcesRequest, codeSpecId: string, opcode: DbOpcode): RepositoryStatus;

    /**
    * Add the resource request that would be needed in order to carry out the specified operation.
    * @param req The request object, which accumulates requests.
    * @param relKey Identifies a LinkTableRelationship: {classFullName, id}
    * @param opcode The operation that will be performed on the LinkTableRelationships.
    */
    buildBriefcaseManagerResourcesRequestForLinkTableRelationship(req: NodeAddonBriefcaseManagerResourcesRequest, relKey: string, opcode: DbOpcode): RepositoryStatus;

    /** Start bulk update mode. Valid only with the pessimistic concurrency control policy */
    briefcaseManagerStartBulkOperation(): RepositoryStatus;

    /** End bulk update mode. This will wait for locks and codes. Valid only with the pessimistic concurrency control policy */
    briefcaseManagerEndBulkOperation(): RepositoryStatus;

    /**
     *  The the pessimistic concurrency control policy.
     */
    setBriefcaseManagerPessimisticConcurrencyControlPolicy(): RepositoryStatus;

    /** Set the optimistic concurrency control policy.
     * @param policy The policy to used
     * @return non-zero if the policy could not be set
     */
    setBriefcaseManagerOptimisticConcurrencyControlPolicy(conflictPolicy: NodeAddonBriefcaseManagerConflictResolutionPolicy): RepositoryStatus;
    
  /**
   * Execute a test known to exist using the id recognized by the addon's test execution handler
   * @param id The id of the test you wish to execute
   * @param params A JSON string that should all of the data/parameters the test needs to function correctly
   */
  executeTestById(id: number, params: string): any;

}

/* The NodeAddonECDb class that is projected by the iModelJs node addon. */
declare class NodeAddonECDb implements IDisposable {
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

/* The NodeAddonECSqlStatement class that is projected by the iModelJs node addon. */
declare class NodeAddonECSqlStatement implements IDisposable {
    constructor();

    /**
     * Prepare an ECSql statement.
     * @param db The native DgnDb object
     * @param ecSql The statement to prepare
     * @return Zero status in case of success. Non-zero error status in case of failure. The error's message property will contain additional information.
     */
    prepare(db: NodeAddonDgnDb | NodeAddonECDb, ecSql: string): StatusCodeWithMessage<DbResult>;

    /** Reset the statement to just before the first row.
     * @return non-zero error status in case of failure.
     */
    reset(): DbResult;

    /** Dispose of the native ECSqlStatement object - call this when finished stepping a statement, but only if the statement is not shared. */
    dispose(): void;

    getBinder(param: number | string): NodeAddonECSqlBinder;

    /** Clear the bindings of this statement. See bindValues.
     * @return non-zero error status in case of failure.
     */
    clearBindings(): DbResult;

    /**
     * Bind one or more values to placeholders in this ECSql statement.
     * @param valuesJson The values to bind in stringified JSON format. The values must be an array if the placeholders are positional, or an any object with properties if the placeholders are named.
     * @return Zero status in case of success. Non-zero error status in case of failure. The error's message property will contain additional information.
     */
    bindValues(valuesJson: string): StatusCodeWithMessage<DbResult>;

    /** Step this statement to move to the next row.
     * @return BE_SQLITE_ROW if the step moved to a new row. BE_SQLITE_DONE if the step failed because there is no next row. Another non-zero error status if step failed because of an error.
    */
    step(): DbResult;

    /**
     * Get the current row, which the most recent step reached.
     * @return The current row in JSON stringified format.
     */
    getRow(): string;

}

/* The NodeAddonECSqlBinder class that is projected by the iModelJs node addon. */
declare class NodeAddonECSqlBinder implements IDisposable {
    constructor();

    /** Dispose of the NodeAddonECSqlBinder object */
    dispose(): void;

    bindNull(): DbResult;
    bindBlob(base64String: string): DbResult;
    bindBoolean(val: boolean): DbResult;
    bindDateTime(isoString: string): DbResult;
    bindDouble(val: number): DbResult;
    bindId(hexStr: string): DbResult;
    bindInt(val: number): DbResult;
    bindInt64(val: string | number): DbResult;
    bindPoint2d(x: number, y: number): DbResult;
    bindPoint3d(x: number, y: number, z: number): DbResult;
    bindString(val: string): DbResult;
    bindNavigation(navIdHexStr: string, relClassName?: string, relClassTableSpace?: string): DbResult;
    bindMember(memberName: string): NodeAddonECSqlBinder;
    addArrayElement(): NodeAddonECSqlBinder;
}

/* The NodeAddonECPresentationManager class that is projected by the iModelJs node addon. */
declare class NodeAddonECPresentationManager {
    constructor();
    /**
     * Handles an ECPresentation manager request
     * @param db The db to run the request on
     * @param options Serialized JSON object that contains parameters for the request
     * @return Serialized JSON response
     */
    handleRequest(db: NodeAddonDgnDb, options: string): string;
}