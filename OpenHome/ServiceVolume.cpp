#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/ServiceVolume.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <vector>

using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::Net;
using namespace std;


    //abstract class ServiceVolume : Service
    //{
ServiceVolume::ServiceVolume(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog)
    : Service(aNetwork, aDevice, aLog)
{
    iBalance = new Watchable<TInt>(aNetwork, Brn("Balance"), 0);
    iFade = new Watchable<TInt>(aNetwork, Brn("Fade"), 0);
    iMute = new Watchable<TBool>(aNetwork, Brn("Mute"), false);
    iValue = new Watchable<TUint>(aNetwork, Brn("Value"), 0);
    iVolumeLimit = new Watchable<TUint>(aNetwork, Brn("VolumeLimit"), 0);
    iVolumeMilliDbPerStep = new Watchable<TUint>(aNetwork, Brn("VolumeMilliDbPerStep"), 0);
    iVolumeSteps = new Watchable<TUint>(aNetwork, Brn("VolumeSteps"), 0);
    iVolumeUnity = new Watchable<TUint>(aNetwork, Brn("VolumeUnity"), 0);
}

ServiceVolume::~ServiceVolume()
{
    delete iBalance;
    delete iFade;
    delete iMute;
    delete iValue;
    delete iVolumeLimit;
    delete iVolumeMilliDbPerStep;
    delete iVolumeSteps;
    delete iVolumeUnity;
}

void ServiceVolume::Dispose()
{
    Service::Dispose();
    iBalance->Dispose();
    iFade->Dispose();
    iMute->Dispose();
    iValue->Dispose();
    iVolumeLimit->Dispose();
    iVolumeMilliDbPerStep->Dispose();
    iVolumeSteps->Dispose();
    iVolumeUnity->Dispose();
}

IProxy* ServiceVolume::OnCreate(IDevice& aDevice)
{
    return (new ProxyVolume(*this, aDevice));
}

IWatchable<TInt>& ServiceVolume::Balance()
{
    return *iBalance;
}

IWatchable<TInt>& ServiceVolume::Fade()
{
    return *iFade;
}

IWatchable<TBool>& ServiceVolume::Mute()
{
    return *iMute;
}

IWatchable<TUint>& ServiceVolume::Value()
{
    return *iValue;
}

IWatchable<TUint>& ServiceVolume::VolumeLimit()
{
    return *iVolumeLimit;
}

TUint ServiceVolume::BalanceMax()
{
    return iBalanceMax;
}

TUint ServiceVolume::FadeMax()
{
    return iFadeMax;
}

TUint ServiceVolume::VolumeMax()
{
    return iVolumeMax;
}

IWatchable<TUint>& ServiceVolume::VolumeMilliDbPerStep()
{
    return *iVolumeMilliDbPerStep;
}

IWatchable<TUint>& ServiceVolume::VolumeSteps()
{
    return *iVolumeSteps;
}

IWatchable<TUint>& ServiceVolume::VolumeUnity()
{
    return *iVolumeUnity;
}

////////////////////////////////////////////////////////

ServiceVolumeNetwork::ServiceVolumeNetwork(INetwork& aNetwork, IInjectorDevice& aDevice, CpDevice& aCpDevice, ILog& aLog)
    :ServiceVolume(aNetwork, aDevice, aLog)
    ,iCpDevice(aCpDevice)
{
    iCpDevice.AddRef();

    iService = new CpProxyAvOpenhomeOrgVolume1(aCpDevice);

    Functor f1 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleBalanceChanged);
    iService->SetPropertyBalanceChanged(f1);

    Functor f2 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleFadeChanged);
    iService->SetPropertyFadeChanged(f2);

    Functor f3 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleMuteChanged);
    iService->SetPropertyMuteChanged(f3);

    Functor f4 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleVolumeChanged);
    iService->SetPropertyVolumeChanged(f4);

    Functor f5 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleVolumeLimitChanged);
    iService->SetPropertyVolumeLimitChanged(f5);

    Functor f6 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleVolumeMilliDbPerStepChanged);
    iService->SetPropertyVolumeMilliDbPerStepChanged(f6);

    Functor f7 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleVolumeStepsChanged);
    iService->SetPropertyVolumeStepsChanged(f7);

    Functor f8 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleVolumeUnityChanged);
    iService->SetPropertyVolumeUnityChanged(f8);

    Functor f9 = MakeFunctor(*this, &ServiceVolumeNetwork::HandleInitialEvent);
    iService->SetPropertyInitialEvent(f9);
}


ServiceVolumeNetwork::~ServiceVolumeNetwork()
{
    delete iService;
}


