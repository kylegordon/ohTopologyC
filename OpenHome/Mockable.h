#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Stream.h>

#include <vector>
#include <map>

namespace OpenHome
{
namespace Topology
{

static const TUint kMaxResultBytes = 5000;

///////////////////////////////////////////////

class Mockable : public IMockable
{
public:
    Mockable();

    void Add(const Brx& aId, IMockable& aMockable);
    void Remove(const Brx& aId);

    // IMockable
    void Execute(ICommandTokens& aTokens);

private:
     std::map<Brn, IMockable*, BufferCmp> iMockables;
};

///////////////////////////////////////////////

class IResultAggregator
{
public:
    virtual void Result(Bwh* aValue) = 0;
    virtual ~IResultAggregator(){}
};

///////////////////////////////////////////////

class MockableScriptRunner : public IResultAggregator
{
private:
    static const TUint kMaxFifoEntries = 1000;
    static const TUint kMaxLineBytes = 5000;

private:
    class AssertError : public Exception
    {
    public :
        AssertError() : Exception("ASSERT") { }
    };

public:
    MockableScriptRunner();
    TBool Run(Functor aWait, OpenHome::ReaderUntil& aStream, IMockable& aMockable);
    void Result(Bwh* aValue);

private:
    TBool Test(const Brx& aActual, const Brx& aExpected);
    void Assert(TBool aExpression);

    Fifo<Bwh*> iResultQueue;
    Bws<kMaxLineBytes> iLine;
};

//////////////////////////////////////////////

template <typename T>
struct MockCbData
{
    FunctorGeneric<const Brx&> iCallback;
    T iData;
};

//////////////////////////////////////////////

class ResultWatcherFactory : public IDisposable, public INonCopyable
{
public:
    ResultWatcherFactory(IResultAggregator& aResult);

    template <class T>
    void Create(const Brx& aId, IWatchable<T>& aWatchable, FunctorGeneric<MockCbData<T>*> aAction);
    //void Create(const Brx& aId, IWatchable<T>& aWatchable, FunctorGeneric<MockCbData<T>*> aAction);

    template <class T>
    void Create(const Brx& aId, IWatchableUnordered<T>& aWatchable, FunctorGeneric<MockCbData<T>*> aAction);
    //void Create(const Brx& aId, IWatchableUnordered<T>& aWatchable, FunctorGeneric<MockCbData<T>*> aAction);

    template <class T>
    void Create(const Brx& aId, IWatchableOrdered<T>& aWatchable, FunctorGeneric<MockCbData<T>*> aAction);
    //void Create(const Brx& aId, IWatchableOrdered<T>& aWatchable, FunctorGeneric<MockCbData<T>*> aAction);

    void Destroy(const Brx& aId);

    // IDisposable
    void Dispose();

private:
    IResultAggregator& iResult;
    std::map<Brn, std::vector<IDisposable*>, BufferCmp> iWatchers;
};

/////////////////////////////////////////////////////////////


template <class T>
class ResultWatcher : public IWatcher<T>, public IDisposable, public INonCopyable
{
public:
    ResultWatcher(IResultAggregator& aResult, const Brx& aId, IWatchable<T>& aWatchable, FunctorGeneric<MockCbData<T>*> aAction);

    // IWatcher<T>
    void ItemOpen(const Brx& aId, T aValue);
    void ItemUpdate(const Brx& aId, T aValue, T aPrevious);
    void ItemClose(const Brx& aId, T aValue);

    // IDisposable
    void Dispose();

private:
    void ItemOpenCallback(const Brx& aId);
    void ItemUpdateCallback(const Brx& aId);
    void ItemCloseCallback(const Brx& aId);

private:
    IResultAggregator& iResult;
    Brn iId;
    IWatchable<T>& iWatchable;
    FunctorGeneric<MockCbData<T>*> iAction;

    //Bws<kMaxResultBytes> iBuf;
};

//////////////////////////////////////////////////////

template <class T>
class ResultUnorderedWatcher : public IWatcherUnordered<T>, public IDisposable, public INonCopyable
{
public:
    ResultUnorderedWatcher(IResultAggregator& aResult, const Brx& aId, IWatchableUnordered<T>& aWatchable, FunctorGeneric<MockCbData<T>*> aAction);

    // IUnorderedWatcher<T>
    void UnorderedOpen();
    void UnorderedInitialised();
    void UnorderedAdd(T aItem);
    void UnorderedRemove(T aItem);
    void UnorderedClose();

    // IDisposable
    void Dispose();

private:
    void UnorderedAddCallback(const Brx& aValue);
    void UnorderedRemoveCallback(const Brx& aValue);

private:
    IResultAggregator& iResult;
    Brn iId;
    IWatchableUnordered<T>& iWatchable;
    FunctorGeneric<MockCbData<T>*> iAction;

