
template <class Item> Item* T_Singleton<Item>::instance_=0 ;

template <class Item>
T_Singleton<Item>::T_Singleton() {
#ifdef PLATFORM_WINDOWS
	SingletonRegistry *registry=SingletonRegistry::Instance() ;
	registry->Insert(this) ;
#endif
}


template <class Item>
T_Singleton<Item>::~T_Singleton() {
}

template <class Item>
Item* T_Singleton<Item>::Instance() {
	if (instance_==0) {
		instance_=new Item ;
	} ;
	return instance_ ;
}

template <class Item>
bool T_Singleton<Item>::IsAvailable()
{
	return (instance_ != 0);
}
