
#include "GP2XGUIWindowImp.h"
#include "Adapters/GP2XSystem/GP2XSystem.h"
#include "Adapters/GP2XSystem/GP2XEventQueue.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include "System/io/Trace.h"
#include <string.h>
#define NYI(s) n_assert("NYI: ##s##")
#include "Application/Model/Config.h"
#include "Application/Utils/char.h"

GP2XGUIWindowImp *instance_ ;

GP2XGUIWindowImp::GP2XGUIWindowImp(GUICreateWindowParams &p) {

	screen_ = SDL_SetVideoMode(p.rect.Width(),p.rect.Height(), 16, SDL_HWSURFACE);
	if (!screen_) {
		Trace::Debug("Trying SDL SW Mode") ;
		screen_ = SDL_SetVideoMode(p.rect.Width(),p.rect.Height(), 16, SDL_SWSURFACE);
		Trace::Debug("Returned from SDL SW Mode") ;
	} ;
	if (!screen_) {
		Trace::Debug("Can't get SDL screen, bailing out") ;
		exit (-1) ;
	} ;
	
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

	offscreen_=SDL_CreateRGBSurface(SDL_HWSURFACE, p.rect.Width(), p.rect.Height(), 16,
	                                   rmask, gmask, bmask, amask);
	                                   
	SDL_ShowCursor(SDL_DISABLE);
	SDL_WarpMouse(320,240) ;
	rect_=p.rect ;
	lastMod_=KMOD_NONE ;
	instance_=this ;
	currentColor_=0;
	prepareFonts() ;
} ;

GP2XGUIWindowImp::~GP2XGUIWindowImp() {
}


#define FONT_WIDTH 1024
#define FONT_COUNT 127
static const unsigned char font[]= {
	#include "Resources/font.h"
};



static SDL_Surface *fonts[FONT_COUNT] ;


void GP2XGUIWindowImp::prepareFonts() {

    Uint32 rmask, gmask, bmask, amask;

    Config *config=Config::GetInstance() ;
    
   unsigned char r,g,b ;
   const char *value=config->GetValue("BACKGROUND") ;
   if (value) {
      char2hex(value,&r) ;
      char2hex(value+2,&g) ;
      char2hex(value+4,&b) ;
   } else {
      r=0xFF ;
      g=0xFF ;
      b=0xFF ;      
   }
   backgroundColor_=SDL_MapRGB(screen_->format, r,g,b) ;
           
   value=config->GetValue("FOREGROUND") ;
   if (value) {
      char2hex(value,&r) ;
      char2hex(value+2,&g) ;
      char2hex(value+4,&b) ;
   } else {
      r=0x00 ;
      g=0x00 ;
      b=0x00 ;      
   }
   foregroundColor_=SDL_MapRGB(screen_->format, r,g,b) ;
              
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
	    fonts[i] = SDL_CreateRGBSurface(SDL_HWSURFACE, 8, 8, 16,
	                                   0, 0, 0, 0);
		if (fonts[i]==NULL) {
			Trace::Dump("Failed to create font surface %d",i) ;
		} else {
			SDL_LockSurface(fonts[i]) ;
			const unsigned char *src=font+i*8 ;
			unsigned short*dest=(unsigned short *)fonts[i]->pixels ;
  	  		for (int y = 0; y < 8; y++) {
    			for (int x = 0; x < 8; x++) {
					*dest =(*src)?backgroundColor_:foregroundColor_;
					dest++;
					src++ ;
            	}
				if (y<7) src+=FONT_WIDTH-8 ;
		  	}
			SDL_UnlockSurface(fonts[i]) ;
		} ;
	};
} ;


void GP2XGUIWindowImp::DrawString(const char *str,GUIPoint &pos,GUITextProperties &p) {

     char string[60] ;
     strcpy(string,str) ;
	int len=int(strlen(str)) ;
	int xx=pos._x ;
	int yy=pos._y ;

    if (len>39) {
		len=39 ;
		string[39]=0 ;
	} ;

	for (int l=0;l<len;l++) {
		if ( string[l]>126 ) string[l]=0;
		if ((currentColor_==foregroundColor_)&&(!p.invert_)) {
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
		} else  {
			SDL_LockSurface(screen_) ;
			const unsigned char *src=font+(string[l]*8) ;
			unsigned short *dest=((unsigned short *)screen_->pixels) + (yy*screen_->pitch/2) + xx;
  	  		for (int y = 0; y < 8; y++) {
    				for (int x = 0; x < 8; x++) {
						*dest =((*src)^(unsigned char)p.invert_)?backgroundColor_:currentColor_ ;
						dest++;
						src++ ;
            		}
				if (y<7) src+=FONT_WIDTH-8 ;
				dest+=(rect_.Width()-8) ;

	  		}
			SDL_UnlockSurface(screen_) ; 
		}
	  	xx+=8 ;
    }

} ;

