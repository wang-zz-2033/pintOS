#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

//NEW
#include "threads/vaddr.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/input.h"
#include "process.h"
#include "string.h"
#include "devices/shutdown.h"
#define MAXCALL 21             //或许你可以把这里改一下
#define MaxFiles 200

#define STDIN 0
#define STDOUT 1
#define SYS_CALL_NUM 20

static void syscall_handler (struct intr_frame *);

//NEW
typedef int pid_t;

typedef void (*CALL_PROC)(struct intr_frame *);
//CALL_PROC syscalls[MAXCALL];                       //还用吗？
static void (*syscalls[SYS_CALL_NUM])(struct intr_frame *);

void sys_halt (struct intr_frame *f);  //FIN
void sys_exit (struct intr_frame *f);  //FIN
//void exit(int status);        //FIN
void sys_exec (struct intr_frame *f);  //
void sys_wait (struct intr_frame *f);  //
void sys_create (struct intr_frame *f);  //FIN
void sys_remove (struct intr_frame *f);  //
void sys_open (struct intr_frame *f);    //FIN R U SURE?
void sys_filesize (struct intr_frame *f);//
void sys_read (struct intr_frame *f );   //
void sys_write (struct intr_frame *f);   //no f?    FIN
void sys_seek (struct intr_frame *f);    //
void sys_tell (struct intr_frame *f);    //
void sys_close (struct intr_frame *f);   //

//struct file_node *GetFile(struct thread *t, int fd);
void halt (void);
void exit (int status);
pid_t exec (const char *cmd_line);
int wait (pid_t pid);
bool create (const char *file, unsigned initial_size);
bool remove (const char *file);
int open (const char *file);
int filesize (int fd);
int read (int fd, void *buffer, unsigned size);
int write (int fd, const void *buffer, unsigned size);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);

static struct file *find_file_by_fd (int fd);
static struct fd_entry *find_fd_entry_by_fd (int fd);
static int alloc_fid (void);
static struct fd_entry *find_fd_entry_by_fd_in_process (int fd);

/*
file descriptors
*/
struct fd_entry{
  int fd;
  struct file *file;
  struct list_elem elem;
  struct list_elem thread_elem;
};

static struct list file_list;
static struct lock file_lock; //......?

///
static int alloc_fid (void)
{
  static int fid = 2;
  return fid++;
}

static struct fd_entry *
find_fd_entry_by_fd_in_process (int fd)
{
  struct fd_entry *ret;
  struct list_elem *l;
  struct thread *t;

  t = thread_current ();

  for (l = list_begin (&t->fd_list); l != list_end (&t->fd_list); l = list_next (l))
    {
      ret = list_entry (l, struct fd_entry, thread_elem);
      if (ret->fd == fd)
        return ret;
    }

  return NULL;
}

static struct file *
find_file_by_fd (int fd)
{
  struct fd_entry *ret;

  ret = find_fd_entry_by_fd (fd);
  if (!ret)
    return NULL;
  return ret->file;
}

//任务3 系统调用
  
//需要实现以下功能：
//void halt (void)   停止运行OS,用不上这个
 
