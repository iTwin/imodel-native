/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/Import/Metadata.h $
|    $RCSfile: Metadata.h,v $
|   $Revision: 1.9 $
|       $Date: 2011/10/21 17:32:09 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <ScalableMesh/Import/Definitions.h>


BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE


struct Field;
struct FieldBase;

struct FieldComposed;
struct FieldString;
struct FieldDouble;
struct FieldDoubleArray;

struct IFieldVisitor;
struct IMetadataVisitor;

/*---------------------------------------------------------------------------------**//**
* @description   
*
* NOTE: Not designed to be a base class
* TDORAY: Optimize with COW.
*
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct MetadataEntry
    {
private:
    struct                              Impl;
    std::auto_ptr<Impl>                 m_implP;

public:
    IMPORT_DLLE explicit                MetadataEntry                          (const WChar*              name,
                                                                                const Field&                field);

    IMPORT_DLLE                         ~MetadataEntry                         ();

    IMPORT_DLLE                         MetadataEntry                          (const MetadataEntry&        rhs);
    IMPORT_DLLE MetadataEntry&          operator=                              (const MetadataEntry&        rhs);

    IMPORT_DLLE const WString&       GetName                                () const;
    IMPORT_DLLE const WChar*         GetNameCStr                            () const;

    IMPORT_DLLE const Field&            GetField                               () const;

    IMPORT_DLLE void                    Accept                                 (IMetadataVisitor&           visitor) const;
    };


/*---------------------------------------------------------------------------------**//**
* @description    
* TDORAY: Optimize with COW.
* @bsiclass                                                  Raymond.Gauthier   04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct MetadataRecord
    {
private:
    struct                              Impl;
    std::auto_ptr<Impl>                 m_implP;

public:
    typedef MetadataEntry               value_type;
    typedef value_type&                 reference;
    typedef const value_type&           const_reference;

    typedef const value_type*           const_iterator;

    IMPORT_DLLE explicit                MetadataRecord                         ();
    IMPORT_DLLE                         ~MetadataRecord                        ();

    IMPORT_DLLE                         MetadataRecord                         (const MetadataRecord&       rhs);
    IMPORT_DLLE MetadataRecord&         operator=                              (const MetadataRecord&       rhs);

    IMPORT_DLLE bool                    IsEmpty                                () const;
    
    IMPORT_DLLE const_iterator          begin                                  () const;
    IMPORT_DLLE const_iterator          end                                    () const;
    
    IMPORT_DLLE const_iterator          FindFor                                (const WChar*              entryName) const;

    IMPORT_DLLE void                    push_back                              (const MetadataEntry&        entry);

    IMPORT_DLLE void                    Accept                                 (IMetadataVisitor&           visitor) const;
    };


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct Field
    {
private:
    friend struct                       InternalFieldHandler;

    SharedPtrTypeTrait<FieldBase>::type m_baseP;
public:
    // Implicit constructor
    IMPORT_DLLE                         Field                                  (const FieldBase&            field);


    IMPORT_DLLE                         ~Field                                 ();

    IMPORT_DLLE                         Field                                  (const Field&                rhs);
    IMPORT_DLLE Field&                  operator=                              (const Field&                rhs);

    IMPORT_DLLE void                    Accept                                 (IFieldVisitor&              visitor) const;
    };


IMPORT_DLLE const FieldComposed&        AsComposed                             (const Field&                field);
IMPORT_DLLE const FieldString&          AsString                               (const Field&                field);
IMPORT_DLLE const FieldDouble&          AsDouble                               (const Field&                field);
IMPORT_DLLE const FieldDoubleArray&     AsDoubleArray                          (const Field&                field);

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IMetadataVisitor
    {
    virtual                             ~IMetadataVisitor                      () = 0 {}


    virtual void                        _Visit                                 (const WChar*             name,
                                                                                const FieldBase&            field) { /* Do nothing */ }

    virtual void                        _Visit                                 (const WChar*             name,
                                                                                const FieldComposed&        field) = 0;
    virtual void                        _Visit                                 (const WChar*             name,
                                                                                const FieldString&          field) = 0;
    virtual void                        _Visit                                 (const WChar*             name,
                                                                                const FieldDouble&          field) = 0;
    virtual void                        _Visit                                 (const WChar*             name,
                                                                                const FieldDoubleArray&     field) = 0;
    };


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IFieldVisitor
    {
    virtual                             ~IFieldVisitor                         () = 0 {}

    virtual void                        _Visit                                 (const FieldBase&            field) { /* Do nothing */ }

    virtual void                        _Visit                                 (const FieldComposed&        field) = 0;
    virtual void                        _Visit                                 (const FieldString&          field) = 0;
    virtual void                        _Visit                                 (const FieldDouble&          field) = 0;
    virtual void                        _Visit                                 (const FieldDoubleArray&     field) = 0;


    };


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FieldBase : public ShareableObjectTypeTrait<FieldBase>::type
    {
private:
    friend struct                       InternalFieldHandler;
    friend struct                       Field;

    virtual FieldBase*                  _Clone                                 () const = 0;
    virtual void                        _Accept                                (IFieldVisitor&              visitor) const = 0;                        
    virtual void                        _Accept                                (const WChar*             fieldName,
                                                                                IMetadataVisitor&           visitor) const = 0; 

    // Disabled as clone should not need it
    FieldBase&                          operator=                              (const FieldBase&);

protected:
    explicit                            FieldBase                              ();
    virtual                             ~FieldBase                             () = 0;
    
                                        FieldBase                              (const FieldBase&            rhs);
    };


