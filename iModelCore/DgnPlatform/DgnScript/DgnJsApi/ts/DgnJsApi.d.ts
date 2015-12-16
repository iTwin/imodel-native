declare module Bentley.Dgn /*** NATIVE_TYPE_NAME = BentleyApi::Dgn ***/ 
{
    /*** BEGIN_FORWARD_DECLARATIONS ***/
    class DPoint3d { /*** NATIVE_TYPE_NAME = JsDPoint3d ***/ }
    class YawPitchRollAngles { /*** NATIVE_TYPE_NAME = JsYawPitchRollAngles ***/ }
    class SolidPrimitive { /*** NATIVE_TYPE_NAME = JsSolidPrimitive ***/ }
    /*** END_FORWARD_DECLARATIONS ***/

    type DPoint3dP = cxx_pointer<DPoint3d>;
    type YawPitchRollAnglesP = cxx_pointer<YawPitchRollAngles>;
    type SolidPrimitiveP = cxx_pointer<SolidPrimitive>;

    enum ECPropertyPrimitiveType { }

    //! Logging serverity level.
    enum LoggingSeverity { }

    //! Access to the message log
    class Logging implements BeJsProjection_SuppressConstructor {

        //! Set the severity level for the specified category
        //! @param catagory     The logging category
        //! @param severity     The minimum severity to display. Note that messages will not be logged if their severity is below this level.
        static SetSeverity(category: Bentley_Utf8String, severity: cxx_enum_class_uint32_t<LoggingSeverity>): void;

        //! Test if the specified severity level is enabled for the specified category
        //! @param category     The logging category
        //! @param severity     The severity to test
        static IsSeverityEnabled(category: Bentley_Utf8String, severity: cxx_enum_class_uint32_t<LoggingSeverity>): cxx_bool;

        //! Send a message to the log
        //! @param catagory     The logging category
        //! @param severity     The severity of the message. Note that the message will not be logged if \a severity is below the severity level set by calling SetSeverity
        //! @param message      The message to log
        static Message(category: Bentley_Utf8String, severity: cxx_enum_class_uint32_t<LoggingSeverity>, message: Bentley_Utf8String): void;
    }

    class Placement3d implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsPlacement3d ***/
        constructor(origin: DPoint3dP, angles: YawPitchRollAnglesP);

        Origin: DPoint3dP;
        Angles: YawPitchRollAnglesP;

        OnDispose(): void;
        Dispose(): void;
    }

    type Placement3dP = cxx_pointer<Placement3d>;

    //! Script Management Utilities
    class Script implements BeJsProjection_SuppressConstructor {

        //! Make sure the that specified library is loaded
        //! @param libName  The name of the library that is to be loaded
        static ImportLibrary(libName: Bentley_Utf8String): void;

        //! Report an error. An error is more than a message. The platform is will treat it as an error. For example, the platform may terminate the current command.
        //! @param description  A description of the error
        static ReportError(description: Bentley_Utf8String): void;
    }

    class DgnDb implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsDgnDb ***/
        Models: DgnModelsP;
        Schemas: SchemaManagerP;
        OnDispose(): void;
        Dispose(): void;
    }

    type DgnDbP = cxx_pointer<DgnDb>;

    //! A wrapper for 64-bit element ids, etc.
    class DgnObjectId implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsDgnObjectId ***/
        IsValid(): cxx_bool;
        Equals(id: DgnObjectIdP): cxx_bool;
        OnDispose(): void;
        Dispose(): void;
    }

    type DgnObjectIdP = cxx_pointer<DgnObjectId>;

    class DgnObjectIdSetIterator implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsDgnObjectIdSetIterator ***/
        OnDispose(): void;
        Dispose(): void;
    }

    type DgnObjectIdSetIteratorP = cxx_pointer<DgnObjectIdSetIterator>;

    //! A wrapper for 64-bit element ids, etc.
    class DgnObjectIdSet implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsDgnObjectIdSet ***/
        Size(): cxx_double;
        Clear(): void;

        Insert(id: DgnObjectIdP): void;
        Begin(): DgnObjectIdSetIteratorP;
        IsValid(iter: DgnObjectIdSetIteratorP): cxx_bool;
        ToNext(iter: DgnObjectIdSetIteratorP): cxx_bool;
        GetId(iter: DgnObjectIdSetIteratorP): DgnObjectIdP;

        OnDispose(): void;
        Dispose(): void;
    }

    type DgnObjectIdSetP = cxx_pointer<DgnObjectIdSet>;

    class AuthorityIssuedCodeValue implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsAuthorityIssuedCode ***/
        OnDispose(): void;
        Dispose(): void;
    }

    type AuthorityIssuedCode = cxx_pointer<AuthorityIssuedCodeValue>;

    class DgnModels implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsDgnModels ***/
        QueryModelId(name: AuthorityIssuedCode): DgnObjectIdP;
        GetModel(id: DgnObjectIdP): DgnModelP;
        OnDispose(): void;
        Dispose(): void;
    }

    type DgnModelsP = cxx_pointer<DgnModels>;

    class DgnCategory implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsDgnCategory ***/
        DgnDb: DgnDbP;
        CategoryId: DgnObjectIdP;
        DefaultSubCategoryId: DgnObjectIdP;
        CategoryName: Bentley_Utf8String;
        static QueryCategoryId(name: Bentley_Utf8String, db: DgnDbP): DgnObjectIdP;
        static QueryCategory(id: DgnObjectIdP, db: DgnDbP): DgnCategoryP;
        static QueryCategories(db: DgnDbP): DgnObjectIdSetP;
        OnDispose(): void;
        Dispose(): void;
    }

    type DgnCategoryP = cxx_pointer<DgnCategory>;

    class DgnElement implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsDgnElement ***/ 
        ElementId: DgnObjectIdP;
        Code: AuthorityIssuedCode;
        Model: DgnModelP;
        Insert(): cxx_int32_t;
        Update(): cxx_int32_t;
        SetParent(parent: cxx_pointer<DgnElement>): void;
        OnDispose(): void;
        Dispose(): void;
    }

    type DgnElementP = cxx_pointer<DgnElement>;

    class PhysicalElement extends DgnElement implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsPhysicalElement ***/

        /**
         * Create a new PhysicalElement
         * @param model The model that is to contain the new element
         * @param categoryId The ID of the category to assign to the new element
         * @paeram elementClassname Optional. The name of the element's ECClass. If not specified, then dgn.PhysicalElement is used
         * @return a new, non-persistent PhysicalElement or null if one of the parameters is invalid
         * @see Insert
        */
        static Create(model: DgnModelP, categoryId: DgnObjectIdP, elementClassName: Bentley_Utf8String): PhysicalElementP;
    }

    type PhysicalElementP = cxx_pointer<PhysicalElement>;

    class DgnModel implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsDgnModel ***/
        ModelId: DgnObjectIdP;
        Code: AuthorityIssuedCode;
        DgnDb: DgnDbP;
        static CreateModelCode(name: Bentley_Utf8String): AuthorityIssuedCode;
        DeleteAllElements(): void;
        OnDispose(): void;
        Dispose(): void;
    }

    type DgnModelP = cxx_pointer<DgnModel>;

    class ElementGeometryBuilder implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsElementGeometryBuilder ***/ 
        constructor(el: DgnElementP, o: DPoint3dP, angles: YawPitchRollAnglesP);
        Append(solid: SolidPrimitiveP): void;
        SetGeomStreamAndPlacement(element: DgnElementP): cxx_double;
        OnDispose(): void;
        Dispose(): void;
    }


    /* ------------------------------------------ EC -----------------------------------------------*/

    class SchemaManager implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECDbSchemaManager ***/
        GetECClass(schemaNameOrPrefix: Bentley_Utf8String, className: Bentley_Utf8String): ECClassP;
        OnDispose(): void;
        Dispose(): void;
    }

    type SchemaManagerP = cxx_pointer<SchemaManager>;

    class ECSchema implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECSchema ***/
        Name: Bentley_Utf8String;
        GetECClass(className: Bentley_Utf8String): ECClassP;

        OnDispose(): void;
        Dispose(): void;
    }

    type ECSchemaP = cxx_pointer<ECSchema>;

    class ECClass implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECClass ***/ 
        Name: Bentley_Utf8String;
        BaseClasses: ECClassCollectionP;
        DerivedClasses: ECClassCollectionP;
        Properties: ECPropertyCollectionP;
        GetCustomAttribute(className: Bentley_Utf8String): ECInstanceP;

        OnDispose(): void;
        Dispose(): void;
    }

    type ECClassP = cxx_pointer<ECClass>;

    class ECInstance implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECInstance ***/
        Class: ECClassP;
        GetValue(propertyName: Bentley_Utf8String): ECValueP;

        OnDispose(): void;
        Dispose(): void;
    }

    type ECInstanceP = cxx_pointer<ECInstance>;

    class ECValue implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECValue ***/
        IsNull: cxx_bool;
        IsPrimitive: cxx_bool;
        PrimitiveType: cxx_enum_class_uint32_t<ECPropertyPrimitiveType>;
        GetString(): Bentley_Utf8String;
        GetInteger(): cxx_int32_t;
        GetDouble(): cxx_double;

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

    class ECProperty implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECProperty ***/
        Name: Bentley_Utf8String;
        IsPrimitive: cxx_bool;
        GetCustomAttribute(className: Bentley_Utf8String): ECInstanceP;

        OnDispose(): void;
        Dispose(): void;
    }

    type ECPropertyP = cxx_pointer<ECProperty>;

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

    type ECPropertyCollectionP = cxx_pointer<ECPropertyCollection>;}