void exit (int status){
  //终止当前用户程序，将状态返回到内核。
 //如果该进程的父进程在等待（wait()），返回这个状态（我猜是status）。返回0为成功，非0为错误
  struct thread *t;
  struct list_elem *l;
  t = thread_current();
  while (!list_empty (&t->fd_list))
  {
    l = list_begin (&t->fd_list);
    close (list_entry (l, struct fd_entry, thread_elem)->fd);
  }

  t->exit_status = status;
  thread_exit ();
}  
 
 
 pid_t exec (const char *cmd_line){
//运行名称在cmd_line给出的可执行文件
// 并传递任何给定的参数，返回新进程的pid。如果程序无法加载，返回pid-1，or非有效的pid
// （需要通过 同步 让父进程知道子进程是否加载成功可执行文件）
  return process_execute(cmd_line); //sure?
 } 
 
 int wait (pid_t pid){
//    等待子进程pid并检索子进程的退出状态  "HARD TO FINISH"
//  详情见网页http://web.stanford.edu/~ouster/cgi-bin/cs140-spring20/pintos/pintos_3.html
   return process_wait(pid);
 }    
 
 bool create (const char *file, unsigned initial_size){
//   创建名为file的文件，初始大小i_s个字节
//  成功返回true，失败返回false。创建新文件不会打开它
  return filesys_create(file,initial_size);
 } 
 bool remove (const char *file){
//   删除名为file的文件
//  成功返回true，失败返回false。
//  无论文件打开还是关闭，都可以删除，并且删除打开的文件不会将其关闭。
  return filesys_remove(file);
 }
 
 int open (const char *file){
//  打开名为file的文件
//  返回一个称为“文件描述符”（fd）的非负整数句柄；如果无法打开文件，则返回-1。
//  为控制台保留编号为0和1的文件描述符：fd 0（STDIN_FILENO）是标准输入，
//  fd 1（STDOUT_FILENO）是标准输出。
//  该open系统调用永远不会返回任何这些文件描述符，这是因为只有如下明确描述的系统调用参数有效的。
//  每个进程都有一组独立的文件描述符。文件描述符不被子进程继承。
//  当单个文件多次打开（无论是通过单个进程还是通过不同进程）时，每个open文件都返回一个新的文件描述符。
//  单个文件的不同文件描述符在对的单独调用中被独立关闭，close并且它们不共享文件位置。
  struct file* f = filesys_open(file);
  if(f == NULL){
    //file_close(f);
    return -1;
  }

  struct fd_entry *fde = (struct fd_entry *)malloc(sizeof(struct fd_entry));
  if(fde == NULL){
    file_close(f);
    return -1;
  }

  struct thread *cur = thread_current();
  fde->fd = alloc_fid();
  fde->file = f;
  list_push_back(&cur->fd_list,&fde->thread_elem);
  list_push_back(&file_list,&fde->elem);

  return fde->fd;

 } 
 
 int filesize (int fd){
   //返回以fd打开的文件的大小（以字节为单位）
   struct file *file = find_file_by_fd(fd);
   if(file == NULL)
    exit(-1);
  return file_length(file);
 } 
 
 int read (int fd, void *buffer, unsigned size){
  if(fd == STDIN){ // keyboard
    for(unsigned int i=0;i< size ; i++){
      *((char **)buffer)[i] = input_getc();
    }
    return size;
  }else{
    struct file *f = find_file_by_fd(fd);

    if(f == NULL){
      return -1;
    }
    return file_read(f,buffer,size);
  }
  //  从fd打开的文件中读取size字节到 buffer。
  //  返回实际读取的字节数（文件末尾为0）
  //  如果无法读取文件（由于文件末尾以外的条件），则返回-1。Fd 0使用 input_getc()从键盘读取
  //  input_getc()---在devices/input.c里面 --在缓冲区检索key，没有就等待
 }
  
 
 int write (int fd, const void *buffer, unsigned size){
// 将buffer中的size字节写入打开的文件fd中。
// 返回实际写入的字节数，如果某些字节无法写入，则可能小于size。
// 如果根本没法写入，则返回0
// Fd 1写入控制台。
// 您写入控制台的代码至少应在一次调用中将所有buffer写入putbuf()，只要大小不超过几百个字节即可。
  if(fd==STDOUT){ 
      putbuf((char *) buffer,(size_t)size);
      return (int)size;
  }else{
    struct file *f = find_file_by_fd(fd);
    if(f==NULL){
      exit(-1);
    }
    return (int) file_write(f,buffer,size);

  }
 }
 
 
 void seek (int fd, unsigned position){
//    将打开文件fd中要读取或写入的下一个字节更改为 position，从文件开头开始以字节表示。
//  （因此，位置0是文件的开始。）
//  在文件的当前末尾进行查找不是错误。以后的读取将获得0字节，表示文件结束。
//  以后的写入将扩展文件，并用零填充所有未写入的间隙。
  struct file *fi = find_file_by_fd(fd);
  if(fi == NULL)
    exit(-1);
  file_seek(fi,position); //unsigned or unsigned int?
 }
 
 
 unsigned tell (int fd){
  //  返回打开文件fd中要读取或写入的下一个字节的位置
  //  以从文件开头开始的字节数表示。
    struct file *f = find_file_by_fd(fd);
    if(f == NULL)
      exit(-1);
    return file_tell(f);
 } 
 
 void close (int fd){
//     关闭文件描述符fd
//  退出或终止进程会隐式关闭其所有打开的文件描述符
//  就像为每个进程调用此函数一样。
    struct fd_entry *f = find_fd_entry_by_fd_in_process(fd);
    if(f == NULL)
      exit(-1);
    file_close(f->file);
    list_remove(&f->elem);
    list_remove(&f->thread_elem);
    free(f);

 }
 
