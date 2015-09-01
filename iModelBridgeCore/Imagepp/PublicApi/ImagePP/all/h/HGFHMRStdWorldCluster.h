//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFHMRStdWorldCluster.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFHMRStdWorldCluster
//-----------------------------------------------------------------------------
// Defines the standard HMR cluster which contains the most useful world
// definitions
//-----------------------------------------------------------------------------
#pragma once

#include <Imagepp/all/h/HGF2DWorld.h>
#include <Imagepp/all/h/HGF2DWorldCluster.h>

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Alain Robert

    This class represents a world cluster containing immediately upon instantiation
    some predefined standard worlds commonly used by HMR applications.
    Three worlds are automatically instantiated, and thus do not need to be by
    the user. The identifiers for these three worlds are:
        HGF2DWorld_UNKNOWNWORLD
        HGF2DWorld_HMRWORLD
        HGF2DWorld_DGNWORLD
        HGF2DWorld_INTERGRAPH
        HGF2DWorld_ITIFFWORLD
        HGF2DWorld_GEOTIFFUNKNOWN
        HGF2DWorld_GEOGRAPHIC

    These worlds should be used in a conventional and predefined fashion in
    order to respect the conventions ruling them. Each of these worlds can be
    defined relatively to the others, and in fact such link descriptions are
    defined through use of the transformation model linking the different worlds.
    The UNKNOWN world is to be used when the world needed to represent a
    graphic object is unknown. It serves as a default world, in case the desired one
    cannot be found. The definition of the unknown world states that there is reversal
    of the Y-axis relative to the other base pre-defined cartographic worlds (DGN).
    There is no translation, rotation, nor scaling however.

    The relation linking the HMR and DGN world is identity. Note that even
    though the relation imposes no transformation between HMR and DGN worlds after
    instantiation of the cluster, the user or owner of the present cluster can change
    their relation. The HMR world represents the space in which are interpreted
    HMR type images. The DGN world is the space where all Intergraph and MicroStation
    vector and raster objects are interpreted. The relation between HMR and DGN world
    depends on the configuration parameters of the DGN world definition. Such parameters
    are usually included in DGN vector files and define the Global Origin (A global
    offset) and the size of the UOR (The only recognized unit in DGN space) relative
    to user defined units. In addition, some data must be made available in order
    to set the relation between DGN defined user units and meters, which is the base
    unit for HMR space.

    The INTERGRAPHWORLD is virtually identical the DGN except it is intended
    to be used as reference by INTERGRAPH image file formats.
    The ITIFFWORLD ius used solely for internal reference of ITIFF version 2.0
    raster files and is not visible outside of this context.
    The GEOTIFFUNKNOWN is used to attach any GeoTIFF raster files for which the
    interpretation coordinate system cannot be determined.
    Finaly the GEOGRAPHIC world is used for Latitude-Longitude coordinates
    that are transformed by identity to the DGN world. This representation is
    of course inadequate but useful for base viewing of geographic based images.

    As for any other coordinate systems it is possible to change the relation
    between worlds automatically instantiated by the cluster. If the user wishes
    to change the relations, it is important to know the structure of the
    coordinate system tree structure they belong to after creation.

    @code
        HGFHMRStdWorldCluster   MyApplicationCluster;

        HFCPtr<HGF2DWorld> pHMRWorld = MyApplicationCluster.GetWorldReference(HGF2DWorld_HMRWORLD);

        HFCPtr<HGF2DWorld> pDGNWorld = MyApplicationCluster.GetWorldReference(HGF2DWorld_DGNWORLD);

        HGF2DTranslation GlobalOriginTrans(HGF2DDisplacement(123.4, 12348623.4));

        pDGNWorld->SetReference(GlobalOriginTrans, pHMRWorld);
    @end

    -----------------------------------------------------------------------------
*/
class HGFHMRStdWorldCluster : public HGF2DWorldCluster
    {
    HPM_DECLARE_CLASS_DLL(IMAGEPP_EXPORT,  HGFHMRStdWorldClusterId)

public:

    IMAGEPP_EXPORT                     HGFHMRStdWorldCluster();
    IMAGEPP_EXPORT virtual             ~HGFHMRStdWorldCluster();

    // From HGF2DWorldCluster
    IMAGEPP_EXPORT virtual HFCPtr<HGF2DWorld>
    GetWorldReference(HGF2DWorldIdentificator pi_Identifier) const;

private:

    // Copy constructor (desactivated)
    HGFHMRStdWorldCluster(const HGFHMRStdWorldCluster&      pi_rObj);
    // Assignement operator (desactivated)
    HGFHMRStdWorldCluster&     operator=(const HGFHMRStdWorldCluster& pi_rObj);
    };

END_IMAGEPP_NAMESPACE