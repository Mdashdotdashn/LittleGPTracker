
#include "GP2XAudio.h"
#include "GP2XAudioDriver.h"
#include "Services/Audio/AudioOutDriver.h"

GP2XAudio::GP2XAudio(AudioSettings &settings):Audio(settings) {
}

GP2XAudio::~GP2XAudio() 
{
}

void GP2XAudio::Init() 
{
	AudioSettings settings ;
  Trace::Log("RANDOM", "Creating audio driver");
     GP2XAudioDriver *drv=new GP2XAudioDriver(settings) ;
  Trace::Log("RANDOM", "Creating audio out");
     AudioOutDriver *out=new AudioOutDriver(*drv) ;
     Insert(out) ;
}

void GP2XAudio::Close() 
{
     IteratorPtr<AudioOut>it(GetIterator()) ;
     for (it->Begin();!it->IsDone();it->Next()) {
         AudioOut &current=it->CurrentItem() ;
         current.Close() ;
     }
}

void GP2XAudio::SetMixerVolume(int v) 
{
     AudioOutDriver *out=(AudioOutDriver*)GetFirst();
     if (out) {
          GP2XAudioDriver *drv=(GP2XAudioDriver *)out->GetDriver() ;
          drv->SetVolume(v);
     }    
}     

int GP2XAudio::GetMixerVolume() 
{
     AudioOutDriver *out=(AudioOutDriver*)GetFirst();
     if (out) {
          GP2XAudioDriver *drv=(GP2XAudioDriver *)out->GetDriver() ;
          return drv->GetVolume();
     }    
     return 0;
}    
