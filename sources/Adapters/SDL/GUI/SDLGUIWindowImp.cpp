#include "SDLGUIWindowImp.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include "System/System/System.h"
#include "Application/Model/Config.h"
#include "System/Console/Trace.h"
#include <string.h>
#include "System/Console/n_assert.h"
#include "Application/Utils/char.h"
#include <assert.h>
#include "UIFramework/BasicDatas/GUIEvent.h"

#ifdef _SHOW_GP2X_
 #include <SDL/SDL_image.h>

 #include "Application/Commands/EventDispatcher.h"
#endif

SDLGUIWindowImp *instance_ ;

#ifdef _SHOW_GP2X_
enum Bitmaps {
	BM_FRONT,
	BM_LS,
	BM_RS,
	BM_A,
	BM_B,
	BM_START,
	BM_SELECT,
	BM_LEFT,
	BM_UP,
	BM_DOWN,
	BM_RIGHT,
	BM_OVERLAY,
	BM_LAST
} ;


SDL_Surface *bitmaps_[BM_LAST] ;
int originX_[BM_LAST] ;
int originY_[BM_LAST] ;

int scaleFactor=6 ;
int gp2xAnchorX=34 ;
int gp2xAnchorY=90 ;

#endif

unsigned short appWidth=320 ;
unsigned short appHeight=240 ;

SDLGUIWindowImp::SDLGUIWindowImp(GUICreateWindowParams &p) 
{

  SDLCreateWindowParams &sdlP=(SDLCreateWindowParams &)p;
  cacheFonts_=sdlP.cacheFonts_ ;
  framebuffer_=sdlP.framebuffer_ ;
  
  // By default if we are not running a framebuffer device
  // we assumed it's windowed

  windowed_ = !framebuffer_;

  const SDL_VideoInfo* videoInfo = SDL_GetVideoInfo();
  NAssert(videoInfo != NULL);
 
 #if defined(PLATFORM_GP2X)
  int screenWidth = 320;
  int screenHeight = 240;
 #elif defined(PLATFORM_PSP)
  int screenWidth = 480; 
  int screenHeight = 272;
  windowed_ = false;
 #elif defined(RS97)
  int screenWidth = 320; 
  int screenHeight = 240;
  windowed_ = false;
 #else
  int screenWidth = videoInfo->current_w;
  int screenHeight = videoInfo->current_h;
 #endif
 
 #if defined(RS97)
  /* Pick the best bitdepth for the RS97 as it will select 32 as its default, even though that's slow */
  bitDepth_ = 16;
 #else
  bitDepth_ = videoInfo->vfmt->BitsPerPixel;
 #endif
  
  char driverName[64] ;
  SDL_VideoDriverName(driverName,64);
  
  Trace::Log("DISPLAY","Using driver %s. Screen (%d,%d) Bpp:%d",driverName,screenWidth,screenHeight,bitDepth_);
  
  bool fullscreen=false ;
  
  const char *fullscreenValue=Config::GetInstance()->GetValue("FULLSCREEN") ;
  if ((fullscreenValue)&&(!strcmp(fullscreenValue,"YES")))
  {
  	fullscreen=true ;
  }
  
  SDL_WM_SetCaption("LittleGPTracker","lgpt");
	
  if (!strcmp(driverName, "fbcon"))
  {
    framebuffer_ = true;
    windowed_ = false;
  }
 
  int multFromSize=MIN(screenHeight/appHeight,screenWidth/appWidth);
 
  const char *mult=Config::GetInstance()->GetValue("SCREENMULT") ;
  if (mult)
  {
    mult_=atoi(mult);
  }
  else
  {
    if (framebuffer_)
    {
      mult_ = multFromSize;
    }
    else
    {
      mult_ = 1;
    }
  }

  // Create a window that is the requested size
  
  screenRect_._topLeft._x=0;
  screenRect_._topLeft._y=0;
  screenRect_._bottomRight._x=windowed_?appWidth*mult_:screenWidth;
  screenRect_._bottomRight._y=windowed_?appHeight*mult_:screenHeight;

  Trace::Log("DISPLAY","Creating SDL Window (%d,%d)",screenRect_.Width(), screenRect_.Height());
	screen_ = SDL_SetVideoMode(screenRect_.Width(),screenRect_.Height(),bitDepth_ ,fullscreen?SDL_FULLSCREEN:SDL_HWSURFACE);
	NAssert(screen_) ;

	// Compute the x & y offset to locate our app window

	appAnchorX_=(screenRect_.Width()-appWidth*mult_)/2 ;
	appAnchorY_=(screenRect_.Height()-appHeight*mult_)/2 ;

	SDL_WM_SetIcon(SDL_LoadBMP("lgpt_icon.bmp"), NULL);

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

	instance_=this ;
	currentColor_=0;
	backgroundColor_=0 ;
	SDL_ShowCursor(SDL_DISABLE);

	
	if (cacheFonts_)
  {
    Trace::Log("DISPLAY","Preparing fonts") ;
		prepareFonts() ;
	}
#ifdef _SHOW_GP2X_
	Trace::Log("DISPLAY","Preparing overlay bitmaps") ;
	prepareBitmaps() ;
#endif
	updateCount_=0 ;
} ;

