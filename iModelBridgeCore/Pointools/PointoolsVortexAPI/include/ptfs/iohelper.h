#ifndef POINTOOLS_IO_HELPER
#define POINTOOLS_IO_HELPER

#include <ptds/DataSource.h>
#include <pt/typedefs.h>
#include <pt/boundingBox.h>
#include <pt/ptstring.h>
#include <ptfs/filepath.h>

#include <map>
#include <iostream>

#define INT_SIZE 4
#define INT64_SIZE 8
#define FLOAT_SIZE 4
#define READ_BUFFER_CHUNK 262144
#define PT_BLOCK_MAX_STRING_LENGTH 1024

namespace ptds
{
#ifdef NEEDS_WORK_VORTEX_DGNDB
	//-------------------------------------------------------------------------
	// simple io helper
	//-------------------------------------------------------------------------
	struct IOhelper
	{
		IOhelper(const DataSourcePtr &h)
		{
			H = h;
			pos = 0;
		}
		template <class T>
		bool write(const T &data)
		{
			pos += sizeof(T);
			return IO::writeBytes(H, &data, sizeof(T))== sizeof(T);
		}
		template <class T>
		bool read(T &data)
		{
			pos += sizeof(T);
			return IO::readBytes(H, &data, sizeof(T))== sizeof(T);
		}
		bool writeBoundingBox(const pt::BoundingBox &bb)
		{
			pos += sizeof(float) * 6;

			return 	(H->writeBytes(&bb.lx(), sizeof(float))== sizeof(float)
				&& H->writeBytes(&bb.ly(), sizeof(float))== sizeof(float)
				&& H->writeBytes(&bb.lz(), sizeof(float))== sizeof(float)
				&& H->writeBytes(&bb.ux(), sizeof(float))== sizeof(float)
				&& H->writeBytes(&bb.uy(), sizeof(float))== sizeof(float)
				&& H->writeBytes(&bb.uz(), sizeof(float))== sizeof(float));
		}
		bool readBoundingBox(pt::BoundingBox &bb)
		{
			pos += sizeof(float) * 6;
			float lower[3];
			float upper[3];

			if 	  (H->readBytes(&lower[0], sizeof(float)) == sizeof(float)
				&& H->readBytes(&lower[1], sizeof(float))== sizeof(float)
				&& H->readBytes(&lower[2], sizeof(float))== sizeof(float)
				&& H->readBytes(&upper[0], sizeof(float))== sizeof(float)
				&& H->readBytes(&upper[1], sizeof(float))== sizeof(float)
				&& H->readBytes(&upper[2], sizeof(float))== sizeof(float))
			{
				bb.set(lower,upper);
				return true;
			}
			return false;
		}

		bool read(void *ptr, uint bytes)
		{
			pos += bytes;
			return H->readBytes(ptr, bytes) == bytes;
		}

		bool write(const void *ptr, uint bytes)
		{
			pos += bytes;
			return H->writeBytes(ptr, bytes) == bytes;
		}

		bool shift(uint bytes)
		{
			pos += bytes;
			return H->movePointerBy(bytes);
		}

		bool position(uint bytes)
		{
			pos = bytes;
			return H->movePointerTo(bytes);
		}

		DataSourcePtr	H;
		DataPointer		pos;
	};
#endif
	//-------------------------------------------------------------------------
	// Tracker helper
	//-------------------------------------------------------------------------
	class Tracker
	{
	protected:
		Tracker() :  _filepointer(0) {};
	public:	
		Tracker(const DataSourcePtr &h) : _H(h), _filepointer(0) {}
		virtual ~Tracker() { }

		/*dont use write through when a WriteBlock object exists!!*/ 
		template <class T> bool writeThrough(const T &d)
		{
			if (_H->writeBytes(d) != sizeof(T)) return false;
			_filepointer += sizeof(T);
			return true;
		}
		bool writeThrough(const void *d, uint size)
		{
			if (size != _H->writeBytes(d, size)) return false;
			_filepointer += size;
			return true;
		}
		template <class T> void readThrough(T &d)
		{
			_H->readBytes(d);
			_filepointer += sizeof(T);
		}
		void readThrough(void *d, uint size)
		{
			_H->readBytes(d, size);
			_filepointer += size;
		}

		DataPointer position() const { return _filepointer; }
		
