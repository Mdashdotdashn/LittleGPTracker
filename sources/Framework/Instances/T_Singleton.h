#ifndef _T_SINGLETON_H_
#define _T_SINGLETON_H_

#include "SingletonRegistry.h"

template <class Item>
class T_Singleton: public I_Singleton {
protected:

	T_Singleton() ;
	virtual ~T_Singleton() ;
public:
    
	static Item *Instance() ;
  static bool IsAvailable();

protected:

	// The static instance of the singleton

	static Item * instance_ ;
} ;

#include "T_Singleton.cpp"     // Include the implementation file.

#endif
