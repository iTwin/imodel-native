/*--------------------------------------------------------------------------*/ 
/*	CommandPipe class definition											*/ 
/*  (C) 2003 Copyright Faraz Ravi - All Rights Reserved						*/ 
/*																			*/ 
/*  Last Updated 15 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#ifndef _COMMANDPIPE_INTERFACE
#define _COMMANDPIPE_INTERFACE

#ifdef CMDPIPE_EXPORTS
#define CMDPIPE_API __declspec(dllexport)
#else
#define CMDPIPE_API __declspec(dllimport)
#endif

#pragma warning (disable : 4251)
#pragma warning (disable : 4275)

#include <Loki/AssocVector.h>
#include <map>
#include <vector>

#include <ptcmdppe/interact.h>
#include <pt/typedefs.h>

#include <ptcmdppe/eventdefs.h>
#include <ptmodm/module.h>
#include <pt/parametermap.h>

#define CP_CMD_UNKNOWN		0
#define CP_CMD_FOUND		1
#define CP_CMD_BADCONTEXT	2
#define CP_CMD_PROMPT		3

namespace ptapp
{
	typedef std::vector<std::string> script;

	/*--------------------------*/ 
	/*Output struct for overload*/ 
	/*--------------------------*/ 
	struct CMDPIPE_API CmdOutput;

	/*--------------------------*/ 
	/*    Command class			*/ 
	/*--------------------------*/ 
	struct CMDPIPE_API Command
	{
		std::string string;
		
		std::string	environment;
		std::string	loadscript;

		uint		eventmask;
		uint		datatypes;
		uint		flags;
		uint		cursor;
		
		const char *prompt;

		/*callbacks*/ 
		eventCB		notify;
		functionCB	call;
	};
	/*--------------------------*/ 
	/*Environment overload this */ 
	/*--------------------------*/ 
	struct CMDPIPE_API CmdEnvironment
	{
		envtestCB complete;
		uint context;
		std::string buildscript;
	};
	/*--------------------------*/ 
	/*Command Scripts			*/ 
	/*--------------------------*/ 
	struct CMDPIPE_API CmdScript
	{
		std::vector<Command*> script;
		std::vector<std::string> prompts;
		bool	quit_on_cmdfail;
		bool	compiled;
		
		/*handles script event drop through*/ 
		eventCB	handler;

		/*position */ 
		int		position;

		/*flags*/ 
		uint	flags;

		/*if compile fails - store for recompile*/ 
		std::string _script;
		std::string _prompts;
	};
	/*--------------------------*/ 
	/*    commandpipe class		*/ 
	/*--------------------------*/ 
	class CMDPIPE_API CommandPipe : public pt::Module<CommandPipe, 1100, 1000>
	{
	public:
		CommandPipe();
		~CommandPipe();
		
		/*setup*/ 
		void setQualifiers(const std::vector<std::string> &qualifiers);
		void setObjectTypes(const std::vector<std::string> &objtypes);
		void setOutputObject(CmdOutput *outobj);
		void setBaseCommand(const std::string &bcmd);

		void addEnvironment(const std::string &name, const envtestCB &cb, uint context, const std::string &script);
		void addScript(const std::string &name, const std::string &cmdlist, const std::string &prompts, eventCB handler = 0, uint flags = 0);

		void addEventCB(const eventCB &cb);
		void addResetCB(const resetCB &cb);
		
		void remEventCB(const eventCB &cb);
		void remResetCB(const resetCB &cb);

		void addEventFunc(const std::string &cmdstring, const eventCB &ecb, const std::string &environment="", uint eventmask=CP_MOUSE_L_DOWN, const char* prompt = 0, uint flags = 0, uint cursor = 0);
		void addFunc(const std::string &cmdstring, const functionCB &fcb, const std::string &environment="", const char* prompt = 0, uint flags=0);

		void addFuncAlias(const std::string &cmdstring, const std::string &aliastring);


		/*invoke*/ 
		uint call(const std::string &str, pt::ParameterList *args = 0);
		uint push(const std::string &str, pt::ParameterList *args = 0);
		uint set(const std::string &str, pt::ParameterList *args = 0);
		void pop();

		void callLast();

		pt::ParameterList *getCallParameters()	{ return m_params; }
		
		/*notify*/ 
		bool event(const EventInfo &e);
		
		/*feedback*/ 
		void pushResult(const pt::CmdResult &result);
		bool popResult(pt::CmdResult &result);
		bool peepResult(pt::CmdResult &result);
		void clearResults();

		/*state*/ 
		void defaultMode();
		bool inEditMode();
		bool enter();

		/*command query*/ 
		inline const std::string &currentCommand() const { return m_command ? m_command->string : m_nocmd; }
		uint commandFlags() const { return m_script ? m_script->flags : (m_command ? m_command->flags : 0); }

		void initialize();
		void destroy();

		/*query*/ 
		void resetIterator();
		uint getNextCommand(char *cmd);

		void setQualifier(uint q)	{ m_qualifier = q; }
		void setContextMask(uint c)	{ m_context = c; }
		
		uint cursor() const		{ return m_cursor ? m_cursor : (m_command ? m_command->cursor : 0); }
		void overideCursor(uint handle) { m_cursor = handle; };

	private:

		Command* getParentCommand();
		Command* findCmd(const std::string &str);
		CmdScript* findScript(const std::string &str);
		CmdEnvironment* findEnvironment(const std::string &str);

		void output(uint err, Command*t);

		void selectObjects();
		void selectSubject();

		void emptyCmdStack();

		void loadCommand();
		void reloadCommand();
		void reset();
		void popCmd();
		void pushCmd(Command *cmd);

		bool runScript(CmdScript *script);

		void buildQualifiedCmdString(std::string &cmd);
		
		/*data*/ 

		typedef std::map<std::string, Command*> CMDMAP;
		typedef std::map<std::string, CmdEnvironment*> ENVMAP;
		typedef std::map<std::string, CmdScript*> SCRIPTMAP;
		typedef std::map<std::string, std::string>  ALIASMAP;

		/*scripting*/ 
		int compileScript(const std::string &txt, const std::string &prompts, CmdScript &script);

		CmdOutput					*m_output;
		
		/*containers*/ 
		CMDMAP						m_cmdmap;
		ALIASMAP					m_aliasmap;
		ENVMAP						m_envmap;
		SCRIPTMAP					m_scriptmap;

		std::stack<Command *>		m_cmdstack;
		std::stack<pt::CmdResult>	m_resultstack;
		std::vector<eventCB>		m_event_cbs;
		std::vector<resetCB>		m_reset_cbs;

		pt::ParameterList			*m_params;

 		Command			*m_command;
		Command			*m_lastcmd;
		std::string		m_basecmd;
		std::string		m_nocmd;
		std::string		m_qualifier;
		uint			m_context;

		uint		m_cursor;
		bool		m_defaultmode;

		CmdScript	*m_script;
	};
};
#endif 
