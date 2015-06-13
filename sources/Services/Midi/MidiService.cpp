#include "MidiService.h"
#include "Application/Player/SyncMaster.h"
#include "System/Console/Trace.h"
#include "System/Timer/Timer.h"
#include "Application/Model/Config.h"
#include "Services/Audio/AudioDriver.h"

#ifdef SendMessage
#undef SendMessage
#endif
	
MidiService::MidiService():
	T_SimpleList<MidiOutDevice>(true),
  inList_(true),
	device_(0),
	sendSync_(true),
  tickToFlush_(0)
{
	const char *delay = Config::GetInstance()->GetValue("MIDIDELAY") ;
  midiDelay_ = delay?atoi(delay):1 ;
  
	const char *sendSync = Config::GetInstance()->GetValue("MIDISENDSYNC") ;
	if (sendSync)
  {
		sendSync_ = (strcmp(sendSync,"YES")==0) ;
	}
}

MidiService::~MidiService()
{
	Close() ;
} ;

bool MidiService::Init()
{
	Empty() ;
  inList_.Empty();
	buildDriverList() ;
	// Add a merger for the input
	merger_=new MidiInMerger() ;
	IteratorPtr<MidiInDevice>it(inList_.GetIterator());
	for  (it->Begin();!it->IsDone();it->Next())
  {
		MidiInDevice &current=it->CurrentItem();
		merger_->Insert(current) ;
	}

	return true ;
}

void MidiService::Close()
{
	Stop();
}

I_Iterator<MidiInDevice> *MidiService::GetInIterator()
{
	return inList_.GetIterator() ;
}

void MidiService::SelectDevice(const std::string &name)
{
	deviceName_=name ;
}

bool MidiService::Start()
{
	return true ;
}


void MidiService::Stop()
{
	stopDevice() ;
}

// Adds a message in the current playback queue

void MidiService::QueueMessage(MidiMessage &m)
{
  if (device_)
  {
    SysMutexLocker lock(queueMutex_);
    NAssert(!messageQueue_.empty());
    messageQueue_.back().push_back(MidiMessage(m.status_,m.data1_,m.data2_));
  }
}

// Trigger is called whenever the player is starting a new slice
// This means we need to advance the queue for the midi messages associated to the queu
// And then queue a MIDI clock message

void MidiService::Trigger()
{
  advancePlayQueue();
  
	if (device_ && sendSync_)
  {
		SyncMaster *sm=SyncMaster::GetInstance() ;
		if (sm->MidiSlice()) {
			MidiMessage msg;
			msg.status_ = 0xF8;
			QueueMessage(msg);
		}
	}
}

void MidiService::advancePlayQueue()
{
  SysMutexLocker lock(queueMutex_);
  messageQueue_.push_back(std::vector<MidiMessage>());
}

//
// Flush is called everytime a new audio buffer slice is copied for sending to
// the audio card. If a midi delay is set, we don't send the buffer right away but
// wait audio driver ticks to trigger the real flush

void MidiService::Flush()
{
  tickToFlush_ = midiDelay_ ;
  if (tickToFlush_ == 0)
  {
    flushOutQueue();
  }
}

// Observable callback for when a new buffer has been send to the audio
// card. The timing of this depends on the audio card buffer size and is not
// in sync with the triggers call. If  midiDelay_ has been set, we didn't send
// the midi buffer straight away but we'll count down tick until we can send it

void MidiService::Update(Observable &o,I_ObservableData *d)
{
  AudioDriver::Event *event=(AudioDriver::Event *)d;
  if (event->type_ == AudioDriver::Event::ADET_DRIVERTICK)
  {  
    onAudioTick();
  }
}

void MidiService::onAudioTick()
{
  if (tickToFlush_>0)
  {
    if (--tickToFlush_ ==0)
    {
      flushOutQueue();
    }
  }
}

void MidiService::flushOutQueue()
{
  SysMutexLocker lock(queueMutex_);
  NAssert(!messageQueue_.empty());
  std::vector<MidiMessage> &queue = messageQueue_.front();
  
	if (device_)
  {
    // Send whatever is on the out queue
		device_->SendQueue(queue) ;
	}
  
  messageQueue_.pop_front();
}

void MidiService::startDevice() {

	// look for the device

	IteratorPtr<MidiOutDevice>it(GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		MidiOutDevice &current=it->CurrentItem() ;
		if (!strcmp(deviceName_.c_str(),current.GetName())) {
			if (current.Init()) {
				if (current.Start()) {
					device_=&current ;
				} else {
					current.Close() ;
				}
			}
			break ;
		}
	}

} ;

void MidiService::stopDevice() {
	if (device_) {
		device_->Stop() ;
		device_->Close() ;
	}
	device_=0 ;
} ;

void MidiService::OnPlayerStart() {

	if (deviceName_.size()!=0) {
		stopDevice() ;
		startDevice() ;
		deviceName_="" ;
	}

	if (sendSync_)
  {
		MidiMessage msg ;
		msg.status_=0xFA ;
		QueueMessage(msg) ;

	}
};

void MidiService::OnPlayerStop() {

	if (sendSync_) 
  {
		MidiMessage msg ;
		msg.status_=0xFC ;
		QueueMessage(msg) ;
	}
} ;
