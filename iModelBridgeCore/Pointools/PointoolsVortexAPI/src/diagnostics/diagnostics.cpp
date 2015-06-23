#include <pt/os.h>
#include <diagnostics/diagnostics.h>

#define BOOST_DATE_TIME_NO_LIB
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/ipc/message_queue.hpp>

using namespace ptdg;
using namespace boost::interprocess;

#define maxsize( A, B ) ((sizeof(A) > sizeof(B)) ? sizeof(A) : sizeof(B))

namespace ptdg {
	Diagnostics *_instance = 0;
}
//-----------------------------------------------------------------------------
Diagnostics::Diagnostics()
{
	m_ptmq = 0;
}
Diagnostics *Diagnostics::instance()
{
	if (!_instance)
	{
		_instance = new Diagnostics;
		_instance->initialise();
	}
	return _instance;
}

//-----------------------------------------------------------------------------
Diagnostics::~Diagnostics()
{
	if (m_ptmq) 
	{
		message_queue::remove("ptvortex");	
		delete m_ptmq;
	}
}
//-----------------------------------------------------------------------------
void Diagnostics::addMetric(ptdg::MetricTypeID id, ubyte *data, int size)
{
	if (!m_ptmq) return;

	int idi = (int)id;

	ubyte *block = new ubyte[ sizeof(int) + size ];
	memcpy( block, &idi, sizeof(int) );
	memcpy( &block[sizeof(int)], data, size );

	// Send data
	m_ptmq->try_send( block, size+sizeof(int), 0 ); 
}
//-----------------------------------------------------------------------------
bool Diagnostics::initialise() 
{
	if (m_ptmq) return true;

	char hasDiag[16]; 
	hasDiag[0] = 0;

	int success = ::GetEnvironmentVariableA("PTVORTEX_DIAGNOSTICS", hasDiag, 16);

	if (success && strcmp(hasDiag, "stream") == 0)
	{
		size_t maxStructSize = maxsize( QueryData1, 
							maxsize( LoadData1, 
							maxsize( PagerData1,
							maxsize( VisibilityData1, RenderData1))));
		maxStructSize *= 2;


		try
		{
			//Erase previous message queue
			message_queue::remove("ptvortex");

			//Create a message_queue. If the queue
			//exists throws an exception
			m_ptmq = new message_queue( open_or_create, "ptvortex", 20, maxStructSize );
		}
		catch(interprocess_exception &ex)
		{
			if (m_ptmq) delete m_ptmq;
			m_ptmq = 0;
			return false;
	}   
	}
	return true;
}
//-----------------------------------------------------------------------------