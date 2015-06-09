#ifndef _COMMAND_DISPATCHER_H_
#define _COMMAND_DISPATCHER_H_

#include "Framework/Instances/T_Singleton.h"
#include "Framework/Notifications/Observable.h"
#include "Framework/Containers/T_SimpleList.h"
#include "Foundation/Types/Types.h"
#include "Services/Controllers/ControlNode.h"
#include "NodeList.h"

class CommandExecuter {
public:
	CommandExecuter() {} ;
	virtual ~CommandExecuter() {} ;
	virtual void Execute(FourCC id,float value)=0 ;
} ;

class CommandTrigger:Observer {
public:
	CommandTrigger(FourCC id,CommandExecuter &executer) ;
	~CommandTrigger() ;
	void Attach(AssignableControlNode &node) ;
	void Detach() ;
protected:
	virtual void ObserverUpdate(Observable &o,ObservableData *d) ;	
private:
	AssignableControlNode *node_ ;
	int id_ ;
	CommandExecuter &executer_ ;
} ;

class CommandDispatcher:public T_Singleton<CommandDispatcher>,T_SimpleList<CommandTrigger>,CommandExecuter {
public:
	CommandDispatcher() ;
	virtual ~CommandDispatcher() ;
	bool Init() ;
	void Close() ;

	virtual void Execute(FourCC id,float value) ;

protected:
	void mapTrigger(FourCC trigger,const char *nodeUrl,CommandExecuter &executer) ; 
} ;
#endif
