#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topology4.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Injector.h>
#include <OpenHome/OsWrapper.h>
#include <OpenHome/Private/OptionParser.h>
#include <OpenHome/Private/Http.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/MetaData.h>
#include <exception>
#include <vector>


using namespace std;
using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::TestFramework;


namespace OpenHome
{
namespace Topology
{
namespace TestTopology4
{


class SuiteTopology4: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteTopology4(ReaderUntil& aReader);

private:
    // from SuiteUnitTest
    void Setup();
    void TearDown();
    void Test1();

private:
    void ExecuteCallback(void* aObj);
    void ScheduleCallback(void* aObj);

private:
    Topology4* iTopology4;
    ReaderUntil& iReader;
};



////////////////////////////////////////////////////////////////////////////////////

class RootWatcher : public IDisposable, public INonCopyable
{
public:
    RootWatcher(MockableScriptRunner& aRunner, ITopologyRoot& aRoot)
        :iFactory(new ResultWatcherFactory(aRunner))
    {
        iFactory->Create<ITopologySource*>(aRoot.Name(), aRoot.Source(), MakeFunctorGeneric(*this, &RootWatcher::CreateCallback1));
        iFactory->Create<vector<ITopologyGroup*>*>(aRoot.Name(), aRoot.Senders(), MakeFunctorGeneric(*this, &RootWatcher::CreateCallback2));
    }

    void CreateCallback1(MockCbData<ITopologySource*>* aArgs)
    {
        ITopologySource* s = aArgs->iData;
        auto f = aArgs->iCallback;

        Bwh* buf = new Bwh(6000);

        buf->Replace(Brn("Source "));

        Ascii::AppendDec(*buf, s->Index());

        buf->Append(Brn(" "));
        buf->Append(s->Group().Name());
        buf->Append(Brn(" "));
        buf->Append(s->Name());
        buf->Append(Brn(" "));
        buf->Append(s->Type());
        buf->Append(Brn(" "));

        if (s->Visible())
        {
            buf->Append(Brn("True "));
        }
        else
        {
            buf->Append(Brn("False "));
        }


        if (s->HasInfo())
        {
            buf->Append(Brn("True "));
        }
        else
        {
            buf->Append(Brn("False "));
        }

        if (s->HasTime())
        {
            buf->Append(Brn("True "));
        }
        else
        {
            buf->Append(Brn("False "));
        }
        buf->Append(s->Device().Udn());

        buf->Append(Brn(" Volume"));

        for(TUint i=0; i<s->Volumes().size(); i++)
        {
            buf->Append(Brn(" "));
            buf->Append(s->Volumes()[i]->Device().Udn());
        }


        f(*buf);
        delete aArgs;
        delete buf;
    }

    void CreateCallback2(MockCbData<vector<ITopologyGroup*>*>* aArgs)
    {
        vector<ITopologyGroup*>* v = aArgs->iData;
        FunctorGeneric<const Brx&> f = aArgs->iCallback;

        Bwh* buf = new Bwh(6000);
        buf->Replace(Brn("\nSenders begin\n"));

        for(TUint i=0; i<v->size(); i++)
        {
            buf->Append(Brn("Sender "));
            buf->Append((*v)[i]->Name());
            buf->Append(Brn("\n"));
        }

        buf->Append(Brn("Senders end"));
        f(*buf);
        delete aArgs;
        delete buf;
    }


    virtual void Dispose()
    {
        iFactory->Dispose();
        delete iFactory;
    }

private:
    ResultWatcherFactory* iFactory;
};

//////////////////////////////////////////////////////////////////////

class RoomWatcher : public IWatcher<vector<ITopologyRoot*>*>, public IDisposable, public INonCopyable
{
private:
    MockableScriptRunner& iRunner;
    ITopologyRoom& iRoom;
    vector<RootWatcher*> iWatchers;

public:
    RoomWatcher(MockableScriptRunner& aRunner, ITopologyRoom& aRoom)
        :iRunner(aRunner)
        ,iRoom(aRoom)
    {
        iRoom.Roots().AddWatcher(*this);
    }

    virtual void Dispose()
    {
        iRoom.Roots().RemoveWatcher(*this);
        for(TUint i=0; i<iWatchers.size(); i++)
        {
            iWatchers[i]->Dispose();
            delete iWatchers[i];
        }
    }

    virtual void ItemOpen(const Brx& /*aId*/, vector<ITopologyRoot*>* aValue)
    {
        for(TUint i=0; i<aValue->size(); i++)
        {
            auto root = (*aValue)[i];
            iWatchers.push_back(new RootWatcher(iRunner, *root));
        }
    }

