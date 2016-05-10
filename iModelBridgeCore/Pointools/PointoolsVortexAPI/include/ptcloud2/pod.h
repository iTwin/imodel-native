#ifndef POINTOOLS_PCLOUD2_POD
#define POINTOOLS_PCLOUD2_POD

#include <ptds/filepath.h>
#include <ptcloud2/pointcloud.h>
#include <ptcloud2/pcloud.h>
#include <ptcloud2/scene.h>
#include <list>


// Supported POD file version, current supported version is 4, 4, 0, 255 (header, structure, data, reserved).
// *** To enable the new strata code set the version to 4, 6, 0, 255 ***
#define POD_HEADER_VERSION		4
#define POD_STRUCTURE_VERSION	4
#define POD_DATA_VERSION		0
#define POD_RESERVED_VERSION	255

namespace pcloud
{
	/** PodBlock class, used internally by PodBlockManager when managing details of PodBlocks
	 to be written to the current POD file.
	 */
	class PodBlock
	{
	public:
		class WriteDataCallback
		{
		public:
			/** This callback will be called for all PodBlocks owned by the PodBlockManager when
			PodBlockManager::write() is called. Implementers can write any data they like in here,
			writing of the PodBlock type, version and size is handled by the PodBlockManager.
			 */
			virtual bool writePodBlockData(ptds::WriteBlock& wb) = 0;
		};

		class WriteDataSizeCallback
		{
		public:
			/** This callback will be called for all PodBlocks owned by the PodBlockManager when
			PodBlockManager::write() is called. Implementers must write a single unsigned int
			representing the size of the data written in WriteDataCallback::writePodBlockData().
			 */
			virtual bool writePodBlockDataSize(ptds::WriteBlock& wb) = 0;
		};

		/** Constructor for a PodBlock where the data and data length are already known
		 */
		PodBlock(std::string const& id, unsigned int version, void* data, unsigned int dataLen) :
		  m_id(id),
			  m_version(version),
			  m_data(data),
			  m_dataLen(dataLen),
			  m_writeDataCallback(NULL)
		  { ; }

		  /** Constructor for a PodBlock where the data and data length are not known, but a callback 
		   object is provided which will be called at the point at which the data is to be written
		   */
		  PodBlock(std::string const& id, unsigned int version, WriteDataCallback* writeDataCallback, WriteDataSizeCallback* writeDataSizeCallback) :
		  m_id(id),
			  m_version(version),
			  m_data(NULL),
			  m_dataLen(0),
			  m_writeDataCallback(writeDataCallback),
			  m_writeDataSizeCallback(writeDataSizeCallback)
		  { ; }

		  void setData(void* data, unsigned long dataLen);

		  bool write(ptds::WriteBlock& wb);

	private:
		std::string		m_id;
		unsigned int	m_version;
		void*			m_data;
		unsigned int	m_dataLen;

		WriteDataCallback* m_writeDataCallback;
		WriteDataSizeCallback* m_writeDataSizeCallback;
	};	

	// Pod Block IDs ------------------------------------------------------------
	static char* PODBLOCK_DOUBLEBOUNDINGBOX = "DBBX";
	// --------------------------------------------------------------------------

	class PodBlockReadHandler	
	{
	public:
		PodBlockReadHandler(std::string const& id);

		// All PodBlockHandlers must implement this function which is called by the PodBlockManager on
		// encountering a PodBlock matching this handlers ID.
		// The first unsigned int in the ReadBlock is the version of the PodBlock being read, 
		// the second unsigned int is the size of the data in the PodBlock, everything
		// following is dependant on the type of PodBlock being read.
		virtual bool read(ptds::ReadBlock& rb) = 0;

	private:
		// 4 character id that defines the type of block
		std::string m_id;
	};

	/** PodBlockManager, any PodBlocks are added will be written to a block section after the structure
	 of the POD file is writtem, but before the point cloud data. Add PodBlocks by calling add().
	 */
	class PodBlockManager
	{
	public:
		PodBlockManager();
		virtual ~PodBlockManager();

		static const PodBlockManager& instance();

		void clear();

		/** The PodBlockManager takes ownership of all added PodBlocks, these can be deleted by calling clear(). 
		 */
		void add(PodBlock* block) { if (block) m_blocks.push_back(block); }

		/** Write all currently stored PodBlocks to the passed WriteBlock.
		 */
		bool write(ptds::WriteBlock& wb);

		/** Read the PodBlocks, this is dependant on setReadBlockPos() being called previously to set the
		 file pointer that the PodBlocks should be read from.
		 */
		bool read(ptds::ReadBlock& rb);

		/** Store the position of the PodBlocks, this position will be used when the blocks are read
		 */
		void setReadBlockPos(ptds::DataPointer& pos) { m_blockReadPos = pos; }

		static void registerHandler(std::string const& id, PodBlockReadHandler* handler) { if (_instance) _instance->m_handlers.insert(std::pair<std::string, PodBlockReadHandler*>(id, handler)); }

	private:
		PodBlockReadHandler* handler(const char* type);
		bool readPodBlock(const char* type, ptds::ReadBlock& rb);
		

		typedef std::list<PodBlock*> PodBlocks;
		PodBlocks m_blocks;

		typedef std::map<std::string, PodBlockReadHandler*> PodBlockHandlers;
		PodBlockHandlers m_handlers;

		ptds::DataPointer m_blockReadPos;
 
		static PodBlockManager* _instance;
	};

	class PCLOUD_API PodJob
	{
	public:
		PodJob(Scene *s, const ptds::FilePath &fp) 
			: scene(s), filepath(fp), fileSize(0) {}

		ptds::FilePath					filepath;
		ptds::DataSourceState			state;
		ptds::DataSourcePtr				h;
		std::vector<ptds::DataPointer>	guidPtrs;
		PodBlockManager					m_podBlockManager;

		Scene							*scene;
		void							*tracker;
		ubyte							endian;
		ubyte							version[4];
		int64_t							fileSize;
	};
	class PCLOUD_API PodIO
	{
	public:
		static bool handlesVersion(ubyte *version);
		static bool writeFile(PodJob &job);

		static bool openForRead(PodJob &job);
		static bool openForWrite(PodJob &job);
		static bool close(PodJob &job);

		static bool readVersion(PodJob &job);
		static bool readHeader(PodJob &job, bool skip=false);
		static bool readSceneStructure(PodJob &job, bool skip=false);
		static bool readCloudStructure(PodJob &job, PointCloud *cloud, bool skip=false);
		
		static bool dumpVersion(PodJob &job);
		static bool dumpHeader(PodJob &job);
		static bool dumpCloudStructure(PodJob &job, PointCloud *cloud);
		static bool dumpData(PodJob &job);

		/*detailed control for incremental writing*/ 
		static bool writeVersion(PodJob &job);
		static bool writeHeader(PodJob &job);
		static bool writeStructure(PodJob &job);
		static bool writeData(PodJob &job);
		static bool writeCloudStructure(PodJob &job, int cloud_idx);
		static bool writeCloudData(PodJob &job, int cloud_idx, int start_voxel=0, int num_voxels=-1, int *initialization_sizes=0, bool delete_after_write=false);
		static bool reWriteCloudSizes(PodJob &job, int cloud_idx);

		// write meta data block into exisiting file - will fail for older file versions
		// file must be already opened read/write
		static bool writeMetaUpdate(PodJob &job, const MetaData &meta);

	private:
		static uint structureSize(PointCloud *cloud);
	};
}
#endif