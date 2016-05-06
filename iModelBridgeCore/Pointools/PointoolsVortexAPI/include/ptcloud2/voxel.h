/*----------------------------------------------------------*/ 
/* Voxel.h													*/ 
/* Voxel Interface file										*/ 
/*----------------------------------------------------------*/ 
/* (c) Copyright Pointools 2004								*/   
/*----------------------------------------------------------*/ 
/* Written by Faraz Ravi									*/ 
/*----------------------------------------------------------*/ 
#ifndef POINTCLOUD_VOXEL
#define POINTCLOUD_VOXEL 1

#include <ptcloud2/pcloud.h>
#include <ptcloud2/node.h>
#include <ptcloud2/datachannel.h>
#include <pt/scenegraph.h>
#include <ptds/DataSourceReadSet.h>
#include <ptds/DataSource.h>
#include <ptengine/VoxelLODSet.h>

#include <omp.h>						// OpenMP multi-threading support

#include <PTRMI/Status.h>

#define NUM_DATA_CHANNELS			7	// total num of potential data channels
#define PT_VOXEL_MAX_PROXY_POINTS	8	// max num of proxy values in channel
#define NUM_STRATA					12	// max num of strata in point ordering

#ifndef POINT_SELECT_CODE
#define POINT_SELECT_CODE			128	// in edit/filter channel the bit that indicates selection
#endif

#define NUM_ITERATION_THREADS		4	// number of threads used via OpenMP for voxel MT points interation methods
#define	LOAD_BLOCK_SIZE				65536

#define LOD_MAX						1
#define LOD_ZERO					0
#define LOD_USE_MINIMUM				1.0f / 255.0f

namespace pcloud
{
	class PointCloud;


	/*------------------------------*/ 
	/*       leaf voxel class       */ 
	/*------------------------------*/ 
	class PCLOUD_API Voxel : public Node
	{	
	public:

		typedef float			LOD;

		typedef int				FileIndex;

		const static FileIndex	FILE_INDEX_NULL = 0x7FFFFFFF;

		enum LODComparison
		{
			LODGreater,
			LODLess,
			LODEqual
		};

	protected:
						Voxel				(void);

	public:
		Voxel( float *lower, float *upper, int deep, const pt::BoundingBoxD &ext, uint pointcount );
		
		Voxel(const Voxel &v);
		~Voxel();

		/* access to voxel data channels, may return null if channel not found */ 
		inline const DataChannel *channel(int ch) const { return _channels[ch]; }
		inline const DataChannel *channel(Channel ch) const { return _channels[ch]; }
		inline DataChannel *channel(int ch) { return _channels[ch]; }
		inline DataChannel *channel(Channel ch) { return _channels[ch]; }

		/*data channel construction*/ 
		void addChannel(Channel ch, DataType native, DataType storeas, int multiple = 1, int allocate = 0, double *offset = 0, double *scaler = 0, const void *samples=0, uint num_samples =0);

		/* used during indexing only */ 
		void readChannel(Channel ch, int count, void* data);
		inline uint64 size() const { return lodPointCount(); }

		/* resize in-core allocation for channel to the requested lod */ 
		void* resizeChannelToRequestedLod(Channel ch, float amount, int &num_points, uint &num_bytes, uint &bytes_offset, bool multRequest = true);

		int resizeAndFillChannelToRequestedLod(Channel ch, float amount, int &num_points, uint &num_bytes, uint &bytes_offset, 
			ptds::DataSourcePtr &dataSrc, int64_t srcPos, void* buffer);

		int getChannelResizeReadInfo(Channel ch, float amount, int &num_points, uint &num_bytes, uint &bytes_offset, int64_t &srcPos);

		
		/* returns info on channel in relation to requested lod */ 
		void lodRequestSizeInfo(Channel ch, float amount, int &num_points, uint &num_bytes, uint &bytes_offset);

		/* total number of additional bytes requested in request lod */ 
		int requestBytes() const;

		/* dump data from data channels, does not remove channels */ 
		void clearChannels();

		/* node override */ 
		bool isLeaf() const { return true; }

		/* compute AA extents of point data in leaf */ 
		void computeExtents();

