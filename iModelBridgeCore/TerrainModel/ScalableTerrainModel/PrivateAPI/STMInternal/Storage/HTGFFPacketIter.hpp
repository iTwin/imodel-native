//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFPacketIter.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
PacketIter<EditorT>::PacketIter ()
    {
    HSTATICASSERT(BentleyApi::ImagePP::IsConstTrait<editor_base_type>::value == BentleyApi::ImagePP::IsConstTrait<value_type>::value);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
PacketIter<EditorT>::PacketIter            (editor_base_type&       pi_rEditorBase,
                                            const PacketIdIter&     pi_rIDIter)
    {
    m_editor.m_pBase = &pi_rEditorBase;
    m_editor.m_idIter = pi_rIDIter;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
typename PacketIter<EditorT>::const_iterator_t PacketIter<EditorT>::ConvertToConst () const
    {
    return rconst_iterator_t(m_editor.m_pBase, m_editor.m_idIter);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
typename PacketIter<EditorT>::const_reference PacketIter<EditorT>::Dereference () const
    {
    HPRECONDITION(0 != m_editor.m_pBase);
    return m_editor;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
typename PacketIter<EditorT>::reference PacketIter<EditorT>::Dereference ()
    {
    HPRECONDITION(0 != m_editor.m_pBase);
    return m_editor;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
void PacketIter<EditorT>::Increment ()
    {
    ++m_editor.m_idIter;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
void PacketIter<EditorT>::Decrement ()
    {
    --m_editor.m_idIter;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
bool PacketIter<EditorT>::EqualTo (const iterator_t& pi_rRight) const
    {
    return m_editor.m_idIter == pi_rRight.m_editor.m_idIter;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
bool PacketIter<EditorT>::EqualTo (const rconst_iterator_t& pi_rRight) const
    {
    return m_editor.m_idIter == pi_rRight.m_editor.m_idIter;
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
PacketIter<const EditorT> PacketIter<EditorT>::Create  (const PacketIdIter&     pi_rIDIter,
                                                        const editor_base_type& pi_rEditorBase)
    {
    return PacketIter<const EditorT>(const_cast<editor_base_type&>(pi_rEditorBase), pi_rIDIter);
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
PacketIter<EditorT> PacketIter<EditorT>::Create(const PacketIdIter& pi_rIDIter,
                                                editor_base_type&   pi_rEditorBase)
    {
    return PacketIter<EditorT>(pi_rEditorBase, pi_rIDIter);
    }
