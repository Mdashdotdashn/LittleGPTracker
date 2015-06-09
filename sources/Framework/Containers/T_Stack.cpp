
#include "T_Stack.h"

template <class Item>
T_Stack<Item>::T_Stack(bool isOwner):T_SimpleList<Item>(isOwner) {
} ;

template <class Item>
void T_Stack<Item>::Push(Item &i) {
	this->Insert(i) ;
}

template <class Item>
Item *T_Stack<Item>::Pop(bool lifo) {
	Item *i ;
	if (lifo) { // Last in first out
		i=this->GetFirst() ;
	} else {
		i=this->GetLast() ;
	}
	if (i!=NULL) { // First in first out
		this->Remove(*i,false) ;
	}
	return i;
}

template <class Item>
void T_Stack<Item>::Insert(Item &i) {
	T_SimpleList<Item>::Insert(i) ;
}

template <class Item>
void T_Stack<Item>::Insert(Item *i) {
	T_SimpleList<Item>::Insert(*i) ;
}

template <class Item>
void T_Stack<Item>::Empty() {
	T_SimpleList<Item>::Empty() ;
}
