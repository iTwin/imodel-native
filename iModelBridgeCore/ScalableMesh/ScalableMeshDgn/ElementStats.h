#pragma once

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementPointStats
    {
    size_t              m_pointCount;
    const size_t        m_maxPointQty;

    explicit            ElementPointStats      () : m_pointCount(0), m_maxPointQty(500000) {}

    bool                HasAny                 () const
        {
        return 0 < m_pointCount;
        }

    size_t              GetPointCapacity       () const
        {
        return (m_pointCount > m_maxPointQty ? m_maxPointQty : m_pointCount);
        }
    };
    
/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementLinearStats
    {
    size_t              m_pointCount;
    size_t              m_featureCount;
    size_t              m_elementMaxPointCount;
    const size_t        m_maxPointQty;

    explicit            ElementLinearStats     () : m_pointCount(0), m_featureCount(0), m_elementMaxPointCount(0), m_maxPointQty(500000) {}

    bool                HasAny                 () const
        {
        return 0 < m_pointCount && 0 < m_featureCount;
        }

    inline void         SetMaxPointQty         (size_t pointQty)
        {
        if(pointQty > m_elementMaxPointCount)
            m_elementMaxPointCount = pointQty;
        }

    size_t              GetPointCapacity       () const
        {
        if(m_pointCount <= m_maxPointQty)
            return m_pointCount;

        if(m_elementMaxPointCount <= m_maxPointQty)
            return m_maxPointQty;

        return m_elementMaxPointCount;
        }
    };
    
/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementMeshStats
    {
    size_t              m_pointCount;
    size_t              m_featureCount;
    size_t              m_elementMaxPointCount;
    const size_t        m_maxPointQty;

    explicit            ElementMeshStats          () : m_pointCount(0), m_featureCount(0), m_elementMaxPointCount(0), m_maxPointQty(500000) {}

    bool                HasAny                    () const
        {
        return 0 < m_pointCount && 0 < m_featureCount;
        }

    inline void         SetMaxPointQty            (size_t pointQty)
        {
        if(pointQty > m_elementMaxPointCount)
            m_elementMaxPointCount = pointQty;
        }

    size_t              GetPointCapacity          () const
        {
        if(m_pointCount <= m_maxPointQty)
            return m_pointCount;

        if(m_elementMaxPointCount <= m_maxPointQty)
            return m_maxPointQty;

        return m_elementMaxPointCount;
        }
    };
    
 struct ElementStats
    {
    ElementPointStats   m_point;
    ElementLinearStats  m_linear;
    ElementMeshStats    m_mesh;

    explicit            ElementStats           ()
        {
        }
    };