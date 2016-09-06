#pragma once

#include "ElementStats.h"    


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
    static const ElementPointExtractor&     GetFor(DgnV8Api::MSElementDescrCP            elmDescP);

    virtual void                            ComputeStats(DgnV8Api::MSElementDescrCP            elmDescP,
                                                                        ElementPointStats&             stats) const = 0;

    virtual StatusInt                       Scan(DgnV8Api::MSElementDescrCP            elmDescP,
                                                                       bvector<DPoint3d>&       pointArray) const = 0;
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
    static const ElementLinearExtractor&    GetFor(DgnV8Api::MSElementDescrCP            elmDescP);

    virtual void                            ComputeStats(DgnV8Api::MSElementDescrCP            elmDescP,
                                                                        ElementLinearStats&            stats) const = 0;

    virtual StatusInt                       Scan(DgnV8Api::MSElementDescrCP            elmDescP,
                                                                        bvector<bvector<DPoint3d>>& linerarArray,
                                                                        bvector<unsigned int>& types,
                                                                        unsigned int featureType,
                                                                        size_t capacity) const = 0;
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
    static const ElementMeshExtractor&      GetFor(DgnV8Api::MSElementDescrCP            elmDescP);

    virtual void                            ComputeStats(DgnV8Api::MSElementDescrCP            elmDescP,
                                                                        ElementMeshStats&              stats) const = 0;

    virtual StatusInt                       Scan(DgnV8Api::MSElementDescrCP            elmDescP,
                                                                        bvector<bvector<DPoint3d>>& linerarArray,
                                                                        bvector<unsigned int>& types,
                                                                        size_t capacity) const = 0;
    };