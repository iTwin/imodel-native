/*---------------------------------------------------------------------------------------------
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 *--------------------------------------------------------------------------------------------*/
import { IModelStatus, StatusCodeWithMessage, RepositoryStatus } from "@bentley/bentleyjs-core/lib/BentleyError";
import { BentleyStatus } from "@bentley/bentleyjs-core/lib/Bentley";
import { DbResult, DbOpcode } from "@bentley/bentleyjs-core/lib/BeSQLite";
import { OpenMode } from "@bentley/bentleyjs-core/lib/BeSQLite";
/* import { IModelStatus } from "@bentley/bentleyjs-core/lib/Bentley"; */

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


/** The return type of synchronous functions that may return an error or a successful result. */
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

/* The NodeAddonDgnDb class that is projected by the iModelJs node addon. */
declare class NodeAddonDgnDb {
  constructor();

  /**
   * Get information on all briefcases cached on disk
   * @param cachePath Path to the root of the disk cache
   */
  getCachedBriefcaseInfosSync(cachePath: string): ErrorStatusOrResult<DbResult, string>;

  /** Get name and description of the root subject of this BIM as a JSON string ({name:, description: }) */
  getRootSubjectInfo(): string;

  /**
   * Get the extents of this BIM as a JSON string ({low: {x:, y:, z:}, high: {x:, y:, z:}})
   */
  getExtents(): string;

  /**
   * Open a local BIM file.
   * @param dbname The full path to the BIM file in the local file system
   * @param mode The open mode
   * @param callback Invoked when the operation completes. The only argument is a status code indicating sucess or failure.
   */
  openDgnDb(dbname: string, mode: OpenMode, callback: IModelJsNodeAddonStatusOnlyCallback<DbResult>): void;

  /**
   * Open a local BIM file.
   * @param dbname The full path to the BIM file in the local file system
   * @param mode The open mode
   * @return non-zero error status if operation failed.
   */
  openDgnDbSync(dbname: string, mode: OpenMode): DbResult;

  /** Close this BIM file. */
  closeDgnDb(): void;

  /**
   * Set the briefcase ID of this BIM file.
   * @param idvalue The briefcase ID value.
   */
  setBriefcaseId(idvalue: number): DbResult;

  /** Get the briefcase ID of this BIM file. */
  getBriefcaseId(): number;

  /** Get the Id of the last change set that was merged into or created from the Db. This is the parent for any new change sets that will be created from the BIM.
   * @return Returns an empty string if the BIM is in it's initial state (with no change sets), or if it's a standalone briefcase disconnected from the Hub.
   */
  getParentChangeSetId(): string;

  /* Get the GUID of the BIM file */
  getDbGuid(): string;

  /* Set the GUID of the BIM file */
  setDbGuid(guid: string): DbResult;

  /**
   * TBD
   * @param briefcaseToken TBD
   * @param changeSetTokens TBD
   */
  openBriefcaseSync(briefcaseToken: string, changeSetTokens: string): DbResult;

  /** Save any pending changes to this BIM file.  @return non-zero error status if save failed. */
  saveChanges(): DbResult;

  /**
   * Import an EC schema.
   * There are a number of restrictions when importing schemas into a briefcase. 
   * When importing into a briefcse, this function will acquire the schema lock. That means that that briefcase must be at the tip of the revision
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
   * @param  callback Invoked when the operation completes. The 'result' argument is the element's properties in stringified JSON format.
   */
  getElement(opts: string, callback: IModelJsNodeAddonCallback<StatusCodeWithMessage<IModelStatus>, string>): void;

  /**
   * Get a model's properties
   * @param opts Identifies the model
   * @param  callback Invoked when the operation completes. The 'result' argument is the model's properties in stringified JSON format.
   */
  getModel(opts: string, callback: IModelJsNodeAddonCallback<StatusCodeWithMessage<IModelStatus>, string>): void

  /**
   * Insert an element.
   * @param elemProps The element's properties, in stringified JSON format.
   * @return non-zero error status if the operation failed.
   */
  insertElementSync(elemProps: string): ErrorStatusOrResult<IModelStatus, string>;

  /**
   * Update an element.
   * @param elemProps The element's properties, in stringified JSON format.
   * @return non-zero error status if the operation failed.
   */
  updateElementSync(elemProps: string): IModelStatus;

  /**
   * Delete an element.
   * @param elemIdJson The element's ID, in stringified JSON format
   * @return non-zero error status if the operation failed.
   */
  deleteElementSync(elemIdJson: string): IModelStatus;

  /**
   * Insert a model.
   * @param modelProps The model's properties, in stringified JSON format.
   * @return non-zero error status if the operation failed.
   */
  insertModelSync(modelProps: string): ErrorStatusOrResult<IModelStatus, string>;

  /**
   * Update a model.
   * @param modelProps The model's properties, in stringified JSON format.
   * @return non-zero error status if the operation failed.
   */
  updateModelSync(modelProps: string): IModelStatus;

