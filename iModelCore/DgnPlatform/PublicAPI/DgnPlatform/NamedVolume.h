/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/NamedVolume.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

using BentleyApi::BeSQLite::EC::ECInstanceKey;
using BentleyApi::BeSQLite::EC::ECSqlStatement;
using BentleyApi::BeSQLite::Statement;

//=======================================================================================
//! API to setup user defined regions that can be shaded, clipped, queried & serialized
// @bsiclass                                                 Ramanujam.Raman      01/2015
//=======================================================================================
struct NamedVolume
{
private:
    Utf8String m_name;
    DPoint3d m_origin;
    bvector<DPoint2d> m_shape;
    double m_height;

    ECInstanceKey m_ecKey;

//__PUBLISH_SECTION_END__

    NamedVolume() {}
    void BindForInsertOrUpdate (ECSqlStatement& statement) const;
    static void DrawFace (DPoint3dCP points, size_t numPoints, uint32_t color, ViewContextR context);

    void Get3dShape (bvector<DPoint3d>& shape) const;
    ClipVectorPtr CreateClipVector() const;
    std::unique_ptr<FenceParams> CreateFence (DgnViewportP viewport, bool allowPartialOverlaps) const;
    static std::unique_ptr<DgnViewport> CreateNonVisibleViewport (DgnDbR dgnDb);

    // Gets the range of the volume, described from the Project Coordinate System in storage units 
    void GetRange(DRange3d& range) const;

    void FindElements 
        (
        DgnElementIdSet* elementIds, 
        FenceParamsR fence,
        Statement& stmt,
        DgnDbR dgnDb
        ) const;

//__PUBLISH_SECTION_START__

public:
    /*
     * Create 
     */

    //! Constructor
    //! @param name Name of the volume
    //! @param origin Origin of the volume, described from the Project Coordinate System in storage units 
    //! (this translated origin, and the Project Coordinate System directions together define a Local Coordinate System for the volume)
    //! @param shapePoints Closed loop of 2D points representing the boundary of the extruded volume, described from the 
    //! Local Coordinate System in storage units.
    //! @param shapeNumPoints Length of array of the shapePoints parameter
    //! @param height Height of the extruded volume in storage units
    inline NamedVolume 
        (
        Utf8StringCR name, 
        DPoint3dCR origin, 
        DPoint2dCP shapePoints, 
        size_t shapeNumPoints,
        double height
        );

    //! Copy constructor
    inline NamedVolume (const NamedVolume& other);

    //! Move constructor
    inline NamedVolume (NamedVolume&& other);

    //! Destructor
    ~NamedVolume() {}

    //! Copy assignment operator
    inline NamedVolume& operator= (const NamedVolume& other);

    //! Move assignment operator
    inline NamedVolume& operator= (NamedVolume&& other);

    //! Set the name of the volume
    //! @param name Name of the volume
    void SetName (Utf8StringCR name) {m_name = name;}

    //! Set the origin of the volume 
    //! @param origin Origin of the volume, described from the Project Coordinate System in storage units 
    //! (this translated origin, and the Project Coordinate System directions together define a Local Coordinate System for the volume)
    void SetOrigin (DPoint3dCR origin) {m_origin = origin;}

    //! Set the shape of the volume
    //! @param shapePoints Closed loop of 2D points representing the boundary of the extruded volume, described from the 
    //! Local Coordinate System in storage units.
    //! @param shapeNumPoints Length of array of the shapePoints parameter
    DGNPLATFORM_EXPORT void SetShape (DPoint2dCP shapePoints, size_t shapeNumPoints);

    //! Set the height of the volume
    //! @param height Height of the extruded volume in storage units
    void SetHeight (double height) {m_height = height;}

    /* 
    * Insert 
    */
    
    //! Inserts the named volume into the Db
    DGNPLATFORM_EXPORT StatusInt Insert (DgnDbR);

    /*
     * Read 
     */
     
     //! Reads a named volume 
    DGNPLATFORM_EXPORT static std::unique_ptr<NamedVolume> Read (Utf8StringCR name, DgnDbR);
   
    //! Get the name of the volume
    Utf8StringCR GetName() const {return m_name;}

    //! Get the origin of the volume, described as a translation from the Project Coordinate System in storage units 
    DPoint3dCR GetOrigin() const {return m_origin;}

    //! Get the shape of the volume, described from the Local Coordinate System in storage units.
    const bvector<DPoint2d>& GetShape() const {return m_shape;}

    //! Get the height of the volume in storage units
    double GetHeight() const {return m_height;}
    
     //! Check if a volume exists
    DGNPLATFORM_EXPORT static bool Exists (Utf8StringCR name, DgnDbR);
   
    /*
     * Update
     */
     //! Updates the named volume in the Db
    DGNPLATFORM_EXPORT StatusInt Update (DgnDbR);

    /*
    * Delete
    */
    //! Deletes a named volume 
    DGNPLATFORM_EXPORT static StatusInt Delete (Utf8StringCR name, DgnDbR);

