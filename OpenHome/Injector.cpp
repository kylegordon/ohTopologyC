#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/Network.h>
#include <OpenHome/Injector.h>
#include <OpenHome/Device.h>
#include <OpenHome/DeviceFactory.h>
#include <OpenHome/Private/Ascii.h>
#include <map>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::Net;
using namespace std;


Injector::Injector(Network& aNetwork, CpStack& aCpStack, const Brx& aDomain, const Brx& aType, TUint aVersion, ILog& aLog)
    :iDisposeHandler(new DisposeHandler())
    ,iNetwork(aNetwork)
    ,iLog(aLog)
{
    FunctorCpDevice fAdded = Net::MakeFunctorCpDevice(*this, &Injector::Added);
    FunctorCpDevice fRemoved = Net::MakeFunctorCpDevice(*this, &Injector::Removed);

    iDeviceList = new CpDeviceListUpnpServiceType(aCpStack, aDomain, aType, aVersion, fAdded, fRemoved);
}


void Injector::Added(/*CpDeviceList& aList, */CpDevice& aDevice)
{
    Brn udn(aDevice.Udn());

    if (!FilterOut(aDevice))
    {
        IInjectorDevice* device = Create(iNetwork, aDevice);


        iDeviceLookup[Brn("udn")] = device;


        iNetwork.Add(device);
    }
}


void Injector::Removed(/*CpDeviceList& aList, */CpDevice& aDevice)
{
    Brn udn(aDevice.Udn());

    if (iDeviceLookup.count(udn)>0)
    {
        IInjectorDevice* device = iDeviceLookup[udn];
        iNetwork.Remove(device);
        iDeviceLookup.erase(udn);
    }

}


IInjectorDevice* Injector::Create(INetwork& aNetwork, CpDevice& aDevice)
{
    DisposeLock lock(*iDisposeHandler);
    return (DeviceFactory::Create(aNetwork, aDevice, iLog));
}


TBool Injector::FilterOut(CpDevice& /*aCpDevice*/)
{
    return false;
}


void Injector::Refresh()
{
    DisposeLock lock(*iDisposeHandler);
    iDeviceList->Refresh();
}


void Injector::Dispose()
{
    //iDeviceList->Dispose();
    iDisposeHandler->Dispose();
}

/////////////////////////////////////////////////////////////////

InjectorProduct::InjectorProduct(Network& aNetwork, CpStack& aCpStack, ILog& aLog)
    : Injector(aNetwork, aCpStack, Brn("av.openhome.org"), Brn("Product"), 1, aLog)
{
}

/////////////////////////////////////////////////////////////////

InjectorSender::InjectorSender(Network& aNetwork, CpStack& aCpStack, ILog& aLog)
    : Injector(aNetwork, aCpStack, Brn("av.openhome.org"), Brn("Sender"), 1, aLog)
{
}


TBool InjectorSender::FilterOut(CpDevice& aCpDevice)
{
    Brh value;
    return aCpDevice.GetAttribute("Upnp.Service.av-openhome-org.Product", value);
}


/////////////////////////////////////////////////////////////////

InjectorMock::InjectorMock(Network& aNetwork, const Brx& /*aResourceRoot*/, ILog& aLog)
    :iNetwork(aNetwork)
    //,iResourceRoot(aResourceRoot)
    ,iLog(aLog)
{
}


InjectorMock::~InjectorMock()
{
    map<Brn, InjectorDeviceMock*, BufferCmp>::iterator it;
    for(it=iMockDevices.begin();it!=iMockDevices.end();it++)
    {
        delete it->second;
    }
}


void InjectorMock::Dispose()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &InjectorMock::DisposeCallback);
    iNetwork.Execute(f, NULL);
}


void InjectorMock::DisposeCallback(void*)
{
    map<Brn, InjectorDeviceMock*, BufferCmp>::iterator it;
    for(it=iMockDevices.begin();it!=iMockDevices.end();it++)
    {
        it->second->Dispose();
        //delete it->second;
    }
}


void InjectorMock::Execute(ICommandTokens& aTokens)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &InjectorMock::ExecuteCallback);
    iNetwork.Execute(f, &aTokens);
}