//  --------------------------------------------------------------------------------
//  
//  实现系统调用的方法： 提供在用户栈读取+写入的方法。小心用户传入空指针（空指针处理见3.1.5
//  
//  可以看的其他文件：
//       lib / syscall-nr.h     系统调用号
//       lib / user / syscall.h 系统调用原型   可不看
//       lib / user / syscall.c 用c程序调用每个系统调用的功能
//  
//  --------------------------------------------------------------------------------
//  根据百度文库的参考，使用了syscalls[系统调用号]实现系统调用

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  //新增初始化syscalls[]数组为NULL
  for(int i=0; i <=SYS_CALL_NUM ; i++){  //MAXCALL == 21
    syscalls[i] = NULL;
  }

  syscalls[SYS_WRITE] = &sys_write;
  syscalls[SYS_EXIT]  = &sys_exit;
  syscalls[SYS_CREATE] = &sys_create;
  syscalls[SYS_OPEN] = &sys_open;
  syscalls[SYS_CLOSE] = &sys_close;
  syscalls[SYS_READ] = &sys_read;
  syscalls[SYS_FILESIZE] = &sys_filesize;
  syscalls[SYS_EXEC] = &sys_exec;
  syscalls[SYS_WAIT] = &sys_wait;
  syscalls[SYS_SEEK] = &sys_seek;
  syscalls[SYS_REMOVE] = &sys_remove;
  syscalls[SYS_TELL] = &sys_tell;
  syscalls[SYS_HALT] = &sys_halt;

  lock_init(&file_lock);
  list_init(&file_list);

}

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */

   //uint8_t unsigned char
   //uaddr is a address
static int
get_user (const uint8_t *uaddr)
{
    //printf("%s\n", "call get user");
  if(!is_user_vaddr((void *)uaddr)){
    return -1;
  }
  if(pagedir_get_page(thread_current()->pagedir,uaddr)==NULL){
    return -1;
  }
  //printf("%s\n","is_user_vaddr" );
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}

/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static bool
put_user (uint8_t *udst, uint8_t byte)
{
  if(!is_user_vaddr(udst))
    return false;
  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
       : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}

/*
check the following address is valid:
if one of them are not valid, the function will return false
*/
bool is_valid_pointer(void* esp,uint8_t argc){
  uint8_t i = 0;
for (; i < argc; ++i)
{
  // if (get_user(((uint8_t *)esp)+i) == -1){
  //   return false;
  // }
  if((!is_user_vaddr(esp))||(pagedir_get_page(thread_current()->pagedir,esp)==NULL)){
    return false;
  }
}
return true;
}

/*
return true if it is a valid string
*/
bool is_valid_string(void *str){
  //return true;
  int ch=-1;
while((ch=get_user((uint8_t*)str++))!='\0' && ch!=-1);
  if(ch=='\0')
    return true;
  else
    return false;
}



static void
syscall_handler (struct intr_frame *f /*UNUSED*/) 
{
  //new
  if(!is_valid_pointer(f->esp,4)){
    exit(-1);
    return;
  }
    
  int sysnum = *((int *)(f->esp));
  
  if(sysnum >= SYS_CALL_NUM || sysnum<=0){
    //printf("NO THIS SYSTEM CALL!\n");
    exit(-1);
  }
  // if(syscalls[No]==NULL){           //也是一种错误处理
  //   printf("SYSTEM CALL %d NOT IMPLEMENT\n",No);
  //   exit(-1);
  // }
  syscalls[sysnum](f);

}

//-------TIME 2 WRITE SOME FUUUUUUUUUUUNCTIONS MY FRIEND!-------------------

