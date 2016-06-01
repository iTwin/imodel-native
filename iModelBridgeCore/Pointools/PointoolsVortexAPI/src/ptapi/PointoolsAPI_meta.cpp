#include "PointoolsVortexAPIInternal.h"

#define POINTOOLS_API_BUILD_DLL

#include <ptapi/PointoolsVortexAPI.h>
#include <ptapi/PointoolsAPI_handle.h>

#include <ptengine/PointsScene.h>
#include <ptengine/PointsPager.h>
#include <ptengine/RenderEngine.h>
#include <ptengine/engine.h>

#include <ptappdll/ptapp.h>
#include <ptcloud2/pod.h>

#include <ptl/branch.h>
#include <ptl/project.h>
#include <pt/project.h>
#include <pt/registry.h>
#include <pt/timestamp.h>

using namespace pcloud;
using namespace pointsengine;

struct FileMeta
{
	MetaData		mdata;

	PThandle		sceneHandle;
	ptds::FilePath	filepath;

	pt::String		name;

	int				numClouds;
	int				numPoints;
	PTuint			sceneSpec;
	PTdouble		boundLower[3];
	PTdouble		boundUpper[3];
};

namespace
{
	typedef std::map< PThandle, FileMeta* > FileMetaMap;
	FileMetaMap m_fileMeta;
    PThandle m_lastHandle = static_cast<PThandle>(15e7);
};
extern pcloud::Scene*	sceneFromHandle(PThandle handle);
extern int				setLastErrorCode( int );