		void setOriginalExtents(pt::BoundingBoxD& ext) { _origextents = ext; }

		/* returns success, memory allocation failure would cause failure */ 
		bool initializeChannels(int newsize=-1 /* ie don't resize */);
		
		/* build the edit channel to store point selection + later state */ 
		void buildEditChannel(ubyte default_value = 0);

		/* destroy the in-core edit channel, will dump data + delete channel object */ 
		void destroyEditChannel();

		/* full point cloud for this leaf node */ 
		inline uint64 fullPointCount() const			{ return _pointCount; }

		/* current lod point count for this leaf node */ 
		inline uint64 lodPointCount() const				{return getNumPointsAtLOD(getCurrentLOD());}
		
		/* parent point cloud object */ 
		void pointCloud(const PointCloud*pc)			{ _pointcloud = pc; }

		/* parent point cloud object */ 
		inline const PointCloud*	pointCloud() const	{ return _pointcloud; }

		/* index of this leaf node in the point clouds leaf vector */ 
		const uint &indexInCloud() const { return _index; }

		/* loading priority for this leaf */ 
		inline float priority() const	{ return _priority; }
		inline void priority(float p)	{ _priority = p; _usage *= 0.25f; _usage += p * 0.75f; }
		
		/* linear nearest point search */ 
		int findNearestPoint(const pt::vector3d &pnt, pt::vector3d &nearest, double &dist) const;

		/* node mutex for sync access, not used for POD Writer library to reduce dependencies */ 
#ifndef POINTOOLS_POD_API
		inline std::mutex &mutex()  { return _mutex; }
#endif
		/* extent of last editing progress as number of points */ 
		inline uint numPointsEdited() const		{ return _numPointsEdited; }
		inline void numPointsEdited(uint pnt)	{ _numPointsEdited = pnt; }

		/* lod Request request*/ 

		void				setRequestLOD				(LOD lod);
		LOD					getRequestLOD				(void) const;
		void				clipRequestLODMax			(LOD maxLOD);
		LOD					getRequestLODMax			(void) const;
		void				clearRequestLODs			(void);
		unsigned int		getNumRequestLODs			(void);
		bool				removeRequestLOD			(pointsengine::VoxelLODSet::ClientID id);
		inline void			setCurrentLOD				(LOD lod)						{m_lod = lod;}
		inline float		getCurrentLOD				(void) const					{return m_lod;}
		inline void			setStreamLOD				(LOD lod)						{m_stream_lod = lod;}
		inline float		getStreamLOD				(void)							{return m_stream_lod;}
		inline void			setPreviousLOD				(LOD lod)						{m_previous_lod = lod;}
		inline float		getPreviousLOD				(void)							{return m_previous_lod;}

		unsigned int		getNumPointsAtLOD			(LOD lod) const;

		unsigned int		getNumChannels				(void);

		ptds::DataSource::DataSourceForm getDataSourceForm	(void) const;

		unsigned int		getVoxelPointStorageSize	(void);
		unsigned int		getNumPointsWithBudget		(ptds::DataSize budgetBytes, ptds::DataSize &budgetNotUsed);
		float				getPotentialLOD				(int64_t numPointsChanged);

		unsigned int		getVoxelDataSourceReadSet	(LOD lod, ptds::DataSourceReadSet &readSet);
		void				getChannelDataSourceRead	(DataChannel &dataChannel, int64_t position, LOD lod, ptds::DataSourceReadSet &readSet);

		LODComparison		compareLOD					(LOD a, LOD b);

		void				setVoxelRGB					(unsigned char *rgb);

		float				am;
		bool				new_ooc;

		pointsengine::VoxelLODSet	voxelLODSet;

		/* usage metric, used in lru computation - suspect redundancy */ 
		float usage() const { return _usage; }

		/* pointer to out-of-core channel data */ 
		void	filePointer(int64_t pos)	{ m_filepointer = pos; }
		int64_t filePointer() const			{ return m_filepointer; }

		/* file index in scene */ 
		FileIndex	fileIndex	(void) const			{return m_fileindex;}
		void		fileIndex	(FileIndex	filei)		{ m_fileindex = filei; }

