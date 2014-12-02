/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/IEcPropertyHandler.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <Bentley/bvector.h>
#include <Bentley/WString.h>
#include <RmgrTools/Tools/HeapzoneAllocator.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

#if defined (_MSC_VER)
    #pragma warning(push)
    #pragma warning (disable:4251)
#endif // defined (_MSC_VER)

struct IRefCountedEcPropertyHandler;
struct ScaleDefinition;

/*=================================================================================**//**
* \addtogroup ElementHandlers
* <h2>EC Properties</h2>
* A Handler can publish EC properties by implementing IEcPropertyHandler. EC Properties
* are used by the Info dialog, fields, and other parts of the MicroStation GUI.
* @beginGroup
*/

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
enum IsNullReturnType
{
ISNULLRETURN_NotNull       = 0,
ISNULLRETURN_IsNullCanSet  = 1,
ISNULLRETURN_IsNullCantSet = 2,
};

/*=================================================================================**//**
*
* Interface adopted by a Handler in order to supply EC properties.
*
* The handler defines a property by supplying access functions, an internal ID, and a display name. See IEcPropertyHandler::EcPropertyDescriptor for details.
* <p>
* The property manager calls the supplied get and set functions to access the value of a property.
* If the handler specifies the units of a property value, the property manager will take care of displaying it. Or, the handler
* can supply its own functions to format and parse the value of a property. See IEcPropertyHandler::EcValueAccessor for details.
* <p>
* The handler organizes properties into categories. See IEcPropertyHandler::EcPropertyCategory for details.
*
* <h4>ECPropertyEnabler vs. IEcPropertyHandler</h4>
*    <ul>
*     <li>IEcPropertyHandler
*        <ul>
*         <li>Implemented by an element Handler
*         <li>Handler is asked to supply properties only for the elements that it controls.
*         <li>Defined entirely in terms of native types.
*         <li>Defines a subset of the GUI customization capabilities offered by PropertyPane
*             <ul>
*             <li>No direct access to managed customization helpers, such as PropertyPane::Category or extended types.
*             <li>Some equivalent customization mechanisms, such as categories and standard types.
*             </ul>
*         </ul>
*     <li>ECPropertyEnablers
*         <ul>
*        <li>Implemented by an application.
*         <li>App is queried to supply properties for any type of element.
*        <li>Defined entirely in terms of managed types.
*         <li>Offers extensive GUI customization, based on helper libraries such as PropertyPane.
*         </ul>
*     </ul>
*
* <h4>Overall Design</h4>
*   \li You specify function pointers for getting and, optionally, setting the value of a given property.
*   \li You supply an internal integer ID for each property. This ID is passed to your get and set functions to identify the property.
*   \li You supply a public ID and display name for each property. The ID identifies your property to fields and the display name identifies it to users.
*   \li You group properties into categories.
*   \li You can group properties into arrays and/or structs.

* <h4>Main Classes</h4>
    \li EcValueAccessor -- Accesses a family of similar property values. Holds get and set functions to access the values. Controls how property values relate to displayable strings.
    \li EcPropertyDescriptor -- Defines a property. Holds the accessor, ID, and name.
    \li EcPropertyCategory -- Defines a category of properties. Holds a set of properties.

* <h4>Examples</h4>
* \code
    #include <DgnPlatform/DgnHandlers/IEcPropertyHandler.h>

    class MyHandler : Handler, IEcPropertyHandler
        {
        virtual WString     _GetEcPropertiesClassName (ElementHandleCR) override {return typeid(*this).raw_name ();}    // do not translate
        virtual StatusInt   _GetEcProperties (IEcPropertyHandler::T_EcCategories&, ElementHandleCR) override;
        }


    StatusInt LineWithBarbellEnds::_GetEcProperties
    (
    IEcPropertyHandler::T_EcCategories& cats,
    ElementHandleCR                      eh
    )
        {
        //  -----------------------------------------------------------------------------------------
        //          Define how various types of property values are accessed. I must have (at least) one
        //          function for each property value type.
        //  -----------------------------------------------------------------------------------------
        EcValueAccessor getDoubleUors       (GetDouble, NULL, EcValueAccessor::DistanceUors);
        EcValueAccessor getDoubleAngle      (GetDouble, NULL, EcValueAccessor::Direction);
        EcValueAccessor getBoolean          (GetBoolean, NULL);
        EcValueAccessor getSetDPoint3d      (getDPoint3d0, setDPoint3d0);
        EcValueAccessor getSetElementColor  (getElementHeaderInt0, setElementHeaderInt0, EcValueAccessor::ElementColor);
        EcValueAccessor getSetStuff         (GetStuffID, SetStuffID, GetStuffStringList);
        EcArrayValueAccessor arrayOfPoints  (getMyPointCount, getSetDPoint3d);

        //  -----------------------------------------------------------------------------------------
        //  Define my own category
        //  -----------------------------------------------------------------------------------------
        IEcPropertyHandler::EcPropertyCategory barbells (L"Barbells", getCatDesc(CATEGORY_Barbells));  // do not translate

        //          Add properties to my category

        barbells.push_back (EcPropertyDescriptor (getSetStuff,        PROPID_Stuff,       "Stuff",    GetLocalizedString (PROPID_Suff)));
        barbells.push_back (EcPropertyDescriptor (getBoolean,         PROPID_SomeBoolean, ...
        barbells.push_back (EcPropertyDescriptor (getSetDPoint3d,     PROPID_APoint,      ...
        barbells.push_back (EcPropertyDescriptor (getSetElementColor, PROPID_AColor,      ...
        barbells.push_back (EcPropertyDescriptor (arrayOfPoints,      PROPID_MyPointArray, ...
        barbells.push_back (EcPropertyDescriptor (new AStruct (),                         "MyStruct",  GetLocalizedString ( ...

        //          Add my category and the properties that it contains to the output list

        cats.push_back (barbells);

        //  -----------------------------------------------------------------------------------------
        //  Get a standard category
        //  -----------------------------------------------------------------------------------------
        IEcPropertyHandler::EcPropertyCategory geometry (IEcPropertyHandler::EcPropertyCategory::Geometry);

        //          Add properties to the standard category

        geometry.push_back (EcPropertyDescriptor (getDoubleUors,    PROPID_Length,      "MyLength",  GetLocalizedString (PROPID_MyLenght)));
        geometry.push_back (EcPropertyDescriptor (getDoubleAngle,   PROPID_MyAngle,     ...

        //          Report that I have added properties to this category

        cats.push_back (geometry);

        return SUCCESS;
        }

  \endcode
*
* <h5>Units</h5>
*   In the example above, two different accessors were defined for doubles with different units:
* \code
        EcValueAccessor getDoubleUors           (GetDouble, NULL, EcValueAccessor::DistanceUors);
        EcValueAccessor getDoubleDirectionAngle (GetDouble, NULL, EcValueAccessor::Direction);
  \endcode
    Both accessors use the same function, GetDouble. The function returns a double, and the accessor specifies
    the units for that value. Units are used to format values in the GUI. Here is the example GetDouble function:
* \code
    StatusInt       GetDouble
    (
    double&         propVal,
    ElementHandleCR    eh,
    UInt32          propID,
    size_t
    )
        {
        DPoint3d pts[2];
        int npts = 2;
        mdlLinear_extract (pts, &npts, eh.GetElementCP(), eh.GetDgnModel());

        switch (propID)
            {
            case PROPID_Length:
                propVal = bsiDPoint3d_distance (&pts[0], &pts[1]);
                return SUCCESS;

            case PROPID_MyAngle:
                propVal = atan2 (pts[1].y - pts[0].y, pts[1].x - pts[0].x);
                return SUCCESS;

            case PROPID_MyTransparency:
                propVal = 100.0 * mdlElement_getTransparency (eh.GetElementCP ());
                return SUCCESS;
            }

        BeAssert (false);
        return ERROR;
        }
  \endcode
*
* <h5>String Lists</h5>
*   In the example above, PROPID_Stuff is a string list.
* \code
        EcValueAccessor getSetStuff (GetStuffID, SetStuffID, GetStuffStringList);
        barbells.push_back (EcPropertyDescriptor (getSetStuff, PROPID_Stuff, "Stuff", GetLocalizedString (PROPID_Suff)));
  \endcode
*   The property value is an integer. It is a number that is associated with a string in the list. It is not necessarily
*   the index of the string. Here is the function that returns the string list that the user sees:
* \code
    void    GetStuffStringList
    (
    EcValueAccessor::T_IntStringPairs& strlist,
    ElementHandleCR    eh_unused,
    UInt32          propID_unused
    )
        {
        strlist.push_back (make_bpair (0, L"zero"));
        strlist.push_back (make_bpair (10, L"ten"));
        strlist.push_back (make_bpair (1, L"one"));
        }
  \endcode
*   The user sees and picks from the string list.
*   0, 10, and 1 are the integer values that the property can take on. GetStuffID and SetStuffID are integer property
*   value accessor functions.
*
* <h5>Structs</h5>
*   In the example above, PROPID_MyStruct is a property with a value that is a group of properties.
* \code
        barbells.push_back (EcPropertyDescriptor (new AStruct (),  "MyStruct", GetLocalizedString (...
  \endcode
*   Instead of an EcValueAccessor, we pass in the address of an IRefCountedEcPropertyHandler. This handler is called to generate
*   the set of nested properties. Here is the definition of \a AStruct:
* \code
    struct      AStruct : RefCounted<IRefCountedEcPropertyHandler>
    {
        virtual WString     _GetEcPropertiesClassName (ElementHandleCR) override {return typeid(*this).raw_name ();}    // do not translate

        virtual StatusInt   _GetEcProperties (IEcPropertyHandler::T_EcCategories& cats, ElementHandleCR host_eh_unused_) override
            {
            IEcPropertyHandler::EcPropertyCategory dummyCategory ...

            dummyCategory.push_back (EcPropertyDescriptor ( ...

            cats.push_back (dummyCategory);

            return SUCCESS;
            }

    }; // AStruct
  \endcode
*   The values of a struct are not shown in categories in the GUI, so the use of a category in AStruct::_GetEcProperties is just a formality.
*   Note that that struct-valued property itself does not have its own internal numerical ID, since it has no accessor. Instead, each member
*   of the struct has its own internal numerical ID and accessor.
*
* <h5>Arrays</h5>
*   In the example above, PROPID_MyPointArray is property with a value that is an array of points.
* \code
        EcArrayValueAccessor arrayOfPoints  (getMyPointCount, getSetDPoint3d);
        barbells.push_back (EcPropertyDescriptor (arrayOfPoints, PROPID_MyPointArray, ...
  \endcode
*   It supplies a function to get the array count. It also specifies the accessor used to get the individual items.
*   In this example, we use the same accessor, getSetDPoint3d, to get simple point properties and to get point items in an array.
*   All property value get and set functions take an array index as an argument. Here is the point '''get''' function for the example:
* \code
    static StatusInt getDPoint3d0
    (
    DPoint3d&       pt,
    ElementHandleCR    eh,
    UInt32          propID,
    size_t          index
    )
        {
        switch (propID)
            {
            case PROPID_APoint:
                index = 0;
                // VVV fall through VVV
            case PROPID_MyPointArray:
                {
                BeAssert (0 <= index && index <= 1);
                DPoint3d pts[2];
                int npts = 2;
                mdlLinear_extract (pts, &npts, eh.GetElementCP(), eh.GetDgnModel());
                pt = pts[index];
                return SUCCESS;
                }
            }
        BeAssert (false);
        return ERROR;
        }
  \endcode

* <h4>Schema</h4>
* The native IEcPropertyHandler specifies the name for the ECClass that contains the Properties.
* All such ECClasses published by native IEcPropertyHandlers are grouped into an artifical,
* catch-all schema called "ExtendedElementSchema". The classes that make up this artifical schema
* will vary from session to session, depending on which handlers have been asked to publish
* properties in that session.
*
* <h4>Serializability</h4>
* Since the schema is not dependable, you cannot reliably serialize the properties published by
* an IEcPropertyHandler.
*
* @remarks
*   A set function is not required to rewrite the element after modifying a property value
*   in the element. The property handler framework will do this for you.
*
* @bsiclass                                                     BSI             05/2007
* See \code msj\miscdev\mdl\examples\handlerwprops \endcode for a complete example.
+===============+===============+===============+===============+===============+======*/
struct  IEcPropertyHandler
{
    //! Accesses a family of similar property values.
    //! Holds get and set functions to access the values.
    //! Controls how property values relate to displayable strings.
    //! <p>Part of EcPropertyDescriptor and EcArrayValueAccessor
    //!
    //! <h4>Access Function Prototypes</h4>
    //! The basic type of an accessor is determined by the get and set functions that are used to create the accessor.
    //! The basic type pairs are:
    //! \li #T_StringValueGet, #T_StringValueSet
    //! \li #T_IntValueGet, #T_IntValueSet
    //! \li #T_DoubleValueGet, #T_DoubleValueSet
    //! \li #T_BooleanValueGet, #T_BooleanValueSet
    //! \li #T_DPoint3dValueGet, #T_DPoint3dValueSet
    //!
    //! <h4>Units</h4>
    //! For many basic property value types, you can specify units to control how the value is displayed in the GUI. The basic types with units are:
    //! \li T_DoubleValueGet -- #StandardDoubleType
    //! \li T_IntValueGet -- #StandardIntType
    //! \li T_BooleanValueGet -- #StandardBooleanType
    //! \li T_DPoint3dValueGet -- #StandardDPoint3dType
    //!
    //! <h4>Formatting and Parsing</h4>
    //! Units control formatting and parsing for standard types. For some types, you can customize how the property value
    //! is converted to and from a string when it is displayed to the user and when the user enters a new value. The basic types with
    //! custom formatting and parsing are:
    //! \li T_DoubleValueGet -- #T_DoubleValueToString
    //! \li T_IntValueGet -- #T_IntValueToString or #T_GetIntStringPairs
    //! \li T_BooleanValueGet -- #T_BooleanValueToString
    //!
    //! <h4>Property IDs</h4>
    //! Note that each get and set function prototype takes an integer property ID as an argument. This allows the
    //! same get or set function to handle multiple properties of the same basic type.
    //!
    //! <h4>Array Support</h4>
    //! Get or set functions also take an array index as an argument to handle cases where the property value is
    //! actually the member of an array.
    //!
    //! @see EcPropertyDescriptor
    //! @see EcArrayValueAccessor
    struct EcValueAccessor
        {
        //! @cond DONTINCLUDEINDOC
        EcValueAccessor ();
        //! @endcond

