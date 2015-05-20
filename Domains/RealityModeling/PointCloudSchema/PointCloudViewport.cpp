/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudViewport.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>

USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Simon.Normand                   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct  ViewPortManager
    {
    static  ViewPortManager* s_instance;
    friend ModelViewportManagerDestroyer;

    ViewPortManager()
        :m_portNumber (0){}
    ~ViewPortManager() {} 
    ViewPortManager(const ViewPortManager &);             // intentionally undefined
    ViewPortManager & operator=(const ViewPortManager &); // intentionally undefined

    public:
        static ViewPortManager& Get () 
            {
            if (NULL == s_instance)
                s_instance = new ViewPortManager();

            return *s_instance;
            }

        int32_t AddViewport (int newVp)
            {
            int32_t vp = PointCloudVortex::AddViewport (newVp, NULL);

            m_viewPortSet.insert (vp);
            return vp; 
            }
        void RemoveViewport (int32_t vp)
            {
            PointCloudVortex::RemoveViewport (vp);
            m_viewPortSet.erase (vp);
            }
    protected:
        std::set<int> m_viewPortSet;
        int      m_portNumber;
    };


// Empty data used to be informed when models are destroyed
static DgnPlatform::DgnModelAppData::Key s_uDataKey;
struct _uData : public DgnPlatform::DgnModelAppData
    {
        virtual void    _OnCleanup (DgnModelR dgnModel) override
            {
            ModelViewportManager::Get().Remove (&dgnModel); 
            delete this;
            }
        virtual void    _OnModelDelete (DgnModelR dgnModel) override
            {
            }
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ModelViewportManager::ModelViewportManager()
    {
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ModelViewportManager::~ModelViewportManager()
    {
/* POINTCLOUD_WIP_IViewManager
    IViewManager::GetManager().DropViewMonitor(this);
*/
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelViewportManager::Register()
    {
/* POINTCLOUD_WIP_IViewManager
    IViewManager::GetManager().AddViewMonitor(this);
*/

    for (int32_t i = 255; i >= 0; i--)
        m_availableViewports.push_back(i);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normandl                   03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/* POINTCLOUD_WIP_IViewMonitor_OnViewClose
void    ModelViewportManager::_OnViewClose (ViewportP vp)
    {
    if (!vp)
        return;

    this->Remove(vp->GetViewNumber());
    }
*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
/* POINTCLOUD_WIP_IViewMonitor_OnViewChanged
void    ModelViewportManager::_OnViewChanged (ViewportP vp)
    {
    if (!vp)
        return;

    int viewIndex = vp->GetViewNumber();
    for ( PTViewportMapItor itr = m_viewportsMap.begin(); itr != m_viewportsMap.end(); ++itr )
        {
        if ( itr->first.GetView() != viewIndex)
            continue;

        // is the pointcloud range contained in the view, if not, destroy ptVp
        DPoint3d box[8];
        itr->first.GetRange().get8Corners (box);

        vp->ActiveToView (box, box, 8);
        vp->ViewToNpc (box, box, 8);

        DRange3d npcRange2;
        npcRange2.initFrom (box, 8);

        DRange3d fullNpcRange;
        fullNpcRange.initFrom (0,0,0,1,1,1);

        if (!npcRange2.intersectsWith (&fullNpcRange))
            {
            QueueCleanUp (itr->first, itr->second);
            }
        }
    }
*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ModelViewportManager::IsQueuedForCleanUp (PTViewportID& id)
    {
    PTViewportMapItor itr = m_vpCleanUpQueueMap.find(id);
    if (itr != m_vpCleanUpQueueMap.end())
        return true;

    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelViewportManager::QueueCleanUp (PTViewportID id, PTViewportInfo info)
    {
    m_vpCleanUpQueueMap.insert (PTViewportMap::value_type(id, info));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelViewportManager::ProcessCleanUpQueue ()
    {
    PTViewportMapItor itr;
    while (m_vpCleanUpQueueMap.end() !=  (itr = m_vpCleanUpQueueMap.begin ()))
        {
        PTViewportMapItor itr2 = m_viewportsMap.find (itr->first);
        // clean up will erase in m_vpCleanUpQueueMap
        CleanUp(itr2);
        }

    m_vpCleanUpQueueMap.clear(); // just in case
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t ModelViewportManager::GetViewport (PTViewportID& id)
    {
    _uData* pData = (_uData*)id.GetDgnModelP()->FindAppData (s_uDataKey);

    if (NULL == pData)
        {
        pData = new _uData();
        id.GetDgnModelP()->AddAppData (s_uDataKey, pData);
        }

    PTViewportMapItor itr = m_viewportsMap.find(id);
    if (itr != m_viewportsMap.end())
       {
       // was this vp scheduled to be destroyed?
       PTViewportMapItor queuedItr =m_vpCleanUpQueueMap.find (id);
       if (queuedItr != m_vpCleanUpQueueMap.end())
           m_vpCleanUpQueueMap.erase (queuedItr);

       // need to update usage
       int64_t lastUsed = itr->second.GetLastUsage ();
       int32_t existingVP = itr->second.GetForUsage ();

       // erase and reinsert in map using the new usage as key
       TimeViewPortIdMapItor timeItor = m_timeViewPortIdMap.find (lastUsed);
       if (timeItor != m_timeViewPortIdMap.end ())
            m_timeViewPortIdMap.erase (timeItor);
       m_timeViewPortIdMap.insert (TimeViewPortIdMap::value_type( itr->second.GetLastUsage (), id));

       // now is a good time to destroy all unused viewports
       ProcessCleanUpQueue();

       return  existingVP;
       }

    int32_t newVp = 0;
    if ((int32_t)m_viewportsMap.size () == PTVORTEX_MAX_VIEWPORTS)
        {
        //remove oldest VP
        PTViewportMapItor removeItr = m_viewportsMap.find (m_timeViewPortIdMap.begin ()->second);
        int32_t vpToRemove = removeItr->second.GetNum ();
        ViewPortManager::Get().RemoveViewport (vpToRemove);
        newVp =  vpToRemove;
        m_viewportsMap.erase (removeItr);
        m_timeViewPortIdMap.erase (m_timeViewPortIdMap.begin ());
        }
    else
        {
        newVp = *(m_availableViewports.rbegin()); 
        m_availableViewports.pop_back();
        }


    newVp = ViewPortManager::Get().AddViewport (newVp);
    int createTime = clock ();
    m_viewportsMap.insert (PTViewportMap::value_type(id, PTViewportInfo (newVp, createTime)));
    m_timeViewPortIdMap.insert (TimeViewPortIdMap::value_type( createTime, id));

    // now is a good time to destroy all unused viewports
    ProcessCleanUpQueue();

    return newVp;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelViewportManager::CleanUp (PTViewportMapItor& itr)
    {
    TimeViewPortIdMapItor timeItr = m_timeViewPortIdMap.find (itr->second.GetLastUsage ());
    if (timeItr != m_timeViewPortIdMap.end())
        {
        m_timeViewPortIdMap.erase(timeItr);
        }

    // erase from cleanup queue if found
    PTViewportMapItor queueItr = m_vpCleanUpQueueMap.find (itr->first);
    if (queueItr != m_vpCleanUpQueueMap.end())
        m_vpCleanUpQueueMap.erase(queueItr);

    m_availableViewports.push_back(itr->second.GetNum ());
    ViewPortManager::Get().RemoveViewport (itr->second.GetNum ());
    m_viewportsMap.erase(itr++);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void   ModelViewportManager::Remove (DgnModelP dgnModelP)
    {
    for ( PTViewportMapItor itr = m_viewportsMap.begin(); itr != m_viewportsMap.end(); /* NOINCREMENT */ )
        {
        if ( itr->first.GetDgnModelP () == dgnModelP)
            CleanUp (itr);
        else
            ++itr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void   ModelViewportManager::Remove (BePointCloud::PointCloudScene* pScene)
    {
    for ( PTViewportMapItor itr = m_viewportsMap.begin(); itr != m_viewportsMap.end(); /* NOINCREMENT */ )
        {
        if ( itr->first.GetScene () == pScene)
            CleanUp (itr);
        else
            ++itr;
        }    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Simon.Normand                   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void   ModelViewportManager::Remove (int viewIndex)
    {
    for ( PTViewportMapItor itr = m_viewportsMap.begin(); itr != m_viewportsMap.end(); /* NOINCREMENT */ )
        {
        if ( itr->first.GetView() == viewIndex)
            CleanUp (itr);
        else
            ++itr;
        }
    }

// Static member initialization
ViewPortManager* ViewPortManager::s_instance=NULL;
ModelViewportManager* ModelViewportManager::s_instance=NULL;

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

struct ModelViewportManagerDestroyer
    {
    public:
        ModelViewportManagerDestroyer() {}
        ~ModelViewportManagerDestroyer()
            {
            if (ModelViewportManager::s_instance!=NULL)
                delete ModelViewportManager::s_instance;
            if (ViewPortManager::s_instance!=NULL)
                delete ViewPortManager::s_instance;
            ModelViewportManager::s_instance=NULL;
            ViewPortManager::s_instance=NULL;
            }
    private:
        ModelViewportManagerDestroyer(ModelViewportManagerDestroyer const&);  //disabled
        ModelViewportManagerDestroyer& operator=(ModelViewportManagerDestroyer const&);   //disabled
    };
END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

ModelViewportManagerDestroyer s_modelViewportManagerdestroyer;


