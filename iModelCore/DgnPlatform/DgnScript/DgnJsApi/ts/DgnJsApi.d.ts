/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnScript/DgnJsApi/ts/DgnJsApi.d.ts $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/** @namespace Bentley::Dgn Types defined by the Bentley DgnPlatform TypeScript/JavaScript API
*/

declare module Bentley.Dgn /*** NATIVE_TYPE_NAME = BentleyApi::Dgn ***/ 
{
    /*** BEGIN_FORWARD_DECLARATIONS ***/
    class Transform { /*** NATIVE_TYPE_NAME = JsTransform ***/ }
    class DPoint3d { /*** NATIVE_TYPE_NAME = JsDPoint3d ***/ }
    class YawPitchRollAngles { /*** NATIVE_TYPE_NAME = JsYawPitchRollAngles ***/ }
    class SolidPrimitive extends Geometry { /*** NATIVE_TYPE_NAME = JsSolidPrimitive ***/ }
    class DgnSphere extends SolidPrimitive {/*** NATIVE_TYPE_NAME = JsDgnSphere ***/ }
    class DgnBox extends SolidPrimitive {/*** NATIVE_TYPE_NAME = JsDgnBpx ***/ }
    class Geometry { /*** NATIVE_TYPE_NAME = JsGeometry ***/ }
    /*** END_FORWARD_DECLARATIONS ***/

    type TransformP = cxx_pointer<Transform>;
    type DPoint3dP = cxx_pointer<DPoint3d>;
    type YawPitchRollAnglesP = cxx_pointer<YawPitchRollAngles>;
    type SolidPrimitiveP = cxx_pointer<SolidPrimitive>;
    type DgnSphereP = cxx_pointer<DgnSphere>;
    type DgnBoxP = cxx_pointer<DgnBox>;
    type GeometryP = cxx_pointer<Geometry>;

    enum ECPropertyPrimitiveType { }

    //! Logging serverity level.
    enum LoggingSeverity { }

    /** Access to the message log */
    class Logging implements BeJsProjection_SuppressConstructor
    {

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
    class Placement3d implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsPlacement3d ***/
        constructor(origin: DPoint3dP, angles: YawPitchRollAnglesP);

        /** The origin of the placement */
        Origin: DPoint3dP;
        /** The angles of the placement */
        Angles: YawPitchRollAnglesP;

        OnDispose(): void;
        Dispose(): void;
    }

    type Placement3dP = cxx_pointer<Placement3d>;

    /** Script Management Utilities */
    class Script implements BeJsProjection_SuppressConstructor
    {

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
    }

