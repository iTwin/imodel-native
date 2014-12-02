/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/DgnGraphicsElement.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Bentley/RefCounted.h>
//__PUBLISH_SECTION_END__
#include <DgnPlatform/DgnCore/XGraphics.h>
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/**  @addtogroup DgnGraphicsGroup

 DgnGraphics is a simplified representation of an element intended to let the user focus on
 working with the geometry in an element without worrying about element specifics.

 DgnGraphics contain one or more geometric primitives and the associated symbology for each.
 A DgnGraphics is created from a DgnModel using either DgnModel::CreatePhysicalGraphics,
 DgnModel::CreateDrawingGraphics, DgnModel::ReadPhysicalGraphics or DgnModel::ReadDrawingGraphics.
 Changes to a DgnGraphics are not persisted to an underlying element (and thus the DgnProject)
 until DgnGraphics::Save is called.

 In addition to being a more user-friendly way to create elements with simple geometry than
 methods like ConeHandler::CreateConeElement, DgnGraphics is also the preferred way to create
 elements with multiple types of geometry (as opposed to cells).

 All examples below use PhysicalGraphics, but the process for working with DrawingGraphics
 is the same. The only distinction between the two is that DrawingGraphics is intended for
 use in DrawingModels (models with DgnModelType::Drawing) and supports fewer geometric primitives.

 <h2>Creating a new element with DgnGraphics</h2>
    \code
    // Assuming an existing dgnModel with DgnModelType::Physical, create the DgnGraphics.
    PhysicalGraphicsPtr graphics = dgnModel.CreatePhysicalGraphics();

    // Create a new mesh with a single triangle offset from the model's origin.
    PolyfaceHeaderPtr mesh = PolyfaceHeader::CreateVariableSizeIndexed();
    DPoint3d polygon[3];
    polygon[0].Init (20000.0, 20000.0, 0.0);
    polygon[1].Init (30000.0, 20000.0, 0.0);
    polygon[2].Init (20000.0, 30000.0, 0.0);
    mesh->AddPolygon (polygon, 3);

    // Add the mesh to the DgnGraphics.
    graphics->AddPolyface (*mesh);

    // Create a new sphere at the model's origin with a radius of 5000 UORs.
    DgnSphereDetail detail (DPoint3d::FromZero(), 5000.0);

    // Add the sphere to the DgnGraphics.
    graphics->AddSolidPrimitive (*ISolidPrimitive::CreateDgnSphere (detail));

    // Save the entries in this DgnGraphics as a new element and return its ElementId.
    // If this function is not called, any changes to the DgnGraphics are discarded.
    ElementId graphicsId = graphics->Save();
    \endcode

 <h2>Reading an existing element with DgnGraphics</h2>
 The sample code below assumes that the code in "Creating a new element with DgnGraphics" has
 just executed.
    \code
    // Load the element that was just created to inspect its properties.
    PhysicalGraphicsPtr loadedGraphics = dgnModel.ReadPhysicalGraphics (graphicsId);

    // Iterate over the DgnGraphics::Entry objects in the loaded DgnGraphics.
    for (DgnGraphics::EntryPtr& loadedEntry : *loadedGraphics)
        {
        // Query the type of each entry to determine how to cast it.
        switch (loadedEntry->GetType())
            {
            case DgnGraphics::Entry::Type::Polyface:
                {
                // We now have a live pointer to the mesh in this DgnGraphics and can
                // perform any PolyfaceHeader operations on it. Modifications will not
                // be persisted to the element until DgnGraphics::Save is called.
                PolyfaceHeaderP loadedMesh = loadedEntry->GetAsPolyfaceHeaderP();
                break;
                }
            case DgnGraphics::Entry::Type::SolidPrimitive:
                {
                ISolidPrimitiveP loadedSphere = loadEntry->GetAsISolidPrimitiveP();
                DgnSphereDetail loadedSphereDetail;
                loadedSphereDetail->TryGetDgnSphereDetail (loadedSphereDetail);
                break;
                }
            }
        }
    \endcode
 Although this example reads an element that was created with DgnGraphics, all graphical elements
 that were not created with DgnGraphics should also be readable.

 <h2>Modifying an existing element with DgnGraphics</h2>
 The sample code below assumes that the code in "Reading an existing element with DgnGraphics" has
 just executed.
    \code
    // Search the DgnGraphics:Entry objects in the loaded DgnGraphics for the mesh that
    // was previously added.
    for (DgnGraphics::iterator iter = loadedGraphics->begin(); iter != loadedGraphics->end(); )
        {
        // When the mesh is found, remove it and set the iterator to the next item.
        if (DgnGraphics::Entry::Type::Polyface == (*iter)->GetType())
            iter = loadedGraphics->Erase (iter);
        else
            ++iter;
        }

    // As before, the changes that were made to the DgnGraphics aren't persisted to its
    // underlying element until DgnGraphics::Save is called.
    graphicsId = loadedGraphics->Save();
    \endcode

 <h2>DgnGraphics and Symbology</h2>
 In the previous examples, DgnGraphics::Entry objects are added with the default DgnGraphics
 symbology, which lets element color, line weight and line style be defined by the level that
 the DgnGraphics is created on. (see \ref DgnLevels::Appearance).

 Symbology can also be specified when adding DgnGraphics::Entry objects, as illustrated in the
 example below.
    \code
    PhysicalGraphicsPtr graphics = dgnModel.CreatePhysicalGraphics();

    // Get the default symbology used for DgnGraphics::Entry so the fields this example
    // doesn't modify are set up properly.
    Symbology symb = DgnGraphics::GetDefaultSymbology();

    // Create an RGB color definition for green.
    RgbColorDef rgb = {0, 255, 0};

    // This method will determine the appropriate color index for a given RgbColorDef.
    // It is essentially a convenience wrapper around a few DgnColors functions.
    symb.color = graphics.GetSymbologyColorFromRgb (rgb);

    // Create a cube at the model's origin.
    DgnBoxDetail boxDetail = DgnBoxDetail::InitFromCenterAndSize (DPoint3d::FromZero(),
                                                                  DPoint3d::From (10000.0, 10000.0, 10000.0),
                                                                  true);

    // Add the cube to this DgnGraphics with the symbology we've defined.
    graphics->AddSolidPrimitive (*ISolidPrimitive::CreateDgnBox (boxDetail), symb);

    // Save the DgnGraphics to its DgnModel.
    graphics->Save();
    \endcode

 The Symbology on an existing DgnGraphics::Entry can also be modified in the same manner as its
 geometric properties.
*/