		/* query */ 
		bool	pointPosition(unsigned int pntIndex, pt::vector3 &localpos) const;
		bool	pointRGB(unsigned int pntIndex, unsigned char *rgb) const;
		bool	pointIntensity(unsigned int pntIndex, short &intensity) const;
		bool	pointNormal(unsigned int pntIndex, pt::vector3s &normal) const;

		/* density metrics - computes fraction of point count required for uniform density at spacing */ 
		float	densityValue() const				{ return (float)_strataSize[0] / _pointCount; }
		int		strataSize( int strata ) const		{ return _strataSize[strata]; }
		int		strataPos( int strata ) const		{ int pos=0; for (int i=0; i<strata-1; i++) pos += _strataSize[i]; return pos; }
		float	strataPosToLod( float strata ) const;
		float	strataSpacing() const				{ return _strataSpacing; }

		void setResizedToStream(bool resized)
		{
			resizedToStream = resized;
		}

		bool getResizedToStream(void)
		{
			return resizedToStream;
		}

		//---------------------------------------------------------------------
		/**
			Local Space Transform is a simple pass through
		*/
		struct LocalSpaceTransform
		{
			void prepare(){}
			template <class T, class T2>
			inline void transform(const T &v0, T2 &v1){ v1.x = v0.x; v1.y = v0.y; v1.z = v0.z; };
		};
		//---------------------------------------------------------------------
		/**
			Coordinate Space Transform
		*/
		struct CoordinateSpaceTransform
		{
			CoordinateSpaceTransform(PointCloud *cl, pt::CoordinateSpace cs = pt::ProjectSpace)	{
				cloud = cl;
				cspace = cs;
			}
			PCLOUD_API void prepare();