        //! Element properties that are stored as an int in the element header
        enum StandardIntType
            {
            ElementColor = 1,   //!< MicroStation element color
            ElementWeight,      //!< MicroStation element weight
            ElementStyle,       //!< MicroStation element style
            ElementLevel,       //!< MicroStation element level
            ElementClass,       //!< MicroStation element class
            SimpleInt,          //!< The value is just an integer
            };

        //! Common uses for double precision values
        enum StandardDoubleType
            {
            DistanceUors = 1,//!< The value represents a length or distance in a design file. The value is displayed and parsed using the current coordinate readout settings. (get/set use UORs)
            DistanceMeters, //!< The value represents a length or distance in a design file. The value is displayed and parsed using the current coordinate readout settings. (get/set use meters)
            AreaUors,       //!< The value represents an area in a design file. The value is displayed and parsed using the current coordinate readout settings. (get/set use UORs)
            AreaMeters,     //!< The value represents an area in a design file. The value is displayed and parsed using the current coordinate readout settings. (get/set use meters)
            VolumeUors,     //!< The value represents a volume in a design file. The value is displayed and parsed using the current coordinate readout settings. (get/set use UORs)
            VolumeMeters,   //!< The value represents a volume in a design file. The value is displayed and parsed using the current coordinate readout settings. (get/set use meters)
            Angle,          //!< The value represents an angle in a design file. The value is displayed and parsed using the current coordinate readout settings. (get/set use radians)
            Direction,      //!< The value represents a direction angle in a design file. The value is displayed and parsed using the current coordinate readout settings. (get/set use radians)
            ElementTransparencyPercent, //!< The value represents element transparency percent, from 0 to 100.
            UnitlessDouble, //!< The value is not scaled in any way
            TimeMillis,     //!< The value represents a time, as milliseconds sine 1/1/70.
            };

