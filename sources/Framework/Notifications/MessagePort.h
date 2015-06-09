#pragma once

#include "EventBroadcaster.h"
#include "System/Console/n_assert.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

template <class MessageType>
class MessagePort
{
public:
  
  using MessageCallback = void(const MessageType&);
  using MessageListCallback = void(const std::vector<MessageType>&);

  //===============================================================================

  class Receiver
  {
  public:
    virtual void OnMessage(const MessageType& message) = 0;
    virtual void OnMessageList(const std::vector<MessageType>& messages)
    {
      for (const auto &message : messages)
      {
        OnMessage(message);
      }
    }
  };


  //===============================================================================

  void Connect(Receiver* receiver)
  {
    mBroadcaster.AddListener(receiver);
  }

  void Disconnect(Receiver* receiver)
  {
    mBroadcaster.RemoveListener(receiver);
  }

  void Broadcast(const MessageType& message) const
  {
    mBroadcaster.template NotifyListeners<const MessageType&>(&Receiver::OnMessage, message);
  }

  void ListBroadcast(const std::vector<MessageType>& messageList) const
  {
    mBroadcaster.template NotifyListeners<const std::vector<MessageType>&>(&Receiver::OnMessageList, messageList);
  }

  void Connect(const std::function<MessageCallback>& callback)
  {
    forwarders_.emplace_back(new Forwarder(callback));
    Connect(forwarders_.back().get());
  }
  
private:

  //===============================================================================

  class Forwarder
    : public Receiver
  {
  public:

    Forwarder(const std::function<MessageCallback>& cb)
      : callback_(cb)
      , listCallback_(nullptr)
    {
    }

    Forwarder(const std::function<MessageListCallback>& listCallback)
    : callback_(nullptr)
    , listCallback_(listCallback)
    {
    }
    
    void OnMessage(const MessageType& message) override
    {
      NAssert(callback_);
      callback_(message);
    }

    void OnMessageList(const std::vector<MessageType&> messageList)
    {
      if (listCallback_)
      {
        listCallback_(messageList);
      }
      else
      {
        for (const auto& message : messageList)
        {
          OnMessage(message);
        }
      }
    }
  private:
    std::function<MessageCallback> callback_;
    std::function<MessageListCallback> listCallback_;
  };
  
  //===============================================================================

  EventBroadcaster<Receiver> mBroadcaster;
  std::vector<std::unique_ptr<Forwarder> > forwarders_;
};


using Bang = MessagePort<void>;
using StringBroadcaster = MessagePort < std::string > ;

//--------------------------------------------------------------------------------------------

