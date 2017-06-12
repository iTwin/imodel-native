/*--------------------------------------------------------------------------------------+
|
|     $Source: typescript/app/Activity.ts $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
import Application from "./Application";
import Environment from "./environment/Environment";

/**
 * A collection of application functionality in an environment.
 */
export default class Activity {

private ActivityTypeId;
private m_application : Application;
private m_environment : Environment;

/**
 * @internal
 */
constructor (application : Application, environment : Environment)
    {
    this.m_application = application;
    this.m_environment = environment;
    }

/**
 * The application associated with this activity.
 */
public GetApplication() : Application
    {
    return this.m_application;
    }

/**
 * The environment available to this activity.
 */
public GetEnvironment() : Environment
    {
    return this.m_environment;
    }

/**
 * Called when this activity is started.
 */
protected OnStart() : void
    {
    ;
    }

/**
 * Called when this activity is paused.
 */
protected OnPause() : void
    {
    ;
    }

/**
 * Called when this activity is resumed.
 */
protected OnResume() : void
    {
    ;
    }

/**
 * Called when this activity is finished.
 */
protected OnFinish() : void
    {
    ;
    }

}
