#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topology1.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Injector.h>
#include <OpenHome/Tests/TestScriptHttpReader.h>
#include <exception>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::TestFramework;


EXCEPTION(TestException);


namespace OpenHome {

namespace Av {


class TestExceptionReporter;

/////////////////////////////////////////////////////////////////////

class SuiteTopology1: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteTopology1(IReader& aReader);

private:
    // from SuiteUnitTest
    void Setup();
    void TearDown();
    void Test1();

private:
    void ExecuteCallback(void* aObj);
    void ScheduleCallback(void* aObj);

private:
    Topology1* iTopology1;
    IReader& iReader;
};

/////////////////////////////////////////////////////////////////////

class ProductWatcher : public IWatcherUnordered<IProxyProduct*> , public INonCopyable
{
public:
    ProductWatcher(MockableScriptRunner& aRunner) : iRunner(aRunner) {}

    virtual void UnorderedOpen() {}
    virtual void UnorderedClose() {}
    virtual void UnorderedInitialised() {}

    virtual void UnorderedAdd(IProxyProduct* aWatcher)
    {
        Bws<100> buf;
        buf.Replace(Brn("product added "));
        buf.Append(aWatcher->Device().Udn());

        Bwh* result = new Bwh(buf);
        iRunner.Result(result);
    }

    virtual void UnorderedRemove(IProxyProduct* aWatcher)
    {
        Bws<100> buf;
        buf.Replace(Brn("product removed "));
        buf.Append(aWatcher->Device().Udn());

        Bwh* result = new Bwh(buf);
        iRunner.Result(result);
    }

private:
    MockableScriptRunner& iRunner;
};


} // namespace Av

} // namespace OpenHome


/////////////////////////////////////////////////////////////////////


SuiteTopology1::SuiteTopology1(IReader& aReader)
    :SuiteUnitTest("SuiteTopology1")
    ,iReader(aReader)
{
    AddTest(MakeFunctor(*this, &SuiteTopology1::Test1));
}


void SuiteTopology1::Setup()
{
}


void SuiteTopology1::TearDown()
{

}


void SuiteTopology1::Test1()
{
    Mockable* mocker = new Mockable();

    ILog* log = new LogDummy();
    Network* network = new Network(50, *log);

    InjectorMock* mockInjector = new InjectorMock(*network, Brx::Empty(), *log);
    mocker->Add(Brn("network"), *mockInjector);

    iTopology1 = new Topology1(network, *log);

    MockableScriptRunner* runner = new MockableScriptRunner();

    ProductWatcher* watcher = new ProductWatcher(*runner);

    FunctorGeneric<void*> fs = MakeFunctorGeneric(*this, &SuiteTopology1::ScheduleCallback);
    network->Schedule(fs, watcher);

    Functor f = MakeFunctor(*network, &Network::Wait);

    TEST(runner->Run(f, iReader, *mocker));

    FunctorGeneric<void*> fe = MakeFunctorGeneric(*this, &SuiteTopology1::ExecuteCallback);
    network->Execute(fe, watcher);

    iTopology1->Dispose();
    mockInjector->Dispose();
    network->Dispose();

    delete watcher;
    delete mocker;
    delete runner;
    delete log;

    delete iTopology1;
    delete network;
    delete mockInjector;
}


void SuiteTopology1::ExecuteCallback(void* aObj)
{
    LOG(kTrace, "SuiteTopology1::ExecuteCallback() \n");
    ProductWatcher* watcher = (ProductWatcher*)aObj;
    iTopology1->Products().RemoveWatcher(*watcher);
}


void SuiteTopology1::ScheduleCallback(void* aObj)
{
    LOG(kTrace, "SuiteTopology1::ScheduleCallback() \n");
    ProductWatcher* watcher = (ProductWatcher*)aObj;
    iTopology1->Products().AddWatcher(*watcher);
}


////////////////////////////////////////////

void TestTopology1(Environment& aEnv, const std::vector<Brn>& aArgs)
{
    std::vector<Brn> args(aArgs);

    if(args.size()<2)
    {
        args.push_back(Brn("--path"));
        args.push_back(Brn("~eamonnb/Topology1TestScript.txt"));
    }

    TestScriptHttpReader reader(aEnv, args);

    Runner runner("Topology1 tests\n");
    runner.Add(new SuiteTopology1(reader));
    runner.Run();
}
