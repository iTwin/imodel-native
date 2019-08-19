/*--------------------------------------------------------------------------*/ 
/*	CommandPipe state definition											*/ 
/*  (C) 2003 Copyright Faraz Ravi - All Rights Reserved						*/ 
/*																			*/ 
/*  Last Updated 1 Oct 2004 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#ifndef _COMMANDPIPE_STATE_ICON
#define _COMMANDPIPE_STATE_ICON

#ifdef CMDPIPE_EXPORTS
#define CMDPIPE_API EXPORT_ATTRIBUTE
#else
	#ifdef POINTOOLS_API_INCLUDE
		#define CMDPIPE_API 
	#else
		#define CMDPIPE_API IMPORT_ATTRIBUTE
	#endif
#endif

namespace ptapp
{
class CMDPIPE_API CmdStateIcon
{
public:
	CmdStateIcon(const char *id, int initial_state, int num_states, const char**imagepaths);
	~CmdStateIcon();

	void state(int st);
private:
	const char *_id;
};

}
#endif
