#include "ProjectView.h"
#include "BaseClasses/UIIntVarField.h"
#include "BaseClasses/UIActionField.h"
#include "BaseClasses/UITempoField.h"
#include "Application/Persistency/PersistencyService.h"
#include "System/System/System.h"
#include "Services/Midi/MidiService.h"
#include "Application/Views/ModalDialogs/MessageBox.h"
#include "Application/Views/ModalDialogs/NewProjectDialog.h"
#include "Application/Views/ModalDialogs/SelectProjectDialog.h"

#define ACTION_PURGE            MAKE_FOURCC('P','U','R','G')
#define ACTION_SAVE             MAKE_FOURCC('S','A','V','E')
#define ACTION_SAVE_AS          MAKE_FOURCC('S','V','A','S')
#define ACTION_LOAD             MAKE_FOURCC('L','O','A','D')
#define ACTION_QUIT             MAKE_FOURCC('Q','U','I','T')
#define ACTION_PURGE_INSTRUMENT MAKE_FOURCC('P','R','G','I')
#define ACTION_TEMPO_CHANGED    MAKE_FOURCC('T','E','M','P')

static void SaveAsProjectCallback(View &v,ModalView &dialog) {

    FileSystemService FSS;
    NewProjectDialog &npd=(NewProjectDialog &)dialog;

    if (dialog.GetReturnCode()>0) {
		std::string str_dstprjdir;
		std::string str_dstsmpdir;
		#ifdef PLATFORM_PSP
			str_dstprjdir = npd.GetName();
			str_dstsmpdir = npd.GetName() + "/samples/";
		#else
			std::string up = "../";
			str_dstprjdir = up + npd.GetName();
			str_dstsmpdir = up + npd.GetName() + "/samples/";
		#endif

		Path path_dstprjdir = Path(str_dstprjdir);
		Path path_dstsmpdir = Path(str_dstsmpdir);
		Path path_srcprjdir("project:");
		Path path_srcsmpdir("project:samples");

		Path path_srclgptdatsav = path_srcprjdir.GetPath() + "/lgptsav.dat";
		Path path_dstlgptdatsav = path_dstprjdir.GetPath() + "/lgptsav.dat";

		if (path_dstprjdir.Exists()) { Trace::Log("ProjectView", "Dst Dir '%s' Exist == true",
			path_dstprjdir.GetPath().c_str()); }
		else {
			Result result = FileSystem::GetInstance()->MakeDir(path_dstprjdir.GetPath().c_str());
			if (result.Failed()) {
				Trace::Log("ProjectView", "Failed to create dir '%s'", path_dstprjdir.GetPath().c_str());
				return;
			};

		result = FileSystem::GetInstance()->MakeDir(path_dstsmpdir.GetPath().c_str()) ;

		if (result.Failed()) {
			Trace::Log("ProjectView", "Failed to create sample dir '%s'", path_dstprjdir.GetPath().c_str());
			return;
		};

		FSS.Copy(path_srclgptdatsav,path_dstlgptdatsav);

		I_Dir *idir_srcsmpdir=FileSystem::GetInstance()->Open(path_srcsmpdir.GetPath().c_str());
		if (idir_srcsmpdir) {
				idir_srcsmpdir->GetContent("*");
				idir_srcsmpdir->Sort();
				IteratorPtr<Path>it(idir_srcsmpdir->GetIterator());
				for (it->Begin();!it->IsDone();it->Next()) {
					Path &current=it->CurrentItem();
					if (current.IsFile())
					{
						Path dstfile = Path((str_dstsmpdir+current.GetName()).c_str());
						Path srcfile = Path(current.GetPath());
						FSS.Copy(srcfile.GetPath(),dstfile.GetPath());
					}
				}
			}

		((ProjectView &)v).OnSaveAsProject((char*)str_dstprjdir.c_str());
		}
    }
}

static void LoadCallback(View &v,ModalView &dialog) {
	if (dialog.GetReturnCode()==MBL_YES) {
		((ProjectView &)v).OnLoadProject() ;
	}
} ;

