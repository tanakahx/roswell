#include "opt.h"

LVal proc_alias(LVal arg,struct proc_opt *popt) {
  char* arg0=firsts(arg);
  char* builtin[][2]= {
    {"-V","version"},
    {"-h","help"},
    {"-?","help"},
    //{"build","dump executable"},
  };
  int i;
  cond_printf(1,"proc_alias: arg0=%s repeat=%d\n",arg0,sizeof(builtin)/sizeof(char*[2]));
  for (i=0;i<sizeof(builtin)/sizeof(char*[2]);++i)
    if(!strcmp(builtin[i][0],arg0))
      return conss(q_(builtin[i][1]),nrest(arg));
  return arg;
}

LVal proc_options(LVal arg,struct proc_opt *popt) {
  char* arg0=firsts(arg);
  cond_printf(1,"proc_options:%s:%s\n",arg0,popt->name);
  if(arg0[0]=='-' &&
     arg0[1]=='-') { /*long option*/
    LVal p;

    for(p=popt->option;p;p=Next(p)) {
      struct sub_command* fp=firstp(p);
      if(strcmp(&arg0[2],fp->name)==0) {
        int result= fp->call(arg,fp);
        if(fp->terminating) {
          cond_printf(1,"terminating:%s\n",arg0);
          exit(result);
        }else
          return nnthcdr(result,arg);
      }
    }
    cond_printf(1,"dispatch:invalid? %s\n",arg0);
  }else { /*short option*/
    if(arg0[1]!='\0') {
      LVal p;
      for(p=popt->option;p;p=Next(p)) {
        struct sub_command* fp=firstp(p);
        if(fp->short_name&&strcmp(arg0,fp->short_name)==0) {
          int result= fp->call(arg,fp);
          if(fp->terminating) {
            cond_printf(1,"terminating:%s\n",arg0);
            exit(result);
          }else
            return nnthcdr(result,arg);
        }
      }
    }
  }
  return nnthcdr(1,arg);
}

LVal proc_set(LVal arg,struct proc_opt *popt,int pos) {
  char* arg0=firsts(arg);
  char *l,*r;
  l=subseq(arg0,0,pos);
  r=subseq(arg0,pos+1,0);
  if(r)
    set_opt(&local_opt,l,r);
  else{
    struct opts* opt=global_opt;
    struct opts** opts=&opt;
    unset_opt(opts, l);
    global_opt=*opts;
  }
  s(l),s(r);
  return nnthcdr(1,arg);
}

void proc_cmd(LVal arg,struct proc_opt *popt) {
  char* arg0=firsts(arg);
  LVal p,p2=0;
  
  cond_printf(1,"proc_cmd:\n");
  /* search internal commands.*/
  for(p=popt->command;p;p=Next(p)) {
    struct sub_command* fp=firstp(p);
    if(fp->name) {
      cond_printf(1,"fname=%s,arg0=%s\n",fp->name,arg0);
      if(strcmp(fp->name,arg0)==0)
        exit(fp->call(arg,fp));
      if(strcmp(fp->name,"*")==0)
        p2=p;
    }
  }

  if(popt->top && position_char(".",arg0)==-1) {
    /* local commands*/
    char* cmddir=configdir();
    char* cmdpath=cat(cmddir,arg0,".ros",NULL);
    if(directory_exist_p(cmddir) && file_exist_p(cmdpath))
      dispatch (conss(q_(cmdpath),arg),popt);
    s(cmddir),s(cmdpath);
    /* systemwide commands*/
    cmddir=subcmddir();
    cmdpath=cat(cmddir,arg0,".ros",NULL);
    if(directory_exist_p(cmddir)) {
      if(file_exist_p(cmdpath))
        dispatch(conss(q_(cmdpath),arg),popt);
      s(cmdpath);cmdpath=cat(cmddir,"+",arg0,".ros",NULL);
      if(file_exist_p(cmdpath))
        dispatch(conss(q_(cmdpath),arg),popt);
    }
    s(cmddir),s(cmdpath);
  }
  if(p2) {
    struct sub_command* fp=firstp(p2);
    exit(fp->call(arg,fp));
  }
  fprintf(stderr,"invalid command\n");
  dispatch(stringlist("help",NULL),&top);
}

LVal dispatch(LVal arg,struct proc_opt *popt) {
  int pos;
  cond_printf(1,"dispatch:%s,name=%s\n",firsts(arg),popt->name);
  arg=proc_alias(arg,popt);
  char* arg0=firsts(arg);
  if(arg0[0]=='-' || arg0[0]=='+')
    return proc_options(arg,popt);
  else if((pos=position_char("=",arg0))!=-1)
    return proc_set(arg,popt,pos);
  else
    proc_cmd(arg,popt);
  return nnthcdr(1,arg);
}

void dispatch_init(struct proc_opt *popt,char* name_) {
  popt->name=q_(name_);
  popt->option=(LVal)0;
  popt->command=(LVal)0;
  popt->top=0;
}