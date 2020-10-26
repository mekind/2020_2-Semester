# 시스템 프로그래밍 
UNIX 운영체제가 어떤 service들을 제공하는지 공부하는 과목

## 1. UNIX System Overview
***
### 1.1 운영체제란? (=kernel)


**In strcit sense**, 하드 웨어를 컨트롤하고 프로그램들이 실행될 수 있는 환경을 마련해 주는 소프트웨어이다.

<img src='https://notes.shichao.io/apue/figure_1.1.png'>
***

### 1.2 UNIX Architecture


### 1.3 History of UNIX

### 1.4 Logging In

#### 1.4.1 /etc/passwd 에 사용자 정보들이 저장된다. 

    형식 :
        login name:password:user ID:group ID:comment:home directory:shell

    예시 :
        chl:x:401:200:C.H. Lee:/cise / chl :/ tcsh

#### 1.4.2 /etc/shadow 는 비밀 번호 저장한다.

    예시 :
        chl : encrypted passwd

#### 1.4.3 /etc/group 은 그룹 리스트를 저장한다. 

    예시 :
        /etc/group


### 1.5 Files and Directories

    디렉토리도 파일이다. 

    At least 255 character filenames (Most commercial UNIX)

    letters (a-z, A-Z), numbers (0-9), period (.), dash (-), and underscore ( _ )

### 1.6 Programs and Processes

프로그램은 실행시킬 수 있는 파일이고 

프로세스는 그 프로그램을 실행시킨 상태이다.

#### 1.6.1 fork() : 자식 프로세스를 생성한다.

#### 1.6.2 exec() : 프로세스가 할 일을 덮어 씌운다.

#### 1.6.3 waitpid() : 자식 프로세스가 끝날 때까지 기다린다. 

### 1.7 Thread : 프로세스에서 만들어짐

다음을 공유한다. 

    - Address space
    
    - File descriptors
    
    - Stacks
    
    - process related attributes


### 1.8 Error Handling

    /usr/include/(sys/)errno.h 
        extern int errno;   

    /usr/include/string.h
        char *strerror(int errnum);

    /usr/include/stdio.h
        void perror(const char *msg);


### 1.9 Signals


    - ignore the signal ( not recommended.)
    
    - let the default action occur.

    - call your own handler (catch the signal.)


### 1.9 UNIX Time Values

    time_t

    clock_t


## 3. FILE I/O

### 3.1. Unbuffered I/O (part of POSIX.1 and the Single UNIX Specification, but not of ISO C)

#### 3.1.1 File Descriptors

open or create 함수의 리턴 값

```c
#include < fcntl.h >
int open(const char * pathname , int oflag , … /* , mode_t mode);
int creat const char * pathname, mode_t mode); = open( pathname , O_WRONLY | O_CREAT | O_TRUNC, mode);

```



< unistd.h > in POSIX compliant systems

```c
int close (int filedes);
ssize_t read(int filedes , void *buf, size_t nbytes);
ssize_t write(int filedes , void *buf, size_t nbytes);

off_t lseek(int filedes , off_t offset , int whence); 
//SEEK_SET: offset from the beginning of the file
//SEEK_CUR: current file offset + offset
//SEEK_END: file size + offset
```

    0 STDIN_FILENO
    1 STDOUT_FILENO
    2 STDERR_FILENO


### 3.1.2 FILE Sharing

| process table entry | file table          | v-node table      |
| ------------------- | ------------------- | ----------------- |
| fd flags            | file status flags   | v node info       |
| file pointer        | current file offset | i node info       |
| NULL                | v-node ptr          | current file size |

```c
int dup(int filedes);
int dup2(int filedes , int filedes2);
```
```c
int fsync (int filedes);
int fdatasync (int filedes);
void sync(void);
```


### 3.2 File sharing among multiple processes


## 4. Files and Directories

### 4.1. File attributes handling


#### 4.1.1. 파일의 상태를 알려주는 함수 

```c
#include <sys/stat.h>
int stat(const char *pathname , struct stat *buf);
int fstat (int filedes , struct stat *buf );
int lstat (const char *pathname, struct stat *buf);
```

#### 4.1.2 Set User ID and Set Group ID




### 4.2. Unix file system structure and symbolic links

### 4.3. Directory operations

#### Shells 

로그인 후 commands를 입력하는 어플 (command line interpreter)


terminal (an interactive shell)

a file (called a shell script)

***

### Files and Directories

#### File System 

'/' 디렉토리를 최상위로 설정하고 쭉쭉 만들어나감 

디렉토리 사실 파일이다. (안에 파일들의 정보를 담고있는 파일)

#### File Name



#### Pathname

file이 원래 정렬 x, ls 명령어가 list 만들때 정렬하는 거임

working directroy (pwd), home directroy(~)

***

### Input and Output

#### File Descriptors

파일에 접근할 때, 파일을 구별하기 위해 **커널**이 사용하는 것.

#### Standard Input, Standard Output, and Standard Error

관습적으로, 쉘이 새로운 프로그램이 실행될 때 위에 세개의 descriptor들을 연다.

#### Unbuffered I/O 

#### Standard I/O 



process table   ->  file table -> v node table


### dup and dup2 function

dup : 