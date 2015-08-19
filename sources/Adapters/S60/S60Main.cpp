#include "Application/Application.h"
#include "Adapters/S60/System/S60System.h"
#include "Foundation/T_Singleton.h"
#include <SDL/SDL.h>
#include <string.h>
#include "Adapters/SDL/GUI/SDLGUIWindowImp.h"
#include "Application/Persistency/PersistencyService.h" 
#include "Adapters/SDL/GUI/SDLGUIWindowImp.h"

int main(int argc,char *argv[]) 
{
  S60System::Boot(argc,argv) ;

  SDLCreateWindowParams params ;
  params.title="littlegptracker" ;
  params.cacheFonts_=true ;
  params.framebuffer_=false ;

  Application::GetInstance()->Init(params) ;

  S60System::MainLoop() ;
  S60System::Shutdown() ; 
}