		/* Warning: does not move the filepointer itself */ 
		void advance(uint bytes) { _filepointer += bytes; }

		/*place holders for references*/ 
		void insertPlaceholder(uint id)
		{
			_refplaceholders.insert(REFPLACEHOLDERS::value_type(id, std::pair<DataPointer, DataPointer>(_filepointer, 0)));
			_filepointer += INT64_SIZE;
		}
		/*place holders for data*/ 
		void insertDataPlaceholder(uint id, uint num_bytes)
		{
			_dataplaceholders.insert(DATAPLACEHOLDERS::value_type(id, std::pair<DataPointer, uint>(_filepointer, num_bytes)));
			_filepointer += num_bytes;
		}
		/*pointer to re-write data later - DOES NOT RESERVE SPACE OR MOVE FILEPOINTER*/ 
		void saveFilePointer(uint id)
		{
			_savedfilepointers.insert(FILEPOSITIONPOINTERS::value_type(id, _filepointer));
		}
		bool getFilePointer(uint id, DataPointer &fpointer) const
		{
			if (_savedfilepointers.count(id))
			{
				fpointer = _savedfilepointers.find(id)->second;
				return true;
			}
			return false;
		}
		void clearFilePointers() { _savedfilepointers.clear(); }

		bool moveToPointer(uint id)
		{
			if (getFilePointer(id, _filepointer))
			{
				moveToPosition();
				return true;
			}
			return false;
		}
		/*write this straight to the file */ 
		bool writePlacedData(uint id, const void *data, uint num_bytes)
		{
			DATAPLACEHOLDERS::iterator i;
			i = _dataplaceholders.find(id);
			if (i!= _dataplaceholders.end())
			{
				_H->movePointerTo(i->second.first);
				_H->writeBytes(data, num_bytes); 
				moveToPosition();
				return true;
			}
			return false;
		}
		void placeReference(uint id)
		{
			/*find placeholder*/ 
			REFPLACEHOLDERS::iterator i = _refplaceholders.find(id);
			if (i!= _refplaceholders.end())
			{
				i->second.second = _filepointer;
			}
#ifdef _VERBOSE
			else std::cout << "ERROR: placeholder " << id << " could not be found" << std::endl;
#endif
		}
		void writePlaceHolders()
		{
			REFPLACEHOLDERS::iterator i;
			for (i= _refplaceholders.begin(); i != _refplaceholders.end(); i++)
			{
				_H->movePointerTo(i->second.first);
				_H->writeBytes(i->second.second);
			}
		}
		bool moveBy(DataPointer offset)
		{
			if (_H->movePointerBy(offset))
			{
				_filepointer += offset;
#ifdef _VERBOSE
				std::cout << "<File Pointer moved by " << offset << " to " << position() << ">" << std::endl;
#endif
				return true;
			}
			return false;
		}
		bool moveTo(DataPointer fp)
		{
			if (fp == _filepointer) return true;

			if (_H->movePointerTo(fp))
			{
				_filepointer = fp;
#ifdef _VERBOSE
				std::cout << "<File Pointer moved to " << position() << ">" << std::endl;
#endif
				return true;
			}
			return false;
		}
		bool moveToPosition()
		{
			return _H->movePointerTo(_filepointer);
		}
		void setHandle(DataSourcePtr &h) { _H = h; }
		void reset() { _filepointer = 0; _H->movePointerTo(0); }
	protected:
		
		typedef std::map<uint, std::pair<DataPointer, DataPointer> > REFPLACEHOLDERS;
		typedef std::map<uint, std::pair<DataPointer, uint> > DATAPLACEHOLDERS;
		typedef std::map<uint, DataPointer> FILEPOSITIONPOINTERS;

		DataSourcePtr _H;
		REFPLACEHOLDERS 		_refplaceholders;
		DATAPLACEHOLDERS 		_dataplaceholders;
		FILEPOSITIONPOINTERS 	_savedfilepointers;
	private:
		DataPointer 	_filepointer;
	};
	
	//----------------------------------------------------------------------
	//DataBlock base class
	//----------------------------------------------------------------------
	class DataBlock
	{
	public:
		DataBlock(DataSourcePtr &h, Tracker *tracker) :
		  _H(h), 
		  _pos(0), 
		  _buffer(0), 
		  _buffersize(0), 
		  _tracker(tracker) 
		  {}