//--------------------------------------------------------------------------------------------------
// extract meta data from a scene pointer - internal use only
//--------------------------------------------------------------------------------------------------
FileMeta *extractSceneMeta( const pcloud::Scene *scene, bool inGraph )
{
	if (!scene) return 0;

	FileMeta *meta = new FileMeta;
	meta->name = pt::String(scene->identifier());
	meta->numClouds = scene->size();
	meta->numPoints = 0;
	meta->sceneSpec = 0;

	for (int i=0; i<meta->numClouds; i++)
	{
        meta->numPoints += static_cast<int>(scene->cloud(i)->numPoints());
		meta->sceneSpec |= scene->cloud(i)->hasChannel( pcloud::PCloud_Intensity ) ? PT_HAS_INTENSITY : 0;
		meta->sceneSpec |= scene->cloud(i)->hasChannel( pcloud::PCloud_RGB ) ? PT_HAS_RGB : 0;
		meta->sceneSpec |= scene->cloud(i)->hasChannel( pcloud::PCloud_Normal ) ? PT_HAS_NORMAL : 0;
	}
	
	if (!inGraph)
	{
		pt::BoundingBox bb = scene->localBounds().bounds();

		meta->boundLower[0] = bb.lower(0);
		meta->boundLower[1] = bb.lower(1);
		meta->boundLower[2] = bb.lower(2);

		meta->boundUpper[0] = bb.upper(0);
		meta->boundUpper[1] = bb.upper(1);
		meta->boundUpper[2] = bb.upper(2);
	}
	else
	{
		pt::BoundingBoxD bb = scene->projectBounds().bounds();
		
		pt::vector3d lower( &bb.lower(0)), wlower;
		pt::vector3d upper( &bb.upper(0)), wupper;

		pt::Project3D::project().project2WorldSpace( lower, wlower );
		pt::Project3D::project().project2WorldSpace( upper, wupper );

		wlower.get(meta->boundLower);
		wupper.get(meta->boundUpper);
	}


	meta->mdata = scene->metaData();
	
	return meta;
}
//--------------------------------------------------------------------------------------------------
// read the files meta data then close
//--------------------------------------------------------------------------------------------------
PThandle PTAPI	ptReadPODMeta( const PTstr filepath )
{
	/* open the file and read the meta */ 

	ptds::FilePath path( filepath );
	pcloud::Scene *scene = pcloud::Scene::createFromFile(path);

	if (!scene) return 0; // TODO: set last error
	
	PThandle retHandle = 0;
	if (path.checkExists())
	{
		pcloud::PodJob job(scene, filepath);
		bool success = pcloud::PodIO::openForRead(job);

		if (success)
		{
			if (pcloud::PodIO::readVersion(job)
				&& pcloud::PodIO::readHeader(job))
			{
				uint i;
				for (i=0; i< scene->size(); i++)
				{
					pcloud::PointCloud *pc = scene->cloud(i);
					if (!pcloud::PodIO::readCloudStructure(job, pc))
					{
						setLastErrorCode( PTV_FILE_READ_FAILURE );		
						break;
					}
				}
				if (i == scene->size()) retHandle = m_lastHandle;	//successfully read
			}
			pcloud::PodIO::close(job);	// close the job file
		}
	}
	else
	{
		setLastErrorCode( PTV_FILE_NOT_EXIST );
	}
	
	if (retHandle)
	{
		FileMeta *meta = extractSceneMeta(scene,false);	//do the meta extraction and copy into local meta
		if (meta) 
		{
			meta->sceneHandle = 0;	
			meta->filepath = filepath;
			m_fileMeta.insert( FileMetaMap::value_type( m_lastHandle, meta ) );	//store
		}
	}
	delete scene;			//clean up
	
	++m_lastHandle;
	return retHandle;	
}
//--------------------------------------------------------------------------------------------------
// get meta data handle from a scenehandle
//--------------------------------------------------------------------------------------------------
PThandle	PTAPI	ptGetMetaDataHandle( PThandle sceneHandle )
{
	pcloud::Scene *scene = sceneFromHandle( sceneHandle );
	if (!scene) return 0;

	FileMeta *meta = extractSceneMeta( scene, true );
	if (meta)
	{
		meta->sceneHandle = sceneHandle;
		m_fileMeta.insert( FileMetaMap::value_type( ++m_lastHandle, meta ) );			
	}
	else return 0;

	return m_lastHandle;
}
//--------------------------------------------------------------------------------------------------
// get basic metadata using a metadata handle
//--------------------------------------------------------------------------------------------------
PTres		PTAPI	ptGetMetaData( PThandle metadataHandle, PTstr name, PTint &num_clouds, 
						PTuint64 &num_points, PTuint &scene_spec, PTdouble *lower3, PTdouble *upper3 )
{
	FileMetaMap::iterator i = m_fileMeta.find( metadataHandle );
	if (i == m_fileMeta.end()) 
	{
		return setLastErrorCode( PTV_INVALID_HANDLE );
	}
	if (!lower3 || !upper3)
		return setLastErrorCode( PTV_VOID_POINTER );

	FileMeta *meta = i->second;
	memcpy(lower3, meta->boundLower, sizeof(PTdouble) * 3);
	memcpy(upper3, meta->boundUpper, sizeof(PTdouble) * 3);

	PTdouble cbase[3];
	ptGetCoordinateBase( cbase );

	// transform to world coordinates
	lower3[0] += cbase[0];
	lower3[1] += cbase[1];
	lower3[2] += cbase[2];

	upper3[0] += cbase[0];
	upper3[1] += cbase[1];
	upper3[2] += cbase[2];

	scene_spec = meta->sceneSpec;
	wcsncpy( name, meta->name.c_wstr(), PT_MAX_META_STR_LEN );
	num_points = meta->numPoints;
	num_clouds = meta->numClouds;

	return setLastErrorCode( PTV_SUCCESS );
}
//--------------------------------------------------------------------------------------------------
// get a single text meta tag, dot notation used for sections
//--------------------------------------------------------------------------------------------------
PTres		PTAPI	ptGetMetaTag( PThandle metadataHandle, const PTstr tagName, PTstr value )
{
	FileMetaMap::iterator i = m_fileMeta.find( metadataHandle );
	if (i == m_fileMeta.end())
	{
		return setLastErrorCode( PTV_INVALID_HANDLE );
	}
	if (!value)
		return setLastErrorCode( PTV_VOID_POINTER );
	
	FileMeta *meta = i->second;

	wchar_t tagName_str[PT_MAX_META_STR_LEN];
	wcsncpy(tagName_str, tagName, PT_MAX_META_STR_LEN);
	
    wchar_t * context;
	wchar_t *first_tok = wcstok( tagName_str, L".", &context);
	wchar_t *second_tok = wcstok( nullptr, L".", &context);

	pt::String tsection, titem, tvalue;

	if (!second_tok)
	{
		tsection = pt::String("Any");
		titem = pt::String( first_tok );
	}
	else 
	{
		tsection = pt::String(first_tok);
		titem = pt::String( second_tok );
	}

	if (meta->mdata.getMetaDataString( tsection, titem, tvalue ) && tvalue.length())
	{
		if (tvalue.length())
		{
			wcsncpy( value, tvalue.c_wstr(), PT_MAX_META_STR_LEN ); 
			return  setLastErrorCode( PTV_SUCCESS );
		}
	}
	value[0] = 0;

	return setLastErrorCode( PTV_METATAG_NOT_FOUND );
}