        //! Common words for Boolean values
        enum StandardBooleanType
            {
            TrueFalse,      //!< true and false values are displayed as the localized equivalent of the words "True" and "False".
            OnOff           //!< true and false values are displayed as the localized equivalent of the words "On" and "Off".
            };

        //! DPoint3d conversion options
        enum StandardDPoint3dType
            {
            CoordinatesUors = 1, //!< The value holds design file coordinates (get/set use UORs)
            CoordinatesMeters,  //!< The value holds design file coordinates (get/set use meters)
            UnitlessPoint,      //!< The values are not scaled in any way
            };

        //! The signature of a callback that gets the value of a string property
        //! @param[out] propVal   Where to return the value of the property
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
        //! @return non-zero if the property value could not be retrieved
        typedef StatusInt T_StringValueGet (WStringR propVal, ElementHandleCR eh, UInt32 propId, size_t arrayIndex);

        //! The signature of a callback that updates the value of a string property.
        //! @remarks MicroStation will call eh.ReplaceInModel () after this function call returns.
        //! @param[in] propVal  The new value of the property
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
        //! @return non-zero if the property value could not be updated
        typedef StatusInt T_StringValueSet (EditElementHandleR eh, UInt32 propId, size_t arrayIndex, WChar const* propVal);

        //! The signature of a callback that gets the value of an integer property
        //! @return non-zero if the property value could not be retrieved
        //! @param[out] propVal  Where to return the value of the property
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
        //! @return non-zero if the property value could not be retrieved
        typedef StatusInt T_IntValueGet (Int32& propVal, ElementHandleCR eh, UInt32 propId, size_t arrayIndex);

