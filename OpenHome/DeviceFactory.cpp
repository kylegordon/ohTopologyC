#include <OpenHome/DeviceFactory.h>
#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/ServiceProduct.h>
#include <OpenHome/ServiceSender.h>
#include <OpenHome/ServiceReceiver.h>
#include <OpenHome/ServiceVolume.h>
#include <OpenHome/ServiceRadio.h>
#include <OpenHome/ServicePlaylist.h>
#include <OpenHome/ServiceInfo.h>
#include <vector>
#include <memory>


using namespace OpenHome ;
using namespace OpenHome::Av;
using namespace OpenHome::Net;
using namespace std;



IInjectorDevice* DeviceFactory::CreateDs(INetwork& aNetwork, const Brx& aUdn, ILog& aLog)
{
    LOG(kTrace, ">DeviceFactory::CreateDs \n");
    return CreateDs(aNetwork, aUdn, Brn("Main Room"), Brn("Mock DS"), Brn("Info Time Volume Sender"), aLog);
}

IInjectorDevice* DeviceFactory::CreateDs(INetwork& aNetwork, const Brx& aUdn, const Brx& aRoom, const Brx& aName, const Brx& aAttributes, ILog& aLog)
{
    LOG(kTrace, ">DeviceFactory::CreateDs \n");
    InjectorDevice* device = new InjectorDevice(aUdn);
    // add a factory for each type of watchable service

    // product service
    std::unique_ptr<SrcXml> xml(new SrcXml());

    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Playlist"), Brn("Playlist"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Radio"), Brn("Radio"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("UPnP AV"), Brn("UpnpAv"), false)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Songcast"), Brn("Receiver"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Net Aux"), Brn("NetAux"), false)));


    device->Add(eProxyProduct, new ServiceProductMock(aNetwork, *device, aRoom, aName, 0, move(xml), true, aAttributes, Brx::Empty(),
                                    Brn("Linn Products Ltd"), Brn("Linn"), Brn("http://www.linn.co.uk"), Brx::Empty(),
                                    Brn("Linn High Fidelity System Component"), Brn("Mock DS"), Brx::Empty(), Brx::Empty(),
                                    Brn("Linn High Fidelity System Component"), Brx::Empty(), aUdn, aLog) );