//--------------------------------------------------------------------------------------------------
// get a single text meta tag, dot notation used for sections
//--------------------------------------------------------------------------------------------------
PTres		PTAPI	ptSetMetaTag( PThandle metadataHandle, const PTstr tagName, const PTstr value )
{
	FileMetaMap::iterator i = m_fileMeta.find( metadataHandle );
	if (i == m_fileMeta.end())
	{
		return setLastErrorCode( PTV_INVALID_HANDLE );
	}
	if (!value)
		return setLastErrorCode( PTV_VOID_POINTER );
	
	FileMeta *meta = i->second;

	wchar_t tagName_str[PT_MAX_META_STR_LEN];
	wcsncpy(tagName_str, tagName, PT_MAX_META_STR_LEN);
	
    wchar_t * context;
	wchar_t *first_tok = wcstok( tagName_str, L".", &context);
	wchar_t *second_tok = wcstok(nullptr, L".", &context);

	pt::String tsection, titem, tvalue;

	if (!second_tok)
	{
		tsection = pt::String("Any");
		titem = pt::String( first_tok );
	}
	else 
	{
		tsection = pt::String(first_tok);
		titem = pt::String( second_tok );
	}

	if (meta->mdata.setMetaDataString( tsection, titem, pt::String(value) ))
	{
		return setLastErrorCode( PTV_SUCCESS );
	}

	return setLastErrorCode( PTV_METATAG_NOT_FOUND );
}
//--------------------------------------------------------------------------------------------------
// free up meta data memory, handle is invalid after this operation
//--------------------------------------------------------------------------------------------------
PTvoid		PTAPI	ptFreeMetaData( PThandle metadataHandle )
{
	FileMetaMap::iterator i = m_fileMeta.find( metadataHandle );
	if (i == m_fileMeta.end()) return ;
	
	delete i->second;
	m_fileMeta.erase( i );
}

