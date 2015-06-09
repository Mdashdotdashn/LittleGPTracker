
#ifndef _STATUS_H_
#define _STATUS_H_

#include "Framework/Instances/T_Installable.h"

class Status: public T_Installable<Status>  {
public:
	virtual void Print(char *)=0 ;
	static void Set(char *fmt, ...) ;
} ;

#endif
