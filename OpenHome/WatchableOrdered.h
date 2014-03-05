#ifndef HEADER_WATCHABLE_ORDERED
#define HEADER_WATCHABLE_ORDERED

#include <OpenHome/IWatchable.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/OhTopologyC.h>
#include<vector>



namespace OpenHome
{
namespace Av
{



/**
    \ingroup watchable
    @{
 */
template <class T>
class WatchableOrdered : public IWatchableOrdered<T>, public WatchableBase, public IDisposable
{
public:
    WatchableOrdered(IWatchableThread& aWatchableThread);

    virtual void Add(T& aValue, TUint aIndex);
    virtual void Move(T& aValue, TUint aIndex);
    virtual void Remove(T& aValue);
    virtual void Clear();

    // IWatchableOrdered<T>
    virtual void AddWatcher(IWatcherOrdered<T>& aWatcher);
    virtual void RemoveWatcher(IWatcherOrdered<T>& aWatcher);

    // IDisposable
    virtual void Dispose();

private:
    void DisposeCallback(void*);

private:
    std::vector<T> iWatchables;
    std::vector<IWatcherOrdered<T>*> iWatchers;
};


/**
    Constructor

    @param[in] aWatchableThread   Reference to a watchable thread object
 */
template <class T>
WatchableOrdered<T>::WatchableOrdered(IWatchableThread& aWatchableThread)
    :WatchableBase(aWatchableThread)
{
}


/**
    Add a watchable to a specified position in the list of watchables

    @param[in] aWatchable   Reference to the watchable being added
    @param[in] aIndex  Integer index where watchable is to be added
 */
template <class T>
void WatchableOrdered<T>::Add(T& aWatchable, TUint aIndex)
{
    WatchableBase::Assert(); /// must be on watchable thread

    iWatchables.insert(iWatchables.begin()+aIndex, aWatchable); /// insert aWatchable into iWatchables

    std::vector<IWatcherOrdered<T>*> watchers(iWatchers); /// get snapshot of iWatchers (can't mutex when calling out to unknown code)

    for(TUint i=0; i<watchers.size(); i++ )
    {
         watchers[i]->OrderedAdd(aWatchable, aIndex); /// add aWatchable to every watcher's list
    }
}


/**
    Move an existing watchable to a new index in the list of watchables

    @param[in] aWatchable   Reference to the watchable being moved
    @param[in] aNewIndex  Integer index where watchable is to be moved
 */
template <class T>
void WatchableOrdered<T>::Move(T& aWatchable, TUint aNewIndex)
{
    WatchableBase::Assert(); /// must be on watchable thread

    typename std::vector<IWatcherOrdered<T>*>::iterator it = std::find(iWatchables.begin(), iWatchables.end(), &aWatchable);
    ASSERT(it != iWatchables.end()); /// aWatchable must exist in iWatchables

    TUint oldIndex = it-iWatchables.begin();
    iWatchables.erase(it); /// remove aWatchable from iWatchables
    iWatchables.insert(iWatchables.begin()+aNewIndex, aWatchable); /// insert aWatchable at aIndex in iWatchables

    std::vector<IWatcherOrdered<T>*> watchers(iWatchers); /// get snapshot of iWatchers (can't mutex when calling out to unknown code)

    for(TUint i=0; i<watchers.size(); i++ )
    {
         watchers[i]->OrderedMove(aWatchable, oldIndex, aNewIndex); /// move aWatchable in every watcher's list
    }
}


/**
    Remove an existing watchable from the list of watchables

    @param[in] aWatchable   Reference to the watchable being removed
 */
template <class T>
void WatchableOrdered<T>::Remove(T& aWatchable)
{
    WatchableBase::Assert(); /// must be on watchable thread

    typename std::vector<IWatcherOrdered<T>*>::iterator it = std::find(iWatchables.begin(), iWatchables.end(), &aWatchable);
    ASSERT(it != iWatchables.end()); /// aWatchable must exist in iWatchables

    TUint index = it-iWatchables.begin();
    iWatchables.erase(it); /// remove aWatchable from iWatchables

    std::vector<IWatcherUnordered<T>*> watchers(iWatchers); /// get snapshot (can't mutex when calling out to unknown code)
    for(TUint i=0; i<watchers.size(); i++ )
    {
        watchers[i]->OrderedRemove(aWatchable, index); /// remove aWatchable from every watcher's list
    }
}


/**
    Clear the list of watchables

 */
template <class T>
void WatchableOrdered<T>::Clear()
{
    WatchableBase::Assert(); /// must be on watchable thread

    std::vector<IWatcherUnordered<T>*> watchers(iWatchers); /// get snapshot (can't mutex when calling out to unknown code)
    std::vector<T*> watchables(iWatchables); /// get snapshot (can't mutex when calling out to unknown code)
    iWatchables.clear(); /// clear list of watchables, iWatchables

    for(TUint i=0; i<watchers.size(); i++ )
    {
        for(TUint x=0; x<watchables.size(); x++ )
        {
            watchers[i]->OrderedRemove(*(watchables[x]), x); /// remove every watchable from every watcher's list
        }
    }
}


/**
    Add a watcher to the list of watchers

    @param[in] aWatcher   Reference to the watcher being added
 */
template <class T>
void WatchableOrdered<T>::AddWatcher(IWatcherOrdered<T>& aWatcher)
{
    WatchableBase::Assert(); /// must be on watchable thread
    iWatchers.push_back(&aWatcher); /// add aWatcher to iWatchers
    aWatcher.OrderedOpen(); /// set aWatcher status to Open

    for(TUint i=0; i<iWatchables.size(); i++)
    {
        aWatcher.OrderedAdd(*(iWatchables[i]), i); /// add all watchables to aWatcher
    }

    aWatcher.OrderedInitialised(); /// set aWatcher status to Initialised
}


/**
    Remove a Watcher from the list of watchers

    @param[in] aWatcher   Reference to the watcher being removed
 */
template <class T>
void WatchableOrdered<T>::RemoveWatcher(IWatcherOrdered<T>& aWatcher)
{
    WatchableBase::Assert(); /// must be on watchable thread

    typename std::vector<T>::iterator it = std::find(iWatchers.begin(), iWatchers.end(), &aWatcher);

    if (it != iWatchers.end()) /// check aWatcher exists
    {
        iWatchers.erase(it); /// remove aWatcher from iWatchers
    }

    aWatcher.OrderedClose(); /// set aWatcher status to Closed
}


template <class T>
void WatchableOrdered<T>::Dispose()
{
    FunctorGeneric<void*> action = MakeFunctorGeneric(*this, &WatchableOrdered::DisposeCallback);
    WatchableBase::Execute(action);
}


template <class T>
void WatchableOrdered<T>::DisposeCallback(void*)
{
    ASSERT(iWatchers.size() == 0);
}






/**
    @}
 */


}  // Av
}  // OpenHome




#endif // HEADER_WATCHABLE_ORDERED
