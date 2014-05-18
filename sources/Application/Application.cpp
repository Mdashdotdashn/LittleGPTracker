#include "Application/Application.h" 
#include "Application/AppWindow.h" 
#include "UIFramework/Interfaces/I_GUIWindowFactory.h"
#include "Application/Persistency/PersistencyService.h" 
#include "Services/Audio/Audio.h"
#include "Application/Commands/CommandDispatcher.h"
#include "Application/Controllers/ControlRoom.h"
#include "Application/Model/Config.h"
#include "Services/Midi/MidiService.h"

#include <math.h>

Application *Application::instance_=NULL ;

Application::Application() {
}

void Application::initMidiInput()
{
  const char *preferedDevice=Config::GetInstance()->GetValue("MIDICTRLDEVICE");

  IteratorPtr<MidiInDevice>it(MidiService::GetInstance()->GetInIterator()) ;
  for(it->Begin();!it->IsDone();it->Next())
  {
    MidiInDevice &in=it->CurrentItem() ;
    if ((preferedDevice) && (!strncmp(in.GetName(), preferedDevice, strlen(preferedDevice))))
    {
      if (in.Init())
      {
        if (in.Start())
        {
          Trace::Log("MIDI","Controlling activated for MIDI interface %s",in.GetName()) ;
        }
        else
        {
          in.Close() ;
        }
      }
    }
  }
}

bool Application::Init(GUICreateWindowParams &params) 
{
  Trace::Log("RANDOM", "reading root");
  
	const char* root=Config::GetInstance()->GetValue("ROOTFOLDER") ;
	if (root) {
		Path::SetAlias("root",root) ;
	} ;
  Trace::Log("RANDOM", "creating window");
	window_=AppWindow::Create(params) ;
  Trace::Log("RANDOM", "init persistency");
	PersistencyService::GetInstance() ;
  Trace::Log("RANDOM", "init audio");  
  Audio *audio=Audio::GetInstance() ;
  audio->Init() ;
  Trace::Log("RANDOM", "init command dispatcher");
	CommandDispatcher::GetInstance()->Init() ;
  Trace::Log("RANDOM", "init midi input");
  initMidiInput();
  Trace::Log("RANDOM", "init mapping");
	ControlRoom::GetInstance()->LoadMapping("bin:mapping.xml") ;
  Trace::Log("RANDOM", "init done");
	return true ;
} ;

GUIWindow *Application::GetWindow() {
	return window_ ;
} ;

Application::~Application() {
	delete window_ ;
}
