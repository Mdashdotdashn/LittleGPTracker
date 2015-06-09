/*
 *  MIDIFacade.h
 *  lgptx
 *
 *  Created by Marc Nostromo on 17/08/13.
 *  Copyright 2013 __MyCompanyName__. All rights reserved.
 *
 */

#include "Foundation/T_Singleton.h"
#include "Services/Midi/MidiOutDevice.h"
#include "Foundation/Errors/Result.h"
#include "Application/Mixer/MixerService.h"
#include "System/Process/Process.h"

#include <vector>
#include <deque>
#include <memory>
#include <string>

class MidiFacade:public T_Singleton<MidiFacade>
{
public:
  
  MidiFacade();
  
  Result Init();
  void Close();
  
  void OnPlayerStart();
  void OnPlayerStop();
  
  void OnPlayerTick();
  
  const std::vector<std::string>& DeviceList();
  bool SelectDevice(const std::string& deviceName);
  
  int GetOutputDeviceCount();
  void QueueMessage(const MidiMessage& message);
  void SendDirectMessage(const MidiMessage& message);
  
  std::string ToString() const;
  
private:
  
  void closeSelectedDevice();
  bool selectDevice(MidiOutDevice& device);
  
private:
  
  std::vector<std::string> deviceList_;
  MidiOutDevice* selectedDevice_;
  bool sendSync_;
  
  //----------------------------------------------------------------------------

  class Queue
  {
  public:
    
    class List
    {
    public:
      List(SystemTick systemTick);
      TimeService::Tick systemTick_;
      std::vector<MidiMessage> messages_;
    };
    
  public:
    void Reset();
    void PrepareFor(const SystemTick& systemTick);
    void Add(const MidiMessage& message);

    CountedPtr<MidiFacade::Queue::List> PopForTick(SystemTick& tick);
    
  private:  
        
    std::deque<CountedPtr<MidiFacade::Queue::List> > messageLists_;
    Mutex mutex_;
  } queue_;
  
  //----------------------------------------------------------------------------
  
  class SenderThread: public Thread
  {
  public:
    SenderThread();
    ~SenderThread();

    Result Init(MidiFacade::Queue& queue);
    void SetDevice(MidiOutDevice* device);
    
  protected:
    virtual Result Prepare();
    virtual bool RunSlice();
  private:
    MidiFacade::Queue* queue_;
    MidiOutDevice *device_;
  } senderThread_;
  
  //----------------------------------------------------------------------------

  enum PlayerChangedState
  {
    None,
    Started,
    Stopped
  } playerChangedState_;
};