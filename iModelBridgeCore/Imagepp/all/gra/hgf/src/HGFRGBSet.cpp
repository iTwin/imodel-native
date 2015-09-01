//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hgf/src/HGFRGBSet.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Methods for class : HGFRGBSet
//---------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <Imagepp/all/h/HGFRGBSet.h>

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HGFRGBSet::HGFRGBSet()
    : m_pChildren(0), m_Count(0)
    {
    }


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HGFRGBSet::HGFRGBSet(const HGFRGBSet& pi_rSrc)
    : m_pChildren(0), m_Count(0)
    {
    LevelDeepCopy(&pi_rSrc, 7);
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HGFRGBSet::~HGFRGBSet()
    {
    if (m_Count < 8)
        {
        delete[] m_pChildren;
        m_pChildren = 0;
        }
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
HGFRGBSet& HGFRGBSet::operator=(const HGFRGBSet& pi_rSrc)
    {
    // This operation may be expensive, so dont do it when not necessary...
    if (&pi_rSrc != this)
        LevelDeepCopy(&pi_rSrc, 7);

    return *this;
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

void HGFRGBSet::LevelDeepCopy(const HGFRGBSet* pi_pChildArray, int8_t pi_Level)
    {
    // We have prefered to use eight "if" statement rather than "for" loop for
    // optimisation purpose.

    if (pi_Level >= 0)
        {
        m_Count = pi_pChildArray->m_Count;
        if (pi_pChildArray->m_pChildren && (m_Count < 8))
            {
            m_pChildren = new HGFRGBSet[8];

            if (pi_pChildArray->m_pChildren[0].m_pChildren)
                m_pChildren[0].LevelDeepCopy(&pi_pChildArray->m_pChildren[0], pi_Level - 1);
            else
                m_pChildren[0].m_Count = pi_pChildArray->m_pChildren[0].m_Count;

            if (pi_pChildArray->m_pChildren[1].m_pChildren)
                m_pChildren[1].LevelDeepCopy(&pi_pChildArray->m_pChildren[1], pi_Level - 1);
            else
                m_pChildren[1].m_Count = pi_pChildArray->m_pChildren[1].m_Count;

            if (pi_pChildArray->m_pChildren[2].m_pChildren)
                m_pChildren[2].LevelDeepCopy(&pi_pChildArray->m_pChildren[2], pi_Level - 1);
            else
                m_pChildren[2].m_Count = pi_pChildArray->m_pChildren[2].m_Count;

            if (pi_pChildArray->m_pChildren[3].m_pChildren)
                m_pChildren[3].LevelDeepCopy(&pi_pChildArray->m_pChildren[3], pi_Level - 1);
            else
                m_pChildren[3].m_Count = pi_pChildArray->m_pChildren[3].m_Count;

            if (pi_pChildArray->m_pChildren[4].m_pChildren)
                m_pChildren[4].LevelDeepCopy(&pi_pChildArray->m_pChildren[4], pi_Level - 1);
            else
                m_pChildren[4].m_Count = pi_pChildArray->m_pChildren[4].m_Count;

            if (pi_pChildArray->m_pChildren[5].m_pChildren)
                m_pChildren[5].LevelDeepCopy(&pi_pChildArray->m_pChildren[5], pi_Level - 1);
            else
                m_pChildren[5].m_Count = pi_pChildArray->m_pChildren[5].m_Count;

            if (pi_pChildArray->m_pChildren[6].m_pChildren)
                m_pChildren[6].LevelDeepCopy(&pi_pChildArray->m_pChildren[6], pi_Level - 1);
            else
                m_pChildren[6].m_Count = pi_pChildArray->m_pChildren[6].m_Count;

            if (pi_pChildArray->m_pChildren[7].m_pChildren)
                m_pChildren[7].LevelDeepCopy(&pi_pChildArray->m_pChildren[7], pi_Level - 1);
            else
                m_pChildren[7].m_Count = pi_pChildArray->m_pChildren[7].m_Count;
            }
        }
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool HGFRGBSet::IsIn(Byte pi_R, Byte pi_G, Byte pi_B, int8_t pi_Level) const
    {
    return (m_Count == 8)
           ? true
           : ((m_pChildren == 0)
              ? false
              : m_pChildren[  ((pi_R >> pi_Level) & 1) << 2
                              | ((pi_G >> pi_Level) & 1) << 1
                              | ((pi_B >> pi_Level) & 1)      ].IsIn(pi_R, pi_G, pi_B, pi_Level-1));
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool HGFRGBSet::Add(Byte pi_R, Byte pi_G, Byte pi_B, int8_t pi_Level)
    {
    if (m_Count == 8)
        return false;

    if (pi_Level >= 0)
        {
        if (!m_pChildren)
            m_pChildren = new HGFRGBSet[8];

        if (m_pChildren[   ((pi_R >> pi_Level) & 1) << 2
                           | ((pi_G >> pi_Level) & 1) << 1
                           | ((pi_B >> pi_Level) & 1)     ].Add(pi_R, pi_G, pi_B, pi_Level-1))
            ++m_Count;

        if (m_Count == 8)
            {
            delete[] m_pChildren;
            m_pChildren = 0;
            }
        }
    else
        m_Count = 8;

    return (m_Count == 8);
    }

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool HGFRGBSet::Remove(Byte pi_R, Byte pi_G, Byte pi_B, int8_t pi_Level)
    {
    if (m_Count == 0)
        return false;

    if (pi_Level >= 0)
        {
        if (!m_pChildren)
            {
            m_pChildren = new HGFRGBSet[8];
            for (size_t i = 0; i < 8; m_pChildren[i++].m_Count = 8);
            }

        if (m_pChildren[   ((pi_R >> pi_Level) & 1) << 2
                           | ((pi_G >> pi_Level) & 1) << 1
                           | ((pi_B >> pi_Level) & 1)     ].Remove(pi_R, pi_G, pi_B, pi_Level-1))
            --m_Count;

        if (m_Count == 0)
            {
            delete[] m_pChildren;
            m_pChildren = 0;
            }
        }
    else
        m_Count = 0;

    return (m_Count == 0);
    }

