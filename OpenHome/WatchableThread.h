#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/Fifo.h>
#include <OpenHome/Exception.h>
#include <OpenHome/OhTopologyC.h>

#include <stddef.h>

namespace OpenHome
{
namespace Topology
{

/////////////////////////////////////////////////////////

class IWatchableThread
{
public:
    virtual ~IWatchableThread() {};

    virtual void Assert() = 0;
    virtual void Schedule(FunctorGeneric<void*> aCallback, void* aObj) = 0;
    virtual void Execute(FunctorGeneric<void*> aCallback, void* aObj) = 0;
    virtual void Execute() = 0;
};

///////////////////////////////////////////////

class SignalledCallback
{
public:
    SignalledCallback();
    void Set(FunctorGeneric<void*> aFunctor, void* aObj, Semaphore& aSem);
    void Set(FunctorGeneric<void*> aFunctor, void* aObj);
    void Callback();

private:
    FunctorGeneric<void*> iFunctor;
    void* iObj;
    Semaphore* iSem;
};

/////////////////////////////////////////////

class AutoSig
{
public:
    AutoSig(Semaphore* aSem);
    ~AutoSig();
private:
    Semaphore* iSem;
};

/////////////////////////////////////////////

class WatchableThread : public IWatchableThread
{
public:
    static const TUint kMaxFifoEntries = 100;

public:
    WatchableThread(IExceptionReporter& aReporter);
    virtual ~WatchableThread();
    virtual void Assert();
    virtual void Schedule(FunctorGeneric<void*> aCallback, void* aObj);
    virtual void Execute(FunctorGeneric<void*> aCallback, void* aObj);
    virtual void Execute();

private:
    void Run();
    void Shutdown(void*);
    TBool IsWatchableThread();
    void DoNothing(void*);

private:
    IExceptionReporter& iExceptionReporter;
    Fifo<SignalledCallback*> iFree;
    Fifo<SignalledCallback*> iScheduled;
    ThreadFunctor* iThread;
};

//////////////////////////////////////////////

} // namsespace Av
} // namespace OpenHome
