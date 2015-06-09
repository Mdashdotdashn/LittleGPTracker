
#ifndef _MULTI_CHANNEL_ADAPTER_H_
#define _MULTI_CHANNEL_ADAPTER_H_

#include "Services/Controllers/Channel.h"
#include "Framework/Containers/T_SimpleList.h"
#include "Framework/Notifications/Observable.h"

class MultiChannelAdapter: T_SimpleList<Channel>,public Observer, public Channel {
public:
	MultiChannelAdapter(const char *name, bool owner=false) ;
	virtual ~MultiChannelAdapter() ;
	bool AddChannel(const char *decription) ;
	void AddChannel(Channel &) ;
	virtual void SetValue(float value,bool notify=true) ;

private:
    virtual void ObserverUpdate(Observable &o,ObservableData *d) ;
} ;
#endif