static void QuitCallback(View &v,ModalView &dialog) {
	if (dialog.GetReturnCode()==MBL_YES) {
		((ProjectView &)v).OnQuit() ;
	}
} ;

static void PurgeCallback(View &v,ModalView &dialog) {
	((ProjectView &)v).OnPurgeInstruments(dialog.GetReturnCode()==MBL_YES) ;
} ;

ProjectView::ProjectView(GUIWindow &w,ViewData *data):FieldView(w,data) {

	lastClock_=0 ;
	lastTick_=0 ;

	project_=data->project_ ;

	GUIPoint position=GetAnchor() ;
	
	Variable *v=project_->FindVariable(VAR_TEMPO) ;
	UITempoField *f=new UITempoField(ACTION_TEMPO_CHANGED,position,*v,"tempo: %d [%2.2x]  ",60,400,1,10) ;
	T_SimpleList<UIField>::Insert(f) ;
	f->AddObserver(*this) ;
	tempoField_=f ;

	v=project_->FindVariable(VAR_MASTERVOL) ;
	position._y+=1 ;
	UIIntVarField *f1=new UIIntVarField(position,*v,"master: %d",10,200,1,10) ;
	T_SimpleList<UIField>::Insert(f1) ;

	v=project_->FindVariable(VAR_TRANSPOSE) ;
	position._y+=1 ;
	UIIntVarField *f2=new UIIntVarField(position,*v,"transpose: %3.2d",-48,48,0x1,0xC) ;
	T_SimpleList<UIField>::Insert(f2) ;

	position._y+=2 ;
	UIActionField *a1=new UIActionField("Compact Sequencer",ACTION_PURGE,position) ;
	a1->AddObserver(*this) ;
	T_SimpleList<UIField>::Insert(a1) ;

	position._y+=1 ;
	a1=new UIActionField("Compact Instruments",ACTION_PURGE_INSTRUMENT,position) ;
	a1->AddObserver(*this) ;
	T_SimpleList<UIField>::Insert(a1) ;

	position._y+=2 ;
	a1=new UIActionField("Load Song",ACTION_LOAD,position) ;
	a1->AddObserver(*this) ;
	T_SimpleList<UIField>::Insert(a1) ;

	position._y+=1 ;
	a1=new UIActionField("Save Song",ACTION_SAVE,position) ;
	a1->AddObserver(*this) ;
	T_SimpleList<UIField>::Insert(a1) ;

	position._y+=1 ;
	a1=new UIActionField("Save Song As",ACTION_SAVE_AS,position) ;
	a1->AddObserver(*this) ;
	T_SimpleList<UIField>::Insert(a1) ;

	v=project_->FindVariable(VAR_MIDIDEVICE) ;
	NAssert(v) ;
	position._y+=2 ;
	UIIntVarField *f3=new UIIntVarField(position,*v,"MIDI: %s",0,MidiService::GetInstance()->Size(),1,1) ;
	T_SimpleList<UIField>::Insert(f3) ;

	position._y+=2 ;
	a1=new UIActionField("Exit",ACTION_QUIT,position) ;
	a1->AddObserver(*this) ;
	T_SimpleList<UIField>::Insert(a1) ;

}

ProjectView::~ProjectView() {
}

void ProjectView::ProcessButtonMask(unsigned short mask,bool pressed) {

	if (!pressed) return ;

	FieldView::ProcessButtonMask(mask) ;

	if (mask&EPBM_R) {
		if (mask&EPBM_DOWN) {
			ViewType vt=VT_SONG;
			ViewEvent ve(VET_SWITCH_VIEW,&vt) ;
			SetChanged();
			NotifyObservers(&ve) ;
		}
	} else {
		if (mask&EPBM_START) {
   			Player *player=Player::GetInstance() ;
			player->OnStartButton(PM_SONG,viewData_->songX_,false,viewData_->songX_) ;
		}
	} ;
} ;

