#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#include "Framework/Instances/T_Installable.h"
#include "typedefs.h"
#include <stdlib.h>


class System: public T_Installable<System> {

public: // Override in implementation
	virtual unsigned long GetClock()=0 ; // millisecs
	virtual int GetBatteryLevel()=0 ;
	virtual void *Malloc(unsigned size)=0 ;
	virtual void Free(void *)=0 ;
  virtual void Memset(void *addr,char value,int size)=0 ;
  virtual void *Memcpy(void *s1, const void *s2, int n)=0  ;
	virtual void PostQuitMessage()=0 ;
	virtual unsigned int GetMemoryUsage()=0 ;

} ;

#define SYS_MEMSET(a,b,c) { System* system=System::Instance() ; system->Memset(a,b,c) ;  }
#define SYS_MEMCPY(a,b,c) {  System* system=System::Instance() ; system->Memcpy(a,b,c) ; }
#define SYS_MALLOC(size) (System::Instance()->Malloc(size))
#define SYS_FREE(ptr) (System::Instance()->Free(ptr))

#define SAFE_DELETE(ptr) if (ptr)  { delete ptr ; ptr=0 ; }
#define SAFE_FREE(ptr) if (ptr) { SYS_FREE(ptr); ptr=0 ; }

#endif