    virtual void ItemUpdate(const Brx& /*aId*/, vector<ITopologyRoot*>* aValue, vector<ITopologyRoot*>* /*aPrevious*/)
    {
        for(TUint i=0; i<iWatchers.size(); i++)
        {
            iWatchers[i]->Dispose();
            delete iWatchers[i];
        }

        iWatchers.clear();

        for(TUint i=0; i<aValue->size(); i++)
        {
            auto root = (*aValue)[i];
            iWatchers.push_back(new RootWatcher(iRunner, *root));
        }
    }

    virtual void ItemClose(const Brx& /*aId*/, vector<ITopologyRoot*>* /*aValue*/)
    {
    }
};

///////////////////////////////////////////////////////////////////

class HouseWatcher : public IWatcherUnordered<ITopologyRoom*>, public IDisposable, public INonCopyable
{
public:
    HouseWatcher(MockableScriptRunner& aRunner)
        :iRunner(aRunner)
        ,iFactory(new ResultWatcherFactory(aRunner))
    {
    }

    virtual void Dispose()
    {
        iFactory->Dispose();
        delete iFactory;

        for(auto it=iWatcherLookup.begin(); it!=iWatcherLookup.end(); it++)
        {
            it->second->Dispose();
            delete it->second;
        }
    }

    virtual void UnorderedOpen()
    {
    }

    virtual void UnorderedInitialised()
    {
    }

    virtual void UnorderedClose()
    {
    }

    virtual void UnorderedAdd(ITopologyRoom* aItem)
    {

        Bwh* result = new Bwh(10000);
        result->Replace(Brn("Room Added "));
        result->Append(aItem->Name());
        iRunner.Result(result);

        iFactory->Create<EStandby>(aItem->Name(), aItem->Standby(), MakeFunctorGeneric(*this, &HouseWatcher::CreateCallback1));
        iFactory->Create<vector<ITopologySource*>*>(aItem->Name(), aItem->Sources(), MakeFunctorGeneric(*this, &HouseWatcher::CreateCallback2));
        iWatcherLookup[aItem] = new RoomWatcher(iRunner, *aItem);
    }

    virtual void UnorderedRemove(ITopologyRoom* aItem)
    {
        Bwh* result = new Bwh(6000);
        result->Replace(Brn("Room Removed "));
        result->Append(aItem->Name());

        iRunner.Result(result);

        iFactory->Destroy(aItem->Name());
        iWatcherLookup[aItem]->Dispose();
        delete iWatcherLookup[aItem];
        iWatcherLookup.erase(aItem);
    }

    void CreateCallback1(MockCbData<EStandby>* aArgs)
    {
        // (v, w) => w("Standby " + v)
        EStandby arg1 = aArgs->iData;
        FunctorGeneric<const Brx&> f = aArgs->iCallback;

        Bwh* buf = new Bwh(6000);

        buf->Replace(Brn("Standby "));

        if (arg1==eOff)
        {
            buf->Append(Brn("eOff"));
        }
        else if (arg1==eOn)
        {
            buf->Append(Brn("eOn"));
        }
        else if (arg1==eMixed)
        {
            buf->Append(Brn("eMixed"));
        }
        else
        {
            ASSERTS();
        }

        f(*buf);
        delete aArgs;
        delete buf;
    }

    void CreateCallback2(MockCbData<vector<ITopologySource*>*>* aArgs)
    {
        vector<ITopologySource*>* v = aArgs->iData;
        FunctorGeneric<const Brx&> f = aArgs->iCallback;

        Bwh* buf = new Bwh(6000);

        buf->Replace(Brn("\nSources begin\n"));

        for(TUint i=0; i<v->size(); i++)
        {
            ITopologySource* s = (*v)[i];

            buf->Append(Brn("Source "));

            Ascii::AppendDec(*buf, s->Index());

            buf->Append(Brn(" "));
            buf->Append(s->Group().Name());
            buf->Append(Brn(" "));
            buf->Append(s->Name());
            buf->Append(Brn(" "));
            buf->Append(s->Type());
            buf->Append(Brn(" "));

            if (s->Visible())
            {
                buf->Append(Brn("True "));
            }
            else
            {
                buf->Append(Brn("False "));
            }

            Brn udn(s->Device().Udn());

            if (s->HasInfo())
            {
                buf->Append(Brn("True "));
            }
            else
            {
                buf->Append(Brn("False "));
            }

            if (s->HasTime())
            {
                buf->Append(Brn("True "));
            }
            else
            {
                buf->Append(Brn("False "));
            }


            buf->Append(udn);
            buf->Append(Brn(" Volume"));


            for(TUint i=0; i<s->Volumes().size(); i++)
            {
                buf->Append(Brn(" "));
                buf->Append(s->Volumes()[i]->Device().Udn());
            }

            buf->Append(Brn("\n"));

        }
        buf->Append(Brn("Sources end"));

        f(*buf);
        delete aArgs;
        delete buf;
    }

private:
    MockableScriptRunner& iRunner;
    ResultWatcherFactory* iFactory;
    map<ITopologyRoom*, RoomWatcher*> iWatcherLookup;
};

} // TestTopology6
} // Av
} // OpenHome


