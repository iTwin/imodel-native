/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScript/DgnJsApi/ts/DgnJsApi.d.ts $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
declare module Bentley.Dgn /*** NATIVE_TYPE_NAME = BentleyApi::Dgn ***/ 
{
    /*** BEGIN_FORWARD_DECLARATIONS ***/
    class DPoint3d { /*** NATIVE_TYPE_NAME = JsDPoint3d ***/ }
    class YawPitchRollAngles { /*** NATIVE_TYPE_NAME = JsYawPitchRollAngles ***/ }
    class SolidPrimitive extends Geometry { /*** NATIVE_TYPE_NAME = JsSolidPrimitive ***/ }
    class DgnSphere extends SolidPrimitive {/*** NATIVE_TYPE_NAME = JsDgnSphere ***/ }
    class DgnBox extends SolidPrimitive {/*** NATIVE_TYPE_NAME = JsDgnBpx ***/ }
    class Geometry { /*** NATIVE_TYPE_NAME = JsGeometry ***/ }
    /*** END_FORWARD_DECLARATIONS ***/

    type DPoint3dP              = cxx_pointer<DPoint3d>;
    type YawPitchRollAnglesP    = cxx_pointer<YawPitchRollAngles>;
    type SolidPrimitiveP        = cxx_pointer<SolidPrimitive>;
    type DgnSphereP             = cxx_pointer<DgnSphere>;
    type DgnBoxP                = cxx_pointer<DgnBox>;
    type GeometryP              = cxx_pointer<Geometry>;

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

        /**
         * Make sure that the specified script is loaded.
         * @param db         The name of the DgnDb to check for a local script library
         * @param scriptName The name which was used to register the script in the script librray
         * @return 0 (SUCCESS) if the script was loaded; otherwise, a non-zero error code.
         */
        static LoadScript(db: DgnDbP, scriptName: Bentley_Utf8String): cxx_int32_t;

        /**
         * Make sure that the specified library is loaded
         * @param libName The name of the library that is to be loaded
         * @note This function differs from LoadScript in that ImportLibrary is used to activate libraries that are provided by the app or the domain,
         * where LoadScript is used to load scripts that are found in the script library.
         */
        static ImportLibrary(libName: Bentley_Utf8String): void;

