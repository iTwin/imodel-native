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

    class ComponentModel extends DgnModel implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsComponentModel ***/
        Name: Bentley_Utf8String;

        /** Find a component model by name 
         * @param db The DgnDB that contains the component model
         * @param name The name of the component model to search for.
         * @return A pointer to the component model with that name or null if not found.
         */
        static FindModelByName(db: DgnDbP, name: Bentley_Utf8String): ComponentModelP;

        /** Place an instance of a component 
         * @param targetModel The model where the new instance will be placed.
         * @param capturedSolutionName Optional. The name of the captured solution to copy. If not specified, then \a params must be specified.
         * @param params Optional. The parameters to use to look up or to generate the instance. If \a capturedSolutionName is specified, then params may be null.
         * @param code Optional. The code for the new instance. If not specified, then the newly created element will have no Code.
         * @return A new, persistent element that is an instance of the specified solution of the specified component, or null if the component could not supply that solution.
         */
        MakeInstance(targetModel: DgnModelP, capturedSolutionName: Bentley_Utf8String, params: Bentley_Utf8String, code: AuthorityIssuedCode): DgnElementP;
        OnDispose(): void;
        Dispose(): void;
    }

    type ComponentModelP = cxx_pointer<ComponentModel>;

    class GeometryBuilder implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsGeometryBuilder ***/ 
        constructor(el: DgnElementP, o: DPoint3dP, angles: YawPitchRollAnglesP);
        Append(solid: SolidPrimitiveP): void;
        SetGeometryStreamAndPlacement(element: DgnElementP): cxx_double;
        OnDispose(): void;
        Dispose(): void;
    }
}
