/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScript/DgnJsApi/ts/DgnJsApi.d.ts $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/** @namespace Bentley::Dgn Types defined by the Bentley DgnPlatform TypeScript/JavaScript API
*/

declare module Bentley.Dgn /*** NATIVE_TYPE_NAME = BentleyApi::Dgn ***/ {
    /*** BEGIN_FORWARD_DECLARATIONS ***/
    class Transform { /*** NATIVE_TYPE_NAME = JsTransform ***/ }
    class DPoint2d { /*** NATIVE_TYPE_NAME = JsDPoint2d ***/ }
    class DPoint3d { /*** NATIVE_TYPE_NAME = JsDPoint3d ***/ }
    class DRange3d { /*** NATIVE_TYPE_NAME = JsDRange3d ***/ }
    class YawPitchRollAngles { /*** NATIVE_TYPE_NAME = JsYawPitchRollAngles ***/ }
    class Angle { /*** NATIVE_TYPE_NAME = JsAngle ***/ }
    class SolidPrimitive extends Geometry { /*** NATIVE_TYPE_NAME = JsSolidPrimitive ***/ }
    class DgnSphere extends SolidPrimitive {/*** NATIVE_TYPE_NAME = JsDgnSphere ***/ }
    class DgnBox extends SolidPrimitive {/*** NATIVE_TYPE_NAME = JsDgnBpx ***/ }
    class Geometry { /*** NATIVE_TYPE_NAME = JsGeometry ***/ }
    class GeometryNode { /*** NATIVE_TYPE_NAME = JsGeometryNode ***/ }
    /*** END_FORWARD_DECLARATIONS ***/

    type TransformP = cxx_pointer<Transform>;
    type DPoint2dP = cxx_pointer<DPoint2d>;
    type DPoint3dP = cxx_pointer<DPoint3d>;
    type DRange3dP = cxx_pointer<DRange3d>;
    type YawPitchRollAnglesP = cxx_pointer<YawPitchRollAngles>;
    type AngleP = cxx_pointer<Angle>;
    type SolidPrimitiveP = cxx_pointer<SolidPrimitive>;
    type DgnSphereP = cxx_pointer<DgnSphere>;
    type DgnBoxP = cxx_pointer<DgnBox>;
    type GeometryP = cxx_pointer<Geometry>;
    type GeometryNodeP = cxx_pointer<GeometryNode>;

    enum ECPropertyPrimitiveType { }

    enum BeSQLiteDbResult { }

    enum BeSQLiteDbOpcode { }

    //! Logging serverity level.
    enum LoggingSeverity { }

    /** Access to the message log */
    class Logging implements BeJsProjection_SuppressConstructor {

        /**
        * Set the severity level for the specified category
        * @param category     The logging category
        * @param severity     The minimum severity to display. Note that messages will not be logged if their severity is below this level.
        */
        static SetSeverity(category: Bentley_Utf8String, severity: cxx_enum_class_uint32_t<LoggingSeverity>): void;

        /**
        * Test if the specified severity level is enabled for the specified category
        * @param category     The logging category
        * @param severity     The severity to test
        */
        static IsSeverityEnabled(category: Bentley_Utf8String, severity: cxx_enum_class_uint32_t<LoggingSeverity>): cxx_bool;

        /**
        * Send a message to the log
        * @param category     The logging category
        * @param severity     The severity of the message. Note that the message will not be logged if \a severity is below the severity level set by calling SetSeverity
        * @param message      The message to log
        */
        static Message(category: Bentley_Utf8String, severity: cxx_enum_class_uint32_t<LoggingSeverity>, message: Bentley_Utf8String): void;
    }

    /** A 3-D placement */
    class Placement3d implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted {
        /*** NATIVE_TYPE_NAME = JsPlacement3d ***/
        constructor(origin: DPoint3dP, angles: YawPitchRollAnglesP);

        /** The origin of the placement */
        Origin: DPoint3dP;
        /** The angles of the placement */
        Angles: YawPitchRollAnglesP;
        /** Get the ElementAlignedBox3d of this Placement3d. */
        ElementBox: DRange3dP;
        /** Calculate the AxisAlignedBox3d of this Placement3d. */
        CalculateRange(): DRange3dP;
        /** Convert the origin and YawPitchRollAngles of this Placement3d into a Transform. */
        Transform: TransformP;

        OnDispose(): void;
        Dispose(): void;
    }

    type Placement3dP = cxx_pointer<Placement3d>;

    /** Script Management Utilities */
    class Script implements BeJsProjection_SuppressConstructor {

        /**
         * Make sure that the specified script is loaded.
         * @param db         The name of the DgnDb to check for a local script library
         * @param scriptName The name which was used to register the script in the script librray
         * @param forceReload  If true, the script's contents will be re-evaluated even if this script was previously loaded. Otherwise, the script is loaded and evaluated only once per session.
         * @return 0 (SUCCESS) if the script was loaded; otherwise, a non-zero error code.
         */
        static LoadScript(db: DgnDbP, scriptName: Bentley_Utf8String, forceReload: cxx_bool): cxx_int32_t;

        /**
         * Make sure that the specified builtin script library is loaded.
         * @param libName Identifies the library that is to be loaded
         * @note This function differs from LoadScript in that ImportLibrary is used to activate builtin libraries that are provided by apps or domains,
         * where LoadScript is used to load external scripts that are found in the script library or are loaded from a URL.
         */
        static ImportLibrary(libName: Bentley_Utf8String): void;

        /**
         * Report an error. An error is more than a message. The platform is will treat it as an error. For example, the platform may terminate the current command.
         * @param description A description of the error
         */
        static ReportError(description: Bentley_Utf8String): void;

        /** Begin a dispose context */
        static BeginDisposeContext(): void;

        /** End a dispose context.
          * This function calls Dispose on all JavaScript objects holding references to native objects that were created since the call to BeginDisposeContext.
          * As a result, the native objects are freed and the corresponding JavaScript objects become invalid and should not be used.
          * Freeing native objects reduces memory consumption and allows for more efficient management of native resources.
          */
        static EndDisposeContext(): void;
    }

    /** A wrapper for fopen/fgets */
    class File implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsFile ***/

        /** a wrapper for fopen.
         * @param name  The full path to the file.
         * @param mode  The usual fopen model specifier
         * @return an opened File object if successful, or null if the file cannot be opened
         * @note Use only text mode, as there is currently no support for reading the contents of binary files.
         * @note on mobile devices, only the temp, local state, and documents directories are accessible. It is up to the caller to supply these
         * directory root paths to the script.
        */
        static Fopen(name: Bentley_Utf8String, mode: Bentley_Utf8String): FileP;

        /** explicitly close the file */
        Close(): void;

