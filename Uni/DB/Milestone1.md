

# 0. 꼭 포함되어야 하는 내용 

- Possible call path of the insert/delete operation   

    - [Insertion](#145-insertion-관련-함수)   

    - [Deletion](#146-deletion-관련-함수)   


- Detail flow of the structure modification (split, merge)

    -

- (Naïve) designs or required changes for building on-disk b+ tree

# 목차 

- [1. Analyzing "bpt.h" with "bpt.c"](#1-analyzing-bpth-with-bptc)
    - [1.1 define 목록](#11-define-목록)
    - [1.2 정의된 구조체](#12-정의된-구조체)
    - [1.3 extern 변수](#13-extern-변수)
    - [1.4 정의된 함수](#14-정의된-함수)

# 1. Analyzing "bpt.h" with "bpt.c"

## 1.1 define 목록 

### 1.1.1 ORDER 관련 
```c
#define DEFAULT_ORDER 4 // 기본값
#define MIN_ORDER 3 // 최솟값
#define MAX_ORDER 20 // 최댓값
```

### 1.1.2 License 관련   ->  나중에 다시 확인 

```c
#define LICENSE_FILE "LICENSE.txt" // 파일명
#define LICENSE_WARRANTEE 0
#define LICENSE_WARRANTEE_START 592
#define LICENSE_WARRANTEE_END 624
#define LICENSE_CONDITIONS 1
#define LICENSE_CONDITIONS_START 70
#define LICENSE_CONDITIONS_END 625
```

## 1.2 정의된 구조체 

### 1.2.1 record 
```c
typedef struct record {
    int value;
} record;
```

실질적인 값을 담고 있는 data이다   

###  1.2.2 node
```c  
typedef struct node {
    void ** pointers; // 자식 포인터 저장
    int * keys; // key 값 저장 
    struct node * parent; // 부모 노드 저장
    bool is_leaf; // 리프 여부 저장 
    int num_keys; // key의 수 저장 
    struct node * next; // Used for queue.
} node;
```

index를 담당하는 부분인 듯 

## 1.3 extern 변수 

### 1.3.1 

```c
extern int order  // bpt.c에서 default값으로 초기화 (DEFAULT_ORDER)
```

### 1.3.2 

```c
extern node * queue; // queue의 헤드 부분 이걸로 NULL 판단 (NULL)
```

### 1.3.3

```c
extern bool verbose_output;	// (false)
```
## 1.4 정의된 함수 

### 1.4.1 UI(User Interface)를 위한 함수 

```c
void license_notice(void);// 버전 및 라이선스 관련 내용을출력한다.   
void print_license(int licence_part);//라이선스 파일 따로 있을때 실행하는 함수이다. (이 프로그램에서는 실행되지 않음)   
void usage_1(void); // order에 관한 설명을 출력
void usage_2(void); // 명령어에 대한 설명을 출력 
void usage_3(void); // 맨 처음 실행 할 때의 인자에 대한 설명을출력
```

### 1.4.2 queue 관련 함수 

```c
node * dequeue(void); // 큐에 새로운 노드 삽입
node * dequeue(void); // 큐에서 맨 앞 노드 제거 
```

### 1.4.3 Tree 정보 관련 함수

```c
void print_leaves(node * root) // 리프 노드 키 값 출력 ??
void print_tree(node * root) // 트리 출력 ??
int height(node * root) // tree 높이 반환
int path_to_root(node * root, node * child)// child 에서 root까지 거리 반환
```
    

### 1.4.4 find 관련 함수 

#### 특정 값 find

```c    
void find_and_print(node * root, int key, bool verbose) 
// 입력한 키 값에 해당하는 노드의 주소, key, value 출력   


    // 함수 내에 존재하는 함수 
    record * find(node * root, int key, bool verbose) // 
```

#### 범위 find

```c
void find_and_print_range(node * root, int key_start, intkey_end, bool verbose) // 범위에 해당하는 노드의 주소, key,value 출력


    //함수 내에 존재하는 함수 (노드를 찾는 함수)
    int find_range(node * root, int key_start, int key_end,bool verbose, int returned_keys[], void *returned_pointers[])

        //함수 내에 존재하는 함수 (해당 키에 해당하는 리프 찾는 함수)
        node * find_leaf(node * root, int key, bool verbose)
```

### 1.4.5 Insertion 관련 함수

<br>

**가능한 call path는 다음과 같다.**

    1. 명령어 i {key} 입력시 insert() 호출
   
    2. find()를 호출해 key의 존재 여부 확인
       - 존재할 경우, root를 반환// Case 1
       - 존재하지 경우, make_record()를 호출해 새로운 레코드를 만들고 pointer 변수에 저장

    3. root 노드의 NULL 여부 확인 
       - NULL일 경우, start_new_tree()를 호출하여 반환 // Case2
       - NULL이 아닐 경우, find_leaf()를 호출하여 key값이 들어갈 leaf 노드를 leaf 변수에 저장 

    4. leaf의 key 개수를 (order-1)와 비교하여
        - key 개수가 더 적을 경우, insert_into_leaf()함수를 호출 해 leaf에 key를 삽입하고 root를 반환 // Case3
        - 아닐 경우, insert_into_leaf_after_splitting()를 호출
  
    5. 적절히 split 후 insert_into_parent() 호출
        - parent가 NULL 일 때, insert_into_new_root()를 호출해 새로운 root를 생성하고 적절히 초기화 후 반환 // Case4.1
        - 
   

**Case 정리**

    - Case0 : 동적할당을 실패할 경우 해당 error를 출력하며 종료
     (모든 동적할당이 있는 곳에 예외처리가 되어있다.)
    - Case1 : 중복 key값 insert
    - Case2 : 처음으로 insert  
    - Case3 : key가 들어갈 리프 노드에 여유 공간이 있을 때
    - Case4: 리프 노드에 여유 공간이 없어 split을 할 경우
        - Case4.1 : key가 들어갈 리프 노드가 root인 경우
        - Case4.2 : 

**자세한 함수 설명**

```c
node * insert(node * root, int key, int value); 
// 명령에서 i {key} 입력시 실행되는 함수
// 여러 함수를 포함해 b+ 트리에 노드 및 레코드 생성 

record * make_record(int value); 
// 새로운 레코드를 생성

node * make_node(void); 
// 새로운 node 생성
//default 값으로 NULL, 0, false로 초기화

node * make_leaf(void);
// 새로운 leaf node 생성 
//내부에서 make_node() 실행 후 is_leaf true로 초기화 

node * start_new_tree(int key, record * pointer);
//첫 번째 삽입 시 실행되는 함수
// make_leaf()로 새로운 리프를 만들고 적절히 초기화 한 후  root 반환

node * insert_into_leaf(node * leaf, int key, record * pointer);
// 리프 노드에 여유 공간이 있을 경우, 이 함수를 호출해 리프에 레코드를 추가한다. 

node * insert_into_leaf_after_splitting(node * root, node * leaf, int key, record * pointer);
// 리프 노드에 여유 공간이 없을 경우, 이 함수를 호출해 split 후 레코드를 삽입한다. 
// 새로운 리프를 만들고, key가 들어갈 insertion_index를 구한다
// 새로운 배열(key, pointer)들을 할당해 key보다 작은 값들은 원래 index에 저장하고 큰 값들은 +1씩 해줘서 저장한다.
// 삽입할 값들은 insertion_index 위치에 저장
// cut()함수를 이용해  split를 구하고 split전 인덱스까지는 원래 노드에 이후는 새로운 리프 노드에 저장한다. 

node * insert_into_parent(node * root, node * left, int key, node * right);
// parent가 root인 경우 insert_into_new_root() 호출
// 아닌 경우 

node * insert_into_new_root(node * left, int key, node * right);
// 새로운 root 생성 후 적절히 초기화 하여 반환

int get_left_index(node * parent, node * left);  

node * insert_into_node_after_splitting(node * root, node * parent,	int left_index,	int key, node * right);
// 




node * insert_into_node(node * root, node * parent, int left_index, int key, node * right);



node * insert_into_parent(node * root, node * left, int key, node * right);

```


### 1.4.6 Deletion 관련 함수

```c
int get_neighbor_index(node * n);
node * adjust_root(node * root);
node * coalesce_nodes(node * root, node * n, node * neighbor,
	int neighbor_index, int k_prime);
node * redistribute_nodes(node * root, node * n, node * neighbor,
	int neighbor_index,
	int k_prime_index, int k_prime);
node * delete_entry(node * root, node * n, int key, void * pointer);
node * delete(node * root, int key);

void destroy_tree_nodes(node * root);
node * destroy_tree(node * root);
```


### 

# 2. Analyzing "main.c"