//=======================================================================================
//! A DgnGraphics is a simplified representation of an element used to read, create and
//! manipulate commonly accessed properties.
//!
//! See the \ref DgnGraphicsGroup module for more information and examples of common workflows.
//!
//! @ingroup DgnGraphicsGroup
// @bsiclass                                                    MattGooding     08/13
//=======================================================================================
struct DgnGraphics : RefCountedBase
{
    //=======================================================================================
    //! A DgnGraphics::Entry represents one unit of geometry in a DgnGraphics and its
    //! associated Symbology.
    //!
    //! @ingroup DgnGraphicsGroup
    // @bsiclass                                                    MattGooding     08/13
    //=======================================================================================
    struct Entry : RefCountedBase
    {
//__PUBLISH_SECTION_END__
        friend struct DgnGraphics;
        friend struct PhysicalGraphics;
        friend struct DrawingGraphics;
//__PUBLISH_SECTION_START__
        enum class Type
            {
            None                = 0,    //!< Entry is invalid
            CurvePrimitive      = 1,    //!< Entry is an ICurvePrimitive
            CurveVector         = 2,    //!< Entry is an CurveVector
            Polyface            = 3,    //!< Entry is an PolyfaceHeader
            MSBsplineSurface    = 5,    //!< Entry is an MSBsplineSurface
            SolidPrimitive      = 6,    //!< Entry is an ISolidPrimitive
            Text                = 7,    //!< Entry is an TextString
            };

//__PUBLISH_SECTION_END__
    private:
        IGeometryPtr                m_geometry;
        TextStringPtr               m_text;
        Symbology                   m_symbology;

