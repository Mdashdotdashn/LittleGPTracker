

#include "PlayerMixer.h"
#include "SyncMaster.h"
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "Application/Utils/fixed.h"
#include "Application/Utils/char.h"
#include "Services/Midi/MidiService.h"
#include "Application/Mixer/MixerService.h"
#include "Application/Utils/char.h"
#include "Application/Model/Mixer.h"
#include <stdlib.h>
#include <math.h>

PlayerMixer::PlayerMixer() {

	for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
        lastInstrument_[i]=0 ;
	} ;

    for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
        channel_[i]=new PlayerChannel(i) ;
    }
}

bool PlayerMixer::Init(Project *project) {

	MixerService *ms=MixerService::Instance() ;
	if (!ms->Init()) {
			return false ;
	}

	AudioMixer *mixer=ms->GetMixBus(STREAM_MIX_BUS) ;
	mixer->Insert(fileStreamer_) ;

	project_=project ;

	// Init states

	for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
        lastInstrument_[i]=0 ;
	} ;

	clipped_=false ;
	return true ;
} ;

void PlayerMixer::Close()  {

	for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
		channel_[i]->Reset() ;
	}


	MixerService *ms=MixerService::Instance() ;
	ms->Close() ;

}

bool PlayerMixer::Start() {
	MixerService *ms=MixerService::Instance() ;
	ms->AddObserver(*this) ;

	for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
        notes_[i]=0xFF ;
    } ;

	return ms->Start() ;
} ;

void PlayerMixer::Stop() {
	MixerService *ms=MixerService::Instance() ;
	ms->Stop() ;
	ms->RemoveObserver(*this) ;
} ;

void PlayerMixer::StartChannel(int channel) {
	isChannelPlaying_[channel]=true ;
} ;

void PlayerMixer::StopChannel(int channel) {

    StopInstrument(channel) ;
	isChannelPlaying_[channel]=false ;
} ;


bool PlayerMixer::IsChannelPlaying(int channel) {
	return isChannelPlaying_[channel] ;
} ;

I_Instrument *PlayerMixer::GetLastInstrument(int channel) {
	return lastInstrument_[channel] ;
} ;


bool PlayerMixer::Clipped() {
     return clipped_ ;
}

void PlayerMixer::ObserverUpdate(Observable &o,ObservableData *d) {

  // Notifies the player so that pattern data is processed

     SetChanged() ;
     NotifyObservers() ;

  // Transfer the mixer data

	Mixer *mixer=Mixer::Instance() ;

	for (int i=0;i<SONG_CHANNEL_COUNT;i++) {
		channel_[i]->SetMixBus(mixer->GetBus(i)) ;
	}
//     out_->SetMasterVolume(project_->GetMasterVolume()) ;
	 MixerService *ms=MixerService::Instance() ;
     ms->SetMasterVolume(project_->GetMasterVolume()) ;
     clipped_=ms->Clipped() ;
} ;


void PlayerMixer::StartInstrument(int channel,I_Instrument *instrument,unsigned char note,bool newInstrument)  {
	channel_[channel]->StartInstrument(instrument,note,newInstrument) ;
	lastInstrument_[channel]=instrument ;
	notes_[channel]=note ;

} ;

void PlayerMixer::StopInstrument(int channel) {
    channel_[channel]->StopInstrument() ;
    notes_[channel]=0xFF ;
}

I_Instrument *PlayerMixer::GetInstrument(int channel) {
    return channel_[channel]->GetInstrument();
}

int PlayerMixer::GetPlayedBufferPercentage() {
	MixerService *ms=MixerService::Instance() ;
	return ms->GetPlayedBufferPercentage() ;
};

void PlayerMixer::SetChannelMute(int channel,bool mode) {
     channel_[channel]->SetMute(mode) ;
}

bool PlayerMixer::IsChannelMuted(int channel) {
     return channel_[channel]->IsMuted() ;
}

void PlayerMixer::StartStreaming(const Path &path) {
	fileStreamer_.Start(path) ;
} ;

void PlayerMixer::StopStreaming() {
	fileStreamer_.Stop() ;
} ;

void PlayerMixer::OnPlayerStart() {
	MixerService *ms=MixerService::Instance() ;
	ms->OnPlayerStart();
}

void PlayerMixer::OnPlayerStop() {
	MixerService *ms=MixerService::Instance() ;
	ms->OnPlayerStop();
}

static char noteBuffer[5] ;

int PlayerMixer::GetChannelNote(int channel) {
	return notes_[channel] ;
}

char *PlayerMixer::GetPlayedNote(int channel) {

    if (notes_[channel]!=0xFF) {
		note2visualizer(notes_[channel],noteBuffer) ; 
		return noteBuffer ;
    }
    return "  " ;
} ;

char *PlayerMixer::GetPlayedOctive(int channel) {
    if (notes_[channel]!=0xFF) {
		if (!IsChannelMuted(channel)) {
	        oct2visualizer(notes_[channel],noteBuffer) ; 
	        return noteBuffer ;
		} else {
			return "--" ;
		}
    }
    return "  " ;
} ;

AudioOut *PlayerMixer::GetAudioOut() {
	MixerService *ms=MixerService::Instance() ;
	return ms->GetAudioOut();
} ;

void PlayerMixer::Lock() {
	MixerService *ms=MixerService::Instance() ;
	ms->Lock() ;
} ;

void PlayerMixer::Unlock() {
	MixerService *ms=MixerService::Instance() ;
	ms->Unlock() ;

} ;