        /** check if the next read position is at the end of the file. If so, do not attempt to read from the file. */
        Feof(): cxx_bool;

        /** Reads the next line of text.
          * @return the next line of text.
          * @note This function throws an exception if you try to read past the end of the file. Call Feof before calling this function.
          */
        ReadLine(): Bentley_Utf8String;

        /**
         * Writes a line of text at the current write position.
         * @param line  The line of text to write
         * @return non-zero if write failed.
         */
        WriteLine(line: Bentley_Utf8String): cxx_int32_t;

        OnDispose(): void;
        Dispose(): void;
    }

    type FileP = cxx_pointer<File>;

    /** An ECSql Value
    */
    class ECSqlValue implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECSqlValue ***/
        /**
         * Get the value as a string
         */
        GetText(): Bentley_Utf8String;
        /**
         * Get the value as an integer
         */
        GetInt(): cxx_int32_t;
        /**
         * Get the value as a double
         */
        GetDouble(): cxx_double;
        /**
         * Get the value as a DgnObjectId
         */
        GetId(): DgnObjectIdP;
        /**
         * Get the value as a DPoint3d
         */
        GetDPoint3d(): DPoint3dP;
        /**
         * Get the value as a DateTime string
         */
        GetDateTime(): Bentley_Utf8String;
        /**
         * Get the value as an Array
         */
        GetArray(): cxx_pointer<ECSqlArrayValue>;

        OnDispose(): void;
        Dispose(): void;
    }

    type ECSqlValueP = cxx_pointer<ECSqlValue>;

    /** Used when iterating over a ECSqlArrayValue */
    class ECSqlArrayValueIterator implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsECSqlArrayValueIterator ***/
        OnDispose(): void;
        Dispose(): void;
    }

    type ECSqlArrayValueIteratorP = cxx_pointer<ECSqlArrayValueIterator>;

    /** An ECSql Array Value
    */
    class ECSqlArrayValue implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECSqlArrayValue ***/
        /** Get an iterator positioned at the start of the array. */
        Begin(): ECSqlArrayValueIteratorP;
        /** Query if the iterator is at a valid position in the array. @param iter The iterator. @return true if the iterator's position is valid.   */
        IsValid(iter: ECSqlArrayValueIteratorP): cxx_bool;
        /** Move the iterator to the next position in the array. @param iter The iterator @return true if the new position is valid. */
        ToNext(iter: ECSqlArrayValueIteratorP): cxx_bool;
        /** Get the value to which the iterator points in the set. @param iter The iterator @return The current ID value. */
        GetValue(iter: ECSqlArrayValueIteratorP): ECSqlValueP;

        OnDispose(): void;
        Dispose(): void;
    }

    type ECSqlArrayValueP = cxx_pointer<ECSqlArrayValue>;

    /** A prepared ECSqlStatement
    */
    class PreparedECSqlStatement implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsPreparedECSqlStatement ***/

        /**
          * Bind a DgnObjectId value to the specified parameter
          * @param parameterIndex Parameter index
          * @param value Value to bind.
          */
        BindId(parameterIndex: cxx_int32_t, value: DgnObjectIdP): void;

        /**
          * Bind a string value to the specified parameter
          * @param parameterIndex Parameter index
          * @param value Value to bind.
          */
        BindText(parameterIndex: cxx_int32_t, value: Bentley_Utf8String): void;

        /**
          * Bind an integer value to the specified parameter
          * @param parameterIndex Parameter index
          * @param value Value to bind.
          */
        BindInt(parameterIndex: cxx_int32_t, value: cxx_int32_t): void;

        /**
          * Bind a double value to the specified parameter
          * @param parameterIndex Parameter index
          * @param value Value to bind.
          */
        BindDouble(parameterIndex: cxx_int32_t, value: cxx_double): void;

        /**
          * Bind a DPoint3d value to the specified parameter
          * @param parameterIndex Parameter index
          * @param value Value to bind.
          */
        BindPoint3d(parameterIndex: cxx_int32_t, value: DPoint3dP): void;

        /**
          * Bind a DRange3d value to the specified parameter
          * @param parameterIndex Parameter index
          * @param value Value to bind.
          */
        BindDRange3d(parameterIndex: cxx_int32_t, value: DRange3dP): void;

        /**
         * Step the statement to the next row.
         * @return false if the query is done.
         */
        Step(): cxx_enum_class_uint32_t<BeSQLiteDbResult>;

        /**
         * Return the index of an SQL parameter given its name. The index value returned is suitable for use
         * as the second parameter to one of the Bind functions. A zero is returned if no matching parameter is found.
         * @note a named parameter is a placeholder in a WHERE clause that is identified by a name, rather than by a simple ?
         * @param parameterName   parameterName Name of the binding parameter
         */
        GetParameterIndex(parameterName: Bentley_Utf8String): cxx_int32_t;

        /**
         * Get the value of the specified column as a string
         * @param columnIndex Index of ECSQL column in result set (0-based)
         */
        GetValueText(columnIndex: cxx_int32_t): Bentley_Utf8String;
        /**
         * Get the value of the specified column as an integer
         * @param columnIndex Index of ECSQL column in result set (0-based)
         */
        GetValueInt(columnIndex: cxx_int32_t): cxx_int32_t;
        /**
         * Get the value of the specified column as a double
         * @param columnIndex Index of ECSQL column in result set (0-based)
         */
        GetValueDouble(columnIndex: cxx_int32_t): cxx_double;
        /**
         * Get the value of the specified column as a DgnObjectId
         * @param columnIndex Index of ECSQL column in result set (0-based)
         */
        GetValueId(columnIndex: cxx_int32_t): DgnObjectIdP;
        /**
         * Get the value of the specified column as a DPoint3d
         * @param columnIndex Index of ECSQL column in result set (0-based)
         */
        GetValueDPoint3d(columnIndex: cxx_int32_t): DPoint3dP;
        /**
         * Get the value of the specified column as a DRange3d
         * @param columnIndex Index of ECSQL column in result set (0-based)
         */
        GetValueDRange3d(columnIndex: cxx_int32_t): DRange3dP;
        /**
         * Get the value of the specified column as a DateTime string
         * @param columnIndex Index of ECSQL column in result set (0-based)
         */
        GetValueDateTime(columnIndex: cxx_int32_t): Bentley_Utf8String;
        /**
         * Get the value of the specified column as an Array
         * @param columnIndex Index of ECSQL column in result set (0-based)
         */
        GetValueArray(columnIndex: cxx_int32_t): ECSqlArrayValueP;

        OnDispose(): void;
        Dispose(): void;
    }

    type PreparedECSqlStatementP = cxx_pointer<PreparedECSqlStatement>;

    /** DgnDb - Projection of BentleyApi::Dgn::DgnDb */
    class DgnDb implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsDgnDb ***/
        /** The collection of models in this DgnDb */
        Models: DgnModelsP;
        /** The collection of ECSchemas in this DgnDb */
        Schemas: SchemaManagerP;
        /** The collection of DgnElements in this DgnDb */
        Elements: DgnElementsP;

        /** Get a prepared ECSqlStatement for selecting rows from this DgnDb
          * @param ecsql    The body of the ECSql SELECT statement that is to be executed.
          * @return a prepared ECSql statement or null if the SQL statement is invalid.
          * @see ECClass::ECSqlName
          * @note Only SELECT statements can be prepared. If you do not specify the SELECT keyword, it will be prepended automatically.
          */
        GetPreparedECSqlSelectStatement(ecsql: Bentley_Utf8String): PreparedECSqlStatementP;

        /** Save changes to the DgnDb, marking the end of a transaction. If undo/redo is enabled, this creates an undo point. @return non-zero if the insert failed. */
        SaveChanges(): cxx_int32_t;

        OnDispose(): void;
        Dispose(): void;
    }

    type DgnDbP = cxx_pointer<DgnDb>;

    /** A 64-bit ID.  */
    class DgnObjectId implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsDgnObjectId ***/

        constructor();

        /** Tests if the ID is valid */
        IsValid(): cxx_bool;
        /** Tests if the ID matches another ID @param id The other ID */
        Equals(id: DgnObjectIdP): cxx_bool;
        /**
         * Gets the value of the ID as a string
         */
        ToString(): Bentley_Utf8String;
        /**
         * Sets the value of the ID from a string
         * @param str   The new value as a string
         */
        FromString(str: Bentley_Utf8String): void;

        static MakeFromString(str: Bentley_Utf8String): DgnObjectIdP;

        OnDispose(): void;
        Dispose(): void;
    }

    type DgnObjectIdP = cxx_pointer<DgnObjectId>;

    /** Used when iterating over a DgnObjectIdSet */
    class DgnObjectIdSetIterator implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsDgnObjectIdSetIterator ***/
        OnDispose(): void;
        Dispose(): void;
    }

    type DgnObjectIdSetIteratorP = cxx_pointer<DgnObjectIdSetIterator>;

    /** A set of DgnObjectIds */
    class DgnObjectIdSet implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsDgnObjectIdSet ***/
        /** Query the number of IDs in the set. */
        Size(): cxx_double;
        /** Remove all IDs from the set. */
        Clear(): void;

        /** Insert an ID into the set. */
        Insert(id: DgnObjectIdP): void;
        /** Get an iterator positioned at the start of the set. */
        Begin(): DgnObjectIdSetIteratorP;
        /** Query if the iterator is at a valid position in the set. @param iter The iterator. @return true if the iterator's position is valid.   */
        IsValid(iter: DgnObjectIdSetIteratorP): cxx_bool;
        /** Move the iterator to the next position in the set. @param iter The iterator @return true if the new position is valid. */
        ToNext(iter: DgnObjectIdSetIteratorP): cxx_bool;
        /** Get the value to which the iterator points in the set. @param iter The iterator @return The current ID value. */
        GetId(iter: DgnObjectIdSetIteratorP): DgnObjectIdP;

        OnDispose(): void;
        Dispose(): void;
    }

    type DgnObjectIdSetP = cxx_pointer<DgnObjectIdSet>;

    /** DgnCode - Projection of BentleyApi::Dgn::DgnCode */
    class DgnCode implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsDgnCode ***/

        /** Make a DgnCode object from a JSON-encoded DgnCode
          * @param db   The BIM
          * @param json The JSON encoding of the code
          */
        static FromJson(db: DgnDbP, json: Bentley_Utf8String): DgnCodeP;

        /** The CodeSpecId of this code */
        CodeSpecId: DgnObjectIdP;

        /** The Scope of this code */
        Scope: DgnObjectIdP;

        /** The Value of this code */
        Value: Bentley_Utf8String;

        OnDispose(): void;
        Dispose(): void;
    }

    type DgnCodeP = cxx_pointer<DgnCode>;

    /** DgnModels - Projection of BentleyApi::Dgn::DgnModels */
    class DgnModels implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsDgnModels ***/
        /** Find or load the model identified by the specified ID. @param id The model ID. @return The loaded model or null if not found */
        GetModel(id: DgnObjectIdP): DgnModelP;
        OnDispose(): void;
        Dispose(): void;
    }

    type DgnModelsP = cxx_pointer<DgnModels>;

    /** DgnElements - Projection of BentleyApi::Dgn::DgnElements */
    class DgnElements implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsDgnElements ***/

        /** The DgnDb that contains this collection of elements */
        DgnDb: DgnDbP;

        /**
         * Look up an element in the pool of loaded elements for this DgnDb.
         * @return A pointer to the element, or nullptr if the is not in the pool.
         * @note This method will return null if the element is not currently loaded. That does not mean the element doesn't exist in the database.
         */
        FindLoadedElement(id: DgnObjectIdP): DgnElementP;

        /** Look for the element that has the specified code
          * @param code The DgnCode to look up
          * @return the DgnElementId of the element found or null if no element with that specified code was found
          */
        QueryElementIdByCode(code: DgnCodeP): DgnObjectIdP;

        /**
         * Get a DgnElement from the DgnDb by its DgnElementId.
         * @param elementId             The element's DgnElementId
         * @remarks The element is loaded from the database if necessary.
         * @return Invalid if the element does not exist.
         */
        GetElement(elementId: DgnObjectIdP): DgnElementP;

        OnDispose(): void;
        Dispose(): void;
    }

    type DgnElementsP = cxx_pointer<DgnElements>;

    /** DgnCategory - Projection of BentleyApi::Dgn::DgnCategory */
    class DgnCategory implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsDgnCategory ***/
        /** The DgnDb that contains this Category */
        DgnDb: DgnDbP;
        /** The ID of this Category */
        CategoryId: DgnObjectIdP;
        /** The ID of the default SubCategory of this Category */
        DefaultSubCategoryId: DgnObjectIdP;
        /** The name of this Category */
        CategoryName: Bentley_Utf8String;
        /** Look up the ID of the Category with the specified name. @param name The name to look up. @param db The DgnDb that contains the Category. @return The ID of the Category if found */
        static QueryCategoryId(name: Bentley_Utf8String, db: DgnDbP): DgnObjectIdP;
        /** Find or load the Category with the specified ID. @param id The ID to look up. @param db The DgnDb that contains the Category. @return The Category if found */
        static QueryCategory(id: DgnObjectIdP, db: DgnDbP): DgnCategoryP;
        /** Get the set of all DgnCategoryIDs in the Db. @param db The DgnDb to query. @return the set of all category IDs. */
        static QueryCategories(db: DgnDbP): DgnObjectIdSetP;
        OnDispose(): void;
        Dispose(): void;
    }

    type DgnCategoryP = cxx_pointer<DgnCategory>;

    /** DgnElement - Projection of BentleyApi::Dgn::DgnElement */
    class DgnElement implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsDgnElement ***/
        /** The element's ID */
        ElementId: DgnObjectIdP;
        /** The element's Code */
        Code: DgnCodeP;
        /** The Model that contains the element */
        Model: DgnModelP;
        /** The Model that breaks down or models the element */
        SubModel: DgnModelP;
        /** The ECClass of this element */
        ElementClass: ECClassP;
        /** Insert this element into its Model in the DgnDb. @return non-zero if the insert failed. */
        Insert(): cxx_int32_t;
        /** Update this element in its Model in the DgnDb. @return non-zero if the update failed. */
        Update(): cxx_int32_t;
        /** Set the Parent of this element. @param parent The parent element. */
        SetParent(parent: cxx_pointer<DgnElement>): void;

        /**
         * Get the value of a property that is defined by the element's class.
         * @param name The name of property
         * @return the value of the property or null if the property is not found
         */
        GetPropertyValue(name: Bentley_Utf8String): ECValueP;

        /**
         * Set the value of a property that is defined by the element's class
         * @param name      The name of property
         * @param value     The new value for the property
         * @return non-zero error status if the property is not found
         */
        SetPropertyValue(name: Bentley_Utf8String, value: ECValueP): cxx_int32_t;

        /** Cast this element to GeometrySource if possible */
        ToGeometrySource(): GeometrySourceP;

        /** Cast this element to GeometrySource3d if possible */
        ToGeometrySource3d(): GeometrySource3dP;

        /** Cast this element to GeometrySource2d if possible */
        ToGeometrySource2d(): GeometrySource2dP;

        /**
        * Add any required locks and/or codes to the specified request in preparation for the specified operation
        * @param request    The request to which to add the required locks and/or codes
        * @param opcode     The operation to be performed
        */
        PopulateRequest(request: RepositoryRequestP, opcode: cxx_enum_class_uint32_t<BeSQLiteDbOpcode>): cxx_enum_class_uint32_t<RepositoryStatus>;

        /**
         * Create a new DgnElement
         * @param model The model that is to contain the new element
         * @param elementClassName The name of the element's ECClass.
         * @return a new, non-persistent DgnElement or null if one of the parameters is invalid
         * @see Insert
        */
        static Create(model: DgnModelP, elementClassName: Bentley_Utf8String): DgnElementP;

        OnDispose(): void;
        Dispose(): void;
    }

    type DgnElementP = cxx_pointer<DgnElement>;

    /** GeometrySource */
    class GeometrySource implements BeJsProjection_IsInterface, IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsGeometrySource ***/
        /* This is a projection of a C++ interface -- no instance is ever created -- it's always an alias for an instance of a concrete class, such as GeometricElement3d */

        /** Get the element's DgnCategoryId */
        CategoryId: DgnObjectIdP;

        /** The element's geometry (read-only) */
        Geometry: GeometryCollectionP;

        /** Cast this element to DgnElement */
        ToDgnElement(): DgnElementP;

        ToGeometrySource2d(): GeometrySource2dP;
        ToGeometrySource3d(): GeometrySource3dP;

        OnDispose(): void;
        Dispose(): void;
    }

    type GeometrySourceP = cxx_pointer<GeometrySource>;

    /** GeometrySource2d */
    class GeometrySource2d extends GeometrySource implements BeJsProjection_IsInterface, IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsGeometrySource2d ***/
        /* This is a projection of a C++ interface -- no instance is ever created -- it's always an alias for an instance of a concrete class, such as GeometricElement2d */

        // *** TBD

        ToGeometrySource2d(): GeometrySource2dP;
        ToGeometrySource2d(): GeometrySource2dP;

        OnDispose(): void;
        Dispose(): void;
    }

    type GeometrySource2dP = cxx_pointer<GeometrySource2d>;

    /** GeometrySource3d */
    class GeometrySource3d extends GeometrySource implements BeJsProjection_IsInterface, IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsGeometrySource3d ***/
        /* This is a projection of a C++ interface -- no instance is ever created -- it's always an alias for an instance of a concrete class, such as GeometricElement3d */

        /** Get the placement of this element */
        Placement: Placement3dP;

        /** Transform the element's Placement
         * @param transform The transform to apply to the element's Placement. The transform must be pure rotation and/or translation.
         * @return non-zero error status if the element could not be transformed or if \a transform is invalid.
        */
        Transform(transform: TransformP): cxx_int32_t;

        ToGeometrySource2d(): GeometrySource2dP;
        ToGeometrySource3d(): GeometrySource3dP;

        OnDispose(): void;
        Dispose(): void;
    }

    type GeometrySource3dP = cxx_pointer<GeometrySource3d>;

    class GeometricElement extends DgnElement implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsGeometricElement ***/

        /** Get the element's DgnCategoryId */
        CategoryId: DgnObjectIdP;

        /** The element's geometry (read-only) */
        Geometry: GeometryCollectionP;

        /** Cast this element to DgnElement */
        ToDgnElement(): DgnElementP;

        OnDispose(): void;
        Dispose(): void;
    }

    type GeometricElementP = cxx_pointer<GeometricElement>;

    class GeometricElement3d extends GeometricElement implements GeometrySource3d, IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsGeometricElement3d ***/

        /** Get the placement of this element */
        Placement: Placement3dP;

        /** Cast this element to GeometrySource */
        ToGeometrySource(): GeometrySourceP;

        /** Cast this element to GeometrySource3d */
        ToGeometrySource3d(): GeometrySource3dP;

        ToGeometrySource2d(): GeometrySource2dP;

        /** Transform the element's Placement
         * @param transform The transform to apply to the element's Placement. The transform must be pure rotation and/or translation.
         * @return non-zero error status if the element could not be transformed or if \a transform is invalid.
        */
        Transform(transform: TransformP): cxx_int32_t;

        /**
         * Create a new GeometricElement3d
         * @param model The model that is to contain the new element
         * @param categoryId The new element's DgnCategoryId
         * @param elementClassName The name of the element's ECClass. Must be a subclass of GeometricElement3d.
         * @return a new, non-persistent GeometricElement3d or null if one of the parameters is invalid
         * @see Insert
        */
        static CreateGeometricElement3d(model: DgnModelP, categoryId: DgnObjectIdP, elementClassName: Bentley_Utf8String): GeometricElement3dP;

        OnDispose(): void;
        Dispose(): void;
    }

    type GeometricElement3dP = cxx_pointer<GeometricElement3d>;

    /** DgnModel - Projection of BentleyApi::Dgn::DgnModel */
    class DgnModel implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsDgnModel ***/
        /** The ID of this model */
        ModelId: DgnObjectIdP;
        /** The DgnDb that contains this model */
        DgnDb: DgnDbP;

        /**
        * Add any required locks and/or codes to the specified request in preparation for the specified operation
        * @param request    The request to which to add the required locks and/or codes
        * @param opcode     The operation to be performed
        */
        PopulateRequest(request: RepositoryRequestP, opcode: cxx_enum_class_uint32_t<BeSQLiteDbOpcode>): cxx_enum_class_uint32_t<RepositoryStatus>;

        OnDispose(): void;
        Dispose(): void;
    }

    type DgnModelP = cxx_pointer<DgnModel>;


    /**
     * ColorDef - Projection of BentleyApi::Dgn::ColorDef
     */
    class ColorDef implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsColorDef ***/

        /**
         * Construct a new ColorDef
         * @param red   The red value. 0-255.
         * @param green   The green value. 0-255.
         * @param blue   The blue value. 0-255.
         * @param alpha   The alpha value. 0-255.
         */
        constructor(red: cxx_uint8_t, green: cxx_uint8_t, blue: cxx_uint8_t, alpha: cxx_uint8_t);

        /** The red value */
        Red: cxx_uint8_t;
        /** The green value */
        Green: cxx_uint8_t;
        /** The blue value */
        Blue: cxx_uint8_t;
        /** The alpha value */
        Alpha: cxx_uint8_t;

        OnDispose(): void;
        Dispose(): void;
    }

    type ColorDefP = cxx_pointer<ColorDef>;

    /** Projection of BentleyApi::Dgn::Render::FillDisplay */
    enum RenderFillDisplay { }

    /** Projection of BentleyApi::Dgn::Render::DgnGeometryClass */
    enum RenderDgnGeometryClass { }

    /**
     * RenderGeometryParams - Projection of BentleyApi::Dgn::Render::GeometryParams
     */
    class RenderGeometryParams implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsRenderGeometryParams ***/

        /** Construct a new empty RenderGeometryParams */
        constructor();

        /** The geometry's Category */
        CategoryId: DgnObjectIdP;
        /** The geometry's SubCategory */
        SubCategoryId: DgnObjectIdP;
        /** The geometry's weight. Must be an integer between 0 and ... */
        Weight: cxx_uint32_t;
        /** The geometry's line color. */
        LineColor: ColorDefP;
        /** Specify if or when the geometry should be filled. */
        FillDisplay: cxx_enum_class_uint32_t<RenderFillDisplay>;
        /** The geometry's fill color. */
        FillColor: ColorDefP;
        /** The geometry class */
        GeometryClass: cxx_enum_class_uint32_t<RenderDgnGeometryClass>;
        /** The geometry's transparency. Must be a floating point number between ... */
        Transparency: cxx_double;
        /** The geometry's fill transparency. Must be a floating point number between ... */
        FillTransparency: cxx_double;
        /** The geometry's display priority. Must be an integer between ... */
        DisplayPriority: cxx_int32_t;
        /** The geometry's material. */
        MaterialId: DgnObjectIdP;

        /* *** TBD:  PatternParams, Gradient, LineStyle */

        OnDispose(): void;
        Dispose(): void;
    }

    type RenderGeometryParamsP = cxx_pointer<RenderGeometryParams>;

    /**
     * TextString - Projection of BentleyApi::Dgn::TextString
     */
    class TextString implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsTextString ***/

        OnDispose(): void;
        Dispose(): void;
    }

    type TextStringP = cxx_pointer<TextString>;

    /**
     * GeometricPrimitive - Projection of BentleyApi::Dgn::GeometricPrimitive
     */
    class GeometricPrimitive implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsGeometricPrimitive ***/

        /** If the primitive is pure geometry, then this property returns its geometry */
        Geometry: GeometryP;
        /** If the primitive is a text string, then this property returns its text string */
        TextString: TextStringP;

        OnDispose(): void;
        Dispose(): void;
    }

    type GeometricPrimitiveP = cxx_pointer<GeometricPrimitive>;

    /**
     * DgnGeometryPart - Projection of BentleyApi::Dgn::DgnGeometryPart
     */
    class DgnGeometryPart implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsDgnGeometryPart ***/

        /**
          * Create a new DgnGeometryPart object.
          * @param db   The DgnDb that will hold the DgnGeometryPart
          * @return the DgnGeometryPart object
          * @see InsertGeometryPart
          */
        static Create(db: DgnDbP): DgnGeometryPartP;

        /**
         * Insert this DgnGeometryPart into the DgnDb.
         * @return non-zero error status if the DgnGeometryPart could not be inserted.
         */
        Insert(): cxx_int32_t;

        OnDispose(): void;
        Dispose(): void;
    }

    type DgnGeometryPartP = cxx_pointer<DgnGeometryPart>;

    class GeometryCollectionIterator implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsGeometryCollectionIterator ***/

        OnDispose(): void;
        Dispose(): void;
    }

    type GeometryCollectionIteratorP = cxx_pointer<GeometryCollectionIterator>;

    /**
     * GeometryCollection - Projection of BentleyApi::Dgn::GeometryCollection
     */
    class GeometryCollection implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsGeometryCollection ***/

        /**
         * Get an interator that points to the first item in this collection
         */
        Begin(): GeometryCollectionIteratorP;

        /**
         * Test if this iterator is not at the end of the collection
         * @param iter  The iterator
         * @return true if the iterator is not at the end of the collection
         */
        IsValid(iter: GeometryCollectionIteratorP): cxx_bool;

        /**
         * Move to the next item in this collection.
         * @param iter  The iterator
         * @return true if the new position of the iterator is not at the end of the collection
         */
        ToNext(iter: GeometryCollectionIteratorP): cxx_bool;

        /**
         * Get the GeometricPrimitive at the specified position in this collection.
         * @param iter  The iterator
         * @return a GeometricPrimiive or null if the current item is not a primitive.
         * @see GetGeometryPart
         * @see GetGeometryToWorld
         */
        GetGeometry(iter: GeometryCollectionIteratorP): GeometricPrimitiveP;

        /**
         * Get the DgnGeometryPart at the specified position in this collection.
         * @param iter  The iterator
         * @return a DgnGeometryPart or null if the current item is not a DgnGeometryPart reference.
         * @see GetGeometry
         * @see GetGeometryToWorld
         */
        GetGeometryPart(iter: GeometryCollectionIteratorP): DgnGeometryPartP;

        /**
         * Get the transform that relates the GeometricPrimitive at the specified position in this collection to the world coordinate system.
         * @param iter  The iterator
         * @return The transform
         */
        GetGeometryToWorld(iter: GeometryCollectionIteratorP): TransformP;

        /**
         * Get the RenderGeometryParams that apply to geometry at the specified position in this collection.
         * @param iter  The iterator
         */
        GetGeometryParams(iter: GeometryCollectionIteratorP): RenderGeometryParamsP;


        OnDispose(): void;
        Dispose(): void;
    }

    type GeometryCollectionP = cxx_pointer<GeometryCollection>;

    /** Projection of BentleyApi::Dgn::GeometryBuilder::CoordSys */
    enum BuilderCoordSystem { }

    /**
     * GeometryBuilder - Projection of BentleyApi::Dgn::GeometryBuilder
     */
    class GeometryBuilder implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsGeometryBuilder ***/

        /**
         * Construct a new GeometryBuilder to prepare geometry using the category and placement of an existing element.
         * @note This is just a short cut for calling CreateForModel.
         * @param el The element to get the category and placement origin/angle(s) from.
         * @return a GeometryBuilder object
         * @see CreateForModel
         */
        static CreateForElement(el: GeometrySourceP): GeometryBuilderP;

        /**
         * Construct a new GeometryBuilder to prepare geometry for elements in the specified model and category
         * @param model The model where the geometry will ultimately be stored
         * @param catid The category of the element that will ultimatley contain the geometry
         * @param transform The placement represented by a transform
         * @return a GeometryBuilder object
         */
        static CreateForModelWithTransform(model: DgnModelP, catid: DgnObjectIdP, transform: TransformP): GeometryBuilderP;

        /**
         * Construct a new GeometryBuilder to prepare 3d geometry for elements in the specified model and category
         * @param model The model where the geometry will ultimately be stored
         * @param catid The category of the element that will ultimatley contain the geometry
         * @param o     The placement origin
         * @param angles The placement angles
         * @return a GeometryBuilder object
         */
        static CreateFor3dModel(model: DgnModelP, catid: DgnObjectIdP, o: DPoint3dP, angles: YawPitchRollAnglesP): GeometryBuilderP;

        /**
         * Construct a new GeometryBuilder to prepare 2d geometry for elements in the specified model and category
         * @param model The model where the geometry will ultimately be stored
         * @param catid The category of the element that will ultimatley contain the geometry
         * @param o     The placement origin
         * @param angle The placement angle
         * @return a GeometryBuilder object
         */
        static CreateFor2dModel(model: DgnModelP, catid: DgnObjectIdP, o: DPoint2dP, angle: AngleP): GeometryBuilderP;

        //! Create builder from DgnModel and DgnCategoryId. A placement will be computed from the first appended GeometricPrimitive.
        //! @note Only CoordSystem::World is supported for append and it's not possible to append a DgnGeometryPartId as it need to be positioned relative to a known coordinate system.
        //!       Generally it's best if the application specifies a more meaningful placement than just using the GeometricPrimitive's local coordinate frame, ex. line mid point vs. start point.
        static CreateWithAutoPlacement (model: DgnModelP, catid: DgnObjectIdP): GeometryBuilderP;

        /**
         * Construct a new GeometryBuilder to prepare geometry for a DgnGeometryPart
         * @param db    The DgnDb that will hold the DgnGeometryPart
         * @param is3d  Will the DgnGeometryPart hold 3-D geometry?
         * @return a GeometryBuilder object
         */
        static CreateGeometryPart(db: DgnDbP, is3d: cxx_bool): GeometryBuilderP;

        /**
         * Append a copy of each geometric primitive in the specified builder to this builder, with a transform.
         * @param builder   the builder to copy from
         * @param relativePlacement if not null, the offset and/or rotation of the copied geometry
         */
        AppendCopyOfGeometry(builder: GeometryBuilderP, relativePlacement: Placement3dP): void;

        /**
         * Append RenderGeometryParams to the builder.
         * @param params The parameters to apply to subsequent geometry
         */
        AppendRenderGeometryParams(params: RenderGeometryParamsP): void;

        /** The RenderGeometryParams that will be applied to geometry appended to the builder. */
        GeometryParams: RenderGeometryParamsP;

        /**
         * Append a SubCategoryId to the builder.
         * @param subcategoryId The SubCategoryId to apply to subsequent geometry.
         */
        AppendSubCategoryId(subcategoryId: DgnObjectIdP): void;

        /**
         * Append a geometry of some kind
         * @param geometry  The geometry
         * @param coordSystem The coordinate system used by the geometry, either BuilderCoordSystem.Local or BuilderCoordSystem.World
         */
        AppendGeometry(geometry: GeometryP, coordSystem: cxx_enum_class_uint32_t<BuilderCoordSystem>): void;

        /**
         * Append the geometry from a GeometryNode.
         * @remark All leaf geometry is transformed to the node's root coordinates and saved as separate geometry items.
         * @param node the root of the geometry.
         * @param coordSystem The coordinate system used by the geometry, either BuilderCoordSystem.Local or BuilderCoordSystem.World
         */
        AppendGeometryNode(node: GeometryNodeP, coordSystem: cxx_enum_class_uint32_t<BuilderCoordSystem>): void;

        /**
         * Append an instance of a DgnGeometryPart
         * @param geometryPart  The DgnGeometryPart
         * @param relativePlacement if not null, the offset and/or rotation of the instance
         */
        AppendGeometryPart(geometryPart: DgnGeometryPartP, relativePlacement: Placement3dP): void;

        /**
         * Copy the geometry in this builder to an element.
         * @param element   The element
         * @return non-zero error status if \a element is invalid or if this geometry stream is invalid
         */
        Finish(element: GeometrySourceP): cxx_int32_t;

        /**
         * Copy the geometry in this builder to a DgnGeometryPart.
         * @param part  The DgnGeometryPart
         * @return non-zero error status if \a part is invalid or if this geometry stream is invalid
         */
        FinishPart(part: DgnGeometryPartP): cxx_int32_t;

        OnDispose(): void;
        Dispose(): void;
    }

    type GeometryBuilderP = cxx_pointer<GeometryBuilder>;

    /* ------------------------------------------ ScriptBasedTool -----------------------------------------------*/

    /**
     * HitDetail - Projection of BentleyApi::Dgn::HitDetail
     */
    class HitDetail implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsHitDetail ***/

        /** The element that was picked */
        Element: GeometrySource3dP;

        /** The point at which the element was picked */
        TestPoint: DPoint3dP;

        /** The adjusted point on the element that was picked */
        HitPoint: DPoint3dP;

        /** The type of hit: 'hit', 'snap', 'intersection' */
        HitType: Bentley_Utf8String;


        // *** TBD: GeomDetail

        OnDispose(): void;
        Dispose(): void;
    }

    type HitDetailP = cxx_pointer<HitDetail>;

    /* ------------------------------------------ Briefcase Management ------------------------------------------*/

    enum LockableType { }

    enum LockLevel { }

    enum RepositoryStatus { }

    /** Describes the ID of a lockable object */
    class LockableId implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted {
        /*** NATIVE_TYPE_NAME = JsLockableId ***/
        /**
        * Construct by type and object ID
        * @param objectType The type of the object
        * @param objectID   The ID of the object
        */
        constructor(objectType: cxx_enum_class_uint8_t<LockableType>, objectId: DgnObjectIdP);

        /** The ID of the object */
        Id: DgnObjectIdP;
        /** The type of the object */
        Type: cxx_enum_class_uint8_t<LockableType>;
        /** Whether this ID is valid */
        IsValid: cxx_bool;
        /** Set this to an invalid ID */
        Invalidate(): void;
        /** Tests if the ID matches another ID @param id The other ID */
        Equals(id: LockableIdP): cxx_bool;
        /** Creates a LockableId for an element @param element The element */
        static FromElement(element: DgnElementP): LockableIdP;
        /** Creates a LockableId for a model @param model The model */
        static FromModel(model: DgnModelP): LockableIdP;
        /** Creates a LockableId for a DgnDb @param dgndb The DgnDb */
        static FromDgnDb(dgndb: DgnDbP): LockableIdP;

        OnDispose(): void;
        Dispose(): void;
    }

    type LockableIdP = cxx_pointer<LockableId>;

    class DgnLock implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted {
        /*** NATIVE_TYPE_NAME = JsDgnLock ***/
        /**
        * Construct by ID and lock level
        * @param id     The ID of the lockable object
        @ @param level  The level at which the object is locked
        */
        constructor(id: LockableIdP, level: cxx_enum_class_uint8_t<LockLevel>);

        /** The Id of the lockable object */
        Id: LockableIdP;
        /** The level at which the object is locked */
        Level: cxx_enum_class_uint8_t<LockLevel>;
        /** Ensure the lock level is no lower than the specified level @param level The minimum level */
        EnsureLevel(level: cxx_enum_class_uint8_t<LockLevel>): void;
        /** Sets this to be an invalid lock */
        Invalidate(): void;

        /**
        * Create a DgnLock for the specified element
        * @param element    The locked element
        * @param level      The level at which the element is locked
        */
        static FromElement(element: DgnElementP, level: cxx_enum_class_uint8_t<LockLevel>): DgnLockP;

        /**
        * Create a DgnLock for the specified model
        * @param model  The locked model
        * @param level  The level at which the model is locked
        */
        static FromModel(model: DgnModelP, level: cxx_enum_class_uint8_t<LockLevel>): DgnLockP;

        /**
        * Create a DgnLock for the specified DgnDb
        * @param dgndb  The locked DgnDb
        * @param level  The level at which the DgnDb is locked
        */
        static FromDgnDb(dgndb: DgnDbP, level: cxx_enum_class_uint8_t<LockLevel>): DgnLockP;

        OnDispose(): void;
        Dispose(): void;
    }

    type DgnLockP = cxx_pointer<DgnLock>;

    /** Represents a request to acquire locks and/or codes, or query the availability thereof */
    class RepositoryRequest implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted {
        /*** NATIVE_TYPE_NAME = JsRepositoryRequest ***/

        /**
        * Create a new, empty request object for the specified briefcase
        * @param dgndb      The requesting briefcase
        * @param operation  The name of the operation for which the request is being made (e.g., a tool name)
        */
        static Create(dgndb: DgnDbP, operation: Bentley_Utf8String): RepositoryRequestP;

        /** The briefcase associated with this request */
        Briefcase: DgnDbP;
        /** The name of the operation for which the request is being made (e.g., a tool name) */
        Operation: Bentley_Utf8String;

        /** Attempts to acquire the locks and/or codes from the repository manager on this request's briefcase's behalf */
        Acquire(): cxx_enum_class_uint32_t<RepositoryStatus>;
        /** Queries the repository manager to determine if the locks and/or codes in this request are available to be acquired by this request's briefcase */
        QueryAvailability(): cxx_enum_class_uint32_t<RepositoryStatus>;
        /** Queries a local cache to determine if the locks and/or codes in this request are available to be acquired by this request's briefcase. Does not contact server, therefore faster than QueryAvailability() but potentially less accurate */
        FastQueryAvailability(): cxx_enum_class_uint32_t<RepositoryStatus>;

        /** Add a request to exclusively lock an element */
        AddElement(element: DgnElementP): void;
        /** Add a request to lock a model at the specified level */
        AddModel(model: DgnModelP, level: cxx_enum_class_uint8_t<LockLevel>): void;
        /** Add a request to lock the briefcase at the specified level */
        AddBriefcase(level: cxx_enum_class_uint8_t<LockLevel>): void;
        /** Add a request to reserve a code */
        AddCode(code: DgnCodeP): void;

        OnDispose(): void;
        Dispose(): void;
    }

    type RepositoryRequestP = cxx_pointer<RepositoryRequest>;

    /* ------------------------------------------ EC -----------------------------------------------*/

    /**
     * Provides access to ECSchemas and ECClasses within a DgnDb
     */
    class SchemaManager implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECDbSchemaManager ***/
        /** Look up an ECClass by its name */
        GetECClass(schemaNameOrPrefix: Bentley_Utf8String, className: Bentley_Utf8String): ECClassP;
        /** Look up an ECClass by its ECClassId */
        GetECClassById(ecclassid: DgnObjectIdP): ECClassP;
        OnDispose(): void;
        Dispose(): void;
    }

    type SchemaManagerP = cxx_pointer<SchemaManager>;

    /**
     * ECSchema - Projection of BentleyApi::ECN::ECSchema
     */
    class ECSchema implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECSchema ***/
        Name: Bentley_Utf8String;
        GetECClass(className: Bentley_Utf8String): ECClassP;

        OnDispose(): void;
        Dispose(): void;
    }

    type ECSchemaP = cxx_pointer<ECSchema>;

    /**
     * ECClass - Projection of BentleyApi::ECN::ECClass
     */
    class ECClass implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECClass ***/

        /** The ID of this class */
        ECClassId: DgnObjectIdP;

        /** The name of this class */
        Name: Bentley_Utf8String;

        /** The ECSql name of this class */
        ECSqlName: Bentley_Utf8String;

        /** The schema to which this class belongs */
        Schema: ECSchemaP;

        /** The base classes of this class */
        BaseClasses: ECClassCollectionP;

        /** The classes that derive from this class */
        DerivedClasses: ECClassCollectionP;

        /** The properties defined by this class and all of its base classes. */
        Properties: ECPropertyCollectionP;

        /**
         * Get the definition of the specified property of this class or any of its base classes
         * @param name The name of the property to look up
         * @return the property definition or null if no such property is found
         $$PUBLISH_INSERT_FILE$$ dgnJsApi_ECClass_GetProperty.sampleCode
         */
        GetProperty(name: Bentley_Utf8String): ECPropertyP;

        /**
         * Query the specified custom attribute on this property definition
         * @param className The class of the custom attribute to look up
         * @return the custom attribute or null if no such custom attribute is defined for this property.
         */
        GetCustomAttribute(className: Bentley_Utf8String): ECInstanceP;

        /** Create a non-persistent instance of this ECClass */
        MakeInstance(): ECInstanceP;

        OnDispose(): void;
        Dispose(): void;
    }

    type ECClassP = cxx_pointer<ECClass>;

    /**
     * ECInstance - Projection of BentleyApi::ECN::ECInstance
     */
    class ECInstance implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECInstance ***/
        Class: ECClassP;
        GetValue(propertyName: Bentley_Utf8String): ECValueP;
        SetValue(propertyName: Bentley_Utf8String, value: ECValueP): void;

        OnDispose(): void;
        Dispose(): void;
    }

    type ECInstanceP = cxx_pointer<ECInstance>;

    /**
    * Provides read-only access to ad-hoc properties defined on an IECInstance.
    * Adhoc properties are name-value pairs stored on an ECInstance.
    */
    class AdhocPropertyQuery implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsAdhocPropertyQuery ***/
        constructor(instance: ECInstanceP, accessString: Bentley_Utf8String);
        Host: ECInstanceP;

        GetPropertyIndex(accessString: Bentley_Utf8String): cxx_uint32_t;
        Count: cxx_uint32_t;

        GetName(index: cxx_uint32_t): Bentley_Utf8String;
        GetDisplayLabel(index: cxx_uint32_t): Bentley_Utf8String;
        GetValue(index: cxx_uint32_t) : ECValueP;
        GetPrimitiveType(index: cxx_uint32_t): cxx_enum_class_uint32_t<ECPropertyPrimitiveType>;
        GetUnitName(index: cxx_uint32_t): Bentley_Utf8String;
        IsReadOnly(index: cxx_uint32_t): cxx_bool;
        IsHidden(index: cxx_uint32_t): cxx_bool;

        OnDispose(): void;
        Dispose(): void;
    }

    /**
     * ECValue - Projection of BentleyApi::ECN::ECValue
     */
    class ECValue implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECValue ***/

        static FromDouble(v: cxx_double): ECValueP;
        static FromInteger(v: cxx_int32_t): ECValueP;
        static FromString(v: Bentley_Utf8String): ECValueP;
        static FromDateTime(v: Bentley_Utf8String): ECValueP;
        static FromPoint3d(v: DPoint3dP): ECValueP;

        IsNull: cxx_bool;
        IsPrimitive: cxx_bool;
        PrimitiveType: cxx_enum_class_uint32_t<ECPropertyPrimitiveType>;
        GetString(): Bentley_Utf8String;
        GetInteger(): cxx_int32_t;
        GetDouble(): cxx_double;
        GetPoint3d(): DPoint3dP;
        GetDateTime(): Bentley_Utf8String;

        OnDispose(): void;
        Dispose(): void;
    }

    type ECValueP = cxx_pointer<ECValue>;

      class ECClassCollectionIterator implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsECClassCollectionIterator ***/
        OnDispose(): void;
        Dispose(): void;
    }

    type ECClassCollectionIteratorP = cxx_pointer<ECClassCollectionIterator>;

    /**
     * A collection of ECClasses
     */
    class ECClassCollection implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsECClassCollection ***/
        Begin(): ECClassCollectionIteratorP;
        IsValid(iter: ECClassCollectionIteratorP): cxx_bool;
        ToNext(iter: ECClassCollectionIteratorP): cxx_bool;
        GetECClass(iter: ECClassCollectionIteratorP): ECClassP;

        OnDispose(): void;
        Dispose(): void;
    }

    type ECClassCollectionP = cxx_pointer<ECClassCollection>;

    /**
     * ECProperty - Projection of BentleyApi::ECN::ECProperty
     */
    class ECProperty implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECProperty ***/

        /** The name of the property */
        Name: Bentley_Utf8String;

        /** If this property holds a primitive type, return a PrimitiveECProperty. If not, it holds a struct or an array. */
        GetAsPrimitiveProperty(): PrimitiveECPropertyP;

        /**
         * Query the specified custom attribute on this property definition
         * @param className The class of the custom attribute to look up
         * @return the custom attribute or null if no such custom attribute is defined for this property.
         */
        GetCustomAttribute(className: Bentley_Utf8String): ECInstanceP;

        OnDispose(): void;
        Dispose(): void;
    }

    type ECPropertyP = cxx_pointer<ECProperty>;

    /**
     * PrimitiveECProperty - Projection of BentleyApi::ECN::PrimitiveECProperty
     */
    class PrimitiveECProperty extends ECProperty implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsPrimitiveECProperty ***/
        Type: cxx_enum_class_uint32_t<ECPropertyPrimitiveType>;

        OnDispose(): void;
        Dispose(): void;
    }

    type PrimitiveECPropertyP = cxx_pointer<PrimitiveECProperty>;

    class ECPropertyCollectionIterator implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsECPropertyCollectionIterator ***/
        OnDispose(): void;
        Dispose(): void;
    }

    type ECPropertyCollectionIteratorP = cxx_pointer<ECPropertyCollectionIterator>;

    /**
     * A collection of ECProperties
     */
    class ECPropertyCollection implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsECPropertyCollection ***/
        Begin(): ECPropertyCollectionIteratorP;
        IsValid(iter: ECPropertyCollectionIteratorP): cxx_bool;
        ToNext(iter: ECPropertyCollectionIteratorP): cxx_bool;
        GetECProperty(iter: ECPropertyCollectionIteratorP): ECPropertyP;

        OnDispose(): void;
        Dispose(): void;
    }

    type ECPropertyCollectionP = cxx_pointer<ECPropertyCollection>;

    /**
     * A ViewController
     */
    class ViewController implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsViewController ***/

        OnDispose(): void;
        Dispose(): void;
    }

    type ViewControllerP = cxx_pointer<ViewController>;

    /**
     * A DgnViewport
     */
    class Viewport implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsViewport ***/

        /** The ViewController for this view */
        ViewController: ViewControllerP;

        OnDispose(): void;
        Dispose(): void;
    }

    type ViewportP = cxx_pointer<Viewport>;

}