using namespace OpenHome::Topology::TestTopology4;

//////////////////////////////////////////////////////////////////////////

SuiteTopology4::SuiteTopology4(ReaderUntil& aReader)
    :SuiteUnitTest("SuiteTopology4")
    ,iReader(aReader)
{
    AddTest(MakeFunctor(*this, &SuiteTopology4::Test1));
}


void SuiteTopology4::Setup()
{
}


void SuiteTopology4::TearDown()
{
}


void SuiteTopology4::Test1()
{
    Mockable* mocker = new Mockable();
    ILog* log = new LogDummy();
    Network* network = new Network(50, *log);

    InjectorMock* mockInjector = new InjectorMock(*network, Brx::Empty(), *log);
    mocker->Add(Brn("network"), *mockInjector);


    Topology1* topology1 = new Topology1(*network, *log);
    Topology2* topology2 = new Topology2(topology1, *log);
    Topology3* topology3 = new Topology3(topology2, *log);
    iTopology4 = new Topology4(topology3, *log);

    MockableScriptRunner* runner = new MockableScriptRunner();
    //HouseWatcher* watcher = new HouseWatcher(*runner);

    GroupWatcher* watcher = new GroupWatcher(wt, runner);
    mocker.Add(Brn("groupwatcher4"), watcher);


    FunctorGeneric<void*> fs = MakeFunctorGeneric(*this, &SuiteTopology4::ScheduleCallback);
    network->Schedule(fs, watcher);

    Functor f = MakeFunctor(*network, &Network::Wait);

    TBool test = runner->Run(f, iReader, *mocker);
    //OpenHome::Log::Print("test = %d\n", test);
    TEST(test);

    FunctorGeneric<void*> fe = MakeFunctorGeneric(*this, &SuiteTopology4::ExecuteCallback);
    network->Execute(fe, watcher);

    iTopology4->Dispose();
    mockInjector->Dispose();
    network->Dispose();

    delete watcher;
    delete runner;

    delete iTopology6;
    delete mockInjector;
    delete network;

    delete log;
    delete mocker;
}


void SuiteTopology4::ExecuteCallback(void* aObj)
{
    HouseWatcher* watcher = (HouseWatcher*)aObj;
    iTopology4->Rooms().RemoveWatcher(*watcher);
    watcher->Dispose();
}


void SuiteTopology4::ScheduleCallback(void* aObj)
{
    HouseWatcher* watcher = (HouseWatcher*)aObj;
    iTopology4->Rooms().AddWatcher(*watcher);
}

//////////////////////////////////////////////////////////////

void TestTopology4(Environment& aEnv, const std::vector<Brn>& aArgs)
{
    if( find(aArgs.begin(), aArgs.end(), Brn("--path")) == aArgs.end() )
    {
        Log::Print("No path supplied!\n");
        ASSERTS();
    }

    OptionParser parser;
    OptionString optionServer("-s", "--server", Brn("eng"), "address of server to connect to");
    parser.AddOption(&optionServer);
    OptionUint optionPort("-p", "--port", 80, "server port to connect on");
    parser.AddOption(&optionPort);
    OptionString optionPath("", "--path", Brn(""), "path to use on server");
    parser.AddOption(&optionPath);
    if (!parser.Parse(aArgs) || parser.HelpDisplayed()) {
        return;
    }

    TUint port = optionPort.Value();
    ASSERT(port <= 65535);
    Bwh uriBuf(100);

    Endpoint endptServer = Endpoint(port, optionServer.Value());
    uriBuf.Replace(Brn("http://"));
    endptServer.AppendEndpoint(uriBuf);
    uriBuf.Append(Brn("/"));
    uriBuf.Append(optionPath.Value());
    Uri uri(uriBuf);

    auto reader = new HttpReader(aEnv);
    TUint code = reader->Connect(uri);

    if ((code < HttpStatus::kSuccessCodes) || (code >= HttpStatus::kRedirectionCodes))
    {
        Log::Print("Failed to connect \n");
        ASSERTS();
    }

    ReaderUntilS<1024>* readerUntil = new ReaderUntilS<1024>(*reader);

    Runner runner("Topology4 tests\n");
    runner.Add(new SuiteTopology4(*readerUntil));
    runner.Run();

    delete readerUntil;
    delete reader;
}