    /*
    * View operations
    */

    //! Setup view clips to the boundary of the volume. 
    //! @param viewport Viewport to setup the clips in 
    DGNPLATFORM_EXPORT void SetClip (DgnViewport& viewport) const;

    //! Clear any view clips previously setup
    //! @param viewport Viewport to clear the clips from.
    DGNPLATFORM_EXPORT static void ClearClip (DgnViewport& viewport);
    
    //! Draw the named volume with the supplied color
    //! @param color Color used to draw/shade the volume. @see Viewport::MakeTrgbColor()
    //! @param context ViewContext used to draw the volume. It needs to be attached to a viewport.
    DGNPLATFORM_EXPORT void Draw (uint32_t color, ViewContextR context) const;

    //! Fit the view to just show the named volume, keeping the view's current rotation.
    //! @param[in] viewport  Viewport to fit the volume
    //! @param[in] aspectRatio The X/Y aspect ratio of the view into which the result will be displayed. If the aspect ratio of the volume does not
    //! match aspectRatio, the shorter axis is lengthened and the volume is centered. If aspectRatio is NULL, no adjustment is made.
    //! @param[in] margin The amount of "white space" to leave around the view volume (which essentially increases the volume
    //! of space shown in the view.) If NULL, no additional white space is added.
    //! @note For 3d views, the camera is centered on the new volume and moved along the view z axis using the default lens angle
    //! such that the entire volume is visible.
    //! @note, for 2d views, only the X and Y values of volume are used.
    //! @see ViewController::LookAtVolume()
    DGNPLATFORM_EXPORT void Fit (DgnViewport& viewport, double const* aspectRatio=nullptr, ViewController::MarginPercent const* margin=nullptr) const;

    /*
     * Find/Test contained elements
     */

    //! Find all elements in the project within the named volume
    //! @param[out] elementIds Element ids found (pass nullptr if not interested). Any existing entries are not cleared. 
    //! @param[in] dgnDb DgnDb containing the elements
    //! @param[in] allowPartialOverlaps Pass false to find only elements that are strictly contained. Pass true 
    //! to include elements that partially overlap the volume (i.e., at the boundary). 
    DGNPLATFORM_EXPORT void FindElements (DgnElementIdSet* elementIds, DgnDbR dgnDb, bool allowPartialOverlaps = true) const;

    //! Find all elements in the specified view within the named volume
    //! @param[out] elementIds Element ids found (pass nullptr if not interested). Any existing entries are not cleared. 
    //! @param[in] viewport Viewport that's used to find only the elements displayed. 
    //! @param[in] allowPartialOverlaps Pass false to find only elements that are strictly contained. Pass true 
    //! to include elements that partially overlap the volume (i.e., at the boundary). 
    DGNPLATFORM_EXPORT void FindElements (DgnElementIdSet* elementIds, DgnViewportR viewport, bool allowPartialOverlaps = true) const;

    //! Determines if the named volume contains the element
    //! @param elementRef Element to check
    //! @param allowPartialOverlaps Pass false to check strict containment. Pass true to allow elements that partially
    //! overlap the volume (i.e., at the boundary). 
    //! @return true if volume contains element. false otherwise. 
    DGNPLATFORM_EXPORT bool ContainsElement (DgnElementR elementRef, bool allowPartialOverlaps = true) const;
};

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
inline NamedVolume::NamedVolume 
(
Utf8StringCR name, 
DPoint3dCR origin, 
DPoint2dCP shapePoints, 
size_t numShapePoints,
double height
) : m_name (name), m_origin (origin), m_height (height)
    {
    SetShape (shapePoints, numShapePoints);
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
inline NamedVolume::NamedVolume (const NamedVolume& other) 
    : m_name (other.m_name), m_origin (other.m_origin), m_shape (other.m_shape), m_height (other.m_height), m_ecKey (other.m_ecKey)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
inline NamedVolume::NamedVolume (NamedVolume&& other)
    : m_origin (other.m_origin), m_height (other.m_height), m_ecKey (other.m_ecKey)
    {
    m_name = std::move (other.m_name);
    m_shape = std::move (other.m_shape);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
inline NamedVolume& NamedVolume::operator= (const NamedVolume& other)
    {
    m_name = other.m_name;
    m_origin = other.m_origin;
    m_shape = other.m_shape;
    m_height = other.m_height;
    m_ecKey = other.m_ecKey;
    return *this;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Ramanujam.Raman                   01/15
//+---------------+---------------+---------------+---------------+---------------+-----
inline NamedVolume& NamedVolume::operator= (NamedVolume&& other)
    {
    m_name = std::move (other.m_name);
    m_origin = other.m_origin;
    m_shape = std::move (other.m_shape);
    m_height = other.m_height;
    m_ecKey = other.m_ecKey;
    return *this;
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE

//__PUBLISH_SECTION_END__

