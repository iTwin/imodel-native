//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFQuadTree.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <Imagepp/all/h/HGF2DLiteExtent.h>

#pragma once

BEGIN_IMAGEPP_NAMESPACE

template <class T> class HGFQuadTreeNode
    {
private:
    typedef list<T> ItemList;

    HGF2DLiteExtent      m_Extent;
    HGF2DLiteExtent      m_ExtentChild[4];
    HGFQuadTreeNode<T> * m_Nodes[4];
    ItemList             m_Items;

    // Disabled
    HGFQuadTreeNode();
public:
    HGFQuadTreeNode  (HGF2DLiteExtent const& pi_rExtent);
    HGFQuadTreeNode  (HGFQuadTreeNode const& pi_obj);
    virtual         ~HGFQuadTreeNode ();

    bool            AddItem (T const& pi_rItem);

    const HGF2DLiteExtent&
    GetExtent () const;

    bool            IsPointIn (double pi_x, double pi_y) const;

    void            Query       (double pi_x, double pi_y, ItemList& pio_rItems) const;
    T const*        QuerySingle (double pi_x, double pi_y) const;

    void            Dump(ofstream& outStream) const;
    };


template <class T> class HGFQuadTree
    {
public:
    typedef list<T> ItemList;

    HGFQuadTree(HGF2DLiteExtent const& pi_rExtent);
    HGFQuadTree(HGFQuadTree const& pi_obj);
    virtual     ~HGFQuadTree();

    // Disabled
    HGFQuadTree();

    bool        AddItem(T const& pi_rItem);

    void        Query       (double pi_x, double pi_y, ItemList& pio_rItems) const;
    T const*    QuerySingle (double pi_x, double pi_y) const;

    void        Dump(ofstream& outStream) const;

private:

    HGFQuadTreeNode<T> m_Root;
    mutable T const*   m_LastFoundNode;
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
inline HGFQuadTreeNode<T>::HGFQuadTreeNode(HGF2DLiteExtent const& pi_rExtent)
    : m_Extent(pi_rExtent)
    {
    memset(m_Nodes, 0, 4 * sizeof(HGFQuadTreeNode<T> *));

    // Compute the extents of the 4 childreen
    m_ExtentChild[0] = HGF2DLiteExtent(HGF2DPosition(pi_rExtent.GetXMin(), pi_rExtent.GetYMin()),
                                       HGF2DPosition(pi_rExtent.GetXMin() + pi_rExtent.GetWidth() / 2.0, pi_rExtent.GetYMin() + pi_rExtent.GetHeight() / 2.0));

    m_ExtentChild[1] = HGF2DLiteExtent(HGF2DPosition(pi_rExtent.GetXMin(), pi_rExtent.GetYMin() + pi_rExtent.GetHeight() / 2.0),
                                       HGF2DPosition(pi_rExtent.GetXMin() + pi_rExtent.GetWidth() / 2.0, pi_rExtent.GetYMax()));

    m_ExtentChild[2] = HGF2DLiteExtent(HGF2DPosition(pi_rExtent.GetXMin() + pi_rExtent.GetWidth() / 2.0, pi_rExtent.GetYMin() + pi_rExtent.GetHeight() / 2.0),
                                       HGF2DPosition(pi_rExtent.GetXMax(), pi_rExtent.GetYMax()));

    m_ExtentChild[3] = HGF2DLiteExtent(HGF2DPosition(pi_rExtent.GetXMin() + pi_rExtent.GetWidth() / 2.0, pi_rExtent.GetYMin()),
                                       HGF2DPosition(pi_rExtent.GetXMax(), pi_rExtent.GetYMin() + pi_rExtent.GetHeight() / 2.0));

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
inline HGFQuadTreeNode<T>::HGFQuadTreeNode(HGFQuadTreeNode<T> const& pi_obj)
    :m_Extent(pi_obj.m_Extent)
    {
    m_ExtentChild[0] = pi_obj.m_ExtentChild[0];
    m_ExtentChild[1] = pi_obj.m_ExtentChild[1];
    m_ExtentChild[2] = pi_obj.m_ExtentChild[2];
    m_ExtentChild[3] = pi_obj.m_ExtentChild[3];

    if (NULL != pi_obj.m_Nodes[0])
        m_Nodes[0] = new HGFQuadTreeNode<T>(*pi_obj.m_Nodes[0]);
    else
        m_Nodes[0] = NULL;

    if (NULL != pi_obj.m_Nodes[1])
        m_Nodes[1] = new HGFQuadTreeNode<T>(*pi_obj.m_Nodes[1]);
    else
        m_Nodes[1] = NULL;

    if (NULL != pi_obj.m_Nodes[2])
        m_Nodes[2] = new HGFQuadTreeNode<T>(*pi_obj.m_Nodes[2]);
    else
        m_Nodes[2] = NULL;

    if (NULL != pi_obj.m_Nodes[3])
        m_Nodes[3] = new HGFQuadTreeNode<T>(*pi_obj.m_Nodes[3]);
    else
        m_Nodes[3] = NULL;

    m_Items = pi_obj.m_Items;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
inline HGFQuadTreeNode<T>::~HGFQuadTreeNode()
    {
    delete m_Nodes[0];
    delete m_Nodes[1];
    delete m_Nodes[2];
    delete m_Nodes[3];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
inline const HGF2DLiteExtent& HGFQuadTreeNode<T>::GetExtent() const
    {
    return m_Extent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
inline bool HGFQuadTreeNode<T>::IsPointIn(double pi_x, double pi_y) const
    {
    return m_Extent.IsPointIn(HGF2DPosition(pi_x, pi_y));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
inline bool HGFQuadTreeNode<T>::AddItem(T const& pi_rItem)
    {
    HGF2DLiteExtent ItemExtent (pi_rItem.GetExtent());

    if (m_Extent.GetXMin() < ItemExtent.GetXMin() && m_Extent.GetYMin() < ItemExtent.GetYMin() &&
        m_Extent.GetXMax() > ItemExtent.GetXMax() && m_Extent.GetYMax() > ItemExtent.GetYMax())
        {
        // Create the node if not exist
        if (NULL == m_Nodes[0])
            m_Nodes[0] = new HGFQuadTreeNode<T>(m_ExtentChild[0]);

        if (m_Nodes[0]->AddItem(pi_rItem))
            return true;

        if (NULL == m_Nodes[1])
            m_Nodes[1] = new HGFQuadTreeNode<T>(m_ExtentChild[1]);

        if (m_Nodes[1]->AddItem(pi_rItem))
            return true;

        if (NULL == m_Nodes[2])
            m_Nodes[2] = new HGFQuadTreeNode<T>(m_ExtentChild[2]);

        if (m_Nodes[2]->AddItem(pi_rItem))
            return true;

        if (NULL == m_Nodes[3])
            m_Nodes[3] = new HGFQuadTreeNode<T>(m_ExtentChild[3]);

        if (m_Nodes[3]->AddItem(pi_rItem))
            return true;

        m_Items.push_back(pi_rItem);
        return true;
        }
    else
        {
        // not in extent
        return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
inline void HGFQuadTreeNode<T>::Query(double pi_x, double pi_y, ItemList& pio_rItems) const
    {
    if (m_Nodes[0] != NULL)
        m_Nodes[0]->Query(pi_x, pi_y, pio_rItems);

    if (m_Nodes[1] != NULL)
        m_Nodes[1]->Query(pi_x, pi_y, pio_rItems);

    if (m_Nodes[2] != NULL)
        m_Nodes[2]->Query(pi_x, pi_y, pio_rItems);

    if (m_Nodes[3] != NULL)
        m_Nodes[3]->Query(pi_x, pi_y, pio_rItems);


    if (m_Extent.IsPointIn(HGF2DPosition(pi_x, pi_y)))
        {
        for (typename ItemList::const_iterator itr(m_Items.begin()); itr != m_Items.end(); ++itr)
            {
            if (itr->IsPointIn(pi_x, pi_y))
                pio_rItems.push_back(*itr);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
inline T const* HGFQuadTreeNode<T>::QuerySingle(double pi_x, double pi_y) const
    {
    if (m_Extent.IsPointIn(HGF2DPosition(pi_x, pi_y)))
        {
        const T (* pItem)(NULL);

        // Search in current node first
        for (typename ItemList::const_iterator itr(m_Items.begin()); itr != m_Items.end(); ++itr)
            {
            if (itr->IsPointIn(pi_x, pi_y))
                return &(*itr);
            }

        // Search all subsequent nodes
        if (m_Nodes[0] != NULL)
            if ((pItem = m_Nodes[0]->QuerySingle(pi_x, pi_y)) != NULL)
                return pItem;

        if (m_Nodes[1] != NULL)
            if ((pItem = m_Nodes[1]->QuerySingle(pi_x, pi_y)) != NULL)
                return pItem;

        if (m_Nodes[2] != NULL)
            if ((pItem = m_Nodes[2]->QuerySingle(pi_x, pi_y)) != NULL)
                return pItem;

        if (m_Nodes[3] != NULL)
            if ((pItem = m_Nodes[3]->QuerySingle(pi_x, pi_y)) != NULL)
                return pItem;
        }

    // Nothing found
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
inline void HGFQuadTreeNode<T>::Dump(ofstream& outStream) const
    {
    // Current item node first
    for (typename ItemList::const_iterator itr(m_Items.begin()); itr != m_Items.end(); ++itr)
        {
        itr->Dump(outStream);
        }

    // Add all subsequent nodes
    if (m_Nodes[0] != NULL)
        m_Nodes[0]->Dump(outStream);

    if (m_Nodes[1] != NULL)
        m_Nodes[1]->Dump(outStream);

    if (m_Nodes[2] != NULL)
        m_Nodes[2]->Dump(outStream);

    if (m_Nodes[3] != NULL)
        m_Nodes[3]->Dump(outStream);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
inline HGFQuadTree<T>::HGFQuadTree(HGF2DLiteExtent const& pi_rExtent)
    : m_Root(pi_rExtent),
      m_LastFoundNode(NULL)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
inline HGFQuadTree<T>::HGFQuadTree(HGFQuadTree<T> const& pi_obj)
    :m_Root(pi_obj.m_Root),
     m_LastFoundNode(NULL)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
HGFQuadTree<T>::~HGFQuadTree()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
inline bool HGFQuadTree<T>::AddItem(T const& pi_rItem)
    {
    return m_Root.AddItem(pi_rItem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
inline void HGFQuadTree<T>::Query (double    pi_x,
                                   double    pi_y,
                                   ItemList& pio_rItems) const
    {
    m_Root.Query(pi_x, pi_y, pio_rItems);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
inline T const* HGFQuadTree<T>::QuerySingle (double    pi_x, double    pi_y) const
    {
    if (m_LastFoundNode != NULL && m_LastFoundNode->IsPointIn(pi_x, pi_y))
        return m_LastFoundNode;

    m_LastFoundNode = m_Root.QuerySingle(pi_x, pi_y);

    return m_LastFoundNode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2003
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T>
inline void HGFQuadTree<T>::Dump (ofstream& outStream) const
    {
    m_Root.Dump(outStream);
    }

END_IMAGEPP_NAMESPACE