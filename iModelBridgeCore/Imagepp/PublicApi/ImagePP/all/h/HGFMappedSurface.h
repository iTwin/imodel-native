//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFMappedSurface.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
// Class : HGFMappedSurface
//-----------------------------------------------------------------------------
// This class encapsulates the functionalities of the page
//-----------------------------------------------------------------------------
#pragma once

#include "HGSSurface.h"
#include "HGF2DCoordSys.h"
#include "HGFCoordSysContainer.h"
#include "HGF2DExtent.h"

/** -----------------------------------------------------------------------------
    @version 1.0
    @author Hugues Lepage

    Il est recommand� de lire au pr�alable le document
    " HMR Graphic Surface - Introduction rapide "
    pour  comprendre les surfaces.

    La classe HGFMappedSurface est un descendant de HGSSurface. Elle ajoute �
    cette classe un "coordsys" pour positionner la surface dans un espace
    g�or�f�renc�.

    Il est possible de construire une surface en y associant directement un
    "coordsys". Il est possible �galement de "fitter" la surface dans un
    "extent". Le constructeur s'occupe alors de calculer les ratios de "scaling"
    � appliquer � l'interne. ATTENTION: Dans ce dernier cas, l'orientation
    de la surface g�or�f�renc�e suivra celle du syst�me de coordonn�es
    de l'"extent", m�me s'il y a un "scale" appliqu�.

    Pour obtenir le "coordsys" de la surface (le v�ritable "coordsys" qui
    mappe chaque pixel physique de la surface, on doit appeler la m�thode
    GetSurfaceCoordSys().

    Il est possible que l'on veuille ne travailler qu'avec une portion de
    la surface, c'est-�-dire "mapper" seulement une portion rectangulaire de
    la surface totale.  Pour ce faire, on peut d�finir des "offsets"
    qui pr�cisent o� comment cette portion. La portion poss�de des
    dimensions en pixels pr�cis� avec la m�thode SetDimensions. Pour
    appliquer un "offset", on appelle la m�thode SetOffsets. Pour
    obtenir le "coordsys" de cette portion, on appelle GetCoordSys.
    On peut toujours obtenir le coordsys de la surface compl�te en
    appelant GetSurfaceCoordSys. Toutefois, toutes les m�thodes de
    transformation du "coordsys" s'appliquent sur le "coordsys" de la
    portion et non sur celui de la surface. Le "coordsys" de la surface ne
    fait que suivre en tenant compte de l'"offset".

    L'interface offre un certain nombre de services pour faciliter le
    positionnement de la surface.
    Il y a des versions "FromRef" et ordinaires. Les versions ordinaires
    appliquent une transformation sur la derni�re position de la surface.
    Les versions "FromRef" appliquent une transformation � partir du premier
    "coordsys" contenu dans la surface (pass� � la construction de la surface
    ou indirectement via l'extent). Ces derni�res versions effacent donc
    toutes les transformations appliqu�es depuis.
    -----------------------------------------------------------------------------
*/
class HGFMappedSurface : public HGSSurface
    {
    // Class ID for this class.
    HDECLARE_CLASS_ID(1219, HGSSurface)

public:

    // Primary methods
    _HDLLg                 HGFMappedSurface(const HFCPtr<HGSSurfaceDescriptor>&    pi_rpDescriptor,
                                            const HGSToolbox*                      pi_pToolbox,
                                            const HGSSurfaceCapabilities*          pi_pCapabilities,
                                            const HFCPtr<HGF2DCoordSys>&           pi_rpRefCoordSys);

    _HDLLg                 HGFMappedSurface(HGSSurfaceImplementation*              pi_pImplementation,
                                            const HFCPtr<HGF2DCoordSys>&           pi_rpRefCoordSys);

    _HDLLg                 HGFMappedSurface(const HFCPtr<HGSSurfaceDescriptor>&    pi_rpDescriptor,
                                            const HGSToolbox*                      pi_pToolbox,
                                            const HGSSurfaceCapabilities*          pi_pCapabilities,
                                            const HGF2DExtent&                     pi_rExtent);

    _HDLLg                 HGFMappedSurface(HGSSurfaceImplementation*              pi_pImplementation,
                                            const HGF2DExtent&                     pi_rExtent);

    _HDLLg virtual         ~HGFMappedSurface();

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
    _HDLLg void            TranslateFromRef(const HGF2DDisplacement& pi_rDisplacement);
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

#include "HGFMappedSurface.hpp"