        //! The signature of a callback that updates the value of an integer property.
        //! @remarks MicroStation will call eh.ReplaceInModel () after this function call returns.
        //! @param[in] propVal  The new value of the property
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
        //! @return non-zero if the property value could not be updated
        typedef StatusInt T_IntValueSet (EditElementHandleR eh, UInt32 propId, size_t arrayIndex, Int32 propVal);

        //! The signature of a callback that converts an integer property value to a string that is displayed in the user interface.
        //! @param[out] displayString  Where to return the formatted property value
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        typedef void      T_IntValueToString (WString& displayString, Int32 value, ElementHandleCR eh, UInt32 propId, size_t arrayIndex);

        //! The signature of a callback that converts a string to a property value.
        //! @param[out] parsedValue  Where to return the parsed value
        //! @param[in] enteredString the string entered by the user
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
        //! @return non-zero if the entered string could not be parsed
        typedef StatusInt  T_IntValueFromString (Int32& parsedValue, WChar const* enteredString, ElementHandleCR eh, UInt32 propId, size_t arrayIndex);

        //! The signature of a callback that gets the value of a double precision floating value property
        //! @param[out] propVal   Where to return the value of the property
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
        //! @return non-zero if the property value could not be retrieved
        typedef StatusInt T_DoubleValueGet (double& propVal, ElementHandleCR eh, UInt32 propId, size_t arrayIndex);

        //! The signature of a callback that updates the value of a double precision floating value property.
        //! @remarks MicroStation will call eh.ReplaceInModel () after this function call returns.
        //! @param[in] propVal  The new value of the property
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
        //! @return non-zero if the property value could not be updated
        typedef StatusInt T_DoubleValueSet (EditElementHandleR eh, UInt32 propId, size_t arrayIndex, double propVal);