void InjectorMock::ExecuteCallback(void* aObj)
{
    ICommandTokens& val = *((ICommandTokens*)aObj);
    ASSERT(val.Count()>0);

    Brn command(val.Next());


    if (Ascii::CaseInsensitiveEquals(command, Brn("small")))
    {
        //CreateAndAdd(DeviceFactory.CreateDsm(iNetwork, "4c494e4e-0026-0f99-1112-ef000004013f", "Sitting Room", "Klimax DSM", "Info Time Volume Sender", iLog));
        //CreateAndAdd(DeviceFactory.CreateMediaServer(iNetwork, "4c494e4e-0026-0f99-0000-000000000000", iResourceRoot, iLog));
        return;
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("medium")))
    {
        CreateAndAdd(DeviceFactory::CreateDs(iNetwork, Brn("4c494e4e-0026-0f99-1111-ef000004013f"), Brn("Kitchen"), Brn("Sneaky Music DS"), Brn("Info Time Volume Sender"), iLog));
        CreateAndAdd(DeviceFactory::CreateDsm(iNetwork, Brn("4c494e4e-0026-0f99-1112-ef000004013f"), Brn("Sitting Room"), Brn("Klimax DSM"), Brn("Info Time Volume Sender"), iLog));
        CreateAndAdd(DeviceFactory::CreateDsm(iNetwork, Brn("4c494e4e-0026-0f99-1113-ef000004013f"), Brn("Bedroom"), Brn("Kiko DSM"), Brn("Info Time Volume Sender"), iLog));
        CreateAndAdd(DeviceFactory::CreateDs(iNetwork, Brn("4c494e4e-0026-0f99-1114-ef000004013f"), Brn("Dining Room"), Brn("Majik DS"), Brn("Info Time Volume Sender"), iLog));
        //CreateAndAdd(DeviceFactory.CreateMediaServer(iNetwork, "4c494e4e-0026-0f99-0000-000000000000", iResourceRoot, iLog));
        return;
    }
    else if (Ascii::CaseInsensitiveEquals(command,Brn("large")))
    {
        ASSERTS();
        THROW(NotImplementedException);
    }
    else if (Ascii::CaseInsensitiveEquals(command,Brn("create")))
    {
        ASSERT(val.Count()>1);

        Brn type(val.Next());
        Brn udn(val.Next());

        if (type == Brn("ds"))
        {
            Create(DeviceFactory::CreateDs(iNetwork, udn, iLog));
            return;
        }
        else if (type == Brn("dsm"))
        {
            //Create(*DeviceFactory:CreateDsm(iNetwork, udn, iLog));
            return;
        }
    }
    else if (Ascii::CaseInsensitiveEquals(command,Brn("add")))
    {
        ASSERT(val.Count()>0);
        Brn udn(val.Next());

        if (iMockDevices.count(udn) > 0)
        {
            InjectorDeviceMock* device = iMockDevices[udn];
            iNetwork.Add(device->On());
            return;
        }
    }
    else if (Ascii::CaseInsensitiveEquals(command,Brn("remove")))
    {
        ASSERT(val.Count()>0);
        Brn udn(val.Next());

        if (iMockDevices.count(udn) > 0)
        {
            InjectorDeviceMock* device = iMockDevices[udn];
            iNetwork.Remove(device->Off());
            return;
        }
    }
    else if (Ascii::CaseInsensitiveEquals(command,Brn("destroy")))
    {
        ASSERT(val.Count()>0);
        Brn udn(val.Next());

        if (iMockDevices.count(udn) > 0)
        {
            InjectorDeviceMock* device = iMockDevices[udn];
            iNetwork.Remove(device->Off());
            device->Dispose();
            return;
        }
    }
    else if (Ascii::CaseInsensitiveEquals(command,Brn("update")))
    {
        ASSERT(val.Count()>1);
        Brn udn(val.Next());

        if (iMockDevices.count(udn) > 0)
        {
            InjectorDeviceMock* device = iMockDevices[udn];
            device->Execute(val);
            return;
        }
    }

    ASSERTS();
}


InjectorDeviceMock* InjectorMock::Create(IInjectorDevice* aDevice)
{
    InjectorDeviceMock* device = new InjectorDeviceMock(aDevice);
    iMockDevices[Brn(aDevice->Udn())] = device;
    return(device);
}


void InjectorMock::CreateAndAdd(IInjectorDevice* aDevice)
{
    InjectorDeviceMock* device = Create(aDevice);
    IInjectorDevice* dev = device->On();
    iNetwork.Add(dev);
}

/////////////////////////////////////////////////////////////////






