/*--------------------------------------------------------------------------------------+
|
|     $Source: typescript/app/History.ts $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
import Session from "./Session";

/**
 * A chronological sequence of activity instances in a session.
 */
export default class History {

private HistoryTypeId;
private m_session : Session;

/**
 * @internal
 */
constructor (session : Session)
    {
    this.m_session = session;
    }

/**
 * The session associated with this history.
 */
public GetSession() : Session
    {
    return this.m_session;
    }

}
