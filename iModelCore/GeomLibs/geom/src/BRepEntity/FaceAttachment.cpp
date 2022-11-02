/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <Geom/BRepEntity.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
FaceAttachment::FaceAttachment()
    {
    m_useColor = m_useMaterial = false;
    m_color = 0;
    m_transparency = 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FaceAttachment::operator == (struct FaceAttachment const& rhs) const
    {
    if (m_useColor      != rhs.m_useColor ||
        m_useMaterial   != rhs.m_useMaterial ||
        m_color         != rhs.m_color ||
        m_transparency  != rhs.m_transparency ||
        m_material      != rhs.m_material)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FaceAttachment::operator < (struct FaceAttachment const& rhs) const
    {
    return (m_useColor         < rhs.m_useColor ||
            m_useMaterial      < rhs.m_useMaterial ||
            m_color            < rhs.m_color ||
            m_transparency     < rhs.m_transparency ||
            m_material         < rhs.m_material);
    }

END_BENTLEY_GEOMETRY_NAMESPACE

