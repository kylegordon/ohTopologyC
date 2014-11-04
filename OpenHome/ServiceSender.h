#ifndef HEADER_OHTOPOLOGYC_SERVICE_SENDER
#define HEADER_OHTOPOLOGYC_SERVICE_SENDER

#include <OpenHome/Buffer.h>
#include <OpenHome/MetaData.h>
#include <OpenHome/Service.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <Generated/CpAvOpenhomeOrgSender1.h>
#include <OpenHome/Job.h>


namespace OpenHome
{
namespace Av
{

////////////////////////////////////////////////////////////////////

class IProxySender : public IProxy
{
public:
    virtual IWatchable<TBool>& Audio() = 0;
    virtual IWatchable<ISenderMetadata*>& Metadata() = 0;
    virtual IWatchable<Brn>& Status() = 0;

    virtual const Brx& Attributes() = 0;
    virtual const Brx& PresentationUrl() = 0;
    virtual ~IProxySender() {};
};

/////////////////////////////////

class ServiceSender : public Service
{
public:
    ~ServiceSender();
    virtual void Dispose();
    virtual IProxy* OnCreate(IDevice& aDevice);
    virtual IWatchable<TBool>& Audio();
    virtual IWatchable<ISenderMetadata*>& Metadata();
    virtual IWatchable<Brn>& Status();
    virtual const Brx& Attributes();
    virtual const Brx& PresentationUrl();

protected:
    ServiceSender(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog);

protected:
    Bws<100> iAttributes;
    Bws<100> iPresentationUrl;
    Watchable<TBool>* iAudio;
    Watchable<ISenderMetadata*>* iMetadata;
    Watchable<Brn>* iStatus;
    ISenderMetadata* iCurrentMetadata;
    TBool iCurrentAudio;
    Bws<100>* iCurrentStatus; //FIXME:random capacity
};

/////////////////////////////////////////////////////////

class ServiceSenderNetwork : public ServiceSender
{
public:
    ServiceSenderNetwork(INetwork& aNetwork, IInjectorDevice& aDevice, Net::CpDevice& aCpDevice, ILog& aLog);

    virtual void Dispose();

protected:
    virtual Job* OnSubscribe();
    virtual void OnCancelSubscribe();
    virtual void OnUnsubscribe();

private:
    void HandleInitialEvent();
    void HandleAudioChanged();
    void HandleMetadataChanged();
    void HandleStatusChanged();

    void AudioChangedCallback(void* aAudio);
    void AudioChangedCallbackCallback(void* aAudio);
    void MetadataChangedCallback(void* aMetadata);
    void MetadataChangedCallbackCallback(void* aMetadata);
    void StatusChangedCallback(void* aStatus);
    void StatusChangedCallbackCallback(void* aStatus);

    void OnSubscribeCallback(void* aObj);


private:
    Net::CpDevice& iCpDevice;
    JobDone* iSubscribedSource;
    Net::CpProxyAvOpenhomeOrgSender1* iService;

    TBool iAudioValue;
};


////////////////////////////////////////////////////////////////

class ServiceSenderMock : public ServiceSender
{
public:
    ServiceSenderMock(INetwork& aNetwork, IInjectorDevice& aDevice,  const Brx& aAttributes, const Brx& aPresentationUrl,
                      TBool aAudio, ISenderMetadata* aMetadata, const Brx& aStatus, ILog& aLog);

    virtual void Execute(ICommandTokens& aValue);
};

///////////////////////////////////////////////////////////////

class ProxySender : /*public Proxy<ServiceSender*>,*/ public IProxySender, public INonCopyable
{
public:
    ProxySender(ServiceSender& aService, IDevice& aDevice);

    virtual void Dispose();
    virtual IDevice& Device();

    virtual const Brx& Attributes();
    virtual const Brx& PresentationUrl();
    virtual IWatchable<TBool>& Audio();
    virtual IWatchable<ISenderMetadata*>& Metadata();
    virtual IWatchable<Brn>& Status();

private:
    ServiceSender& iService;
    IDevice& iDevice;
};



} // Av
} // OpenHome


#endif // HEADER_OHTOPOLOGYC_SERVICE_SENDER
