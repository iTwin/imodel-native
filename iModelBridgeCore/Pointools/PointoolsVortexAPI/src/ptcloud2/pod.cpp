#include "PointoolsVortexAPIInternal.h"
#include <pt/trace.h>
#include <pt/ptmath.h>
#include <pt/unicodeconversion.h>
#include <ptcloud2/pod.h>
#include <assert.h>
#include <map>
#include <string>
#include <iostream>

using namespace pcloud;
using namespace ptds;

typedef std::map<const Node*, int> NODEMAP;
typedef std::map<const Voxel*, int> VOXELMAP;

#define STRUCTURE_BASE		1000000
#define DATA_BASE			2000000
#define VOXEL_BASE			3000000
#define CLOUD_MULTIPLIER	100000
#define OFFSET_SIZE			0.05
#define VERSION_NUM_BYTES	5

#define PT_NUM_DC_SAMPLES	16

#define PT_POD_MAX_NUM_SCAN_POSITIONS	4096
#define PT_POD_MAX_NUM_IMAGES			4096

#define VERBOSE_TRACKER
#define RESERVED_BlOCK_SIZE 32768

#define STRUCTURE_BLOCK_TRACKER_ID		(STRUCTURE_BASE + 500000)

#define CHANNEL_PLACE_HOLDER_ID(channel, voxel_index, cloud_index) \
	((CLOUD_MULTIPLIER * (cloud_index) * 500) + ((voxel_index) * 10) + (channel))

#define VOXEL_PLACE_HOLDER_ID(voxel_index, cloud_index) \
	(VOXEL_BASE + CLOUD_MULTIPLIER * (cloud_index) + (voxel_index))

#ifdef POINTOOLS_API_INCLUDE
#undef debugAssertM
#define debugAssertM(a,b) //if(!a) std::cout << "assertion failed!!" << std::endl
#endif

#if (POD_STRUCTURE_VERSION > 4)
#define VOXEL_STRATA_ORDERING_HEADER_V5
#endif

using namespace pt;

namespace pod
{
	unsigned char g_demoCode = 254;
	class ReWriteSizes : public Node::Visitor
	{
	public:
		ReWriteSizes(ptds::Tracker *_tracker, int cloudindex, int structure_version)
			: tracker(_tracker), 
			_cloudindex(cloudindex), 
			_nodeindex(0)
		{};
		

		bool visitNode(const Node* node)
		{
			uint size = (uint)node->lodPointCount();	// problem is 4bi limit, however value is re-computed for non-leaf nodes
			const pcloud::Voxel *vox = 0;
			if (node->isLeaf())
			{
				vox = static_cast<const Voxel*>(node);
				size = static_cast<uint>(vox->fullPointCount());
			}
			if (tracker->moveToPointer(CLOUD_MULTIPLIER * _cloudindex + _nodeindex))
			{
				tracker->writeThrough(size);


#ifdef VOXEL_STRATA_ORDERING_HEADER_V5
	
				if (vox)	// not for non-leaf nodes
				{
					int numStrata = NUM_STRATA;
					float strataSpacing  = 0;
					int strata[NUM_STRATA];
					memset(strata, 0, sizeof(strata));
					
					for (int s=0; s<NUM_STRATA; s++)	
						strata[s] = vox->strataSize(s);

					tracker->writeThrough( numStrata );
					tracker->writeThrough( vox->strataSpacing() );
					tracker->writeThrough( strata, sizeof(strata) );
				}
#endif
			}
			_nodeindex++;

			return true;
		}
	private:
		uint _cloudindex;
		uint _nodeindex;
		ptds::Tracker *tracker;
	};
	//-------------------------------------------------------------------------
	// write tree visitor
	//-------------------------------------------------------------------------
	class WriteTree : public Node::Visitor
	{
	public:
		WriteTree(WriteBlock *db, int cloudindex, int structure_version) 
			: block(db), _cloudindex(cloudindex), _nodeindex(0), _structure(structure_version)
		{}
		
		bool visitNode(const Node*node)
		{			
			/*write out node data*/ 
			ubyte depth = node->depth();
			  
			int children[8] = {-1,-1,-1,-1,-1,-1,-1,-1};
			int parent = -1;
			int i;
			for (i=0; i<8; i++)
			{
				NODEMAP::const_iterator n = nodes.find(node->child(i));
				if (n!= nodes.end())
					children[i] = n->second;
			}
			if (node->parent())
			{
				NODEMAP::const_iterator n = nodes.find(node->parent());
				if (n!= nodes.end()) parent = n->second;
			}

			ubyte nodetype = node->isLeaf() ? 1 : 0;
			uint size = (uint)node->lodPointCount();

			if (node->isLeaf())
			{
				const Voxel *v = static_cast<const Voxel*>(node);
                size = static_cast<uint>(v->fullPointCount());
			}
			block->write(nodetype);
			block->write(depth);
			
			// bounding box save - must be done like this due to historic reasons
		//	const ubyte *ptr = (const ubyte*)(const void*)&node->extents(); // Old version
			// save bounding box as floats to allow backward compatibilty in all products that are not expecting bounding boxes as doubles
			pt::BoundingBox bbf(static_cast<float>(node->extents().ux()), 
                                static_cast<float>(node->extents().lx()), 
                                static_cast<float>(node->extents().uy()), 
                                static_cast<float>(node->extents().ly()), 
                                static_cast<float>(node->extents().uz()), 
                                static_cast<float>(node->extents().lz()));
			const ubyte *ptr = (const ubyte*)(const void*)&bbf;
			
			// push forward by size of vftable pointer
			ptr += sizeof(pt::BoundingBox)-48;	// TEST ON 64BIT BUILD

			//8 bytes spare where we squeeze in qt value without killing fwd compat
			ubyte qt = node->quadTreeAxis() + 1;	// quadtree spec
			ubyte hasQT = 133;

			// just a code
			block->write(hasQT);
			block->write(hasQT);
			block->write(hasQT);
			block->write(qt);

			//write just data
			block->write(ptr, 48);
			//block->write( node->extents() );

			block->write(parent);
			block->write(children, INT_SIZE*8);

			block->write(&node->lx(), sizeof(float)*3);
			block->write(&node->ux(), sizeof(float)*3);
			
			/* store this position for position later rewrite when exact size is known */ 
			block->tracker()->saveFilePointer(CLOUD_MULTIPLIER * _cloudindex + _nodeindex);
			block->write(size);

			// HEADER STRUCTURE VERSION 5 : Density / Strata info
#ifdef VOXEL_STRATA_ORDERING_HEADER_V5			

			{
				
				if (nodetype)
				{
					int numStrata = NUM_STRATA;
					float strataSpacing  = 0;
					int strata[NUM_STRATA];
					memset(strata, 0, sizeof(strata));
				
					block->write( numStrata );
					block->write( strataSpacing );
					block->write( strata, sizeof(strata) );
				}
			}
#endif
			// END V5
			
			if (node->isLeaf())
			{
				const Voxel *v = static_cast<const Voxel*>(node);
				VOXELMAP::iterator vf = voxels.find(v);
				assert(vf != voxels.end());
				uint index = vf->second;

				int version = 160906;//221004; overall file version ensures earlier versions don't try to read this.
				block->write(version);
				
				/*channel specs*/ 
				uint channels = 0;

				int c;
				for (c=0;c<NUM_DATA_CHANNELS; c++)
					if (v->channel(c)) channels++;
				
				block->write(channels);

				for (c=1;c<NUM_DATA_CHANNELS; c++)
				{
					const DataChannel *ch = v->channel(c);
					pt::vector3d offset(0,0,0), scaler(1.0f,1.0f,1.0f);

					if (ch)
					{			
						memcpy(&offset, ch->offset(), sizeof(double) * ch->multiple());
						memcpy(&scaler, ch->scaler(), sizeof(double) * ch->multiple());

						uint nativeType = (uint)ch->nativeType();
						uint storeAs = (uint)ch->storeType();
						uint multiple = ch->multiple();

						block->write(c);
						block->write(nativeType);
						block->write(storeAs);
						block->write(multiple);
						block->write(offset);
						block->write(scaler);

						/* data placeholder - sample points for proxy loading */ 
						uint num_samples = PT_NUM_DC_SAMPLES;
						block->write(num_samples);
						block->insertDataPlaceholder(
							CHANNEL_PLACE_HOLDER_ID(c, index, _cloudindex), ch->multiple() * ch->typesize() * PT_NUM_DC_SAMPLES);
					} 
				}
				/*placeholder - filepointer for beginning of data*/ 
				block->insertPlaceholder(VOXEL_PLACE_HOLDER_ID(index, _cloudindex));	
			}
#ifdef VOXEL_STRATA_ORDERING_HEADER_V5
			/* some reserve - this was added in 080510 as version[1] = 5 */
			block->reserve(1024);
#endif
			//END V5
			
			_nodeindex++;

			return true;
		}
		uint	_cloudindex;
		uint	_nodeindex;
		uint	_structure;

