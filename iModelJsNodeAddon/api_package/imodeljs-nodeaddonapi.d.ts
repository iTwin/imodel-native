/*---------------------------------------------------------------------------------------------
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
 *--------------------------------------------------------------------------------------------*/
import { IModelStatus, StatusCodeWithMessage } from "@bentley/bentleyjs-core/lib/BentleyError";
import { DbResult } from "@bentley/bentleyjs-core/lib/BeSQLite";
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
 * The NodeAddonDgnDb class that is projected by the iModelJs node addon. 
 */
declare class NodeAddonDgnDb {
  constructor();

  /**
   * Get information on all briefcases cached on disk
   * @param cachePath Path to the root of the disk cache
   */
  getCachedBriefcaseInfosSync(cachePath: string): ErrorStatusOrResult<DbResult, string>;

  /** Get name and description of the root subject of this iModel as a JSON string ({name:, description: }) */
  getRootSubjectInfo(): string;

  /**
   * Get the project extents of this iModel.
   */
  getExtents(): string;

  /**
   * Open a local iModel.
   * @param dbname The full path to the iModel in the local file system
   * @param mode The open mode
   * @param callback Invoked when the operation completes. The only argument is a status code indicating success or failure.
   */
  openDgnDb(dbname: string, mode: OpenMode, callback: IModelJsNodeAddonStatusOnlyCallback<DbResult>): void;

  /**
   * Open a local iModel.
   * @param dbname The full path to the iModel in the local file system
   * @param mode The open mode
   * @return non-zero error status if operation failed.
   */
  openDgnDbSync(dbname: string, mode: OpenMode): DbResult;

  /** Close this iModel. */
  closeDgnDb(): void;

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
   */
  openBriefcaseSync(briefcaseToken: string, changeSetTokens: string): DbResult;

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
   * @param  callback Invoked when the operation completes. The 'result' argument is the element's properties in stringified JSON format.
   */
  getElement(opts: string, callback: IModelJsNodeAddonCallback<StatusCodeWithMessage<IModelStatus>, string>): void;

  /**
   * Get the properties of a Model
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
   * Delete an element from this iModel.
   * @param elemIdJson The element's Id, in stringified JSON format
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
   * @param modelIdJson The model's Id, in stringified JSON format
   * @return non-zero error status if the operation failed.
   */
  deleteModelSync(modelIdJson: string): IModelStatus;

  /**
   * Format an element's properties, suitable for display to the user.
   * @param id The element's Id, in stringified JSON format
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
   * Execute a test known to exist using the id recognized by the addon's test execution handler
   * @param id The id of the test you wish to execute
   * @param params A JSON string that should all of the data/parameters the test needs to function correctly
   */
  executeTestById(id: number, params: string): any;
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