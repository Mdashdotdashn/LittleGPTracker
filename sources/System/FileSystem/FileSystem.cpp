#include "FileSystem.h"
#include "Application/Utils/wildcard.h"	
#include "System/Console/Trace.h"
#include <algorithm>

T_SimpleList<Path::Alias> Path::aliases_(true) ;

using namespace std ;

Path::Path():type_(FT_UNKNOWN),gotType_(false) {
	path_=(char *)malloc(1) ;
	strcpy(path_,"") ;
} ;

Path::Path(const char *path):type_(FT_UNKNOWN),gotType_(false) {
	path_=(char *)malloc((int)strlen(path)+1) ;
	strcpy(path_,path) ;
} ;

Path::Path(const std::string &path):type_(FT_UNKNOWN),gotType_(false) {
	path_=(char *)malloc(path.size()+1) ;
	strcpy(path_,path.c_str()) ;
} ;

Path::Path(const Path &other) {
	path_=(char *)SYS_MALLOC((int)strlen(other.path_)+1) ;
	strcpy(path_,other.path_) ;
	gotType_=other.gotType_;
	type_=other.type_ ;
} ;

Path &Path::operator=(const Path &other) {
	SAFE_FREE(path_) ;
	path_=(char *)SYS_MALLOC((int)strlen(other.path_)+1) ;
	strcpy(path_,other.path_) ;
	return *this ;
} ;

Path::~Path() {
	SYS_FREE (path_) ;
};

std::string Path::GetPath() const
{
	std::string path=path_ ;
	std::string::size_type pos ;

	bool search=true ;
	while ( (search)&&((pos= path.find (":",0))!=string::npos)) {
		string aliasString = path.substr(0,pos);
		const char *aliasPath=resolveAlias(aliasString.c_str()) ;
		string forward ;
		if (aliasPath) {
			forward+=string(aliasPath)+"/" ;
			path=forward+path.substr(pos+1) ;
		} else {
			search=false ;
		} ;
	} ;
	fullPath_=path ;
	return fullPath_ ;
} ;

std::string Path::GetCanonicalPath() {
	std::string copy=GetPath(); 
	std::string::size_type pos ;
	while ((pos=copy.find("\\")) != std::string::npos) {
		std::string rpart=copy.substr(pos+1) ;
		copy=copy.substr(0,pos) ;
		copy+="/" ;
		copy+=rpart ;
	} ;
	return copy ;
} ;

Path Path::Descend(const std::string& leaf)
{
  std::string currentPath = GetPath();
  if (currentPath[currentPath.size()-1] != '/')
  {
    currentPath += "/";
  }
  return Path(currentPath+leaf);
}

void Path::getType() {
	if (!gotType_) {
		gotType_=true ;
		type_=FileSystem::GetInstance()->GetFileType(path_) ;
	} ;
} ;

std::string Path::GetName() {

	unsigned int index=0 ;
	for (unsigned int i=0;i<strlen(path_);i++) {
		if (path_[i]=='/') {
			index=i ;
		} ;
	} ;
	if (index!=0) index++ ;
	return std::string(path_+index);
} ;

int Path::Compare(const Path &other) {
	return strcasecmp(path_,other.path_) ;
} ;

bool Path::Exists() {
	getType() ;
	return type_!=FT_UNKNOWN ;
} ;

bool Path::IsFile() {
	getType() ;
	return type_==FT_FILE ;
} ;

bool Path::IsDirectory() {
	getType() ;
	return type_==FT_DIR ;
} ;

bool Path::Matches(const char *pattern) {
	std::string name=GetName() ;
	std::transform(name.begin(), name.end(),name.begin(), ::tolower);
	return wildcardfit(pattern,name.c_str())  == 1 ;
} ;

Path Path::GetParent() {
	std::string current=GetCanonicalPath() ;
	std::string::size_type index=current.rfind("/",current.size()-2) ;
	std::string parentPath=current.substr(0,index) ;
	Path parent(parentPath) ;
	return parent ;
}


void Path::SetAlias(const char *alias,const char *path) {
	IteratorPtr<Alias> it(aliases_.GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		Alias &current=it->CurrentItem() ;
		if (!strcmp(current.GetAliasName(),alias)) {
				current.SetPath(path) ;
				return ;	
			} ;
	};
	Alias *a=new Alias(alias,path) ;
	aliases_.Insert(a) ;
} ;

const char *Path::resolveAlias(const char *alias) {
	IteratorPtr<Alias> it(aliases_.GetIterator()) ;
	for (it->Begin();!it->IsDone();it->Next()) {
		Alias &current=it->CurrentItem() ;
		if (!strcmp(current.GetAliasName(),alias)) {
			return current.GetPath() ;	
		} ;
	};
	return 0 ;
} ;
Path::Alias::Alias(const char *alias,const char *path) {
	alias_=alias ;
	path_=path ;
} ;

const char *Path::Alias::GetAliasName() {
	return alias_.c_str() ;
} ;

const char *Path::Alias::GetPath() {
	return path_.c_str() ;
}

void Path::Alias::SetAliasName(const char *alias) {
	alias_=alias ;
} ;

void Path::Alias::SetPath(const char *path) {
	path_=path ;
} ;

int FileSystemService::Copy(const Path &src,const Path &dst)
{
  const int bufsize=4096;
  char buffer[bufsize];
  int  count=0;
  int  nbwrite=-1;

  FileSystem * fs=FileSystem::GetInstance() ;
  I_File     * isrc=fs->Open(src.GetPath().c_str(),"r");
  I_File     * idst=fs->Open(dst.GetPath().c_str(),"w");

  Trace::Log("FS","FileSystemService::Copy %s to %s",
  src.GetPath().c_str(), dst.GetPath().c_str());
  if (isrc)
    Trace::Log("FS","src open ok");
  else {
    Trace::Log("FS","src open fail");
    return nbwrite;
  }
  if (idst)
    Trace::Log("FS","dst open ok");
  else {
    Trace::Log("FS","dst open fail");
    return nbwrite;
  }

  while (count=isrc->Read(buffer,sizeof(char),bufsize))
    {
      idst->Write(buffer,sizeof(char),count);
      nbwrite++;
    }

  isrc->Close();
  idst->Close();
  return nbwrite;
}