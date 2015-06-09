
#ifndef _OBSERVABLE_H_
#define _OBSERVABLE_H_

#include "Framework/Containers/T_SimpleList.h"
#include <vector>
#include <map>

// Data to be passed from the observable to the observer

class ObservableData {
public:
	virtual ~ObservableData() {}
} ;

// FWD>> Declaration

class Observable ;

// Callback mechanism

class __ObservableCallBackExecutable {
public:
    virtual ~__ObservableCallBackExecutable() {};
	virtual void Execute(Observable &o,ObservableData *d)=0 ;
} ;


// The observer: Simply allows to be notified with data

class Observer {
public:
	virtual ~Observer() {} ;
    virtual void ObserverUpdate(Observable &o,ObservableData *d)=0 ;
} ;

// The observable

class Observable {
public:
	Observable() ;
	virtual ~Observable() ;

	void AddObserver(Observer &o) ;
	void RemoveObserver(Observer &o) ;
	void RemoveAllObservers() ;
	int  CountObservers() ;

	void AddCallback(__ObservableCallBackExecutable *) ;
	void RemoveCallback(__ObservableCallBackExecutable *) ;

	void NotifyObservers() ;
	void NotifyObservers(ObservableData *d) ;

	void SetChanged() ;
	void ClearChanged() ;
	bool HasChanged() ;
private:
	T_SimpleList<Observer> *_list ;
	bool _hasChanged ;
	T_SimpleList<__ObservableCallBackExecutable> callbacks_ ;
} ;

#endif

