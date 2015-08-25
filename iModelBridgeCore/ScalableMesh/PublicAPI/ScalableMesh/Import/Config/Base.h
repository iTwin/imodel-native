/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Config/Base.h $
|    $RCSfile: Base.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/07/07 16:13:55 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/Import/Definitions.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct IImportConfigVisitor;


/*---------------------------------------------------------------------------------**//**
* @description  This is the base class that a specific import config must implement. 
*
*               The private _Accept method must be implemented in such a way so that
*               it calls the visitor's Visit method with itself(*this) when it decides
*               to accept the visitor (which is almost always in our case). New 
*               configuration implementer has to add a new Visit method overload to the 
*               ImportConfigVisitor's interface that takes his new config class as a
*               parameter. If not done so, ImportConfig overload will be automatically
*               called.
*               
*               
* @see ImportConfig
*    
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportConfigComponentBase : public ShareableObjectTypeTrait<ImportConfigComponentBase>::type
    {
protected:
    typedef const std::type_info*       ClassID;
private:
    friend struct                       ImportConfigComponent;

    void*                               m_implP; // Reserved some space for further use

    // Disabled as clone should not need it
    ImportConfigComponentBase&          operator=                      (const ImportConfigComponentBase&);

    virtual ClassID                     _GetClassID                    () const = 0;
    virtual ImportConfigComponentBase*  _Clone                         () const = 0;
    virtual void                        _Accept                        (IImportConfigVisitor&                   visitor) const = 0;

protected:
    IMPORT_DLLE explicit                ImportConfigComponentBase      ();
    IMPORT_DLLE explicit                ImportConfigComponentBase      (const ImportConfigComponentBase&        rhs);
public:
    IMPORT_DLLE virtual                 ~ImportConfigComponentBase     () = 0;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
struct ImportConfigComponentMixinBase : public ImportConfigComponentBase
    {
private:
    struct UniqueTokenType {};

    virtual ClassID                     _GetClassID                    () const override;
    virtual ImportConfigComponentBase*  _Clone                         () const override;
    virtual void                        _Accept                        (IImportConfigVisitor&                   visitor) const override;

    // Disabled as clone should not need it
    ImportConfigComponentMixinBase&     operator=                      (const ImportConfigComponentMixinBase&);

protected:
    typedef ImportConfigComponentMixinBase<ComponentT>           
                                        super_class;

    explicit                            ImportConfigComponentMixinBase ();
    virtual                             ~ImportConfigComponentMixinBase() = 0;

                                        ImportConfigComponentMixinBase (const ImportConfigComponentMixinBase&   rhs);

    static ClassID                      s_GetClassID                   ()
        {
        static ClassID CLASS_ID = &typeid(UniqueTokenType());
        return CLASS_ID;
        }
    };



END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
