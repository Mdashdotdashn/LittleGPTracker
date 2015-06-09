/*
 *  MIDIFacade.cpp
 *  lgptx
 *
 *  Created by Marc Nostromo on 17/08/13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */

#include "MIDIFacade.h"
#include "Services/Config/Config.h"
#include "Services/Midi/MidiService.h"
#include "System/Console/Trace.h"

#ifdef SendMessage // No idea while the windows definition still gets in the way here
#undef SendMessage
#endif

MidiFacade::MidiFacade()
  : selectedDevice_(NULL)
  , playerChangedState_(None)
{  
}

Result MidiFacade::Init()
{
  sendSync_ = Config::Instance()->GetBool("MIDISENDSYNC", true);
  
  Result result = senderThread_.Init(queue_);  
  RETURN_IF_FAILED_MESSAGE(result, "failed to init sender thread");
  return senderThread_.Start();
}

void MidiFacade::Close()
{
}

void MidiFacade::OnPlayerStart()
{
  playerChangedState_ = Started;
}

void MidiFacade::OnPlayerStop()
{
  playerChangedState_ = Stopped;  
}

void MidiFacade::OnPlayerTick()
{ 
  SystemTick now = TimeService::Instance()->GetSystemTick();
//  Trace::Debug("MidiFacade::OnplayerTick now %s",STickToString(now).c_str());
               
  SystemTick predictedSystemTick = 
    MixerService::Instance()->PredictCurrentSliceTimeInSystemTick();
  
//  Trace::Debug("MidiFacade::OnplayerTick [%f >> %s] ahead of %s", 
//               beatTime.GetValue(), 
//               STickToString(predictedSystemTick).c_str(),
//               STickToString(predictedSystemTick-now).c_str());

  if (playerChangedState_ == Started)
  {
    queue_.Reset();    
  }
  
  queue_.PrepareFor(predictedSystemTick);

  if (sendSync_)
  {
    switch (playerChangedState_)
    {
      case Started:
      {
        MidiMessage msg ;
        msg.status_= MidiMessage::TIMING_START ;
        QueueMessage(msg) ;
        break;        
      }
      case Stopped:
      {
        MidiMessage msg;
        msg.status_ = MidiMessage::TIMING_STOP;
        QueueMessage(msg);
        break;                
      }
    }    
  }
  
  playerChangedState_ = None;

  if (sendSync_) 
  {
		MidiMessage msg ;
		msg.status_= MidiMessage::TIMING_CLOCK ;
		QueueMessage(msg) ;
	}  
}


const std::vector<std::string>& MidiFacade::DeviceList()
{
  return deviceList_;
}

bool MidiFacade::SelectDevice(const std::string& deviceName)
{
  closeSelectedDevice();
  
  IteratorPtr<MidiOutDevice> it(MidiService::Instance()->GetOutIterator()) ;
  for (it->Begin(); !it->IsDone(); it->Next())
  {
    MidiOutDevice& current = it->CurrentItem();
    if (current.GetName() == deviceName)
    {
      return selectDevice(current);
    }
  }
  return false;
}

int MidiFacade::GetOutputDeviceCount()
{
  deviceList_.clear();
  
  IteratorPtr<MidiOutDevice> it(MidiService::Instance()->GetOutIterator()) ;
  for (it->Begin(); !it->IsDone(); it->Next())
  {
    MidiOutDevice& current = it->CurrentItem();
    deviceList_.push_back(current.GetName());    
  }

  return deviceList_.size();
}

void MidiFacade::QueueMessage(const MidiMessage& message)
{
  queue_.Add(message);
}

void MidiFacade::SendDirectMessage(const MidiMessage& message)
{
  if (selectedDevice_)
  {
    selectedDevice_->SendMessage(message);
  }
}

void MidiFacade::closeSelectedDevice()
{
  if (selectedDevice_)
  {
    senderThread_.SetDevice(NULL);
    selectedDevice_->Stop();
    selectedDevice_->Close();
    selectedDevice_ = NULL;
  }
}

bool MidiFacade::selectDevice(MidiOutDevice& device)
{
  NAssert(!selectedDevice_);
  if (device.Init().Succeeded())
  {
    if (device.Start().Succeeded())
    {
      selectedDevice_ = &device;
      senderThread_.SetDevice(selectedDevice_);
    }
    else 
    {
      device.Close();
    }
  }
  return (selectedDevice_ != NULL);
}


//==============================================================================

void MidiFacade::Queue::Reset()
{
  MutexLocker lock(mutex_);
  messageLists_.clear();
}

void MidiFacade::Queue::PrepareFor(const SystemTick &systemTick)
{
  MutexLocker lock(mutex_);
#ifdef CHECK_FACADE_TIMING
  NAssert(messageLists_.empty() || (systemTick > messageLists_.back()->systemTick_));
#endif
  List *newList = new List(systemTick);
  messageLists_.push_back(CountedPtr<List>(newList));
}

CountedPtr<MidiFacade::Queue::List> MidiFacade::Queue::PopForTick(SystemTick& tick)
{
  MutexLocker lock(mutex_);
  std::deque<CountedPtr<List> >::iterator it = messageLists_.begin();
  if (it != messageLists_.end())
  {
    CountedPtr<List> element = *it;
    if (element->systemTick_ < tick)
    {
//      Trace::Debug("MidiFacade::Poppin' element at %s  [%s]", STickToString(tick).c_str(), STickToString(element->systemTick_).c_str());

      messageLists_.erase(it);
      return element;
    }
  }
  return CountedPtr<List>();
}

void MidiFacade::Queue::Add(const MidiMessage& message)
{

  MutexLocker lock(mutex_);
  
  if(!messageLists_.empty())
  {
    CountedPtr<List> list = messageLists_.back() ;
    list->messages_.push_back(message);    
  } 
  else 
  {
    Trace::Log("MIDI"," dropping !!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  }

}

//==============================================================================

MidiFacade::Queue::List::List(SystemTick tick)
  : systemTick_(tick)
{
}

//==============================================================================

MidiFacade::SenderThread::SenderThread()
  : Thread("Midi facade sender Thread")
  , queue_(NULL)  
  , device_(NULL)
{
}


MidiFacade::SenderThread::~SenderThread()
{  
}

Result MidiFacade::SenderThread::Init(MidiFacade::Queue& queue)
{
  queue_ = &queue;
  return Result::NoError;
}

void MidiFacade::SenderThread::SetDevice(MidiOutDevice* device)
{
  device_ = device;
}

Result MidiFacade::SenderThread::Prepare()
{
  return Result::NoError;
}

bool MidiFacade::SenderThread::RunSlice()
{
  SystemTick tick = TimeService::Instance()->GetSystemTick();
  CountedPtr<MidiFacade::Queue::List> messageList = queue_->PopForTick(tick);
  
  if (device_ && !messageList.IsNULL())
  {
    std::vector<MidiMessage>::iterator it = messageList->messages_.begin();
    while (it != messageList->messages_.end())
    {
      device_->SendMessage(*it);
      it++;
    }
  }
  else {
    Sleep(1);
  }

  return true;
}