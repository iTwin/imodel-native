#ifndef POINTOOLS_FLAGS_HELPER_H
#define POINTOOLS_FLAGS_HELPER_H

namespace pt
{
class Flags
{
public:
	Flags(unsigned int val=0) : _flags(val) {}
	~Flags(){};

	inline void set(unsigned int f) { _flags |= f; }
	inline void clear(unsigned int f) { _flags &= ~f; }
	inline bool flag(unsigned int f) const { return _flags & f ? true : false; }

	inline unsigned int getFlags() const { return _flags; }
	void setFlags(const unsigned int &f) { _flags = f; }

	inline void clearAll()	{ _flags = 0; }
	inline void setAll()	{ _flags = 0xffffffff; }

	inline void operator = (const Flags &f) { _flags = f._flags; };

	inline void operator |= (const unsigned int &f) { _flags |= f; }
	inline void operator &= (const unsigned int &f) { _flags &= f; }

	inline unsigned int operator | (const unsigned int &f) { return _flags | f; }
	inline unsigned int operator & (const unsigned int &f) { return _flags & f; }

	inline unsigned int operator | (const Flags &f) { return _flags | f._flags; }
	inline unsigned int operator & (const Flags &f) { return _flags & f._flags; }

private:
	unsigned int	_flags;
};
}
#endif
