/*--------------------------------------------------------------------------*/ 
/*	CommandPipe class implementation										*/ 
/*  (C) 2003 Copyright Pointools Ltd, UK - All Rights Reserved				*/ 
/*																			*/ 
/*  Last Updated 14 Oct 2003 Faraz Ravi										*/ 
/*--------------------------------------------------------------------------*/ 
#ifdef _WIN32
#pragma warning (disable : 4786)
#pragma warning (disable : 4251)
#endif

#include "PointoolsVortexAPIInternal.h"


#include <ptcmdppe/CommandPipe.h>
#include <ptcmdppe/CmdOutput.h>
#include <pt/trace.h>
#include <fltk/cursor.h>
#include <stack>
#include <vector>


#define SUBOP_NONE		0
#define SUBOP_ACTOR		1
#define SUBOP_SUBJECT	2

/*#define __DEBUG_CMDOUT*/ 
using namespace ptapp;
using namespace pt;

namespace cmdppe_private
{
extern CmdOutput *_output;
}

/*--------------------------------------------------------------------------*/ 
/* Constructor																*/ 
/*--------------------------------------------------------------------------*/ 
CommandPipe::CommandPipe()
{
	PTTRACE("CommandPipe::CommandPipe");
	m_nocmd = "<none>";
	m_output = 0;
	m_command = 0; 
	m_basecmd = "_edit#";
	m_qualifier = "";
	m_lastcmd = 0;
	m_defaultmode = true;
	m_cursor = 0;
	m_script = 0;
	m_params = 0;
}
/*--------------------------------------------------------------------------*/ 
/* Destructor																*/ 
/*--------------------------------------------------------------------------*/ 
CommandPipe::~CommandPipe()
{
	PTTRACE("CommandPipe::~CommandPipe");
	destroy();
}
/*--------------------------------------------------------------------------*/ 
/* initialize																*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::initialize() {}
/*--------------------------------------------------------------------------*/ 
/* output object															*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::setOutputObject(CmdOutput *outobj)
{
	m_output = outobj; 
	m_output->initialize();
	cmdppe_private::_output = outobj;
}
/*--------------------------------------------------------------------------*/ 
/* base command																*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::setBaseCommand(const std::string &bcmd)
{
	m_basecmd = bcmd;
}
/*--------------------------------------------------------------------------*/ 
/* event cb that is called for any event									*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::addEventCB(const eventCB &cb)
{
	m_event_cbs.push_back(cb);
}
/*--------------------------------------------------------------------------*/ 
/* event cb that is called for any event									*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::remEventCB(const eventCB &cb)
{
	std::vector<eventCB>::iterator i = m_event_cbs.begin();

	while (i != m_event_cbs.end())
	{
		/* this is not a great way to do it */ 
		if ((*i).functor.func_ptr == cb.functor.func_ptr)
		{
			m_event_cbs.erase(i);
			return;
		}
		++i;
	};
}
/*--------------------------------------------------------------------------*/ 
/* reset cb that is called when command status is reset						*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::addResetCB(const resetCB &cb)
{
	m_reset_cbs.push_back(cb);
}
/*--------------------------------------------------------------------------*/ 
/* event cb that is called for any event									*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::remResetCB(const resetCB &cb)
{
	std::vector<resetCB>::iterator i = m_reset_cbs.begin();

	while (i != m_reset_cbs.end())
	{
		/* this is not a great way to do it */ 
		if ((*i).functor.func_ptr == cb.functor.func_ptr)
		{
			m_reset_cbs.erase(i);
			return;
		}
		++i;
	};
}
/*--------------------------------------------------------------------------*/ 
/* Destroy maps																*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::destroy()
{
	CMDMAP::iterator itc = m_cmdmap.begin();

	/*destroy command map*/ 
	while (itc != m_cmdmap.end())
	{
		delete itc->second;
		itc++;
	}
	/*destroy scripts*/ 
	SCRIPTMAP::iterator iti = m_scriptmap.begin();

	while (iti != m_scriptmap.end())
	{
		delete iti->second;
		iti++;
	}
	/*destroy scripts*/ 
	SCRIPTMAP::iterator its = m_scriptmap.begin();

	while (its != m_scriptmap.end())
	{
		delete its->second;
		its++;
	}
	/*destroy environments*/ 
	ENVMAP::iterator ite = m_envmap.begin();

	while (ite != m_envmap.end())
	{
		delete ite->second;
		ite++;
	}
	m_scriptmap.clear();
	m_cmdmap.clear();
	m_aliasmap.clear();
	m_envmap.clear();
}
/*--------------------------------------------------------------------------*/ 
/* find command																*/ 
/*--------------------------------------------------------------------------*/ 
Command* CommandPipe::findCmd(const std::string &str)
{
	Command *c = 0;
	std::string cmd;

	ALIASMAP::iterator it = m_aliasmap.find(str);

	if (it != m_aliasmap.end()) cmd = it->second;
	else cmd = str;

	CMDMAP::iterator itc = m_cmdmap.find(cmd);
	if (itc != m_cmdmap.end())
	{
		c = itc->second;
	}
	return c;
}
/*--------------------------------------------------------------------------*/ 
/* find Script in Script map												*/ 
/*--------------------------------------------------------------------------*/ 
CmdScript *CommandPipe::findScript(const std::string &str)
{
	CmdScript *c = 0;

	SCRIPTMAP::iterator it = m_scriptmap.find(str);
	if (it != m_scriptmap.end())
	{
		c = it->second;
	}
	return c;
}
/*--------------------------------------------------------------------------*/ 
/* find Script in Environment map											*/ 
/*--------------------------------------------------------------------------*/ 
CmdEnvironment *CommandPipe::findEnvironment(const std::string &str)
{
	CmdEnvironment *e = 0;

	ENVMAP::iterator it = m_envmap.find(str);
	if (it != m_envmap.end())
	{
		e = it->second;
	}
	return e;
}
/*--------------------------------------------------------------------------*/ 
/* push result stack														*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::pushResult(const CmdResult &result)
{
	m_resultstack.push(result);
}
/*--------------------------------------------------------------------------*/ 
/* pop result stack															*/ 
/*--------------------------------------------------------------------------*/ 
bool CommandPipe::popResult(CmdResult &result)
{
	if (m_resultstack.size())
	{
		result = m_resultstack.top();
		m_resultstack.pop();

		return true;
	}
	return false;
}
/*--------------------------------------------------------------------------*/ 
/* peep result stack														*/ 
/*--------------------------------------------------------------------------*/ 
bool CommandPipe::peepResult(CmdResult &result)
{
	if (m_resultstack.size())
	{
		result = m_resultstack.top();

		return true;
	}
	return false;
}
/*--------------------------------------------------------------------------*/ 
/* clear result stack														*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::clearResults()
{
	while (m_resultstack.size()) m_resultstack.pop();
}
/*--------------------------------------------------------------------------*/ 
/* std output																*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::output(uint err, Command *t)
{
	char mess[256];
	if (m_output)
	{
		switch(err)
		{
		case CP_CMD_PROMPT: if (t->prompt) m_output->prompt(t->prompt);	break;
		case CP_CMD_FOUND:	m_output->info(t->string);	break;
		case CP_CMD_UNKNOWN:
			if (t)
			{
				sprintf(mess, "Error: %s is not a recognised command", t->string.c_str());
				m_output->error(mess);
			}
			else
			{
				m_output->error("Error: Command not recognised");
			}
			break;
		case CP_CMD_BADCONTEXT:
			if (t)
			{
				sprintf(mess, "Error: Invalid input context whilst processing data for %s", t->string.c_str());
				m_output->error(mess);
			}
			else
			{
				m_output->error("Error: Invalid input context");
			}
			break;
		}
	}
}
/*--------------------------------------------------------------------------*/ 
/* call a command directly, no stack involved								*/ 
/*--------------------------------------------------------------------------*/ 
uint CommandPipe::call(const std::string &_str, ParameterList *args)
{
	std::string str(_str);
	buildQualifiedCmdString(str);

	uint retval = CP_CMD_FOUND;
	Command *c = findCmd(str);
	
	/*hack*/ 
	HCURSOR cur = ::GetCursor();
	if (str.at(0) != '_')
		::SetCursor(LoadCursor(0,IDC_WAIT));

	if (c && c->prompt) m_output->prompt(c->prompt);

	if (!c) retval = CP_CMD_UNKNOWN;
	else if (c->call)
	{
		/*clear results*/ 
		m_params = args;
		c->call();
		m_params = 0;
	}
	else
		output(CP_CMD_UNKNOWN, c);

	if (c && c->prompt) m_output->ready();

	if (str.at(0) != '_')
		::SetCursor(cur);

	//static int init = 0;

	//if (init < 20)
	//{
	//	/* produce list of registered commands */ 
	//	std::ofstream ofs;
	//	ofs.open("d:\\commands.txt");
	//	CMDMAP::iterator it = m_cmdmap.begin();
	//	while (it != m_cmdmap.end())
	//	{
	//		if (it->first.c_str()[0] != '_')
	//			ofs << (it->first.c_str()) << std::endl;
	//		++it;
	//	}
	//	ofs.close();
	//	
	//	init = true;
	//}

	return retval;
}
/*--------------------------------------------------------------------------*/ 
/* set base command (mode) 													*/ 
/*--------------------------------------------------------------------------*/ 
uint CommandPipe::set(const std::string &_str, ParameterList *args)
{
	std::string str(_str);	
	buildQualifiedCmdString(str);	
	Command *c = findCmd(str);
	if (c && c->flags & CPF_CALL_ONLY) return call(_str);

	reset();
	return push(_str);
}
/*--------------------------------------------------------------------------*/ 
/* push a command onto command stack										*/ 
/*--------------------------------------------------------------------------*/ 
uint CommandPipe::push(const std::string &_str, ParameterList *args)
{
	std::string str(_str);	
	buildQualifiedCmdString(str);	
	
	uint retval = CP_CMD_UNKNOWN;
	Command *c = findCmd(str);
	CmdScript*s = findScript(str);

	m_cursor = 0;

	if (c)	
	{
		if (c->flags & CPF_CALL_ONLY && !m_script) return call(_str);

		pushCmd(c);
		m_params = args;
		return true;
	}
	else if (s)
	{
		runScript(s);
		return true;
	}
	
	output(CP_CMD_UNKNOWN, c);
	return false;
}
/*--------------------------------------------------------------------------*/ 
/* push command on command stack - ALWAYS USE THIS METHOD NOT ON STACK		*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::pushCmd(Command *c)
{
	if (!(c->flags & CPF_NO_REPEAT)) m_lastcmd = c;
	
	/*initialization*/ 
	m_cmdstack.push(c);
	loadCommand();
}
/*--------------------------------------------------------------------------*/ 
/* pop command off command stack - ALWAYS USE THIS METHOD - NOT stack.pop   */ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::popCmd()
{
	if (!m_cmdstack.size()) return;
	Command *t = m_cmdstack.top();

	/*this order is crucial											*/ 
	/*since any data entry will need parent/child pointers in place */ 
	if (t->eventmask & CP_TERMINATE)
	{
		EventInfo e;
		e.event = CP_TERMINATE;
		t->notify(e, 0);
	}
	if (m_script && m_script->handler)
	{
		EventInfo e;
		e.event = CP_TERMINATE;
		m_script->handler(e, 0);
	}
	m_cursor = 0;
	m_cmdstack.pop();
	/*repeat commands*/ 
	if (t->flags & CPF_LOOP_CMD)	pushCmd(t);
}
/*--------------------------------------------------------------------------*/ 
/* pop command off stack(s)													*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::pop()
{
	if (m_cmdstack.size() > 1)
	{
		Command *t = m_cmdstack.top();
		popCmd();
		/*when script finishes reload command underneath - doesn't initialize*/ 
		if (m_script && !m_script->position--)
		{
			if (m_script->flags & CPF_LOOP_CMD)
				runScript(m_script);
			
			else
			{
				m_script = 0;
				reloadCommand();
			}
		}
		/*scripts inserted in reverse order so do full load*/ 
		else				
		{
			loadCommand();
		}

