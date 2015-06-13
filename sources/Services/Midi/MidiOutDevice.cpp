#include "MidiOutDevice.h"

MidiOutDevice::MidiOutDevice(const char *name) {
	name_=name ;
} ;

MidiOutDevice::~MidiOutDevice() {
} ;

const char *MidiOutDevice::GetName() {
	return name_.c_str() ;
} ;

void MidiOutDevice::SetName(const char *name)
{
	name_ = name;
}

void MidiOutDevice::SendQueue(const std::vector<MidiMessage>& queue)
{
  std::vector<MidiMessage>::const_iterator it = queue.begin();
  while (it != queue.end())
  {
		SendMessage(*it);
    it++;
	} ;

}