    /** DgnDb - Projection of BentleyApi::Dgn::DgnDb */
    class DgnDb implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsDgnDb ***/
        /** The collection of models in the DgnDb */
        Models: DgnModelsP;
        /** The collection of ECSchemas in the DgnDb */
        Schemas: SchemaManagerP;
        OnDispose(): void;
        Dispose(): void;
    }

    type DgnDbP = cxx_pointer<DgnDb>;

    /** A 64-bit ID.  */
    class DgnObjectId implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsDgnObjectId ***/
        /** Tests if the ID is valid */
        IsValid(): cxx_bool;
        /** Tests if the ID matches another ID @param id The other ID */
        Equals(id: DgnObjectIdP): cxx_bool;
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

    /** AuthorityIssuedCodeValue - Projection of BentleyApi::Dgn::DgnCode */
    class AuthorityIssuedCodeValue implements IDisposable, BeJsProjection_SuppressConstructor, BeJsProjection_RefCounted
    {
        /*** NATIVE_TYPE_NAME = JsAuthorityIssuedCode ***/
        OnDispose(): void;
        Dispose(): void;
    }

    type AuthorityIssuedCode = cxx_pointer<AuthorityIssuedCodeValue>;

    /** DgnModels - Projection of BentleyApi::Dgn::DgnModels */
    class DgnModels implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsDgnModels ***/
        /** Look up a DgnModelId by the model's code. @param name The code to look up. @return The ID of the corresponding model if found */
        QueryModelId(name: AuthorityIssuedCode): DgnObjectIdP;
        /** Find or load the model identified by the specified ID. @param id The model ID. @return The loaded model or null if not found */
        GetModel(id: DgnObjectIdP): DgnModelP;
        OnDispose(): void;
        Dispose(): void;
    }

    type DgnModelsP = cxx_pointer<DgnModels>;

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

    /** DgnElement - Projection of BentleyApi::Dgn::DgnElement
     *  <h2>Properties</h2>
     *  On any given element, there may be the following kinds of properties:
     *  * Properties that are defined by the ECClass 
     *          * Properties that are controlled by the C++ element class
     *              * You must use methods on that class to access them
     *          * Properties that are not controlled by the C++ element class – “Unhandled Properties”
     *              * You must use the Get/SetUnhandledProperty functions to access them
     *  * Properties that are not defined by the ECClass but are added by the user – “User Properties”
     *          * You must use the UserProperties.Get/SetValueEC methods to access them
     */
    class DgnElement implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsDgnElement ***/ 
        /** The Element's ID */
        ElementId: DgnObjectIdP;
        /** The Element's Code */
        Code: AuthorityIssuedCode;
        /** The Model that contains the Element */
        Model: DgnModelP;
        /** The ECClass of this element */
        ElementClass: ECClassP;
        /** Insert this Element into its Model in the DgnDb. @return non-zero if the insert failed. */
        Insert(): cxx_int32_t;
        /** Update this Element in its Model in the DgnDb. @return non-zero if the update failed. */
        Update(): cxx_int32_t;
        /** Set the Parent of this Element. @param parent The parent element. */
        SetParent(parent: cxx_pointer<DgnElement>): void;

        /**
         * Get the value of a property that is not controlled by the element's class.
         * @note A handled property must be accessed using the API provided by the class that defines it.
         * @param name The name of property
         * @return the value of the property or null if the property is not found or is handled
         */
        GetUnhandledProperty(name: Bentley_Utf8String): ECValueP;

        /**
         * Set the value of a property that is not controlled by the element's class
         * @note A handled property must be accessed using the API provided by the class that defines it.
         * @param name      The name of property
         * @param value     The new value for the property
         * @return non-zero error status if the property is not found or is handled
         */
        SetUnhandledProperty(name: Bentley_Utf8String, value: ECValueP): cxx_int32_t;

        /**
         * Get/set the user properties for this element
         */
        UserProperties: AdHocJsonValueP;

        OnDispose(): void;
        Dispose(): void;
    }

    type DgnElementP = cxx_pointer<DgnElement>;

    /** PhysicalElement - Projection of BentleyApi::Dgn::PhysicalElement */
    class PhysicalElement extends DgnElement implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsPhysicalElement ***/

        /** Get the placement of this element */
        Placement: Placement3dP;

        /** Transform the element's Placement 
         * @param transform The transform to apply to the element's Placement. The transform must be pure rotation and/or translation.
         * @return non-zero error status if the element could not be transformed or if \a transform is invalid.
        */
        Transform(transform: TransformP): cxx_int32_t;

        /** Get the element's DgnCategoryId */
        CategoryId: DgnObjectIdP;
    
        /** The element's geometry (read-only) */
        Geometry: GeometryCollectionP;

        /**
         * Create a new PhysicalElement
         * @param model The model that is to contain the new element
         * @param categoryId The ID of the category to assign to the new element
         * @param elementClassName Optional. The name of the element's ECClass. If not specified, then dgn.PhysicalElement is used
         * @return a new, non-persistent PhysicalElement or null if one of the parameters is invalid
         * @see Insert
        */
        static Create(model: DgnModelP, categoryId: DgnObjectIdP, elementClassName: Bentley_Utf8String): PhysicalElementP;
    }

    type PhysicalElementP = cxx_pointer<PhysicalElement>;

    /** DgnModel - Projection of BentleyApi::Dgn::DgnModel */
    class DgnModel implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsDgnModel ***/
        /** The ID of this model */
        ModelId: DgnObjectIdP;
        /** The Code of this model */
        Code: AuthorityIssuedCode;
        /** The DgnDb that contains this model */
        DgnDb: DgnDbP;
        /** Make a DgnModelCode from a string. @param name The name to use. @return The DgnModelCode based on the specified name. */
        static CreateModelCode(name: Bentley_Utf8String): AuthorityIssuedCode;
        OnDispose(): void;
        Dispose(): void;
    }

    type DgnModelP = cxx_pointer<DgnModel>;

    /** ComponentModel - Projection of BentleyApi::Dgn::ComponentModel */
    class ComponentModel extends DgnModel implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsComponentModel ***/

        OnDispose(): void;
        Dispose(): void;
    }

    type ComponentModelP = cxx_pointer<ComponentModel>;

    /** ComponentDef - Projection of BentleyApi::Dgn::ComponentDef */
    class ComponentDef implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
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

        /** Place an instance of a component.
         * If \a variation has instance parameters, then the \a instanceParameters argument may be supplied in order to supply the values of the instance parameter values to use.
         * If the values in \a instanceParameters differ from the instance parameters of \a variation, then a unique instance is created.
         * If \a instanceParameters is not supplied or if it matches the instance parameters of \a variation, then a copy of \a variation is made.
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
         * Make a unique instance that is not based on a pre-defined variation. This method must be used if \a instanceParameters includes per-instance parameters that do not match the default values
         * of any pre-defined variation. This method may also be used for components that do not have pre-defined variations.
         * @note This function should not be used when the compponent definition does define a set of variations. In that case, call MakeInstanceOfVariation instead.
         * @param[in] targetModel  The model where the instance is to be inserted
         * @param[in] instanceParameters The parameters for the new instance
         * @param[in] code         Optional. The code to assign to the new item. If invalid, then a code will be generated by the CodeAuthority associated with this component model
         * @return A handle to the instance that was created and persisted in \a destModel. If more than one element was created, the returned element is the parent. If the instance
         * cannot be created, then this function returns nullptr and sets \a stat to a non-error status. Some of the possible error values include:
         *     * DgnDbStatus::WrongClass - \a instanceParameters is not an instance of a component definition ECClass.
         *     * DgnDbStatus::WrongDgnDb - \a instanceParameters and \a targetModel must both be in the same DgnDb.
         *     * DgnDbStatus::BadRequest - The component's geometry could not be generated, possibly because the values in \a instanceParameters are invalid.
         * @see MakeInstanceOfVariation
         */
        MakeUniqueInstance(targetModel: DgnModelP, instanceParameters: ECInstanceP, code: AuthorityIssuedCode): DgnElementP;

        OnDispose(): void;
        Dispose(): void;
    }

    type ComponentDefP = cxx_pointer<ComponentDef>;

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

    enum RenderFillDisplay { }
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
        /** Set the geometry's fill color to match the view background. */
        SetFillColorToViewBackground(): void;
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

    /**
     * GeometryBuilder - Projection of BentleyApi::Dgn::GeometryBuilder
     */
    class GeometryBuilder implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsGeometryBuilder ***/ 

        /**
         * Construct a new GeometryBuilder with the intention of using it to set up the geometry of the specified element.
         * @param el    The element to which this geometry will be attached
         * @param o     The placement origin
         * @param angles The placement angles
         * @see SetGeometryStreamAndPlacement for how to copy the geometry in this builder to an element.
        */
        constructor(el: DgnElementP, o: DPoint3dP, angles: YawPitchRollAnglesP);

        /**
         * Construct a new GeometryBuilder to prepare geometry for the specified element. 
         * @note This is just a short cut for calling CreateForModel.
         * @param el    The element to which this geometry will be attached
         * @param o     The placement origin
         * @param angles The placement angles
         * @return a GeometryBuilder object
         * @see CreateForModel
         */
        static CreateForElement(el: DgnElementP, o: DPoint3dP, angles: YawPitchRollAnglesP): GeometryBuilderP;

        /**
         * Construct a new GeometryBuilder to prepare geometry for elements in the specified model and category
         * @param model The model where the geometry will ultimatley be stored
         * @param catid The category of the element that will will ultimatley contain the geometry
         * @param o     The placement origin
         * @param angles The placement angles
         * @return a GeometryBuilder object
         */
        static CreateForModel(model: DgnModelP, catid: DgnObjectIdP, o: DPoint3dP, angles: YawPitchRollAnglesP): GeometryBuilderP;

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
         */
        AppendGeometry(geometry: GeometryP): void;

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
        SetGeometryStreamAndPlacement(element: DgnElementP): cxx_int32_t;

        /**
         * Copy the geometry in this builder to a DgnGeometryPart.
         * @param part  The DgnGeometryPart
         * @return non-zero error status if \a part is invalid or if this geometry stream is invalid
         */
        SetGeometryStream(part: DgnGeometryPartP): cxx_int32_t;

        OnDispose(): void;
        Dispose(): void;
    }

    type GeometryBuilderP = cxx_pointer<GeometryBuilder>;

    /* ------------------------------------------ ScriptBasedTool -----------------------------------------------*/

    /**
     * HitDetail
     */
    class HitDetail implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor {
        /*** NATIVE_TYPE_NAME = JsHitDetail ***/ 

        /** The element that was picked */
        Element: PhysicalElementP;

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

    /* ------------------------------------------ EC -----------------------------------------------*/

    /**
     * Provides access to ECSchemas and ECClasses within a DgnDb
     */
    class SchemaManager implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECDbSchemaManager ***/
        GetECClass(schemaNameOrPrefix: Bentley_Utf8String, className: Bentley_Utf8String): ECClassP;
        OnDispose(): void;
        Dispose(): void;
    }

    type SchemaManagerP = cxx_pointer<SchemaManager>;

    /**
     * ECSchema
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
     * ECClass
     */
    class ECClass implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsECClass ***/ 

        /** The name of the class */
        Name: Bentley_Utf8String;

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
     * ECInstance
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
     * ECValue
     */
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

    /**
     * AdHocJsonValue
     */
    class AdHocJsonValue implements IDisposable, BeJsProjection_RefCounted, BeJsProjection_SuppressConstructor
    {
        /*** NATIVE_TYPE_NAME = JsAdHocJsonValue ***/

        /** Set the value of the specified ad hoc property.
         * @param name  The name of the property
         * @param value The value of the property.
         */
        SetValueEC(name: Bentley_Utf8String, value: ECValueP): void;

        /**
         * Set the units of the property value
         * @param[in] name Name of the property
         * @param[in] units String representing the units of the property value
         */
        SetUnits(name: Bentley_Utf8String, units: Bentley_Utf8String): void;

        /** Remove the specified ad hoc property.
         * @param name  The name of the property
         */
        RemoveValue(name: Bentley_Utf8String): void;

        /** Get the value of the specified ad hoc property.
         * @param name  The name of the property
         * @return the value of the property or null if the property is not found.
         */
        GetValueEC(name: Bentley_Utf8String): ECValueP;

        /**
         * Get the units of the property value
         * @param[in] name Name of the property
         * @return units String representing the units of the property
         */
        GetUnits(name: Bentley_Utf8String): Bentley_Utf8String;

        OnDispose(): void;
        Dispose(): void;
    }

    type AdHocJsonValueP = cxx_pointer<AdHocJsonValue>;

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
     * An ECProperty.
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
     * A PrimitiveECProperty
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

    type ECPropertyCollectionP = cxx_pointer<ECPropertyCollection>;}