		WriteBlock *block;
		NODEMAP		nodes;
		VOXELMAP	voxels;
	};
	//-------------------------------------------------------------------------
	// collect Nodes Visitor
	//-------------------------------------------------------------------------
	class CollectNodes : public  Node::Visitor
	{
	public:
		CollectNodes(NODEMAP *n)
		{

			nodes = n;
			id = 0;
		}
		bool visitNode(const Node *node)
		{
			nodes->insert(NODEMAP::value_type(node, id++));
			return true;
		}

		NODEMAP *nodes;
		int id;
	};

	//-------------------------------------------------------------------------
	// Visitor for writing voxel extents as doubles. 
	//-------------------------------------------------------------------------
	class WriteDoubleBoundsToBlock : public Node::Visitor, public PodBlock::WriteDataCallback
	{
	public:
		WriteDoubleBoundsToBlock(PointCloud* pc) : m_pointCloud(pc), m_writeBlock(NULL) { ; }		

		bool visitNode(const Node *node)
		{
			if (node->isLeaf())
			{
				if (const Voxel* vox = reinterpret_cast<const Voxel*>(node))
				{
					// Write this Voxel's index
					m_writeBlock->write(vox->indexInCloud());
					// Write the Voxel extents as doubles
					const pt::BoundingBoxD ext = vox->extents();
					m_writeBlock->write(ext.ux());
					m_writeBlock->write(ext.lx());
					m_writeBlock->write(ext.uy());
					m_writeBlock->write(ext.ly());
					m_writeBlock->write(ext.uz());
					m_writeBlock->write(ext.lz());
				}
			}
			return true;
		}

		// Callback from PodBlock::WriteDataCallback
		bool writePodBlockData(ptds::WriteBlock& wb)
		{
			m_writeBlock = &wb;

			// Write the number of voxels in this point cloud
			unsigned int numVoxels = m_pointCloud->root()->countNodes(true);
			wb.write(numVoxels);

			// Traverse the point cloud writing double bounds information for each voxel
			m_pointCloud->root()->traverseTopDown(this);

			return true;
		}

	private:
		PointCloud* m_pointCloud;
		WriteBlock* m_writeBlock;
	};

	//-------------------------------------------------------------------------
	// Visitor for calculating the size
	//-------------------------------------------------------------------------
	class WriteDoubleBoundsBlockSizeToBlock : public Node::Visitor, public PodBlock::WriteDataSizeCallback
	{
	public:
		WriteDoubleBoundsBlockSizeToBlock(PointCloud* pc) : m_pointCloud(pc), m_size(0) { ; }		

		bool visitNode(const Node *node)
		{
			if (node->isLeaf())
			{
				m_size += sizeof(unsigned int); // voxel index
				m_size += sizeof(double)*6;		// voxel extents
			}
			return true;
		}

		// Callback from PodBlock::WriteDataSizeCallback
		bool writePodBlockDataSize(ptds::WriteBlock& wb)
		{
			m_size = sizeof(unsigned int); // num voxels

			// Traverse the point cloud accumulating size info
			m_pointCloud->root()->traverseTopDown(this);

			wb.write(m_size);

			return true;
		}

	private:
		PointCloud* m_pointCloud;
		unsigned int m_size;
	};

	/** Handler for reading PodBlocks in the structure section of a POD file. The handler is registered with the
	 current instance of PodBlockManager when the handler's constructor is called. Once registered any PodBlocks
	 with type PODBLOCK_DOUBLEBOUNDINGBOX ("DBBX") will be passed to this handler to be read by the PodBlockManager.
	 */
	class DoubleBoundingPodBlockBoxHandler : public PodBlockReadHandler
	{
	public:
		DoubleBoundingPodBlockBoxHandler() : PodBlockReadHandler(std::string(PODBLOCK_DOUBLEBOUNDINGBOX)) { ; }