		DataPointer 	pos() const { return _pos; }
		Tracker 		*tracker() 	{ return _tracker; }

	protected:
		DataPointer _pos;
		DataSize 	_buffersize;
		DataSourcePtr 	_H;
		ubyte 		*_buffer;	
		Tracker 	*_tracker;
	};

	class BlockErrorTracker
	{
	public:
		BlockErrorTracker() : m_numErrors(0) { ; }
		void reset(void) { m_numErrors = 0; }
        void increment(void) { ++m_numErrors; }
		long numErrors(void) 
		{ 
			return m_numErrors; 
		}

	private:
		std::atomic<long> m_numErrors;
	};

	//----------------------------------------------------------------------
	// Block Reader
	//----------------------------------------------------------------------
	class ReadBlock :  public DataBlock
	{
	public:
		ReadBlock(DataSourcePtr &h, Tracker *tracker, void *buffer, const char*note=0, bool blockSize64bit = false)
			: DataBlock(h, tracker)
		{
			if (!blockSize64bit)
			{
				uint buffersize32bit = 0;
				if (!h->readBytes(buffersize32bit))
					_blockErrors.increment();
				_buffersize = buffersize32bit;
			}
			else
			{
				h->readBytes(_buffersize);
			}
			_buffer = 0;
			if (!h->readBytes(buffer, _buffersize))
				_blockErrors.increment();
			if (_tracker)
			{
                if (note)
                {
#ifdef _VERBOSE
                    std::cout << std::endl << "[" << (note? note : " ") << " Block  | " << _buffersize << "bytes  | fp = " << _tracker->position() << " ] " << std::endl;
                    std::cout << "{" << std::endl;
#endif
                }
				_tracker->advance(INT_SIZE);		
				_tracker->advance(_chunksize);
			}
		}
		ReadBlock(DataSourcePtr &h, Tracker *tracker, const char*note=0, bool blockSize64bit = false) 
			: DataBlock(h, tracker)
		{
		
			if (!blockSize64bit)
			{
				uint buffersize32bit = 0;
				if (!h->readBytes(buffersize32bit))
					_blockErrors.increment();
				_buffersize = buffersize32bit;
			}
			else
			{
				h->readBytes(_buffersize);
			}
			_chunksize = _buffersize > READ_BUFFER_CHUNK ? READ_BUFFER_CHUNK : _buffersize;
			_chunkstart = 0;

			_buffer = new ubyte[_chunksize];
			
			if (_tracker)
			{
                if (note)
                {
#ifdef _VERBOSE
                    std::cout << std::endl << "[" << (note? note : " ") << " Block  | " << _buffersize << "bytes  | fp = " << _tracker->position() << " ] " << std::endl;
                    std::cout << "{" << std::endl;
#endif
                }
				_tracker->advance(INT_SIZE);		
				_tracker->advance(_chunksize);
			}
			if (!h->readBytes(_buffer, _chunksize))
				_blockErrors.increment();
		}
		~ReadBlock()
		{
			if (_tracker)
			{
#ifdef _VERBOSE
				std::cout << "} " << " read " << _pos << " bytes" << "+ 4";
				std::cout << "  | fp = " << _tracker->position() << std::endl;
#endif
			}
			if (_buffer) delete [] _buffer;
		}
		DataPointer position()
		{
			if (_tracker) return tracker()->position() - _buffersize + _pos;
			return 0;
		}
		uint readChunk(int size = READ_BUFFER_CHUNK)
		{
			/*check end condition*/ 
			if (_pos >= _buffersize) return 0;			
			if (_pos + size > _buffersize)	size = _buffersize - _pos;

			/*calc remaining unread of chunk*/ 
			int chunkunread = _chunksize - (_pos - _chunkstart);

			size += chunkunread;

			/*allocate buffer*/ 
			ubyte *buffer = new ubyte[size];
			
			/*copy unread stuff into new buffer*/ 
			if (chunkunread > 0)
				memcpy(buffer, &_buffer[_chunksize - chunkunread], chunkunread);
			else chunkunread = 0;

			delete [] _buffer;
			_buffer = buffer;
			
			_chunksize = size;
			_chunkstart = _pos;

			/*read chunk*/ 
			unsigned int ret =_H->readBytes(&_buffer[chunkunread], _chunksize-chunkunread);
			if (!ret)
				_blockErrors.increment();
	
			return ret;
		}
		/*read																*/ 
		template <class T> uint read(T &d)
		{
			if (_pos + uint(sizeof(T)) > _chunkstart + _chunksize)
				if (!readChunk()) 
				{
					_blockErrors.increment();
					return 0;
				}
			
			memcpy(&d, &_buffer[_pos-_chunkstart], sizeof(T));

			_pos += sizeof(T);
			return sizeof(T);
		}
		/*read	with output															*/ 
		template <class T> uint read(T &d, const char* note)
		{
            if (note)
            {
#ifdef _VERBOSE
                if (_tracker && note) std::cout << "     " << note << " : fp = " << position() 
                    << " value = " << d << std::endl;
#endif
            }
			
			unsigned int ret = read(d);
			if (!ret)
				_blockErrors.increment();

			return ret;
		}
		uint read( pt::String &str )
		{
			wchar_t buffer[PT_BLOCK_MAX_STRING_LENGTH];
			memset(buffer, 0, sizeof(buffer));

			int len = 0;
			read(len);

			if (len > 0 && len < PT_BLOCK_MAX_STRING_LENGTH) /* sanity check */ 
			{
				uint size = (read(buffer, (len+1)*sizeof(wchar_t)));
				if (!size) 
				{
					_blockErrors.increment();
					return 0;
				}

				str = buffer;
				return size;
			}
			else 
			{
				_blockErrors.increment();
				return 0;			
			}
		}
		uint read(void *d, int size)
		{
			if (_pos + size > _chunkstart + _chunksize)
				if (!readChunk(READ_BUFFER_CHUNK > size ? READ_BUFFER_CHUNK : size)) 
				{
					_blockErrors.increment();
					return 0;
				}

			memcpy(d, &_buffer[_pos-_chunkstart], size);

			_pos += size;
			return size;
		}
		uint read(void *d, int size, const char* note)
		{
            if (note)
            {
#ifdef _VERBOSE
                if (_tracker && note) std::cout << "     " << note << " : fp = " << position() << std::endl;			
#endif			
            }
			unsigned int ret = read(d, size);
			if (!ret)
				_blockErrors.increment();

			return ret;
		}
		bool advance(int size) 
		{ 
			if (_pos + size > _chunkstart + _chunksize)
			{
				if (!readChunk())
				{
					_blockErrors.increment();
					return false;		
				}
			}
			_pos += size;
			return true;
		}