SDLGUIWindowImp::~SDLGUIWindowImp() {

}

#ifdef _SHOW_GP2X_

SDL_Surface *SDLGUIWindowImp::load_image(int which,char *filename,int x,int y) {

	Trace::Debug("loading %s",filename) ;
    //Temporary storage for the image that's loaded
    SDL_Surface* loadedImage = NULL;

    //The optimized image that will be used
    SDL_Surface* optimizedImage = NULL;

    //Load the image
    loadedImage = IMG_Load(filename);

    //If nothing went wrong in loading the image
    if (loadedImage != NULL) {
		// First convert to bit depth
		SDL_Surface *converted=SDL_DisplayFormatAlpha(loadedImage); 
		// Scale using factor
		if (scaleFactor!=1) {

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


			SDL_Surface *scaled = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCALPHA, converted->w*scaleFactor, converted->h*scaleFactor, bitDepth_,
			                               rmask, gmask, bmask, amask);
			if (scaled==NULL) {
				Trace::Error("Failed to scaled surface for %s",filename) ;
			} else {
        Trace::Debug("Scaling %d,%d",scaled->w,scaled->h) ;
				SDL_LockSurface(scaled) ;
				SDL_LockSurface(converted) ;
				unsigned int *src=(unsigned int *)converted->pixels;
				unsigned int *dest=(unsigned int *)scaled->pixels;

				for (int i=0;i<converted->h;i++) {
					for (int ii=0;ii<scaleFactor;ii++) {
						for (int j=0;j<converted->w;j++) {
								for (int jj=0;jj<scaleFactor;jj++) {
									*dest++=src[j] ;
								}
						} ;
						dest+=scaled->pitch/sizeof(int)-scaleFactor*converted->w ;
					}
					src+=converted->pitch/sizeof(int) ; ;
				}
				SDL_UnlockSurface(scaled) ;
				SDL_UnlockSurface(converted) ;
				bitmaps_[which] = scaled;
				SDL_FreeSurface(converted) ;
			}
		} else {
        //Create an optimized image
	        bitmaps_[which] = SDL_DisplayFormatAlpha(loadedImage);
		}
//		SDL_SetAlpha(optimizedImage,SDL_SRCALPHA,SDL_ALPHA_TRANSPARENT) ;
        //Free the old image
Trace::Debug("Freeing image") ;
        SDL_FreeSurface(loadedImage);
    } else {
	 	Trace::Debug("**ERR** IMG_Load: %s\n", IMG_GetError());

	}
Trace::Debug("storing point %d",which) ;
	originX_[which]=x ;
	originY_[which]=y ;
Trace::Debug("returning") ;

    return optimizedImage;
}

