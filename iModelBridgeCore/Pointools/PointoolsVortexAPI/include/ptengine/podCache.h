#pragma once

#include <ptcloud2/pointcloud.h>
#include <ptds/DataSourceFile.h>

// Not adapted for DataSource yet

namespace vortex
{
	class PODCacheManager
	{
	public:
		PODCacheManager();
		virtual ~PODCacheManager();

		bool setCacheFolder( const ptds::DataSource &folder );
		
		bool createCacheFile( const pcloud::Scene *pod, uint maxNumBytes = 52428800); // max file size defaults to 50mb
		bool hasCacheFile( const pcloud::Scene *pod ) const;
		bool readCacheFile( const pcloud::Scene *pod );

	private:
		ptds::FilePath  generateCacheFilename( const pcloud::Scene *pod ) const;
		ptds::FilePath	m_folder;
	};

}