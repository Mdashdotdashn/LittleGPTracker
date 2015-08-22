#include "S60AudioDriver.h"
#include <math.h>
#include "System/Console/Trace.h"
#include "System/System/System.h"
#include "Application/Model/Config.h"
#include <assert.h>

S60AudioDriver::S60AudioDriver(AudioSettings &settings):
    AudioDriver(settings), buffersize_(settings.bufferSize_){
    const char *delta = Config::GetInstance()->GetValue("S60SANDBAGDELTA");
    if (delta)
        sandbagdelta_ = atoi(delta);
    else
        sandbagdelta_ = 11500;  // works reliably on Nokia E63 w/ fw 500.x
}

S60AudioDriver::~S60AudioDriver() {
}

bool S60AudioDriver::InitDriver() {
    return true;
}

void S60AudioDriver::CloseDriver() {
} ;

bool S60AudioDriver::StartDriver() {
    thread_=new S60AudioDriverThread(this, bufferSize_, sandbagdelta_) ;
    thread_->Start() ;
    return true ;
} ;

void S60AudioDriver::StopDriver() {
    thread_->RequestTermination();
} ;

double S60AudioDriverThread::GetStreamTime() {
    return streamTime_;
}

double S60AudioDriver::GetStreamTime() {
    if (!thread_)
        return 0;
    return thread_->GetStreamTime() ;
} ;

bool S60AudioDriverThread::Execute() {
    _LIT(KThreadName,"lgpt_audio_thread");
    TPtrC threadname(KThreadName);
    User::RenameThread(threadname);

    RProcess cur_process;
    cur_process.SetPriority(EPriorityHigh);
    RThread cur_thread;
    cur_thread.SetPriority(EPriorityAbsoluteHigh);
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
    terminating = true;
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
    caps.iBufferSize = buffersize_;
    TRAP(err, dev->SetConfigL(caps));
    samples_pushed = 0;
    TRAP(err, dev->PlayInitL());
} ;

int S60AudioDriver::FillBuffer(CMMFBuffer *aBuffer) {
    AudioBufferData *abd = &(pool_[poolPlayPosition_]);
    if (!abd->size_)
        OnNewBufferNeeded();

    CMMFDataBuffer *buf = static_cast<CMMFDataBuffer*>(aBuffer);
    TDes8 &output = buf->Data();
    output.Copy((TUint8*)abd->buffer_, abd->size_);
    return abd->size_ / 4;
}

void S60AudioDriver::NextBuffer() {
    SAFE_FREE(pool_[poolPlayPosition_].buffer_);
    poolPlayPosition_=(poolPlayPosition_+1)%SOUND_BUFFER_COUNT;
    onAudioBufferTick();
    OnNewBufferNeeded();
}

void S60AudioDriverThread::BufferToBeFilled(CMMFBuffer *aBuffer) {
    if (terminating) {
        dev->Stop();
        CActiveScheduler::Stop();
        return;
    }

    samples_pushed += driver->FillBuffer(aBuffer);

    // This is a filthy hack to reduce the amount of buffering (which we can't control) supplied by the OS.
    // Once the output sample counter comes up, we start sandbagging to let that buffer drain.
    // This is a terrible magic number.
    if (sandbagdelta_ && dev->SamplesPlayed()) {
        while (dev->SamplesPlayed() < (samples_pushed - sandbagdelta_)) {
            User::After(100);
        }
    }

    TInt err;
    TRAP(err, dev->PlayData());
    User::After(100);   // yield
    streamTime_ = dev->SamplesPlayed() / 44100.;
    driver->NextBuffer();
}

void S60AudioDriverThread::PlayError(TInt aError) {
}