  /**
   * Delete a model.
   * @param modelIdJson The model's ID, in stringified JSON format
   * @return non-zero error status if the operation failed.
   */
  deleteModelSync(modelIdJson: string): IModelStatus;

  /**
   * Format an element's properties, suitable for display to the user.
   * @param id The element's ID, in stringified JSON format
   * @param callback Invoked when the operation completes. The 'result' argument is an object containing the object's properties, in stringified JSON format.
   */
  getElementPropertiesForDisplay(id: string, callback: IModelJsNodeAddonCallback<StatusCodeWithMessage<IModelStatus>, string>): void;

  /**
   * Get information about an ECClass
   * @param schema The name of the ECSchema
   * @param className The name of the ECClass
   * @param callback Invoked when the operation completes. The 'result' argument is an object containing the properties of the class, in stringified JSON format.
   */
  getECClassMetaData(schema: string, className: string, callback: IModelJsNodeAddonCallback<StatusCodeWithMessage<IModelStatus>, string>): void;

 /**
   * Get information about an ECClass
   * @param schema The name of the ECSchema
   * @param className The name of the ECClass
   * @return An object containing the properties of the class, in stringified JSON format.
   */
  getECClassMetaDataSync(schema: string, className: string): ErrorStatusOrResult<IModelStatus, string>;

  /**
   * Execute a statement repeatedly until all rows are found.
   * @param ecsql The ECSql statement to execute
   * @param bindings The bindings to the statement. Pass null if there are no bindings.
   * @param callback Invoked when the operation completes. The 'result' argument is an array or rows in stringified JSON format.
   */
  executeQuery(ecsql: string, bindings: string, callback: IModelJsNodeAddonCallback<StatusCodeWithMessage<DbResult>, string>): void;

    /**
    * Add the lock, code, and other resource requests that would be needed in order to carry out the specified operation.
    * @param req The request object, which accumulates requests.
    * @param elemProps The IDs of the elements
    * @param opcode The operation that will be performed on the element.
    */  
    buildBriefcaseManagerResourcesRequestForElement(req: NodeAddonBriefcaseManagerResourcesRequest, elemProps: string, opcode: DbOpcode): RepositoryStatus;

    /**
    * Add the lock, code, and other resource requests that would be needed in order to carry out the specified operation.
    * @param req The request object, which accumulates requests.
    * @param modelProps The IDs of the models
    * @param opcode The operation that will be performed on the model.
    */
    buildBriefcaseManagerResourcesRequestForModel(req: NodeAddonBriefcaseManagerResourcesRequest, modelProps: string, opcode: DbOpcode): RepositoryStatus;

    /**
     * Try to acquire the requested resources from iModelHub.
     * This function may fulfill some requests and fail to fulfill others. This function returns a zero status
     * if all requests were fulfilled. It returns a non-zero status if some or all requests could not be fulfilled.
     * This function updates req to remove the requests that were successfully fulfilled. Therefore, if a non-zero
     * status is returned, the caller can look at req to see which requests failed.
     * @param req On input, this is the list of resource requests to be fulfilled. On return, this is the list of
     * requests that could not be fulfilled.
     * @return 0 if all resources were acquired, or non-zero if some could not be acquired.
     */
    requestBriefcaseManagerResources(req: NodeAddonBriefcaseManagerResourcesRequest): RepositoryStatus;

    /**
     * Check to see if *all* of the requested resources could be acquired from iModelHub.
     * @param req the list of resource requests to be fulfilled.
     * @return true if all resources could be acquired or false if any could not be acquired.
     */
    areBriefcaseManagerResourcesAvailable(req: NodeAddonBriefcaseManagerResourcesRequest): boolean;
}

/* The NodeAddonECSqlStatement class that is projected by the iModelJs node addon. */
declare class NodeAddonECSqlStatement {
    constructor();

    /**
     * Prepare an ECSql statement.
     * @param db The native DgnDb object
     * @param ecSql The statement to prepare
     * @return Zero status in case of success. Non-zero error status in case of failure. The error's message property will contain additional information.
     */
    prepare(db: NodeAddonDgnDb, ecSql: string): StatusCodeWithMessage<DbResult>;

    /** Reset the statement to just before the first row.
     * @return non-zero error status in case of failure.
     */
    reset(): DbResult;

    /** Dispose of the native ECSqlStatement object - call this when finished stepping a statement, but only if the statement is not shared. */
    dispose(): void;

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

declare class NodeAddonECDb {
    constructor();

    createDb(): void;
    openDb(): void;
    IsDbOpen(): void;
    closeDb(): void;
    saveChanges(): void;
    abandonChanges(): void;
    importSchema(): void;
    insertInstance(): void;
    readInstance(): void;
    updateInstance(): void;
    deleteInstance(): void;
    containsInstance(): void;
    executeQuery(): void;
    executeStatement(): void;

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