		int size() const { return _buffersize; }

		unsigned int numErrors(void) { return _blockErrors.numErrors(); }

		uint				_chunksize;
		int64_t				_chunkstart;

		BlockErrorTracker	_blockErrors;
	};
	//----------------------------------------------------------------------
	// Block Writer
	//----------------------------------------------------------------------
	class WriteBlock : public DataBlock
	{
	public:
		WriteBlock(DataSourcePtr &h, DataSize buffersize, Tracker *tracker, uint refid = 0, const char* note=0, bool blockSize64bit = false)  
			: DataBlock(h, tracker), _is64bitSize(blockSize64bit)
		{
			/*record position if this is referenced*/ 
			if (refid)	{ tracker->placeReference(refid); }
			_startpos = _tracker->position();
            if (note)
            {
#ifdef _VERBOSE
                std::cout << std::endl << "[ " << (note ? note : " ") << " Block | fp = " << _tracker->position() << "] " << std::endl;
                std::cout << "{ " << std::endl;
#endif
            }
			_buffersize = buffersize;		
			_buffer = new ubyte[_buffersize];
			_commitpos = 0;

			/*shift forward for size*/ 
			if (blockSize64bit)
			{
				DataPointer res = 0;
				_tracker->writeThrough(res);		// reserved for overall blocksize (buffersize)
			}
			else
			{
				uint res = 0;
				_tracker->writeThrough(res);		// reserved for overall blocksize (buffersize)
			}
			// _pos tracks local buffer
			// when buffer full commit to disk
		};
		~WriteBlock()
		{
			close();
		}
	
