#include "S60Audio.h"
#include "S60AudioDriver.h"
#include "Services/Audio/AudioOutDriver.h"
#include "System/Console/Trace.h"

S60Audio::S60Audio(AudioSettings &settings):Audio(settings),
	sampleRate_(44100) 
{
}

S60Audio::~S60Audio() {
}

void S60Audio::Init() {
     S60AudioDriver *drv=new S60AudioDriver(settings_) ;
     AudioOutDriver *out=new AudioOutDriver(*drv) ;
     Insert(out) ;
	 sampleRate_=drv->GetSampleRate() ;
} ;

void S60Audio::Close() {
     IteratorPtr<AudioOut>it(GetIterator()) ;
     for (it->Begin();!it->IsDone();it->Next()) {
         AudioOut &current=it->CurrentItem() ;
         current.Close() ;
     }
} ;

int S60Audio::GetSampleRate()  {
	return sampleRate_ ;
} ;