void ServiceVolumeNetwork::Dispose()
{
    ServiceVolume::Dispose();
    //iService->Dispose();
    iCpDevice.RemoveRef();
}


Job* ServiceVolumeNetwork::OnSubscribe()
{
    ASSERT(iSubscribedSource == NULL);
    iSubscribedSource = new JobDone();
    iService->Subscribe();
    return(iSubscribedSource->GetJob());

/*
    ASSERT(iSubscribedSource == NULL);
    iSubscribedSource = new TaskCompletionSource<TBool>();
    iService->Subscribe();
    return iSubscribedSource.Task.ContinueWith((t) => { });
*/
}


void ServiceVolumeNetwork::OnCancelSubscribe()
{
    if (iSubscribedSource != NULL)
    {
        iSubscribedSource->Cancel();
        //iSubscribedSource.TrySetCanceled();
    }
}


void ServiceVolumeNetwork::HandleInitialEvent()
{
    if (!iSubscribedSource->GetJob()->IsCancelled())
    {
        iSubscribedSource->SetResult(true);
    }
}


void ServiceVolumeNetwork::OnUnsubscribe()
{
    if (iService != NULL)
    {
        iService->Unsubscribe();
    }

    iSubscribedSource = NULL;
}


void ServiceVolumeNetwork::SetBalance(TInt aValue)
{
    Job2* job = new Job2(); // FIXME: read from existing pool - don't allocate new jobs
    FunctorAsync f = job->AsyncCb();
    iService->BeginSetBalance(aValue, f);

/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService.BeginSetBalance(aValue, (ptr) =>
    {
        try
        {
            iService.EndSetBalance(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServiceVolumeNetwork::SetFade(TInt aValue)
{
    Job2* job = new Job2(); // FIXME: read from existing pool - don't allocate new jobs
    FunctorAsync f = job->AsyncCb();
    iService->BeginSetFade(aValue, f);

/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService.BeginSetFade(aValue, (ptr) =>
    {
        try
        {
            iService.EndSetFade(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServiceVolumeNetwork::SetMute(TBool aValue)
{
    Job2* job = new Job2(); // FIXME: read from existing pool - don't allocate new jobs
    FunctorAsync f = job->AsyncCb();
    iService->BeginSetMute(aValue, f);
/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService.BeginSetMute(aValue, (ptr) =>
    {
        try
        {
            iService.EndSetMute(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServiceVolumeNetwork::SetVolume(TUint aValue)
{
    Job2* job = new Job2(); // FIXME: read from existing pool - don't allocate new jobs
    FunctorAsync f = job->AsyncCb();
    iService->BeginSetVolume(aValue, f);
/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService.BeginSetVolume(aValue, (ptr) =>
    {
        try
        {
            iService.EndSetVolume(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServiceVolumeNetwork::VolumeDec()
{
    Job2* job = new Job2(); // FIXME: read from existing pool - don't allocate new jobs
    FunctorAsync f = job->AsyncCb();
    iService->BeginVolumeDec(f);
/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService.BeginVolumeDec((ptr) =>
    {
        try
        {
            iService.EndVolumeDec(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServiceVolumeNetwork::VolumeInc()
{
    Job2* job = new Job2(); // FIXME: read from existing pool - don't allocate new jobs
    FunctorAsync f = job->AsyncCb();
    iService->BeginVolumeInc(f);
/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService.BeginVolumeInc((ptr) =>
    {
        try
        {
            iService.EndVolumeInc(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServiceVolumeNetwork::HandleVolumeUnityChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeUnityChangedCallback);
    iNetwork.Schedule(f, NULL);
/*
    TUint unity = iService.PropertyVolumeUnity();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iVolumeUnity.Update(unity);
        });
    });
*/
}


void ServiceVolumeNetwork::VolumeUnityChangedCallback(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeUnityChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::VolumeUnityChangedCallbackCallback(void*)
{
    TUint unity;
    iService->PropertyVolumeUnity(unity);
    iVolumeUnity->Update(unity);
}


void ServiceVolumeNetwork::HandleVolumeStepsChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeStepsChangedCallback);
    iNetwork.Schedule(f, NULL);
/*
    TUint steps = iService.PropertyVolumeSteps();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iVolumeSteps.Update(steps);
        });
    });
*/
}


void ServiceVolumeNetwork::VolumeStepsChangedCallback(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeStepsChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::VolumeStepsChangedCallbackCallback(void*)
{
    TUint steps;
    iService->PropertyVolumeSteps(steps);
    iVolumeSteps->Update(steps);
}



void ServiceVolumeNetwork::HandleVolumeMilliDbPerStepChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeMilliDbPerStepChangedCallback);
    iNetwork.Schedule(f, NULL);
/*
    TUint step = iService.PropertyVolumeMilliDbPerStep();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iVolumeMilliDbPerStep.Update(step);
        });
    });
*/
}


void ServiceVolumeNetwork::VolumeMilliDbPerStepChangedCallback(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeMilliDbPerStepChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::VolumeMilliDbPerStepChangedCallbackCallback(void*)
{
    TUint mdbs;
    iService->PropertyVolumeMilliDbPerStep(mdbs);
    iVolumeMilliDbPerStep->Update(mdbs);
}


void ServiceVolumeNetwork::HandleVolumeLimitChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeLimitChangedCallback);
    iNetwork.Schedule(f, NULL);
/*
    TUint limit = iService.PropertyVolumeLimit();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iVolumeLimit.Update(limit);
        });
    });
*/
}


void ServiceVolumeNetwork::VolumeLimitChangedCallback(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeLimitChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::VolumeLimitChangedCallbackCallback(void*)
{
    TUint limit;
    iService->PropertyVolumeLimit(limit);
    iVolumeLimit->Update(limit);
}

void ServiceVolumeNetwork::HandleVolumeChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeChangedCallback);
    iNetwork.Schedule(f, NULL);
/*
    TUint volume = iService.PropertyVolume();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            //Console.WriteLine("VolumeChanged: " + Device.Udn + " " + iService.PropertyVolume());
            iValue.Update(volume);
        });
    });
*/
}


void ServiceVolumeNetwork::VolumeChangedCallback(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::VolumeChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::VolumeChangedCallbackCallback(void*)
{
    TUint volume;
    iService->PropertyVolume(volume);
    iValue->Update(volume);
}

void ServiceVolumeNetwork::HandleMuteChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::MuteChangedCallback);
    iNetwork.Schedule(f, NULL);
/*
    TBool mute = iService.PropertyMute();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iMute.Update(mute);
        });
    });
*/
}


void ServiceVolumeNetwork::MuteChangedCallback(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::MuteChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::MuteChangedCallbackCallback(void*)
{
    TBool mute;
    iService->PropertyMute(mute);
    iMute->Update(mute);
}

void ServiceVolumeNetwork::HandleFadeChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::FadeChangedCallback);
    iNetwork.Schedule(f, NULL);
/*
    TInt fade = iService.PropertyFade();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iFade.Update(fade);
        });
    });
*/
}


void ServiceVolumeNetwork::FadeChangedCallback(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::FadeChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::FadeChangedCallbackCallback(void*)
{
    TInt fade;
    iService->PropertyFade(fade);
    iFade->Update(fade);
}

void ServiceVolumeNetwork::HandleBalanceChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::BalanceChangedCallback);
    iNetwork.Schedule(f, NULL);
/*
    TInt balance = iService.PropertyBalance();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iBalance.Update(balance);
        });
    });
*/
}


void ServiceVolumeNetwork::BalanceChangedCallback(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceVolumeNetwork::BalanceChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceVolumeNetwork::BalanceChangedCallbackCallback(void*)
{
    TInt balance;
    iService->PropertyBalance(balance);
    iBalance->Update(balance);
}


    /////////////////////////////////////////////////////////////

/*
ServiceVolumeMock(INetwork aNetwork, IInjectorDevice aDevice, string aId, TInt aBalance, TUint aBalanceMax, TInt aFade, TUint aFadeMax, TBool aMute, TUint aValue, TUint aVolumeLimit, TUint aVolumeMax,
    TUint aVolumeMilliDbPerStep, TUint aVolumeSteps, TUint aVolumeUnity, ILog aLog)
    : ServiceVolume(aNetwork, aDevice, aLog)
{
    TUint volumeLimit = aVolumeLimit;
    if (volumeLimit > aVolumeMax)
    {
        volumeLimit = aVolumeMax;
    }
    iCurrentVolumeLimit = volumeLimit;

    TUint value = aValue;
    if (value > aVolumeLimit)
    {
        value = aVolumeLimit;
    }
    iCurrentVolume = value;

    iBalance.Update(aBalance);
    iFade.Update(aFade);
    iMute.Update(aMute);
    iValue.Update(value);
    iVolumeLimit.Update(volumeLimit);
    iVolumeMilliDbPerStep.Update(aVolumeMilliDbPerStep);
    iVolumeSteps.Update(aVolumeSteps);
    iVolumeUnity.Update(aVolumeUnity);
}

Task SetBalance(TInt aValue)
{
    Task task = Task.Factory.StartNew(() =>
    {
        iNetwork.Schedule(() =>
        {
            iBalance.Update(aValue);
        });
    });
    return task;
}

Task SetFade(TInt aValue)
{
    Task task = Task.Factory.StartNew(() =>
    {
        iNetwork.Schedule(() =>
        {
            iFade.Update(aValue);
        });
    });
    return task;
}

Task SetMute(TBool aValue)
{
    Task task = Task.Factory.StartNew(() =>
    {
        iNetwork.Schedule(() =>
        {
            iMute.Update(aValue);
        });
    });
    return task;
}

Task SetVolume(TUint aValue)
{
    Task task = Task.Factory.StartNew(() =>
    {
        TUint value = aValue;
        if (value > iCurrentVolumeLimit)
        {
            value = iCurrentVolumeLimit;
        }

        if (value != iCurrentVolume)
        {
            iCurrentVolume = value;
            iNetwork.Schedule(() =>
            {
                iValue.Update(aValue);
            });
        }
    });
    return task;
}

Task VolumeDec()
{
    Task task = Task.Factory.StartNew(() =>
    {
        if (iCurrentVolume > 0)
        {
            --iCurrentVolume;
            iNetwork.Schedule(() =>
            {
                iValue.Update(iCurrentVolume);
            });
        }
    });
    return task;
}

Task VolumeInc()
{
    Task task = Task.Factory.StartNew(() =>
    {
        if (iCurrentVolume < iCurrentVolumeLimit)
        {
            ++iCurrentVolume;
            iNetwork.Schedule(() =>
            {
                iValue.Update(iCurrentVolume);
            });
        }
    });
    return task;
}

void Execute(IEnumerable<string> aValue)
{
    string command = aValue.First().ToLowerInvariant();
    if (command == "balance")
    {
        IEnumerable<string> value = aValue.Skip(1);
        iBalance.Update(TInt.Parse(value.First()));
    }
    else if (command == "fade")
    {
        IEnumerable<string> value = aValue.Skip(1);
        iFade.Update(TInt.Parse(value.First()));
    }
    else if (command == "mute")
    {
        IEnumerable<string> value = aValue.Skip(1);
        iMute.Update(TBool.Parse(value.First()));
    }
    else if (command == "value")
    {
        IEnumerable<string> value = aValue.Skip(1);
        iValue.Update(TUint.Parse(value.First()));
    }
    else if (command == "volumeinc")
    {
        VolumeInc();
    }
    else if (command == "volumedec")
    {
        VolumeDec();
    }
    else
    {
        throw new NotSupportedException();
    }
}
*/
//////////////////////////////////////////////////////


ProxyVolume::ProxyVolume(ServiceVolume& aService, IDevice& aDevice)
    :iService(aService)
    ,iDevice(aDevice)
{
}

IWatchable<TInt>& ProxyVolume::Balance()
{
    return iService.Balance();
}

IWatchable<TInt>& ProxyVolume::Fade()
{
    return iService.Fade();
}

IWatchable<TBool>& ProxyVolume::Mute()
{
    return iService.Mute();
}

IWatchable<TUint>& ProxyVolume::Value()
{
    return iService.Value();
}

IWatchable<TUint>& ProxyVolume::VolumeLimit()
{
    return iService.VolumeLimit();
}

IWatchable<TUint>& ProxyVolume::VolumeMilliDbPerStep()
{
    return iService.VolumeMilliDbPerStep();
}

IWatchable<TUint>& ProxyVolume::VolumeSteps()
{
    return iService.VolumeSteps();
}

IWatchable<TUint>& ProxyVolume::VolumeUnity()
{
    return iService.VolumeUnity();
}

TUint ProxyVolume::BalanceMax()
{
    return iService.BalanceMax();
}

TUint ProxyVolume::FadeMax()
{
    return iService.FadeMax();
}

TUint ProxyVolume::VolumeMax()
{
    return iService.VolumeMax();
}

void ProxyVolume::SetBalance(TInt aValue)
{
    return iService.SetBalance(aValue);
}

void ProxyVolume::SetFade(TInt aValue)
{
    return iService.SetFade(aValue);
}

void ProxyVolume::SetMute(TBool aValue)
{
    return iService.SetMute(aValue);
}

void ProxyVolume::SetVolume(TUint aValue)
{
    return iService.SetVolume(aValue);
}

void ProxyVolume::VolumeDec()
{
    return iService.VolumeDec();
}

void ProxyVolume::VolumeInc()
{
    return iService.VolumeInc();
}


void ProxyVolume::Dispose()
{
    iService.Unsubscribe();
}

IDevice& ProxyVolume::Device()
{
    return (iDevice);
}