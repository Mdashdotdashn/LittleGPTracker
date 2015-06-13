#pragma once

#include <SDL/SDL.h>

class SysMutex {
public:
	SysMutex() ;
	~SysMutex() ;
	bool Lock() ;
	void Unlock() ;
private:
	SDL_mutex *mutex_ ;
} ;

class SysMutexLocker {
public:
	SysMutexLocker(SysMutex &mutex) ;
	~SysMutexLocker() ;
private:
	SysMutex *mutex_ ;
} ;