		// From PodBlockReadHandler, must be implemented for a PodBlockReadHandler
		bool read(ptds::ReadBlock& rb)
		{
			unsigned int version, size, numBoxes, voxelID;
			double ux, lx, uy, ly, uz, lz;

			// Make sure the version is 1 (mandatory element of a PodBlock)
			rb.read(version);
			if (version != 1)
				return false;

			// The size of the PodBlock (mandatory element of a PodBlock)
			rb.read(size);
			
			// Read the number of bounding boxes to expect
			rb.read(numBoxes);

			// Read the boxes, these are voxel IDs followed by 6 doubles representing a double bounding box, ux, lx, uy, ly, uz, lz
			for (uint i = 0; i < numBoxes; i++)
			{
				rb.read(voxelID);
				rb.read(ux);
				rb.read(lx);
				rb.read(uy);
				rb.read(ly);
				rb.read(uz);
				rb.read(lz);

				pt::BoundingBoxD dbbx(ux, lx, uy, ly, uz, lz);
				m_boxes.insert(std::pair<unsigned int, pt::BoundingBoxD>(voxelID, dbbx));
			}

			return (m_boxes.size() != 0);
		}

		void apply(pcloud::PointCloud* cloud)
		{
			DBBXMap::iterator it;
			std::vector<Voxel*>::iterator vit;
			std::vector<Voxel*> voxels = cloud->voxels();
		
			for (it = m_boxes.begin(); it != m_boxes.end(); it++)
			{
				unsigned int voxelID = it->first;

				for (vit = voxels.begin(); vit != voxels.end(); vit++)
				{
					if (Voxel* v = (*vit))
					{
						if (v->indexInCloud() == voxelID)
						{
							// Update the extents of this Voxel with the correct double versions
							(*vit)->setExtents(it->second);
							(*vit)->setOriginalExtents(it->second);
						}
					}
				}
			}
		}

	private:
		typedef std::map<unsigned int, pt::BoundingBoxD> DBBXMap;
		DBBXMap m_boxes;
	};

#ifdef VERBOSE_TRACKER
	Tracker *readTracker = 0;
#endif

	//-------------------------------------------------------------------------
	// read String - deals with unicode conversion
	//-------------------------------------------------------------------------
	// numbytes should have been numchars, but for backwards compat, we are now stuck
	// TODO: write out full strings at end of header, messy, but a neccessary evil
	// then roll into new format
	std::wstring readString(ReadBlock &rb, int numbytes)
	{
		char buffer[512];
		memset(buffer, 0, sizeof(buffer));
		debugAssertM(numbytes < sizeof(buffer), "Buffer overflow in pod::readString");
		rb.read(buffer, numbytes);

		std::wstring ws;
		/* check if this is wide char */ 
		uint16 widecheck; memcpy(&widecheck, buffer, 2);
		ws = (widecheck == 65335) ? 
			(wchar_t*)&buffer[sizeof(widecheck)] : Ascii2Unicode::convert(std::string((char*)buffer));
		return ws;
	}
	void writeString(WriteBlock &wb, const wchar_t*str, int numbytes)
	{
		wchar_t buffer[512];
		memset(buffer, 0, sizeof(buffer));
		debugAssertM(numbytes < sizeof(buffer), "Buffer overflow in pod::writeString");
		
		wcsncpy( buffer, str, numbytes / 2);

		uint16 i = 65335; /* wide byte version */ 
		wb.write(i);
		wb.write(buffer, numbytes - sizeof(uint16));
	}
	void writeVarString( WriteBlock &wb, const pt::String &str )
	{
		int len = str.length();
		wb.write(len);
		
		/* write string and include null char */
		wb.write(str.c_wstr(), (len+1) * sizeof(wchar_t));
	}
	bool readVarString( ReadBlock &rb, pt::String &str )
	{
		wchar_t buffer[1024];
		memset(buffer, 0, sizeof(buffer));

		int len = 0;
		rb.read(len);
		if (len > 0 && len < 1024) /* sanity check */ 
		{
			rb.read(buffer, len+1);
			str = buffer;
			return true;
		}
		else return false;
	}
}
using namespace pod;

//-------------------------------------------------------------------------
// File Handling
//-------------------------------------------------------------------------
#define TRACKER(A) reinterpret_cast<Tracker *>(A)
bool PodIO::openForRead(PodJob &job)
{
	job.h = ptds::dataSourceManager.openForRead(&(job.filepath));

	bool valid = false;
	
	if(job.h)
	{
		valid = job.h->validHandle();
	}

	return valid;
}

bool PodIO::openForWrite(PodJob &job)
{
	job.h = ptds::dataSourceManager.openForWrite(&(job.filepath));

	if(job.h)
	{
		return job.h->validHandle();
	}

	return false;
}