		/* start a write through pass - un-buffered*/  
		/* DO NOT USE ANY WRITE FUNCTIONS ON THIS PASS OTHER THAN WRITETHROUGH */ 
		void startWriteThroughPass()
		{
			commitBuffer();

			DataPointer size = _commitpos;
			_tracker->moveTo(_startpos);
			_tracker->writeThrough(size);

			delete [] _buffer;
			_buffer = 0;
			_commitpos = 0;
		}
		void writeThroughAdvance(DataSize num_bytes)
		{
			_tracker->moveBy(num_bytes);
		}
		template <class T> bool writeThrough(const T &d)
		{
			bool res = _tracker->writeThrough(d);
			if (!res)
				_blockErrors.increment();

			return res;
		}
		template <class T> bool write(const T &d)
		{
			assert(_buffer);

			if (_buffersize < _pos + uint(sizeof(T)))	
			{
				if (!commitBuffer()) 
				{
					_blockErrors.increment();
					return false;
				}
			}
			
			memcpy(&_buffer[_pos], &d, sizeof(T));

			_pos += sizeof(T);
			_tracker->advance(sizeof(T));
			return true;
		}
		template <class T> bool write(const T &d, const char *note)
		{
			assert(_buffer);
            if (note)
            {
#ifdef _VERBOSE
                std::cout << "     " << note << " : fp = " << _tracker->position() << 
                    "  value = " << d << std::endl;
#endif
            }
			bool res = write(d);
			if (!res)
				_blockErrors.increment();

			return res;
		}
		bool write(const void *d, uint size)
		{
			assert(_buffer);

			if (size > _buffersize) expandBuffer(size+_pos);
			if (_buffersize < _pos + size)
			{
				if (!commitBuffer())
				{
					_blockErrors.increment();
					return false;
				}
			}
			
			memcpy(&_buffer[_pos], d, size);

			_pos += size;
			_tracker->advance(size);
			return true;
		}
		bool write(const pt::String &str)
		{
			int len = str.length();
			if (!write(len)) 
			{
				_blockErrors.increment();
				return false;
			}
		
			/* write string and include null char */
			if (len)
			{
				bool res = (write(str.c_wstr(), (len+1) * sizeof(wchar_t)));
				if (!res)
					_blockErrors.increment();

				return res;
			}
			else 
				return true;
		}

		bool write(const void *d, uint size, const char *note)
		{
			assert(_buffer);
            if (note)
            {
#ifdef _VERBOSE
                std::cout << "     " << note << " : fp = " << _tracker->position() << std::endl;			
#endif
            }
			bool res = write(d, size);
			if (!res)
				_blockErrors.increment();

			return res;
		}
		bool insertPlaceholder(uint id, const char* note=0)
		{
			assert(_buffer);
            if (note)
            {
#ifdef _VERBOSE
                if (note) std::cout << "     " << note << " : fp = " << _tracker->position() << std::endl;
#endif
            }
			if (_buffersize < _pos + INT64_SIZE) 
			{
				if (!commitBuffer())
				{
					_blockErrors.increment();
					return false;
				}
			}
			tracker()->insertPlaceholder(id);			
			_pos += INT64_SIZE;
			return true;
		}		
		bool insertDataPlaceholder(uint id, uint num_bytes, const char *note=0)
		{
			assert(_buffer);
            if (note)
            {
#ifdef _VERBOSE
                if (note) std::cout << "     " << note << " : fp = " << _tracker->position() << std::endl;
#endif
            }
			if (_buffersize < _pos + num_bytes) 
			{
				if (!commitBuffer())
				{
					_blockErrors.increment();
					return false;
				}
			}
			tracker()->insertDataPlaceholder(id, num_bytes);			
			_pos += num_bytes;
			return true;
		}
		bool reserve(uint size)
		{
			assert(_buffer);

			if (size > _buffersize) expandBuffer(size+_pos);
			if (_buffersize < _pos + size) 
			{
				if (!commitBuffer())
				{
					_blockErrors.increment();
					return false;
				}
			}
#ifdef _VERBOSE
            std::cout << "     " << "Reserve " << " : fp = " << _tracker->position() << std::endl;
#endif
			memset(&_buffer[_pos], 0, size);

			_pos += size;
			_tracker->advance(size);
			return true;
		}
		DataPointer size() const { return _pos; }

