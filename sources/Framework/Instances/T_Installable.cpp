#include <System/Console/n_assert.h>

template <class Item> Item* T_Installable<Item>::instance_=0 ;

template <class Item>
void T_Installable<Item>::Install(Item *instance) 
{
	NAssert(!instance_) ;
	instance_=instance ;
}

template <class Item>
Item* T_Installable<Item>::Instance() 
{
	NAssert(instance_) ;
	return instance_ ;
}

template <class Item>
bool T_Installable<Item>::IsAvailable() 
{
	return (instance_ != 0) ;
}
