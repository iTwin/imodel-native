/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/Import/Config/Content/Base.h $
|    $RCSfile: Base.h,v $
|   $Revision: 1.6 $
|       $Date: 2011/07/18 14:03:13 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ScalableTerrainModel/Import/Definitions.h>

BEGIN_BENTLEY_MRDTM_IMPORT_NAMESPACE

struct ILayerConfigVisitor;
struct IContentConfigVisitor;


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentConfigComponentBase : public ShareableObjectTypeTrait<ContentConfigComponentBase>::type
    {
protected:
    typedef const std::type_info*           ClassID;
private:
    friend struct                           ContentConfigComponent;

    void*                                   m_implP; // Reserved some space for further use

    // Disabled as clone should not need it
    ContentConfigComponentBase&             operator=                          (const ContentConfigComponentBase&);

    virtual ClassID                         _GetClassID                        () const = 0;
    virtual ContentConfigComponentBase*     _Clone                             () const = 0;
    virtual void                            _Accept                            (IContentConfigVisitor&                      visitor) const = 0;
    virtual void                            _Accept                            (ILayerConfigVisitor&                        visitor) const = 0;


protected:
    IMPORT_DLLE explicit                    ContentConfigComponentBase         ();
    IMPORT_DLLE explicit                    ContentConfigComponentBase         (const ContentConfigComponentBase&           rhs);

public:
    IMPORT_DLLE virtual                     ~ContentConfigComponentBase        () = 0;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
struct ContentConfigComponentMixinBase : public ContentConfigComponentBase
    {
private:
    struct UniqueTokenType {};

    virtual ClassID                         _GetClassID                        () const override;
    virtual ContentConfigComponentBase*     _Clone                             () const override;
    virtual void                            _Accept                            (IContentConfigVisitor&                      visitor) const;
    virtual void                            _Accept                            (ILayerConfigVisitor&                        visitor) const;

    // Disabled as clone should not need it
    ContentConfigComponentMixinBase&        operator=                          (const ContentConfigComponentMixinBase&);

protected:
    typedef ContentConfigComponentMixinBase<ComponentT>           
                                            super_class;

    explicit                                ContentConfigComponentMixinBase    ();
    virtual                                 ~ContentConfigComponentMixinBase   () = 0;

                                            ContentConfigComponentMixinBase    (const ContentConfigComponentMixinBase&      rhs);

    static ClassID                          s_GetClassID                       ()
        {
        static ClassID CLASS_ID = &typeid(UniqueTokenType());
        return CLASS_ID;
        }

    };



END_BENTLEY_MRDTM_IMPORT_NAMESPACE