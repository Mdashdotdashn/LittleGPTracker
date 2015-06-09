#ifndef _PROJECT_VIEW_H_
#define _PROJECT_VIEW_H_

#include "BaseClasses/FieldView.h"
#include "Framework/Notifications/Observable.h"
#include "ViewData.h"


class ProjectView: public FieldView,public Observer {
public:
	ProjectView(GUIWindow &w,ViewData *data) ;
	virtual ~ProjectView() ;

	virtual void ProcessButtonMask(unsigned short mask,bool pressed) ;
	virtual void DrawView() ;
	virtual void OnPlayerUpdate(PlayerEventType,unsigned int) {} ;
	virtual void OnFocus() {} ;

	// Observer for action callback

	virtual void ObserverUpdate(Observable &,ObservableData *) ;

	void OnLoadProject() ;
	void OnPurgeInstruments(bool removeFromDisk) ;
	void OnQuit() ;

protected:
private:
	Project *project_ ;
// Debug
	unsigned long lastTick_ ;
	unsigned long lastClock_ ;
	UIField *tempoField_ ;
} ;
#endif
