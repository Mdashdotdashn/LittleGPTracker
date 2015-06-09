#ifndef _EVENT_DISPATCHER_H_
#define _EVENT_DISPATCHER_H_

#include "CommandDispatcher.h"
#include "Framework/Instances/T_Singleton.h"
#include "Application/AppWindow.h"
#include "System/Timer/Timer.h"
#include "Framework/Notifications/Observable.h"

class EventDispatcher: public T_Singleton<EventDispatcher>,public CommandExecuter,public Observer {
public:
	EventDispatcher() ;
	~EventDispatcher() ;
	void SetWindow(GUIWindow *window) ;
	virtual void Execute(FourCC id,float value) ;
	unsigned int OnTimerTick() ;
	int GetEventMask() { return eventMask_ ; } ;
    virtual void ObserverUpdate(Observable &o,ObservableData *d) ;
private:
	GUIWindow *window_ ;	
	static int keyRepeat_ ;
	static int keyDelay_ ;
	unsigned int eventMask_ ;
	unsigned int repeatMask_ ;
	I_Timer *timer_ ;
} ;

#endif