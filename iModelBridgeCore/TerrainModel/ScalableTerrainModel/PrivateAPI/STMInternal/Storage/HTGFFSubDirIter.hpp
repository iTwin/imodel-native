//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFSubDirIter.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------




/*---------------------------------------------------------------------------------**//**
* @description    
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
SubDirIter<EditorT>::SubDirIter ()
    {
    HSTATICASSERT(BentleyApi::ImagePP::IsConstTrait<editor_base_type>::value == BentleyApi::ImagePP::IsConstTrait<value_type>::value);
    }

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
SubDirIter<EditorT>::SubDirIter            (DirectoryTagID          pi_tagID,
                                            const SubDirIdIter&     pi_rIDIter,
                                            editor_base_type&       pi_rEditorBase)
    {
    m_editor.m_tagID = pi_tagID;
    m_editor.m_idIter = pi_rIDIter;
    m_editor.m_pBase = &pi_rEditorBase;
    }

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
typename SubDirIter<EditorT>::const_iterator_t SubDirIter<EditorT>::ConvertToConst () const
    {
    return rconst_iterator_t(m_editor.m_pBase, m_editor.m_idIter);
    }

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
typename SubDirIter<EditorT>::const_reference SubDirIter<EditorT>::Dereference () const
    {
    HPRECONDITION(0 != m_editor.m_pBase);
    return m_editor;
    }

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
typename SubDirIter<EditorT>::reference SubDirIter<EditorT>::Dereference ()
    {
    HPRECONDITION(0 != m_editor.m_pBase);
    return m_editor;
    }

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
void SubDirIter<EditorT>::Increment ()
    {
    ++m_editor.m_idIter;
    }

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
void SubDirIter<EditorT>::Decrement ()
    {
    --m_editor.m_idIter;
    }

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
bool SubDirIter<EditorT>::EqualTo (const iterator_t& pi_rRight) const
    {
    return m_editor.m_idIter == pi_rRight.m_editor.m_idIter;
    }

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
bool SubDirIter<EditorT>::EqualTo (const rconst_iterator_t& pi_rRight) const
    {
    return m_editor.m_idIter == pi_rRight.m_editor.m_idIter;
    }

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
SubDirIter<const EditorT> SubDirIter<EditorT>::Create  (DirectoryTagID          pi_tagID,
                                                        const SubDirIdIter&     pi_rIDIter,
                                                        const editor_base_type& pi_rEditorBase)
    {
    return SubDirIter<const EditorT>(pi_tagID, pi_rIDIter, const_cast<editor_base_type&>(pi_rEditorBase));
    }

/*---------------------------------------------------------------------------------**//**
* @description    
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
SubDirIter<EditorT> SubDirIter<EditorT>::Create(DirectoryTagID      pi_tagID, 
                                                const SubDirIdIter& pi_rIDIter,
                                                editor_base_type&   pi_rEditorBase)
    {
    return SubDirIter<EditorT>(pi_tagID, pi_rIDIter, pi_rEditorBase);
    }