bool PodIO::close(PodJob &job)
{
	try
	{
		ptds::dataSourceManager.close(job.h);
	}
	catch (...) { return false; }
	return true;
}
//-------------------------------------------------------------------------
// write Version
//-------------------------------------------------------------------------
bool PodIO::writeVersion(PodJob &job)
{
	/* versions, see pod.h for currently supported version */
	ubyte version [] = { POD_HEADER_VERSION, POD_STRUCTURE_VERSION, POD_DATA_VERSION, POD_RESERVED_VERSION }; // header, structure, data, res
	ubyte endian_ = 0; /*little*/ 
	
	/* only write a version 4 header if there are images, otherwise don't */ 
	uint num_scanpos = job.scene->numScanPositions();
	for (uint i=0; i<num_scanpos; i++)
	{
		const ScanPosition *sp = job.scene->scanPos(i);
		if (sp->numImages()) 
		{
			version[0] = 4;
			break;
		}
	}

	TRACKER(job.tracker)->writeThrough(version, 4);
	TRACKER(job.tracker)->writeThrough(endian_);

	memcpy( job.version, version, 4 );
	return true;
}
//-------------------------------------------------------------------------
// write Header
//-------------------------------------------------------------------------
bool PodIO::writeHeader(PodJob &job)
{
	{
	/*version as 4 bytes*/ 
	WriteBlock db(job.h, 65536, TRACKER(job.tracker), 0, "Header");

	/* pre unicode compatibility */ 
	writeString(db, job.scene->identifier(), 64);
	
	uint num_clouds = job.scene->size();
	db.write(num_clouds, "Number of Clouds");

	/* buffer for padding		*/ 
	char buff[64];
	memset(buff, 0, sizeof(buff));

	uint i = 0;

	/*for each cloud			*/ 
	for (i=0; i<num_clouds; i++)
	{
		const PointCloud *pc = job.scene->cloud(i);

		db.write(pc->guid(), "Guid");

		writeString(db, pc->identifier(), 128);

		db.insertPlaceholder(STRUCTURE_BASE + i);
		db.insertPlaceholder(DATA_BASE + i);
	}
	/* write scan positions		*/ 
	uint num_scanpos = job.scene->numScanPositions();
	db.write(num_scanpos, "Number of Scan Positions");

	for (i=0; i<num_scanpos; i++)
	{
		const ScanPosition *sp = job.scene->scanPos(i);

		writeString(db, sp->identifier(), 128);
		db.write(sp->registration().matrix().data(), sizeof(double) * 16);

		/* write scan image references and callibration */ 
		/* new in header version 4						*/ 
		db.write((uint)sp->numImages());
		for (int img=0;img<sp->numImages();img++)
		{
			/* stupid mistake, this is bytes not char, here it actually matters, 
			130 char not enough for filepath in some cases...what to do - backward compatibility
			would be broken by update, could put strings in again at end */ 
			writeString(db, sp->image(img).filepath().c_wstr(), 260);
			db.write(sp->image(img).calibration());
		}
	}

	/* Updated July 2009 for Metadata tags and Keywords */ 
	/* scene name again for full 64 wchars */ 
	/* stupid mistake made in older code, writeString take numBytes not numChars
	this has caused most strings to be written half size, metatags + keywords use variable length strings
	*/
	writeString(db, job.scene->identifier(), 128);
	}
	{
	WriteBlock mb(job.h, 65536, TRACKER(job.tracker), 0, "Meta");

	int zero = 0;
	int subv = 1;

	/* write sub version */ 
	mb.write( zero );	// prevent previous release from reading
	mb.write( zero );
	mb.write( subv );
	
	/* does have meta tags ?? */ 
	MetaData &metadata = job.scene->metaData();
	metadata.writeToBlock(mb);
	}
	/* reserved space for meta edits */ 
	{
		WriteBlock mb(job.h, 65536, TRACKER(job.tracker), 0, "Reserved");
		char *emptyBuffer = new char[32768];
		memset( emptyBuffer, 0, 32768 );
		for (int b=0;b<8;b++)
		{
			mb.write( emptyBuffer, 32768 );
		}
		delete [] emptyBuffer;
	}
	return true;
}
//-------------------------------------------------------------------------
// Handles Version
//-------------------------------------------------------------------------
bool PodIO::handlesVersion(ubyte *version)
{
#ifndef PTAPI_DEMO_DATA
	
	return (
			version[0] < 5 
		&&	(version[1] >= 2 && version[1] <= 5) 
		&&	(version[2] == g_demoCode || version[2] == 254) 
		&&	version[3] == 255) ? true : false;
#else
	return (
		version[0] < 4 
		&&	(version[1] >= 2 && version[1] <= 4) 
		&&	version[2] == 254 
		&&	version[3] == 255) ? true : false;
#endif
}
//-------------------------------------------------------------------------
// write Structure
//-------------------------------------------------------------------------
bool PodIO::writeStructure(PodJob &j)
{
	for (uint i=0; i<j.scene->size(); i++)
	{
		writeCloudStructure(j, i);
	}
	return true;
}
//-------------------------------------------------------------------------
// write Data
//-------------------------------------------------------------------------
bool PodIO::writeData(PodJob &job)
{
	/*write voxel datachannels*/ 
	for (uint i=0; i<job.scene->size(); i++)
		writeCloudData(job, i);

	return true;
}
//-------------------------------------------------------------------------
// write Data
//-------------------------------------------------------------------------
bool PodIO::writeCloudData(PodJob &job, int cloud_idx, int start_voxel, int num_voxels, int *initialization_sizes, bool delete_after_write)
{
	PTTRACE_FUNC

	Tracker *tracker = TRACKER(job.tracker);
	tracker->moveToPosition();

	const PointCloud *pc = job.scene->cloud(cloud_idx);
	if (num_voxels < 0)
	{
		num_voxels = static_cast<int>(pc->voxels().size());
		start_voxel = 0;
	}

	int end_voxel = num_voxels + start_voxel;

	for (int j=start_voxel; j<end_voxel; j++)
	{
		tracker->placeReference(VOXEL_PLACE_HOLDER_ID(j, cloud_idx));

		Voxel* v= pc->voxels()[j];
		
		/* data is initialised, channels are randomised. */ 
		if (initialization_sizes)
		{
			if (!v->initializeChannels( initialization_sizes[j-start_voxel] )) 
				return false; /* most likely memory error */ 
		}

		for (int c=1; c<NUM_DATA_CHANNELS; c++)
		{
			const DataChannel *dc = v->channel(c);
			if (dc && dc->size())
			{
				tracker->writeThrough(dc->begin(), dc->bytesize());
				/* write the samples via the data placeholders */ 

				int num_samples = PT_NUM_DC_SAMPLES < dc->size() ? PT_NUM_DC_SAMPLES : dc->size();

				tracker->writePlacedData(CHANNEL_PLACE_HOLDER_ID(c, j, cloud_idx), dc->begin(), dc->typesize() * dc->multiple() * num_samples);

				/* this deletes the data after its written to conserve memory */ 
				if (delete_after_write) const_cast<DataChannel*>(dc)->dump();
			}
		}
	}
	return true;
}
//-------------------------------------------------------------------------
// write Cloud Structure
//-------------------------------------------------------------------------
bool PodIO::writeCloudStructure(PodJob &job, int cloud_idx)
{
	Tracker *tracker = TRACKER(job.tracker);

	PointCloud *pc = job.scene->cloud(cloud_idx);

	/*position placeholder*/ 
	{
		WriteBlock pcinfo(job.h, 1024, tracker, DATA_BASE + cloud_idx, "CloudInfo");
		pcinfo.write(pc->ibound());
		pcinfo.write(pc->jbound());
		pcinfo.write(pc->registration().matrix());
		pcinfo.write(pc->compressionTolerance());
		pcinfo.insertPlaceholder(STRUCTURE_BLOCK_TRACKER_ID);
		pcinfo.reserve(256-sizeof(float)-INT64_SIZE);
	}	
	WriteBlock pcstruct(job.h, 16000, tracker, STRUCTURE_BASE + cloud_idx, "Structure");
	
	/*write out octree structure*/ 
	/*key nodes to ids*/ 
	WriteTree wt(&pcstruct, cloud_idx, job.version[POD_STRUCTURE_VERSION]);
	CollectNodes cn(&wt.nodes);
	pc->root()->traverseTopDown(&cn);
	
	/*key and write partitions*/ 
	int v_count = static_cast<int>(pc->voxels().size());
	int n_count = pc->root()->countNodes();
	int p_count = 0; /* no partition system - hangover from previous version*/ 

	/*write nodes*/ 
	pcstruct.write(p_count);
	pcstruct.write(n_count);

	int k;
	for (k=0;k<v_count; k++)
	{
		Voxel*v = pc->voxels()[k];
		wt.voxels.insert(VOXELMAP::value_type(v, k));
	}
	/*base offset*/ 
	pc->root()->traverseTopDown(&wt);

	// Write the double bounding box details to a PodBlock ready for writing with any other PodBlocks
	WriteDoubleBoundsToBlock wdb(pc);
	WriteDoubleBoundsBlockSizeToBlock wdbs(pc);
	PodBlock* dbbxBlock = new PodBlock(std::string(PODBLOCK_DOUBLEBOUNDINGBOX), 1, &wdb, &wdbs);
	job.m_podBlockManager.add(dbbxBlock);

	// Write pod blocks
	job.m_podBlockManager.write(pcstruct);
	job.m_podBlockManager.clear();

	return true;
}