        //! The signature of a callback that converts a double precision property value to a string that is displayed in the user interface.
        //! @param[out] displayString Where to return the formatted property value
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
        typedef void      T_DoubleValueToString (WString& displayString, double value, ElementHandleCR eh, UInt32 propId, size_t arrayIndex);

        //! The signature of a callback that converts a string to a property value.
        //! @param[out] parsedValue  Where to return the parsed value
        //! @param[in] enteredString the string entered by the user
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
        //! @return non-zero if the entered string could not be parsed
        typedef StatusInt T_DoubleValueFromString (double& parsedValue, WChar const* enteredString, ElementHandleCR eh, UInt32 propId, size_t arrayIndex);

        //! The signature of a callback that gets the value of a Boolean property
        //! @param[out] propVal  Where to return the value of the property
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
        //! @return non-zero if the property value could not be retrieved
        typedef StatusInt T_BooleanValueGet (bool& propVal, ElementHandleCR eh, UInt32 propId, size_t arrayIndex);

        //! The signature of a callback that updates the value of a Boolean property.
        //! @remarks MicroStation will call eh.ReplaceInModel () after this function call returns.
        //! @param[in] propVal  The new value of the property
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
        //! @return non-zero if the property value could not be updated
        typedef StatusInt T_BooleanValueSet (EditElementHandleR eh, UInt32 propId, size_t arrayIndex, bool propVal);

        //! The signature of a callback that converts a Boolean property value to a string that is displayed in the user interface.
        //! @param[out] displayString  Where to return the formatted property value
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
        typedef void      T_BooleanValueToString (WString& displayString, bool value, ElementHandleCR eh, UInt32 propId, size_t arrayIndex);

        //! The signature of a callback that converts a string to a property value.
        //! @param[out] parsedValue  Where to return the parsed value
        //! @param[in] enteredString the string entered by the user
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
        //! @return non-zero if the entered string could not be parsed
        typedef StatusInt   T_BooleanValueFromString (bool& parsedValue, WChar const* enteredString, ElementHandleCR eh, UInt32 propId, size_t arrayIndex);

        typedef bpair<int,WString>      T_IntStringPair;
        typedef bvector<T_IntStringPair> T_IntStringPairs;

        //! The signature of a callback that returns a string list
        //! @param[out] strlist  Where to return the string list
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        typedef void      T_GetIntStringPairs (T_IntStringPairs& strlist, ElementHandleCR eh, UInt32 propId);

        //! The signature of a callback that gets the value of a DPoint3d property
        //! @param[out] propVal   Where to return the value of the property
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
        //! @return non-zero if the property value could not be retrieved
        typedef StatusInt T_DPoint3dValueGet (DPoint3d& propVal, ElementHandleCR eh, UInt32 propId, size_t arrayIndex);

        //! The signature of a callback that updates the value of a DPoint3d property.
        //! @remarks MicroStation will call eh.ReplaceInModel () after this function call returns.
        //! @param[in] propVal  The new value of the property
        //! @param[in] eh       the element that contains properties
        //! @param[in] propId   identifies the requested property
        //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
        //! @return non-zero if the property value could not be updated
        typedef StatusInt T_DPoint3dValueSet (EditElementHandleR eh, UInt32 propId, size_t arrayIndex, DPoint3d const& propVal);

        //! The signature of a callback that gets the value of a Scale property
        //! @param[out] propVal     Where to return the value of the property
        //! @param[in]  eh          the element that contains properties
        //! @param[in]  propId      identifies the requested property
        //! @param[in]  arrayIndex  0 if this property value is not a member of an array; otherwise, the array index
        //! @return non-zero if the property value could not be retrieved
        typedef StatusInt T_NamedScaleValueGet (ScaleDefinition& propVal, ElementHandleCR eh, UInt32 propId, size_t arrayIndex);

        //! The signature of a callback that updates the value of a Scale property.
        //! @remarks MicroStation will call eh.ReplaceInModel () after this function call returns.
        //! @param[in] eh           the element that contains properties
        //! @param[in] propId       identifies the requested property
        //! @param[in] arrayIndex   0 if this property value is not a member of an array; otherwise, the array index
        //! @param[in] propVal      The new value of the property
        //! @return non-zero if the property value could not be updated
        typedef StatusInt T_NamedScaleValueSet (EditElementHandleR eh, UInt32 propId, size_t arrayIndex, ScaleDefinition propVal);

        //! A string value.
        //! @param[in] g        callback to get the value of the property
        //! @param[in] s        [optional] callback to set a new value of the property. If NULL, the property is read-only.
        DGNPLATFORM_EXPORT
        EcValueAccessor (T_StringValueGet* g, T_StringValueSet* s);

        //! An integer value. Uses a non-standard units or no units.
        //! @param[in] g        callback to get the value of the property
        //! @param[in] s        [optional] callback to set a new value of the property. If NULL, the property is read-only.
        //! @param[in] ts       [optional] callback to convert the property value to a string to be displayed. If NULL, %d formatting is done.
        //! @param[in] fs       [optional] callback to convert a string entered by the user to a property value. If NULL, %d parsing is done.
        DGNPLATFORM_EXPORT
        EcValueAccessor (T_IntValueGet* g, T_IntValueSet* s, T_IntValueToString* ts, T_IntValueFromString* fs);

