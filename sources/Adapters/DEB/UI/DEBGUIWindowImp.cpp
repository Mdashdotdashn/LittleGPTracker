#include "DEBGUIWindowImp.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include "System/System/System.h"
#include "System/io/Trace.h"
#include <string.h>

#define NYI(s) n_assert("NYI: ##s##")

DEBGUIWindowImp *instance_ ;

DEBGUIWindowImp::DEBGUIWindowImp(GUICreateWindowParams &p) {

//	screen_ = SDL_SetVideoMode(p.rect.Width(),p.rect.Height(), 0, 0);

	screen_ = SDL_SetVideoMode(p.rect.Width(),p.rect.Height(), 32, p.fullscreen_?SDL_FULLSCREEN:SDL_HWSURFACE);

    Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

	offscreen_=SDL_CreateRGBSurface(SDL_HWSURFACE, p.rect.Width(), p.rect.Height(), 32,
	                                   rmask, gmask, bmask, amask);
	rect_=p.rect ;
	lastMod_=KMOD_NONE ;
	instance_=this ;
	currentColor_=0;
	backgroundColor_=0 ;
//	prepareFonts() ;
} ;

DEBGUIWindowImp::~DEBGUIWindowImp() {

}


#define FONT_WIDTH 1024
#define FONT_COUNT 127
static const unsigned char font[]= {
	#include "Resources/font.h"
};

static SDL_Surface *fonts[FONT_COUNT] ;

void DEBGUIWindowImp::prepareFonts() {

    Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

	for (int i=0;i<FONT_COUNT;i++) {
	    fonts[i] = SDL_CreateRGBSurface(SDL_HWSURFACE, 8, 8, 32,
	                                   rmask, gmask, bmask, amask);
		if (fonts[i]==NULL) {
			Trace::Debug("Failed to create font surface %d",i) ;
		} else {
			SDL_LockSurface(fonts[i]) ;
			const unsigned char *src=font+i*8 ;
			unsigned int *dest=(unsigned int *)fonts[i]->pixels ;
  	  		for (int y = 0; y < 8; y++) {
    			for (int x = 0; x < 8; x++) {
					*dest =(*src)?0xFFFFFFFF:0xFF000000 ;
					dest++;
					src++ ;
            	}
				if (y<7) src+=FONT_WIDTH-8 ;
		  	}
			SDL_UnlockSurface(fonts[i]) ;
		} ;
	};
} ;

void DEBGUIWindowImp::DrawString(const char *string,GUIPoint &pos,GUITextProperties &p) {

	int len=int(strlen(string)) ;
	int xx=pos._x ;
	int yy=pos._y ;

//     if (len>39) {
// 		len=39 ;
// 		string[39]=0 ;
// 	} ;

	for (int l=0;l<len;l++) {

//		if ( string[l]>FONT_COUNT-1 ) string[l]=0;
/*
		if (currentColor_==0) {
			SDL_Rect srcRect ;
			srcRect.x=0 ;
			srcRect.y=0 ;
			srcRect.w=8 ;
			srcRect.h=8 ;

			SDL_Rect dstRect ;
			dstRect.x=xx ;
			dstRect.y=yy ;
			dstRect.w=8 ;
			dstRect.h=8 ;

			SDL_BlitSurface(fonts[string[l]], &srcRect,screen_, &dstRect);
		} else {
*/			const unsigned char *src=font+(string[l]*8) ;
			unsigned int *dest=((unsigned int *)screen_->pixels) + (yy*rect_.Width()) + xx;
  	  		for (int y = 0; y < 8; y++) {
    				for (int x = 0; x < 8; x++) {
						*dest =((*src)^(unsigned char)p.invert_)?backgroundColor_:currentColor_ ;
						dest++;
						src++ ;
            		}
				if (y<7) src+=FONT_WIDTH-8 ;
				dest+=(rect_.Width()-8) ;
//	  		}
		}
	  	xx+=8 ;
	}

} ;

void DEBGUIWindowImp::DrawRect(GUIRect &r) {
    SDL_Rect rect;
     rect.x=r.Left() ;
     rect.y=r.Top() ;
     rect.w=r.Width() ;
     rect.h=r.Height() ;
     SDL_FillRect(screen_, &rect,currentColor_) ;     
} ;

