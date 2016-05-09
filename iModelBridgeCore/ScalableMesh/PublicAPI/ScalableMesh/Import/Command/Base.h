/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Command/Base.h $
|    $RCSfile: Base.h,v $
|   $Revision: 1.4 $
|       $Date: 2011/07/07 16:14:12 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ScalableMesh/Import/Definitions.h>
#include <ScalableMesh/Import/DataTypeFamily.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
struct ImporterImpl;
namespace Internal
    {
    class Config;
    }
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

    struct UniqueTokenType {};

    void*                                   m_implP; // Reserved some space for further use

    uint32_t                                m_sourceLayer;
    DataTypeFamily                      m_sourceType;
    uint32_t                                m_targetLayer;
    DataTypeFamily                      m_targetType;

    bool                                m_sourceLayerSet;
    bool                      m_sourceTypeSet;
    bool                                m_targetLayerSet;
    bool                      m_targetTypeSet;

    // Disabled as clone should not need it
    ImportCommandBase&                      operator=                          (const ImportCommandBase&);

    virtual ClassID                         _GetClassID() const
        {
        static ClassID CLASS_ID = &typeid(UniqueTokenType());
        return CLASS_ID;
        }
    virtual ImportCommandBase*              _Clone() const { return new ImportCommandBase(*this); };
    virtual void                            _Accept(IImportSequenceVisitor&             visitor)const {};

    public:
    IMPORT_DLLE                     ImportCommandBase                  ();
    IMPORT_DLLE                     ImportCommandBase                  (const ImportCommandBase&            rhs);
    IMPORT_DLLE                     ImportCommandBase(const ImportCommand&            cmd);


    IMPORT_DLLE virtual                     ~ImportCommandBase() ;

    virtual void                           _Execute(ImporterImpl&       impl,
                                                    const Internal::Config&       config)
        {}
    void                                    SetSourceLayer(uint32_t layer)
        {
        m_sourceLayer = layer;
        m_sourceLayerSet = true;
        }

    void                                    SetTargetLayer(uint32_t layer)
        {
        m_targetLayer = layer;
        m_targetLayerSet = true;
        }

    void                                    SetSourceType(const DataTypeFamily& type)
        {
        m_sourceType = type;
        m_sourceTypeSet = true;
        }

    void                                    SetTargetType(const DataTypeFamily& type)
        {
        m_targetType = type;
        m_targetTypeSet = true;
        }
    };

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
