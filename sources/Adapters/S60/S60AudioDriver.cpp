#include "S60AudioDriver.h"
#include <math.h>
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "Services/Midi/MidiService.h"
#include <assert.h>

S60AudioDriver::S60AudioDriver(AudioSettings &settings):AudioDriver(settings) {
}

S60AudioDriver::~S60AudioDriver() {
}

bool S60AudioDriver::InitDriver() {
    return true;
}    

void S60AudioDriver::CloseDriver() {

} ;

bool S60AudioDriver::StartDriver() {
    streamTime_=0 ;
    thread_=new S60AudioDriverThread(this) ;
    thread_->Start() ;
    return true ;
} ; 

void S60AudioDriver::StopDriver() {
} ;

double S60AudioDriver::GetStreamTime() {
    return streamTime_ ;
} ;

bool S60AudioDriverThread::Execute() {
    CActiveScheduler* activeScheduler = new (ELeave) CActiveScheduler;
    CleanupStack::PushL(activeScheduler);
    CActiveScheduler::Install(activeScheduler);
    dev = CMMFDevSound::NewL();
    dev->InitializeL(*this, KMMFFourCCCodePCM16, EMMFStatePlaying);
    CActiveScheduler::Start();
    CleanupStack::PopAndDestroy(activeScheduler);
    return true;
}

void S60AudioDriverThread::RequestTermination() {
}

void S60AudioDriverThread::InitializeComplete(TInt aError) {
    TInt err;
    TMMFPrioritySettings pri;
    pri.iPref = EMdaPriorityPreferenceTimeAndQuality;
    pri.iPriority = EMdaPriorityMax;
    TRAP(err, dev->SetPrioritySettings(pri));
    TMMFCapabilities caps = dev->Config();
    caps.iChannels = EMMFStereo;
    caps.iEncoding = EMMFSoundEncoding16BitPCM;
    caps.iRate = EMMFSampleRate44100Hz;
    caps.iBufferSize = 0;
    TRAP(err, dev->SetConfigL(caps));
    TRAP(err, dev->PlayInitL());
} ; 

void S60AudioDriver::FillBuffer(CMMFBuffer *aBuffer, FILE *fp) {
    CMMFDataBuffer *buf = static_cast<CMMFDataBuffer*>(aBuffer);
    TDes8 &output = buf->Data();
    AudioBufferData *abd = &(pool_[poolPlayPosition_]);
    if (!abd->size_) {
        OnNewBufferNeeded();
        while (!abd->size_);    // XXX spinning!
    }
    output.Copy((TUint8*)abd->buffer_, abd->size_);
    streamTime_ += abd->size_ / (2*44100);
    SAFE_FREE(pool_[poolPlayPosition_].buffer_);
    poolPlayPosition_=(poolPlayPosition_+1)%SOUND_BUFFER_COUNT;
    output.SetLength(abd->size_);
    
    onAudioBufferTick();
    OnNewBufferNeeded();
}

void S60AudioDriverThread::BufferToBeFilled(CMMFBuffer *aBuffer) {
    driver->FillBuffer(aBuffer, fp);
    dev->EmptyBuffers();
    TInt err;
    TRAP(err, dev->PlayData());
}

void S60AudioDriverThread::PlayError(TInt aError) {
}
