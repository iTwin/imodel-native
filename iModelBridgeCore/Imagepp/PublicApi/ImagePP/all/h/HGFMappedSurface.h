//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFMappedSurface.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFMappedSurface
//-----------------------------------------------------------------------------
// This class encapsulates the functionalities of the page
//-----------------------------------------------------------------------------
#pragma once

#include "HRASurface.h"
#include "HGF2DCoordSys.h"
#include "HGFCoordSysContainer.h"
#include "HGF2DExtent.h"

BEGIN_IMAGEPP_NAMESPACE

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Hugues Lepage

    Il est recommandé de lire au préalable le document
    " HMR Graphic Surface - Introduction rapide "
    pour  comprendre les surfaces.

    La classe HGFMappedSurface est un descendant de HGSSurface. Elle ajoute à
    cette classe un "coordsys" pour positionner la surface dans un espace
    géoréférencé.

    Il est possible de construire une surface en y associant directement un
    "coordsys". Il est possible également de "fitter" la surface dans un
    "extent". Le constructeur s'occupe alors de calculer les ratios de "scaling"
    à appliquer à l'interne. ATTENTION: Dans ce dernier cas, l'orientation
    de la surface géoréférencée suivra celle du système de coordonnées
    de l'"extent", même s'il y a un "scale" appliqué.

    Pour obtenir le "coordsys" de la surface (le véritable "coordsys" qui
    mappe chaque pixel physique de la surface, on doit appeler la méthode
    GetSurfaceCoordSys().

    Il est possible que l'on veuille ne travailler qu'avec une portion de
    la surface, c'est-à-dire "mapper" seulement une portion rectangulaire de
    la surface totale.  Pour ce faire, on peut définir des "offsets"
    qui précisent où comment cette portion. La portion possède des
    dimensions en pixels précisé avec la méthode SetDimensions. Pour
    appliquer un "offset", on appelle la méthode SetOffsets. Pour
    obtenir le "coordsys" de cette portion, on appelle GetCoordSys.
    On peut toujours obtenir le coordsys de la surface complète en
    appelant GetSurfaceCoordSys. Toutefois, toutes les méthodes de
    transformation du "coordsys" s'appliquent sur le "coordsys" de la
    portion et non sur celui de la surface. Le "coordsys" de la surface ne
    fait que suivre en tenant compte de l'"offset".

    L'interface offre un certain nombre de services pour faciliter le
    positionnement de la surface.
    Il y a des versions "FromRef" et ordinaires. Les versions ordinaires
    appliquent une transformation sur la dernière position de la surface.
    Les versions "FromRef" appliquent une transformation à partir du premier
    "coordsys" contenu dans la surface (passé à la construction de la surface
    ou indirectement via l'extent). Ces dernières versions effacent donc
    toutes les transformations appliquées depuis.
    -----------------------------------------------------------------------------
*/
class HGFMappedSurface : public HRASurface
    {
    // Class ID for this class.
    HDECLARE_CLASS_ID(HGFMappedSurfaceId, HRASurface)

public:
    
    // Primary methods
    HGFMappedSurface(const HFCPtr<HGSSurfaceDescriptor>& pi_rpDescriptor, const HFCPtr<HGF2DCoordSys>& pi_rpRefCoordSys);

    HGFMappedSurface(const HFCPtr<HGSSurfaceDescriptor>& pi_rpDescriptor, const HGF2DExtent& pi_rExtent);

    virtual ~HGFMappedSurface();

    // Coordsys
    HFCPtr<HGF2DCoordSys>
    GetSurfaceCoordSys() const;
    HFCPtr<HGF2DCoordSys>
    GetCoordSys() const;
    HFCPtr<HGF2DTransfoModel>
    GetTransfoModelToRef() const;
    void            SetTransfoModelToRef(const HGF2DTransfoModel& pi_rTransfoModel);
    HFCPtr<HGFCoordSysContainer>
    GetCoordSysContainer() const;

    void            SetTransfoModelToSurface(const HGF2DTransfoModel& pi_rTransfoModel);

    // Page transformations
    void            Translate(const HGF2DDisplacement& pi_rDisplacement);
    void            TranslateFromRef(const HGF2DDisplacement& pi_rDisplacement);
    void            Rotate(double pi_rRotation);
    void            RotateFromRef(double pi_rRotation);
    void            Scale(double pi_ScaleX, double pi_ScaleY);
    void            ScaleFromRef(double pi_ScaleX, double pi_ScaleY);
    void            FitToExtent(const HGF2DExtent& pi_rExtent);


    // Informations
    HGF2DExtent     GetExtent() const;

    // SLO coordsys (Coordsys to be used by the Renderer ONLY)
    HFCPtr<HGF2DCoordSys>
    GetCoordSysForSLO() const;

    // offsets
    uint32_t        GetXOffset() const;
    uint32_t        GetYOffset() const;
    void            SetOffsets(uint32_t pi_OffsetX, uint32_t pi_OffsetY);

    // dimensions (considering the offset)
    uint32_t        GetWidth() const;
    uint32_t        GetHeight() const;

protected:


private:

    // Attributes
    HFCPtr<HGFCoordSysContainer>
    m_pCoordSysContainer;
    HFCPtr<HGF2DCoordSys>
    m_pCoordSysForSLO;
    HFCPtr<HGF2DCoordSys>
    m_pSurfaceCoordSys;

    // position of the are to draw on
    uint32_t        m_Width;
    uint32_t        m_Height;
    uint32_t        m_OffsetX;
    uint32_t        m_OffsetY;

    // Private method
    HGFMappedSurface();
    HGFMappedSurface(const HGFMappedSurface& pi_rObj);
    HGFMappedSurface&        operator=(const HGFMappedSurface& pi_rObj);
    void            DeepDelete();
    void            UpdateCoordSysWithSLO();
    void            UpdateSurfaceCoordSys();
    void            InitObject(const HFCPtr<HGF2DCoordSys>& pi_rpRefCoordSys);
    };

END_IMAGEPP_NAMESPACE
#include "HGFMappedSurface.hpp"
