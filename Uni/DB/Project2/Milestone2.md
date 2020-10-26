- [API 상세 설명](#api-상세-설명)
  - [1. File Manager API](#1-file-manager-api)
    - [1.1 System Call 관련](#11-system-call-관련)
    - [1.2 디자인 고려사항](#12-디자인-고려사항)
      - [1.2.1 Layer의 독립성](#121-layer의-독립성)
      - [1.2.2 기능 세분화](#122-기능-세분화)
    - [1.3 define 목록](#13-define-목록)
    - [1.4 변수 설정](#14-변수-설정)
    - [1.5 구조체](#15-구조체)
    - [1.4 함수 상세 설명](#14-함수-상세-설명)
      - [1.4.1 free 관련](#141-free-관련)
      - [1.4.2 Read](#142-read)
      - [1.4.3 Write](#143-write)
      - [1.4.4 Table 관련](#144-table-관련)
  - [2. Index Management API](#2-index-management-api)
    - [2.1  define 목록](#21-define-목록)
    - [2.2 함수 상세 설명](#22-함수-상세-설명)
      - [2.2.1 출력 및 Utility를 위한 함수](#221-출력-및-utility를-위한-함수)
      - [2.2.2 Find 관련 함수](#222-find-관련-함수)
      - [2.2.3 Insertion 관련 함수](#223-insertion-관련-함수)
      - [2.2.4 Deletion 관련 함수](#224-deletion-관련-함수)
      - [2.2.5 Delay Merge에 관련](#225-delay-merge에-관련)
      - [2.2.6 Open / Close](#226-open--close)


## 0. 간단 요약 

### 0.1 사용자 API (main.c)

사용자는 다음과 같은 커맨드로 동작을 실행할 수 있다.

    1. i <key> <value> : key, value를 삽입한다.
    2. d <key> : key를 삭제한다.
    3. f <key> : key를 찾고 value를 출력한다.
    4. o <pathname> : 해당 파일을 연다.
    5. t : tree 전체를 출력한다.(BFS 방식)
    6. l : tree의 leaves만 출력한다. 
    7. p <page> : 해당 페이지의 정보를 출력한다.
    8. h : 헤더 페이지의 정보를 출력한다.
    9. q : 종료
    
위 커맨드 입력시 아래의 API가 실행된다. 

---

### 0.2 Index Management API (bpt.c)

위의 사용자 API 번호와 대응하여 설명하였다.

    1. db_insert(key, vale) : 실행하여 해당 index/leaf를 만들고 경우에 따라서 merge와 split을 한다.
        + 기존에 주어진 B+tree 구조를 거의 그대로 유지하였다. 
        + File Management API를 통해 페이지 정보를 불러오고 변경사항을 바로 저장한다. 

    2. db_delete(key) : 해당 key를 삭제하고 delay merge를 실행한다. 
        + redistribution은 완전히 삭제하고 coalesce_pages에 새로운 방식을 추가했다.
        + File Management API를 통해 페이지 정보를 불러오고 변경사항을 바로 저장한다. 

    3. db_find(key, value) : B+tree 탐색 방법을 기반으로 File Management API 를 통해 그 페이지의 정보를 불러오고 확인한다.
        + 기존에 주어진 B+tree 구조를 거의 그대로 유지하였다.

    4. open_table_in_memory(pathname) : File Management API 파일을 열고 Header의 정보를 불러오거나 새로 만든다. 
    5. print_tree() : BFS 탐색으로 key를 출력한다.
    6. print_leaves() : 오름차순으로 leaf의 key, value를 출력한다.
    7. print_page(pagenum) : 해당 페이지의 정보를 출력한다.
    8. print_header() : 전역 변수로 선언된 header의 정보를 출력한다.

위 함수를 통해 메모리에서 작업과 함께 아래의 API가 실행된다. 

---

### 0.3 File Management API (file.c)

위의 Index Management API 번호와 일대일 대응이 기능에 따라 설명을 하겠다. 

    - Read
        1. file_read_page() : 해당 페이지의 정보를 불러온다.
        2. file_read_page_parent() : 해당 페이지의 부모만 불러온다
        3. file_read_header_page() : 테이블의 헤더 페이지 정보를 불러온다.
            
    - Write
        1. file_write_page() : 페이지에 해당하는 위치에 정보를 저장한다.
        2. file_write_page_parent() : 해당 페이지의 부모를 바꾼다.
        3. file_write_header_page() : 헤더 페이지에 정보를 저장한다.

    - Free
        1. file_alloc_page() : 새로운 페이지 번호를 반환한다.
        2. file_free_page() : 해당 페이지를 Free page로 만든다.

    - Table 관련
        1. open_table() : 해당 테이블을 열어 파일 디스크립터를 반환
        2. close_table() : 테이블 닫는다. 

---


# API 상세 설명 

## 1. File Manager API

### 1.1 System Call 관련

과제를 진행한 순서로 설명을 진행할 것이다.

처음 시작할 때 데이터 파일을 어떻게 접근할 수 있고 어떻게불러  올 수 있는지를 전혀 감이 안 잡히기에 Index Management API가 어떤     인자를 넘겨주고 받아와야 하는 전혀 알수가 없었다. 그래서 일단  system call을 공부하여 FileManager API를 먼저 완성하게  되었다. 

[블로그](https://hororolol.tistory.com/263)를 참고하여시스템   콜을 조금 알게 되고 [Linux manual page](https:/www.man7.org/ linux/man-pages/man2/pwrite.2.html)를 통해구체적인 사용법을     익혔다. 여러 함수를 찾아보고 이번과제에는 offset을 인자로 주어     바로 read/write을 할 수있는 prea()d와 pwrite()가 적합하다고    판단하였다.

### 1.2 디자인 고려사항

#### 1.2.1 Layer의 독립성

    독립성을 통해
    1. 코드가 문제가 생겼을 때 디버깅 하기도 좋고
    2. 하나의 함수가 굉장히 길어지는 일이 드물고 
    3. 깔끔하게 기능별로 설계를 할 수 있기에 
    독립성이 지켜지는 것이 중요하다 생각했다. 

    그렇기에 파일을 불러오고 저장하는 일 빼고는 추가적인 기능을 하지 않도록 했다.

#### 1.2.2 기능 세분화 

    이 부분은 Index Management API를 하면서 느낀 점인데 
    페이지의 부모를 변경하는 일이 상당히 많이 일어난다는 것을 알게 되었다.

    현재의 함수만으로는 read, write 두 번의 FILE I/O을 해야된다는 점이 불필요하다고 느껴 
    parent 정보 관련 함수를 추가하였다. 

    또한, 헤더 페이지 또한 상당히 많이 변경되기에 관련 함수를 추가하고 Header라는 전역변수로 저장하였다. 
    
### 1.3 define 목록

숫자로 쓰는 것보다 알아볼 수 있게 쓰는 걸 좋아하는 편이라 다음과 같이 정의하였다.

```c
#define OPEN_MODE 0666 // 읽기와 쓰기 권한
#define PAGE_SIZE (size_t)4096 // 페이지 크기
#define INTERNAL_ORDER 249 
#define LEAF_ORDER 32
#define VALUE_SIZE 120

typedef uint64_t pagenum_t;
typedef long long my_key_t;
```

### 1.4 변수 설정

다음과 같은 변수를 전역으로 선언하였다. 

코드의 효율성을 위해 Header 를 따로 정의(Header Page 정보 저장)하였고 bpt.c 에서 extern 한다. 

```c
int table; // 파일 디스크립터 
Header_page *Header; // Header 페이지
```

### 1.5 구조체 

```c
typedef struct Header_page {
	pagenum_t Free_page; //첫번째 아직 쓰이지 않은 페이지, 없으면 0
	pagenum_t Root_page; //root page 저장
	pagenum_t Number_of_pages; //현재 존재하는 페이지 수
	char Reserved[4072]; // size 조정
} Header_page;

typedef struct Record {
	my_key_t key;
	char value[VALUE_SIZE];
} Record;

typedef struct Index {
	my_key_t key;
	pagenum_t Next_Page;
} Index;

typedef struct page_t {
	union
	{
		pagenum_t Parent; // Leaf, Internal은 부모 페이지 저장
		pagenum_t Next_Free; // Free는 다음 Free 페이지 저장, Free는 여기만 초기화 한다.
	};	

	int Is_Leaf; // Leaf 여부 저장
	int Num_Keys; //key 개수 저장 
	char Reserved[104];

	pagenum_t Next_Page;
	//Leaf에서는 다음 Leaf 저장
	//Internal에서는 leftmost child page 저장

	union
	{
		Record Records[LEAF_ORDER - 1]; // Order 32
		Index Indexes[INTERNAL_ORDER - 1]; // Order 249
	};
} page_t;
```

Milestone1과 다른 점이 없으므로 주석으로 설명을 생략하겠다.

### 1.4 함수 상세 설명 


#### 1.4.1 free 관련

```c
pagenum_t file_alloc_page();
void file_free_page(pagenum_t pagenum);
```

- file_alloc_page() : 
    1. Header에 Free Page가 있으면 그 번호를 출력
    2. 없으면 총 페이지 수 + 1 을 반환
    3. Header 정보를 적당히 초기화 후 저장 
- file_free_page() : 
    1. 해당 페이지 Parent를 현재 Header의 Free Page로 바꾸고
    2. 해당 페이지를 Header의 Free Page에 저장


#### 1.4.2 Read
```c
void file_read_page(pagenum_t pagenum, page_t* dest);
void file_read_page_parent(pagenum_t pagenum, pagenum_t* dest);
void file_read_header_page();
```
별다른 기능 없이 pread로 해당 페이지의 정보를 가져오는 함수다.

1. file_read_page() 해당 페이지 전체 읽어오기
2. file_read_page_parent() 해당 페이지 부모만 읽어오기
3. file_read_header_page() 헤더 페이지 전체 읽어오기

#### 1.4.3 Write

별다른 기능 없이 pwrite로 해당 페이지의 정보를 저장하는 함수다.

```c
void file_write_page(pagenum_t pagenum, const page_t* src);
void file_write_page_parent(pagenum_t pagenum, pagenum_t * buffer);
void file_write_header_page();
```
1. file_write_page() : 해당 페이지 전체 저장하기
2. file_write_page_parent() : 해당 페이지 부모만 저장하기
3. file_write_header_page() : 헤더 페이지 전체 저장하기

기능 실행 후 모두 sync()를 실행한다. 

#### 1.4.4 Table 관련

별다른 기능 없이 open/close로 해당 파일을 열고 닫는 함수이다.

```c
int open_table(char *pathname);
int close_table();
```
1. open_table() : 해당 경로의 파일을 연다.
2. close_table() : 열려있는 파일을 닫는다.



## 2. Index Management API



### 2.1  define 목록


```c
#define NO_PAGE (pagenum_t)0
#define SUCCESS 0
#define FAIL 1
```

### 2.2 함수 상세 설명

#### 2.2.1 출력 및 Utility를 위한 함수

```c
int cut(int length);
int path_to_root(pagenum_t pagenum); //root 까지 거리
void enqueue(pagenum_t pnum); // queue 구현
pagenum_t dequeue( void ); // queue 구현
void print_tree(); // tree 전체 출력 
void print_leaves(); // leaf들의 key value만 출력 
void print_page(pagenum_t p); // 해당 페이지 출력 
void print_header(); // 헤더의 정보 출력 
```

1. cut() : 기존에도 있었던 split을 할 기준을 정하는 함수 
2. 나머지는 본 과제와 관련 없으니 주석으로 간략하게 설명한다.



#### 2.2.2 Find 관련 함수 

```c
pagenum_t db_find_leaf(my_key_t key);
int db_find(my_key_t key, char * ret_val);
int get_my_index(page_t * parent, pagenum_t target);
```

기존에 있던 함수를 자료형만 적절히 바꾸었다. 

get_my_index()에서 조금 다른 점은 on-disk는 page에서 key와 pagenum이 같은 인덱스에 존재하여 get_left_index가 get_my_index로 바뀌었다. 

#### 2.2.3 Insertion 관련 함수 

기존의 형태를 그대로 유지하였다.

```c
int db_insert(my_key_t key, char *value);
int start_new_tree(my_key_t key, char * value);
int insert_into_leaf(pagenum_t leaf_num, page_t leaf, my_key_t key, char* value);
int insert_into_leaf_after_splitting(pagenum_t leaf_num, page_t leaf, my_key_t key, char * value);
int insert_into_parent(pagenum_t parent_num, pagenum_t left_num, my_key_t key, pagenum_t right_num);
int insert_into_new_root(pagenum_t left_num, my_key_t key, pagenum_t right_num);
int insert_into_page(pagenum_t n_num, page_t n, int left_index, my_key_t key, pagenum_t right_num);
int insert_into_page_after_splitting(pagenum_t old_page_num, page_t old_page, int left_index,
	my_key_t key, pagenum_t right_num);
```

동작 방식은 기존과 동일하여 특별한 설명을 덧붙이지 않는다. 

변경/추가된 점은

1. 페이지의 변경사항이 있으면 file_write_page()를 통해 바로 저장한다. 
(부모변경도 바로바로 file_write_page_parent() 로 저장)
2. 기존에는 포인터 사용/ 동적할당을 하였지만 ORDER 및 크기가 고정되어 있어 정적할당을 하였다. 
3. 페이지 추가시 file_alloc_page() 호출


#### 2.2.4 Deletion 관련 함수 

아래 함수는 기존과 동일하다. 

```c
int db_delete(my_key_t key);
int delete_entry(pagenum_t page_num, my_key_t key);
page_t remove_entry_from_page(pagenum_t page_num, page_t page, my_key_t key);
int adjust_root(pagenum_t root_num, page_t root);
```

변경/추가된 점은 insert와 거의 같고 

페이지가 삭제되면 file_free_page()를 호출하고

Header 페이지의 정보를 수정 후 저장한다. 

#### 2.2.5 Delay Merge에 관련 

merge와 split이 많이 일어나기에 ping-pong 현상을 줄이기 위해 Delay Merge를 한다고 이해하였다. 따라서 redistribution은 하나의 key 변경만으로도 계속 일어날 수 있으므로 ping-pong 현상을 줄이기에 적합하지 않다고 판다하고 과감히 삭제 후 Insertion에서 사용한 merge-split 방식을 사용하여 구현하였다.

```c
int coalesce_pages(pagenum_t n_num, page_t n, pagenum_t neighbor_num, 
	page_t neighbor, int n_index, my_key_t k_prime);
```

ORDER에 따른 최수 key의 개수에 상관없이

leaf 또는 internal page에서 key의 수가 0개가 되면 merge를 실행하였다.

발생하는 경우의 수를 다음과 같이 크게 2가지 자세히는 3가지로 나누어 생각했다.

1. Key가 0개인 page가 Leaf인 경우 :
   - Leaf에서는 child를 가르키는 정보가 없으므로 key가 0개라면 무조건 merge가 가능하다. 
   - merge 후 delete_entry()를 통해 부모 page에서도 해당 키를 삭제한다.
    

Internal에서는 key가 0개 이여도 Leftmost child가 존재하기 때문에 

부모 page에서 key를 가져와서 merge해야 한다. 따라서 

1. Key가 0개인 page가 Internal인 경우 
    

    2. 1 이웃 페이지의 key 수가 ORDER-1보다 작을때
        - 부모 page에서 key를 하나 가져와서 merge한다.
        -  Leaf에서와 같이 delete_entry()를 통해 부모 page에서 해당 키를 삭제한다.


    2. 2 이웃 페이지에 key 수가 ORDER-1일 때
        - 부모 page에서 key를 하나 가져와서 merge한다.
        - insert에서 split하듯이 진행한다.
        - 다른 함수를 호출하지 않고 부모 page의 키를 변경한다.

    
   

#### 2.2.6 Open / Close

```c
int open_table_in_memory(char *pathname);
int close_table_in_memory();
```

1. open_table_in_memory() : open_table()을 호출해 파일을 열고 Header 정보를 불러오거나 생성한다.
2. close_table_in_memory() : close_table()를 호출해 파일을 닫는다.