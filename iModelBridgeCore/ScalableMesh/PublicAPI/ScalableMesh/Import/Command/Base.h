/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Command/Base.h $
|    $RCSfile: Base.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/07/07 16:14:12 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/Import/Definitions.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

struct IImportSequenceVisitor;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   7/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct ImportCommandBase : public ShareableObjectTypeTrait<ImportCommandBase>::type
    {
protected:
    typedef const std::type_info*           ClassID;
private:
    friend struct                           ImportCommand;

    void*                                   m_implP; // Reserved some space for further use

    // Disabled as clone should not need it
    ImportCommandBase&                      operator=                          (const ImportCommandBase&);

    virtual ClassID                         _GetClassID                        () const = 0;
    virtual ImportCommandBase*              _Clone                             () const = 0;
    virtual void                            _Accept                            (IImportSequenceVisitor&             visitor) const = 0;

protected:
    IMPORT_DLLE explicit                    ImportCommandBase                  ();
    IMPORT_DLLE explicit                    ImportCommandBase                  (const ImportCommandBase&            rhs);

public:
    IMPORT_DLLE virtual                     ~ImportCommandBase                 () = 0;
    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename ComponentT>
struct ImportCommandMixinBase : public ImportCommandBase
    {
private:
    struct UniqueTokenType {};

    virtual ClassID                         _GetClassID                        () const override;
    virtual ImportCommandBase*              _Clone                             () const override;
    virtual void                            _Accept                            (IImportSequenceVisitor&             visitor) const override;

    // Disabled as clone should not need it
    ImportCommandMixinBase&                 operator=                          (const ImportCommandMixinBase&);

protected:
    typedef ImportCommandMixinBase<ComponentT>           
                                            super_class;

    explicit                                ImportCommandMixinBase             ();
    virtual                                 ~ImportCommandMixinBase            () = 0;

                                            ImportCommandMixinBase             (const ImportCommandMixinBase&       rhs);

    static ClassID                          s_GetClassID                       ()
        {
        static ClassID CLASS_ID = &typeid(UniqueTokenType());
        return CLASS_ID;
        }

    };



END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
