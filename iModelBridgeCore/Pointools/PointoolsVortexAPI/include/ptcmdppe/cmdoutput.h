/*--------------------------------------------------------------------------*/ 
/*	CommandPipe class definition											*/ 
/*  (C) 2003 Copyright Faraz Ravi - All Rights Reserved						*/ 
/*																			*/ 
/*  Last Updated 15 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#ifndef _COMMANDPIPE_OUTPUT
#define _COMMANDPIPE_OUTPUT
#include <string>

namespace ptapp
{
	struct CmdOutput
	{
		virtual void initialize() {};
		virtual void ready() {};
		virtual void prompt(const std::string &txt) {};
		virtual void status(const std::string &txt) {};
		virtual void info(const std::string &txt) {};
		virtual void error(const std::string &txt) {};
		virtual void progress(const std::string &txt) {};

		virtual void progressMax(int mx) {};
		virtual void progressReset() {};
		virtual void progressInc() {};
		virtual void progressInc(int p) {};
		virtual void progressSet(int p) {};

		virtual void flush()=0;

		virtual void addStateIcon(const char *id, int initial_state, int num_states, const char**images) {};
		virtual void remStateIcon(const char *id) {};
		virtual void state(const char*id, int st) {};
	};
}
#endif