/*-------------------------------------------------------------------------------------+
|
|     $Source: ScalableMeshSchema/ScalableMeshProgressiveDisplay.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__


BEGIN_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE



//=======================================================================================
// @bsiclass                                                    BentleySystems
//=======================================================================================
struct ScalableMeshProgressiveDisplay : IProgressiveDisplay, NonCopyableClass
    {
    DEFINE_BENTLEY_REF_COUNTED_MEMBERS

private : 

    IScalableMeshPtr                        m_smPtr;    
    IScalableMeshDisplayCacheManagerPtr     m_displayNodesCache;
    IScalableMeshProgressiveQueryEnginePtr  m_progressiveQueryEngine;        
    ScalableMeshDrawingInfoPtr              m_currentDrawingInfoPtr;
    DMatrix4d                               m_storageToUorsTransfo;        

protected:
                 
    ScalableMeshProgressiveDisplay(IScalableMeshPtr& smPtr, DMatrix4d& storageToUorsTransfo);
    virtual ~ScalableMeshProgressiveDisplay();

    //! Displays tiled rasters and schedules downloads. 
    virtual Completion _Process(ViewContextR) override;

    // set limit and returns true to cause caller to call EnableStopAfterTimout
    virtual bool _WantTimeoutSet(uint32_t& limit) override {return false;}

    bool ShouldDrawInContext (ViewContextR context) const;

    //void DrawLoadedChildren(ViewContextR context, TextureTileR tile, uint32_t resolutionDelta);

    //bool WantTexture() const {return m_wantTexture;}

public:

    void Draw (ViewContextR context);

    static RefCountedPtr<TerrainProgressiveDisplay> Create(TexturedMeshR mesh, ViewContextR context);
};


END_BENTLEY_SCALABLEMESH_SCHEMA_NAMESPACE