        Entry (IGeometryPtr& geometry, SymbologyCR symbology);
        Entry (TextStringP text, SymbologyCR symbology);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
    public:
        //! Returns the DgnGraphics::Entry::Type for this entry.
        DGNPLATFORM_EXPORT Type                 GetType() const;

        //! Returns the TextString contained by this entry.  If this entry is not a TextString, returns NULL.
        //! @remarks This is a live pointer.
        DGNPLATFORM_EXPORT TextStringP          GetAsTextStringP();
        //! Returns the TextString contained by this entry.  If this entry is not a TextString, returns NULL.
        //! @remarks This is a live pointer.
        DGNPLATFORM_EXPORT TextStringCP         GetAsTextStringCP() const;

        //! Returns the ICurvePrimitive contained by this entry.  If this entry is not a ICurvePrimitive, returns NULL.
        //! @remarks This is a live pointer.
        DGNPLATFORM_EXPORT ICurvePrimitiveP     GetAsICurvePrimitiveP();
        //! Returns the ICurvePrimitive contained by this entry.  If this entry is not a ICurvePrimitive, returns NULL.
        //! @remarks This is a live pointer.
        DGNPLATFORM_EXPORT ICurvePrimitiveCP    GetAsICurvePrimitiveCP() const;

        //! Returns the CurveVector contained by this entry.  If this entry is not a CurveVector, returns NULL.
        //! @remarks This is a live pointer.
        DGNPLATFORM_EXPORT CurveVectorP         GetAsCurveVectorP();
        //! Returns the CurveVector contained by this entry.  If this entry is not a CurveVector, returns NULL.
        //! @remarks This is a live pointer.
        DGNPLATFORM_EXPORT CurveVectorCP        GetAsCurveVectorCP() const;

        //! Returns the PolyfaceHeader contained by this entry.  If this entry is not a PolyfaceHeader, returns NULL.
        //! @remarks This is a live pointer.
        DGNPLATFORM_EXPORT PolyfaceHeaderP      GetAsPolyfaceHeaderP();
        //! Returns the PolyfaceHeader contained by this entry.  If this entry is not a PolyfaceHeader, returns NULL.
        //! @remarks This is a live pointer.
        DGNPLATFORM_EXPORT PolyfaceHeaderCP     GetAsPolyfaceHeaderCP() const;

        //! Returns the MSBsplineSurface contained by this entry.  If this entry is not a MSBsplineSurface, returns NULL.
        //! @remarks This is a live pointer.
        DGNPLATFORM_EXPORT MSBsplineSurfaceP    GetAsMSBsplineSurfaceP();
        //! Returns the MSBsplineSurface contained by this entry.  If this entry is not a MSBsplineSurface, returns NULL.
        //! @remarks This is a live pointer.
        DGNPLATFORM_EXPORT MSBsplineSurfaceCP   GetAsMSBsplineSurfaceCP() const;

        //! Returns the ISolidPrimitive contained by this entry.  If this entry is not a ISolidPrimitive, returns NULL.
        //! @remarks This is a live pointer.
        DGNPLATFORM_EXPORT ISolidPrimitiveP     GetAsISolidPrimitiveP();
        //! Returns the ISolidPrimitive contained by this entry.  If this entry is not a ISolidPrimitive, returns NULL.
        //! @remarks This is a live pointer.
        DGNPLATFORM_EXPORT ISolidPrimitiveCP    GetAsISolidPrimitiveCP() const;

        //! Returns the Symbology used for drawing this entry.
        DGNPLATFORM_EXPORT SymbologyR           GetSymbologyR();
        //! Returns the Symbology used for drawing this entry.
        DGNPLATFORM_EXPORT SymbologyCR          GetSymbology() const;

        //! Apply transform to all DgnGraphics entries.
        DGNPLATFORM_EXPORT bool TransformInPlace (TransformCR transform);
    };

    typedef RefCountedPtr <Entry> EntryPtr; //!< a smart pointer to a DgnGraphics::Entry.

//__PUBLISH_SECTION_END__
protected:
    bvector <EntryPtr>      m_entries;
    EditElementHandle       m_element;
    ElementRefP             m_sourceElement;
    bool                    m_isFinished; //!< true if Finish was called

