
#include "NDSGUIWindowImp.h"
#include "UIFramework/SimpleBaseClasses/GUIWindow.h"
#include "System/IO/Trace.h"
#include "System/System/System.h"
#include <string.h>
#include <nds.h>

NDSGUIWindowImp *instance_ ;

NDSGUIWindowImp::NDSGUIWindowImp(GUICreateWindowParams &p) {

	GUIRect windowRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT) ;
	rect_=windowRect ;
	instance_=this ;
	framebuffer_=VRAM_A ;

} ;

NDSGUIWindowImp::~NDSGUIWindowImp() {
}

void NDSGUIWindowImp::Clear(GUIColor &c) {
     
     u16 *pixel=framebuffer_ ;
     backColor_=RGB15(c._r>>1,c._g>>1,c._b>>1) ;
     u32 colcol = backColor_ | backColor_ << 16;
     swiFastCopy(&colcol, framebuffer_, 192*256*2/4 | COPY_MODE_FILL);     

     
} 

#define FONT_WIDTH 1024
#define FONT_COUNT 127

static const unsigned char font[]= {
	#include "Resources/font.h"
};

void NDSGUIWindowImp::DrawString(char *string,GUIPoint &pos,GUITextProperties& p) {

	int len=int(strlen(string)) ;
	int xx=pos._x ;
	int yy=pos._y ;

    if (len>39) {
		len=39 ;
		string[39]=0 ;
	} ;

	for (int l=0;l<len;l++) {
		if ( string[l]>126 ) string[l]=0;
		const unsigned char *src=font+(string[l]*8) ;
		unsigned short *dest=(framebuffer_) + (yy*SCREEN_WIDTH) + xx;
  		for (int y = 0; y < 8; y++) {
				for (int x = 0; x < 8; x++) {
					*dest =((*src)^(unsigned char)p.invert_)?backColor_:frontColor_ ;
					dest++;
					src++ ;
        		}
			if (y<7) src+=FONT_WIDTH-8 ;
			dest+=(rect_.Width()-8) ;

  		}

         xx+=8 ;
     }     
} ;

void NDSGUIWindowImp::SetColor(GUIColor &c) {
     frontColor_=RGB15(c._r>>1,c._g>>1,c._b>>1) ;
}

GUIRect NDSGUIWindowImp::GetRect() {
        return rect_ ;
}

void NDSGUIWindowImp::Invalidate() {
     NDSEvent *event=new NDSEvent() ;
     event->type_=NDSET_REDRAW ;
     NDSEventQueue::GetInstance()->Push(*event) ;     
} ;

void NDSGUIWindowImp::Flush() {
} ;

void NDSGUIWindowImp::Save() {
} ;

void NDSGUIWindowImp::Restore() {
} ;

void NDSGUIWindowImp::ProcessEvent(NDSEvent &event) {
     switch(event.type_) {
     case NDSET_REDRAW:
//     	  instance_->currentBuffer_^=0x01 ;
	      instance_->_window->Update() ;
//	      gp_setFramebuffer(instance_->framebuffer_[instance_->currentBuffer_],1); 	
          break ;
     case NDSET_ESC:
    	System *system=System::GetInstance() ;
    	unsigned long now=system->GetClock() ;
    	GUIEvent event(EKT_ESCAPE,ET_KEYDOWN,now,0,0,0) ;
    	instance_->_window->DispatchEvent(event) ;
          break ;          
     }     
} ;

static GUIEventPadButtonType eventMapping[10]= { EPBT_A,
EPBT_B,EPBT_SELECT,EPBT_START,EPBT_RIGHT,EPBT_LEFT,EPBT_UP,EPBT_DOWN,EPBT_R,EPBT_L } ;
                                                                                
void NDSGUIWindowImp::ProcessButtonChange(u16 changeMask,u16 buttonMask) {

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
}