        /**
         * Report an error. An error is more than a message. The platform is will treat it as an error. For example, the platform may terminate the current command.
         * @param description A description of the error
         */
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
        OnDispose(): void;
        Dispose(): void;
    }

    type DgnModelP = cxx_pointer<DgnModel>;

    /**
     * A "sandbox" model where a component definition script can write elements and aspects that will be harvested by the platform and combined into instances.
     */
    class ComponentModel extends DgnModel implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsComponentModel ***/
        
        OnDispose(): void;
        Dispose(): void;
    }

    type ComponentModelP = cxx_pointer<ComponentModel>;

    /**
     * A component definition
     */
    class ComponentDef implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsComponentDef ***/
        
        /**
         * The name of the component
         */
        Name: Bentley_Utf8String;

        /**
         * The ECClass of the component
         */
        ComponentECClass: ECClassP;

        /**
         * The Category of the component
         */
        Category: DgnCategoryP;

        /**
         * The name of the CodeAuthority of the component
         */
        CodeAuthority: Bentley_Utf8String;

        /** Find a component definition by name 
         * @param db The DgnDB that contains the component definition
         * @param name The name of the component definition to search for.
         * @return A pointer to the component model with that name or null if not found.
         */
        static FindByName(db: DgnDbP, name: Bentley_Utf8String): ComponentDefP;

        /** Look up a variation by name
         * @param variationName The name of the variation to look up.
         * @return the variation or null if not found
         */
        QueryVariationByName(variationName: Bentley_Utf8String): DgnElementP;

        /** Return the properties of the specified instance of this component in the form of an IECInstance.
         * @param instance The component instance element.
         * @returns nullptr if \a instance is an instance of a component
         * @see QueryVariationByName
         */
        static GetParameters(instance: DgnElementP): ECInstanceP;

        /**
         * Make an ECInstance whose properties are the input parameters to the ComponentDef's script or solver. 
         * The caller should then assign values to the properties of the instance.
         * The caller may then pass the parameters instance to a function such as MakeInstanceOfVariation or MakeUniqueInstance.
         */
        MakeParameters(): ECInstanceP;

        /** Place an instance of a component 
         * If \a variation has instance parameters, then the \a instanceParameters argument may be passed into specific the instance parameter values to use.
         * If the values in \a instanceParameters differs from the instance parameters of \a variation, then a unique instance is created.
         * If \a instanceParameters is not specified or if it matches the instance parameters of \a variation, then a copy of \a variation is made.
         * @note Per-type parameters are ignored when comparing \a parameters to \a variation. It is illegal for the caller to specify new values for per-type parameters.
         * @param targetModel The model where the new instance will be placed.
         * @param variation The variation that is to be turned into an instance.
         * @param instanceParameters The instance parameters to use. If null, then default instance parameter values are used. Pass null if the variation has no instance parameters.
         * @param code Optional. The code to assign to the new instance. If invalid, then a code will be generated by the CodeAuthority associated with this component definition.
         * @return null if the variation or instanceParamters are invalie. Otherwise, instance that was created and persisted in \a destModel. If more than one element was created, the returned element is the parent. 
         * @see QueryVariationByName, GetParameters
         */
        MakeInstanceOfVariation(targetModel: DgnModelP, variation: DgnElementP, instanceParameters: ECInstanceP, code: AuthorityIssuedCode): DgnElementP;

        /**
         * Make a unique instance that is not based on a pre-defined variation. This method must be used if \a parameters include per-instance parameters that do not match the default values
         * of any pre-defined variation. This method may also be used for components that do not have pre-defined variations.
         * @note This function should not be used when the compponent definition does define a set of variations. In that case, call MakeInstanceOfVariation instead.
         * @param[out] stat        Optional. If not null, then an error code is stored here in case the creation of the instance fails.
         * @param[in] targetModel  The model where the instance is to be inserted
         * @param[in] parameters   The parameters that specify the desired variation
         * @param[in] code         Optional. The code to assign to the new item. If invalid, then a code will be generated by the CodeAuthority associated with this component model
         * @return A handle to the instance that was created and persisted in \a destModel. If more than one element was created, the returned element is the parent. If the instance
         * cannot be created, then this function returns nullptr and sets \a stat to a non-error status. Some of the possible error values include:
         *     * DgnDbStatus::WrongClass - \a parameters is not an instance of a component definition ECClass.
         *     * DgnDbStatus::WrongDgnDb - \a parameters and \a targetModel must both be in the same DgnDb.
         *     * DgnDbStatus::BadRequest - The component's geometry could not be generated, possibly because the values in \a parameters are invalid.
         * @see MakeInstanceOfVariation
         */
        MakeUniqueInstance(targetModel: DgnModelP, instanceParameters: ECInstanceP, code: AuthorityIssuedCode): DgnElementP;

        OnDispose(): void;
        Dispose(): void;
    }

    type ComponentDefP = cxx_pointer<ComponentDef>;

    class ElementGeometryBuilder implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsElementGeometryBuilder ***/ 
        constructor(el: DgnElementP, o: DPoint3dP, angles: YawPitchRollAnglesP);
        Append(geometry: GeometryP): void;
        AppendSolidPrimitive(geometry: SolidPrimitiveP): void;
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

        MakeInstance(): ECInstanceP;

        OnDispose(): void;
        Dispose(): void;
    }

    type ECClassP = cxx_pointer<ECClass>;

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

    //! Provides read-only access to ad-hoc properties defined on an IECInstance.
    //! Adhoc properties are name-value pairs stored on an ECInstance.
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

    class ECValue implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECValue ***/

        static FromDouble(v: cxx_double): ECValueP;
        static FromString(v: Bentley_Utf8String): ECValueP;

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
