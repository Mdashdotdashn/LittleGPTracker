#ifndef _S60AUDIO_H_
#define _S60AUDIO_H_

#include "Services/Audio/Audio.h"

class S60Audio: public Audio {
public:
    S60Audio(AudioSettings &settings) ;
    ~S60Audio() ;
    virtual void Init() ;
    virtual void Close() ;
	virtual int GetSampleRate()  ;
private:
	int sampleRate_ ;
};
#endif
