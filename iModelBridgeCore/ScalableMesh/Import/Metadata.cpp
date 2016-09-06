/*--------------------------------------------------------------------------------------+
|
|     $Source: Import/Metadata.cpp $
|    $RCSfile: Metadata.cpp,v $
|   $Revision: 1.8 $
|       $Date: 2011/10/21 17:32:19 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ScalableMeshPCH.h>

#include <ScalableMesh/Import/Metadata.h>
#include <STMInternal/Foundations/PrivateStringTools.h>

BEGIN_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct InternalFieldHandler
    {

    template <typename T>
    static const T&                 GenericAs                          (const Field&                field)
        {
        return dynamic_cast<const T&>(*field.m_baseP);
        }

    static void                     Accept                             (const Field&                field,
                                                                        const WChar*             fieldName,
                                                                        IMetadataVisitor&         visitor)
        {
        field.m_baseP->_Accept(fieldName, visitor);
        }

    };


namespace {
typedef InternalFieldHandler FieldHandler;
}



/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct MetadataEntry::Impl
    {
    WString                  m_name;
    Field                       m_field;

    explicit                    Impl                       (const WChar*     name,
                                                            const Field&        field)
        :   m_name(name),
            m_field(field)
        {
        }
    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MetadataEntry::MetadataEntry     (const WChar*  name,
                        const Field&    field)
    :   m_implP(new Impl(name, field))
    {
    
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MetadataEntry::~MetadataEntry ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MetadataEntry::MetadataEntry (const MetadataEntry& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MetadataEntry& MetadataEntry::operator= (const MetadataEntry& rhs)
    {
    *m_implP = *rhs.m_implP;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& MetadataEntry::GetName () const
    {
    return m_implP->m_name;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* MetadataEntry::GetNameCStr () const
    {
    return m_implP->m_name.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const Field& MetadataEntry::GetField () const
    {
    return m_implP->m_field;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void MetadataEntry::Accept (IMetadataVisitor& visitor) const
    {
    FieldHandler::Accept(m_implP->m_field, m_implP->m_name.c_str(), visitor);
    }


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct MetadataRecord::Impl
    {
    typedef bvector<MetadataEntry>              EntryList;
    EntryList                                   m_entries;

    const_iterator                              FindFor                        (const WChar*     entryName) const
        {
        // TDORAY: Use binary search.

        struct HasName
            {
            const WChar* m_name;
            explicit HasName (const WChar* name) : m_name(name) {}

            bool operator () (const MetadataEntry& entry) const
                {
                return 0 == wcscmp(entry.GetNameCStr(), m_name);
                }
            };

        return &*std::find_if(m_entries.begin(), m_entries.end(), HasName(entryName));
        }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MetadataRecord::MetadataRecord ()
    :   m_implP(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MetadataRecord::~MetadataRecord ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MetadataRecord::MetadataRecord (const MetadataRecord& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MetadataRecord& MetadataRecord::operator= (const MetadataRecord& rhs)
    {
    *m_implP = *rhs.m_implP;
    return *this;
    }   

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool MetadataRecord::IsEmpty () const
    {
    return m_implP->m_entries.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MetadataRecord::const_iterator MetadataRecord::begin () const
    {
    return &*m_implP->m_entries.begin();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MetadataRecord::const_iterator MetadataRecord::end () const
    {
    return &*m_implP->m_entries.end();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
MetadataRecord::const_iterator MetadataRecord::FindFor (const WChar* entryName) const
    {
    return m_implP->FindFor(entryName);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void MetadataRecord::push_back (const MetadataEntry& entry)
    {
    m_implP->m_entries.push_back(entry);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void MetadataRecord::Accept (IMetadataVisitor& visitor) const
    {
    struct AcceptVisitor
        {
        IMetadataVisitor& m_visitor;

        explicit AcceptVisitor (IMetadataVisitor& visitor) : m_visitor(visitor) {}
        void operator () (const MetadataEntry& entry) const { entry.Accept(m_visitor); }
        };

    std::for_each(m_implP->m_entries.begin(), m_implP->m_entries.end(), AcceptVisitor(visitor));
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Field::Field (const FieldBase& field)
    :   m_baseP(field._Clone())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Field::~Field ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Field::Field (const Field& rhs)
    :   m_baseP(rhs.m_baseP->_Clone())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Field& Field::operator= (const Field& rhs)
    {
    m_baseP = rhs.m_baseP->_Clone();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void Field::Accept (IFieldVisitor& visitor) const
    {
    m_baseP->_Accept(visitor);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const FieldComposed& AsComposed (const Field& field)
    {
    return FieldHandler::GenericAs<FieldComposed>(field);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const FieldString& AsString (const Field& field)
    {
    return FieldHandler::GenericAs<FieldString>(field);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const FieldDouble& AsDouble (const Field& field)
    {
    return FieldHandler::GenericAs<FieldDouble>(field);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const FieldDoubleArray& AsDoubleArray (const Field& field)
    {
    return FieldHandler::GenericAs<FieldDoubleArray>(field);
    }


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldBase::FieldBase ()
    {
    // Nothing to do
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldBase::~FieldBase ()
    {
    // Nothing to do
    }
    
/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldBase::FieldBase (const FieldBase& rhs)
    {
    // Nothing to do
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename FieldT>
FieldMixinBase<FieldT>::FieldMixinBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename FieldT>
FieldMixinBase<FieldT>::~FieldMixinBase ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename FieldT>
FieldMixinBase<FieldT>::FieldMixinBase (const FieldMixinBase& rhs) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename FieldT>
FieldBase* FieldMixinBase<FieldT>::_Clone () const
    {
    return new FieldT(static_cast<const FieldT&>(*this));
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename FieldT>
void FieldMixinBase<FieldT>::_Accept (IFieldVisitor& visitor) const
    {
    return visitor._Visit(static_cast<const FieldT&>(*this));
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename FieldT>
void FieldMixinBase<FieldT>::_Accept (const WChar*     fieldName,
                                      IMetadataVisitor& visitor) const
    {
    return visitor._Visit(fieldName, static_cast<const FieldT&>(*this));
    }



/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FieldComposed::Impl
    {
    bvector<Field>              m_fields;
    };

         
/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldComposed::FieldComposed (const FieldComposed& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldComposed::FieldComposed ()
    :   m_implP(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldComposed::~FieldComposed ()
    {
    }
    
/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t FieldComposed::GetSize () const
    {
    return (uint32_t) m_implP->m_fields.size();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldComposed::const_iterator FieldComposed::begin  () const
    {
    return &*m_implP->m_fields.begin();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldComposed::const_iterator FieldComposed::end () const
    {
    return &*m_implP->m_fields.end();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void FieldComposed::push_back (const Field& rhs)
    {
    m_implP->m_fields.push_back(rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const Field& FieldComposed::operator [] (uint32_t index) const
    {
    assert(index < m_implP->m_fields.size());
    return m_implP->m_fields[index];
    }

/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FieldString::Impl
    {
    WString                  m_value;

    explicit                    Impl                       (const WChar*         string)
        :   m_value(string)
        {

        }

    };

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldString::FieldString (const FieldString& rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldString::FieldString (const WChar* value)
    :   m_implP(new Impl(value))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldString::~FieldString ()
    {
    }
    
/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WString& FieldString::Get () const
    {
    return m_implP->m_value;
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const WChar* FieldString::GetCStr () const
    {
    return m_implP->m_value.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldDouble::FieldDouble (const FieldDouble& rhs)
    :   m_value(rhs.m_value)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldDouble::FieldDouble (double value)
    :   m_value(value)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
double FieldDouble::Get () const
    {
    return m_value;
    }


/*---------------------------------------------------------------------------------**//**
* @description   
* @bsiclass                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct FieldDoubleArray::Impl
    {
    bvector<double>             m_values;

    explicit                    Impl                       ()
        {
        }

    explicit                    Impl                       (const_iterator  begin,
                                                            const_iterator  end)
        :   m_values(begin, end)
        {
        }

    };


/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldDoubleArray::FieldDoubleArray  (const FieldDoubleArray&  rhs)
    :   m_implP(new Impl(*rhs.m_implP))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldDoubleArray::FieldDoubleArray  ()
    :   m_implP(new Impl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldDoubleArray::FieldDoubleArray  (const_iterator  begin,
                                            const_iterator  end)
    :   m_implP(new Impl(begin, end))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldDoubleArray::~FieldDoubleArray ()
    {
    }
    
/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t FieldDoubleArray::GetSize () const
    {
    return (uint32_t) m_implP->m_values.size();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldDoubleArray::const_iterator FieldDoubleArray::begin () const
    {
    return &*m_implP->m_values.begin();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
FieldDoubleArray::const_iterator FieldDoubleArray::end () const
    {
    return &*m_implP->m_values.end();
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
const double& FieldDoubleArray::operator [] (uint32_t index) const
    {
    assert(index < m_implP->m_values.size());
    return m_implP->m_values[index];
    }

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsimethod                                                  Raymond.Gauthier   04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void FieldDoubleArray::push_back (double rhs)
    {
    m_implP->m_values.push_back(rhs);
    }

END_BENTLEY_SCALABLEMESH_IMPORT_NAMESPACE
