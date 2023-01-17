
#include "DINGOOSerialMidiDevice.h"
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <memory.h>

#ifdef USB_TTY
const char *port="/dev/usb/tts/0" ;
#else
const char *port="/dev/ttyS1" ;
#endif

#define BAUDRATE B57600

#ifdef RS97
DINGOOSerialMidiDevice::DINGOOSerialMidiDevice():MidiOutDevice("RS-97 Serial") {
#else
DINGOOSerialMidiDevice::DINGOOSerialMidiDevice():MidiOutDevice("DINGOO Serial") {
#endif
    fd_=0 ;
} ;

bool DINGOOSerialMidiDevice::Init(){

     struct termios newtio;

     printf("about to open port\n") ;
     fd_ = open(port, O_RDWR /*| O_NOCTTY | O_NDELAY*/ ); 
     if (fd_ <=0) {printf("Failed to open %s",port); fd_=0 ;return false ; }
     printf("opened serial successfully");
     tcgetattr(fd_,&oldtio_); /* save current port settings */
    
     memset(&newtio,0,sizeof(newtio)) ;
     tcgetattr(fd_,&newtio); /* save current port settings */
     newtio.c_cflag = BAUDRATE | CS8 | CLOCAL;
     newtio.c_iflag = IGNPAR;
     newtio.c_oflag = 0;
    
    /* set input mode (non-canonical, no echo,...) */
    
    newtio.c_lflag = 0;
    
    tcflush(fd_, TCIFLUSH);
    int code=tcsetattr(fd_,TCSANOW,&newtio);
    if (code<0) {
       printf("Failed to set attributes\n") ;
    } else {
       printf("Serial attributes set\n") ;
    }

 	return (code>=0)   ;
} ;

void DINGOOSerialMidiDevice::Close(){

     if (fd_>0) {
        tcsetattr(fd_,TCSANOW,&oldtio_);
        close(fd_) ;
        fd_=0 ;
     }
}  ;

bool DINGOOSerialMidiDevice::Start(){
	return true ;
}  ;

void DINGOOSerialMidiDevice::Stop(){
} ;


void DINGOOSerialMidiDevice::SendMessage(MidiMessage &msg) {
     unsigned char buffer[3] ;
     if (fd_>0) {
        buffer[0]=msg.status_ ;
        int len=1 ;
 
		if (msg.status_<0xF0) {
          buffer[1]=msg.data1_;
          buffer[2]=msg.data2_ ;
          len=3 ;
         }
       write(fd_,buffer,len);
       printf("Sending 0x%x 0x%x 0x%x\n",buffer[0],buffer[1],buffer[2]) ;
    }
     
} ;
