//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DCoordSys.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DCoordSys
//-----------------------------------------------------------------------------
// Description of 2D coordinate system. This class implements the context in
// which graphical and positional object are interpreted.
//-----------------------------------------------------------------------------
#pragma once

#include "HGF2DTransfoModel.h"
#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HPMPersistentObject.h>
#include <Imagepp/all/h/HFCExclusiveKey.h>

BEGIN_IMAGEPP_NAMESPACE
class HGF2DTransfoModel;
class HGF2DCoordSysImpl;

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class encapsulates a 2D coordinate system. A coordinate system represents
    and up to a certain point defines the world in which objects can exist.
    Unfortunately, there is no absolute coordinate system to base the definition on.
    Therefore, all coordinates system are defined relative to other coordinate
    systems, or if not related to any other, simply are in a world of their own.

    This class is strongly linked with the class HGF2DTransfoModel which permits to
    express bi-directional relations between two coordinate systems. To each
    HGF2DCoordSys intance, can be assigned a relation to an other instance of
    HGF2DCoordSys with the intermediate HGF2DTransfoModel expressing the relation.

    This class provides the interface to transform coordinates or a list of coordinates
    to and from the coordinate system to or from another. To perform this
    transformation, it will try to find a relation or a list of relations which
    permit the transformation. This resolution may be indirect. An instance
    of a HGF2DCoordSys may know the relation permitting to immediately
    perform the transformation. If it does not know this relation, it
    will ask other HGF2DCoordSys it is related to if they know how to transform
    to the specified coordinate system, and so on, until a set of relations is
    built. This set of relations is then built into a new HGF2DTransfoModel,
    and added as a relation to itself.

    This class therefore permits to instantiate objects, which can learn up to
    a certain point, how to generate relations, and therefore accelerate the
    processing time upon usage.


    Worlds

    A world is a coordinate system that is constructed without making a
    reference to another coordinate system. There is always a world constructed
    first in any application using coordinate systems. After construction of
    a world, the coordinate system is alone in its interpretation space.

    Creating a coordinate system relative to a world

    After a world has been created, it is possible to create other coordinate
    systems defined relative to this world. First a transformation model must be
    constructed to represent the relation between the new coordinate system,
    and the world. Then the new coordinate system is constructed by giving
    this transformation model from new model to this world.

    The coordinate system upon construction sets the world as its primary
    reference system. It then tells the world that it is used as a reference,
    and this world will add an entry in its inverse reference list to the
    newly constructed coordinate system. Since transformation models are
    always bi-directional, and that entries exist in both coordinate systems
    of links to each other, the world will be able to find the transformation
    model required to convert to the new coordinate system and vice-versa.

    The result of coordinate systems constructions
    It is not possible to manually add links between coordinate systems, although
    it is possible to change the reference system of a coordinate system.
    It results that user construction of a coordinate system set will have
    a tree structure.

    Automatic generation of links through usage

    Whenever a conversion from a coordinate system to another is made, if the
    two systems only know each other through related coordinate system, a new
    transformation model is created as a composite of all the transformation
    models required and added as an alternate link between these coordinate
    systems.

    If alternate links were to be considered equal, with primary links,
    searching for a new link would become extremely complicated, and
    would result into possible infinite loop, since many paths would
    be available. Alternate links are never used as bridge during the
    creation of new links. They can only be used by the two coordinate
    systems linked by it as a shortcut for transformations. The primary
    treelike structure is thus preserved, and efficient transformations
    can be performed between remotely related systems.

    Since transformation models to reference can be replaced, the alternate
    links may not remain valid for the existence of the coordinate systems
    it bridges. If for some reason, the reference system of a coordinate system
    is changed, all alternate links will be destroyed, and all coordinate system
    which make reference to this coordinate system directly or indirectly will also
    be ordered to invalidate its alternate links.

    If it happens that a coordinate system is destroyed, it will advise all linked coordinate
    systems of this destruction, and those will terminate their own connection.


    -----------------------------------------------------------------------------
*/

class HGF2DCoordSys : public HPMPersistentObject, public HPMShareableObject<HGF2DCoordSys>
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HGF2DCoordSysId)

