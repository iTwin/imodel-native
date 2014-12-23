#ifndef POINTOOLS_API_HANDLE_H
#define POINTOOLS_API_HANDLE_H

#define PT_SCENE_MULTIPLIER	10000

// handle management
#define _GetCloudHandleByIndex(scene, cloud_index)	(((cloud_index) + 1) + (scene))
#define _GetSceneHandleByCloud(cloud)				((cloud) / PT_SCENE_MULTIPLIER)
#define _GetSceneHandleByIndex(scene_index)			(((scene_index) + 1) * PT_SCENE_MULTIPLIER)
#define _GetSceneIndexByHandle(scene_or_cloud_handle) ((scene_or_cloud_handle) / PT_SCENE_MULTIPLIER - 1)
#define _GetCloudIndexByHandle(cloud)				((cloud) % PT_SCENE_MULTIPLIER - 1)

#endif