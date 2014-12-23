/*--------------------------------------------------------------------------*/ 
/*	CommandPipe progress definition											*/ 
/*  (C) 2003 Copyright Faraz Ravi - All Rights Reserved						*/ 
/*																			*/ 
/*  Last Updated 15 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#ifndef _COMMANDPIPE_PROGRESS
#define _COMMANDPIPE_PROGRESS

#ifdef CMDPIPE_EXPORTS
#define CMDPIPE_API __declspec(dllexport)
#else
	#ifdef POINTOOLS_API_INCLUDE
		#define CMDPIPE_API 
	#else
		#define CMDPIPE_API __declspec(dllimport)
	#endif
#endif

namespace ptapp
{
class CMDPIPE_API CmdProgress
{
public:
	CmdProgress(const char *st, int mn, int mx, bool dedicated_win = false);
	~CmdProgress();

	void inc();
	void inc(int amount);
	void set(int amount);
	int get() const;
	void status(const char* d);
};

}
#endif
