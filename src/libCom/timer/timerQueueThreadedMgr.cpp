/*
 *      $Id$
 *
 *      Author  Jeffrey O. Hill
 *              johill@lanl.gov
 *              505 665 1831
 *
 *      Experimental Physics and Industrial Control System (EPICS)
 *
 *      Copyright 1991, the Regents of the University of California,
 *      and the University of Chicago Board of Governors.
 *
 *      This software was produced under  U.S. Government contracts:
 *      (W-7405-ENG-36) at the Los Alamos National Laboratory,
 *      and (W-31-109-ENG-38) at Argonne National Laboratory.
 *
 *      Initial development by:
 *              The Controls and Automation Group (AT-8)
 *              Ground Test Accelerator
 *              Accelerator Technology Division
 *              Los Alamos National Laboratory
 *
 *      Co-developed with
 *              The Controls and Computing Group
 *              Accelerator Systems Division
 *              Advanced Photon Source
 *              Argonne National Laboratory
 *
 */

#include <limits.h>

#define epicsExportSharedSymbols
#include "epicsTimerPrivate.h"

timerQueueThreadedMgr::~timerQueueThreadedMgr ()
{
    epicsAutoMutex locker ( this->mutex );
    while ( timerQueueThreaded * pQ = this->queueList.get () ) {
        timerQueueThreadedMgrPrivate *pPriv = pQ;
        delete pPriv;
    }
}
    
timerQueueThreaded & timerQueueThreadedMgr::create (
        bool okToShare, int threadPriority )
{
    epicsAutoMutex locker ( this->mutex );
    tsDLIterBD < timerQueueThreaded > iter = this->queueList.firstIter ();
    while ( iter.valid () ) {
        if ( iter->threadPriority () == threadPriority ) {
            assert ( iter->timerQueueThreadedMgrPrivate::referenceCount < UINT_MAX );
            iter->timerQueueThreadedMgrPrivate::referenceCount++;
            return *iter;
        }
    }
    timerQueueThreaded *pQueue = new timerQueueThreaded ( threadPriority );
    if ( ! pQueue ) {
        throwWithLocation ( timer::noMemory () );
    }
    pQueue->timerQueueThreadedMgrPrivate::referenceCount = 1u;
    this->queueList.add ( *pQueue );
    return *pQueue;
}

void timerQueueThreadedMgr::release ( timerQueueThreaded &queue )
{
    epicsAutoMutex locker ( this->mutex );
    assert ( queue.timerQueueThreadedMgrPrivate::referenceCount > 0u );
    queue.timerQueueThreadedMgrPrivate::referenceCount--;
    if ( queue.timerQueueThreadedMgrPrivate::referenceCount == 0u ) {
        this->queueList.remove ( queue );
        timerQueueThreadedMgrPrivate *pPriv = &queue;
        delete pPriv;
    }
}

timerQueueThreadedMgrPrivate::timerQueueThreadedMgrPrivate () :
    referenceCount ( 0u )
{
}

timerQueueThreadedMgrPrivate::~timerQueueThreadedMgrPrivate () 
{
}