			inline void transform(const pt::vector3 &v0, pt::vector3d &v1) {
				m.vec3_multiply_mat4(pt::vector3d(v0), v1);
			}
			inline void transform(const pt::vector3 &v0, pt::vector3 &v1) {
				m.vec3_multiply_mat4f(v0, v1);
			}
			inline void transform(const pt::vector3d &v0, pt::vector3d &v1) {
				m.vec3_multiply_mat4(v0, v1);
			}
			PointCloud *cloud;
			mmatrix4d m;
			pt::CoordinateSpace cspace;
		};
		//---------------------------------------------------------------------
		/**
			iterateFullTransformedPoints iterates through points with all channels returned to Receiver
		*/
		template <class Receiver>
		void iterateFullTransformedPoints(Receiver &R, pt::CoordinateSpace cs = pt::ProjectSpace, ubyte layerMask=0)
		{
			CoordinateSpaceTransform cst(const_cast<PointCloud*>(_pointcloud),  cs);
			_iterateFullTransformedPoints(R, cst, layerMask);
		}
		//---------------------------------------------------------------------
		/**
			iterateTransformedPoints iterates through points with minimal channels returned to Receiver
		*/
		template <class Receiver>
		void iterateTransformedPoints(Receiver &R, pt::CoordinateSpace cs = pt::ProjectSpace, 
			ubyte layerMask=0, float amount=1.0f)
		{
			CoordinateSpaceTransform cst(const_cast<PointCloud*>(_pointcloud),  cs);
			unsigned int num_points = getNumPointsAtLOD(amount);
			if (layerMask && numPointsEdited())
			{
				if (num_points > numPointsEdited()) num_points = numPointsEdited();
			}
			_iterateTransformedPointsRange(R, cst, layerMask, 0, num_points);
		}
		//---------------------------------------------------------------------
		/**
			iterateTransformedPointsRange iterates through points range
		*/
		template <class Receiver>
		void iterateTransformedPointsRange(Receiver &R, pt::CoordinateSpace cs = pt::ProjectSpace, 
			ubyte layerMask=0, uint start_point=0, uint end_point=0)
		{
			CoordinateSpaceTransform cst(const_cast<PointCloud*>(_pointcloud),  cs);
			_iterateTransformedPointsRange(R, cst, layerMask, start_point, end_point);
		}
		//---------------------------------------------------------------------
		/**
			_iterateTransformedPoints4Threads iterates through points range using 4 threads (omp)
		*/
		template <class Receiver, class Transformer>
		void _iterateTransformedPoints4Threads(Receiver &R, Transformer &T, 
			ubyte layerMask=0, uint start_point=0, uint end_point=0)
		{
			pt::vector3 pnt1;
			pt::vector3d pnt1d;

			T.prepare();
			ubyte nullFilter = 0xff & ~POINT_SELECT_CODE;

			int index = 0;
			ubyte *f_b=0, *f_e=0;

			pcloud::DataChannel *geom = _channels[pcloud::PCloud_Geometry];
			pcloud::DataChannel *layer = _channels[pcloud::PCloud_Filter];

			unsigned int currentNumPoints = getNumPointsAtLOD(getCurrentLOD());

			if (layer && layer->size() >= currentNumPoints) 
			{ 
				layer->begin(&f_b); 
				layer->end(&f_e); 
			}
			else layer = 0;

			if (geom->typesize() == 4)
			{
				_iterateTransformedPointsRange(R, T, layerMask, start_point, end_point);
				return;
			}
			else
			{
				pt::vector3s *i, *e;
				pt::vector3d pnt;
				pt::vector3d scaler, offset;

				scaler.set(geom->scaler());
				offset.set(geom->offset());

				if (!end_point)
				{
					end_point = currentNumPoints;
				}
				else
				if (end_point > currentNumPoints)
				{
					end_point = currentNumPoints;
				}
				
				if (start_point > end_point) return;

				int num_points = end_point - start_point;//geom->size();
				if (!num_points) return;

				int tid;

				#pragma omp parallel if (num_points > 1024) private(tid, i, e, pnt, pnt1d, index, f_b) num_threads(NUM_ITERATION_THREADS)
				{	
					int nthreads = omp_get_num_threads();
					int points_per_thread = num_points / nthreads;
					int points_first_thread = points_per_thread + num_points % points_per_thread;

					tid = omp_get_thread_num();

					if (tid)
					{
						index = (tid-1) * points_per_thread + points_first_thread + start_point;
						geom->getptr( &i, index );
						geom->getptr( &e, tid * points_per_thread + points_first_thread );

						if (layer) layer->getptr( &f_b, index );
					}
					else
					{
						index = 0;
						geom->getptr( &i, start_point );
						geom->getptr( &e, points_first_thread + start_point );

						if (layer) layer->getptr( &f_b, start_point );
					}

					if (layer)
					{
						while (i < e && index < num_points)
						{
							pnt.x = i->x; pnt.y = i->y; pnt.z = i->z;
							pnt *= scaler; pnt += offset;

							if (!layerMask || *f_b & layerMask)
							{
								T.transform(pnt, pnt1d);
								R.mt_point(tid, pnt1d, index, *f_b);
							}
							
							++i; ++index; ++f_b;
						}
					}
					else
					{
						while (i < e && index < num_points)
						{
							pnt.x = i->x; pnt.y = i->y; pnt.z = i->z;
							pnt *= scaler; pnt += offset;

							T.transform(pnt, pnt1d);
							R.mt_point(tid, pnt1d, index, nullFilter);
							
							++i; ++index;
						}
					}
				}
			}
		}
		//---------------------------------------------------------------------------
		template <class Receiver, class Transformer>
		void _iterateTransformedPoints(Receiver &R, Transformer &T, ubyte layerMask=0, float amount=1.0f)
		{
			pcloud::DataChannel *geom = _channels[pcloud::PCloud_Geometry];

            uint end = static_cast<uint>(amount * getNumPointsAtLOD(getCurrentLOD()));

			_iterateTransformedPointsRange( R, T, layerMask, 0, end );
		}
		//---------------------------------------------------------------------------
		template <class Receiver, class Transformer>
		void _iterateTransformedPointsRange(Receiver &R, Transformer &T, ubyte layerMask=0, 
			uint start_point=0, uint end_point=0)
		{
			T.prepare();

			//if(start_point == end_point)
			//{
			//	return;
			//}

			int index = start_point;
			ubyte *f_b=0, *f_e=0;
			ubyte nullFilter = 0xff & ~POINT_SELECT_CODE;
			ubyte pnt_layer = layers(0);
			pcloud::DataChannel *geom = _channels[pcloud::PCloud_Geometry];
			pcloud::DataChannel *layer = _channels[pcloud::PCloud_Filter];

			pt::vector3d pntc, pntt, scaler, offset;
			scaler.set(geom->scaler());
			offset.set(geom->offset());

			unsigned int currentNumPoints = getNumPointsAtLOD(getCurrentLOD());

			if(!end_point || end_point > currentNumPoints)
			{
				end_point = currentNumPoints;
			}


			pt::vector3d pnt;


			if (layer && layer->size() >= currentNumPoints) 
			{ 
				layer->begin( &f_b );
				f_b += start_point;
				layer->end(&f_e); 
			}
			else layer = 0;

			if (geom->typesize() == 4)
			{
				pt::vector3 *i, *e;

				geom->begin(&i);
				i += start_point;
				e = (pt::vector3*)geom->element(end_point);

				if(!layer)
				{
					while (i < e)
					{
						pntc.set( *i );
						pntc *= scaler;
						pntc += offset;

						T.transform(pntc, pntt);
						R.point(pntt, index, pnt_layer);	
						++i;
						++index;
					}
				}
				else
				{
					if (layerMask)
					{
						while (i < e)
						{
							if (f_b >= f_e || *f_b & layerMask)
							{
								pntc.set( *i );
								pntc *= scaler;
								pntc += offset;
						
								T.transform(pntc, pntt);
								R.point(pntt, index, *f_b);
							}
							++i;
							++f_b;
							++index;
						}
					}
					else
					{
						while (i < e)
						{
							pntc.set( *i );
							pntc *= scaler;
							pntc += offset;

							T.transform(pntc, pntt);
							R.point(pntt, index, *f_b);
							
							++i;
							++f_b;
							++index;
						}
					}
				}
			}
			else 
			{
				pt::vector3s *i, *e;
				pt::vector3d pnt;

				geom->begin(&i);
				i += start_point;
				e = (pt::vector3s*)geom->element(end_point);

				if(!layer)
				{
					while (i < e)
					{
						pntc.x = i->x;
						pntc.y = i->y;
						pntc.z = i->z;

						pntc *= scaler;
						pntc += offset;

						T.transform(pntc, pntt);
						R.point(pntt, index, pnt_layer);	
						++i;
						++index;
					}
					
				}
				else
				{
					if (layerMask)
					{
						while (i<e)
						{
							if (f_b >= f_e || *f_b & layerMask)
							{
								pntc.x = i->x;
								pntc.y = i->y;
								pntc.z = i->z;

								pntc *= scaler;
								pntc += offset;

								T.transform(pntc, pntt);
								R.point(pntt, index, *f_b);	
							}
							++i;
							++f_b;
							++index;
						}
					}
					else
					{
						while (i<e)
						{
							pntc.x = i->x;
							pntc.y = i->y;
							pntc.z = i->z;

							pntc *= scaler;
							pntc += offset;

							T.transform(pntc, pntt);
							R.point(pntt, index, *f_b);	
						
							++i;
							++f_b;
							++index;
						}
					}
				}
			}
		}
		//---------------------------------------------------------------------------
		/* not recommended to use by client code, used internally during indexing */ 
		void _getPointPosition(int index, pt::vector3 &pnt)
		{
			const pcloud::DataChannel *geom = _channels[pcloud::PCloud_Geometry];
			if (geom->typesize() == 4)
			{
				geom->getval(pnt, index);
				pnt *= geom->scaler();
				pnt += geom->offset();
			}
			else
			{
				pt::vector3s pnts;
				geom->getval(pnts, index);	
				pnt.x = pnts.x;
				pnt.y = pnts.y;
				pnt.z = pnts.z;
				pnt *= geom->scaler();
				pnt += geom->offset();
			}
		}
		//---------------------------------------------------------------------------
		/* Used to iterate through 'Full' points with intensity, colour and normal information */ 
		template <class Receiver, class Transformer>
		void _iterateFullTransformedPoints(Receiver &R, Transformer &T, ubyte layerMask = 0)
		{	
			T.prepare();

			int index = 0;

			ubyte nullFilter = 0xff & ~(POINT_SELECT_CODE);
			pcloud::DataChannel *geom 	= _channels[pcloud::PCloud_Geometry];
			pcloud::DataChannel *inten 	= _channels[pcloud::PCloud_Intensity];
			pcloud::DataChannel *norm 	= _channels[pcloud::PCloud_Normal];
			pcloud::DataChannel *rgb 	= _channels[pcloud::PCloud_RGB];
			pcloud::DataChannel *layer 	= _channels[pcloud::PCloud_Filter];

			pt::vector3d pntc, pntt, scaler, offset;
			scaler.set(geom->scaler());
			offset.set(geom->offset());

			short *i_b=0, *i_e=0;
			ubyte *f_b=0, *f_e=0;
			pt::vector3s *n_b=0, *n_e=0;
			pt::vec3<unsigned char> *rgb_b=0, *rgb_e=0;

			if (inten) 	{ inten->begin(&i_b); inten->end(&i_e);	}
			if (rgb)  	{	rgb->begin(&rgb_b);	rgb->end(&rgb_e); }
			if (norm) 	{	norm->begin(&n_b);	norm->end(&n_e); }
			if (layer) 	{ layer->begin(&f_b); layer->end(&f_e); }

			if (geom->typesize() == 4)	/* points as floats */ 
			{
				pt::vector3 *i, *e;

				/* other channels */ 
				geom->begin(&i);
				geom->end(&e);
				
				if (!layer)
				{
					while (i < e)
					{
						pntc.set(*i);
						pntc *= geom->scaler();
						pntc += geom->offset();

						T.transform(pntc, pntt);
						R.point(pntt, index, nullFilter, 
							(i_b < i_e) ? i_b : 0, 
							(rgb_b < rgb_e) ? rgb_b : 0, 
							(n_b < n_e) ? n_b : 0);

						++i_b; ++rgb_b;	++n_b; ++i;	++index;
					}
				}
				else
				{
					if (layerMask)
					{
						while (i < e)
						{
							if (f_b >= f_e || *f_b & layerMask)
							{
								pntc.set(*i);
								pntc *= geom->scaler();
								pntc += geom->offset();
								T.transform(pntc, pntt);
								R.point(pntt, index, *f_b, 
									(i_b < i_e) ? i_b : 0, 
									(rgb_b < rgb_e) ? rgb_b : 0, 
									(n_b < n_e) ? n_b : 0);
							}
							++i_b; ++rgb_b;	++n_b; ++i; ++index; ++f_b;
						}
					}
					else
					{
						while (i < e)
						{
							pntc.set(*i);
							pntc *= geom->scaler();
							pntc += geom->offset();
							T.transform(pntc, pntt);

							R.point(pntt, index, *f_b, 
								(i_b < i_e) ? i_b : 0, 
								(rgb_b < rgb_e) ? rgb_b : 0, 
								(n_b < n_e) ? n_b : 0);

							++i_b; ++rgb_b;	++n_b; ++i;	++f_b, ++index;
						}
					}
				}
			}
			else	//points as shorts
			{
				pt::vector3s *i, *e;
				geom->begin(&i);
				geom->end(&e);

				if (!layer)
				{
					while (i < e)
					{
						pntc.x = i->x;
						pntc.y = i->y;
						pntc.z = i->z;

						pntc *= scaler;
						pntc += offset;

						T.transform(pntc, pntt);
						R.point(pntt, index, nullFilter, 
							(i_b < i_e) ? i_b : 0, 
							(rgb_b < rgb_e) ? rgb_b : 0, 
							(n_b < n_e) ? n_b : 0);	
					
						++i; ++i_b; ++rgb_b; ++n_b; ++index;
					}
				}
				else
				{
					if (layerMask)
					{
						while (i < e)
						{
							if (f_b >= f_e || *f_b & layerMask)
							{
								pntc.x = i->x;
								pntc.y = i->y;
								pntc.z = i->z;

								pntc *= scaler;
								pntc += offset;

								T.transform(pntc, pntt);
								R.point(pntt, index, *f_b, 
									(i_b < i_e) ? i_b : 0, 
									(rgb_b < rgb_e) ? rgb_b : 0, 
									(n_b < n_e) ? n_b : 0);	
							}
							++i; ++i_b; ++rgb_b; ++n_b; ++index; ++f_b;
						}
					}
					else
					{
						while (i < e)
						{
							pntc.x = i->x;
							pntc.y = i->y;
							pntc.z = i->z;

							pntc *= scaler;
							pntc += offset;

							T.transform(pntc, pntt);
							R.point(pntt, index, *f_b, 
								(i_b < i_e) ? i_b : 0, 
								(rgb_b < rgb_e) ? rgb_b : 0, 
								(n_b < n_e) ? n_b : 0);	
						
							++i; ++i_b; ++rgb_b; ++n_b; ++index; ++f_b;
						}
					}
				}
			}
		}
		//---------------------------------------------------------------------------
#define MAX_INTERSECTION_POINTS 8
		/* returns number of close points*/ 
		int intersectRay(const pt::Ray<float> &ray, int *pnt_indices, float tolerance);