void sys_halt (struct intr_frame *f){
  //停止运行OS
  shutdown(); //需要power off吗
}

void sys_exit (struct intr_frame *f){
  if(!is_valid_pointer(f->esp+4,4))            //需要加括号吗？
    exit(-1);
  int status = *(int *)(f->esp +4);
  exit(status);

}

void sys_exec (struct intr_frame *f){
  if(!is_valid_pointer(f->esp+4,4)||!is_valid_string(*(char **)(f->esp + 4)))
    exit(-1);

  char *file_name = *(char **)(f->esp+4);

  lock_acquire(&file_lock);
  f->eax = exec(file_name);
  lock_release(&file_lock);

}

void sys_wait (struct intr_frame *f){
  pid_t pid;
  if(!is_valid_pointer(f->esp+4,4))
    exit(-1);
  pid = *((int*)f->esp+1);
  f->eax = wait(pid);
}

void sys_create (struct intr_frame *f){
  if(!is_valid_pointer(f->esp+4,4))            //需要加括号吗？
    exit(-1);
  char* filename = *(char **)(f->esp+4);

  if(!is_valid_string(filename))
    exit(-1);
  unsigned size = *(int *)(f->esp+8);

  f->eax = create(filename,size);
}

void sys_remove (struct intr_frame *f){
  if (!is_valid_pointer(f->esp +4, 4) || !is_valid_string(*(char **)(f->esp + 4)))
    exit(-1);

  char *filename = *(char **)(f->esp+4);

  f->eax = remove(filename);
}

void sys_open (struct intr_frame *f){
  if (!is_valid_pointer(f->esp +4, 4) || !is_valid_string(*(char **)(f->esp + 4)))
    exit(-1);

  char *filename = *(char **)(f->esp+4);
  lock_acquire(&file_lock);
  f->eax = open(filename);
  lock_release(&file_lock);
}

void sys_filesize (struct intr_frame *f){
  if(!is_valid_pointer(f->esp+4,4))
    exit(-1);

  int fd = *(int *)(f->esp + 4);
  f->eax = filesize(fd);
}

void sys_read (struct intr_frame *f ){
  if (!is_valid_pointer(f->esp + 4, 12))
    exit(-1);

  int fd = *(int *)(f->esp + 4);
  void *buffer = *(char**)(f->esp + 8);
  unsigned size = *(unsigned *)(f->esp + 12);

  if (!is_valid_pointer(buffer, 1) || !is_valid_pointer(buffer + size,1))
    exit(-1);
  
  lock_acquire(&file_lock);
  f->eax = read(fd,buffer,size);
  lock_release(&file_lock);
}

void sys_write (struct intr_frame *f){
  if (!is_valid_pointer(f->esp + 4, 12))
    exit(-1);

  int fd = *(int *)(f->esp + 4);
  void *buffer = *(char**)(f->esp + 8);
  unsigned size = *(unsigned *)(f->esp + 12);

  if (!is_valid_pointer(buffer, 1) || !is_valid_pointer(buffer + size,1))
    exit(-1);
  
  lock_acquire(&file_lock);
  f->eax = write(fd,buffer,size);
  lock_release(&file_lock);
  return;
}

void sys_seek (struct intr_frame *f){
  if(!is_valid_pointer(f->esp + 4, 8))
    exit(-1);

  int fd = *(int *)(f->esp + 4);
  unsigned pos = *(unsigned *)(f->esp + 8);
  seek(fd,pos);
}

void sys_tell (struct intr_frame *f){
  if(!is_valid_pointer(f->esp + 4, 4))
    exit(-1);
  int fd = *(int *)(f->esp + 4);
  f->eax = tell(fd);
}

void sys_close (struct intr_frame *f){
  if(!is_valid_pointer(f->esp + 4, 4))
    return exit(-1);
  int fd = *(int *)(f->esp + 4);

  close(fd);
}

static struct fd_entry *
find_fd_entry_by_fd (int fd)
{
  struct fd_entry *ret;
  struct list_elem *l;

  for (l = list_begin (&file_list); l != list_end (&file_list); l = list_next (l))
    {
      ret = list_entry (l, struct fd_entry, elem);
      if (ret->fd == fd)
        return ret;
    }

  return NULL;
}