//-------------------------------------------------------------------------
// write Cloud Structure
//-------------------------------------------------------------------------
bool PodIO::reWriteCloudSizes(PodJob &job, int cloud_idx)
{
	ptds::Tracker *tracker = TRACKER(job.tracker);
	int64_t savepos = tracker->position();

	ReWriteSizes rw(tracker, cloud_idx, job.version[POD_STRUCTURE_VERSION]);
	
	PointCloud *pc = job.scene->cloud(cloud_idx);
	if (pc)
	{
		pc->root()->traverseTopDown(&rw);
	}
	else return false;
	
	/*restore position */ 
	tracker->moveTo(savepos);

	return true;
}
//-------------------------------------------------------------------------
// write File
//-------------------------------------------------------------------------
bool PodIO::writeFile(PodJob &job)
{
	if (openForWrite(job))
	{
		Tracker t(job.h);
		job.tracker = &t;
		job.state = ptds::DataSourceStateWrite;

		writeVersion(job);
		writeHeader(job);
		writeStructure(job);
		writeData(job);
		t.writePlaceHolders();

		ptds::dataSourceManager.close(job.h);

		return true;
	}
	else return false;
}
//-------------------------------------------------------------------------
// read Version
//-------------------------------------------------------------------------
bool PodIO::readVersion(pcloud::PodJob &job)
{
	if(job.h)
	{
		job.h->readBytes(job.version, 4);
		job.h->readBytes(job.endian);

		return true;
	}

	return false;
}
//-------------------------------------------------------------------------
// write (ie. inject) updated meta data into pod file
//-------------------------------------------------------------------------
bool PodIO::writeMetaUpdate(pcloud::PodJob &job, const MetaData &meta)
{
	if (job.h)
	{
		if (!job.h->isReadWrite()) 
			return false;	// must be openend read/write

		readVersion(job);

		//check version for meta support
		if (job.version[0] >= 4 && job.version[1] >= 4)
		{
			//skip over structure to meta data section
			readHeader(job, true);

			// move back 3 integers
			ptds::Tracker *tracker = TRACKER(job.tracker);
			tracker->moveTo(job.h->getDataPointer());
			job.state = ptds::DataSourceStateWrite;

			// we're in position to write the metadata
			WriteBlock mb(job.h, 65536, TRACKER(job.tracker), 0, "Meta");

			int zero = 0;
			int subv = 1;

			/* write sub version */ 
			mb.write( zero );	// prevent previous release from reading
			mb.write( zero );
			mb.write( subv );
			
			/* does have meta tags ?? */ 
			meta.writeToBlock(mb);

			return true;
		}
	}
	return false;
}
//-------------------------------------------------------------------------
// read Header
//-------------------------------------------------------------------------
bool PodIO::readHeader(pcloud::PodJob &job, bool skip)
{
	int pos = 0;

	if(job.h == NULL)
	{
		return false;
	}

	if (!job.h->movePointerTo(VERSION_NUM_BYTES))
		return false;

	if (!handlesVersion(job.version))
		return false;
	
	pos += VERSION_NUM_BYTES;	// version, 4 bytes
	pos += sizeof(int);			// ReadBlock block size

	ReadBlock rb(job.h, 0);

	std::wstring ws = readString(rb, 64);
	if (!skip) job.scene->setIdentifier(ws.c_str());
	pos += 64;

	uint num_clouds, num_scanpos;
	rb.read(num_clouds);
	pos += sizeof(uint);

	int64_t guid=0;
	int64_t structure_pointer=0;
	int64_t data_pointer=0;

	std::vector<int64_t> data_pointers;
	std::vector<PointCloud*> pointclouds;

	uint i;
	for (i=0; i<num_clouds; i++)
	{
		std::wstring ws;

		pos += rb.read(guid);

		/* older file guids can be 0, so store filepointer for update*/
		if (!skip && !guid)
		{
			job.guidPtrs.push_back( pos-sizeof(guid) );
		}
		else job.guidPtrs.push_back(0);

		if (job.version[0] >= 2)
			ws = readString(rb, 128);
		
		if (!ws.length())
			ws = L"Cloud";

		pos += rb.read(structure_pointer);
		pos += rb.read(data_pointer);
		
		PointCloud *pc = 0;
		if (!skip) 
		{
			pc = job.scene->newCloud(guid);
			pc->setIdentifier(ws.c_str());

			pc->filepointer(structure_pointer);
			job.scene->addCloud(pc);

			pointclouds.push_back(pc);
		}
		data_pointers.push_back(data_pointer < structure_pointer ? data_pointer : 0);
	}
	for (i=0; i<num_clouds; i++)
	{
		if (data_pointers[i] && job.h->movePointerTo(data_pointers[i]))	
		{
			ReadBlock hrb(job.h, 0);
			uint ibound, jbound;
			mmatrix4d mat;

			hrb.read(ibound);
			hrb.read(jbound);
			hrb.read(mat);
			float comp;

			if (job.version[1]>=4)
			{
				if (!skip)
				{
					hrb.read(pointclouds[i]->_compressionTolerance);
				}
				else
				{
					hrb.read(comp);
				}

				ptds::DataPointer podBlockPos = 0;
				hrb.read(podBlockPos);
				if (podBlockPos > 0)
					job.m_podBlockManager.setReadBlockPos(podBlockPos);
			}
 
			if (!skip) 
			{
				pointclouds[i]->registration().matrix(mat);
			}
			if (hrb.numErrors())
				return false;
		}
	}

	/* read scan positions		 - version 3 of header*/ 
	if (job.version[0] >= 3)
	{
		// This must be read as it is always written even if it is zero
		rb.read(num_scanpos);
		mmatrix4d mat;

		if(num_scanpos > PT_POD_MAX_NUM_SCAN_POSITIONS)
		{
			return false;
		}

		for (i=0; i<num_scanpos; i++)
		{
			std::wstring name = readString(rb, 128);
			rb.read(mat.data(), sizeof(double)*16);

			ScanPosition *sp = 0;
			if (!skip) sp = job.scene->addScanPosition(mat);
			
			// read Images - version 4 or later of header (1.7b5+) 
			if (job.version[0] >= 4)
			{
				uint numImages;
				rb.read(numImages);

				if(numImages > PT_POD_MAX_NUM_IMAGES)
				{
					return false;
				}

				for (uint img=0;img<numImages;img++)
				{
					String filepath(readString(rb, 260).c_str());

					Calibration callib;
					rb.read(callib);
					
					if (!skip)
					{
						ScanImage si(filepath, callib);
						si.setScannerPos(sp);
						sp->addImage(si);
					}
				}
			}
		}
	}
	/* Meta Data - July 2009*/ 
	if (job.version[0] >= 4)
	{
		if (!num_scanpos)
		{
			/* correct full string read */ 
			std::wstring ws = readString(rb, 128);
			
			if (!skip && ws.length()) // Should never be zero length, if it is something is wrong with the read/write alignment of the header
				job.scene->setIdentifier(ws.c_str());
		}

		/* move the pointer back into place */ 
		job.h->movePointerTo(VERSION_NUM_BYTES + //version block
										rb.size() + // previous block
										+ sizeof(int)); // size as uint of previous block

		/* check for meta block */ 
		ReadBlock mb(job.h, 0);

		if (mb.size() > 32)
		{
			int subversion = 0;

			mb.read( subversion ); //nulls to avoid previous vesion reading
			mb.read( subversion ); //nulls to avoid previous vesion reading
			mb.read( subversion ); //actual version

			if ( subversion == 1)
			{
				// Ignore errors from this ReadBlock, 
				// the meta data being read here may not actually be available for this particular scan
				if (!skip) 
				{
					job.scene->metaData().readFromBlock( mb );
				}
			}		
		}
	}

	if (rb.numErrors())
		return false;

	return true;
}
//-------------------------------------------------------------------------
// read Structure
//-------------------------------------------------------------------------
bool PodIO::readCloudStructure(pcloud::PodJob &job, PointCloud *cloud, bool skip)
{
	if (!handlesVersion(job.version))
	{
		return false;
	}

	if(job.h == NULL)
	{
		return false;
	}

	if (job.h->validHandle())
	{
		job.state = ptds::DataSourceStateRead;

		/*position placeholder*/ 
		uint p_count, n_count;

		/*position file pointer*/ 
		int64_t fp = cloud->filepointer();

		if (!fp || !job.h->movePointerTo(fp))	
		{
			//std::cout << "filepointer move failed, filepointer = " << ((int)fp) << std::endl;		
			return false;
		}

		ReadBlock block(job.h, 0);

		/*read num partitions and nodes*/ 
		block.read(p_count);
		block.read(n_count);

		/*build partitions*/ 
		uint i;
		float v;
		std::vector<Voxel*> *voxels = const_cast< std::vector<Voxel*>* >(&cloud->voxels());
		std::vector<float> partitions;

		/* old version */ 
		for (i=0;i<p_count;i++)
		{
			block.read(v);
			partitions.push_back(v);
		}

		/*build nodes*/ 
		ubyte depth;
		ubyte nodetype;
		pt::BoundingBox extents;
		pt::BoundingBoxD extentsD;
		Node* children[8];
		int part[6];

		Node **tree = new Node*[n_count];
		int *node_children = new int[n_count*8];
		int *parents = new int[n_count];
		uint size = 0;
		
		// V5 Structure
		int numStrata = NUM_STRATA;
		float strataSpacing  = 0;
		int strata[NUM_STRATA*20];	// larger incase this is increased at a later date
		memset(strata, 0, sizeof(strata));
		//

		int64_t filepointer;

		for (i=0;i<n_count;i++)
		{
			float p[6];
			ubyte hasQT = 0;
			ubyte qt = 0;

			block.read(nodetype);
			block.read(depth);
			ubyte *ptr = (ubyte*)(void*)(&extents);
			ptr += sizeof(void*);
			
			//block.read( buff );
			block.read( hasQT );	// byte 1 unused
			block.read( hasQT );	// byte 2 unused
			block.read( hasQT );	// byte 3, should be 133 if following qt valid
			block.read( qt );

			block.read( ptr, 48 ); /* 44 bytes for BB + got a vftable 32 bit ptr in here from old implementation */ 

			// the extents are set here from the float values written to the POD file, however if the POD file being
			// loaded contains a double bounding box data block, these extents will be overwritten by the 
			// DoubleBoundingPodBlockBoxHandler handler (@see dbbxHandler->apply(cloud);)
			extentsD.setBox(extents.ux(), extents.lx(), extents.uy(), extents.ly(), extents.uz(), extents.lz());

			block.read(parents[i]);
			block.read(&node_children[i*8], INT_SIZE*8);

			if (p_count)	block.read(part, INT_SIZE*6);
			else 			block.read( p, sizeof(float)*6 );

			block.read(size);

			// V5 STRUCTURE : Strata info
			if (job.version[1] > 4 && nodetype)
			{
				block.read( numStrata );
				block.read( strataSpacing );
				block.read( strata, NUM_STRATA * sizeof(int) );
			}
			// END V5

			if (p_count)
				for (int j=0; j<6; j++) p[j] = partitions[part[j]];

			if (nodetype)
			{
				Voxel*vox;

				if (!skip)
				{
					tree[i] = vox = new Voxel(p, &p[3], depth, extentsD, size);
					vox->pointCloud(cloud);
				}
				
				uint c, ch, multiple;
				uint native, storeas;
				uint channels;
				bool quan64 = false;
				bool hasSamples = false;

				pt::vector3 offset32, scaler32;
				pt::vector3d offset64, scaler64;
				
				block.read(channels);

				int version = channels;
				if (version == 221004 || version == 160906)	//this is a version
				{
					if (version == 160906)
						hasSamples = true;
					block.read(channels);
					quan64 = true;
				}

				for (c=0; c<channels; c++)
				{
					uint num_samples = 0;
					ubyte sampledata[sizeof(pt::vector3d)*3*40];
					ubyte samplebuffer[sizeof(pt::vector3d)*3*20];

					block.read(ch);
					block.read(native);
					block.read(storeas);
					block.read(multiple);
					
					if (!quan64)
					{
						block.read(offset32);
						block.read(scaler32);
						offset64.set(offset32);
						scaler64.set(scaler32);
					}
					else
					{
						block.read(offset64);
						block.read(scaler64);
					}
					if (hasSamples)
					{	
						/* read samples */ 
						block.read(num_samples, "Num samples");
						block.read(sampledata, num_samples * dataTypeSize(pcloud::datatype(storeas)) * multiple, "Sample Data");

						/* read past rubbish samples */ 
						if (PT_NUM_DC_SAMPLES - num_samples > 0)
						{
							block.read(samplebuffer, (PT_NUM_DC_SAMPLES - num_samples) * dataTypeSize(pcloud::datatype(storeas)) * multiple);
						}
					}
					if (!skip)
					{
						vox->addChannel(pcloud::channel(ch), pcloud::datatype(native), pcloud::datatype(storeas), 
							multiple, 0, offset64, scaler64, sampledata, num_samples);
					}
				}

				block.read(filepointer);
				
				vox->filePointer(filepointer);
				
				if (job.version[1] > 4)
				{
					block.advance(1024);	//reserved block

					// V5 strata stuff
					vox->setStrataSizes( numStrata, strata, strataSpacing );
				}
				
				if (!skip)
				{
					voxels->push_back(vox);
				}
			}
			else
			{
				if (qt > 0 && hasQT == 133) qt = 1 << (qt-1);
				else qt = 0;

				if (!skip)
				{
					tree[i] = new Node(p, &p[3], depth, extentsD, qt);	
				}

				if (job.version[1] > 4)
				{
					block.advance(1024);	// reserved block
				}
			}
		}

		/*link parent/child*/ 
		if (!skip)
		{
			for (i=0;i<n_count;i++)
			{
				for (int j=0;j<8;j++)
				{
					if (node_children[i*8+j] != -1)
						children[j] = tree[node_children[i*8+j]];
					else children[j] = 0;
				}

				tree[i]->setChildren(children);
				if (parents[i] != -1)
					tree[i]->reparent(tree[parents[i]]);
			}
			/*set root of point cloud*/ 
			cloud->setRoot(tree[0]);
		}

		job.state = ptds::DataSourceStateClosed;

		delete [] tree;
		delete [] parents;
		delete [] node_children;

		// Create any additional handlers that may be needed to read PodBlocks (possibly move this somewhere better)
		DoubleBoundingPodBlockBoxHandler* dbbxHandler = new DoubleBoundingPodBlockBoxHandler;

		// Read any PodBlocks that are in this file
		job.m_podBlockManager.read(block);

		// Apply the data read by the DoubleBoundingPodBlockBoxHandler to the point cloud
		if (!skip)
		{
			dbbxHandler->apply(cloud);
		}

		if (block.numErrors())
			return false;

		return true;
	}
	return false;
}
//-------------------------------------------------------------------------
// dump Version
//-------------------------------------------------------------------------
bool PodIO::dumpVersion(PodJob &job)
{
	if (!readTracker) readTracker = new Tracker(job.h);
	else readTracker->reset();

	ubyte version[4];
	ubyte endian_ = 0; 

	job.h->readBytes(version, 4);
	job.h->readBytes(endian_);

	readTracker->advance(5);

	std::cout << "=========================================" << std::endl;
	std::cout << "Pointools POD file Version " 
		<< (int)version[0] 
		<< (int)version[1] 
		<< (int)version[2] 
		<< (int)version[3] 
		<< std::endl;
	std::cout << (endian_ ?  "Big " : "Little ") << "Endian" << std::endl;

	return true;
}
#ifdef RELEASE_VERSION
bool PodIO::dumpHeader(pcloud::PodJob &job) { return false; }
bool PodIO::dumpData(pcloud::PodJob &job) { return false; }
bool PodIO::dumpCloudStructure(pcloud::PodJob &job, PointCloud *cloud) { return false; }
#else
//-------------------------------------------------------------------------
// dump Header
//-------------------------------------------------------------------------
bool PodIO::dumpHeader(pcloud::PodJob &job)
{
	if (!readTracker) readTracker = new Tracker(job.h);
	ReadBlock rb(job.h, readTracker, "Header");

	char desc[64];
	rb.read(desc, 64);

	uint num_clouds;
	rb.read(num_clouds, "Number of Clouds");

	int64_t guid;
	int64_t structure_pointer;
	int64_t data_pointer;

	for (uint i=0; i<num_clouds; i++)
	{
		rb.read(guid, "Cloud GUID");
		rb.read(structure_pointer, "Structure Pointer");
		rb.read(data_pointer, "Data Pointer");
		
		PointCloud *pc = job.scene->newCloud();
		job.scene->addCloud(pc);

		pc->filepointer(structure_pointer);
	}
	return true;
}
bool PodIO::dumpData(pcloud::PodJob &job)
{
	if (job.h->validHandle())
	{
		PointCloud *pc = job.scene->cloud(0);
		if (!pc) return false;

		Voxel *vox = pc->voxels()[0];
		if (!vox) return false;

		DataChannel *dc = vox->channel(PCloud_Geometry);
		if (!dc) return false;

		int limit = 50;

		pt::vector3 v[64];
		job.h->movePointerTo(vox->filePointer());
		job.h->readBytes(v, limit * sizeof(pt::vector3));

		printf("%d Points:\n", limit);

		for (int i=0; i<limit; i++)
		{
			printf("Point %d: %f,%f,%f\n", i, v[i].x, v[i].y, v[i].z);
		}
		return true;
	}
	else printf("File Not Open\n");
	return false;
}
//-------------------------------------------------------------------------
// dump Structure
//-------------------------------------------------------------------------
bool PodIO::dumpCloudStructure(pcloud::PodJob &job, PointCloud *cloud)
{
	std::cout << "Starting pod structure read...\n";

	if (job.h->validHandle())
	{
		job.state = ptds::DataSourceStateRead;

		uint p_count, n_count;

		/*position file pointer*/ 
		int64_t fp = cloud->filepointer();
		if (!fp) return false;
		
		std::cout << "Current pos = " << readTracker->position() 
			<< " file pointer = " << ((int)fp) << std::endl;

		if (!readTracker->moveTo(fp))
		{
			std::cout << "File pointer position failed\n";
			return false;
		}

		ReadBlock block(job.h, readTracker, "Structure");

		/*read num partitions and nodes*/ 
		block.read(p_count, "Partitions");
		block.read(n_count, "Nodes");

		/*build partitions*/ 
		uint i;
		float v;

		for (i=0;i<p_count;i++)
		{
			block.read(v);
		}
		std::cout << "\tRead " << p_count << " partitions" << std::endl;
		std::cout << "Position after Partitions = " << block.position() << std::endl;

		/*build nodes*/ 
		ubyte depth;
		ubyte nodetype;
		pt::BoundingBox extents;
		int part[6];

		Node **tree = new Node*[n_count];
		int *node_children = new int[n_count*8];
		int *parents = new int[n_count];
		uint size = 0;

		int64_t filepointer;

		std::cout << "\tReading Node data..." << std::endl;

		int voxels = 0;

		for (i=0;i<n_count;i++)
		{
			block.read(nodetype, "Nodetype");
			block.read(depth, "Depth");
			block.read(extents);
			block.read(parents[i], "Parent");
			block.read(&node_children[i*8], INT_SIZE*8, "Children");
			block.read(part, INT_SIZE*6, "Partitions");
			block.read(size, "Pointcount");

			std::cout <<"N.." << (int)depth << ".." << size << " pnts " << std::endl;

			if (nodetype)
			{
				uint c,ch, multiple;
				DataType native, storeas;
				uint channels;
				
				block.read(channels, "Number Channels");

				for (c=0; c<channels; c++)
				{
					block.read(ch, "ChannelID");
					block.read(native, "Native");
					block.read(storeas, "StoreAs");
					block.read(multiple, "Multiple");
					voxels++;
				}
				block.read(filepointer, "Voxel Pointer");
			}
			std::cout << std::endl;
		}
		std::cout << "Read " << n_count << " nodes" << std::endl;
		std::cout << "Read " << voxels << " voxels" << std::endl;

		job.state = ptds::DataSourceStateClosed;

		ptds::dataSourceManager.close(job.h);

		delete [] tree;
		delete [] parents;
		delete [] node_children;

		if (block.numErrors())
			return false;

		return true;
	}
	return false;
}
#endif

