//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFPacketIter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once


#include <ImagePP/h/HTraits.h>
#include <ImagePP/h/HIterators.h>

#include <STMInternal/Storage/HTGFFDirectory.h>

#include <STMInternal/Storage/HTGFFPacketIdIter.h>

namespace HTGFF {

template <typename EditorT>
class                   PacketIterManager;

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename BaseT>
class PacketEditorBase
    {
protected:
    typedef BaseT           base_type;                         
    typedef uint32_t        PacketID;
private:
    template <typename EditorT>
    friend class            PacketIter;

    friend class            PacketManagerBase;

    mutable PacketIdIter    m_idIter;
    mutable base_type*      m_pBase;

protected:  
    PacketID                GetPacketID                    () const {return *m_idIter;} 
    const base_type&        GetBase                        () const {return *m_pBase;}
    base_type&              GetBase                        () {return *m_pBase;}

    explicit                PacketEditorBase               () : m_idIter(), m_pBase(0) {}

    // Not meant to be used polymorphically
                            ~PacketEditorBase              () {}

    // Use default copy/assignment
    };

/*---------------------------------------------------------------------------------**//**
* @description
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
class PacketIter : public BentleyApi::ImagePP::BidirectionalIteratorWithAutoReverseConst<PacketIter<EditorT>,
                                                                            PacketIter<typename BentleyApi::ImagePP::ReverseConstTrait<EditorT>::type>,
                                                                            EditorT>
    {
public: 
    typedef BentleyApi::ImagePP::BidirectionalIteratorWithAutoReverseConst<PacketIter<EditorT>,
                                                                PacketIter<typename BentleyApi::ImagePP::ReverseConstTrait<EditorT>::type>,
                                                                EditorT>
                BidirectionalIteratorWithAutoReverseConst_Type;

    friend class            PacketIterManager<EditorT>;

    typedef typename BentleyApi::ImagePP::RemoveConst<typename EditorT::base_type>::type 
                            editor_base_type;

    typename BentleyApi::ImagePP::RemoveConst<EditorT>::type           
                            m_editor;

    typename BidirectionalIteratorWithAutoReverseConst_Type::const_iterator_t        ConvertToConst                 () const;

    typename BidirectionalIteratorWithAutoReverseConst_Type::const_reference         Dereference                    () const;
    typename BidirectionalIteratorWithAutoReverseConst_Type::reference               Dereference                    ();

    void                    Increment                      ();
    void                    Decrement                      ();

    bool                    EqualTo                        (const typename BidirectionalIteratorWithAutoReverseConst_Type::iterator_t&               pi_rRight) const;
    bool                    EqualTo                        (const typename BidirectionalIteratorWithAutoReverseConst_Type::rconst_iterator_t&        pi_rRight) const;

    explicit                PacketIter                     (editor_base_type&               pi_rEditorBase,
                                                            const PacketIdIter&             pi_rIDIter);

public:
    explicit                PacketIter                     ();


    static PacketIter<const EditorT>         
                            Create                         (const PacketIdIter&             pi_rIDIter,
                                                            const editor_base_type&         pi_rEditorBase);
    static PacketIter<EditorT>         
                            Create                         (const PacketIdIter&             pi_rIDIter,
                                                            editor_base_type&               pi_rEditorBase);

    // Use default copy/assignment
    };
                                                  




#include <STMInternal/Storage/HTGFFPacketIter.hpp>

} //End namespace HTGFF