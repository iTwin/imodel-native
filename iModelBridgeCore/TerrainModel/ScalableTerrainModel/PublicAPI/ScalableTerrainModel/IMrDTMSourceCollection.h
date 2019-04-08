/*--------------------------------------------------------------------------------------+
|
|     $Source: ScalableTerrainModel/PublicAPI/ScalableTerrainModel/IMrDTMSourceCollection.h $
|    $RCSfile: IMrDTMSourceCollection.h,v $
|   $Revision: 1.14 $
|       $Date: 2011/10/26 17:55:51 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/

#include <TerrainModel/TerrainModel.h>
#include <ScalableTerrainModel/Foundations/Iterator.h>
#include <ScalableTerrainModel/MrDTMDefs.h>

BEGIN_BENTLEY_MRDTM_NAMESPACE

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
            auto_ptr<Impl>                      m_implP;
            explicit                            IteratorBase           (Impl*                       implP);

        public:
            BENTLEYSTM_EXPORT explicit                IteratorBase           ();
            BENTLEYSTM_EXPORT                         ~IteratorBase          ();

            BENTLEYSTM_EXPORT                         IteratorBase           (const IteratorBase&         rhs);
            BENTLEYSTM_EXPORT IteratorBase&           operator=              (const IteratorBase&         rhs);

            BENTLEYSTM_EXPORT const_reference         Dereference            () const;            
            BENTLEYSTM_EXPORT reference               Dereference            ();

            BENTLEYSTM_EXPORT void                    Increment              ();
            BENTLEYSTM_EXPORT void                    Decrement              ();
            BENTLEYSTM_EXPORT bool                    EqualTo                (const IteratorBase&         rhs) const;
            };


        BENTLEYSTM_EXPORT IteratorBase                _Begin                 () const;
        BENTLEYSTM_EXPORT IteratorBase                _End                   () const;

        BENTLEYSTM_EXPORT IteratorBase                _BeginEdit             ();
        BENTLEYSTM_EXPORT IteratorBase                _EndEdit               ();

        BENTLEYSTM_EXPORT IteratorBase                _Release               (const IteratorBase&         sourceIt,
                                                                        IDTMSourcePtr&          sourcePtr);


        BENTLEYSTM_EXPORT IteratorBase                _GetIterFor            (const IDTMSource&       source) const;
        BENTLEYSTM_EXPORT IteratorBase                _EditIterFor           (const IDTMSource&       source);

        BENTLEYSTM_EXPORT IteratorBase                _GetIterAt             (UInt                        index) const;
        BENTLEYSTM_EXPORT IteratorBase                _EditIterAt            (UInt                        index);

        BENTLEYSTM_EXPORT IteratorBase                _Remove                (const IteratorBase&         sourceIt);  

        BENTLEYSTM_EXPORT IteratorBase                _UnGroup               (const IteratorBase&         groupSourceIt);

        BENTLEYSTM_EXPORT IteratorBase                _MoveDown              (const IteratorBase&         sourceIt);
        BENTLEYSTM_EXPORT IteratorBase                _MoveToBottom          (const IteratorBase&         sourceIt);                
        BENTLEYSTM_EXPORT IteratorBase                _MoveToPos             (const IteratorBase&         sourceIt, 
                                                                        const IteratorBase&         destinationIt);        
        BENTLEYSTM_EXPORT IteratorBase                _MoveToTop             (const IteratorBase&         sourceIt);
        BENTLEYSTM_EXPORT IteratorBase                _MoveUp                (const IteratorBase&         sourceIt);    


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
        BENTLEYSTM_EXPORT                             ~IDTMSourceCollection  ();

        BENTLEYSTM_EXPORT                             IDTMSourceCollection   (const IDTMSourceCollection& rhs);
        BENTLEYSTM_EXPORT IDTMSourceCollection&       operator=              (const IDTMSourceCollection& rhs);



        BENTLEYSTM_EXPORT const IDTMSource&           GetAt                  (UInt                        index) const;
        BENTLEYSTM_EXPORT IDTMSource&                 EditAt                 (UInt                        index);

        BENTLEYSTM_EXPORT UInt                        GetCount               () const;

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

        const_iterator                          GetIterAt              (UInt                        index) const;
        iterator                                EditIterAt             (UInt                        index);


        BENTLEYSTM_EXPORT StatusInt                   Add                    (const IDTMSourcePtr&    sourcePtr); 
        iterator                                Remove                 (const const_iterator&       sourceIt);  

        iterator                                UnGroup                (const const_iterator&       groupSourceIt);

        iterator                                MoveDown               (const const_iterator&       sourceIt);
        iterator                                MoveToBottom           (const const_iterator&       sourceIt);                
        iterator                                MoveToPos              (const const_iterator&       sourceIt, 
                                                                        const const_iterator&       destinationIt);        

        iterator                                MoveToTop              (const const_iterator&       sourceIt);
        iterator                                MoveUp                 (const const_iterator&       sourceIt);     
    };

#include <ScalableTerrainModel/IMrDTMSourceCollection.hpp>

END_BENTLEY_MRDTM_NAMESPACE