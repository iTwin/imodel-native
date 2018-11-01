/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ScalableMesh/IScalableMeshSourceCollection.hpp $
|    $RCSfile: IScalableMeshSourceCollection.hpp,v $
|   $Revision: 1.6 $
|       $Date: 2011/10/26 17:55:50 $
|     $Author: Raymond.Gauthier $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__PUBLISH_SECTION_START__*/


inline IDTMSourceCollection::const_iterator IDTMSourceCollection::Begin () const
    {
    return const_iterator(_Begin());
    }

inline IDTMSourceCollection::const_iterator IDTMSourceCollection::End () const
    {
    return const_iterator(_End());
    }

inline IDTMSourceCollection::iterator IDTMSourceCollection::BeginEdit ()
    {
    return iterator(_BeginEdit());
    }

inline IDTMSourceCollection::iterator IDTMSourceCollection::EndEdit ()
    {
    return iterator(_EndEdit());
    }


/*__PUBLISH_SECTION_END__*/

inline IDTMSourceCollection::iterator IDTMSourceCollection::BeginEditInternal ()
    {
    return iterator(_BeginEditInternal());
    }

inline IDTMSourceCollection::iterator IDTMSourceCollection::EndEditInternal ()
    {
    return iterator(_EndEditInternal());
    }

/*__PUBLISH_SECTION_START__*/

inline IDTMSourceCollection::const_reverse_iterator IDTMSourceCollection::rBegin () const 
    { 
    return const_reverse_iterator(End()); 
    }

inline IDTMSourceCollection::const_reverse_iterator IDTMSourceCollection::rEnd () const 
    { 
    return const_reverse_iterator(Begin()); 
    }

inline IDTMSourceCollection::reverse_iterator IDTMSourceCollection::rBeginEdit () 
    { 
    return reverse_iterator(EndEdit()); 
    }

inline IDTMSourceCollection::reverse_iterator IDTMSourceCollection::rEndEdit () 
    { 
    return reverse_iterator(BeginEdit()); 
    }

inline IDTMSourceCollection::iterator IDTMSourceCollection::Release(const const_iterator&   sourceIt,
                                                                    IDTMSourcePtr&      sourcePtr)
    {
    return iterator(_Release(sourceIt.GetBase(), sourcePtr));
    }

inline IDTMSourceCollection::const_iterator IDTMSourceCollection::GetIterFor (const IDTMSource& source) const
    {
    return const_iterator(_GetIterFor(source));
    }

inline IDTMSourceCollection::iterator IDTMSourceCollection::EditIterFor (const IDTMSource& source)
    {
    return iterator(_EditIterFor(source));
    }


inline IDTMSourceCollection::const_iterator IDTMSourceCollection::GetIterAt (uint32_t index) const
    {
    return const_iterator(_GetIterAt(index));
    }


inline IDTMSourceCollection::iterator IDTMSourceCollection::EditIterAt (uint32_t index)
    {
    return iterator(_EditIterAt(index));
    }


inline IDTMSourceCollection::iterator IDTMSourceCollection::Remove (const const_iterator& sourceIt)
    {
    return iterator(_Remove(sourceIt.GetBase()));
    }

inline BENTLEY_NAMESPACE_NAME::ScalableMesh::Import::UpToDateState   IDTMSourceCollection::GetUpToDateState(const const_iterator& sourceIt)
    {
    return _GetUpToDateState(sourceIt.GetBase());
    }

inline IDTMSourceCollection::iterator IDTMSourceCollection::SetRemoveState(const const_iterator& sourceIt)
    {
    return iterator(_SetRemoveState(sourceIt.GetBase()));
    }

inline IDTMSourceCollection::iterator IDTMSourceCollection::UnGroup (const const_iterator& groupSourceIt)
    {
    return iterator(_UnGroup(groupSourceIt.GetBase()));
    }

inline IDTMSourceCollection::iterator IDTMSourceCollection::MoveDown (const const_iterator& sourceIt)
    {
    return iterator(_MoveDown(sourceIt.GetBase()));
    }

inline IDTMSourceCollection::iterator IDTMSourceCollection::MoveToBottom (const const_iterator& sourceIt)
    {
    return iterator(_MoveToBottom(sourceIt.GetBase()));
    }

inline IDTMSourceCollection::iterator IDTMSourceCollection::MoveToPos  (const const_iterator&        sourceIt, 
                                                                        const const_iterator&         destinationIt)
    {
    return iterator(_MoveToPos(sourceIt.GetBase(), destinationIt.GetBase()));
    }

inline IDTMSourceCollection::iterator IDTMSourceCollection::MoveToTop (const const_iterator& sourceIt)
    {
    return iterator(_MoveToTop(sourceIt.GetBase()));
    }

inline IDTMSourceCollection::iterator IDTMSourceCollection::MoveUp (const const_iterator& sourceIt)
    {
    return iterator(_MoveUp(sourceIt.GetBase()));
    }

inline void IDTMSourceCollection::Clear()
    {
    return _Clear();
    }