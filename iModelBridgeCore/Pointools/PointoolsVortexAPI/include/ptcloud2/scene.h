#ifndef POINTOOLS_PCLOUD2_SCENE
#define POINTOOLS_PCLOUD2_SCENE

#include <pt/ptstring.h>
#include <ptcloud2/pointcloud.h>
#include <ptcloud2/scanpos.h>
#include <ptcloud2/indexstream.h>
#include <pt/scenegraph.h>

#include <ptcloud2/metatags.h>
#include <ptcloud2/metadata.h>

namespace pcloud
{
	class PodJob;
	class IncNode;

	class PCLOUD_API Scene : public pt::Scene3D
	{
	public:
		static Scene *createFromPntsStream( IndexStream *pntsStream, int &error, float uniform_filter_spacing = -1 );
		static Scene *createFromFile( const ptds::FilePath &path );
		static const char* creationErrorText( int errorCode );

		~Scene();
		
		enum CreateSceneResult
		{	
			ReadPntStreamFailure =	-100,
			OutOfMemory =			-101,
			FileWriteError =		-102,
			NoPointsInStream =		-103,
			ReadStreamError =		-104,
			CantOpenPODFile =		-105,
			InvalidPODFile =		-106,
			PODVersionNotHandled =	-107,
			Success =				0
		};

		/*management*/ 
		PointCloud* newCloud( int64_t guid = 0 );

		void addCloud(PointCloud* cloud);
		void removeCloud(PointCloud* cloud);
		void deleteCloud(PointCloud* cloud);
		

		/* access */ 
		inline const PointCloud *cloud(int i) const			{ return _clouds[i]; }
		inline PointCloud *cloud(int i)						{ return _clouds[i]; }
		
		inline const pt::Object3D* object(int i) const		{ return cloud(i); }
		inline pt::Object3D* object(int i)					{ return cloud(i); }
		inline int numObjects() const						{ return (int)_clouds.size(); }

		inline PointCloud *operator [] (int i)				{ return _clouds[i]; }
		inline const PointCloud *operator [] (int i) const	{ return _clouds[i]; }

		inline PointCloud*	findCloud( const PointCloudGUID &guid )	
		{
			std::map<PointCloudGUID, PointCloud*>::iterator i = 
				_cloudsByGUID.find( guid );
			return i == _cloudsByGUID.end() ? 0 : i->second;
		}

		inline const PointCloud*	findCloud( const PointCloudGUID &guid ) const	
		{
			std::map<PointCloudGUID, PointCloud*>::const_iterator i = 
				_cloudsByGUID.find( guid );
			return i == _cloudsByGUID.end() ? 0 : i->second;
		}

		inline uint size() const							{ return _clouds.size(); }
		void clear();
		
		const wchar_t *typeDescriptor() const	{ return L"Point Cloud"; }
		const char	*className() const			{ return s_className(); }
		static const char*s_className()			{ return "Point Scene"; }

		/*scan positions*/ 
		ScanPosition* addScanPosition(const mmatrix4d &mat);

		const ScanPosition *scanPos(int i) const		{ return _scanpositions[i]; }
		ScanPosition *scanPos(int i)					{ return _scanpositions[i]; }

		int		numScanPositions() const				{ return (int)_scanpositions.size(); }

		MetaData &metaData()				{ return _metadata; }
		const MetaData &metaData() const	{ return _metadata; }

		bool	loaded() const						{ return _loaded; }
		void	unload();
		void	reload();
		void	setInstanceIndex( int instance );
		int		getInstanceIndex() const			{ return _instance; }

		int64_t	fullPointCount() const;
		int64_t lodPointCount() const;

		pt::Guid	objectGuid() const;

		int		editStateID() const;
		void	setEditStateID( int id );

	private:

		Scene( IndexStream *stream, int &error, float uniform_filter_spacing = -1 );
		Scene( const ptds::FilePath &stream );

		struct SceneBuildData
		{
			IndexStream *stream;
			float uniformFilterSpacing;
			PodJob *job;
			int cloudIndex;
			bool transform;
			bool combine;
			int startCloud;
			int endCloud;

			IndexStream::CloudInfo *cloudInfo() { return stream->cloudInfo( cloudIndex ); }
		};

		int		readSinglePass( SceneBuildData *buildData );
		int		readMultiPass( SceneBuildData *buildData );
		
		int		buildPreliminaryOctree( IncNode *root, int initial_depth, int target_depth, SceneBuildData *buildInfo );
		int		extractCloudsFromStream( SceneBuildData *buildData );
		int		buildCloudGroupFromStream( SceneBuildData *buildData );

		int		writePointData( SceneBuildData *buildData, PointCloud *pc );
		void	updateBB();
		void	calcBounds();

		std::vector<PointCloud*>				_clouds;
		std::map<PointCloudGUID, PointCloud*>	_cloudsByGUID;
		std::vector<ScanPosition*>				_scanpositions;
		std::set<PointCloud*>					_tmpclouds;
		std::set<pt::String>					_keywords;

		MetaData								_metadata;
		
		bool									_loaded;
		float									_accuracy;
		int										_instance;
		int										_editStateID;
	};

    // Custom literals used by the scene
    constexpr unsigned long long operator"" _Kilo(unsigned long long val)
        {
        return val * 1000;
        }
    constexpr unsigned long long operator"" _Million(unsigned long long val)
        {
        return val * 1000000;
        }
    constexpr unsigned long long operator"" _Billion(unsigned long long val)
        {
        return val * 1000000000;
        }

}
#endif