#ifdef __DEBUG_CMDOUT
		std::cout << "[" << m_cmdstack.size() << "] - pop" << std::endl;
#endif
	}
	else
	{
		if (m_cmdstack.size() == 1)
		{
			/*check if this is last command in script*/ 
			if (m_script)
			{
				/*re-run script if it wants to loop*/ 
				if (m_script->flags & CPF_LOOP_CMD)
				{
					emptyCmdStack();
					runScript(m_script);
					return;
				}
				else m_script = 0;
			}
			popCmd();
#ifdef __DEBUG_CMDOUT
			std::cout << "[" << m_cmdstack.size() << "] - pop" << std::endl;
#endif
		}
		defaultMode();
	}
}
/*--------------------------------------------------------------------------*/ 
/* nofitys of event - returns isHandled										*/ 
/*--------------------------------------------------------------------------*/ 
bool CommandPipe::event(const EventInfo &e)
{
	/*mousepos is received in GDI so convert*/ 	
	bool retval = false;

	if (m_command)
	{
		if (e.event == CP_MOUSE_R_DOWN)
		{
			enter();
			retval = true;
		}
		else if (m_command && m_command->notify && (m_command->eventmask & e.event))
		{
			m_command->notify(e, 0);
			retval = true;
		}
		/*pass commands to script handler if any*/ 
		if (m_script && m_script->handler)
		{
			EventInfo e1; memcpy(&e1, &e, sizeof(EventInfo));
			e1.script_pos = m_script->script.size() - m_script->position;
			m_script->handler(e1, 0);
			retval = true;
		}
		/*check if underlaying command still wants events*/ 
		Command *p = getParentCommand();

		if (p && p!= m_command)
		{
			if (p->flags & CPF_RECIEVE_CHILD_EVENTS
				&& p->eventmask & e.event && p->notify)
			{
				p->notify(e, 0);
				retval = true;
			}
		}
	}
	/*do unqualified registered event callbacks*/ 
	for (uint i=0; i<m_event_cbs.size(); i++) m_event_cbs[i](e, 0);

	return retval;
}
/*--------------------------------------------------------------------------*/ 
/* select Mode - default cmd for main stack									*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::defaultMode()
{
	reset();

	std::string cmd = m_basecmd;
	buildQualifiedCmdString(cmd);

	Command*c = findCmd(cmd);
	if (c)
	{
		pushCmd(c);
		m_defaultmode = true;
	}
	else if (m_output) m_output->ready();

#ifdef __DEBUG_CMDOUT
	std::cout << "[" << m_cmdstack.size() << "] + push: default mode" << std::endl;
#endif
}
/*--------------------------------------------------------------------------*/ 
/* build command mode string - replaces # for _mode in command string		*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::buildQualifiedCmdString(std::string &cmd)
{
	std::string temp;
	temp = cmd;

 	const char mchar = '#';
	int pos = cmd.find_first_of(mchar);
	
	if (pos > 1)
	{
		cmd = temp.substr(0, pos) + "_" + m_qualifier;
	}
}
/*--------------------------------------------------------------------------*/ 
/* reload COmmand at top of stack											*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::reloadCommand()
{
	m_defaultmode = false;
	m_command = m_cmdstack.top();
}
/*--------------------------------------------------------------------------*/ 
/* load COmmand at top of stack												*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::loadCommand()
{
	m_defaultmode = false;
	bool pp = false;

	if (m_cmdstack.size())
	{
		m_command = m_cmdstack.top();

		if (!m_script)
		{
			/*check environment is valid*/ 
			CmdEnvironment *env = findEnvironment(m_command->environment);

			if (env && env->complete && !env->complete(0,0))
			{
				CmdScript *script = findScript(env->buildscript);
				if (script && !runScript(script)) return;
			}
		}
		/*if not running a script*/ 
		if (m_script && m_script->position && m_script->prompts.size() >= m_script->position
			&& m_script->prompts[m_script->position-1] != "-")
		{
			m_output->prompt(m_script->prompts[m_script->position-1]);
		}
		else output(CP_CMD_PROMPT, m_command);

		/*script advance event*/ 
		if (m_script && m_script->handler)
		{
			EventInfo e;
			e.script_pos = m_script->script.size() - m_script->position;
			e.event = CP_SCRIPT_ADVANCE;
			m_script->handler(e, 0);
		}
	}
	if (!m_command) return;

	/*call initializer callback*/ 
	if (m_command->call)		m_command->call();

	/*if this is a call only - pop after call*/ 
	/*command may have been popped off already*/ 
	if (!m_command) return;
	if (m_command->flags & CPF_CALL_ONLY) pop();

	else if (m_command->notify && m_command->eventmask & CP_INITIALIZE)
	{
		EventInfo e;
		e.event = CP_INITIALIZE;
		m_command->notify(e, 0);
	}
}
/*--------------------------------------------------------------------------*/ 
/* enter - commit command - return indicates command is dropped				*/ 
/*--------------------------------------------------------------------------*/ 
bool CommandPipe::enter()
{
	bool retval = false;

	/*check mode*/ 
	if (m_defaultmode)
	{
		/*default mode*/ 
		/*repeat last command*/ 
		defaultMode();
		callLast();
	}
	else
	{
		/*command mode*/ 
		if (m_command && !(m_command->flags & CPF_EXPECT_USER_DROP))
		{
			/*abort command/script*/ 
			reset();
			retval = true;
		}
		pop();
	}
	return retval;
}
/*--------------------------------------------------------------------------*/ 
/* Call Last																*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::callLast()
{
	if (m_lastcmd)
	{
		reset();
		pushCmd(m_lastcmd);

#ifdef __DEBUG_CMDOUT
		std::cout << "last cmd [" << m_cmdstack.size() << "] + push: ";
#endif
	}
}
/*--------------------------------------------------------------------------*/ 
// empty stacks
/*--------------------------------------------------------------------------*/ 
void CommandPipe::emptyCmdStack()
{
	while (m_cmdstack.size()) popCmd();
}
/*--------------------------------------------------------------------------*/ 
/* CommandMap management													*/ 
/*--------------------------------------------------------------------------*/ 
/* adds command																*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::addEventFunc(const std::string &cmdstring, const eventCB &ecb, const std::string &environment, uint eventmask, const char* prompt, uint flags, uint cursor)
{
	Command *cmd = new Command;
	cmd->notify = ecb;
	cmd->eventmask = eventmask;
	cmd->environment = environment;
	cmd->flags = flags;
	cmd->string = cmdstring;
	cmd->cursor = cursor;
	cmd->prompt = prompt;

	m_cmdmap.insert(CMDMAP::value_type(cmdstring, cmd));
}
/*--------------------------------------------------------------------------*/ 
/* adds command																*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::addFunc(const std::string &cmdstring, const functionCB &fcb, const std::string &environment, const char* prompt, uint flags)
{
	Command *cmd = new Command;
	cmd->call = fcb;
	cmd->eventmask = 0;
	cmd->environment = environment;
	cmd->flags = flags;
	cmd->string = cmdstring;
	cmd->cursor = 0;
	cmd->prompt = prompt;

	m_cmdmap.insert(CMDMAP::value_type(cmdstring, cmd));
}
/*--------------------------------------------------------------------------*/ 
/* adds command																*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::addFuncAlias(const std::string &cmdstring, const std::string &aliasstring)
{
	m_aliasmap.insert(ALIASMAP::value_type(aliasstring, cmdstring));
}
/*--------------------------------------------------------------------------*/ 
/* adds script																*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::addScript(const std::string &name, const std::string &cmdlist, 
							const std::string &prompts, eventCB handler, uint flags)
{
	CmdScript *script = new CmdScript;
	script->handler = handler;
	script->compiled = false;
	script->flags = flags;

	compileScript(cmdlist, prompts, *script);
	m_scriptmap.insert(SCRIPTMAP::value_type(name, script));
}
/*--------------------------------------------------------------------------*/ 
/* get Parent Command														*/ 
/*--------------------------------------------------------------------------*/ 
Command *CommandPipe::getParentCommand()
{
	/*get item below this command*/ 
	Command *t = 0;
	Command *t2 = 0;

	if (m_cmdstack.size() > 1)
	{
		t2 = m_cmdstack.top();
		m_cmdstack.pop();
		t = m_cmdstack.top();
		m_cmdstack.push(t2);
	}
	return t;
}
/*--------------------------------------------------------------------------*/ 
/* Reset command status														*/ 
/*--------------------------------------------------------------------------*/ 
void CommandPipe::reset()
{
	emptyCmdStack();
	m_cursor = 0;
	m_script = 0;
	m_command = 0;

	for(int i=0; i<m_reset_cbs.size(); i++)
		m_reset_cbs[i]();
}
/*--------------------------------------------------------------------------*/ 
/*Command Script build from tokens											*/ 
/*--------------------------------------------------------------------------*/ 
int CommandPipe::compileScript(const std::string &txt, const std::string &prompts, CmdScript &script)
{
	if (script.compiled) return script.script.size();

	/*find tokens*/ 	
	std::string cmdstr;
	uint i;
	for (i=0; i<txt.size(); i++)
	{
		char d = txt.at(i);
		if (d == ' ' || d == ',' || d == ';' || d == '\t')
		{
			if (cmdstr.size())
			{
				Command* cmd = findCmd(cmdstr);
				if (cmd) script.script.push_back(cmd);
				else
				{
					/*nested script*/ 
					CmdScript *sc = findScript(cmdstr);
					if (sc && compileScript(sc->_script, sc->_prompts, *sc))
					{		
						/*add these commands in here*/ 
						for (uint s = 0; s < sc->script.size(); s++)
							script.script.push_back(sc->script[s]);
					}
					else
					{
						PTTRACEOUT << "Script compile error in script " << cmdstr.c_str() << ":";
						PTTRACEOUT << "Command at char " << d << " not recognised.";
						PTTRACEOUT << "Compilation will be delayed until invocation (this is usually ok)";

						script.compiled = false;
						script._script = txt;
						script._prompts = prompts;
						script.script.clear();
						return 0;
					}
				}
			}
			cmdstr = "";
		}
		else cmdstr += txt.at(i);
	}
	//break out prompt strings
	std::string prompt;
	for (i=0; i<prompts.size(); i++)
	{
		char d = prompts.at(i);
		if (d == ';' || d == '\t')
		{
			if (prompt.size())
			{
				script.prompts.push_back(prompt);
			}
			else script.prompts.push_back("-");

			prompt = "";
		}
		else prompt += prompts.at(i);
	}

	script.compiled = true;
	script._script = "";
	return script.script.size();
}
/*--------------------------------------------------------------------------*/ 
/* Run compiled script														*/ 
/*--------------------------------------------------------------------------*/ 
bool CommandPipe::runScript(CmdScript *script)
{
	/*check script has been compiled*/ 
	if (script->compiled || 
		(!script->compiled && compileScript(script->_script, script->_prompts, *script)))
	{
		int size = script->script.size();

		for (int i=size-1; i>=0; i--)
		{
			m_cmdstack.push(script->script[i]);
		}
		m_script = script;
		script->position = size;

		/*load command on top*/ 
		loadCommand();

		return true;
	}
	return false;
}
/*--------------------------------------------------------------------------*/ 
/* Command ARgs																*/ 
/*--------------------------------------------------------------------------*/ 
bool CmdArgs::popArg(CmdResult &res, ResultType t) 
{
	if (_args.size() && _args.top().type == t)
	{
		res = _args.top();
		_args.pop();
		return true;
	}
	return false;
}