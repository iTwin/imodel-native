/*----------------------------------------------------------*/ 
/* Indexstream.h											*/ 
/* Point Cloud import data stream							*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004-2008						*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#ifndef PCLOUD2_POINTCLOUD_INDEXSTREAM
#define PCLOUD2_POINTCLOUD_INDEXSTREAM 1

#include <vector>

#include <ptcloud2/pcloud.h>
#include <ptcloud2/voxel.h>
#include <ptcloud2/datachannel.h>
#include <ptcloud2/pcimage.h>
#include <ptcloud2/metatags.h>
#include <ptcloud2/metadata.h>
#include <pt/scenegraph.h>
#include <ptfs/filepath.h>
#include <math/matrix_math.h>

#include <ptfs/iohelper.h>

#define PCLOUD_CLOUD_ORDERED		0x01
#define PCLOUD_CLOUD_ROW_MAJOR		0x02
#define PCLOUD_CLOUD_RGB			0x04
#define PCLOUD_CLOUD_INTENSITY		0x08
#define PCLOUD_CLOUD_NORMAL			0x10
#define PCLOUD_CLOUD_CLASSIFICATION	0x20

#define PCLOUD_STREAM_BLOCKSIZE		262144

namespace pcloud { class PointStreamFilter; }

namespace pcloud
{
	class PointCloud;

	class PCLOUD_API IndexStream
	{

public:

	static pcloud::PointStreamFilter *globalPointStreamFilter;

	public:
		IndexStream();
		~IndexStream();

		struct CloudInfo
		{
			int64_t		numPoints;
			uint		spec;
			uint		bytesPerPoint;
			uint		ibound;
			uint		jbound;
			pt::BoundingBoxD bounds;
			pt::BoundingBox xbounds;
			int64_t		imagePointer;
			
			mmatrix4d	 matrix;
			pt::vector3d	offset;
			pt::vector3i truncation;

			uint		res0;
			uint		res1;
			uint		res2;
			uint		res3;

			bool hasRGB() const				{ return spec & PCLOUD_CLOUD_RGB ? true : false; }
			bool hasIntensity() const		{ return spec & PCLOUD_CLOUD_INTENSITY ? true : false; }
			bool hasNormals() const			{ return spec & PCLOUD_CLOUD_NORMAL ? true : false; }
			bool hasClassification() const	{ return spec & PCLOUD_CLOUD_CLASSIFICATION ? true : false; }
			bool isOrdered() const			{ return spec & PCLOUD_CLOUD_ORDERED ? true : false; }

			uint		_orderedpos;
			uint		_cloudPosInScene;

			wchar_t		name[PT_IDENTIFIER_LENGTH];

			std::vector<ScanImage>	images;
		};

		/* cloud group ---------------------------------------------*/  
		/* contains clouds of same specification and can be used to */ 
		/* combine clouds/scans into a single cloud                 */ 
		struct CloudGroup
		{
			/* clouds */ 
			std::vector<CloudInfo*> clouds;

			/* compression tolerance */ 
			float tolerance;
			
			/* combine data into single cloud */ 
			bool  combine;

			/* generate normals flag*/ 
			bool  generate_normals;

			/* normal quality, 0 to 1.0 */ 
			float normal_quality;
			
			/* transformed bounds of group */ 
			pt::BoundingBox xbounds;

			/* cloud pos in new Scene */ 
			uint _cloudPosInScene;

			/* name of group used as cloud name */ 
			wchar_t		name[PT_IDENTIFIER_LENGTH];
		};

		/* Add some meta data into header*/ 
		MetaData &metaData()							{ return _metaData; }

		/* Add a callibrated image to the last cloud or scan position */ 
		void	addCalibratedImageToCloud(const char *filename, const mmatrix4d &extrinsic, 
			double cx, double cy, double fx, double fy, 
			double k1=0, double k2=0, double k3=0, double k4=0, 
			double p1=0, double p2=0);

		void	addCallibratedImageToCloud(const ScanImage &si);

		/* Add a cloud group to the database - there must be at least 1 group */ 
		int 	addGroup(bool combine, float tolerance, bool gen_normals, float normal_quality=0.3f, const wchar_t*name=0);
		
		/* Add a cloud to the group */ 
		bool	addCloud(uint cloud_spec, const mmatrix4d *mat=0, uint ibound=0, uint jbound=0, const wchar_t*name=0);

		/* Add a point to the cloud */ 
		bool	addPoint(const pt::vector3 &geom, const ubyte *rgb, const short *intensity, const pt::vector3s *normal, const ubyte *classification)
		{
			pt::vector3d geomd(geom);
			return addPoint(geomd, rgb, intensity, normal, classification);
		}

		/* Add a point to the cloud */ 
		bool	addPoint(const pt::vector3d &geom, const ubyte * const rgb, const short * const intensity, const pt::vector3s * const normal, const ubyte * const classification);
		
		/* Add a null point to retain grid order */ 
		void	addNull();

		/* Multiple pass entry for input - used for rgb / intensity from different sources*/ 
		void	restartCloudPass();
		void	addPassPointColour(const ubyte *rgb);
		void	addPassPointIntensity(const short *intensity);

		/* Rescale intensity flag */ 
		void rescaleIntensities()						{ _rescaleIntensities = true; }

		/* Start the writing stream. <filename> temporary file location */ 
		bool startStream(const wchar_t *filename);

		/* End the writing stream */ 
		bool closeStream();

		/* Start the reading stream */ 
		bool startReadStream();

		/* Close the reading stream */ 
		void closeReadStream();

		/* Get the file stream position, useful to check for fat32 4Gb limit failure */ 
		int64_t writeStreamPosition();

		/* read cloud - starts reading a clouds data */ 
		const CloudInfo *readCloud(int idx);

		/* get point info - returns position in read*/ 
		int  readPoint(pt::vector3 &geom, ubyte *rgb, short *intensity, pt::vector3s *normal, ubyte *classification=0); 

		/* num of clouds in the group or total? */ 
		int  numClouds() const											{ return (int)_clouds->size(); }

		/* reset the read pointer back to the begining of the cloud */ 
		void resetReadPointer();

		/* generate normals for a cloud group */ 
		void generateNormals();

		/* set filepath */
		void setFilepath(const ptds::FilePath &path)					{_filePath = path;}

		/* filepath of the stream file */ 
		const ptds::FilePath &filepath() const { return _filePath; }

		void setStructuredStorageFilePath(const ptds::FilePath &path)	{_structuredStorageFilePath = path;}

		const ptds::FilePath *getStructuredStorageFilePath(void)
		{
			if(_structuredStorageFilePath.isEmpty() == false)
				return &_structuredStorageFilePath;

			return NULL;
		}

		/* cloud information by cloud index */ 
		const CloudInfo *cloudInfo(int idx) const						{ return (*_clouds)[idx]; }
		CloudInfo *cloudInfo(int idx)									{ return (*_clouds)[idx]; }

		/* Current groups transformed bounds */ 
		const pt::BoundingBox &xbounds() const							{ return _group->xbounds; }

		/* Start reading a group */ 
		CloudGroup *setGroup(int g);

		/* num of groups in total */ 
		int numGroups() const											{ return _groups.size(); }

		/* get group information */ 
		const CloudGroup *group() const									{ return _group; } 

		void setRootStream(bool isRoot);
		bool getRootStream(void) const;

		bool applyPointStreamFilter(void) const;

		void setAddedPointCount(uint64_t v)
		{
			addedPointCount = v;
		}

		uint64_t getAddedPointCount(void) const
		{
			return addedPointCount;
		}

		uint64_t getNumCloudPoints() const;

	private:
		void checkWriteBuffer();
		void checkReadBuffer();
		void buildNormals(bool transform, float quality = 0.3f);
		bool _buildNormals(int cloud_idx, pt::vector3s*normals, bool transform, float quality);
		
		bool _rescaleIntensities;
		pt::Bounds<1, int> _intensityBounds;
		unsigned int _intensityHist[260];

		bool _rootStream;

		class PointsImage
		{	
		public:
			PointsImage(int i, int j, int cloud_idx, bool writeTranspose = false)
			{
				_transpose = writeTranspose;
				_i = i;
				_j = j;
				_cloud_idx = cloud_idx;
				_data = new int[i*j];
				memset(_data, -1, sizeof(int)*_i*_j);
			}
			~PointsImage() { delete [] _data; }
			int validPoints() 
			{ 
				int count = 0;
				for (int i=0; i<_i*_j; i++) if (_data[i]>=0) count++; 
				return count;
			}
			inline const int	pos(const int &i, const int &j) const
			{
				return (i*_j + j);
			}
			inline const int	&get(const int &i, const int &j) const { return _data[pos(i,j)]; }
			inline void set(const int &i, const int &j, const int &p)  { _data[pos(i,j)] = p; }

			inline void set(const int &i, const int &p)  
			{ 
				if (_transpose) 
					_data[_j*(i%_i)+(i/_i)] = p;
				else
					_data[i] = p; 
			}
			inline const int	&get(const int &i) const				{ return _data[i]; }
			
			int getNeighbourhood(int i, int *points, int size)
			{
				int ip = i / _j;
				int jp = i % _j;
				return getNeighbourhood(ip, jp, points, size);
			}
			int getNeighbourhood(int i, int j, int* points, int size)
			{
				int hsize = size / 2;
				
				int i1 = i + hsize;
				int j1 = j + hsize;
				i -= hsize;
				j -= hsize;

				if (i<0) i = 0;
				if (j<0) j = 0;
				if (i1 >= _i) i1 = _i - 1;
				if (j1 >= _j) j1 = _j - 1;
				
				int count = 0;

				for (int a=i; a<i1; a++)
					for (int b=j; b<j1; b++)
						if (get(a,b)>=0) { points[count++] = get(a,b); }
				return count;
			}
			int *data() { return _data; }
			int ibound () const { return _i; }
			int jbound () const { return _j; }
			int cloud_idx() const { return _cloud_idx; }
		private:
			int _i;
			int _j;
			int *_data;
			int _cloud_idx;
			bool _transpose;
		};

		bool readImage(PointsImage *image, int cloud_idx);
		bool writeImage(PointsImage *image);
		
		ptds::BlockPager			*_pager;
		ptds::BlockPager			*_normalpager;
		ptds::WriteBlock			*_writeblock;
		ptds::ReadBlock				*_readblock;
		ptds::ReadBlock				*_normalreadblock;

		ptds::FilePath				_filePath;
		ptds::FilePath				_structuredStorageFilePath;

		int							_pointBytes;
		uint						_cloudSpec;

		CloudInfo					*_cloud;
		CloudGroup					*_group;
		int							_groupidx;
		int							_truncationMultiplier;

		PointsImage					*_image;
		std::vector<CloudGroup*>	_groups;
		std::vector<CloudInfo*>		*_clouds;

		MetaData					_metaData;

		uint64_t			addedPointCount;

		pcloud::PointStreamFilter	*_pointStreamFilter;
	};
}
#endif