		//---------------------------------------------------------------------------
		/* returns index of point*/ 
		int intersectRay(const pt::Rayf &ray, double &rayPos, float tolerance);

		union VoxelID
		{
			struct Components {
			ubyte tmpScene;
			ubyte uidCloud;
			uint uidVoxel;
			};
			Components comp;
			uint64_t uid;
		};
		const VoxelID &uid() const 
		{ 
			return m_id; 
		}

		void setLastStreamManagerIteration(uint64_t iteration)
		{
			m_lastStreamManagerIteration = iteration;
		}

		uint64_t getLastStreamManagerIteration(void) const
		{
			return m_lastStreamManagerIteration;
		}

	protected:

	friend class PodIO;	// PodIO reads POD files and creates Voxels, needs some internal access
	friend PointCloud;	// PointCloud aggregates Voxels, needs access to some internal stuff

		DataChannel*	_channels[MAX_CHANNELS];	// data channels
#ifndef POINTOOLS_POD_API
        // Used to be boost::try_mutex: 
        // From boost doc: "boost::try_mutex is a typedef to boost::mutex, provided for backwards compatibility with previous releases of boost. "
        // So it is basically a std mutex.  Use _mutex.try_lock to get that behavior.
        std::mutex _mutex;				// mutex for access, not required in POD Writer API
#endif

