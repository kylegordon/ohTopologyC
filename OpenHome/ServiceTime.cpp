#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/ServiceTime.h>
#include <OpenHome/Network.h>
//#include <OpenHome/AsyncAdaptor.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Net/Private/XmlParser.h>
#include <Generated/CpAvOpenhomeOrgTime1.h>
#include <vector>

using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::Net;
using namespace std;



ServiceTime::ServiceTime(IInjectorDevice& aDevice, ILog& aLog)
    :Service(aDevice, aLog)
    ,iDuration(new Watchable<TUint>(iNetwork, Brn("Duration"), 0))
    ,iSeconds(new Watchable<TUint>(iNetwork, Brn("Seconds"), 0))
{
}


void ServiceTime::Dispose()
{
    ServiceTime::Dispose();

    iDuration->Dispose();
    //iDuration = NULL;

    iSeconds->Dispose();
    //iSeconds = null;
}


IProxy* ServiceTime::OnCreate(IDevice& aDevice)
{
    return(new ProxyTime(*this, aDevice));
}


IWatchable<TUint>& ServiceTime::Duration()
{
    return *iDuration;
}


IWatchable<TUint>& ServiceTime::Seconds()
{
    return *iSeconds;
}
//    }

///////////////////////////////////////////////////////////////

//    public sealed class ServiceTimeNetwork : ServiceTime
//    {
ServiceTimeNetwork::ServiceTimeNetwork(IInjectorDevice& aDevice, Net::CpProxyAvOpenhomeOrgTime1* aService, ILog& aLog)
    :ServiceTime(aDevice, aLog)
    ,iService(aService)
{

    //iService->SetPropertyDurationChanged(HandleDurationChanged);

    Functor f1 = MakeFunctor(*this, &ServiceTimeNetwork::HandleDurationChanged);
    iService->SetPropertyDurationChanged(f1);

    Functor f2 = MakeFunctor(*this, &ServiceTimeNetwork::HandleSecondsChanged);
    iService->SetPropertySecondsChanged(f2);

    Functor f3 = MakeFunctor(*this, &ServiceTimeNetwork::HandleInitialEvent);
    iService->SetPropertyInitialEvent(f3);
}


void ServiceTimeNetwork::Dispose()
{
    ServiceTime::Dispose();
    //iService->Dispose();
}


TBool ServiceTimeNetwork::OnSubscribe()
{
/*
    ASSERT(iSubscribedSource == NULL);

    iSubscribedSource = new TaskCompletionSource<TBool>();

    iService.Subscribe();

    iSubscribed = true;

    return iSubscribedSource.Task.ContinueWith((t) => { });
*/

    iService->Subscribe();
    iSubscribed = true;
    return(false); // false = not mock
}


void ServiceTimeNetwork::OnCancelSubscribe()
{
/*
    if (iSubscribedSource != null)
    {
        iSubscribedSource.TrySetCanceled();
    }
*/
}


void ServiceTimeNetwork::HandleInitialEvent()
{
/*
    if (!iSubscribedSource.Task.IsCanceled)
    {
        iSubscribedSource.SetResult(true);
    }
*/
    SubscribeCompleted();
}


void ServiceTimeNetwork::OnUnsubscribe()
{
    if (iService != NULL)
    {
        iService->Unsubscribe();
    }
    iSubscribed = false;
}


void ServiceTimeNetwork::HandleDurationChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceTimeNetwork::DurationChangedCallback1);
    Schedule(f, NULL);
}


void ServiceTimeNetwork::DurationChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceTimeNetwork::DurationChangedCallback1);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceTimeNetwork::DurationChangedCallback2(void*)
{
    if (iSubscribed)
    {
        TUint duration;
        iService->PropertyDuration(duration);
        iDuration->Update(duration);
    }
}


void ServiceTimeNetwork::HandleSecondsChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceTimeNetwork::SecondsChangedCallback1);
    Schedule(f, NULL);
}


void ServiceTimeNetwork::SecondsChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceTimeNetwork::SecondsChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceTimeNetwork::SecondsChangedCallback2(void*)
{
    if (iSubscribed)
    {
        TUint seconds;
        iService->PropertyDuration(seconds);
        iSeconds->Update(seconds);
    }
}



/*
    class ServiceTimeMock : ServiceTime, IMockable
    {
        public ServiceTimeMock(IInjectorDevice aDevice, TUint aSeconds, TUint aDuration, ILog aLog)
            : base(aDevice, aLog)
        {
            iDuration.Update(aDuration);
            iSeconds.Update(aSeconds);
        }

        public override void Execute(IEnumerable<string> aValue)
        {
            string command = aValue.First().ToLowerInvariant();
            if (command == "duration")
            {
                IEnumerable<string> value = aValue.Skip(1);
                iDuration.Update(TUint.Parse(value.First()));
            }
            else if (command == "seconds")
            {
                IEnumerable<string> value = aValue.Skip(1);
                iSeconds.Update(TUint.Parse(value.First()));
            }
            else
            {
                throw new NotSupportedException();
            }
        }
    }
*/

    //ProxyTime : Proxy<ServiceTime>, IProxyTime
    //{

///////////////////////////////////////////////////////////

ProxyTime::ProxyTime(ServiceTime& aService, IDevice& aDevice)
    :iService(aService)
    ,iDevice(aDevice)
{
}


IWatchable<TUint>& ProxyTime::Duration()
{
    return iService.Duration();
}


IWatchable<TUint>& ProxyTime::Seconds()
{
    return iService.Seconds();
}


IDevice& ProxyTime::Device()
{
    return(iDevice);
}


void ProxyTime::Dispose()
{
    iService.Unsubscribe();
}