		/** Called on destruction of the WriteBlock.
		 Call this prior to deleting a WriteBlock object in order to check for errors.
		 */
		bool close(void)
		{
			if (_buffer)
			{
				commitBuffer();
#ifdef _VERBOSE			
				std::cout << std::endl << "} Writing Block " << _commitpos << " bytes" << "+ 4";
				std::cout << "  | file position = " << tracker()->position() << std::endl;
#endif
				/*write the size.*/ 
				_H->movePointerTo(_startpos);
				
				if (_is64bitSize)
				{
					_H->writeBytes(_commitpos);	
				}
				else
				{
					uint size = _commitpos;
					_H->writeBytes(size);	
				}
			}
			/*move tracker back*/ 
			_tracker->moveToPosition();

			delete [] _buffer;
			_buffer = NULL;

			return (_blockErrors.numErrors() == 0);
		}
		unsigned int numErrors(void)	{ return _blockErrors.numErrors(); }

	private:
		bool expandBuffer(int size)
		{
#ifdef _VERBOSE
			std::cout << "#e";
#endif
			ubyte *buff = 0;

			try	{
				buff = new ubyte[size];
			}
			catch (std::bad_alloc) {
				_blockErrors.increment();
				return false;
			}
			_buffersize = size;

			memcpy(buff, _buffer, _pos);
			delete [] _buffer;
			_buffer = buff;
			return true;
		}
		bool commitBuffer()
		{
			assert(_buffer);

			if (!_pos) return true;
#ifdef _VERBOSE
			std::cout << "#c";
#endif

			/*write the buffer*/ 
			if (_pos != _H->writeBytes(_buffer, _pos))
			{
				_blockErrors.increment();
				return false;
			}
			_commitpos += _pos;

			memset(_buffer, 0, _buffersize);
			_pos = 0;
			return true;
		}

		DataPointer _commitpos;
		DataPointer _startpos;
		bool 		_is64bitSize;
		BlockErrorTracker	_blockErrors;
	};
//----------------------------------------------------------------------
	//Block Pager, page data to disk and retrieve it in the same instance
	//----------------------------------------------------------------------
	class BlockPager : public Tracker
	{
	public: 
		BlockPager(const wchar_t *filename)  
		{
			/*create the file*/ 
			ptds::FilePath fp(filename);
			_H = ptds::dataSourceManager.openForWrite(&fp);
			if(_H)
			{
				_H->writeBytes(_H);
				ptds::dataSourceManager.close(_H);

				/*open for read/write*/ 
				_H = ptds::dataSourceManager.openForReadWrite(&fp);
				if (_H->validHandle())
					setHandle(_H);	

				_filepath.setPath(filename);
			}
		}		

		~BlockPager()
		{
			if(_H)
				_H->closeAndDelete();
		}

		bool fileValid()
		{
			if(_H)
				return _H->validHandle(); 

			return false;
		}

		WriteBlock *rewriteBlock(DataSize buffersize, uint id, const char *note=0)
		{
			BLOCKMAP::iterator it = _blocks.find(id);
			if (it != _blocks.end())
			{
				moveTo(it->second);
				return new WriteBlock(_H, buffersize, this, 0, note);
			}		
			return 0;			
		}

		WriteBlock *newWriteBlock(DataSize buffersize, uint id, const char *note=0, bool blocksize64bit = false)
		{
			_blocks.insert(BLOCKMAP::value_type(id, position()));
			WriteBlock *wr = new WriteBlock(_H, buffersize, this, 0, note, blocksize64bit);
			return wr;
		}
		ReadBlock *newReadBlock(uint id, const char* note=0,  bool blockSize64bit = false)
		{
			BLOCKMAP::iterator it = _blocks.find(id);
			if (it != _blocks.end())
			{
				moveTo(it->second);
				return new ReadBlock(_H, this, note, blockSize64bit);
			}		
			return 0;
		}
		bool readBlock(uint id, void *data, const char* note=0, bool blockSize64bits = false)
		{
			BLOCKMAP::iterator it = _blocks.find(id);
			if (it != _blocks.end())
			{
				moveTo(it->second);
				ReadBlock rb(_H, this, data, note, blockSize64bits);
				return (rb.numErrors() == 0);
			}		
			return false;
		}
		const wchar_t* filepath() const { return _filepath.path(); }
	private:
		typedef std::map<uint, DataPointer> BLOCKMAP;
		BLOCKMAP _blocks;
		FilePath _filepath;
	};
}
#endif