        //! An integer value. Uses standard units.
        //! @param[in] g        callback to get the value of the property
        //! @param[in] s        [optional] callback to set a new value of the property. If NULL, the property is read-only.
        //! @param[in] st       the int value represents an element header property
        DGNPLATFORM_EXPORT
        EcValueAccessor (T_IntValueGet* g, T_IntValueSet* s, StandardIntType st = SimpleInt);

        //! A list of <integer,string> pairs. The strings are presented to the user, and the user may only
        //! select values from the list. The internal value is the associated integer.
        //! @param[in] g        callback to get the value of the property
        //! @param[in] s        [optional] callback to set a new value of the property. If NULL, the property is read-only.
        //! @param[in] pairs    callback to get the list of all valid <integer,string> pairs
        DGNPLATFORM_EXPORT
        EcValueAccessor (T_IntValueGet* g, T_IntValueSet* s, T_GetIntStringPairs* pairs);

        //! A double precision floating point value. Uses non-standard units or no units.
        //! @param[in] g        callback to get the value of the property
        //! @param[in] s        [optional] callback to set a new value of the property. If NULL, the property is read-only.
        //! @param[in] ts       [optional] callback to convert the property value to a string to be displayed. If NULL, %0.17G formatting is done.
        //! @param[in] fs       [optional] callback to convert a string entered by the user to a property value. If NULL, %G parsing is done.
        DGNPLATFORM_EXPORT
        EcValueAccessor (T_DoubleValueGet* g, T_DoubleValueSet* s, T_DoubleValueToString* ts, T_DoubleValueFromString* fs);

        //! A double precision floating point value. Uses standard units.
        //! @param[in] g        callback to get the value of the property
        //! @param[in] s        callback to set a new value of the property. If NULL, the property is read-only.
        //! @param[in] st       What the value represents. This controls formatting and parsing.
        DGNPLATFORM_EXPORT
        EcValueAccessor (T_DoubleValueGet* g, T_DoubleValueSet* s, StandardDoubleType st = UnitlessDouble);

        //! A Boolean value. Uses non-standard values.
        //! @param[in] g        callback to get the value of the property
        //! @param[in] s        [optional] callback to set a new value of the property. If NULL, the property is read-only.
        //! @param[in] ts       callback to convert the property value to a string to be displayed.
        DGNPLATFORM_EXPORT
        EcValueAccessor (T_BooleanValueGet* g, T_BooleanValueSet* s, T_BooleanValueToString* ts);

        //! A Boolean value. Uses standard values.
        //! @param[in] g        callback to get the value of the property
        //! @param[in] s        [optional] callback to set a new value of the property. If NULL, the property is read-only.
        DGNPLATFORM_EXPORT
        EcValueAccessor (T_BooleanValueGet* g, T_BooleanValueSet* s, StandardBooleanType = TrueFalse);

        //! A DPoint3d value.
        //! @param[in] g        callback to get the value of the property
        //! @param[in] s        [optional] callback to set a new value of the property. If NULL, the property is read-only.
        //! @param[in] st       What the value represents. This controls formatting and parsing.
        DGNPLATFORM_EXPORT
        EcValueAccessor (T_DPoint3dValueGet* g, T_DPoint3dValueSet* s, StandardDPoint3dType st = CoordinatesUors);

        //! A Scale object value
        //! @param[in] g        callback to get the value of the property
        //! @param[in] s        callback to set the new value of the property.
        DGNPLATFORM_EXPORT
        EcValueAccessor (T_NamedScaleValueGet* g, T_NamedScaleValueSet* s);

        //! @cond DONTINCLUDEINDOC
        enum ValueType { VALUETYPE_Uninitialized, VALUETYPE_String, VALUETYPE_StringFromList, VALUETYPE_Int, VALUETYPE_StringList,
                         VALUETYPE_Double, VALUETYPE_Boolean, VALUETYPE_DPoint3d, VALUETYPE_Array, VALUETYPE_Scale};

        ValueType           m_type;
        int                 m_subType;
        void*               m_get;
        void*               m_set;
        void*               m_toString;
        void*               m_fromString;
        T_GetIntStringPairs* m_getIntStringPairs;

        private:
        void Clear ();
        //! @endcond
        };

    //! Defines how an array of values is accessed. Has a function to get the array count. Holds the accessor for the item values.
    //! Part of EcPropertyDescriptor. Holds EcValueAccessor.
    //! @see EcPropertyDescriptor
    //! @see EcValueAccessor
    struct EcArrayValueAccessor : EcValueAccessor
        {
        typedef StatusInt T_ArrayCountGet (int& count, ElementHandleCR eh, UInt32 propId);

        EcValueAccessor     m_underlying;
        T_ArrayCountGet*    m_getCount;

        DGNPLATFORM_EXPORT
        EcArrayValueAccessor (T_ArrayCountGet* g, EcValueAccessor a) : m_underlying(a), m_getCount(g) {;}
        };

