
#include "Observable.h"
#include "System/Console/Trace.h"

Observable::Observable() {
	_list=new T_SimpleList<Observer>() ;
	_hasChanged=false ;
}

Observable::~Observable() {

	if (_list->Size()!=0) {
		Trace::Debug("ERROR: destroying an observable that is still observed") ;
		NAssert(0) ;
	} ;
	if (callbacks_.Size()!=0) {
		Trace::Debug("ERROR: destroying an observable that has still callbacks") ;
		NAssert(0) ;
	}
	delete _list ;
}

void Observable::AddObserver(Observer &o) {
	_list->Insert(o) ;
}

void Observable::RemoveObserver(Observer &o) {
	_list->Remove(o) ;
}

void Observable::AddCallback(__ObservableCallBackExecutable *cbe) {
	callbacks_.Insert(cbe) ;
} ;

void Observable::RemoveCallback(__ObservableCallBackExecutable *cbe) {
	callbacks_.Remove(*cbe) ;
} ;

void Observable::RemoveAllObservers() {
	_list->Empty() ;
}

void Observable::NotifyObservers() {
	this->NotifyObservers(NULL) ;
}

void Observable::NotifyObservers(ObservableData *d) {
	if (_hasChanged) {
		IteratorPtr<Observer> i(_list->GetIterator()) ;
		for (i->Begin();!i->IsDone();i->Next()) {
			Observer &o=i->CurrentItem() ;
			o.ObserverUpdate(*this,d) ;
		};
		IteratorPtr<__ObservableCallBackExecutable> i2(callbacks_.GetIterator()) ;
		for (i2->Begin();!i2->IsDone();i2->Next()) {
			__ObservableCallBackExecutable &ocbe=i2->CurrentItem() ;
			ocbe.Execute(*this,d) ;
		};
		ClearChanged() ;
	}
}

void Observable::SetChanged() {
	_hasChanged=true ;
}

void Observable::ClearChanged() {
	_hasChanged=false ;
}

bool Observable::HasChanged() {
	return _hasChanged ;
}
