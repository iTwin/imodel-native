//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HPSWorldCluster.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HPSWorldCluster
//-----------------------------------------------------------------------------
// Defines the standard cluster which contains the most useful world
// definitions for PictureScript files.
//-----------------------------------------------------------------------------

#pragma once

#include <Imagepp/all/h/HGF2DWorld.h>
#include <Imagepp/all/h/HGF2DWorldCluster.h>

BEGIN_IMAGEPP_NAMESPACE

class HPSWorldCluster : public HGF2DWorldCluster
    {
public:

    // Class ID for this class.
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HPSWorldClusterId)

    IMAGEPP_EXPORT              HPSWorldCluster();
    virtual             ~HPSWorldCluster();

    // Added method

    bool               SetRelation(HGF2DWorldIdentificator pi_World,
                                    const HGF2DTransfoModel& pi_rRelation,
                                    HGF2DWorldIdentificator pi_BaseWorld);

private:

    // Disabled methods

    HPSWorldCluster(const HPSWorldCluster& pi_rObj);
    HPSWorldCluster& operator=(const HPSWorldCluster& pi_rObj);
    };

END_IMAGEPP_NAMESPACE