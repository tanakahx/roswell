#include "util.h"

char* homedir(void) {
  char *c;
  char *postfix="_HOME";
  struct passwd * pwd;
  char *env=NULL;
  int i;

  c=s_cat2(upcase(q_(PACKAGE)),q_(postfix));
  env=getenv(c);
  s(c);
  if(env) {
    return append_trail_slash(env);
  }
#ifdef HAVE_WINDOWS_H
  TCHAR szAppData[MAX_PATH];
  if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, szAppData))) {
    c=q_(szAppData);
  }
#else
  pwd= getpwnam(getenv("USER"));
  if(pwd) {
    c=q_(pwd->pw_dir);
  }
#endif
  else {
    /* error? */
    return NULL;
  }
  return s_cat(append_trail_slash(c),NULL);
}

char* configdir(void) {
  char* home=homedir();
  if(home) {
    return s_cat(home,q_("."),q(PACKAGE),q(SLASH),NULL);
  }
  return NULL;
}

char* lispdir(void) {
    char *ros_bin=pathname_directory(truename(which(argv_orig[0])));
    char* ros_bin_lisp=cat(ros_bin,"lisp",SLASH,NULL);
    char* lisp_path;
    s(ros_bin);
    if(directory_exist_p(ros_bin_lisp)) {
      lisp_path=ros_bin_lisp;
    }else {
      s(ros_bin_lisp);
      lisp_path=q(LISP_PATH);
#ifdef _WIN32
      substitute_char('\\','/',lisp_path);
#endif
      lisp_path=append_trail_slash(lisp_path);
    }
    return lisp_path;
}

char* truename(const char* path) {
#ifndef _WIN32
  char* ret=realpath(path,NULL);
  if(ret)
    return ret;
#else
  /* TBD */
#endif
  return which((char*)path);
}

char* pathname_directory(char* path) {
  int i;
  char* ret;
  for(i=strlen(path)-1;i>=0&&path[i]!=SLASH[0];--i);
  ret=append_trail_slash(subseq(path,0,i));
  s(path);
  return ret;
}

char* file_namestring(char* path) {
  int i;
  char* ret;
  for(i=strlen(path)-1;i>=0&&path[i]!='/';--i);
  if(path[i]=='/') {
    ret=subseq(path,i+1,0);
    s(path);
  }else {
    ret=path;
  }
  return ret;
}

char* ensure_directories_exist (char* path) {
  int len = strlen(path);
  if(!directory_exist_p(path)) {
    if(len) {
      for(--len;(path[len]!=SLASH[0]||len==-1);--len);
      path=subseq(path,0,len+1);
    }
#ifndef _WIN32
    char* cmd=s_cat2(q("mkdir -p "),path);
#else
    char* cmd=s_cat(q("cmd /C md "),path,q(" 2>NUL"),NULL);
#endif
    if(system(cmd)!=0) {
      fprintf(stderr,"failed:%s\n",cmd);
      return NULL;
    };
    s(cmd);
  }
  return path;
}

int directory_exist_p (char* path) {
#ifndef HAVE_WINDOWS_H
# ifdef HAVE_SYS_STAT_H
  struct stat sb;
  int ret=0;
  if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
    ret=1;
  }
  return ret;
# else
#  error not imprement directory_exist_p
# endif
#else
  WIN32_FIND_DATA fd;
  char *p=cat(path,"*.*",NULL);
  HANDLE dir=FindFirstFile(p,&fd);
  s(p);
  if(dir==INVALID_HANDLE_VALUE)
    return 0;
  FindClose(dir);
  return 1;
#endif
}

int file_exist_p (char* path) {
#ifdef HAVE_SYS_STAT_H
  struct stat sb;
  int ret=0;
  if (stat(path, &sb) == 0 && S_ISREG(sb.st_mode)) {
    ret=1;
  }
  return ret;
#else
  return 1;
#endif
}

int change_directory(const char* path) {
#ifndef _WIN32
  return chdir(path);
#else
  return _chdir(path);
#endif  
}

int delete_directory(char* pathspec,int recursive) {
#ifndef _WIN32
  char* cmd;
  int ret;
  if(recursive) {
    cmd=s_cat2(q("rm -rf "),q(pathspec));
  }else {
    cmd=s_cat2(q("rmdir "),q(pathspec));
  }
  ret=system(cmd);
  s(cmd);
  return ret==0;
#else
  if(!recursive) {
    return(!!RemoveDirectory(pathspec));
  }else {
    SHFILEOPSTRUCT fs;
    ZeroMemory(&fs, sizeof(SHFILEOPSTRUCT));
    fs.hwnd = NULL;
    fs.wFunc = FO_DELETE;
    fs.pFrom = pathspec;
    fs.pTo = NULL;
    fs.fFlags=FOF_SIMPLEPROGRESS|FOF_NOCONFIRMATION;
    return (SHFileOperation(&fs) == 0);
  }
#endif  
}
