//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DWorldCluster.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Default Constructor for a world cluster
    -----------------------------------------------------------------------------
*/
//-----------------------------------------------------------------------------
inline HGF2DWorldCluster::HGF2DWorldCluster()
    {
    }



//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DWorldCluster::~HGF2DWorldCluster()
    {
    }

/** -----------------------------------------------------------------------------
    Adds a reference to a given world in the cluster

    @param pi_rpWorld IN Smart pointer to world added in cluster. The world (ID)
                         should not allready be included in cluster.

    @code
        HGF2DWorldCluster       MyCluster;
        HFCPtr<HGF2DWorld>      pImageASystem(new HGF2DWorld(HGF2DWorld_DGNWORLD));

        MyCluster.AddWorldReference(pImageASystem);
    @end

    @see GetWorldReference()
    @see GetCoordSysReference()
    -----------------------------------------------------------------------------
*/
inline void HGF2DWorldCluster::AddWorldReference(const HFCPtr<HGF2DWorld>& pi_rpWorld)
    {
// HChk AR 2003-08-25 Should we not check that the world is not already referenced
// by the cluster ????
    m_ListOfWorlds.insert(ListOfWorlds::value_type(pi_rpWorld->GetIdentificator(),
                                                   pi_rpWorld));
    }


/** -----------------------------------------------------------------------------
    Extracts a reference to a indicated world in the cluster. If the indicated
    world cannot be found then a 0 (NULL) pointer is returned.

    @param pi_Identifier IN Identifier of the world to obtain reference to.

    @return A smart pointer to world or NULL pointer if the world was not found.

    @code
        HGF2DWorldCluster       MyCluster;
        HGF2DWorldIdentificator MyWorldId("HMR");
        HFCPtr<HGF2DWorld>      pImageASystem(new HGF2DWorld(HGF2DWorld_DGNWORLD));

        MyCluster.AddWorldReference(pImageASystem);
        ...
        HFCPtr<HGF2DWorld>      pTheWorld = MyCluster.GetWorldReference(HGF2DWorld_DGNWORLD);
    @end

    @see AddWorldReference()
    @see GetCoordSysReference()
    @see HasWorld()
    -----------------------------------------------------------------------------
*/
inline HFCPtr<HGF2DWorld> HGF2DWorldCluster::GetWorldReference(HGF2DWorldIdentificator pi_Identifier) const
    {
    HFCPtr<HGF2DWorld> pReturnedWorld;

    // Locate using identifier
    ListOfWorlds::const_iterator Itr = m_ListOfWorlds.find(pi_Identifier);

    // Check if found
    if (Itr != m_ListOfWorlds.end())
        {
        // Found!
        pReturnedWorld = (*Itr).second;
        }

    return(pReturnedWorld);
    }


/** -----------------------------------------------------------------------------
    Extracts a reference to a indicated world in the cluster. If the indicated
    world cannot be found then a 0 (NULL) pointer is returned. This is identical
    to GetWorldReference except that the pointer return is casted to point to a
    plain coordinate system.

    @param pi_Identifier IN Identifier of the world to obtain reference to.

    @return A smart pointer to coordinate system or NULL pointer if the world was not found.

    @code
        HGF2DWorldCluster       MyCluster;
        HGF2DWorldIdentificator MyWorldId("HMR");
        HFCPtr<HGF2DWorld>      pImageASystem(new HGF2DWorld(HGF2DWorld_DGNWORLD));

        MyCluster.AddWorldReference(pImageASystem);
        ...
        HFCPtr<HGF2DCoordSys>   pTheWorldCoordSys = MyCluster.GetCoordSysReference(HGF2DWorld_DGNWORLD);
    @end


    @see AddWorldReference()
    @see GetWorldReference()
    -----------------------------------------------------------------------------
*/
inline HFCPtr<HGF2DCoordSys> HGF2DWorldCluster::GetCoordSysReference(HGF2DWorldIdentificator pi_Identifier) const
    {
    return((HFCPtr<HGF2DCoordSys>)GetWorldReference(pi_Identifier));
    }


/** -----------------------------------------------------------------------------
    Checks if the indicated world is referenced by the cluster.

    @param pi_Identifier IN Identifier of the world to obtain reference to.

    @return true if the world is referenced or false otherwise.

    @see GetWorldReference()
    -----------------------------------------------------------------------------
*/
inline bool HGF2DWorldCluster::HasWorld(HGF2DWorldIdentificator pi_Identifier) const
    {
    // Check if present
    return(m_ListOfWorlds.find(pi_Identifier) != m_ListOfWorlds.end());
    }

END_IMAGEPP_NAMESPACE