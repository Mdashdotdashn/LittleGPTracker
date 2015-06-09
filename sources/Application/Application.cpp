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
  const char *preferedDevice=Config::Instance()->GetValue("MIDICTRLDEVICE");

  IteratorPtr<MidiInDevice>it(MidiService::Instance()->GetInIterator()) ;
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

bool Application::Init(GUICreateWindowParams &params) {
	const char* root=Config::Instance()->GetValue("ROOTFOLDER") ;
	if (root) {
		Path::SetAlias("root",root) ;
	} ;
	window_=AppWindow::Create(params) ;
	PersistencyService::Instance() ;
  Audio *audio=Audio::Instance() ;
  audio->Init() ;
	CommandDispatcher::Instance()->Init() ;
  initMidiInput();
	ControlRoom::Instance()->LoadMapping("bin:mapping.xml") ;
	return true ;
} ;

GUIWindow *Application::GetWindow() {
	return window_ ;
} ;

Application::~Application() {
	delete window_ ;
}