		// used at creation time to stratify points by reordering. Returns num of generated strata, last one contains remaining points
		int		computeStrataIndex(std::vector<int> &resultIndex, int *strata, float strataSpacing);	
		void	setStrataSizes( int numStrata, int strata[NUM_STRATA], float spacing );					// used by PodIO to set strata sizes as read from Pod
		
		const	PointCloud*		_pointcloud;	// Point cloud object that owns this
		float	_priority;						// Loading priority copmuted by visibilityengine
		float	_usage;							// 
		int 	_strataSize[NUM_STRATA];		// Size of strata spaced at strata spacing computed on octree generation
		float	_strataSpacing;					// Spacing in measurement units (m) at which density is evaluated
		uint	_numPointsEdited;				// Point to which filter has reached, used by editing
		uint	_index;							// Index in the point cloud vector

		pt::BoundingBoxD _origextents;			// Original bounding extents, used when transformed to recompute transformed bounds

		unsigned char	m_extents_dirty;		// Extents dirty flag for lazy evaluation
		LOD				m_lod;					// Actual current lod
		LOD				m_stream_lod;			// Stream LOD. States current progress.
		LOD				m_previous_lod;			// Previous LOD before voxel resize.
		FileIndex		m_fileindex;			// Fileindex in points pager

												// Last stream manager iteration that this voxel was processed
		uint64_t m_lastStreamManagerIteration;

		
		VoxelID m_id;							// Unique voxel id

		int64_t	m_filepointer;					// Pointer in file to start of channel data

		bool resizedToStream;					// True if voxel channels have been resized ready for streamed data to be placed in them

	};
}
#endif
