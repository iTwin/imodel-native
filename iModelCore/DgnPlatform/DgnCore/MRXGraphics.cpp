/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/MRXGraphics.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MrxCategoryContainer:: MrxCategoryContainer (XGraphicsContainerCR xGraphics, MrxResolution resolution)  { AddXGraphics (xGraphics, resolution); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void MrxCategoryContainer::AddXGraphics (XGraphicsContainerCR xGraphics,  MrxResolution resolution)
    {
    T_MrxResolutionMap::iterator found = m_resolutions.find (resolution);

    if (found == m_resolutions.end())
        m_resolutions[resolution] = new XGraphicsContainer (xGraphics);
    else
        found->second->Append (xGraphics);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MrxCategoryContainer::~MrxCategoryContainer ()
    {
    for (T_MrxResolutionMap::iterator curr = m_resolutions.begin(); curr != m_resolutions.end(); curr++)
        delete curr->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
MrxGraphicsContainer::~MrxGraphicsContainer ()
    {
    for (T_MrxCategoryMap::iterator curr = m_categories.begin(); curr != m_categories.end(); curr++)
        delete curr->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void MrxGraphicsContainer::AddXGraphics (XGraphicsContainerCR xGraphics, MrxCategory category, MrxResolution resolution)
    {
    T_MrxCategoryMap::iterator found = m_categories.find (category);

    if (found == m_categories.end())
        m_categories[category] = new MrxCategoryContainer (xGraphics, resolution);
    else
        found->second->AddXGraphics (xGraphics, resolution);
    }




























                                                                                                                                                                                                                      