//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFRGBCube.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Inline methods for class HGFRGBSet
//---------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HGFRGBCube::HGFRGBCube()
    : m_Rmin(0), m_Rmax(255),
      m_Gmin(0), m_Gmax(255),
      m_Bmin(0), m_Bmax(255)
    {
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HGFRGBCube::HGFRGBCube(Byte pi_Rmin, Byte pi_Rmax,
                              Byte pi_Gmin, Byte pi_Gmax,
                              Byte pi_Bmin, Byte pi_Bmax)
    : m_Rmin(pi_Rmin), m_Rmax(pi_Rmax),
      m_Gmin(pi_Gmin), m_Gmax(pi_Gmax),
      m_Bmin(pi_Bmin), m_Bmax(pi_Bmax)
    {
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HGFRGBCube::HGFRGBCube(const HGFRGBCube& pi_rSrc)
    : m_Rmin(pi_rSrc.m_Rmin), m_Rmax(pi_rSrc.m_Rmax),
      m_Gmin(pi_rSrc.m_Gmin), m_Gmax(pi_rSrc.m_Gmax),
      m_Bmin(pi_rSrc.m_Bmin), m_Bmax(pi_rSrc.m_Bmax)
    {

    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HGFRGBCube::~HGFRGBCube()
    {
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline HGFRGBCube& HGFRGBCube::operator=(const HGFRGBCube& pi_rSrc)
    {
    m_Rmin = pi_rSrc.m_Rmin;
    m_Rmax = pi_rSrc.m_Rmax;

    m_Gmin = pi_rSrc.m_Gmin;
    m_Gmax = pi_rSrc.m_Gmax;

    m_Bmin = pi_rSrc.m_Bmin;
    m_Bmax = pi_rSrc.m_Bmax;

    return *this;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
inline bool HGFRGBCube::IsIn(Byte pi_R, Byte pi_G, Byte pi_B) const
    {
    return    ((pi_R >= m_Rmin) && (pi_R <= m_Rmax))
              && ((pi_G >= m_Gmin) && (pi_G <= m_Gmax))
              && ((pi_B >= m_Bmin) && (pi_B <= m_Bmax));
    }

END_IMAGEPP_NAMESPACE