//-------------------------------------------------------------------------
// PodBlock management
//-------------------------------------------------------------------------
/** Set the data portion of a PodBlock. This will assert in debug mode if
 data has already been set for this PodBlock.
 @param data	Pointer to the data to be written for this PodBlock
 @param dataLen	Size of data in bytes
 */
void PodBlock::setData(void* data, unsigned long dataLen) 
{
#ifdef _DEBUG
	// The data should only be set if this PodBlock does not already have data and does
	// not have a WriteDataCallback. When written is there is a WriteDataCallback then that
	// will be used for writing this PodBlock's data rather than the data stored in this PodBlock.
	assert(m_data == NULL);
	assert(m_writeDataCallback == NULL);
#endif // _DEBUG
	m_data = data; 
	m_dataLen = dataLen; 
}
/** Write this PodBlock to the passed WriteBlock.
 PodBlocks are written as follows:
 - 4 byte block identifier, e.g "DBBX"
 - 4 byte block version
 - 4 byte block data size
 - block data

 @param wb	The WriteBlock to write this PodBlock's data to
 @return	true if all information about this PodBlock was written correctly,
			false otherwise
 */
bool PodBlock::write(ptds::WriteBlock& wb)
{
#ifdef _DEBUG
	assert(m_id.length() == 4);
#endif // _DEBUG

	bool res = wb.write(m_id.c_str(), 4);

	if (res)
		res = wb.write(m_version);

	if (m_writeDataCallback)
	{
		if (res)
			res = m_writeDataSizeCallback->writePodBlockDataSize(wb);

		if (res)
			res = m_writeDataCallback->writePodBlockData(wb);
	}
	else
	{
		if (res)
			res = wb.write(m_dataLen);

		if (res)
			res = wb.write(m_data, m_dataLen);
	}

	return res;
}

