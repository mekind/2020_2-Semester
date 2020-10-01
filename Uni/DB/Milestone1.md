

# 0. 포함되어야 하는 내용 

- Possible call path of the insert/delete operation   

    - [Insertion](#145-insertion-관련-함수)   

    - [Deletion](#146-deletion-관련-함수)   


- Detail flow of the structure modification (split, merge)

    - [Split](#151-split)

    - [Merge](#152-merge)

- (Naïve) designs or required changes for building on-disk b+ tree

    - [Designs](#31-designs)

    - [Merge](#32-required-changes)

# 목차 

- [1. Analyzing "bpt.h" with "bpt.c"](#1-analyzing-bpth-with-bptc)
    - [1.1 define 목록](#11-define-목록)
    - [1.2 정의된 구조체](#12-정의된-구조체)
    - [1.3 extern 변수](#13-extern-변수)
    - [1.4 정의된 함수](#14-정의된-함수)

# 1. Analyzing "bpt.h" with "bpt.c"

## 1.1 define 목록 

### 1.1.1 ORDER (노드에 들어갈 수 있는 key의 개수)
```c
#define DEFAULT_ORDER 4 // 기본값
#define MIN_ORDER 3 // 최솟값
#define MAX_ORDER 20 // 최댓값
```

### 1.1.2 License 관련   

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

### 1.2.1 Record 
```c
typedef struct record {
    int value;
} record;
```

실질적인 data를 저장하는 구조체 이다.

이 프로그램에서는 key값과 value 값이 동일하다. 

###  1.2.2 Node
```c  
typedef struct node {
    void ** pointers; 
    // Leaf Node : record의 주소들을 저장한다. (key와 같은 인덱스)
    //             맨 끝 포인터는 다음 리프 노드를 가리킨다. 
    // Internal Node : 자식 노드의 주소를 저장한다. (key의 인덱스 + 1)

    int * keys; 
    // Leaf Node : record의 key를 저장한다.
    // Internal Node : leaf를 찾아가기 위한 key값 저장

    struct node * parent; 
    // Leaf Node, Internal Node : 부모 노드의 주소를 저장한다.

    bool is_leaf; 
    // Leaf Node, Internal Node : 리프 여부를 저장한다.

    int num_keys; 
    // Leaf Node, Internal Node : 저장된 키의 수를 저장한다.

    struct node * next; // queue에서 다음 노드를 찾을 때 쓰인다.
} node;
```

## 1.3 extern 변수 (global하게 사용 가능)

```c
extern int order;  // order를 저장하는 변수
extern node * queue; // queue의 첫 정보를 담는 주소를 저장하는 변수 
extern bool verbose_output;	// 경로 출력 여부를 저장하는 변수
```
## 1.4 정의된 함수 

### 1.4.1 UI(User Interface)를 위한 함수 

```c
void license_notice(void);// 버전 및 라이선스 관련 내용을출력한다.   
void print_license(int licence_part);//라이선스 파일 따로 있을때 실행하는 함수이다. (이 프로그램에서는 실행되지 않음)   
void usage_1(void); // order에 관한 설명을 출력
void usage_2(void); // 명령어에 대한 설명을 출력 
void usage_3(void); // 파일 실행시 입력 인자에 대한 설명을 출력
```

### 1.4.2 queue 관련 함수 

```c
node * dequeue(void); // 큐에 새로운 노드 삽입
node * dequeue(void); // 큐에서 맨 앞 노드 제거 
```

### 1.4.3 Tree 정보 관련 함수

```c
void print_leaves(node * root); // 리프 노드 키 값 출력
void print_tree(node * root); // 트리 출력 
int height(node * root); // tree 높이 반환
int path_to_root(node * root, node * child); // child 에서 root까지 거리 반환
```
    

### 1.4.4 find 관련 함수 

#### 특정 값 find

```c    
void find_and_print(node * root, int key, bool verbose); 
// 입력한 키 값에 해당하는 노드의 주소, key, value 출력   

    // 함수 내에 존재하는 함수 
    record * find(node * root, int key, bool verbose); // key값이 들어있는 record 주소 반환
```

#### 범위 find

```c
void find_and_print_range(node * root, int key_start, intkey_end, bool verbose); // 범위에 해당하는 노드의 주소, key,value 출력


    //함수 내에 존재하는 함수 
    int find_range(node * root, int key_start, int key_end,bool verbose, int returned_keys[], void *returned_pointers[]);
    // 범위에 해당하는 리프 노드를 찾고 선형탐색으로 끝값까지 배열에 저장

        //함수 내에 존재하는 함수 (해당 키에 해당하는 리프 찾는 함수)
        node * find_leaf(node * root, int key, bool verbose);
        //해당 키가 들어있는 leaf node를 출력 
```
***
### 1.4.5 Insertion 관련 함수

<br>

**가능한 call path는 다음과 같다.**

    1. 명령어 i {key} 입력시 insert() 호출 
   
    2. find()를 호출해 key의 존재 여부 확인
       - 존재할 경우, root를 반환// Case 1
       - 존재하지 경우, make_record()를 호출해 새로운 레코드를 만들고 pointer 변수에 저장

    3. root 노드의 NULL 여부 확인 
       - NULL일 경우, start_new_tree()를 호출하여 반환 // Case2
       - NULL이 아닐 경우, find_leaf()를 호출하여 key값이 들어갈 leaf 노드를 찾는다.

    4. leaf 에 여유 공간이 있는지 확인
        - 있을 경우, insert_into_leaf()함수를 호출 해 leaf에 key를 삽입하고 root를 반환 // Case3
        - 없을 경우, insert_into_leaf_after_splitting()를 호출
  
    5. 적절히 split 후 insert_into_parent() 호출
        - parent가 NULL 일 때, insert_into_new_root()를 호출해 새로운 root를 생성하고 적절히 초기화 후 반환 // Case4.1
        - 아닐 경우 get_left_index()를 호출하여 부모 노드에서의 split이 생긴 노드의 pointers 인덱스를 구한다.
  
    6. 부모 노드에 여유 공간이 있는지 확인
        - 있을 경우, insert_into_node()를 호출하여 반환 // Case4.2.1
        - 없을 경우 insert_into_node_after_splitting()호출 하여 split을 진행한다. //Case4.2.2

    7. leaf node 나 internal node는 split 진행 후 부모 노드에게 요구하는 행위가 같기 때문에 5번부터 다시 반복한다. 
   

**Case 정리**

    - Case0 : 동적할당을 실패할 경우 해당 error를 출력하며 종료
     (모든 동적할당이 있는 곳에 예외처리가 되어있다.)
    - Case1 : 중복 key값 insert
    - Case2 : 처음으로 insert  
    - Case3 : key가 들어갈 리프 노드에 여유 공간이 있을 경우
    - Case4: 리프 노드에 여유 공간이 없어 split을 할 때
        - Case4.1 : key가 들어갈 리프 노드가 root인 경우
        - Case4.2 : key가 들어갈 리프 노드가 root가 아닐 때
            - Case4.2.1 : 부모 노드에 여유 공간이 있을 경우
            - Case4.2.2 : 부모 노드에 여유 공간이 없을 경우 
              (Case4.2.2는 끝나지 않고 Case4 처음으로 돌아간다.)

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
//pointers는 초기화하지 않는다.

node * make_leaf(void);
// 새로운 leaf node 생성 
//내부에서 make_node() 실행 후 is_leaf true로 초기화 

node * start_new_tree(int key, record * pointer);
//첫 번째 삽입 시 실행되는 함수
// make_leaf()로 새로운 리프를 만들고 적절히 초기화 한 후
// 그 리프를 root에 저장해서 반환

node * insert_into_leaf(node * leaf, int key, record * pointer);
// key 값에 해당하는 위치를 찾아 
// keys와 pointers의 동일한 index에 key와 record 주소를 저장한다.
// 그 leaf를 반환 

node * insert_into_leaf_after_splitting(node * root, node * leaf, int key, record * pointer);
// 리프 노드에 여유 공간이 없을 경우, 이 함수를 호출해 split 후 레코드를 삽입한다. 
// 새로운 리프를 만들고, key가 들어갈 insertion_index를 구한다
// 새로운 배열(key, pointer)들을 할당해 key보다 작은 값들은 원래 index에 저장하고 큰 값들은 +1씩 해줘서 저장한다.
// 삽입할 값들은 insertion_index 위치에 저장
// cut()함수를 이용해  split를 구하고 split 전 인덱스까지는 
// 원래 노드에, 이후는 새로운 리프 노드에 저장한다. 
// 초기화 되지 않은 pointers들은 NULL로 초기화 한다.
// 새로운 key값을 저장하고 insert_into_parent() 실행

node * insert_into_parent(node * root, node * left, int key, node * right);
// parent가 root인 경우 insert_into_new_root() 호출
// 아닌 경우 parent의 여유 공간이 있으면 insert_into_node()
// 없으면 insert_into_node_after_splitting() 호출

node * insert_into_new_root(node * left, int key, node * right);
// 새로운 root 생성 후 적절히 초기화 하여 반환
// 자식들의 부모도 초기화

int get_left_index(node * parent, node * left);  
// left에 해당하는 parent의 pointers의 index를 반환한다. 

node * insert_into_node(node * root, node * parent, int left_index, int key, node * right);
// 노드에 여유 공간이 있을 때 새로운 포인터와 키 값을 추가하는 함수
// insert_into_leaf 와 다른 점: right 인자가 추가 되고 
// keys 와 pointers가 초기화 할때 인덱스가 다르다.
// root 반환 


node * insert_into_node_after_splitting(node * root, node * parent,	int left_index,	int key, node * right);
// insert_into_leaf_after_splitting와 거의 동일하나
// 리프는 keys와 pointers가 동일한 인덱스에서 record를 가르키나
// internal은 pointers 인덱스가 1 크기 때문에 약간의 차이가 있다.
//  insert_into_parent() 반환한다.


```
***

### 1.4.6 Deletion 관련 함수
<br>

**가능한 call path는 다음과 같다.**

    1. 명령어 d {key} 입력시 delete() 호출 

    2. find(), find_leaf() 함수를 통해 key가 존재 여부 확인
        - 존재하지 않는 경우, root 반환// Case1
        - 존재하는 경우, 찾은 노드(N)로 delete_entry() 호출
  
    3. 해당 노드(N)로 remove_entry_from_node()를 실행해 key, pointer를 삭제하고 적절히 초기화 한다. 
    (해당 노드에서만 삭제 다른 노드는 건드리지 않는다.)

    4. 해당 노드(N)가 root인지 확인한다.
        - root인 경우, adjust_root()호출
            - root의 num_keys가 0이 아닐 경우, root를 그대로 반환//Case2.1
            - 0일 경우
                - 자식이 있을 때, 첫 번째 자식을 반환 //Case2.2.1
                - 없을 때, NULL 반환 //Case2.2.2
        - root가 아닌 경우, 노드에 들어가야할 key의 최소 개수를 구한다.
  
    5. 현재 노드(N)의 num_keys와 key의 최소 개수를 비교해
        - num_keys가 더 크거나 같을 때, root를 반환//Case3.1
        - 작을 때, get_neighbor_index() 통해 같은 높이의 이웃 노드를 구한다. 

    6. 현재 노드(N)와 이웃 노드를 합쳤을 때 
        - key의 최대 개수를 넘을 경우 redistribute_nodes()를 호출하여 이웃 노드에서 key를 하나 가져오고 root 반환//Case3.2.1
        - key의 최대 개수를 넘지 않을 경우 coalesce_nodes()호출 //Case3.2.2
        (parent로 delete_entry()호출, 3번부터 다시 실행)

**Case 정리**

    - Case1 : 해당 key가 tree에 존재하지 않는 경우
    - Case2 : 해당 key의 리프가 root일 때
        - Case2.1 : 삭제 후에도 root에 key 존재하는 경우
        - Case2.2 : 삭제 후에도 root에 key 존재하지 않을 때
            - Case2.2.1 : root의 자식이 존재하는 경우
            - Case2.2.2 : root의 자식이 없는 경우
    - Case3 : 해당 key의 leaf node가 root가 아닐 때
        - Case3.1 : 삭제 후 leaf node의 최소 개수를 만족하는 경우
        - Case3.2 : 삭제 후 leaf node의 최소 개수를 만족하지 않을 때
            - Case3.2.1 : 이웃 노드와 합치는 것이 불가능 할 경우
            - Case3.2.2 : 이웃 노드와 합치는 것이 가능한 경우
            (Case3.2.2는 끝나지 않고 parent에 대해 deletion을 다시 수행)


**자세한 함수 설명**
```c
node * delete(node * root, int key);
//키가 있는지 검사하고 
// 있으면 delete_entry()를 실행 후
// root를 반환한다. 

node * delete_entry(node * root, node * n, int key, void * pointer);
// 노드 n 에서 key, pointer를 삭제하는 함수이다 .
// n이 leaf node일 수도 있고 internal node일 수도 있다.
// remove_entry_from_node() 호출하여 실질적으로 n에서 key, pointer 제거 
// n이 root인 경우 adjust_root() 호출하고 종료
// 삭제된 노드가 root인 경우 adjust_root()를 실행
// 노드에 해당하는 최소 키 개수를 만족하는 지를 확인하여
// 만족하면 반환


node * remove_entry_from_node(node * n, int key, node * pointer);
//node에서 해당 ket, pointer를 삭제하는 함수
// n이 leaf node일 수도 있고 internal node일 수도 있다.
// 삭제가 이루어진 leaf 노드 반환

node * adjust_root(node * root);
//삭제가 root에서 일어났을 때, root를 조정하는 함수이다.
//삭제 후  key값이 있는지
//없으면 자식이 있는지에 따라 적절히 root를 반환하고
//원래 root 노드를 free한다.

int get_neighbor_index(node * n);
// n의 부모 노드에서의 n의 pointer 인덱스를 찾고
// -1을 하여 반환한다. 


node * coalesce_nodes(node * root, node * n, node * neighbor, int neighbor_index, int k_prime);
// n 노드와 neighbor 노드를 합치는 함수이다. 
// n 이 leftmost일 때도 일반적으로 처리하기 위해 neighbor와 swap시킨다.
// 현재 위치가 leaf일 때 internal일 때를 나누어 적절히 초기화 한다.

node * redistribute_nodes(node * root, node * n, node * neighbor, int neighbor_index,	int k_prime_index, int k_prime);
// n 노드에서 부족한 key를 보충하기 위해 neighbor에서 가져오는 함수이다.
// n이 leftmost일 때와 아닐 때를 구분한다.
// leaf일 때 internal일 때를 나누어 적절히 초기화 한다.

node * destroy_tree(node * root); // 전체 tree를 삭제하는 함수 

    //내부 함수 
    void destroy_tree_nodes(node * root);
    //leaf부터 올라오며 모든 데이터 삭제 
```
***

## 1.5 Detail flow of the structure modification 

### 1.5.1 Split
<br>

- B+트리에서 Split은 insert할 때 최대 개수를 초과하면 일어난다. 

- 또한, node의 종류에 따라 약간의 차이를 보인다.

<br>

**Leaf Node에서의 Split**

    insert_into_leaf_after_splitting()

- Internal Node에서의 Split
    

    insert_into_node_after_splitting()

<br>

### 1.5.2 Merge   
<br>   

B+트리에서 Merge는 
<br>

### 

# 2. Analyzing "main.c"

# 3. Designs or Required Changes for building on-disk b+ tree

## 3.1 Designs

## 3.2 Required Changes