public:

    // Primary methods
    IMAGEPP_EXPORT                 HGF2DCoordSys();
    IMAGEPP_EXPORT                 HGF2DCoordSys(const HGF2DTransfoModel&       pi_rModelToRef,
                                         const HFCPtr<HGF2DCoordSys>&   pi_rpRefSys);
    IMAGEPP_EXPORT                 HGF2DCoordSys(const HGF2DCoordSys&           pi_rObj);
    IMAGEPP_EXPORT virtual         ~HGF2DCoordSys();

    // Conversion interface
    IMAGEPP_EXPORT void             ConvertFrom (const HFCPtr<HGF2DCoordSys>&   pi_rpCoordSys,
                                         double                        pi_XIn,
                                         double                        pi_YIn,
                                         double*                       po_pNewX,
                                         double*                       po_pNewY) const;

    IMAGEPP_EXPORT void             ConvertFrom (const HFCPtr<HGF2DCoordSys>&   pi_rpCoordSys,
                                         double*                       pio_pX,
                                         double*                       pio_pY) const;

    IMAGEPP_EXPORT void             ConvertTo   (const HFCPtr<HGF2DCoordSys>&   pi_rpCoordSys,
                                         double                        pi_XIn,
                                         double                        pi_YIn,
                                         double*                       po_pNewX,
                                         double*                       po_pNewY) const;

    IMAGEPP_EXPORT void             ConvertTo(const HFCPtr<HGF2DCoordSys>&      pi_rpCoordSys,
                                      double*                           pio_pX,
                                      double*                           pio_pY) const;


    // Miscalenious
    IMAGEPP_EXPORT HFCPtr<HGF2DTransfoModel>    GetTransfoModelTo(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys) const;

    IMAGEPP_EXPORT void             SetReference(const HGF2DTransfoModel&      pi_rModel,
                                         const HFCPtr<HGF2DCoordSys>&  pi_rpRefSys);

    const HFCPtr<HGF2DCoordSys>& GetReference() const;

    bool                   IsUsedAsReference() const;

    // Geometric property relation flags
    bool                   HasLinearityPreservingRelationTo    (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    bool                   HasShapePreservingRelationTo        (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    bool                   HasDirectionPreservingRelationTo    (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    bool                   HasParallelismPreservingRelationTo  (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;
    bool                   HasStretchRelationTo                (const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const;

#ifdef __HMR_DEBUG
    size_t          GetBranchCount() const;
    size_t          GetCoordSysCount() const;
#endif

private:
    friend class HGF2DCoordSysImpl;

    HFCPtr<HGF2DCoordSys>       m_pRefCoordSys;
    HFCPtr<HGF2DCoordSysImpl>   m_pCoordSysImpl;

    };



class HGF2DCoordSysImpl : public HFCShareableObject<HGF2DCoordSysImpl>
    {
public:

    // Primary methods
    HGF2DCoordSysImpl   ();
    HGF2DCoordSysImpl   (const HGF2DTransfoModel&           pi_rModel,
                         const HFCPtr<HGF2DCoordSysImpl>&   pi_rpRefSys);
    IMAGEPP_EXPORT HGF2DCoordSysImpl   (const HGF2DCoordSysImpl&           pi_rObj);
    virtual         ~HGF2DCoordSysImpl  ();

    // Conversion interface
    void            ConvertFrom (const HFCPtr<HGF2DCoordSysImpl>&   pi_rpCoordSys,
                                 double                            pi_XIn,
                                 double                            pi_YIn,
                                 double*                           po_pNewX,
                                 double*                           po_pNewY) const;

    void            ConvertFrom (const HFCPtr<HGF2DCoordSysImpl>&   pi_rpCoordSys,
                                 double*                           pio_pX,
                                 double*                           pio_pY) const;

    void            ConvertTo   (const HFCPtr<HGF2DCoordSysImpl>&   pi_rpCoordSys,
                                 double                            pi_XIn,
                                 double                            pi_YIn,
                                 double*                           po_pNewX,
                                 double*                           po_pNewY) const;

    void            ConvertTo   (const HFCPtr<HGF2DCoordSysImpl>&   pi_rpCoordSys,
                                 double*                           pio_pX,
                                 double*                           pio_pY) const;

    // Miscalenious
    HFCPtr<HGF2DTransfoModel>       GetTransfoModelTo(const HFCPtr<HGF2DCoordSysImpl>& pi_pCoordSys) const;

    void                            SetReference(const HGF2DTransfoModel&      pi_rModel,
                                                 const HFCPtr<HGF2DCoordSysImpl>&  pi_rpRefSys);
    const HFCPtr<HGF2DCoordSysImpl>&    GetReference() const;
    bool                           IsUsedAsReference() const;

    // Geometric property relation flags
    bool           HasLinearityPreservingRelationTo    (const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys) const;
    bool           HasShapePreservingRelationTo        (const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys) const;
    bool           HasDirectionPreservingRelationTo    (const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys) const;
    bool           HasParallelismPreservingRelationTo  (const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys) const;
    bool           HasStretchRelationTo                (const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys) const;


    // called by HGF2DCoordSys
    IMAGEPP_EXPORT void    RemoveReferences();


#ifdef __HMR_DEBUG
    size_t          GetBranchCount() const;
    size_t          GetCoordSysCount() const;
#endif

protected:

private:

    // Private type
    typedef enum
        {
        HGFCS_SUCCESS,
        HGFCS_NO_RELATION_FOUND
        } Status;

    typedef enum
        {
        DIRECT,
        INVERSE
        } TransfoDirection;


    HGF2DTransfoModel*          m_pTransfoModel;
    HFCPtr<HGF2DCoordSysImpl>   m_pRefCoordSys;
    bool                       m_HasReference;

    // Acceleration attributes
    mutable  HFCExclusiveKey    m_CacheKey;
    HGF2DCoordSysImpl*          m_pLastSystem;
    HGF2DTransfoModel*          m_pLastModel;
    TransfoDirection            m_LastDirection;

    // STL typeDef
    typedef map<HFCPtr<HGF2DCoordSysImpl>, HGF2DTransfoModel*,
            less<HFCPtr<HGF2DCoordSysImpl> >, allocator<HGF2DTransfoModel*> >    ListCoordSysToModel;

    mutable HFCExclusiveKey         m_ListsKey;
    mutable ListCoordSysToModel     m_ListIsRefTo;
    mutable ListCoordSysToModel     m_ListIsAltRefTo;
    mutable ListCoordSysToModel     m_ListHasRefTo;

    // Private methods

    // Assignement operator
    HGF2DCoordSysImpl&  operator=(const HGF2DCoordSysImpl& pi_rObj);

    // Control method
    void            AddInverseRef(HGF2DTransfoModel* pi_pModel,
                                  const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys);

    void            RemoveInverseRef(const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys);

    void            DisconnectFromAltRef(const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys);

    void            AddAltInverseRef(HGF2DTransfoModel* pi_pModel,
                                     const HFCPtr<HGF2DCoordSysImpl>&     pi_rpCoordSys);

    void            RemoveAltInverseRef(const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys);

    void            RemoveAllAlternateRefAndRecurse();


    HGF2DTransfoModel*  FindOrCreateTransfoModel(const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys,
                                                 Status*            po_pStatus,
                                                 TransfoDirection*  po_pDirect);

    HGF2DTransfoModel* FindNearTransfoModel(const HFCPtr<HGF2DCoordSysImpl>&  pi_rpCoordSys,
                                            Status*              po_pStatus,
                                            TransfoDirection*    po_pDirect);

    HGF2DTransfoModel* SearchAndCreateBridgeTransfoModel(const HFCPtr<HGF2DCoordSysImpl>&    pi_rpCoordSys,
                                                         Status*           po_pStatus,
                                                         TransfoDirection* po_pDirect);

    HFCPtr<HGF2DTransfoModel> ProtectedGetTransfoModelTo(const HFCPtr<HGF2DCoordSysImpl>& pi_rpCoordSys,
                                                         Status*              po_pStatus,
                                                         TransfoDirection*    po_pDirect,
                                                         const HGF2DCoordSysImpl& pi_rInitiatorSystem) const;

    void            SaveNewPathTo(HGF2DTransfoModel& pi_rModel,
                                  const HFCPtr<HGF2DCoordSysImpl>&     pi_rpCoordSys);


    };

END_IMAGEPP_NAMESPACE

#include "HGF2DCoordSys.hpp"
