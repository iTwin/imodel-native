/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshSourceCollection.h $
|    $RCSfile: IScalableMeshSourceCollection.h,v $
|   $Revision: 1.14 $
|       $Date: 2011/10/26 17:55:51 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <TerrainModel/TerrainModel.h>
#include <ScalableMesh/Foundations/Iterator.h>
#include <ScalableMesh/ScalableMeshDefs.h>
#include <ScalableMesh\Import\ScalableMeshData.h>
BEGIN_BENTLEY_SCALABLEMESH_NAMESPACE

struct IDTMSource;
struct IDTMSourceGroup;
struct EditListener;

typedef RefCountedPtr<IDTMSource>           IDTMSourcePtr;
typedef RefCountedPtr<IDTMSourceGroup>      IDTMSourceGroupPtr;


/*---------------------------------------------------------------------------------**//**
* @description    
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct IDTMSourceCollection 
    {
    private:
        struct Impl;
        std::auto_ptr<Impl>                 m_implP;

        /*---------------------------------------------------------------------------------**//**
        * @description    
        * @bsiclass                                                  Raymond.Gauthier   03/2011
        +---------------+---------------+---------------+---------------+---------------+------*/
        struct IteratorBase : public Foundations::Iterator::Policy<IDTMSourceCollection, IDTMSource>
            {
        private:
            friend struct                       IDTMSourceCollection;

            struct                              Impl;
            std::auto_ptr<Impl>                 m_implP;
            explicit                            IteratorBase           (Impl*                       implP);

        public:
            BENTLEY_SM_EXPORT explicit                IteratorBase           ();
            BENTLEY_SM_EXPORT                         ~IteratorBase          ();

            BENTLEY_SM_EXPORT                         IteratorBase           (const IteratorBase&         rhs);
            BENTLEY_SM_EXPORT IteratorBase&           operator=              (const IteratorBase&         rhs);

            BENTLEY_SM_EXPORT const_reference         Dereference            () const;            
            BENTLEY_SM_EXPORT reference               Dereference            ();

            BENTLEY_SM_EXPORT void                    Increment              ();
            BENTLEY_SM_EXPORT void                    Decrement              ();
            BENTLEY_SM_EXPORT bool                    EqualTo                (const IteratorBase&         rhs) const;
            };


        BENTLEY_SM_EXPORT IteratorBase                _Begin                 () const;
        BENTLEY_SM_EXPORT IteratorBase                _End                   () const;

        BENTLEY_SM_EXPORT IteratorBase                _BeginEdit             ();
        BENTLEY_SM_EXPORT IteratorBase                _EndEdit               ();

        BENTLEY_SM_EXPORT IteratorBase                _Release               (const IteratorBase&         sourceIt,
                                                                        IDTMSourcePtr&          sourcePtr);


        BENTLEY_SM_EXPORT IteratorBase                _GetIterFor            (const IDTMSource&       source) const;
        BENTLEY_SM_EXPORT IteratorBase                _EditIterFor           (const IDTMSource&       source);

        BENTLEY_SM_EXPORT IteratorBase                _GetIterAt             (uint32_t                        index) const;
        BENTLEY_SM_EXPORT IteratorBase                _EditIterAt            (uint32_t                        index);

        BENTLEY_SM_EXPORT IteratorBase                _Remove                (const IteratorBase&         sourceIt);  

        BENTLEY_SM_EXPORT Import::UpToDateState     _GetUpToDateState(const IteratorBase& sourceIt);
        // NEEDS_WORK_SM : move this in IScalableMeshSource.
        BENTLEY_SM_EXPORT Import::ScalableMeshData&    _GetScalableMeshData(const IteratorBase& sourceIt);
        BENTLEY_SM_EXPORT void                        _SetScalableMeshData(IteratorBase& sourceIt, Import::ScalableMeshData data);

        BENTLEY_SM_EXPORT IteratorBase              _SetRemoveState(const IteratorBase& sourceIt);

        BENTLEY_SM_EXPORT IteratorBase                _UnGroup               (const IteratorBase&         groupSourceIt);

        BENTLEY_SM_EXPORT IteratorBase                _MoveDown              (const IteratorBase&         sourceIt);
        BENTLEY_SM_EXPORT IteratorBase                _MoveToBottom          (const IteratorBase&         sourceIt);                
        BENTLEY_SM_EXPORT IteratorBase                _MoveToPos             (const IteratorBase&         sourceIt, 
                                                                        const IteratorBase&         destinationIt);        
        BENTLEY_SM_EXPORT IteratorBase                _MoveToTop             (const IteratorBase&         sourceIt);
        BENTLEY_SM_EXPORT IteratorBase                _MoveUp                (const IteratorBase&         sourceIt);    
        BENTLEY_SM_EXPORT void                        _Clear                 ();


    public:
        typedef Foundations::Iterator::ConstBidirectional<IteratorBase>
                                                const_iterator;
        typedef Foundations::Iterator::Bidirectional<IteratorBase> 
                                                iterator;

        typedef std::reverse_iterator<const_iterator>
                                                const_reverse_iterator;
        typedef std::reverse_iterator<iterator>
                                                reverse_iterator;


