#ifndef _UI_TEMPOFIELD_H_
#define _UI_TEMPOFIELD_H_

#include "UIIntVarField.h"
#include "Framework/Notifications/Observable.h"

class UITempoField: public UIIntVarField,public Observable,public Observer {
public:
	UITempoField(FourCC action,GUIPoint &position,Variable &variable,const char *format,int min,int max,int xOffset,int yOffset) ;
	virtual void OnBClick() ;
	virtual void ObserverUpdate(Observable &,ObservableData *) ;
	void ProcessArrow(unsigned short mask) ;
	void ProcessBArrow(unsigned short mask) ;

private:
	FourCC action_ ;


} ;
#endif
