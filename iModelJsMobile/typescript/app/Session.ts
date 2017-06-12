/*--------------------------------------------------------------------------------------+
|
|     $Source: typescript/app/Session.ts $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
import Application from "./Application";
import Host from "./host/Host";
import Activity from "./Activity";
import History from "./History";

/**
 * A period of application usage on a host.
 */
export default class Session {

private SessionTypeId;
private m_application : Application;
private m_host : Host;
private m_history : History;

/**
 * @internal
 */
constructor (application : Application, host : Host)
    {
    this.m_application = application;
    this.m_host = host;
    this.m_history = new History (this);
    }

/**
 * The application associated with this session.
 */
public GetApplication() : Application
    {
    return this.m_application;
    }

/**
 * The host available to this session.
 */
public GetHost() : Host
    {
    return this.m_host;
    }

/**
 * The history of this session.
 */
public GetHistory() : History
    {
    return this.m_history;
    }

/**
 * The activity that is currently active in this session.
 */
public GetCurrentActivity() : Activity
    {
    throw "WIP";
    }

/**
 * Called when this session is started.
 */
protected OnStart() : void
    {
    ;
    }

/**
 * Called when this session is paused.
 */
protected OnPause() : void
    {
    ;
    }

/**
 * Called when this session is resumed.
 */
protected OnResume() : void
    {
    ;
    }

/**
 * Called when this session is finished.
 */
protected OnFinish() : void
    {
    ;
    }

}