void DEBGUIWindowImp::Clear(GUIColor &c) {
     SDL_Rect rect;
     rect.x=rect_.Left() ;
     rect.y=rect_.Top() ;
     rect.w=rect_.Width() ;
     rect.h=rect_.Height() ;
	 backgroundColor_=SDL_MapRGB(screen_->format,c._r&0xFF,c._g&0xFF,c._b&0xFF);
     SDL_FillRect(screen_, &rect,backgroundColor_) ;  
} ;

void DEBGUIWindowImp::ClearRect(GUIRect &r) {
     SDL_Rect rect;
     rect.x=r.Left() ;
     rect.y=r.Top() ;
     rect.w=r.Width() ;
     rect.h=r.Height() ;
     SDL_FillRect(screen_, &rect,backgroundColor_) ;
} ;

void DEBGUIWindowImp::Save() {
    SDL_Rect rect;
      rect.x=rect_.Left() ;
     rect.y=rect_.Top() ;
     rect.w=rect_.Width() ;
     rect.h=rect_.Height() ;
	SDL_BlitSurface(screen_, &rect,offscreen_, &rect);
}

void DEBGUIWindowImp::Restore() {
    SDL_Rect rect;
      rect.x=rect_.Left() ;
     rect.y=rect_.Top() ;
     rect.w=rect_.Width() ;
     rect.h=rect_.Height() ;
	SDL_BlitSurface(offscreen_, &rect,screen_, &rect);
}

GUIRect DEBGUIWindowImp::GetRect() {
	return rect_ ;
}

// Sends a event to specify screen has to be redrawn.

void DEBGUIWindowImp::Invalidate() {
	SDL_Event event ;
	event.type=SDL_VIDEOEXPOSE ;
	SDL_PushEvent(&event) ;
} ;



void DEBGUIWindowImp::SetColor(GUIColor &c) {
	currentColor_=SDL_MapRGB(screen_->format,c._r&0xFF,c._g&0xFF,c._b&0xFF);
};


void DEBGUIWindowImp::Flush() {
	SDL_UpdateRect(screen_, 0, 0, rect_.Width(), rect_.Height()); 
} ;

void DEBGUIWindowImp::SendEscape() {

	System *system=System::GetInstance() ;
	unsigned long now=system->GetClock() ;
	GUIEvent event(EKT_ESCAPE,ET_KEYDOWN,now,0,0,0) ;
	instance_->_window->DispatchEvent(event) ;
} ;

bool DEBGUIWindowImp::ProcessDEBEvent(SDL_Event &sdlevent) {

	switch(sdlevent.type) {
		case SDL_VIDEOEXPOSE:
			{
				if (SDL_MUSTLOCK(screen_)) {
					if (!(SDL_LockSurface(screen_) < 0)) {
				
					_window->Update() ;
				 
					SDL_UnlockSurface(screen_);
					}
				} else {
					_window->Update() ;
				}
				SDL_UpdateRect(screen_, 0, 0, rect_.Width(), rect_.Height()); 
			}
			break ;
		case SDL_KEYUP:
			if (sdlevent.key.keysym.sym == SDLK_ESCAPE)
				instance_->SendEscape() ;
			break;

	}
	return true ;
} ;

static GUIEventPadButtonType eventMapping[10]= { EPBT_A,
EPBT_B,EPBT_LEFT,EPBT_RIGHT,EPBT_UP,EPBT_DOWN,EPBT_L,EPBT_R,EPBT_START,EPBT_SELECT } ;

void DEBGUIWindowImp::ProcessButtonChange(unsigned short changeMask,unsigned short buttonMask) {
	int e=1 ;
	System *system=System::GetInstance() ;
	unsigned long now=system->GetClock() ;
	for (int i=0;i<10;i++) {
		if (changeMask&e) {
			GUIEventType type=(buttonMask&e)? ET_PADBUTTONDOWN:ET_PADBUTTONUP ;
			GUIEvent event(eventMapping[i],type,now,0,0,0) ;
			instance_->_window->DispatchEvent(event) ;
		}
		e=e<<1 ;
	}
} ;

bool ProcessDEBEvent(SDL_Event &event) {
	return instance_->ProcessDEBEvent(event) ;
}

void ProcessButtonChange(unsigned short sendMask,unsigned short mask) {
	instance_->ProcessButtonChange(sendMask,mask) ;
}
