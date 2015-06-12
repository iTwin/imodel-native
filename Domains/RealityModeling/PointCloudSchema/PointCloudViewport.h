/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudViewport.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <time.h>
#include <Bentley/DateTime.h>

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

#define PTVORTEX_MAX_VIEWPORTS      256

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Simon.Normand                   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct PTViewportInfo
    {
    public:
        PTViewportInfo (int32_t num, int createTime)
            :m_vpNum (num), m_useTime (createTime) {}

        int32_t GetNum () 
            {
            return m_vpNum;
            }

        int32_t GetForUsage ()
            {
            DateTime::GetCurrentTime ().ToUnixMilliseconds (m_useTime);
            return GetNum (); 
            }

        int64_t GetLastUsage ()
            {
            return m_useTime;
            }

    private:
        int32_t m_vpNum;
        int64_t m_useTime;
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Simon.Normand                   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct PTViewportID
    {
    public:
        PTViewportID () : m_dgnModelP(0), m_sceneP(0), m_msView(0) 
            {
            m_rangeUOR.Init ();
            }
        PTViewportID (DgnModelP modelRefP, BePointCloud::PointCloudScene* sceneP, int view) : m_dgnModelP(modelRefP), m_sceneP(sceneP), m_msView(view)
            {
            m_rangeUOR.Init ();
            }
        PTViewportID (DgnModelP modelRefP, BePointCloud::PointCloudScene* sceneP, int view, DRange3dR rangeUOR) : m_dgnModelP(modelRefP), m_sceneP(sceneP), m_msView(view), m_rangeUOR (rangeUOR) 
            {}
        
        bool operator ==  (PTViewportID const & object) const
            {
            if (m_dgnModelP ==  object.m_dgnModelP && m_sceneP == object.m_sceneP && m_msView == object.m_msView)
                return true;

            return false;
            }
        bool operator <  (PTViewportID const & object) const 
            {
            if (m_dgnModelP < object.m_dgnModelP)
                return true;
            if (m_dgnModelP == object.m_dgnModelP)
                {
                if (m_sceneP < object.m_sceneP)
                    return true;
                if (m_sceneP == object.m_sceneP)
                    return m_msView < object.m_msView;
                }

            return false;
            }

        DgnModelP GetDgnModelP () const {return m_dgnModelP;}
        BePointCloud::PointCloudScene* GetScene () const {return m_sceneP;}
        int GetView () const {return m_msView;}
        DRange3dCR GetRange () const {return m_rangeUOR;}

    private:

        DgnModelP                           m_dgnModelP;
        BePointCloud::PointCloudScene*      m_sceneP;
        int                                 m_msView;
        DRange3d                            m_rangeUOR;
    };

typedef std::map<PTViewportID, PTViewportInfo> PTViewportMap;
typedef PTViewportMap::iterator PTViewportMapItor;

typedef std::map<int64_t, PTViewportID> TimeViewPortIdMap;
typedef TimeViewPortIdMap::iterator  TimeViewPortIdMapItor;

struct ModelViewportManagerDestroyer;

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                    Simon.Normand                   11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
// POINTCLOUD_WIP_IViewMonitor 
// struct ModelViewportManager : public DgnPlatform::IViewMonitor
struct ModelViewportManager
    {
    friend ModelViewportManagerDestroyer;
    static  ModelViewportManager* s_instance;

    ModelViewportManager();
    ~ModelViewportManager(); 
    ModelViewportManager(const ModelViewportManager &);             // intentionally undefined
    ModelViewportManager & operator=(const ModelViewportManager &); // intentionally undefined

    public:
        static ModelViewportManager& Get ()
            {
            if (NULL == s_instance)
                s_instance = new ModelViewportManager();

            return *s_instance;
            }

        void Register();

        int32_t GetViewport (PTViewportID& id);

        void  Remove (BePointCloud::PointCloudScene* pScene);
        void  Remove (DgnModelP modelP);
        void  Remove (int viewIndex);

        void ProcessCleanUpQueue ();
        bool IsQueuedForCleanUp (PTViewportID& id);

    protected:
        void    CleanUp (PTViewportMapItor& itr);
        void    QueueCleanUp (PTViewportID id, PTViewportInfo info);

/* POINTCLOUD_WIP_IViewMonitor
        virtual void    _OnViewClose (ViewportP) override;
        virtual void    _OnViewChanged (ViewportP) override;
*/

        PTViewportMap m_vpCleanUpQueueMap;


        PTViewportMap m_viewportsMap;
        TimeViewPortIdMap m_timeViewPortIdMap;
        std::list<int32_t> m_availableViewports;
    };

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