/*__PUBLISH_SECTION_END__*/
    private :
        
        void                                    _OnPublicEdit          ();

        IteratorBase                            _BeginEditInternal     ();
        IteratorBase                            _EndEditInternal       ();

    public:
        explicit                                IDTMSourceCollection   ();

        void                                    RegisterEditListener   (EditListener&               listener);
        void                                    UnregisterEditListener (const EditListener&         listener);

        // TDORAY: Try remove need for these
        // Internal accessors. 
        iterator                                BeginEditInternal      ();
        iterator                                EndEditInternal        ();
        void                                    AddInternal            (const IDTMSourcePtr&    sourcePtr);

/*__PUBLISH_SECTION_START__*/

    public : 
        // Not meant to be used polymorphically
        BENTLEY_SM_EXPORT                             ~IDTMSourceCollection  ();

        BENTLEY_SM_EXPORT                             IDTMSourceCollection   (const IDTMSourceCollection& rhs);
        BENTLEY_SM_EXPORT IDTMSourceCollection&       operator=              (const IDTMSourceCollection& rhs);



        BENTLEY_SM_EXPORT const IDTMSource&           GetAt                  (uint32_t                        index) const;
        BENTLEY_SM_EXPORT IDTMSource&                 EditAt                 (uint32_t                        index);

        BENTLEY_SM_EXPORT uint32_t                        GetCount               () const;

        const_iterator                          Begin                  () const;
        const_iterator                          End                    () const;

        iterator                                BeginEdit              ();
        iterator                                EndEdit                ();

        const_reverse_iterator                  rBegin                 () const;
        const_reverse_iterator                  rEnd                   () const;

        reverse_iterator                        rBeginEdit             ();
        reverse_iterator                        rEndEdit               ();

        iterator                                Release                (const const_iterator&       sourceIt,
                                                                        IDTMSourcePtr&          sourcePtr);

        const_iterator                          GetIterFor             (const IDTMSource&       source) const;
        iterator                                EditIterFor            (const IDTMSource&       source);

        const_iterator                          GetIterAt              (uint32_t                        index) const;
        iterator                                EditIterAt             (uint32_t                        index);


        BENTLEY_SM_EXPORT StatusInt                   Add                    (const IDTMSourcePtr&    sourcePtr); 
        iterator                                Remove                 (const const_iterator&       sourceIt);  
        Import::UpToDateState                   GetUpToDateState       (const const_iterator&       sourceIt);        
        iterator                                SetRemoveState          (const const_iterator&      sourceIt);

        iterator                                UnGroup                (const const_iterator&       groupSourceIt);

        iterator                                MoveDown               (const const_iterator&       sourceIt);
        iterator                                MoveToBottom           (const const_iterator&       sourceIt);                
        iterator                                MoveToPos              (const const_iterator&       sourceIt, 
                                                                        const const_iterator&       destinationIt);        

        iterator                                MoveToTop              (const const_iterator&       sourceIt);
        iterator                                MoveUp                 (const const_iterator&       sourceIt);     
        void                                    Clear                  ();
    };

#include <ScalableMesh/IScalableMeshSourceCollection.hpp>

END_BENTLEY_SCALABLEMESH_NAMESPACE
