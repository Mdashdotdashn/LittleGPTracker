#ifndef _S60_SOUND_H_
#define _S60_SOUND_H_

#include "Services/Audio/AudioDriver.h"
#include "System/Process/Process.h"
#include "Foundation/T_Stack.h"
#define Time E32Time
#include <mmf/server/sounddevice.h>
#undef Time

class S60AudioDriver;

class S60AudioDriverThread: public SysThread, MDevSoundObserver {
public:
    S60AudioDriverThread(S60AudioDriver *driver, int buffersize, int sandbagdelta)
            : driver(driver),
              terminating(false),
              buffersize_(buffersize),
              sandbagdelta_(sandbagdelta) {};
    virtual ~S60AudioDriverThread() {} ;
    virtual bool Execute() ;
    void RequestTermination() ;
    double GetStreamTime() ;

public: // From MDevSoundObserver
   void InitializeComplete(TInt aError);
   void ToneFinished(TInt aError) {};
   void BufferToBeFilled(CMMFBuffer* aBuffer);
   void PlayError(TInt aError);
   void BufferToBeEmptied(CMMFBuffer *aBuffer) {};
   void RecordError(TInt aError) {};
   void ConvertError(TInt aError) {};
   void DeviceMessage(TUid aMessageType, const TDesC8 &aMsg) {};

private:
    CMMFDevSound *dev;
    S60AudioDriver *driver;
    double streamTime_ ;
    int samples_pushed;
    bool terminating;
    int buffersize_, sandbagdelta_;
} ;

class S60AudioDriver: public AudioDriver {
public:
    S60AudioDriver(AudioSettings &settings) ;
    ~S60AudioDriver() ;
     // Audio implementation
    virtual bool InitDriver() ;
    virtual void CloseDriver();
    virtual bool StartDriver() ;
    virtual void StopDriver();
    virtual int GetPlayedBufferPercentage() { return 0 ;} ;
    virtual int GetSampleRate() { return 44100; } ;
    virtual bool Interlaced() { return true ; } ;
    virtual double GetStreamTime() ;

    int FillBuffer(CMMFBuffer *aBuffer);
    void NextBuffer();
private:
    S60AudioDriverThread * thread_ ;
    int buffersize_, sandbagdelta_;
} ;
#endif