/*---------------------------------------------------------------------------------**//**
* @description  Mix-in base that automatically implements simple case of Clone and 
*               Accept behaviors.
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename FieldT>
struct FieldMixinBase : FieldBase
    {
private:
    virtual FieldBase*                  _Clone                                 () const override;
    virtual void                        _Accept                                (IFieldVisitor&              visitor) const override;
    virtual void                        _Accept                                (const WChar*             fieldName,
                                                                                IMetadataVisitor&           visitor) const override; 
    // Disabled as clone should not need it
    FieldMixinBase&                     operator=                              (const FieldMixinBase&);

protected:
    typedef FieldMixinBase<FieldT>      super_class;

    explicit                            FieldMixinBase                         ();
    virtual                             ~FieldMixinBase                        () = 0;

                                        FieldMixinBase                         (const FieldMixinBase&       rhs);
    };



/*---------------------------------------------------------------------------------**//**
* @description  
* TDORAY: Optimize with COW.
* TDORAY: Make this one visitable
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FieldComposed : FieldMixinBase<FieldComposed>
    {
private:
    friend struct                       super_class;

    struct                              Impl;
    std::auto_ptr<Impl>                 m_implP;

                                        FieldComposed                          (const FieldComposed&        rhs);

public:
    typedef const Field*                const_iterator;

    IMPORT_DLLE explicit                FieldComposed                          ();
    IMPORT_DLLE virtual                 ~FieldComposed                         ();
    
    IMPORT_DLLE uint32_t                    GetSize                                () const;

    IMPORT_DLLE const_iterator          begin                                  () const;
    IMPORT_DLLE const_iterator          end                                    () const;

    IMPORT_DLLE const Field&            operator []                            (uint32_t                        index) const;

    IMPORT_DLLE void                    push_back                              (const Field&                rhs);
    };

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FieldString : FieldMixinBase<FieldString>
    {
private:
    friend struct                       super_class;

    struct                              Impl;
    std::auto_ptr<Impl>                 m_implP;

                                        FieldString                            (const FieldString&          rhs);

public:
    IMPORT_DLLE explicit                FieldString                            (const WChar*              value);

    IMPORT_DLLE virtual                 ~FieldString                           ();
    
    IMPORT_DLLE const WString&       Get                                    () const;
    IMPORT_DLLE const WChar*         GetCStr                                () const;
    };

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FieldDouble : FieldMixinBase<FieldDouble>
    {
private:
    friend struct                       super_class;

    double                              m_value;

                                        FieldDouble                            (const FieldDouble&          rhs);
public:
    IMPORT_DLLE explicit                FieldDouble                            (double                      value);
    
    // Use default copy constructor

    IMPORT_DLLE double                  Get                                    () const;
    };



/*---------------------------------------------------------------------------------**//**
* @description   
* TDORAY: Optimize with COW.
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FieldDoubleArray : FieldMixinBase<FieldDoubleArray>
    {
private:
    friend struct                       super_class;

    struct                              Impl;
    std::auto_ptr<Impl>                 m_implP;

                                        FieldDoubleArray                       (const FieldDoubleArray&     rhs);

public:
    typedef const double*               const_iterator;

    IMPORT_DLLE explicit                FieldDoubleArray                       ();
    IMPORT_DLLE explicit                FieldDoubleArray                       (const_iterator              begin,
                                                                                const_iterator              end);

    IMPORT_DLLE virtual                 ~FieldDoubleArray                      ();
    
    IMPORT_DLLE uint32_t                    GetSize                                () const;

    IMPORT_DLLE const_iterator          begin                                  () const;
    IMPORT_DLLE const_iterator          end                                    () const;

    IMPORT_DLLE const double&           operator []                            (uint32_t                        index) const;

    IMPORT_DLLE void                    push_back                              (double                      rhs);
    };



// TDORAY: Consider a metadata record field?

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
