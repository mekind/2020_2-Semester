# 시스템 프로그래밍 
UNIX 운영체제가 어떤 service들을 제공하는지 공부하는 과목

## 1단원 UNIX System Overview
***
### 운영체제란? (=kernel)


**In strcit sense**, 하드 웨어를 컨트롤하고 프로그램들이 실행될 수 있는 환경을 마련해 주는 소프트웨어이다.

<img src='https://notes.shichao.io/apue/figure_1.1.png'>
***

### Logging In


#### Login Name

로그인 시도시 /etc/passwd 파일에서 정보를 찾는다.

ex)  sar:x:205:105:Stephen Rago:/home/sar:/bin/ksh

----> login name:password:user ID:group ID:comment:home directory:shell

***

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

letters (a-z, A-Z), numbers (0-9), period (.), dash (-), and underscore ( _ )

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