    //! Defines a property.
    //! Part of EcPropertyCategory.
    //! Holds a:
    //! \li EcValueAccessor, which is used to access and control formatting and parsing of the value of the property.
    //! \li integer ID, which is passed to the accessor's get and set functions to identify the property.
    //! \li string ID, which identifies the property to fields.
    //! \li display name, which is displayed in the GUI
    //! \li sort priority, which controls the order in which properties are displayed within a category.
    //!
    //! Stored in an EcPropertyCategory
    struct EcPropertyDescriptor
        {
        //! @cond DONTINCLUDEINDOC
        UInt32              m_propID;
        WString    m_name;
        WString    m_displayName;
        UInt32              m_priority;
        EcValueAccessor     m_value;
        IRefCountedEcPropertyHandler* m_struct;     // member genearator for struct (m_value will be nop)
        EcValueAccessor     m_underlying;           // array item type (m_value will have the EcArrayValueAccessor::T_ArrayCountGet function)
        //! @endcond

        //! Standard sort priority values
        enum {PriorityVeryHigh = 400000, PriorityHigh = 300000, PriorityMedium = 200000, PriorityLow = 100000, PriorityVeryLow = 0};

        //! @cond DONTINCLUDEINDOC
        DGNPLATFORM_EXPORT
        EcPropertyDescriptor ();
        //! @endcond

        //! Define a property, including how it is accessed and how it is displayed.
        //! @param[in] accessor how the value is accessed and how its value is displayed
        //! @param[in] propId the numerical ID of the property. This is passed to the get and set callbacks in order to identify the property
        //! @param[in] name   the string ID of the property. This is used by fields to identify the property. This value should \em not be translated or changed between versions.
        //! This ID must be unique within the set of properties associated with the EcPropertiesClassName returned by the handler.
        //! @param[in] displayName the name of the property that is to be displayed to the user. This value should be a translated string and may change between versions.
        //! @param[in] pri    [optional] The sort priority of the property. See PriorityVeryHigh, etc.
        DGNPLATFORM_EXPORT
        EcPropertyDescriptor (EcValueAccessor const& accessor, UInt32 propId, WString const& name, WString const& displayName, UInt32 pri = PriorityMedium);

        //! Define an array-valued property, including how it is accessed and how it is displayed. An array is a sequence of properties that are accessed by position.
        //! @param[in] accessor how the array is accessed
        //! @param[in] propId the numerical ID of the property. This is passed to the get and set callbacks in order to identify the property
        //! @param[in] name   the string ID of the property. This is used by fields to identify the property. This value should \em not be translated or changed between versions.
        //! This ID must be unique within the set of properties associated with the EcPropertiesClassName returned by the handler.
        //! @param[in] displayName the name of the property that is to be displayed to the user. This value should be a translated string and may change between versions.
        //! @param[in] pri    [optional] The sort priority of the property. See PriorityVeryHigh, etc.
        DGNPLATFORM_EXPORT
        EcPropertyDescriptor (EcArrayValueAccessor const& accessor, UInt32 propId, WString const& name, WString const& displayName, UInt32 pri = PriorityMedium);

        //! Define an struct-valued property, including how it is accessed and how it is displayed. A struct is a group of properties with a name.
        //! @param[in] s        the group of properties
        //! @param[in] name     the string ID of the property. This is used by fields to identify the property. This value should \em not be translated or changed between versions.
        //! This ID must be unique within the set of properties associated with the EcPropertiesClassName returned by the handler.
        //! @param[in] displayName the name of the property that is to be displayed to the user. This value should be a translated string and may change between versions.
        //! @param[in] pri      [optional] The sort priority of the property. See PriorityVeryHigh, etc.
        DGNPLATFORM_EXPORT
        EcPropertyDescriptor (IRefCountedEcPropertyHandler* s, WString const& name, WString const& displayName, UInt32 pri = PriorityMedium);

        }; // EcPropertyDescriptor

    //! Defines a category of EC properties. Holds EcPropertyDescriptor.
    struct EcPropertyCategory : bvector<EcPropertyDescriptor>
        {
        //! Standard sort priority values
        enum {PriorityVeryHigh = 400000, PriorityHigh = 300000, PriorityMedium = 200000, PriorityLow = 100000, PriorityVeryLow = 0};

        //! Identifies existing categories that may be used.
        enum StandardId
            {
            NonStandard         =   -1,
            Miscellaneous       =   0,
            General             =   1,
            Extended            =   1 << 1,
            RawData             =   1 << 2,
            Geometry            =   1 << 3,
            Groups              =   1 << 4,
            Material            =   1 << 5,
            Relationships       =   1 << 6,
            };

        //! @cond DONTINCLUDEINDOC
        StandardId          m_standardId;
        WString    m_name;
        WString    m_description;
        UInt32              m_priority;
        //! @endcond

        //! Get a standard category
        DGNPLATFORM_EXPORT
        EcPropertyCategory (StandardId);

