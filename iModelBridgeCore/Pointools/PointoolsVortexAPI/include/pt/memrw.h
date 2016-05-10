#ifndef MEM_READ_WRITE_HEADER
#define MEM_READ_WRITE_HEADER

#define MEMRW_ALLOCATE_STEP	(1024*128)

struct MemRW
{
	MemRW() 
		: wdata(0), rdata(0), byte_size(0), alloc_size(0) 
	{}

	~MemRW() 
	{ 
		release(); 
	}

	void reset() { byte_size = 0; };
	
	unsigned char *get_wdata() { return wdata; }

	void set_wdata(unsigned char* ptr) 
	{ 
		wdata = ptr; 
	};

	void move_wptr_by( int num_bytes )
	{
		byte_size += num_bytes;
	}
	void move_wptr_to( int num_bytes )
	{
		byte_size = num_bytes;
	}
	void set_rdata(const unsigned char* ptr) 
	{ 
		rdata = ptr; 
	};

	bool allocate(int bytes)
	{
		unsigned char *tmp = wdata;

		try
		{
			wdata = new unsigned char[bytes];
			if (tmp) memcpy(wdata, tmp, alloc_size);
			alloc_size = bytes;
		}
		catch (std::bad_alloc)
		{
			wdata = tmp;
			return false;
		}
		return true;
	}

	void release()
	{
		if (wdata) delete [] wdata;
	}
	
	bool checkSize( int additional_bytes )
	{
		if (additional_bytes+byte_size>=alloc_size)
		{
			if (additional_bytes > MEMRW_ALLOCATE_STEP)
				return allocate( alloc_size + additional_bytes );
			else 
				return allocate( alloc_size +MEMRW_ALLOCATE_STEP );
		}
		return true;
	}

	bool write_bytes(const void *data, int size)
	{ 
		if (checkSize(size))
		{
			memcpy(&wdata[byte_size], data, size); 
			byte_size += size; 
			return true;
		}
		return false;
	};

	void read_bytes(void *data, int size, int &pos) const
	{ 
		memcpy(data, &rdata[pos], size); 
		pos += size; 
	};

	template <class T>
	bool write(T s)
	{ 
		if (checkSize(sizeof(T)))
		{
			memcpy(&wdata[byte_size], &s, sizeof(T)); 
			byte_size += sizeof(T); 
			return true;
		}
		return false;
	};

	template <class T>
	void read(T &s, int &pos) const
	{ 
		memcpy(&s, &rdata[pos], sizeof(T)); 
		pos += sizeof(T); 
	};

	bool write_s(const char *s)
	{		
		int size = static_cast<int>(strlen(s) + 1);
		if (checkSize(size+sizeof(int)))
		{
			write(size);
			memcpy(&wdata[byte_size], s, size);
			byte_size += size;
			return true;
		}
		return false;
	};

	void read_s(char *s, int &pos) const
	{
		int size;
		read(size, pos);
		memcpy(s, &rdata[pos], size); pos += size;
	};
	bool write_ws(const wchar_t *s)
	{
		int size = static_cast<int>(wcslen(s) + 1);
			
		if (checkSize(size*2+sizeof(int)))
		{	
			write(size);
			memcpy(&wdata[byte_size], s, size * sizeof(wchar_t));
			byte_size += size * sizeof(wchar_t);
			return true;
		}
		return false;
	};

	void read_ws(wchar_t *s, int &pos) const
	{
		int size;
		read(size, pos);
		memcpy(s, &rdata[pos], size * sizeof(wchar_t)); 
		pos += size * sizeof(wchar_t);
	};
	template <class T>
	bool write_array(T *s, int numItems)
	{ 
		if (checkSize(sizeof(T)*numItems))
		{	
			memcpy(&wdata[byte_size], s, numItems * sizeof(T)); 
			byte_size += sizeof(T)*numItems; 
			return true;
		}
		return false;
	};

	template <class T>
	void read_array(T *s, int numItems, int &pos) const
	{ 
		memcpy(s, &rdata[pos], numItems* sizeof(T)); pos += sizeof(T)*numItems; 
	};

	unsigned char*			wdata;
	const unsigned char*	rdata;
	int						byte_size;
	int						alloc_size;
};

#endif