    //Bws<kMaxResultBytes> iBuf;
};

/////////////////////////////////////////////////

template <class T>
class ResultOrderedWatcher : public IWatcherOrdered<T>, public IDisposable, public INonCopyable
{
public:
    ResultOrderedWatcher(IResultAggregator& aResult, const Brx& aId, IWatchableOrdered<T>& aWatchable, FunctorGeneric<MockCbData<T>*> aAction);

    // IWatcherOrdered<T>
    void OrderedOpen();
    void OrderedInitialised();
    void OrderedAdd(T aItem, TUint aIndex);
    void OrderedMove(T aItem, TUint aFrom, TUint aTo);
    void OrderedRemove(T aItem, TUint aIndex);
    void OrderedClose();

    // IDisposable
    void Dispose();

private:
    void OrderedAddCallback(const Brx& aValue);
    void OrderedMoveCallback(const Brx& aValue);
    void OrderedRemoveCallback(const Brx& aValue);

private:
    IResultAggregator& iResult;
    Brn iId;
    IWatchableOrdered<T>& iWatchable;
    FunctorGeneric<MockCbData<T>*> iAction;

    Bws<kMaxResultBytes> iBuf;
};

//////////////////////////////////////////////


template <class T>
void ResultWatcherFactory::Create(const Brx& aId, IWatchable<T>& aWatchable, FunctorGeneric<MockCbData<T>*> aAction)
{
    Brn id(aId);

    if (iWatchers.count(id)==0)
    {
        std::vector<IDisposable*> watchers;
        iWatchers[id] = watchers;
    }

    iWatchers[id].push_back(new ResultWatcher<T>(iResult, aId, aWatchable, aAction));
}


template <class T>
void ResultWatcherFactory::Create(const Brx& aId, IWatchableUnordered<T>& aWatchable, FunctorGeneric<MockCbData<T>*> aAction)
{
    Brn id(aId);

    if (iWatchers.count(id)==0)
    {
        std::vector<IDisposable*> watchers;
        iWatchers[id] = watchers;
    }

    iWatchers[id].push_back(new ResultUnorderedWatcher<T>(iResult, aId, aWatchable, aAction));
}


template <class T>
void ResultWatcherFactory::Create(const Brx& aId, IWatchableOrdered<T>& aWatchable, FunctorGeneric<MockCbData<T>*> aAction)
{
    Brn id(aId);

    if (iWatchers.count(id)==0)
    {
        std::vector<IDisposable*> watchers;
        iWatchers[id] = watchers;
    }

    iWatchers[id].push_back(new ResultOrderedWatcher<T>(iResult, aId, aWatchable, aAction));
}

/////////////////////////////////////////////////////////////////////////////////////

template <class T>
ResultWatcher<T>::ResultWatcher(IResultAggregator& aResult, const Brx& aId, IWatchable<T>& aWatchable, FunctorGeneric<MockCbData<T>*> aAction)
    :iResult(aResult)
    ,iId(aId)
    ,iWatchable(aWatchable)
    ,iAction(aAction)
{
    iWatchable.AddWatcher(*this);
}


template <class T>
void ResultWatcher<T>::ItemOpen(const Brx& /*aId*/, T aValue)
{
    // ignoring aId - we're not using iId
    FunctorGeneric<const Brx&> f = MakeFunctorGeneric(*this, &ResultWatcher::ItemOpenCallback);
    auto args = new MockCbData<T>();
    args->iCallback = f;
    args->iData = aValue;

    iAction(args);
}


template <class T>
void ResultWatcher<T>::ItemOpenCallback(const Brx& aValue)
{
    Bwh* result = new Bwh(6000);
    result->Replace(iId);
    result->Append(Brn(" open "));
    result->Append(aValue);
    iResult.Result(result);
}


template <class T>
void ResultWatcher<T>::ItemUpdate(const Brx& /*aId*/, T aValue, T /*aPrevious*/)
{
    // ignoring aId - we're not using iId
    // ignoring aPrevious - not used
    FunctorGeneric<const Brx&> f = MakeFunctorGeneric(*this, &ResultWatcher::ItemUpdateCallback);
    auto args = new MockCbData<T>();
    args->iCallback = f;
    args->iData = aValue;
    iAction(args);
}


template <class T>
void ResultWatcher<T>::ItemUpdateCallback(const Brx& aValue)
{
    Bwh* result = new Bwh(6000);
    result->Replace(iId);
    result->Append(Brn(" update "));
    result->Append(aValue);
    iResult.Result(result);
}



template <class T>
void ResultWatcher<T>::ItemClose(const Brx& /*aId*/, T aValue)
{
    // ignoring aId - we're not using iId
    FunctorGeneric<const Brx&> f = MakeFunctorGeneric(*this, &ResultWatcher::ItemCloseCallback);
    auto args = new MockCbData<T>();
    args->iCallback = f;
    args->iData = aValue;

    iAction(args);
}


template <class T>
void ResultWatcher<T>::ItemCloseCallback(const Brx& /*aValue*/)
{

}


template <class T>
void ResultWatcher<T>::Dispose()
{
    iWatchable.RemoveWatcher(*this);
}

////////////////////////////////////////////////////////////


template <class T>
ResultUnorderedWatcher<T>::ResultUnorderedWatcher(IResultAggregator& aResult, const Brx& aId, IWatchableUnordered<T>& aWatchable, FunctorGeneric<MockCbData<T>*> aAction)
    :iResult(aResult)
    ,iId(aId)
    ,iWatchable(aWatchable)
    ,iAction(aAction)
{
    iWatchable.AddWatcher(*this);
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedOpen()
{
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedInitialised()
{
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedAdd(T aItem)
{
    FunctorGeneric<const Brx&> f = MakeFunctorGeneric(*this, &ResultUnorderedWatcher::UnorderedAddCallback);
    auto args = new MockCbData<T>();
    args->iCallback = f;
    args->iData = aItem;
    iAction(args);
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedAddCallback(const Brx& aValue)
{
    Bwh* result = new Bwh(6000);
    result->Replace(iId);
    result->Append(Brn(" add "));
    result->Append(aValue);
    iResult.Result(result);
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedRemove(T aItem)
{
    FunctorGeneric<const Brx&> f = MakeFunctorGeneric(*this, &ResultUnorderedWatcher::UnorderedRemoveCallback);
    auto args = new MockCbData<T>();
    args->iCallback = f;
    args->iData = aItem;
    iAction(args);
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedRemoveCallback(const Brx& aValue)
{
    Bwh* result = new Bwh(6000);
    result->Replace(iId);
    result->Append(Brn(" remove "));
    result->Append(aValue);
    iResult.Result(result);
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedClose()
{
}


template <class T>
void ResultUnorderedWatcher<T>::Dispose()
{
    iWatchable.RemoveWatcher(*this);
}

//////////////////////////////////////////////////////////////////////////////


template <class T>
ResultOrderedWatcher<T>::ResultOrderedWatcher(IResultAggregator& aResult, const Brx& aId, IWatchableOrdered<T>& aWatchable, FunctorGeneric<MockCbData<T>*> aAction)
    :iResult(aResult)
    ,iId(aId)
    ,iWatchable(aWatchable)
    ,iAction(aAction)
{
    iWatchable.AddWatcher(*this);
}


template <class T>
void ResultOrderedWatcher<T>::OrderedOpen()
{
}



template <class T>
void ResultOrderedWatcher<T>::OrderedInitialised()
{
}


template <class T>
void ResultOrderedWatcher<T>::OrderedAdd(T aItem, TUint /*aIndex*/)
{
    // Ignoring aIndex - not used
    FunctorGeneric<const Brx&> f = MakeFunctorGeneric(*this, &ResultOrderedWatcher::OrderedAddCallback);
    iAction(aItem, f);

}


template <class T>
void ResultOrderedWatcher<T>::OrderedAddCallback(const Brx& aValue)
{
    Bwh* result = new Bwh(6000);
    result->Replace(iId);
    result->Append(Brn(" add "));
    result->Append(aValue);
    iResult.Result(result);
}



template <class T>
void ResultOrderedWatcher<T>::OrderedMove(T aItem, TUint aFrom, TUint aTo)
{
    iBuf.Replace(iId);
    iBuf.Append(Brn(" moved from "));
    Ascii::AppendDec(iBuf, aFrom);
    iBuf.Append(Brn(" to "));
    Ascii::AppendDec(iBuf, aTo);
    iBuf.Append(Brn(" "));
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ResultOrderedWatcher::OrderedMoveCallback);
    iAction(aItem, f);

}



template <class T>
void ResultOrderedWatcher<T>::OrderedMoveCallback(const Brx& aValue)
{
    iBuf.Append(aValue);
    Bwh* result = new Bwh(iBuf);
    iResult.Result(result);
}



template <class T>
void ResultOrderedWatcher<T>::OrderedRemove(T aItem, TUint /*aIndex*/)
{
    // Ignoring aIndex - not used
    FunctorGeneric<const Brx&> f = MakeFunctorGeneric(*this, &ResultOrderedWatcher::OrderedRemoveCallback);
    iAction(aItem, f);
}



template <class T>
void ResultOrderedWatcher<T>::OrderedRemoveCallback(const Brx& aValue)
{
    Bwh* result = new Bwh(6000);
    result->Replace(iId);
    result->Append(Brn(" remove "));
    result->Append(aValue);
    iResult.Result(result);
}



template <class T>
void ResultOrderedWatcher<T>::OrderedClose()
{
}



template <class T>
void ResultOrderedWatcher<T>::Dispose()
{
    iWatchable.RemoveWatcher(*this);
}

//////////////////////////////////////////////////////////////////////////////////


} // namespace Topology
} // namespace OpenHome
