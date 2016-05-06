#include "PointoolsVortexAPIInternal.h"

#define _VERBOSE	
//#define FILE_TRACE 1
#include <pt/trace.h>

#include <ptcloud2/buildindex.h>
#include <ptcloud2/scene.h>
#include <ptcloud2/pod.h>
#include <ptcloud2/octreeIndexing.h>

#include <ptcmdppe/cmdprogress.h>
#include <algorithm>
#include <queue>
#include <iostream>


using namespace pcloud;

#define ONE_PASS_THRESHOLD 0
#define MAX_WRITE_BYTES 120000000

#define MAX_INITIAL_DEPTH 8
#define UPDATE_PROGRESS_EVERY 500000
#define CLOUD_VALID(cloud) ((cloud->numPoints > 1) ? true : false)

//using namespace pt;

//---------------------------------------------------------
// Build Scene Database from Stream
//---------------------------------------------------------
// total file size may be considerably larger
// than available memory
// POD file is generated
//---------------------------------------------------------
Scene::Scene(IndexStream *stream, int &error, float unform_filter_spacing )
{
	PTTRACE_FUNC

	/*setup POD job and start write*/ 
	m_filepath = stream->filepath();
	m_filepath.setExtension(L"pod");

	PodJob job(this, m_filepath);

	SceneBuildData buildData;
	buildData.job = &job;
	buildData.stream = stream;
	buildData.uniformFilterSpacing = unform_filter_spacing;

	/* add in the meta tags */ 
	_metadata = stream->metaData();
	_metadata.audit.import_settings.spatial_filtering = unform_filter_spacing;

	/* check for empty stream */ 
	int g;

	bool hasPnts = false;
	for (g=0; g<stream->numGroups(); g++)
	{
		IndexStream::CloudGroup *group = stream->setGroup(g);
		for (uint c=0;c<group->clouds.size();c++)
		{
			if (group->clouds[c]->numPoints > 0)
			{
				hasPnts = true;
				break;
			}
		}
	}
	if (!hasPnts)
	{
		error = NoPointsInStream;	
		return;
	}

	/* create file */ 
	this->setIdentifier(m_filepath.filename());

	if (PodIO::openForWrite(job))
	{
		ptds::Tracker tracker(job.h);
		job.tracker = &tracker;
		
		PodIO::writeVersion(job);

		error = extractCloudsFromStream( &buildData );
		if ( error == Success && stream->startReadStream() )
		{
			if (!PodIO::writeHeader(job))
			{
				error = FileWriteError;
			}
			else
			{
				stream->generateNormals();

				for (g=0; g<stream->numGroups(); g++)
				{
					IndexStream::CloudGroup *group = stream->setGroup(g);

					if (group->combine)
					{
						buildData.cloudIndex = -1;
						error = readMultiPass(&buildData);
					}
					else
						error = buildCloudGroupFromStream(&buildData);
				}
				stream->closeReadStream();
			}
		}
		else
		{
			error = ReadStreamError;
		}
		tracker.writePlaceHolders();
		PodIO::close(job);
	}
	else
	{
		error = FileWriteError;
	}
}
/*--------------------------------------------------------------------------*/ 
/*  uncombined cloud group													*/ 
/*--------------------------------------------------------------------------*/ 
int Scene::buildCloudGroupFromStream(SceneBuildData *buildInfo)
{
	PTTRACE_FUNC

	ptapp::CmdProgress progress("Loading", 0, buildInfo->stream->numClouds());
	
	int i;

	/*build the structure*/ 
	for (i=0; i<buildInfo->stream->numClouds(); i++)
	{
		char status[256];
		sprintf(status, "Processing cloud %d of %d...", i+1, buildInfo->stream->numClouds());

		progress.status(status);
		progress.set(i);

		const IndexStream::CloudInfo *cloud_info = buildInfo->stream->readCloud(i);

		int error = Success;

		if (cloud_info && CLOUD_VALID(cloud_info))
		{
			int64_t numPoints = cloud_info->numPoints;
			buildInfo->cloudIndex = i;

			if (numPoints < ONE_PASS_THRESHOLD)
			{
				error = readSinglePass(buildInfo);
			}
			else
			{
				error = readMultiPass(buildInfo);
			}
		}
		if (error != Success) return error;
	}
	return Success;
}
/*--------------------------------------------------------------------------*/ 
/* extractCloudsFromStream													*/ 
/* Extracts (empty) clouds and adds them to Scene							*/ 
/*--------------------------------------------------------------------------*/ 
int Scene::extractCloudsFromStream(SceneBuildData *buildInfo)
{	
PTTRACE_FUNC

/* insert clouds without data or structure	*/
/* this is header data						*/
    int i, g;

	std::vector<pcloud::PointCloud *> gclouds;

	for (g=0; g<buildInfo->stream->numGroups(); g++)
	{
		IndexStream::CloudGroup *group = buildInfo->stream->setGroup(g);
		if (group->clouds.size() <= 1) group->combine = false;
		
		/* add the clouds to the scene				*/ 
		if (group->combine)
		{
			/* record which scene cloud this will be*/ 
			for (i=0;i<group->clouds.size(); i++)
			{
				IndexStream::CloudInfo *cloudinfo = group->clouds[i];
				cloudinfo->_cloudPosInScene = _clouds.size();

				if (CLOUD_VALID(cloudinfo))
				{
					ScanPosition *sp = addScanPosition(cloudinfo->matrix);
					
					/* add the scan images in here */ 
					for (int im = 0; im<cloudinfo->images.size(); im++)
					{
                        sp->addImage(cloudinfo->images[im]);
					}
				}
			}
			PointCloud *pc = new PointCloud(group->name, this, group->tolerance);
			pc->generateGuid();

			gclouds.push_back(pc);
			addCloud(gclouds.back());
		}
		else /* not combined */ 
		{
			for (i=0; i<buildInfo->stream->numClouds(); i++)
			{
				/* record which scene cloud this will be*/ 
				buildInfo->stream->cloudInfo(i)->_cloudPosInScene = _clouds.size();

				IndexStream::CloudInfo *cloudinfo = buildInfo->stream->cloudInfo(i);

				if (CLOUD_VALID(cloudinfo))
				{
					PointCloud *pc = new PointCloud( buildInfo->stream->cloudInfo(i)->name, 
						this, group->tolerance );
					pc->generateGuid();

					gclouds.push_back(pc);
					addCloud(gclouds.back());

					/* add an Scan Position if there are any images */ 
					if (cloudinfo->images.size())
					{
						ScanPosition *sp = addScanPosition(cloudinfo->matrix);
						
						/* add the scan images in here */ 
						for (int im = 0; im<cloudinfo->images.size(); im++)
							sp->addImage(cloudinfo->images[im]);
					}
				}
			}
		}
		/* set up matrices							*/ 
		if (!group->combine)
			for (i=0; i<gclouds.size(); i++)
				gclouds[i]->registration().matrix( buildInfo->stream->cloudInfo(i)->matrix );	
				
		/* note that combined clouds have a single  */ 
		/* matrix, this is dealt with later			*/ 
		/* calc combined matrix						*/ 
		else
		{
			mmatrix4d m = mmatrix4d::identity();
			m.translate(pt::vector4d(group->xbounds.center()));
					
			gclouds[0]->registration().matrix(m);
		}
		gclouds.clear();
	}
	return Success;
}
/*--------------------------------------------------------------------------*/ 
/*  Single Pass cloud read													*/ 
/*--------------------------------------------------------------------------*/ 
int Scene::readSinglePass( SceneBuildData *buildInfo )
{
	PTTRACE_FUNC

	const IndexStream::CloudInfo *cloud_info = buildInfo->stream->readCloud( buildInfo->cloudIndex );
	PointCloud * pc = _clouds[cloud_info->_cloudPosInScene];

	int min_depth = -1;
	if (buildInfo->stream->group()->tolerance > 0) 
	{
		pt::BoundingBoxD bnds(cloud_info->xbounds.ux(), cloud_info->xbounds.lx(),
							cloud_info->xbounds.uy(), cloud_info->xbounds.ly(),
							cloud_info->xbounds.uz(), cloud_info->xbounds.lz());
		OctreeIndexing::minDepthFor16bitQuant(bnds, buildInfo->stream->group()->tolerance);
	}
	if (min_depth < 2) min_depth = 2;

 	ubyte *rgb = 0;
	short *intensity = 0;
	pt::vector3s *normals = 0;

	/*Allocations						*/ 
	pt::vector3 *geom = new pt::vector3[cloud_info->numPoints];
	if (cloud_info->hasRGB()) rgb = new ubyte[cloud_info->numPoints*3];
	if (cloud_info->hasIntensity()) intensity = new short[cloud_info->numPoints];
	if (cloud_info->hasNormals())	normals = new pt::vector3s[cloud_info->numPoints];

	/*start cloud read again			*/ 
	buildInfo->stream->readCloud( buildInfo->cloudIndex );

	/*read cloud data from stream		*/ 
	for (int j=0; j<cloud_info->numPoints; j++)
	{
		if (j % 10000 ==0) std::cout << ".";

		buildInfo->stream->readPoint(geom[j], rgb ? &rgb[j*3] : 0, 
			intensity ? &intensity[j] : 0, normals ? &normals[j] : 0);
	}

	/*build point cloud											*/ 
	if (min_depth >= 0)
	{
		BuildIndex::minDepth() = min_depth > 0 ? min_depth : 1;
		BuildIndex::geomStoreType() = Short16;
	}
	else
	{
		BuildIndex::geomStoreType() = Float32;
		BuildIndex::minDepth() = 1;
	}
    pc->buildFromRaw(static_cast<int>(cloud_info->numPoints), (float*)geom, rgb, intensity, (short*)normals);

	/*save this cloud + clear voxel data						*/ 
	pc->initialize();

	PodIO::writeCloudStructure( *buildInfo->job, buildInfo->cloudIndex);
	PodIO::writeCloudData( *buildInfo->job, buildInfo->cloudIndex );
	
	if (geom) delete [] geom;
	if (rgb) delete [] rgb;
	if (intensity) delete [] intensity;
	if (normals) delete [] normals;

	pc->clearData();

	return Success;
}
/*------------------------------------------------------------------------------------------------------*/ 
/* buildPreliminaryOctree																				*/ 
/*------------------------------------------------------------------------------------------------------*/ 
/* this will create structure to the depth value of the larger of max_initial_depth or required depth   */ 
/* since we are not doing multiple iterations over the larger nodes, this should be sufficient to break */ 
/* down large (num points) nodes																		*/ 
/*------------------------------------------------------------------------------------------------------*/ 
int Scene::buildPreliminaryOctree( IncNode *root, int initial_depth, int target_depth, SceneBuildData *buildInfo )
{
	PTTRACE_FUNC

	int C;
	uint j;

	pt::vector3 pnt;
	pt::vector3d pntd, worldPntd;
	const IndexStream::CloudInfo *cloud_info;
	mmatrix4d mat;

	/*progress bar */ 
	ptapp::CmdProgress *indexprogress = new ptapp::CmdProgress("Indexing Data...", buildInfo->startCloud, buildInfo->endCloud*100);

	unsigned int maxIterations	= 5;
	unsigned int numSubdivided	= 0;
	unsigned int numMerged		= 0;


	for (uint readIterations = 0; readIterations == 0 || (numSubdivided > 0 && readIterations < maxIterations); readIterations++)
	{
		bool secondRun = readIterations ? true : false;

		/* count points and compute bounds */ 
		for (C=buildInfo->startCloud; C<buildInfo->endCloud; C++)
		{
			cloud_info = buildInfo->stream->readCloud(C);

			mat = cloud_info->matrix;
			if (buildInfo->transform) mat.inv_translate( pt::vector4d(buildInfo->stream->xbounds().center()) );

			/* build octree */ 
			for (j=0; j<cloud_info->numPoints; j++)
			{
				if (buildInfo->stream->readPoint(pnt, 0, 0, 0) == ReadPntStreamFailure)
				{
					return ReadPntStreamFailure;					// error during a read point, something probably failed in the buffer read 
				}
				// Get point in world coords as a double (double required to avoid loss of precision with points far from the origin)
				pntd.x = pnt.x;
				pntd.y = pnt.y;
				pntd.z = pnt.z;
				mat.vec3_multiply_mat4(pntd, worldPntd); 
				
				root->countPoint( buildInfo->combine ? worldPntd : pntd, worldPntd, initial_depth, secondRun );

				if (j % UPDATE_PROGRESS_EVERY ==0)
				{
					int percent_done = (int)((((float)j) / cloud_info->numPoints) * 100);
					indexprogress->set(C*100+percent_done);
				}
			}

																				// 2M,	5M,		10M,	50M,	100M,	500M,	1Bn,	10Bn	Larger
																				// 50k, 100k,	100k,	100k,	100k,	100k,	200k,	1M,		2M

            const unsigned int numThresholds = 8;
            uint64_t thresholds[numThresholds]	= { 2_M, 5_M, 10_M, 50_M, 100_M, 500_M, 1_B, 10_B };
			uint64_t points[numThresholds + 1]	= { 50_K, 100_K, 100_K, 100_K, 100_K, 100_K, 200_K, 1_M, 2_M };

			int				i;
            unsigned int	minPointsTarget = static_cast<uint>(points[numThresholds]);					// Default to largest target

			for (i=0; i< numThresholds; i++)
			{
				if(root->lodPointCount() < thresholds[i])
				{
					minPointsTarget = static_cast<uint>(points[i]);
					break;
				}
			}
															// check for large nodes requiring another iteration */ 
			numSubdivided	= root->subdivideLarge(minPointsTarget, 4);
															// merge nodes that have been subdivided past SUBDIVISION_COUNT and are deeper than req_depth
			numMerged		= root->merge(minPointsTarget, target_depth);

			PTTRACEOUT << "Num of large nodes = " << numSubdivided;
		}
	}

	delete indexprogress;

	return Success;
}
/*--------------------------------------------------------------------------*/ 
/*  Multi Pass cloud read													*/ 
/*--------------------------------------------------------------------------*/ 
int Scene::readMultiPass( SceneBuildData *buildInfo )
{	
	PTTRACE_FUNC

	/* combine clouds into single */ 
	buildInfo->combine = buildInfo->cloudIndex < 0 ? true : false;

	/* cloud counter */ 
	int C;	

	if (buildInfo->combine)
	{
		buildInfo->startCloud = 0;
		buildInfo->endCloud = buildInfo->stream->numClouds();
		buildInfo->cloudIndex = 0;
	}
	else
	{
		buildInfo->startCloud = buildInfo->cloudIndex;
		buildInfo->endCloud = buildInfo->cloudIndex + 1;
	}
	PTTRACE_LINE 

	if (_clouds.size() < buildInfo->stream->cloudInfo( buildInfo->startCloud )->_cloudPosInScene)
	{
		PTTRACEOUT << "Incorrect scene index. Start cloud has index of " <<
			buildInfo->stream->cloudInfo( buildInfo->startCloud )->_cloudPosInScene << " Scene only has " 
			<< _clouds.size() << " clouds";
	}
	/* The point cloud that indexstream data is being read into*/ 
	PointCloud *pc = _clouds[
		buildInfo->stream->cloudInfo( buildInfo->startCloud )->_cloudPosInScene ];
	
	/*this is a hack for now */ 
	buildInfo->transform = buildInfo->combine;
	
	/* overall bounds */ 
	pt::BoundingBoxD bounds;
	if (buildInfo->transform)
	{
		bounds.setBox(buildInfo->stream->xbounds().ux(), buildInfo->stream->xbounds().lx(),
					buildInfo->stream->xbounds().uy(), buildInfo->stream->xbounds().ly(),
					buildInfo->stream->xbounds().uz(), buildInfo->stream->xbounds().lz());
	}
	else
		bounds = buildInfo->cloudInfo()->bounds;
	
	/* if combined center data, this transformation is place in the transform matrix later	*/ 
	if (buildInfo->transform)
	{
		pt::vector3d cntr(buildInfo->stream->xbounds().center().x, buildInfo->stream->xbounds().center().y, buildInfo->stream->xbounds().center().z); 
		bounds.translateBy(-cntr);
	}

	PTTRACE_LINE 
	
	int req_depth, base_depth;
	bool compress = OctreeIndexing::reqDepthFor16bitCompress(bounds, buildInfo->stream->group()->tolerance, base_depth, req_depth);

	/*create root node																		*/ 
	IncNode *root = new IncNode(bounds);

	PTTRACE_LINE 

	/* check for a thin box for Quad Tree indexing rather than Octree*/ 
	int quadTreeAxis = -1;	//ie don't make qt, make ot
	pt::vector3 boxSize = bounds.diagonal();
	if (boxSize.z < ( boxSize.y + boxSize.x) * 0.025) 
		quadTreeAxis = 2;
	if (boxSize.y < ( boxSize.x + boxSize.z) * 0.025)
	{
		if (quadTreeAxis == -1)  quadTreeAxis = 1;
		else quadTreeAxis = -1;	// thin in more than one dim
	}
	if (boxSize.x < ( boxSize.z + boxSize.x) * 0.025)
	{
		if (quadTreeAxis == -1)  quadTreeAxis = 0;
		else quadTreeAxis = -1;	// thin in more than one dim
	}

	/* Subdivide to base depth																*/ 
	if (base_depth > 0)	root->subdivide( base_depth, quadTreeAxis );
	else				root->subdivide( 1, quadTreeAxis );

	/*find overall cloud spec																*/ 
	uint spec = 0;
	int64_t pointcount = 0;

	PTTRACE_LINE 

	for (C=buildInfo->startCloud; C<buildInfo->endCloud; C++) 
	{
		spec |= buildInfo->stream->cloudInfo(C)->spec;
		pointcount += buildInfo->stream->cloudInfo(C)->numPoints;
	}

	/* build the octree																		*/ 
	int initial_depth = req_depth > MAX_INITIAL_DEPTH ? req_depth : MAX_INITIAL_DEPTH;

	PTTRACE_LINE 

	int error = buildPreliminaryOctree( root, initial_depth, req_depth, buildInfo );

	if (error < 0) return error;

	/* now need to make this node-voxel tree and attatch to point cloud						*/ 
	root->makeLeavesVoxels(pc->voxels(), 
		(spec & PCLOUD_CLOUD_RGB) != 0, 
		(spec & PCLOUD_CLOUD_INTENSITY) != 0, 
		(spec & PCLOUD_CLOUD_NORMAL) != 0, 
		(spec & PCLOUD_CLOUD_CLASSIFICATION) != 0, 
		(spec & PCLOUD_CLOUD_ORDERED) != 0, 
		(compress ? Short16 : Float32), 
		buildInfo->stream->group()->tolerance);

	PTTRACE_LINE 

	pc->setRoot(root);

	/*translate extents*/ 
	if (buildInfo->transform)
	{
		pt::BoundingBoxD xbd(buildInfo->stream->xbounds().ux(), buildInfo->stream->xbounds().lx(),
							buildInfo->stream->xbounds().uy(), buildInfo->stream->xbounds().ly(),
							buildInfo->stream->xbounds().uz(), buildInfo->stream->xbounds().lz());
		(*const_cast<pt::BoundingBoxD*>(&root->extents())) = xbd;
		(*const_cast<pt::BoundingBoxD*>(&pc->projectBounds().bounds())) = xbd;
		pt::vector3d cntr(buildInfo->stream->xbounds().center().x, buildInfo->stream->xbounds().center().y, buildInfo->stream->xbounds().center().z);
		for (int i=0; i<pc->voxels().size(); i++)
		{
			const_cast<pt::BoundingBoxD*>(&pc->voxels()[i]->extents())->translateBy(cntr);
		}
	}
	PTTRACE_LINE 

	PodIO::writeCloudStructure( *buildInfo->job, buildInfo->cloudInfo()->_cloudPosInScene );

	return writePointData( buildInfo, pc );
}
/*--------------------------------------------------------------------------*/ 
/*  Write points to POD file in one or more iteratons                       */ 
/*--------------------------------------------------------------------------*/ 
int Scene::writePointData( SceneBuildData *buildInfo, PointCloud *pc )
{
	PTTRACE_FUNC
	
	int error = Success;
	int iteration = 0;
	int start_voxel = 0;
	pt::BoundingBoxD bb;

	pt::vector3 pnt;
	pt::vector3d pntd, pnt1d;
	pt::vector3s normal;
	short intensity;
	ubyte rgb[3];
	ubyte classification = 0 ;

	uint64_t writebuffsize = 1024 * 1024 * 100;

	/*do write in manageable chunks (ie < MAX_WRITE_BYTES)*/ 
	std::queue<Voxel*> voxels;

	for (int i=0; i<pc->voxels().size(); i++) 
		voxels.push(pc->voxels()[i]);

	/* check how much memory is available */ 
	MEMORYSTATUSEX mem;
	mem.dwLength = sizeof(MEMORYSTATUSEX);
    uint64_t aval;
	int writebuffsizeMB;

	UniformFilter uniFilter;

	/* loop through voxels */ 
	while (voxels.size())
	{
		PTTRACE_LINE

		char mess[256];
	
		GlobalMemoryStatusEx(&mem);		
		aval = static_cast<uint64_t>(mem.ullAvailVirtual * 0.75); /* leave space for read / write block allocations */
		writebuffsize = aval;
        writebuffsizeMB = static_cast<int>(writebuffsize / (1024 * 1024));
		
		sprintf_s(mess, "Gathering data for POD file : iteration %d using up to %dMb for buffer", ++iteration, writebuffsizeMB);
		ptapp::CmdProgress writeprogress(mess, 0, (buildInfo->endCloud - buildInfo->startCloud)+1);

		/* the voxels that will be processed in this iteration */ 
		std::vector<Voxel*> processvoxels;
		std::vector<unsigned long> voxread;

		uint v, gridpos;
        int C;
        uint64_t pointbytes=0;

		bb.clear();

		/*keep track of voxels in map*/ 
		typedef std::map< Voxel*, std::pair<int, pt::vector3*> > TEMPVOXELMAP ;
		TEMPVOXELMAP voxelmap;

		/*get next bunch of voxels to process and add to list*/ 
		/* following loop only does a gather of voxels, does not gather points */ 
		while (pointbytes < writebuffsize && voxels.size())
		{
			PTTRACE_LINE

			Voxel *vox = voxels.front();
			bool memory_failure = false;

			/*compute bytes required and allocate in channels*/ 
			for (int ch=0; ch<NUM_DATA_CHANNELS; ch++)
			{
				DataChannel *dc = const_cast<DataChannel*>(vox->channel(ch));
				if (dc)
				{
					pointbytes += static_cast<uint64_t>(dc->typesize() * dc->multiple() * vox->fullPointCount() * 1.2);

					/* check for failure here */ 
					if (!dc->allocate(static_cast<int>(vox->fullPointCount())))
					{
						//MessageBoxA(NULL, "Out of memory during channel allocation - import may fail", "Memory Error", MB_OK);
						memory_failure = true;
						PTTRACEOUT << "WARNING: * Memory Failure * Allocating Channel of " << (vox->fullPointCount()) << " points";
						PTTRACE_LINE

						if (vox->fullPointCount() > 1e7)
						{
							//voxels.pop();	//error
							static int problem = 0;
							problem++;
						}
																									// This is a critical error. We don't want a file with missing data, so exit
						return OutOfMemory;
					}
				}
			}
			if (memory_failure) break;

			/* reserve some space for the filter if needed */ 
			if (buildInfo->uniformFilterSpacing > 0 )
				pointbytes += vox->fullPointCount() * 24;

			processvoxels.push_back(vox);

			/* compute bounds of this list */ 
			bb.expandBy(vox->extents());

			/* track how many points are being read into each voxel */ 
			voxread.push_back(0);

			voxels.pop();
		};


		for (v=0; v<processvoxels.size(); v++)
		{
			voxelmap.insert( TEMPVOXELMAP::value_type
				(processvoxels[v], std::pair<int, pt::vector3*>(v, 0)) );
		}

		PTTRACE_LINE

		/*progress scope*/ 
		{
			/* read in all the points into voxel channels for this iteration */ 
			for (C=buildInfo->startCloud; C<buildInfo->endCloud; C++)
			{
				const IndexStream::CloudInfo *cloud_info = buildInfo->stream->cloudInfo(C);
				if (!CLOUD_VALID(cloud_info)) 
				{
					PTTRACEOUT << "Cloud Valid Error";
					PTTRACE_LINE
					continue;
				}

				/* does this cloud intersect the region of voxels? */ 
				// SA_BB not sure about this, check. Extents should be in world, but are being intersected with transformed extents?
                pt::BoundingBox truncatedBnds(static_cast<float>(bb.ux()), static_cast<float>(bb.lx()), static_cast<float>(bb.uy()),
                                              static_cast<float>(bb.ly()), static_cast<float>(bb.uz()), static_cast<float>(bb.lz()));
				if (!cloud_info->xbounds.intersects(&truncatedBnds))
				{
					continue;
				}
				try
				{
					cloud_info = buildInfo->stream->readCloud(C);
					writeprogress.inc();

					mmatrix4d mat = cloud_info->matrix;

					if (buildInfo->transform) 
						mat.inv_translate( pt::vector4d( buildInfo->stream->xbounds().center() ) );

					for (uint j=0; j<cloud_info->numPoints; j++)
					{
						/*set some default values in case no value available*/ 
						rgb[0] = 255; rgb[1] = 255; rgb[2] = 255;
						intensity = 0; 
						normal.zero();

						gridpos = buildInfo->stream->readPoint(pnt, rgb, &intensity, &normal, &classification);
						pntd.x = pnt.x;
						pntd.y = pnt.y;
						pntd.z = pnt.z;
						if (buildInfo->transform) mat.vec3_multiply_mat4(pntd, pnt1d); 
						else pnt1d = pntd;

						/* check which voxel to write in */ 
						/* find the voxel using the tree */ 

						Voxel* vox = static_cast<Voxel*>(pc->root()->findContainingLeaf(pnt1d));
						if (vox)
						{
							uniFilter.setVoxel(vox);

							TEMPVOXELMAP::iterator it = voxelmap.find(vox);
							if (it != voxelmap.end()) /* otherwise it'll be in the next iteration */ 
							{
								int voxindex = it->second.first;
								int pos = voxread[voxindex];
						
								/*add this point to this voxel and add other channel data*/ 
								if (buildInfo->uniformFilterSpacing <= 0 || 
									uniFilter.filter( pnt1d, buildInfo->uniformFilterSpacing ))
								{
									const DataChannel *dc = vox->channel( PCloud_Geometry );

									if ( dc->storeType() == Short16 )
									{
										pnt1d -= dc->offset();
										if (!pnt1d.is_zero()) pnt1d /= dc->scaler();
										
										pt::vector3s pnt16;
										pnt16.x = static_cast<short>(pnt1d.x);
										pnt16.y = static_cast<short>(pnt1d.y);
										pnt16.z = static_cast<short>(pnt1d.z);
										if (vox->channel(PCloud_Geometry)->validIndex(pos))
											vox->channel(PCloud_Geometry)->set(pos, &pnt16);								
									}
									else 
									{								
										pnt1d -= dc->offset();
										pnt1d /= dc->scaler();

										pt::vector3 pp = pnt1d;

										if (vox->channel(PCloud_Geometry)->validIndex(pos))
											vox->channel(PCloud_Geometry)->set(pos, &pp);								
									}
						
									if (cloud_info->hasRGB())	
									{
										if (vox->channel(PCloud_RGB)->validIndex(pos))
											vox->channel(PCloud_RGB)->set(pos, rgb);								
									}
									if (cloud_info->hasIntensity())
									{
										if (vox->channel(PCloud_Intensity)->validIndex(pos))
											vox->channel(PCloud_Intensity)->set(pos, &intensity);								
									}
									if (cloud_info->hasNormals())	
									{
										if (vox->channel(PCloud_Normal)->validIndex(pos))
											vox->channel(PCloud_Normal)->set(pos, &normal);								
									}
									if (cloud_info->hasClassification())
									{
										if (vox->channel(PCloud_Classification)->validIndex(pos))
											vox->channel(PCloud_Classification)->set(pos, &classification);								
									}
									if (cloud_info->isOrdered())
									{
										if (vox->channel(PCloud_Grid)->validIndex(pos))
											vox->channel(PCloud_Grid)->set(pos, &gridpos);								
									}
									
									voxread[voxindex] = voxread[voxindex] + 1;
								}
							}
						}
						else 
						{
							///* TODO: Log error */ 
							PTTRACEOUT << "INFO: Voxel for point not found";
							//BoundingBox bb;
							//pc->root()->getBounds(bb);
							//PTTRACEOUT << "INFO: Cloud bounds: " << bb.lx() << ", " << bb.ly() " ," << bb.lz() << " to " << bb.ux() << ", " << bb.uy() " ," << bb.uz();
							//PTTRACEOUT << "Point: " << pnt1d.x << ", " << pnt1d.y << ", " << pnt1d.z;

							//PTTRACE_LINE
						}
					}
				}
				catch (...)
				{
					PTTRACEOUT << "FAILURE: POD File creation failed. This is most likley to be a memory failure";
					GlobalMemoryStatusEx(&mem);
					int64_t aval = mem.ullAvailVirtual;	
                    int writebuffsizeMB = static_cast<int>(aval / (1024 * 1024));

					PTTRACEOUT << aval << "Mb available in this processes virtual memory space";

					error = OutOfMemory;
					/* empty voxel cache */ 
					break;
				}
			}
		}
		
		writeprogress.status("Writing data to POD file...");

		/*write this to disk*/ 
		PTTRACEOUT << "PodIO::writeCloudData";
		int *voxel_sizes = 0;

		try
		{
			voxel_sizes = new int[voxread.size()];
		}
		catch(...)
		{
			error = OutOfMemory;
			voxel_sizes = 0;
			break;
		}

		for (v=0;v<voxread.size(); v++) voxel_sizes[v] = voxread[v];

		PodIO::writeCloudData( *buildInfo->job, buildInfo->cloudInfo()->_cloudPosInScene, 
			start_voxel, processvoxels.size(), voxel_sizes, true );

		delete [] voxel_sizes;

		PTTRACE_LINE
		
		start_voxel += processvoxels.size();

		writeprogress.inc(1);

		PTTRACE_LINE

		processvoxels.clear();
		voxread.clear();
	}
	/* re-write the Cloud sizes since filtering may have changed it */ 
	/* this now had to be done in all instances for density to get written (structure == 5)*/ 
	if (buildInfo->job->version[POD_STRUCTURE_VERSION] == 5 || buildInfo->uniformFilterSpacing > 0)
	{
		const_cast<Node*>( pc->root())->calcLodPointCount();		// fast 
		PodIO::reWriteCloudSizes( *buildInfo->job, buildInfo->cloudInfo()->_cloudPosInScene );
	}
	return error;
}
//
//
//
				