void ProjectView::DrawView() {
   
	Clear() ;

	GUITextProperties props ;
	GUIPoint pos=GetTitlePosition() ;

// Draw title

	char projectString[80] ;
	sprintf(projectString,"Project - Build %s%s_%s",PROJECT_NUMBER,PROJECT_RELEASE,BUILD_COUNT) ;

	SetColor(CD_NORMAL) ;
	DrawString(pos._x,pos._y,projectString,props) ;

	FieldView::Redraw() ;
	drawMap() ;
} ;

void ProjectView::Update(Observable &,I_ObservableData *data) {

	if (!hasFocus_) {
		return ;
	}

	int fourcc=(unsigned int)data ;

	UIField *focus=GetFocus() ;
	if (fourcc!=ACTION_TEMPO_CHANGED) {
		focus->ClearFocus() ;
		focus->Draw(w_) ;
		w_.Flush() ;
		focus->SetFocus() ;
	} else {
		focus=tempoField_ ;
	}
	Player *player=Player::GetInstance() ;

	switch (fourcc) {
		case ACTION_PURGE:
			project_->Purge() ;
			break ;
		case ACTION_PURGE_INSTRUMENT:
		{
			MessageBox *mb=new MessageBox(*this,"Purge unused samples from disk ?",MBBF_YES|MBBF_NO) ;
			DoModal(mb,PurgeCallback) ;
			break ;
		}
		case ACTION_SAVE:
			if (!player->IsRunning()) {
				PersistencyService *service=PersistencyService::GetInstance() ;
				service->Save() ;
			} else {
				MessageBox *mb=new MessageBox(*this,"Not while playing",MBBF_OK) ;
				DoModal(mb) ;
			}
			break ;
		case ACTION_SAVE_AS:
			if (!player->IsRunning()) {
				PersistencyService *service=PersistencyService::GetInstance() ;
				service->Save() ;
				NewProjectDialog *mb=new NewProjectDialog(*this) ;
				DoModal(mb,SaveAsProjectCallback) ;

			} else {
				MessageBox *mb=new MessageBox(*this,"Not while playing",MBBF_OK) ;
				DoModal(mb) ;
			}
			break ;

		case ACTION_LOAD:
		{
			if (!player->IsRunning()) {
				MessageBox *mb=new MessageBox(*this,"Load song and lose changes ?",MBBF_YES|MBBF_NO) ;
				DoModal(mb,LoadCallback) ;
			} else {
				MessageBox *mb=new MessageBox(*this,"Not while playing",MBBF_OK) ;
				DoModal(mb) ;
			}
			break ;
		}
		case ACTION_QUIT:
		{
			if (!player->IsRunning()) {
				MessageBox *mb=new MessageBox(*this,"Quit and lose faith ?",MBBF_YES|MBBF_NO) ;
				DoModal(mb,QuitCallback) ;
			} else {
				MessageBox *mb=new MessageBox(*this,"Duh ! Not while playing",MBBF_OK) ;
				DoModal(mb) ;
			}
			break ;
		}
		case ACTION_TEMPO_CHANGED:
			break ;
		default:
			NInvalid ;
			break ;
	} ;
    focus->Draw(w_) ;
	isDirty_=true ;
} ;

void ProjectView::OnPurgeInstruments(bool removeFromDisk) {
	project_->PurgeInstruments(removeFromDisk) ;
} ;

void ProjectView::OnLoadProject() {
	ViewEvent ve(VET_QUIT_PROJECT) ;
	SetChanged();
	NotifyObservers(&ve) ;
} ;

void ProjectView::OnSaveAsProject(char * data) {
        ViewEvent ve(VET_SAVEAS_PROJECT,data) ;
	SetChanged();
	NotifyObservers(&ve) ;
} ;

void ProjectView::OnQuit() {
	ViewEvent ve(VET_QUIT_APP) ;
	SetChanged();
	NotifyObservers(&ve) ;
} ;
