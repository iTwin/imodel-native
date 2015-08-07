//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFGraphicObject.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFGraphicObject
//-----------------------------------------------------------------------------
// Defines common interface for all graphic objects (rasters and vectors)
//-----------------------------------------------------------------------------
#pragma once

#include "HFCPtr.h"

#include "HGF2DExtent.h"

#include "HMGMessageDuplex.h"

#include "HPMPersistentObject.h"

class HGFMappedSurface;
class HGFDrawOptions;
class HMDMetaDataContainerList;
class HPMAttributeSet;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert 

    This abstract class describes any graphical representation of an object,
    simple or complex, that occupies a specific region (known as its extent)
    in a two-dimensional space. This space can be interpreted using the graphic
    object coordinate system. This class allows general manipulation of graphic
    objects without knowing their specific kind.
    -----------------------------------------------------------------------------
*/
class HNOVTABLEINIT HGFGraphicObject : public HPMPersistentObject,
    public HPMShareableObject<HGFGraphicObject>,
    public HMGMessageDuplex
    {

    HPM_DECLARE_CLASS_DLL(_HDLLg, 1010)

public:
    enum Location
        {
        S_INSIDE,
        S_ON_BOUNDARY,
        S_OUTSIDE
        };

    // Primary methods
    _HDLLg                     HGFGraphicObject ();
    _HDLLg                     HGFGraphicObject(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys);
    _HDLLg                     HGFGraphicObject(const HGFGraphicObject& pi_rObj);
    _HDLLg virtual             ~HGFGraphicObject();
    _HDLLg HGFGraphicObject&   operator=(const HGFGraphicObject& pi_rObj);

    // Coordinate system management
    _HDLLg const HFCPtr<HGF2DCoordSys>&
    GetCoordSys () const;
    _HDLLg virtual void        SetCoordSys (const HFCPtr<HGF2DCoordSys>& pi_pCoordSys);

    // Extent management

    /** -----------------------------------------------------------------------------
        Returns the extent of this object.

        @return A constant reference to a HGF2DExtent object that describes the extent
                of this graphic object
        @see HGF2DLiteExtent
        -----------------------------------------------------------------------------
    */
    virtual HGF2DExtent GetExtent() const = 0;

    // Location and Transformation

    /** -----------------------------------------------------------------------------
        Moves this graphic object in its coordinate system by the specified displacement.

        @param pi_rDisplacement IN A constant reference to a displacement object which
                                   specifies the distance and direction of the move.
        @see HGF2DDisplacement
        @see Scale()
        -----------------------------------------------------------------------------
    */
    virtual void        Move(const HGF2DDisplacement& pi_rDisplacement) = 0;

    /** -----------------------------------------------------------------------------
        Scales this graphic object in its coordinate system with regard to its own origin
        or with regard to an origin specified as parameter.
        The same scale factor is applied along both the X and Y axis.

        @param pi_ScaleFactor IN The scaling factor.

        @param pi_rOrigin IN OPTIONAL A constant reference to a location object that
                          specifies the point of origin of the scaling.  If this parameter
                          is not specified, the object is scaled with regard to its
                          own origin.
        @see HGF2DLocation
        @see Move()
        -----------------------------------------------------------------------------
    */
    virtual void        Scale(double pi_ScaleFactor,
                              const HGF2DLocation& pi_rOrigin) = 0;

    // Debug function.
    _HDLLg virtual void        PrintState(ostream& po_rOutput) const;

    // Information
    /** -----------------------------------------------------------------------------
        Returns the relative position of a point compared to the graphic
        representation of object. Three return status are possible: Inside
        (S_INSIDE) which indicates the point is located inside the drawable area
        of the object, S_ON_BOUNDARY indicates that the point is on the boundary of
        the object, or outside (S_OUTSIDE) indicating that the point is located in a
        region where the object is not.

        @param pi_rPoint IN The point to check the relative position of.

        @see HGF2DLocation
        -----------------------------------------------------------------------------
    */
    _HDLLg virtual Location
    Locate(const HGF2DLocation& pi_rPoint) const = 0;

    _HDLLg virtual bool   IsOpaque() const;

    // Attributes

    _HDLLg /*IppImaging_Needs*/ const HPMAttributeSet& GetAttributes() const;

    void            SetAttributes(const HPMAttributeSet&   pi_rAttributes);
    void            SetAttributes(HFCPtr<HPMAttributeSet>& pi_rpAttributes);


    HPMAttributeSet&
    LockAttributes() const;
    void            UnlockAttributes() const;

    // MetaData
    _HDLLg const HFCPtr<HMDMetaDataContainerList>& GetMetaDataContainerList() const;
    _HDLLg void                                    SetMetaDataContainerList(const HFCPtr<HMDMetaDataContainerList>& pi_rpMDContainers);

    // BEIJING_WIP_IPP_HGS &&MM review what a graphic is. HVE shape and HRA are child class. We cannot draw shapes but still
    // want the coordSys stuff. Maybe we should call it HGFSpatialObject and move the draw to HRARaster?
    _HDLLg virtual void LinkTo(HMGMessageSender* pi_pSender, bool pi_ReceiveMessagesInCurrentThread = false) const;

protected:

    virtual void    SetCoordSysImplementation (const HFCPtr<HGF2DCoordSys>& pi_pOldCoordSys);

private:

    HFCPtr<HGF2DCoordSys>            m_pSysCoord;

    HFCPtr<HPMAttributeSet>          m_pAttributes;

    HFCPtr<HMDMetaDataContainerList> m_pMetaDataContainerList;
    };