void GP2XGUIWindowImp::DrawRect(GUIRect &r) {
     SDL_Rect rect;
     rect.x=r.Left() ;
     rect.y=r.Top() ;
     rect.w=r.Width() ;
     rect.h=r.Height() ;
     SDL_FillRect(screen_, &rect,currentColor_) ;   
} ;

void GP2XGUIWindowImp::Clear(GUIColor &c) {
     SDL_Rect rect;
     rect.x=rect_.Left() ;
     rect.y=rect_.Top() ;
     rect.w=rect_.Width() ;
     rect.h=rect_.Height() ;
     backgroundColor_=SDL_MapRGB(screen_->format, c._r, c._g, c._b);

     SDL_FillRect(screen_, &rect,backgroundColor_) ;   
} ; 

void GP2XGUIWindowImp::ClearRect(GUIRect &r) {
     SDL_Rect rect;
     rect.x=r.Left() ;
     rect.y=r.Top() ;
     rect.w=r.Width() ;
     rect.h=r.Height() ;
     SDL_FillRect(screen_, &rect,backgroundColor_) ;
} ;

GUIRect GP2XGUIWindowImp::GetRect() {
	return rect_ ;
}

void GP2XGUIWindowImp::Save() {
    SDL_Rect rect;
      rect.x=rect_.Left() ;
     rect.y=rect_.Top() ;
     rect.w=rect_.Width() ;
     rect.h=rect_.Height() ;
	SDL_BlitSurface(screen_, &rect,offscreen_, &rect);
}

void GP2XGUIWindowImp::Restore() {
    SDL_Rect rect;
      rect.x=rect_.Left() ;
     rect.y=rect_.Top() ;
     rect.w=rect_.Width() ;
     rect.h=rect_.Height() ;
	SDL_BlitSurface(offscreen_, &rect,screen_, &rect);
}

void GP2XGUIWindowImp::Invalidate() {
     
     GP2XEvent *event=new GP2XEvent() ; ;
     event->type_=GP2XET_REDRAW ;
     GP2XEventQueue::GetInstance()->Push(*event) ;
 
} ;


void GP2XGUIWindowImp::SetColor(GUIColor &c) {

     currentColor_=SDL_MapRGB(screen_->format, c._r, c._g, c._b);

};


void GP2XGUIWindowImp::Flush() {
//	SDL_UpdateRect(screen_, 0, 0, rect_.Width(), rect_.Height()); 
} ;

bool GP2XGUIWindowImp::ProcessGP2XEvent(GP2XEvent &sdlevent) {

    _window->Update() ;
 //   SDL_UpdateRect(screen_, 0, 0, rect_.Width(), rect_.Height()); 				
	return true ;
} ;

static GUIEventPadButtonType eventMapping[10]= { EPBT_A,
EPBT_B,EPBT_LEFT,EPBT_RIGHT,EPBT_UP,EPBT_DOWN,EPBT_L,EPBT_R,EPBT_START,EPBT_SELECT } ;

void GP2XGUIWindowImp::ProcessButtonChange(unsigned short changeMask,unsigned short buttonMask) {
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

void GP2XGUIWindowImp::SendESCMessage() {
	System *system=System::GetInstance() ;
	unsigned long now=system->GetClock() ;
	GUIEvent event(EKT_ESCAPE,ET_KEYDOWN,now,0,0,0) ;
	instance_->_window->DispatchEvent(event) ;
     
} ;
     
bool ProcessGP2XEvent(GP2XEvent &event) {
	return instance_->ProcessGP2XEvent(event) ;
}

void ProcessButtonChange(unsigned short sendMask,unsigned short mask) {
	instance_->ProcessButtonChange(sendMask,mask) ;
}

void SendESCMessage() {
     instance_->SendESCMessage() ;
}
