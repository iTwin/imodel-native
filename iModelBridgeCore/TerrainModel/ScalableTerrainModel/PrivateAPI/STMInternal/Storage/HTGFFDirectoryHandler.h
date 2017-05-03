//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: ScalableTerrainModel/PrivateAPI/STMInternal/Storage/HTGFFDirectoryHandler.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#pragma once

#include <ImagePP/all/h/HFCPtr.h>
#include <STMInternal/Storage/HPUPacket.h>

namespace HTGFF {


class Directory;

class AttributeManager;
class SubDirManagerBase;
class PacketManagerBase;

template <typename T>
class SubDirManager;
template <typename EditorT>
class SubDirIterManager;
template <typename EditorT>
class PacketIterManager;


/*---------------------------------------------------------------------------------**//**
* @description  Base class for IDTM packet handlers. A packet handler is a directory's
*               companion class used to access a directory's data. This class permit to
*               uncouple directory attributes access from directory's data access. This
*               uncoupling enables having multiples data accessors of different types on
*               the same directory type (not instance). Validation for the type is made
*               at the creation of the handler which enables many assumptions afterward
*               (no type validity need to be made afterward on a per function basis as
*               the handler could not have been created for a directory it does not
*               supports).
*
*               User must implement specific behavior for the Save and Load event.
*
*               Close event is triggered by the directory's close method when the handler
*               is correctly registered to the directory. Close event triggers the
*               save event and disable any further save events.
*
*               Save event is triggered by the directory's save method when the handler
*               is correctly registered to the directory.
*
*               Load event may be triggered manually by the user in his CreateFrom
*               factory function implementation.
*
*               Use RegisterTo and UnregisterFrom to manually register/unregister
*               the handler to handled directories (when not using SingleDirHandler).
*
*               Save method should usually be invoked by specialization's destructor
*               in order to get the handler's cached states saved.
*
* @see          HTGFFDirectory
* @see          SingleDirHandler
* @bsiclass                                                  Raymond.Gauthier   4/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class DirectoryHandler : public BentleyApi::ImagePP::HFCShareableObject<DirectoryHandler>
    {
public:
    typedef HPU::Packet         Packet;
    typedef uint32_t           PacketID;
    typedef uint32_t             AttributeID;

     virtual              ~DirectoryHandler              () = 0;

     bool                 Save                           ();

protected:
     explicit             DirectoryHandler               ();

    bool                        Load                           ();

    bool                        RegisterTo                     (Directory&                      pi_rDir);
    void                        UnregisterFrom                 (Directory&                      pi_rDir);


     const AttributeManager&
                                AttributeMgr                   (const Directory&                pi_rDir) const;
     AttributeManager&    AttributeMgr                   (Directory&                      pi_rDir);

     const PacketManagerBase&
                                PacketMgr                      (const Directory&                pi_rDir) const;
     PacketManagerBase&   PacketMgr                      (Directory&                      pi_rDir);


    template <typename EditorT>
    const PacketIterManager<EditorT>&   
                                PacketIterMgr                  (const Directory&                pi_rDir) const;
    template <typename EditorT>
    PacketIterManager<EditorT>& PacketIterMgr                  (Directory&                      pi_rDir);

     const SubDirManagerBase&
                                SubDirMgr                      (const Directory&                pi_rDir) const;
     SubDirManagerBase&   SubDirMgr                      (Directory&                      pi_rDir);


    template <typename T>
    const SubDirManager<T>&     SubDirMgr                      (const Directory&                pi_rDir) const;
    template <typename T>
    SubDirManager<T>&           SubDirMgr                      (Directory&                      pi_rDir);

    template <typename EditorT>
    const SubDirIterManager<EditorT>& 
                                SubDirIterMgr                  (const Directory&                pi_rDir) const;
    template <typename EditorT>
    SubDirIterManager<EditorT>& SubDirIterMgr                  (Directory&                      pi_rDir);

private:

    friend class                Directory;
    friend class                DirectoryImpl;

    // Disable copies of any kind
                                DirectoryHandler               (const DirectoryHandler&         pi_rObj);
    DirectoryHandler&           operator=                      (const DirectoryHandler&         pi_rObj);

    bool                        Close                          ();

    virtual bool                _Save                          () = 0;
    virtual bool                _Load                          () = 0;

    bool                        m_IsClosed;
    };



/*---------------------------------------------------------------------------------**//**
* @description  None template part of the SingleDirHandler. Could not be used
*               directly as a base. Use SingleDirHandler instead.
*
*               Encapsulate the automatic handler registration mechanism.
*
* @see SingleDirHandler
*
* @bsiclass                                                  Raymond.Gauthier   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
class SingleDirHandlerConcretePart : public DirectoryHandler
    {
private:
    template <typename HandlerT, typename DirT>
    friend class                SingleDirHandler;

    virtual                     ~SingleDirHandlerConcretePart  () = 0;
    explicit                    SingleDirHandlerConcretePart   (Directory&                      pi_rDir);

    Directory&                  m_rDir;
    };


/*---------------------------------------------------------------------------------**//**
* @description  Base helper class for simple handlers that handle a single
*               directory at a time. Automatically register this handler to
*               the single handled directory. Add a generic CreateFrom implementation
*               that specializations could use in their CreateFrom function. When using
*               CreateFromImpl, the Load event is automatically triggered.
*
* @see DirectoryHandler
* @see SingleDirHandlerConcretePart
*
* @bsiclass                                                  Raymond.Gauthier   12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename HandlerT, typename DirT>
class SingleDirHandler : public SingleDirHandlerConcretePart
    {
private:
    typedef BentleyApi::ImagePP::HFCPtr<DirT>  DirPtr;

public:
    // Provide these convenience typedefs
    typedef BentleyApi::ImagePP::HFCPtr<HandlerT>    Ptr;
    typedef BentleyApi::ImagePP::HFCPtr<HandlerT>    CPtr;

    virtual                     ~SingleDirHandler             () {}


    // Provide these convenience accessors
    const DirT& GetDir () const {return static_cast<const DirT&>(m_rDir);}
    DirT&       GetDir ()       {return static_cast<DirT&>(m_rDir);}

protected:
    typedef SingleDirHandler<HandlerT, DirT>
    super_class;

    explicit SingleDirHandler (DirT& pi_rDir)
        :     SingleDirHandlerConcretePart(pi_rDir)
        {}

    static Ptr CreateFromImpl (DirT* pi_rpDir)
        {
        if (0 == pi_rpDir || !HandlerT::IsCompatibleWith(*pi_rpDir))
            return Ptr();

        Ptr pHandler = new HandlerT(pi_rpDir);

        if (!pHandler->_Load())
            return Ptr();

        return pHandler;
        }
    };


/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename FacadeT>
const PacketIterManager<FacadeT>& DirectoryHandler::PacketIterMgr (const Directory& pi_rDir) const
    {
    return pi_rDir.PacketIterMgr<FacadeT>();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename FacadeT>
PacketIterManager<FacadeT>& DirectoryHandler::PacketIterMgr (Directory& pi_rDir)
    {
    return pi_rDir.PacketIterMgr<FacadeT>();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
const SubDirManager<T>& DirectoryHandler::SubDirMgr (const Directory& pi_rDir) const
    {
    return pi_rDir.SubDirMgr<T>();
    }

/*---------------------------------------------------------------------------------**//**
* @description
* @bsimethod                                                  Raymond.Gauthier   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename T>
SubDirManager<T>& DirectoryHandler::SubDirMgr (Directory& pi_rDir)
    {
    return pi_rDir.SubDirMgr<T>();
    }

/*---------------------------------------------------------------------------------**//**
* @description     
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
const SubDirIterManager<EditorT>& DirectoryHandler::SubDirIterMgr (const Directory& pi_rDir) const
    {
    return pi_rDir.SubDirIterMgr<EditorT>();
    }

/*---------------------------------------------------------------------------------**//**
* @description     
* @bsimethod                                                  Raymond.Gauthier   11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
template <typename EditorT>
SubDirIterManager<EditorT>& DirectoryHandler::SubDirIterMgr (Directory& pi_rDir)
    {
    return pi_rDir.SubDirIterMgr<EditorT>();
    }


} //End namespace HTGFF