//--------------------------------------------------------------------------------------------------
// write meta data updates back to the file
//--------------------------------------------------------------------------------------------------
PTres		PTAPI	ptWriteMetaTags( PThandle metadataHandle )
{
	// check the file version supports meta tag writing
	FileMetaMap::iterator i = m_fileMeta.find( metadataHandle );
	if (i == m_fileMeta.end()) 
		return setLastErrorCode( PTV_INVALID_HANDLE );

	FileMeta *meta = i->second;
	
	// check the meta tags section is not too large to fit into the reserve
	pcloud::Scene *scene = sceneFromHandle( meta->sceneHandle );	// would fail if ptReadPODMeta was used

	if (!scene && meta->filepath.isEmpty()) 
		return setLastErrorCode( PTV_INVALID_HANDLE );

	ptds::FilePath filepath = scene ? scene->filepath() : meta->filepath;

	// close the file if it is already in use
	if (scene)
	{
		thePointsPager().pause();
		thePointsPager().closeSceneFile(scene);
	}
	
	bool retval = setLastErrorCode( PTV_SUCCESS ) != 0;
	
	// reopen read write
	ptds::DataSourcePtr dataSrc = ptds::DataSourceManager().openForReadWrite( &filepath );

	if (dataSrc)
	{
		// write the meta data
		pcloud::PodJob pod(0, filepath);
		pod.h = dataSrc;
		
		ptds::Tracker t(pod.h);
		pod.tracker = &t;
		
		if (!PodIO::writeMetaUpdate( pod, meta->mdata ))
		{
			retval = setLastErrorCode( PTV_FILE_WRITE_FAILURE ) != 0;
		}
		dataSrc->close();			
	}
	else 
		retval = setLastErrorCode ( PTV_FILE_WRITE_FAILURE ) != 0;
	
	if (scene)
	{
		thePointsPager().reopenScene(scene);
		thePointsPager().unpause();
	}

	return retval;
}
//--------------------------------------------------------------------------------------------------
// retur nnumber of user sections
//--------------------------------------------------------------------------------------------------
PTint		PTAPI	ptNumUserMetaSections( PThandle h )
{
	FileMetaMap::iterator i = m_fileMeta.find( h );
	if (i == m_fileMeta.end()) return 0;

	FileMeta *meta = i->second;

	return meta->mdata.user.numGroups();
}
//--------------------------------------------------------------------------------------------------
// return the user section name
//--------------------------------------------------------------------------------------------------
const PTstr		PTAPI	ptUserMetaSectionName( PThandle h , PTint section_index )
{
	FileMetaMap::iterator i = m_fileMeta.find( h );
	if (i == m_fileMeta.end()) return 0;

	FileMeta *meta = i->second;

	static pt::String name;
	if (!meta->mdata.user.groupName( section_index, name )) return 0;

	return name.c_wstr();
}
//--------------------------------------------------------------------------------------------------
// return num of user meta tags within a section
//--------------------------------------------------------------------------------------------------
PTint		PTAPI	ptNumUserMetaTagsInSection( PThandle h , PTint section_index )
{
	FileMetaMap::iterator i = m_fileMeta.find( h );
	if (i == m_fileMeta.end()) return 0;

	FileMeta *meta = i->second;	

	pt::String section;

	if (!meta->mdata.user.groupName( section_index, section )) return 0;
	return meta->mdata.user.numTagsInGroup( section );
}
//--------------------------------------------------------------------------------------------------
// get user meta data tag by its index
//--------------------------------------------------------------------------------------------------
PTres		PTAPI	ptGetUserMetaTagByIndex( PThandle h , PTint section_index, PTint tag_index, PTstr name, PTstr value )
{
	FileMetaMap::iterator i = m_fileMeta.find( h );
	if (i == m_fileMeta.end()) 
		return setLastErrorCode( PTV_INVALID_HANDLE );

	if (!value)
		return setLastErrorCode( PTV_VOID_POINTER );

	FileMeta *meta = i->second;

	pt::String sname, svalue;

	if (!meta->mdata.user.getMetaTag( section_index, tag_index, sname, svalue )) 	
	{
		return setLastErrorCode( PTV_METATAG_NOT_FOUND );
	}
	
	if (!sname.length() || !svalue.length()) 
	{
		return setLastErrorCode( PTV_METATAG_EMPTY );
	}

	wcsncpy( name, sname.c_wstr(), PT_MAX_META_STR_LEN );
	wcsncpy( value, svalue.c_wstr(), PT_MAX_META_STR_LEN );

	return setLastErrorCode( PTV_SUCCESS );
}
//--------------------------------------------------------------------------------------------------
// get a user meta tag by name in the form "section.Name"
//--------------------------------------------------------------------------------------------------
PTres		PTAPI	ptGetUserMetaTagByName( PThandle h , const PTstr sectionDotName, PTstr value )
{
	FileMetaMap::iterator i = m_fileMeta.find( h );
	if (i == m_fileMeta.end())
		return setLastErrorCode( PTV_INVALID_HANDLE );

	if (!value)
		return setLastErrorCode( PTV_VOID_POINTER );

	FileMeta *meta = i->second;

	wchar_t tagName_str[PT_MAX_META_STR_LEN];
	wcsncpy(tagName_str, sectionDotName, PT_MAX_META_STR_LEN);
	
    wchar_t * context;
	wchar_t *first_tok = wcstok( tagName_str, L".", &context);
	wchar_t *second_tok = wcstok(nullptr, L".", &context);

	pt::String tsection, titem, tvalue;

	if (!second_tok)
	{
		tsection = pt::String("Any");
		titem = pt::String( first_tok );
	}
	else 
	{
		tsection = pt::String(first_tok);
		titem = pt::String( second_tok );
	}

	if (meta->mdata.user.getMetaTag( tsection, titem, tvalue ))
	{
		wcsncpy( value, tvalue.c_wstr(), PT_MAX_META_STR_LEN ); 
		return setLastErrorCode( PTV_SUCCESS );
	}
	value[0] = 0;
	return setLastErrorCode( PTV_METATAG_NOT_FOUND );
}
