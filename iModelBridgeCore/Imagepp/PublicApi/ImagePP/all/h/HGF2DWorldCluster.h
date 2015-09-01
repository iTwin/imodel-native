//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DWorldCluster.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGF2DWorldCluster
//-----------------------------------------------------------------------------
// Description of 2D world. A world is simply a coordinate system
// which possess an identification
//-----------------------------------------------------------------------------
#pragma once


#include <Imagepp/all/h/HFCPtr.h>
#include <Imagepp/all/h/HGF2DWorld.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class encapsulates a cluster of worlds. A typical usage of coordinate
    systems and worlds will require the coexistence of many different worlds,
    probably sharing a relation (direct or indirect) to one another. The purpose of
    the current class, is to bind many different worlds together, that share such
    relations with each other, in one cohesive entity. The HGF2DWorldCluster class
    makes references to many worlds, but does not own them. The smart pointer mechanism
    is completely responsible of destroying them when they are no more used.

    A cluster appears to contain many worlds. The cluster does not impose, nor know
    the coordinate system network linking the different coordinate systems together.
    There can be links to ordinary coordinate system, which of course cannot
    be part of the cluster.

    Since there can be many instances of HGF2DWorld of a specific identifier, the
    cluster will refuse addition (through direct or indirect linking) of a world
    sharing the same identifier as one already known. There can exist within an
    application many instances of such world, but they must be used in completely
    different contexts.

    The main objective behind the HGF2DWorldCluster is to provide a mechanism
    for an application of obtaining a world whose identifier is known, but its
    instance reference unknown. The application will typically keep a reference
    at all times to the cluster, and will add (link) any world created to this
    cluster. Whenever a specific reference to a world is required, the application
    asks to the cluster for the reference, which should be provided.

    It is quite possible to inherit from the class HGF2DWorldCluster. In fact, it
    is encouraged, since a typical application will require numerous world, and will
    therefore either create all of those itself or relinquish the task a descendant
    of HGF2DWorldCluster. Such a descendant could be responsible of the creation
    of a certain amount of worlds it knows, while still providing the normal
    cluster behavior.

    As an example, someone could decide that two types of worlds are required by
    every application it will implement. Let us call these two worlds A and B.
    A new class is created inheriting from HGF2DWorldCluster, of class name
    USERClusterAandB. The class possesses exactly the same user interface as
    the normal HGF2DWorldCluster, however, upon construction, worlds A and B
    are automatically constructed, with the proper relation between them, and
    linked into the cluster structure. The user therefore has immediate access
    to required types of world, without having to create them manually.
    -----------------------------------------------------------------------------
*/
class HGF2DWorldCluster : public HPMPersistentObject, public HPMShareableObject<HGF2DWorldCluster>
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HGF2DWorldId_Cluster)

public:

    // Primary methods
    IMAGEPP_EXPORT                     HGF2DWorldCluster();
    IMAGEPP_EXPORT virtual             ~HGF2DWorldCluster();

    // World link management
    virtual void        AddWorldReference(const HFCPtr<HGF2DWorld>& pi_rpWorld);
    virtual HFCPtr<HGF2DWorld>
    GetWorldReference(HGF2DWorldIdentificator pi_Identifier) const;
    virtual HFCPtr<HGF2DCoordSys>
    GetCoordSysReference(HGF2DWorldIdentificator pi_Identifier) const;

    virtual bool       HasWorld(HGF2DWorldIdentificator pi_Identifier) const;


protected:

private:

    // STL typeDef
    typedef map<HGF2DWorldIdentificator, HFCPtr<HGF2DWorld>,
            less<HGF2DWorldIdentificator>, allocator<HFCPtr<HGF2DWorld> > >    ListOfWorlds;

    // Attribute
    ListOfWorlds  m_ListOfWorlds;

    // Copy constructor (desactivated)
    HGF2DWorldCluster(const HGF2DWorldCluster&      pi_rObj);
    // Assignement operator (desactivated)
    HGF2DWorldCluster&     operator=(const HGF2DWorldCluster& pi_rObj);
    };

END_IMAGEPP_NAMESPACE
#include "HGF2DWorldCluster.hpp"
