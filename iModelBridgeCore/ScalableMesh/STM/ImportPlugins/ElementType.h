/*--------------------------------------------------------------------------------------+
|
|     $Source: STM/ImportPlugins/ElementType.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

        
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

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
struct ElementPointExtractor
    {
private:
    // Uncopyable
                                            ElementPointExtractor      (const ElementPointExtractor&);
    ElementPointExtractor&                  operator=                  (const ElementPointExtractor&);

protected:
    explicit                                ElementPointExtractor      () {}

    // Not virtual as ownership are initialized inside this module as statics
                                            ~ElementPointExtractor     () {}
public:
    static const ElementPointExtractor&     GetFor                     (MSElementDescrCP            elmDescP);

    virtual void                            ComputeStats               (MSElementDescrCP            elmDescP,
                                                                        ElementPointStats&             stats) const = 0;

    virtual StatusInt                       Scan                       (MSElementDescrCP            elmDescP,
                                                                        HPU::Array<DPoint3d>&       pointArray) const = 0;
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
struct ElementLinearExtractor
    {
private:
    // Uncopyable
                                            ElementLinearExtractor     (const ElementLinearExtractor&);
    ElementLinearExtractor&                 operator=                  (const ElementLinearExtractor&);

protected:
    explicit                                ElementLinearExtractor     () {}

    // Not virtual as ownership are initialized inside this module as statics
                                            ~ElementLinearExtractor    () {}
public:
    static const ElementLinearExtractor&    GetFor                     (MSElementDescrCP            elmDescP);

    virtual void                            ComputeStats               (MSElementDescrCP            elmDescP,
                                                                        ElementLinearStats&            stats) const = 0;

    virtual StatusInt                       Scan                       (MSElementDescrCP            elmDescP,
                                                                        IDTMFeatureArray<DPoint3d>& linerarArray) const = 0;
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

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                 Raymond.Gauthier   02/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ElementMeshExtractor
    {
private:
    // Uncopyable
                                            ElementMeshExtractor       (const ElementMeshExtractor&);
    ElementMeshExtractor&                   operator=                  (const ElementMeshExtractor&);

protected:
    explicit                                ElementMeshExtractor       () {}

    // Not virtual as ownership are initialized inside this module as statics
                                            ~ElementMeshExtractor      () {}
public:
    static const ElementMeshExtractor&      GetFor                     (MSElementDescrCP            elmDescP);

    virtual void                            ComputeStats               (MSElementDescrCP            elmDescP,
                                                                        ElementMeshStats&              stats) const = 0;

    virtual StatusInt                       Scan                       (MSElementDescrCP            elmDescP,
                                                                        IDTMFeatureArray<DPoint3d>& linerarArray) const = 0;
    };



END_BENTLEY_SCALABLEMESH_NAMESPACE