    void Load (ElementHandleCR element);
    void Load (ElementRefP element);
    void Init (DgnModelR model, LevelId level);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
private:
    DgnGraphicsR operator= (DgnGraphicsCR from);
    explicit DgnGraphics (DgnGraphicsCR from);

//__PUBLISH_SECTION_END__
    void DrawSymbology (ViewContextR context, SymbologyCR symb);
    void DrawEntry (IDrawGeomR drawGeom, Entry const& entry);
    BentleyStatus SaveEntriesToElement() {return SaveEntries(GetElementHandleR());}
//__PUBLISH_SECTION_START__

protected:
    //! @private
    DgnGraphics();

public:
    DGNPLATFORM_EXPORT BentleyStatus SaveEntries(EditElementHandleR);
    
    //! Indicate that no more graphics will be added so the element can be created and its range can be calculated.
    //! @note DgnGraphics::Save calls DgnGraphics::Finish
    DGNPLATFORM_EXPORT BentleyStatus Finish();

    //! Save this DgnGraphics as an element in the DgnModel it was created from. If this DgnGraphics was read from
    //! an existing element, this function will attempt to update that element. Otherwise, a new element will be
    //! created.
    //! @return The ElementId for the newly created or updated element. ElementId::IsValid will be false if the
    //! element was not successfully added.
    //! @remarks If this function is not called, any changes made to the DgnGraphics are discarded when it is
    //! freed.
    //! @remarks This method will fail if a DgnGraphics doesn't contain any valid entries.
    //! @note This method calls DgnGraphics::Finish
    DGNPLATFORM_EXPORT ElementId Save();

    //! Save this DgnGraphics as potentially multiple elements in the DgnModel it was created from. If the original
    //! DgnGraphics would be be expensive to process, this method will attempt to sub-divide it into multiple elements
    //! that can be drawn individually for better performance. The resulting elements will be grouped into an assembly.
    //! If the DgnGraphics does not need to be optimized, this method will still save it but will not create an assembly.
    //! See \ref AssemblyIterator for more information on how to work with assemblies.
    //! @param[out] elementIds The ids of the elements that were created. The first ID is the leader of the assembly and will hold any EC properties assigned to the original DgnGraphics.
    //! @remarks Even if this DgnGraphics was loaded from an existing element, this method will not update the original element. It will always create new elements. The caller must clean up the original element if so desired.
    //! @return SUCCESS if the graphics were saved, ERROR otherwise
    DGNPLATFORM_EXPORT BentleyStatus OptimizeAndSave (bvector <ElementId>& elementIds);

    typedef bvector <EntryPtr>::iterator iterator; //!< a writeable iterator to the entries in a DgnGraphics.
    typedef bvector <EntryPtr>::const_iterator const_iterator; //!< a read-only iterator to the entries in a DgnGraphics.

    DGNPLATFORM_EXPORT const_iterator begin() const; //!< const_iterator for the first DgnGraphics::Entry in this DgnGraphics.
    DGNPLATFORM_EXPORT iterator begin(); //!< iterator for the first DgnGraphics::Entry in this DgnGraphics.
    DGNPLATFORM_EXPORT const_iterator end() const; //!< const_iterator for the end of the DgnGraphics::Entry objects in this DgnGraphics.
    DGNPLATFORM_EXPORT iterator end(); //!< iterator for the end of the DgnGraphics::Entry objects in this DgnGraphics.

    //! Erases a DgnGraphics::Entry from this DgnGraphics.
    //! @param[in] position A DgnGraphics::Entry to remove from this DgnGraphics.
    //! @return An iterator pointing to the new location of the element that followed the last element erased by the
    //! function call. This is the container end if the operation erased the last element in the sequence.
    DGNPLATFORM_EXPORT iterator Erase (iterator position);

    //! Returns the number of DgnGraphics::Entry objects in this DgnGraphics.
    DGNPLATFORM_EXPORT size_t GetSize() const;

