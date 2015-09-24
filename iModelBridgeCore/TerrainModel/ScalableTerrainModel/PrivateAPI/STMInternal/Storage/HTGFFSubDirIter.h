//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFSubDirIter.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once

#include <STMInternal/Storage/HTGFFSubDirManager.h> // NTERAY: Try to uncouple from this? If SubDirManagerBase came as a separate header, it could be done.

#include <STMInternal/Storage/HTGFFSubDirIdIter.h>

namespace HTGFF
{

template <typename EditorT>
class                       SubDirManager;

/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename BaseT>
class SubDirEditorBase
    {
protected:
    typedef BaseT           base_type;                         
    typedef uint32_t       DirectoryID;  
    typedef uint32_t       DirectoryTagID;
private:
    template <typename EditorT>
    friend class            SubDirIter;

    friend class            SubDirManagerBase;

    mutable DirectoryTagID  m_tagID;
    mutable SubDirIdIter    m_idIter;
    mutable base_type*      m_pBase;


protected:  
    typedef SubDirEditorBase<BaseT>
                            super_class;

    DirectoryTagID          GetSubDirTagID                 () const { return m_tagID; }
    size_t                  GetSubDirIndex                 () const { return m_idIter.ConvertToIndex(); } 
    DirectoryID             GetSubDirID                    () const { return *m_idIter; }


    const SubDirIdIter&     GetSubDirIDIter                () const { return m_idIter; }


    const base_type&        GetBase                        () const { return *m_pBase; }
    base_type&              GetBase                        () {return *m_pBase;}

    explicit                SubDirEditorBase               () : m_tagID(0), m_idIter(), m_pBase(0) {}

    // Not meant to be used polymorphically
                            ~SubDirEditorBase              () {}


    // Use default copy/assignment
    };



/*---------------------------------------------------------------------------------**//**
* @description  
* @bsiclass                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
class SubDirIter : public BentleyApi::ImagePP::BidirectionalIteratorWithAutoReverseConst<SubDirIter<EditorT>,
                                                                             SubDirIter<typename BentleyApi::ImagePP::ReverseConstTrait<EditorT>::type>,
                                                                             EditorT>
    {
public:
    friend                  rconst_iterator_t;
    friend class            PacketIterManager<EditorT>;

    typedef uint32_t       DirectoryTagID;

    typedef typename BentleyApi::ImagePP::RemoveConst<typename EditorT::base_type>::type 
                            editor_base_type;

    typename BentleyApi::ImagePP::RemoveConst<EditorT>::type           
                            m_editor;

    const_iterator_t        ConvertToConst                 () const;

    const_reference         Dereference                    () const;
    reference               Dereference                    ();

    void                    Increment                      ();
    void                    Decrement                      ();

    bool                    EqualTo                        (const iterator_t&               pi_rRight) const;
    bool                    EqualTo                        (const rconst_iterator_t&        pi_rRight) const;

    explicit                SubDirIter                     (DirectoryTagID                  pi_tagID,
                                                            const SubDirIdIter&             pi_rIDIter,
                                                            editor_base_type&               pi_rEditorBase);

public:
    explicit                SubDirIter                     ();

    static SubDirIter<const EditorT>         
                            Create                         (DirectoryTagID                  pi_tagID,
                                                            const SubDirIdIter&             pi_rIDIter,
                                                            const editor_base_type&         pi_rEditorBase);
    static SubDirIter<EditorT>         
                            Create                         (DirectoryTagID                  pi_tagID,
                                                            const SubDirIdIter&             pi_rIDIter,
                                                            editor_base_type&               pi_rEditorBase);

    // Use default copy/assignment
    };









#include <STMInternal/Storage/HTGFFSubDirIter.hpp>

} //End namespace HTGFF