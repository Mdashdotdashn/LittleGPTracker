#include "PICOFileSystem.h"
#include "Adapters/PICO/sdcard/sdcard.h"
#include "Application/Utils/wildcard.h"
#include "system/Console/Trace.h"

PICODir::PICODir(const char *path) : I_Dir(path){};

void PICODir::GetContent(char *mask) {
  Trace::Log("FILESYSTEM", "GetContent %s with mask %s", path_, mask);
  Empty();
  FsBaseFile dir;

  if (!dir.open(path_)) {
    Trace::Error("Failed to open %s", path_);
    return;
  }

  if (!dir.isDir()) {
    Trace::Error("Path \"%s\" is not a directory", path_);
    return;
  }

  FsBaseFile entry;
  while (entry.openNext(&dir, O_READ)) {
    char current[128];
    entry.getName(current, 128);
    char *c = current;
    while (*c) {
      *c = tolower(*c);
      c++;
    }

    if (wildcardfit(mask, current)) {
      entry.getName(current, 128);
      std::string fullpath = path_;
      if (path_[strlen(path_) - 1] != '/') {
        fullpath += "/";
      }
      fullpath += current;
      Path *path = new Path(fullpath.c_str());
      Insert(path);
    }
  }
  dir.close();
}

PICOFile::PICOFile(FsBaseFile file) { file_ = file; };

int PICOFile::Read(void *ptr, int size, int nmemb) {
  return file_.read(ptr, size * nmemb);
}

int PICOFile::GetC() { return file_.read(); }

int PICOFile::Write(const void *ptr, int size, int nmemb) {
  return file_.write(ptr, size * nmemb);
}

void PICOFile::Printf(const char *fmt, ...) {
  Trace::Debug("PICOFileSystem::Printf called...");
  // TODO: What is this for?
}

void PICOFile::Seek(long offset, int whence) {
  switch (whence) {
  case SEEK_SET:
    file_.seek(offset);
    break;
  case SEEK_CUR:
    file_.seekCur(offset);
    break;
  case SEEK_END:
    file_.seekEnd(offset);
    break;
  default:
    Trace::Error("Invalid seek whence: %s", whence);
  }
}

long PICOFile::Tell() { return file_.curPosition(); }

void PICOFile::Close() { file_.close(); }

PICOFileSystem::PICOFileSystem() {

  // Check for the common case, FAT filesystem as first partition
  Trace::Log("FILESYSTEM", "Try to mount SD Card");
  if (SD_.begin(SD_CONFIG)) {
    Trace::Log("FILESYSTEM",
               "Mounted SD Card FAT Filesystem from first partition");
    return;
  }

  // Do we have any kind of card?
  if (!SD_.card() || SD_.sdErrorCode() != 0) {
    Trace::Log("FILESYSTEM", "No SD Card present");
    return;
  }
  // Try to mount the whole card as FAT (without partition table)
  if (static_cast<FsVolume *>(&SD_)->begin(SD_.card(), true, 0)) {
    Trace::Log("FILESYSTEM",
               "Mounted SD Card FAT Filesystem without partition table");
    return;
  }
}

I_File *PICOFileSystem::Open(const char *path, char *mode) {
  Trace::Log("FILESYSTEM", "Open file %s, mode: %s", path, mode);
  oflag_t rmode;
  switch (*mode) {
  case 'r':
    rmode = FILE_READ; // O_RDONLY
    break;
  case 'w':
    rmode = FILE_WRITE; // O_RDWR | O_CREAT | O_AT_END
    break;
  default:
    Trace::Error("Invalid mode: %s", mode);
    return 0;
  }

  FsBaseFile file;
  PICOFile *wFile = 0;
  if (file.open(path, rmode)) {
    wFile = new PICOFile(file);
  }
  return wFile;
};

I_Dir *PICOFileSystem::Open(const char *path) {
  Trace::Log("FILESYSTEM", "Open dir %s", path);
  return new PICODir(path);
};

Result PICOFileSystem::MakeDir(const char *path) {
  Trace::Log("FILESYSTEM", "Make dir %s", path);
  if (!SD_.mkdir(path, false)) {
    std::string result = "Could not create path ";
    result += path;
    return Result(result);
  }
  return Result::NoError;
};

FileType PICOFileSystem::GetFileType(const char *path) {
  Trace::Log("FILESYSTEM", "Get File type %s", path);
  FileType filetype;
  FsBaseFile file;
  if (!file.open(path, O_READ)) {
    return FT_UNKNOWN;
  }
  if (file.isDirectory()) {
    Trace::Debug("%s is directory", path);
    filetype = FT_DIR;
  } else if (file.isFile()) {
    Trace::Debug("%s is regular file", path);
    filetype = FT_FILE;
  } else {
    filetype = FT_UNKNOWN;
  }

  file.close();
  return filetype;
};