// Static member vars
PodBlockManager* PodBlockManager::_instance = NULL;

PodBlockManager::PodBlockManager() :
m_blockReadPos(0)
{ 
#ifdef _DEBUG
	assert(_instance == NULL);
#endif // _DEBUG

	_instance = this;
}

PodBlockManager::~PodBlockManager()
{
#ifdef _DEBUG
	assert(_instance);
#endif // _DEBUG

	clear();	

	PodBlockHandlers::iterator it;
	for (it = m_handlers.begin(); it != m_handlers.end(); it++)
	{
		if (PodBlockReadHandler* handler = it->second)
			delete handler;
	}
	m_handlers.clear();

	_instance = NULL;
}

const PodBlockManager& PodBlockManager::instance() 
{
#ifdef _DEBUG
	assert(_instance);
#endif // _DEBUG

	return *_instance; 
}

void PodBlockManager::clear()
{
	// clean up PodBlocks
	PodBlocks::iterator it;
	for (it = m_blocks.begin(); it != m_blocks.end(); it++)
	{
		if (PodBlock* block = (*it))
			delete block;
	}
	m_blocks.clear();
}

bool PodBlockManager::write(ptds::WriteBlock& wb)
{
	int errs = 0;
	PodBlocks::iterator it;

	// Overwrite the placeholder saved earlier with the position of the blocks
	wb.tracker()->placeReference(STRUCTURE_BLOCK_TRACKER_ID);

	// Write the number of PodBlocks
	wb.write(static_cast<unsigned int>(m_blocks.size()));

	for (it = m_blocks.begin(); it != m_blocks.end(); it++)
	{
		if (PodBlock* block = (*it))
			if (!block->write(wb))
				errs++;
	}

	return (errs == 0);
}

