#ifndef _T_INSTALLABLE_H_
#define _T_INSTALLABLE_H_

//
// Encapsulate a Factory pattern allowing the installation
// of different factories matching one given interface with
// only one available at the time
//

template <class Item>
class T_Installable {
protected:
	virtual ~T_Installable<Item>() {} ;
public :

	static void Install(Item *) ;
	static Item *Instance() ;

  static bool IsAvailable();

protected:

	// The static instance of the singleton

	static Item * instance_ ;
} ;

#include "T_Installable.cpp"

#endif
