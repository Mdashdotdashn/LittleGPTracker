#pragma once

#include "Observable.h"

// ObserverCallback Mechanism

template < class Class>
class ObservableCallback:public __ObservableCallBackExecutable
{

   public:

    typedef void (Class::*Method)(Observable &o,ObservableData *d);

    ObservableCallback(Class* _class_instance, Method _method)
    {
       class_instance = _class_instance;
       method         = _method;
    };

    void operator()(Observable &o,ObservableData *d)
    {
       return (class_instance->*method)(o,d);
    };

    virtual void Execute(Observable &o,ObservableData *d)
    {
       return operator()(o,d);
    };

    private:

      Class*  class_instance;
      Method  method;

};

// Callback observer: derive from this to allow specific method to be called

typedef std::map<Observable *,__ObservableCallBackExecutable *> CallbackMap ;

template < class Class > 

class T_CallbackObserver {
public:
	typedef void (Class::*Method)(Observable &o,ObservableData *d);
	typedef ObservableCallback<Class> COC ;

	T_CallbackObserver() {} ;

	virtual ~T_CallbackObserver() {} ;

	void RegisterCallback(Observable &o, Method method) {

		// let's see if the observable is watched already

		CallbackMap::iterator it=callbacks_.find(&o) ;
		if(it!=callbacks_.end()){
			NAssert(0) ;
			return ;
		} ;

		__ObservableCallBackExecutable *cb=new COC((Class *)this,method) ;
		o.AddCallback(cb) ;

		callbacks_[&o]=cb ;
	} ;

	void UnregisterCallback(Observable &o) {
		CallbackMap::iterator it=callbacks_.find(&o) ;
		if(it==callbacks_.end()){
			NAssert(0) ;
			return ;
		}
		o.RemoveCallback(it->second) ;
		callbacks_.erase(it) ;
	} ;

private:
	CallbackMap callbacks_ ;
} ;