bool PodBlockManager::read(ptds::ReadBlock& rb)
{
	if (!m_blockReadPos)
		return false;

	// Read the number of PodBlocks
	unsigned int numPodBlocks = 0;
	rb.read(numPodBlocks);

	for (uint i = 0; i < numPodBlocks; i++)
	{
		// Read the type of PodBlock then match it with a registered PodBlock handler
		char type[5] = {0};
		rb.read(type, 4);
		
		readPodBlock(type, rb);
	}

	return true;
}

PodBlockReadHandler* PodBlockManager::handler(const char* type)
{
	if (!type || (strlen(type) != 4))
		return NULL;

	PodBlockHandlers::iterator it = m_handlers.find(std::string(type));
	if (it != m_handlers.end())
		return it->second;

	return NULL;
}

bool PodBlockManager::readPodBlock(const char* type, ptds::ReadBlock& rb)
{
	// Find the handler for this type
	if (PodBlockReadHandler* h = handler(type))
		return h->read(rb);

	return false;
}

PodBlockReadHandler::PodBlockReadHandler(std::string const& id) : 
m_id(id) 
{ 
#ifdef _DEBUG
	// PodBlock IDs must be 4 characters long
	assert(m_id.length() == 4);
#endif // _DEBUG

	PodBlockManager::instance().registerHandler(id, this); 
}