void SDLGUIWindowImp::prepareBitmaps() {

	load_image(BM_FRONT,"../bitmaps/GP2X_bg.png",0,0) ;
	load_image(BM_LS,"../bitmaps/GP2X_LS.png",0,0) ;
	load_image(BM_RS,"../bitmaps/GP2X_RS.png",77,0) ;
	load_image(BM_A,"../bitmaps/GP2X_Button.png",76,17) ;
	load_image(BM_B,"../bitmaps/GP2X_Button.png",81,12) ;
	load_image(BM_SELECT,"../bitmaps/GP2X_SS.png",79,33) ;
	load_image(BM_START,"../bitmaps/GP2X_SS.png",85,33) ;
	load_image(BM_LEFT,"../bitmaps/GP2X_LEFT.png",7,17) ;
	load_image(BM_UP,"../bitmaps/GP2X_UP.png",7,17) ;
	load_image(BM_DOWN,"../bitmaps/GP2X_DOWN.png",7,17) ;
	load_image(BM_RIGHT,"../bitmaps/GP2X_RIGHT.png",7,17) ;
	load_image(BM_OVERLAY,"../bitmaps/GP2X_overlay.png",0,0) ;
}
#endif

#define FONT_WIDTH 1024
#define FONT_COUNT 127
static const unsigned char font[]= {1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1,0,1,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,0,0,1,1,1,0,0,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,1,0,0,0,0,1,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,0,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,0,1,1,0,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,0,1,1,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,1,1,0,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,0,1,1,0,1,1,1,1,0,1,1,0,1,1,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,0,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,0,0,1,1,1,0,0,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0,1,1,1,0,0,1,0,0,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,0,0,0,1,1,1,1,0,0,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,0,0,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,0,0,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,0,0,1,0,0,1,1,1,0,0,1,1,1,1,1,0,0,0,1,0,0,0,1,1,0,0,0,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,1,1,1,0,1,1,0,1,1,0,0,0,0,1,0,0,1,1,0,0,1,0,0,0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,1,1,0,0,1,1,0,1,1,1,1,1,0,0,1,0,0,1,0,1,1,0,0,1,1,0,0,0,0,1,1,1,1,1,0,1,0,0,1,1,0,1,1,1,1,0,1,1,0,1,0,0,1,0,1,0,1,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,0,0,1,1,1,0,0,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0,1,1,1,0,0,1,0,0,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,0,0,1,0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0,1,1,1,1,1,1,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,0,0,1,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,0,0,1,1,1,0,0,1,1,1,0,0,0,0,1,1,1,0,0,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,0,0,0,1,0,0,1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,0,0,0,0,1,1,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,0,1,1,1,0,0,1,1,0,0,1,0,1,1,0,0,1,1,0,0,0,0,1,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,0,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,0,0,1,0,0,1,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,0,0,1,0,1,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,0,0,1,0,0,0,1,1,0,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,1,0,1,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,0,0,1,1,1,1,1,0,0,1,1,1,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,0,0,1,1,1,0,0,1,1,1,0,0,0,0,1,1,1,0,0,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,0,1,1,0,0,0,0,1,1,1,0,0,1,0,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,1,0,0,1,1,1,0,0,1,0,0,1,0,0,1,0,0,0,0,0,0,1,1,1,1,0,1,1,1,1,0,1,1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,1,1,1,1,1,1,0,0,1,0,1,0,1,1,0,0,0,1,1,1,0,0,1,1,1,1,1,1,0,0,1,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,0,0,1,1,1,0,0,1,1,1,1,1,1,0,0,1,0,0,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,0,0,1,1,1,1,0,0,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,1,0,0,1,1,0,0,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,1,1,0,0,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,0,0,1,1,1,1,1,1,0,0,1,1,1,0,0,1,0,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,0,0,1,0,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,0,0,1,1,0,0,1,0,0,0,0,1,0,0,1,1,0,0,1,0,0,0,0,0,1,1,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,1,1,1,1,0,0,1,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,0,1,1,1,1,1,0,0,0,1,1,1,1,1,0,1,1,1,1,0,1,1,0,1,0,0,1,0,1,0,1,1,1,1,1,1,0,0,1,1,0,1,1,1,0,0,0,1,1,1,0,0,1,1,1,1,1,1,0,0,1,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,0,0,1,1,1,0,0,1,1,1,1,1,1,0,0,1,0,0,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,0,0,1,1,0,0,1,0,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,1,1,0,0,1,1,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,0,0,1,1,1,1,1,0,0,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,0,0,0,0,0,0,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,0,0,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,0,0,1,1,1,0,0,1,1,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,1,0,0,0,0,1,1,0,0,0,1,0,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,0,0,1,1,1,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,0,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,0,0,0,0,1,1,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,1,1,0,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,0,1,1,1,0,0,0,0,1,1,1,1,1,1,0,1,1,0,1,1,1,1,0,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,0,0,1,1,1,1,1,0,0,1,1,1,1,0,1,1,1,0,0,1,1,0,0,0,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,0,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,1,1,1,1,1,1,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,0,1,1,0,0,1,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,0,0,0,0,1,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,0,0,0,0,1,1,1,0,0,1,1,1,1,1,1,1,0,0,1,0,0,1,1,0,0,1,1,0,0,1,1,1,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,0,0,0,0,0,1,1,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,0,1,1,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,0,0,1,1,0,0,1,1,1,0,0,0,0,1,1,0,0,1,1,1,0,0,1,1,0,0,1,1,0,0,1,1,1,0,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,1,0,0,1,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,1,0,0,0,0,0,0,1,1,1,1,1,0,0,0,1,1,1,1,0,0,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,0,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

static SDL_Surface *fonts[FONT_COUNT] ;

void SDLGUIWindowImp::prepareFullFonts()
{
  Trace::Log("DISPLAY","Preparing full font cache") ;
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

	for (int i=0;i<FONT_COUNT;i++)
  {
    
	  fonts[i] = SDL_CreateRGBSurface(
                 SDL_HWSURFACE, 
                 8*mult_, 8*mult_, 
                 bitDepth_,
                 0, 0, 0, 0);
		if (fonts[i]==NULL) 
    {
			Trace::Error("[DISPLAY] Failed to create font surface %d",i) ;
		}
    else
    {
			SDL_LockSurface(fonts[i]) ;
			int pixelSize=fonts[i]->format->BytesPerPixel ;
			unsigned char *bgPtr=(unsigned char *)&backgroundColor_ ;
			unsigned char *fgPtr=(unsigned char *)&foregroundColor_ ;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			bgPtr+=(4-pixelSize) ;
			fgPtr+=(4-pixelSize) ;
#endif
			const unsigned char *src=font+i*8 ;
			unsigned char *dest=(unsigned char *)fonts[i]->pixels;
      for (int y = 0; y < 8; y++)
      {
				for (int n=0;n<mult_;n++)
        {
					for (int x = 0; x < 8; x++)
          {
						unsigned char *pxlPtr=src[x]?bgPtr:fgPtr ;
						for (int m=0;m<mult_;m++)
            {
							memcpy(dest,pxlPtr,pixelSize) ;
							dest+=pixelSize ;
						}
					}
					dest+=fonts[i]->pitch-8*pixelSize*mult_ ;
        }
				if (y<7) src+=FONT_WIDTH ;
      }
			SDL_UnlockSurface(fonts[i]) ;
		};
	};
}

void SDLGUIWindowImp::prepareFonts() 
{

  Trace::Log("DISPLAY","Preparing font cache") ;
  Config *config=Config::GetInstance() ;
  
  unsigned char r,g,b ;
  const char *value=config->GetValue("BACKGROUND") ;
  if (value)
  {
    char2hex(value,&r) ;
    char2hex(value+2,&g) ;
    char2hex(value+4,&b) ;
  } 
  else
  {
    r=0xF1 ;
    g=0xF1 ;
    b=0x96 ;      
  }
  backgroundColor_=SDL_MapRGB(screen_->format, r,g,b) ;
           
  value=config->GetValue("FOREGROUND") ;
  if (value)
  {
    char2hex(value,&r) ;
    char2hex(value+2,&g) ;
    char2hex(value+4,&b) ;
  } 
  else
  {
    r=0x77 ;
    g=0x6B ;
    b=0x56 ;      
  }
  foregroundColor_=SDL_MapRGB(screen_->format, r,g,b) ;
        
  prepareFullFonts() ;
}

void SDLGUIWindowImp::DrawChar(const char c, GUIPoint &pos, GUITextProperties &p) 
{
  int xx,yy;
  transform(pos, &xx, &yy);

  if ((xx<0) || (yy<0)) return;
  if ((xx>=screenRect_._bottomRight._x) || (yy>=screenRect_._bottomRight._y))
       return ;
	if ((!framebuffer_)&&(updateCount_<MAX_OVERLAYS)) {
		SDL_Rect *area=updateRects_+updateCount_++ ;
		area->x=xx ;
		area->y=yy ;
		area->h=8*mult_ ;
		area->w=8*mult_ ;
	}

	if (((cacheFonts_)&&(currentColor_==foregroundColor_)&&(!p.invert_))) {

		SDL_Rect srcRect ;
		srcRect.x=0 ;
		srcRect.y=0 ;
		srcRect.w=8*mult_ ;
		srcRect.h=8*mult_ ;

		SDL_Rect dstRect ;
		dstRect.x=xx ;
		dstRect.y=yy ;
		dstRect.w=8*mult_ ;
		dstRect.h=8*mult_ ;

		unsigned int fontID=c ;
		if (fontID<FONT_COUNT) {
			SDL_BlitSurface(fonts[fontID], &srcRect,screen_, &dstRect);
		}

	} else {
		// prepare bg & fg pixel ptr
		int pixelSize=screen_->format->BytesPerPixel ;
		unsigned char *bgPtr=(unsigned char *)&backgroundColor_ ;
		unsigned char *fgPtr=(unsigned char *)&currentColor_ ;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		bgPtr+=(4-pixelSize) ;
		fgPtr+=(4-pixelSize) ;
#endif
		const unsigned char *src=font+c*8 ;
		unsigned char *dest=((unsigned char *)screen_->pixels) + (yy*screen_->pitch) + xx*pixelSize;

		for (int y = 0; y < 8; y++) {
			for (int n=0;n<mult_;n++) {
				for (int x = 0; x < 8; x++) {
					unsigned char *pxlPtr=((src[x])^(unsigned char)p.invert_)?bgPtr:fgPtr ; ;
					for (int m=0;m<mult_;m++) {
						memcpy(dest,pxlPtr,pixelSize) ;
						dest+=pixelSize ;
					}
				}
				dest+=screen_->pitch-8*pixelSize*mult_ ;
    		}
			if (y<7) src+=FONT_WIDTH ;
  		}

	}
}

void SDLGUIWindowImp::transform(const GUIRect &srcRect,SDL_Rect *dstRect)
{
  dstRect->x = srcRect.Left() * mult_ + appAnchorX_;
  dstRect->y = srcRect.Top() * mult_  + appAnchorY_;
  dstRect->w = srcRect.Width() * mult_ ;
  dstRect->h = srcRect.Height() * mult_ ; 
}

void SDLGUIWindowImp::transform(const GUIPoint &srcPoint, int *x, int *y)
{
 	*x=appAnchorX_ + srcPoint._x*mult_ ;
	*y=appAnchorY_ + srcPoint._y*mult_ ; 
}

void SDLGUIWindowImp::DrawString(const char *string,GUIPoint &pos,GUITextProperties &p,bool overlay) 
{

	int len=int(strlen(string)) ;
  int xx,yy;
  transform(pos, &xx , &yy);

	if ((!framebuffer_)&&(updateCount_<MAX_OVERLAYS))
  {
		SDL_Rect *area=updateRects_+updateCount_++ ;
		area->x=xx ;
		area->y=yy ;
		area->h=8*mult_ ;
		area->w=len*8*mult_ ;
	}

	for (int l=0;l<len;l++)
  {
		if (((cacheFonts_)&&(currentColor_==foregroundColor_)&&(!p.invert_)))
    {
			SDL_Rect srcRect ;
			srcRect.x=0 ;
			srcRect.y=0 ;
			srcRect.w=8*mult_ ;
			srcRect.h=8*mult_ ;

			SDL_Rect dstRect ;
			dstRect.x=xx ;
			dstRect.y=yy ;
			dstRect.w=8*mult_ ;
			dstRect.h=8*mult_ ;

			unsigned int fontID=string[l] ;
			if (fontID<FONT_COUNT)
      {
				SDL_BlitSurface(fonts[fontID], &srcRect,screen_, &dstRect);
			}
		} 
    else
    {
			// prepare bg & fg pixel ptr
			int pixelSize=screen_->format->BytesPerPixel ;
			unsigned char *bgPtr=(unsigned char *)&backgroundColor_ ;
			unsigned char *fgPtr=(unsigned char *)&currentColor_ ;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			bgPtr+=(4-pixelSize) ;
			fgPtr+=(4-pixelSize) ;
#endif
			const unsigned char *src=font+(string[l]*8) ;
			unsigned char *dest=((unsigned char *)screen_->pixels) + (yy*screen_->pitch) + xx*pixelSize;

			for (int y = 0; y < 8; y++) {
				for (int n=0;n<mult_;n++) {
					for (int x = 0; x < 8; x++) {
						unsigned char *pxlPtr=((src[x])^(unsigned char)p.invert_)?bgPtr:fgPtr ; ;
						for (int m=0;m<mult_;m++) {
							memcpy(dest,pxlPtr,pixelSize) ;
							dest+=pixelSize ;
						}
					}
					dest+=screen_->pitch-8*pixelSize*mult_ ;
        		}
				if (y<7) src+=FONT_WIDTH ;
	  		}

		}
    xx+=8*mult_ ;
	}
}

void SDLGUIWindowImp::DrawRect(GUIRect &r) 
{
  SDL_Rect rect;
  transform(r, &rect);
  SDL_FillRect(screen_, &rect,currentColor_) ;
} ;

void SDLGUIWindowImp::Clear(GUIColor &c,bool overlay) 
{
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = screenRect_.Width();
  rect.h = screenRect_.Height();
 
	backgroundColor_=SDL_MapRGB(screen_->format,c._r&0xFF,c._g&0xFF,c._b&0xFF);
  SDL_FillRect(screen_, &rect,backgroundColor_) ;  

#ifdef _SHOW_GP2X_
	drawGP2X() ;
	rect.x=appAnchorX_;
	rect.y=appAnchorY_;
	rect.w=320;
	rect.h=240;
  SDL_FillRect(screen_, &rect,backgroundColor_) ;  
#endif

	if (!framebuffer_)
  {		
		SDL_Rect *area=updateRects_;
		area->x=rect.x ;
		area->y=rect.y ;
		area->w=rect.w ;
		area->h=rect.h ;
		updateCount_=1 ;
	}
}

void SDLGUIWindowImp::ClearRect(GUIRect &r) 
{
  SDL_Rect rect;
  transform(r, &rect);
  SDL_FillRect(screen_, &rect,backgroundColor_) ;
} ;

// To the app we might have a smaller window
// than the effective one (PSP)

GUIRect SDLGUIWindowImp::GetRect() 
{
	return GUIRect(0,0,appWidth,appHeight) ;
}

// Pushback a SDL event to specify screen has to be redrawn.

void SDLGUIWindowImp::Invalidate() 
{
	SDL_Event event ;
	event.type=SDL_VIDEOEXPOSE ;
	SDL_PushEvent(&event) ;
}

void SDLGUIWindowImp::SetColor(GUIColor &c) 
{
	currentColor_=SDL_MapRGB(screen_->format,c._r&0xFF,c._g&0xFF,c._b&0xFF);
}

void SDLGUIWindowImp::Lock() 
{
  if (framebuffer_)
  {
    return;
  }

	if (SDL_MUSTLOCK(screen_)) 
  {
		SDL_LockSurface(screen_) ;
	}
}

void SDLGUIWindowImp::Unlock() 
{
  if (framebuffer_)
  {
    return;
  }

	if (SDL_MUSTLOCK(screen_)) 
  {
		SDL_UnlockSurface(screen_) ;
	}
}

void SDLGUIWindowImp::Flush()
{
#ifdef _SHOW_GP2X_
  drawGP2XOverlay() ;
  SDL_UpdateRect(screen_, 0, 0, rect_.Width(), rect_.Height());
#endif
  
	if ((!framebuffer_)&&(updateCount_!=0)) 
  {
		if (updateCount_<MAX_OVERLAYS)
    {
			SDL_UpdateRects(screen_,updateCount_, updateRects_);
		} 
    else
    {
			SDL_UpdateRect(screen_, 0, 0, screenRect_.Width(),screenRect_.Height());
		}
	}
	updateCount_=0;
}

void SDLGUIWindowImp::ProcessExpose() 
{
	_window->Update() ;
}

void SDLGUIWindowImp::ProcessQuit()
{
	GUIPoint p;
	GUIEvent e(p,ET_SYSQUIT) ;
	_window->DispatchEvent(e) ;	
} ;

void SDLGUIWindowImp::PushEvent(GUIEvent &event)
{
	SDL_Event sdlevent ;
	sdlevent.type=SDL_USEREVENT ;
	sdlevent.user.data1=&event ;
	SDL_PushEvent(&sdlevent) ;
} ;

void SDLGUIWindowImp::ProcessUserEvent(SDL_Event &event)
{
	GUIEvent *guiEvent=(GUIEvent *)event.user.data1 ;
	_window->DispatchEvent(*guiEvent) ;
	delete(guiEvent) ;
}

#ifdef _SHOW_GP2X_

void SDLGUIWindowImp::drawSub(int which)
{
	SDL_Rect srcRect ;
	srcRect.x=0 ;
	srcRect.y=0 ;
	srcRect.w=bitmaps_[which]->w ;
	srcRect.h=bitmaps_[which]->h ;

	SDL_Rect dstRect ;
	dstRect.x=gp2xAnchorX+originX_[which]*scaleFactor ;
	dstRect.y=gp2xAnchorY+originY_[which]*scaleFactor ;
	dstRect.w=srcRect.w ;
	dstRect.h=srcRect.h ;

	SDL_BlitSurface(bitmaps_[which], &srcRect,screen_, &dstRect);
}

void SDLGUIWindowImp::drawGP2X()
{
	drawSub(BM_FRONT) ;
}

void SDLGUIWindowImp::drawGP2XOverlay()
{
	int mask=EventDispatcher::GetInstance()->GetEventMask() ;
	Trace::Debug("got mask=%d",mask) ;
	drawSub(BM_OVERLAY) ;
	if (mask&(1<<EPBT_L)) drawSub(BM_LS) ;
	if (mask&(1<<EPBT_R)) drawSub(BM_RS) ;
	if (mask&(1<<EPBT_A)) drawSub(BM_A) ;
	if (mask&(1<<EPBT_B)) drawSub(BM_B) ;
	if (mask&(1<<EPBT_START)) drawSub(BM_START) ;
	if (mask&(1<<EPBT_SELECT)) drawSub(BM_SELECT) ;
	if (mask&(1<<EPBT_LEFT)) drawSub(BM_LEFT) ;
	if (mask&(1<<EPBT_UP)) drawSub(BM_UP) ;
	if (mask&(1<<EPBT_RIGHT)) drawSub(BM_RIGHT) ;
	if (mask&(1<<EPBT_DOWN)) drawSub(BM_DOWN) ;
}
#endif