        //! Define a custom category
        //! @param[in] id internal identifier for the category. Do not translate or change between versions.
        //! @param[in] desc the displayed name of the category. This should be a translated string. It may change between versions.
        //! @param[in] pri    [optional] The sort priority of the category. See PriorityVeryHigh, etc.
        DGNPLATFORM_EXPORT
        EcPropertyCategory (WString const& id, WString const& desc, UInt32 pri = PriorityMedium);
        };

    typedef bvector<EcPropertyCategory> T_EcCategories;

    //! Return an internal identifer for the EC Class that will be created to hold the properties
    //! @param[in] eh the element that is being queried for properties
    //! @return An identifier that is globally unique among all handlers. The fully qualified class name is a good choice.
    DGNPLATFORM_EXPORT
    WString   GetEcPropertiesClassName (ElementHandleCR eh);

    /** Return the EC properties for the specified element.
    @see IEcPropertyHandler::EcValueAccessor
    @see IEcPropertyHandler::EcPropertyDescriptor
    @param[in] c  Where to return the properties
    @param[in] eh the element that is being queried for properties
    @return non-zero if there was a problem querying the properties of this element.
    @remarks Example
\code
StatusInt LineWithBarbellEnds::_GetEcProperties
(
IEcPropertyHandler::T_EcCategories& cats,
ElementHandleCR                      eh
)
    {
    // The handler controls how the value of a property is accessed by supplying get and set functions.
    // The handler may also specify the units of the property value. Standard units are supported by the property grid GUI.
    EcValueAccessor getDoubleUors (GetDouble, NULL, EcValueAccessor::DistanceUors);
    EcValueAccessor getBoolean (GetBoolean, NULL);

    // The handler organizes properties into categories.
    // This is a custom category.
    EcPropertyCategory barbells (L"Barbells", getCatDesc(CATEGORY_Barbells));  // do not translate

    //  A property is defined by supplying its access functions, an internal ID, and a display name.
    barbells.push_back (EcPropertyDescriptor (getBoolean, PROPID_HasCustomBsize, L"HasCustomBsize", getPropName(PROPID_HasCustomBsize)));
    ... more "barbells" properties ...
    cats.push_back (barbells);

    // This is a standard category.
    EcPropertyCategory geometry (IEcPropertyHandler::EcPropertyCategory::Geometry);

    geometry.push_back (EcPropertyDescriptor (getDoubleUors, PROPID_Length, L"Length", getPropName(PROPID_Length)));
    ... more "geometry" properties ...

    cats.push_back (geometry);
    return SUCCESS;
    }
\endcode
    */
    DGNPLATFORM_EXPORT
    StatusInt GetEcProperties (T_EcCategories& c, ElementHandleCR eh);

    //! Check if a given property's value is always NULL because it does not apply to specific sub-type
    //! @param[in] enabler      name of property enabler
    //! @param[in] className    name of class that owns the property
    //! @param[in] propName     name of property
    //! @return \a true if property should be hidden.
    DGNPLATFORM_EXPORT
    bool    IsNullProperty (WCharCP enabler, WCharCP className, WCharCP propName);

    //! Check if a given property's value is NULL.
    //! @param[in] eh           the element that contains properties
    //! @param[in] propId       identifies the requested property
    //! @param[in] arrayIndex   0 if this property value is not a member of an array; otherwise, the array index
    //! @remarks If the value is NULL, then it will not be shown in the property grid.
    DGNPLATFORM_EXPORT
    IsNullReturnType    IsNullProperty (ElementHandleCR eh, UInt32 propId, size_t arrayIndex);

    //! Query if the specified property of the specified element should be presented as read-only.
    //! @param[in] eh       the element that contains properties
    //! @param[in] propId   -1 indicates the entire element; otherwise, identifies the requested property
    //! @param[in] arrayIndex 0 if this property value is not a member of an array; otherwise, the array index
    //! @return \a true if the property is read-only on this element.
    //! @remarks The default implementation is to return true if the element is locked.
    DGNPLATFORM_EXPORT
    bool      IsPropertyReadOnly (ElementHandleCR eh, UInt32 propId, size_t arrayIndex);

protected:
    virtual WString   _GetEcPropertiesClassName (ElementHandleCR) = 0;
    virtual StatusInt _GetEcProperties (T_EcCategories&, ElementHandleCR) = 0;
    DGNPLATFORM_EXPORT
    virtual bool      _IsPropertyReadOnly (ElementHandleCR eh, UInt32 propId, size_t arrayIndex); // returns true if element is locked
    DGNPLATFORM_EXPORT
    virtual bool      _IsNullProperty (WCharCP enabler, WCharCP className, WCharCP propName);
    DGNPLATFORM_EXPORT
    virtual IsNullReturnType    _IsNullProperty (ElementHandleCR eh, UInt32 propId, size_t arrayIndex);
};

struct IRefCountedEcPropertyHandler : IEcPropertyHandler, IRefCounted
    {
    };

/// @endGroup

#if defined (_MSC_VER)
    #pragma warning(pop)
#endif // defined (_MSC_VER)

END_BENTLEY_DGNPLATFORM_NAMESPACE