    //! Returns true if this DgnGraphics contains no DgnGraphics::Entry objects, false otherwise.
    DGNPLATFORM_EXPORT bool IsEmpty() const;

    //! Returns the EditElementHandle for this element. Can be used to access element properties not exposed by DgnGraphics.
    DGNPLATFORM_EXPORT EditElementHandleR GetElementHandleR();

    //! Add a ICurvePrimitive entry to element with the provided Symbology.
    //! @param[in] curve The ICurvePrimitive to be copied and added to this DgnGraphics.
    //! @param[in] symb The Symbology to draw this ICurvePrimitive with.
    //! @return SUCCESS if the ICurvePrimitive was successfully added, ERROR otherwise.
    //! @remarks Because CurveVector can contain CurvePrimitive objects, DgnGraphics read from an existing element
    //! will have CurveVector objects instead of CurvePrimitive objects.
    DGNPLATFORM_EXPORT BentleyStatus AddCurvePrimitive (ICurvePrimitiveCR curve, SymbologyCR symb);
    //! Add a ICurvePrimitive entry to element with Symbology equal to the one returned by DgnGraphics::GetDefaultSymbology.
    //! @param[in] curve The ICurvePrimitive to be copied and added to this DgnGraphics.
    //! @return SUCCESS if the ICurvePrimitive was successfully added, ERROR otherwise.
    //! @remarks Because CurveVector can contain CurvePrimitive objects, DgnGraphics read from an existing element
    //! will have CurveVector objects instead of CurvePrimitive objects.
    DGNPLATFORM_EXPORT BentleyStatus AddCurvePrimitive (ICurvePrimitiveCR curve);

    //! Add a CurveVector entry to element with the provided Symbology.
    //! @param[in] curves The CurveVector to be copied and added to this DgnGraphics.
    //! @param[in] symb The Symbology to draw this CurveVector with.
    //! @return SUCCESS if the CurveVector was successfully added, ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus AddCurveVector (CurveVectorCR curves, SymbologyCR symb);
    //! Add a CurveVector entry to element with Symbology equal to the one returned by DgnGraphics::GetDefaultSymbology.
    //! @param[in] curves The CurveVector to be copied and added to this DgnGraphics.
    //! @return SUCCESS if the CurveVector was successfully added, ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus AddCurveVector (CurveVectorCR curves);

    //! Add a TextString entry to element with the provided Symbology.
    //! @param[in] text The TextString to be copied and added to this DgnGraphics.
    //! @param[in] symb The Symbology to draw this TextString with.
    //! @return SUCCESS if the TextString was successfully added, ERROR otherwise.
    //! @remarks TextStrings ignore the style parameter of Symbology.
    DGNPLATFORM_EXPORT BentleyStatus AddTextString (TextStringCR text, SymbologyCR symb);
    //! Add a TextString entry to element with Symbology equal to the one returned by DgnGraphics::GetDefaultSymbology.
    //! @param[in] text The TextString to be copied and added to this DgnGraphics.
    //! @return SUCCESS if the TextString was successfully added, ERROR otherwise.
    //! @remarks TextStrings ignore the style parameter of Symbology.
    DGNPLATFORM_EXPORT BentleyStatus AddTextString (TextStringCR text);

    //! @return The default Symbology used for a DgnGraphics entry, with color, weight and style initialized to "by-level" values.
    DGNPLATFORM_EXPORT static SymbologyCR GetDefaultSymbology();

    //! Searches the color table in the DgnProject of this DgnGraphics for the given RgbColorDef. If an entry
    //! does not already exist, it is added. This index is suitable for use in Symbology entries in this DgnGraphics.
    //! @param[in] rgb RgbColorDef with 0-255 values for red/green/blue.
    //! @return A color index for the given RgbColorDef in the DgnProject for this DgnGraphics.
    DGNPLATFORM_EXPORT UInt32 GetSymbologyColorFromRgb (RgbColorDefCR rgb);

    //! Apply transform to all DgnGraphics entries.
    DGNPLATFORM_EXPORT void TransformInPlace (TransformCR transform);
};

