- [0. 디자인 설계](#0-디자인-설계)
  - [0.1 bpt.c](#01-bptc)
  - [0.2 buffer.c](#02-bufferc)
  - [0.3 file.c](#03-filec)
- [1. 계층별 상세 설명](#1-계층별-상세-설명)
  - [1.1 File Manager](#11-file-manager)
    - [1.1.1 Read / Write](#111-read--write)
    - [1.1.2 Page 관련](#112-page-관련)
    - [1.1.3 Open/Close](#113-openclose)
  - [1.2 Buffer Manager](#12-buffer-manager)
    - [1.2.1 Frame 정의](#121-frame-정의)
    - [1.2.2 Table_info 정의](#122-table_info-정의)
    - [1.2.3 Hash_table 정의](#123-hash_table-정의)
    - [1.2.4 Buffer 초기화 함수](#124-buffer-초기화-함수)
    - [1.2.5 Table 관련 함수](#125-table-관련-함수)
    - [1.2.6 Frame 관련 함수](#126-frame-관련-함수)
    - [1.2.7 보조 기능](#127-보조-기능)
  - [1.3 Index Manager](#13-index-manager)

# 0. 디자인 설계 

## 0.1 bpt.c 

1. Buffer에 저장되어 있는 frame의 정보를 수정할 수 있다.(LRU 관련 정보 제외)

2. B+ tree 를 통해 해당 table에서 data를 insert, delete, find하는 기능을 담당한다.

3. Buffer layer에 table_id, pagenum을 제공한다.

## 0.2 buffer.c

1. Buffer를 초기화 하거나 파괴한다.

2. bpt.c 로부터 받은 table_id, pagenum에 해당하는 Frame의 index를 해싱하여 저장한다.(없으면 비었거나 가장 오래된 frame에 불러와 반환)

3. table_id, pathname, file descriptor을 묶어서 저장/ 관리한다.

4. LRU policy를 지킬 수 있도록 double linked list 정보를 저장 

 

## 0.3 file.c 

1. file에 직접 open, close, read, write를 한다.

2. buffer에 file descriptor를 제공한다.


# 1. 계층별 상세 설명

## 1.1 File Manager

### 1.1.1 Read / Write
```c
void file_read_page(int fd, pagenum_t pagenum, page_t* dest);
void file_read_header_page(int fd, Header_page* dest);
void file_write_page(int fd, pagenum_t pagenum, const page_t* src);
void file_write_header_page(int fd, const Header_page* src);
```
Project2 와 다른 점이 없으니 생략한다.

### 1.1.2 Page 관련
```c
pagenum_t file_alloc_page(int fd);
void file_free_page(int fd, pagenum_t pagenum);
```

Project2에서는 사용했지만

Project3에서는 사용후 변경 내용을 frame에도 반영해야 되는 복잡함이 있어
더 이상 이 함수를 사용하지 않고 Buffer Management에서 담당한다.

### 1.1.3 Open/Close
```c
int open_file(char *pathname);
int close_file(int fd);
```
open은 파일을 열고 file descriptor를 반환하는 함수이고

close는 파일을 닫는 함수이다.


## 1.2 Buffer Manager

### 1.2.1 Frame 정의

```c

typedef int fnum_t; // Frame Index type

typedef struct Frame{
	union
	{
		page_t page;
		header_t hpage;
	};

	pagenum_t pnum;
	union
	{
		int table_id; 
		int now; // meta
	};

	char is_dirty;

	union
	{
		int is_pinned;
		int size; // meta
	};
	
	fnum_t prev; // meta, tail
	fnum_t next; // meta, head
} frame_t;
```
과제 명세에 나온 내용을 제외하고 추가된 내용은

Buffer를 초기화할때 크기를 하나 더 크게 초기화 해서 

0번째 frame은 meta data를 저장할 것이다.

0번째 frame은 현재 몇 번째 frame까지 사용되었는가 (now), 총 크기는 얼마인가(size), LRU를 위한 double linked list의 head, tail을 저장한다.

### 1.2.2 Table_info 정의

```c
typedef struct Table_Info{
	int fd[TABLE_TOTAL]; // file descriptor 저장
	char pathname[TABLE_TOTAL][PATHNAME_SIZE]; // pathname 저장
	int cnt;
} table_t;
```

Buffer Management를 버퍼에 관련된 기능만 하도록 하고 싶었지만 

현재 과제에서는 file descriptor, pathname, table_id를 따로 관리할 계층이 없기에 File Manager과 Index Manager를 이어주는 Buffer Manager가 이 일을 하도록 설계하였다.


### 1.2.3 Hash_table 정의

```c
#include<unordered_map>
std::unordered_map<pagenum_t, fnum_t> hash_table[11]
```
table_id, pagenum에 해당하는 Frame의 index를 O(1)만에 구할 수 있도록 hash_table을 제공하는 STL을 사용하였다.

### 1.2.4 Buffer 초기화 함수 

```c
int init_db(int num_buf);
int shutdown_db();
```

- init_db() : 해당 크기만큼 버퍼를 만드는 함수
- shutdown_db() : Buffer에 존재하는 모든 dirty frame을 write하고 버퍼를 파괴한다.

### 1.2.5 Table 관련 함수

```c
int open_table(char *pathname);
int close_table(int table_id);
bool is_open(int table_id);
```

- open_table : 해당 제목의 table을 불러온다. 11개 이상 열지 않고 table이 닫혀도 table_id는 유지한다.
- close_table : 해당 테이블에 관련한 frame을 모두 write하고 file을 닫는다.

### 1.2.6 Frame 관련 함수

```c
void buffer_write_frame(int table_id, int fnum);
fnum_t buffer_read_frame(int table_id, pagenum_t pnum);
fnum_t buffer_alloc_frame();
fnum_t buffer_alloc_page(int table_id);
```
- buffer_write_frame : 해당 프레임을 file에 write하고 프레임 내 페이지의 모든 데이터를 0으로 초기화 (write는 close_table이나 eviction시에만 호출한다.)
- buffer_read_frame : 테이블 id, 페이지 번호에 해당하는 page 정보를 Buffer에 있으면 바로 그 프레임의 index를 반환하고 없으면 Table에서 읽어오고 그 프레임의 index를 반환한다.
- buffer_alloc_frame : 버퍼가 꽉 차지 않았다면 가장 뒤 프레임의 index+1을 반환하고 꽉 찼다면 LRU policy에 해당하는 프레임의 index를 반환한다.
- buffer_alloc_page : table에 해당하는 free page를 불러오거나 생성하여 그 프레임의 인덱스를 반환한다.

### 1.2.7 보조 기능

```c
void buffer_unpin(fnum_t fnum);
void LRU_change(fnum_t fnum);
```

- buffer_unpin : 해당 인덱스의 frame을 unpin하고 dirty을 1로 초기화 한다.
- LRU_change : 해당 인덱스의 frame을 현재 위치하는 double linked list 자리에서 옮겨 맨 뒤로 배치한다.

## 1.3 Index Manager

Project2와 크게 달라진 점은 2가지다.

1. 인자에 table_id가 추가되었다.

2. 프레임 인덱스를 통해 페이지 내용을 수정하고 읽어온다. (fnum_t)


1번 같은 경우는 인자를 프레임 인덱스 자체를 받아오면 필요 없지만 is_pinned이 오랫동안 지속될 가능성이 있어 되도록이면 함수 내에서 table_id와 pagenum을 통해 프레임 인덱스를 불러올 때 핀을 꽂고 종료시 핀을 제거하도록 설계하였다.

나머지 구조는 Project2와 동일하므로 생략한다.