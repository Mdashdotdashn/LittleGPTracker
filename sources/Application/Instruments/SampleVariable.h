
#ifndef _SAMPLE_VARIABLE_H_
#define _SAMPLE_VARIABLE_H_

#include "Foundation/Variables/WatchedVariable.h"
#include "Framework/Notifications/Observable.h"

class SampleVariable:public WatchedVariable,public Observer {
public:
	SampleVariable(const char *name,FourCC id) ;
	~SampleVariable() ;
protected:
    virtual void ObserverUpdate(Observable &o,ObservableData *d) ;

} ;
#endif