//=======================================================================================
//! A PhysicalGraphics is a DgnGraphics contained in a PhysicalModel. PhysicalGraphics allow
//! more types of geometry than DrawingGraphics.
//! PhysicalGraphics not be instantiated directly - use \ref DgnModel::CreatePhysicalGraphics
//! to create a new element or \ref DgnModel::ReadPhysicalGraphics to read an existing one.
//!
//! @ingroup DgnGraphicsGroup
// @bsiclass
//=======================================================================================
struct PhysicalGraphics : public DgnGraphics
{
//__PUBLISH_SECTION_END__
    friend struct PhysicalModel;
//__PUBLISH_SECTION_START__

private:
    PhysicalGraphicsR operator= (PhysicalGraphicsCR from);
    explicit PhysicalGraphics (PhysicalGraphicsCR from);
    PhysicalGraphics(){}

public:
    //! Add a ISolidPrimitive entry to element with the provided Symbology.
    //! @param[in] solid The ISolidPrimitive to be copied and added to this DgnGraphics.
    //! @param[in] symb The Symbology to draw this ISolidPrimitive with.
    //! @return SUCCESS if the ISolidPrimitive was successfully added, ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus AddSolidPrimitive (ISolidPrimitiveCR solid, SymbologyCR symb);
    //! Add a ISolidPrimitive entry to element with Symbology equal to the one returned by DgnGraphics::GetDefaultSymbology.
    //! @param[in] solid The ISolidPrimitive to be copied and added to this DgnGraphics.
    //! @return SUCCESS if the ISolidPrimitive was successfully added, ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus AddSolidPrimitive (ISolidPrimitiveCR solid);

    //! Add a PolyfaceQuery entry to element with the provided Symbology.
    //! @param[in] polyface The PolyfaceQuery to be copied and added to this DgnGraphics.
    //! @param[in] symb The Symbology to draw this PolyfaceQuery with.
    //! @return SUCCESS if the PolyfaceQuery was successfully added, ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus AddPolyface (PolyfaceQueryCR polyface, SymbologyCR symb);
    //! Add a PolyfaceQuery entry to element with Symbology equal to the one returned by DgnGraphics::GetDefaultSymbology.
    //! @param[in] polyface The PolyfaceQuery to be copied and added to this DgnGraphics.
    //! @return SUCCESS if the PolyfaceQuery was successfully added, ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus AddPolyface (PolyfaceQueryCR polyface);

    //! Add a MSBsplineSurface entry to element with the provided Symbology.
    //! @param[in] surface The MSBsplineSurface to be copied and added to this DgnGraphics.
    //! @param[in] symb The Symbology to draw this MSBsplineSurface with.
    //! @return SUCCESS if the MSBsplineSurface was successfully added, ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus AddMSBsplineSurface (MSBsplineSurfaceCR surface, SymbologyCR symb);
    //! Add a MSBsplineSurface entry to element with Symbology equal to the one returned by DgnGraphics::GetDefaultSymbology.
    //! @param[in] surface The MSBsplineSurface to be copied and added to this DgnGraphics.
    //! @return SUCCESS if the MSBsplineSurface was successfully added, ERROR otherwise.
    DGNPLATFORM_EXPORT BentleyStatus AddMSBsplineSurface (MSBsplineSurfaceCR surface);
};

//=======================================================================================
//! A DrawingGraphics is a DgnGraphics contained in a DrawingModel. DrawingGraphics allow
//! fewer types of geometry than PhysicalGraphics.
//! DrawingGraphics can not be instantiated directly - use \ref DgnModel::CreateDrawingGraphics
//! to create a new element or \ref DgnModel::ReadDrawingGraphics to read an existing one.
//!
//! @ingroup DgnGraphicsGroup
//=======================================================================================
struct DrawingGraphics : public DgnGraphics
{
//__PUBLISH_SECTION_END__
    friend struct DgnModel2d;
//__PUBLISH_SECTION_START__

private:
    DrawingGraphicsR operator= (DrawingGraphicsCR from);
    explicit DrawingGraphics (DrawingGraphicsCR from);
    DrawingGraphics(){}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
