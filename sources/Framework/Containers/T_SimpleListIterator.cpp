#include "System/Console/n_assert.h"
//
// Implementation of the Iterator working on the T_SimpleList
//

template <class Item>
T_SimpleListIterator<Item>::T_SimpleListIterator(T_SimpleList<Item>& list,bool reverse):_list(list),_reverse(reverse) {
	Begin() ;
}

template <class Item>
void T_SimpleListIterator<Item>::Begin() {
	if (_reverse) {
		_current=_list._last ;
		_next=(_current!=0)?_current->prev:0 ;
	} else {
		_current=_list._first ;
		_next=(_current!=0)?_current->next:0 ;
	}
}

template <class Item>
void T_SimpleListIterator<Item>::Next() {
	NAssert(!IsDone()) ;
	if (_reverse) {
		_current=_next ;
		_next=(_current!=0)?_current->prev:0 ;
	} else {
		_current=_next ;
		_next=(_current!=0)?_current->next:0 ;
	}
}

template <class Item>
bool T_SimpleListIterator<Item>::IsDone() const {
	return (_current==NULL) ;
}

template <class Item>
Item &T_SimpleListIterator<Item>::CurrentItem() const {
	NAssert(!IsDone()) ;
	return _current->data ;
}