/*
    // volume service
    device->Add<IProxyVolume>(new ServiceVolumeMock(aNetwork, device, aUdn, 0, 15, 0, 0, false, 50, 100, 100, 1024, 100, 80, aLog));

    // info service
    device->Add<IProxyInfo>(new ServiceInfoMock(aNetwork, device, new InfoDetails(0, 0, Brx:Empty(), 0, false, 0), new InfoMetadata(aNetwork.TagManager.FromDidlLite(Brx:Empty()), Brx:Empty()), new InfoMetatext(aNetwork.TagManager.FromDidlLite(Brx:Empty())), aLog));

    // time service
    device->Add<IProxyTime>(new ServiceTimeMock(aNetwork, device, 0, 0, aLog));
*/
    // sender service



    Bwh senderMeta(Brn("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Main Room:Mock DS</dc:title><res protocolInfo=\"ohz:*:*:u\">ohz://239.255.255.250:51972/"));
    senderMeta.Grow(2000);
    senderMeta.Append(aUdn);
    senderMeta.Append(Brn("</res><upnp:albumArtURI>http://10.2.10.27/images/Icon.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));




    SenderMetadata* smd = new SenderMetadata(senderMeta);
    ServiceSenderMock* ssm = new ServiceSenderMock(aNetwork, *device, aAttributes, Brx::Empty(), false, smd, Brn("Enabled"), aLog);
    device->Add(eProxySender, ssm);

    // receiver service

    ServiceReceiverMock* srm =  new ServiceReceiverMock(aNetwork, *device, Brx::Empty(), Brn("ohz:*:*:*,ohm:*:*:*,ohu:*.*.*"), Brn("Stopped"), Brx::Empty(), aLog);
    device->Add(eProxyReceiver, srm);



    /*
    // radio service
    List<IMediaMetadata> presets = new List<IMediaMetadata>();
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Linn Radio (Variety)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"40000\">http://opml.radiotime.com/Tune.ashx?id=s122119&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s122119q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Linn Jazz (Jazz)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"40000\">http://opml.radiotime.com/Tune.ashx?id=s122120&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s122120q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(null);
    presets.Add(null);
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Linn Classical (Classical)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"40000\">http://opml.radiotime.com/Tune.ashx?id=s122116&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s122116q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>BBC World Service (World News)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"4000\">http://opml.radiotime.com/Tune.ashx?id=s50646&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s50646q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Sky Radio News (World News)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"4000\">http://opml.radiotime.com/Tune.ashx?id=s81093&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s81093q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));


    device->Add<IProxyRadio>(new ServiceRadioMock(aNetwork, device, 0, presets, InfoMetadata.Empty, Brx:Empty(), "Stopped", 100, aLog));

    // playlist service
    List<IMediaMetadata> tracks = new List<IMediaMetadata>();
    device->Add<IProxyPlaylist>(new ServicePlaylistMock(aNetwork, device, 0, tracks, false, false, "Stopped", Brx:Empty(), 1000, aLog));
*/
    return device;
}



IInjectorDevice* DeviceFactory::CreateDsm(INetwork& aNetwork, const Brx& aUdn, ILog& aLog)
{
    return CreateDsm(aNetwork, aUdn, Brn("Main Room"), Brn("Mock Dsm"), Brn("Info Time Volume Sender"), aLog);
}

IInjectorDevice* DeviceFactory::CreateDsm(INetwork& aNetwork, const Brx& aUdn, const Brx& aRoom, const Brx& aName, const Brx& aAttributes, ILog& aLog)
{
    InjectorDevice* device = new InjectorDevice(aUdn);
    // add a factory for each type of watchable service

    // product service
    std::unique_ptr<SrcXml> xml(new SrcXml());
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Playlist"), Brn("Playlist"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Radio"), Brn("Radio"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("UPnP AV"), Brn("UpnpAv"), false)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Songcast"), Brn("Receiver"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Net Aux"), Brn("NetAux"), false)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Analog1"), Brn("Analog"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Analog2"), Brn("Analog"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("Phono"), Brn("Analog"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("SPDIF1"), Brn("Digital"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("SPDIF2"), Brn("Digital"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("TOSLINK1"), Brn("Digital"), true)));
    xml->Add(unique_ptr<TopologySource>(new TopologySource(Brn("TOSLINK2"), Brn("Digital"), true)));



    device->Add(eProxyProduct, new ServiceProductMock(aNetwork, *device, aRoom, aName, 0, std::move(xml), true, aAttributes, Brx::Empty(),
                Brn("Linn Products Ltd"), Brn("Linn"), Brn("http://www.linn.co.uk"), Brx::Empty(),
                Brn("Linn High Fidelity System Component"), Brn("Mock DSM"), Brx::Empty(), Brx::Empty(),
                Brn("Linn High Fidelity System Component"), Brx::Empty(), aUdn, aLog));

/*
    // volume service
    device->Add<IProxyVolume>(new ServiceVolumeMock(aNetwork, device, aUdn, 0, 15, 0, 0, false, 50, 100, 100, 1024, 100, 80, aLog));

    // info service
    device->Add<IProxyInfo>(new ServiceInfoMock(aNetwork, device, new InfoDetails(0, 0, Brx:Empty(), 0, false, 0), new InfoMetadata(aNetwork.TagManager.FromDidlLite(Brx:Empty()), Brx:Empty()), new InfoMetatext(aNetwork.TagManager.FromDidlLite(Brx:Empty())), aLog));

    // time service
    device->Add<IProxyTime>(new ServiceTimeMock(aNetwork, device, 0, 0, aLog));
*/
    // sender service
    Bwh senderMeta(Brn("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Main Room:Mock DSM</dc:title><res protocolInfo=\"ohz:*:*:u\">ohz://239.255.255.250:51972/"));
    senderMeta.Grow(2000);
    senderMeta.Append(aUdn);
    senderMeta.Append(Brn("</res><upnp:albumArtURI>http://10.2.10.27/images/Icon.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));

    device->Add(eProxySender, new ServiceSenderMock(aNetwork, *device, aAttributes, Brx::Empty(), false, new SenderMetadata(senderMeta), Brn("Enabled"), aLog));

    // receiver service
    device->Add(eProxyReceiver, new ServiceReceiverMock(aNetwork, *device, Brx::Empty(), Brn("ohz:*:*:*,ohm:*:*:*,ohu:*.*.*"), Brn("Stopped"), Brx::Empty(), aLog));

    /*
    // radio service
    List<IMediaMetadata> presets = new List<IMediaMetadata>();
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Linn Radio (Variety)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"40000\">http://opml.radiotime.com/Tune.ashx?id=s122119&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s122119q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Linn Jazz (Jazz)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"40000\">http://opml.radiotime.com/Tune.ashx?id=s122120&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s122120q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(null);
    presets.Add(null);
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Linn Classical (Classical)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"40000\">http://opml.radiotime.com/Tune.ashx?id=s122116&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s122116q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>BBC World Service (World News)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"4000\">http://opml.radiotime.com/Tune.ashx?id=s50646&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s50646q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));
    presets.Add(aNetwork.TagManager.FromDidlLite("<DIDL-Lite xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns:upnp=\"urn:schemas-upnp-org:metadata-1-0/upnp/\" xmlns=\"urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/\"><item id=\"\" parentID=\"\" restricted=\"True\"><dc:title>Sky Radio News (World News)</dc:title><res protocolInfo=\"*:*:*:*\" bitrate=\"4000\">http://opml.radiotime.com/Tune.ashx?id=s81093&amp;formats=mp3,wma,aac,wmvideo,ogg&amp;partnerId=`ah2rjr68&amp;username=linnproducts&amp;c=ebrowse</res><upnp:albumArtURI>http://d1i6vahw24eb07.cloudfront.net/s81093q.png</upnp:albumArtURI><upnp:class>object.item.audioItem</upnp:class></item></DIDL-Lite>"));

    device->Add<IProxyRadio>(new ServiceRadioMock(aNetwork, device, 0, presets, InfoMetadata.Empty, Brx:Empty(), "Stopped", 100, aLog));

    // playlist service
    List<IMediaMetadata> tracks = new List<IMediaMetadata>();
    device->Add<IProxyPlaylist>(new ServicePlaylistMock(aNetwork, device, 0, tracks, false, false, "Stopped", Brx:Empty(), 1000, aLog));
*/
    return device;
}

/*
IInjectorDevice* DeviceFactory::CreateMediaServer(INetwork aNetwork, const Brx& aUdn, const Brx& aResourceRoot, ILog aLog)
{
    return (new DeviceMediaEndpointMock(aNetwork, aUdn, aResourceRoot, aLog));
}
*/


IInjectorDevice* DeviceFactory::Create(INetwork& aNetwork, CpDevice& aDevice, ILog& aLog)
{
    InjectorDevice* device = new InjectorDevice(aDevice.Udn());


    Brh value;

    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Product", value))
    {
        if (Ascii::Uint(value)==1)
        {
            device->Add(eProxyProduct, new ServiceProductNetwork(aNetwork, *device, aDevice, aLog));
        }
    }


    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Info", value))
    {
        if (Ascii::Uint(value)==1)
        {
            device->Add(eProxyInfo, new ServiceInfoNetwork(aNetwork, *device, aDevice, aLog));
        }
    }

/*
    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Time", value))
    {
        if (Ascii::Uint(value)==1)
        {
            device->Add(eProxyTime, new ServiceTimeNetwork(aNetwork, device, aDevice, aLog));
        }
    }
*/
    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Sender", value))
    {
        if (Ascii::Uint(value)==1)
        {
            device->Add(eProxySender, new ServiceSenderNetwork(aNetwork, *device, aDevice, aLog));
        }
    }

    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Volume", value))
    {
        if (Ascii::Uint(value)==1)
        {
            device->Add(eProxyVolume, new ServiceVolumeNetwork(aNetwork, *device, aDevice, aLog));
        }
    }

    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Playlist", value))
    {
        if (Ascii::Uint(value)==1)
        {
            device->Add(eProxyPlaylist, new ServicePlaylistNetwork(aNetwork, *device, aDevice, aLog));
        }
    }
    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Radio", value))
    {
        if (Ascii::Uint(value)==1)
        {
            device->Add(eProxyRadio, new ServiceRadioNetwork(aNetwork, *device, aDevice, aLog));
        }
    }

    if (aDevice.GetAttribute("Upnp.Service.av-openhome-org.Receiver", value))
    {
        if (Ascii::Uint(value)==1)
        {
            device->Add(eProxyReceiver, new ServiceReceiverNetwork(aNetwork, *device, aDevice, aLog));
        }
    }
/*
    if (aDevice.GetAttribute("Upnp.Service.linn-co-uk.Sdp", value))
    {
        //if (uint.Parse(value) == 1)
        if (Ascii::Uint(value)==1)
        {
            device->Add(eProxySdp, new ServiceSdpNetwork(aNetwork, device, aDevice, aLog));
        }
    }
*/
    return device;
}




