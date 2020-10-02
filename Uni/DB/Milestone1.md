
<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

# 0. 포함되어야 하는 내용 

- Possible call path of the insert/delete operation   

    - [Insertion](#145-insertion-관련-함수)   

    - [Deletion](#146-deletion-관련-함수)   


- Detail flow of the structure modification (split, merge)

    - [Split](#151-split)

    - [Merge](#152-merge)

- (Naïve) designs or required changes for building on-disk b+ tree

    - [Designs](#21-naive-on-disc-b-tree-designs)

# 목차 

- [0. 포함되어야 하는 내용](#0-포함되어야-하는-내용)
- [목차](#목차)
- [1. Analyzing B+ Tree (bpt)](#1-analyzing-b-tree-bpt)
  - [1.1 define 목록](#11-define-목록)
    - [1.1.1 ORDER (노드에 들어갈 수 있는 key의 개수)](#111-order-노드에-들어갈-수-있는-key의-개수)
    - [1.1.2 License 관련](#112-license-관련)
  - [1.2 정의된 구조체](#12-정의된-구조체)
    - [1.2.1 Record](#121-record)
    - [1.2.2 Node](#122-node)
  - [1.3 extern 변수 (global하게 사용 가능)](#13-extern-변수-global하게-사용-가능)
  - [1.4 정의된 함수](#14-정의된-함수)
    - [1.4.1 UI(User Interface)를 위한 함수](#141-uiuser-interface를-위한-함수)
    - [1.4.2 queue 관련 함수](#142-queue-관련-함수)
    - [1.4.3 Tree 정보 관련 함수](#143-tree-정보-관련-함수)
    - [1.4.4 find 관련 함수](#144-find-관련-함수)
      - [특정 값 find](#특정-값-find)
      - [범위 find](#범위-find)
    - [1.4.5 Insertion 관련 함수](#145-insertion-관련-함수)
    - [1.4.6 Deletion 관련 함수](#146-deletion-관련-함수)
  - [1.5 Detail flow of the structure modification](#15-detail-flow-of-the-structure-modification)
    - [1.5.1 Split](#151-split)
    - [1.5.2 Merge](#152-merge)
  - [Design 으로 바로가기](#design-으로-바로가기)
  - [1.6 Analyzing Main Function (전제적인 프로그램 실행 flow)](#16-analyzing-main-function-전제적인-프로그램-실행-flow)
    - [1.6.1 파일 실행 시 가능 옵션](#161-파일-실행-시-가능-옵션)
    - [1.6.2 파일 실행 후 가능한 명령어](#162-파일-실행-후-가능한-명령어)
- [2. Designs for building on-disk b+ tree](#2-designs-for-building-on-disk-b-tree)
  - [2.0 주어진 조건 정리](#20-주어진-조건-정리)
    - [2.0.1 추가해야되는 API 기능](#201-추가해야되는-api-기능)
    - [2.0.2 추가로 필요한 함수 정리](#202-추가로-필요한-함수-정리)
    - [2.0.3 자세한 구조 설명 (Size 및 구성 요소)](#203-자세한-구조-설명-size-및-구성-요소)
  - [2.1 Naive on-disc B+ Tree Designs](#21-naive-on-disc-b-tree-designs)
    - [2.1.1 struct node 에서 struct page로 변화](#211-struct-node-에서-struct-page로-변화)
    - [2.1.2 추가되는 함수들](#212-추가되는-함수들)

<br>


# 1. Analyzing B+ Tree (bpt)

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

- B+트리에서 Split은 insert할 때 key의 최대 개수를 초과하면 일어난다. 

- Split은 Node의 종류에 따라 약간의  차이가 있다. 

<br>

**Internal Node에서의 Split**

    insert_into_parent()함수가 실행되었을 때 여유 공간이 없으면
    insert_into_node_after_splitting() 함수를 통해 Split이 이루어진다.
    자세한 단계는 다음과 같다.
    
    1. 새로운 leaf node를 만든다.
    2. 새로운 key가 들어갈 index(IDX)를 선형탐색을 통해 구한다
    3. 임시 배열(key, pointer)들을 할당해 
    4. IDX보다 작은 index를 가지는 key 값들은 원래 index에, 큰 값들은 원래 index+1에 저장한다. 
    (새로운 key는 IDX에 저장)
    5. IDX보다 작은 같은 index를 가지는 pointer 값들은 원래 index에, 큰 값들은 원래 index+1에 저장한다. 
    (새로운 pointer는 IDX+1에 저장)
    6. key 임시배열의 첫번째부터 order/2 까지는 원래 leaf node에 저장하고
    7. key 임시배열의 order/2 + 2부터 끝까지는 새로운 leaf node에 저장한다. (pointer도 알맞게 분배)
    8. 새로운 leaf node의 부모를 원래 leaf node와 같게 한다. 
    9. 새로운 leaf node 자식 노드의 부모를 새로운 leaf node로 초기화한다.
    10.  order/2 + 1 번째 key는 insert_into_parent() 통해 부모에 삽입한다. 
    

**Leaf Node에서의 Split**

    insert()함수가 실행되었을 때 여유 공간이 없으면
    insert_into_leaf_after_splitting() 함수를 통해 Split이 이루어진다.
    자세한 단계는 Internal Node에서의 Split과 거의 동일하나 다음이 다르다.

    1. leaf node는 key에 대응되는 pointer(record)의 인덱스가 같다.
    2. 마지막 key 다음 pointer가 아닌 마지막 pointer를 초기화 한다. 
    3. 부모로 보내는 key를 따로 저장하여 삽입하는 것이 아니라 
    새로운 leaf node의 첫 key값을 부모에 삽입한다. 


위의 insert()로 최대 개수 초과 문제가 생길 경우,

위 두 과정을 거쳐 leaf node에서부터 문제가 없을 때까지 

부모 노드로 이동하여 split을 진행한다. 
    

---

### 1.5.2 Merge    

- B+트리에서 Merge는 delete할 때 key의 최소 개수미만이면 일어난다. 

- Merge에 영향을 끼치는 요소는 다음과 같다
    - Node의 종류(Leaf, Internal) 
    - 이웃 노드의 key 개수 
    - 삭제를 진행하는 노드의 부모에서의 그 노드를 가르키는 pointer index 
    (leftmost or not)

자세한 내용은 다음과 같다. 
(Node의 종류 때문에 발생하는 index 관련 issue와 부모로 보내는 key값 관련 issue는 위에 설명했기에 생략한다.)

//최댓값 추가설명??

**coalesce_nodes() 함수**

     문제가 생긴 node의 key 개수와 이웃 노드의 key 개수 합이 
     key 최대 개수를 넘지 않을 경우 실행한다.
     자세한 단계는 다음과 같다.

    1. 문제가 생긴 node가 leftmost이면 일반성을 위해 이웃 노드와 swap한다.
    2. 문제가 생긴 노드의 정보를 모두 이웃 노드 뒤에 적절히 저장한다.
    3. delete_entry() 통해 부모 노드에 문제가 생긴 노드의 key와 pointer를 지우도록 한다.


**redistribute_nodes() 함수**

     문제가 생긴 node의 key 개수와 이웃 노드의 key 개수 합이 
     key 최대 개수를 넘을 경우 실행한다.
     자세한 단계는 다음과 같다.

    1.1 문제가 생긴 node가 leftmost이면 이웃 노드의 첫번째 key, pointer를 문제가 생긴 node 뒤에 적절히 저장한다.
    1.2 문제가 생긴 node가 leftmost이 아니면 이웃 노드의 마지막 key, pointer를 문제가 생긴 node 첫 key,pointer에 적절히 저장한다.
    2. 그에 따라 적절한 key 값으로 부모의 key값을 초기화 한다.
    3. 다른 노드의 key, pointer 개수에 변화를 주지 않기 때문에 이대로 종료한다.

이어지는 내용은 main함수에 대한 분석과 과제 조건 정리입니다.

on-disk B+tree Design으로 바로 넘어가시려면 아래 링크를 누르세요.
## [Design 으로 바로가기](#2-designs-or-required-changes-for-building-on-disk-b-tree)
<br>

---
## 1.6 Analyzing Main Function (전제적인 프로그램 실행 flow)

주어진 파일들로 make를 실행시 "main"이라는 실행 파일이 만들어진다. 

"main"을 실행 시 다음과 같다.

### 1.6.1 파일 실행 시 가능 옵션 
실행과 동시에 최대 2개의 인자를 추가로 입력할 수 있다.

    실행 형식 : ./main {order} {input_file} {추가인자}

    - {order} : order을 변경하는 인자이다. 3이상 20 이하의 수만 입력 가능하다.  (default=3)

    - {input_file} : 데이터가 저장된 파일 경로를 나타내고 파일 내 정보들을 tree에 insert한다.

    - {추가인자} : 이 부분부터는 모든 내용을 무시한다.

### 1.6.2 파일 실행 후 가능한 명령어 

usage_2()함수에 자세히 기술되어 있다. 그 내용은 다음과 같다.

    i <k>  -- Insert <k> (an integer) as both key and value).
    f <k>  -- Find the value under key <k>.
    p <k> -- Print the path from the root to key k andits associated value.
    r <k1> <k2> -- Print the keys and values found in therange [<k1>, <k2>
    d <k>  -- Delete key <k> and its associated value.
    x -- Destroy the whole tree.  Start again with anempty tree of the same order.
    t -- Print the B+ tree.
    l -- Print the keys of the leaves (bottom row of thetree).
    v -- Toggle output of pointer addresses ("verbose")in tree and leaves.
    q -- Quit. (Or use Ctl-D.)
    ? -- Print this help message.

---

# 2. Designs for building on-disk b+ tree

## 2.0 주어진 조건 정리

### 2.0.1 추가해야되는 API 기능

**형식 : >{명령어} {인자1} {인자2}**

    1. open <pathname>
        • <pathname> 경로에 존재하는 파일을 열거나 없으면 생성한다.
        • 아래 3개의 명령어는 open을 실행하고 실행되어야 한다.

    2. insert <key> <value>
        • <key>,<value> 쌍의 record를 file의 적절한 위치에 저장한다.
        • 중복키는 허용하지 않는다.

    3. find <key>
        • <key>에 해당하는 'value'를 반환한다.

    4. delete <key>
        • <key>에 해당하는 'record'를 삭제한다.

### 2.0.2 추가로 필요한 함수 정리 

**Data Manager API**

```c
1. int open_table (char *pathname);
//  • <pathname> 경로에 존재하는 파일을 열거나 없으면 생성한다.
//  • 성공하면 unique한 테이블 id를, 실패하면 음수값을 반환한다.

2. int db_insert (int64_t key, char * value);
//  • <key>,<value> 쌍의 record를 file의 적절한 위치에 저장한다.
//  • 성공하면 0, 실패하면 0이 아닌 값을 반환한다. 

3. int db_find (int64_t key, char * ret_val);
//  • <key>에 해당하는 'value'를 찾는다.
//  • 해당하는 value가 존재하면 ret_val에 저장 후 0을 반환하고, 
//    존재하지 않으면 0이아닌 값을 반환한다.
//  • ret_val에 대한 메모리 할당은 caller 함수에서 일어나야 된다.
    
4. int db_delete (int64_t key);
//  • <key>에 해당하는 'record'를 찾고 삭제한다.
//  • 성공하면 0, 실패하면 0이 아닌 값을 반환한다.
```
**Page Manager API**

```c
typedef uint64_t pagenum_t; //8 Bytes를 담을 수 있는 자료형

pagenum_t file_alloc_page();
// Free page 중에서 하나를 할당한다. 

void file_free_page(pagenum_t pagenum);
// 해당 페이지를 초기화하고 Free List에 추가 

void file_read_page(pagenum_t pagenum, page_t* dest);
// on-disk page를 in-memory page 구조로 읽어낸다.

void file_write_page(pagenum_t pagenum, const page_t* src);
// in-memory page를 on-disk page로 저장한다. 

//추가로 Delayed Merge 관련 함수 : Merge를 Order에 상관없이 늦춘다.
```



### 2.0.3 자세한 구조 설명 (Size 및 구성 요소)

**~~다행히도~~ Fixed된 크기로 data file를 생성한다. 자세한 구조는 다음과 같다.**

    - 고정된 page size : 4096 Bytes
    - 고정된 record size : 128(8+120) Bytes -> 한 페이지 내 31개 record
        - type : key => integer & value => string
    - Page Type

        1. Header page (special, containing metadata)
            - Free page number [0-7] : 첫번째 아직 쓰이지 않은 페이지, 없으면 0
            - Root page number [8-15] : root page 저장
            - Number of pages [16-23] : 현재 존재하는 페이지 수
        
        2. Free page (maintained by free page list)
            - Next free page Number: [0-7] (다음 Free page, 없으면 0)

        3. Leaf page (containing records)
            - Page Header : [0-127] 
                - Parent page Number [0-7] : 부모 페이지 저장
                - Is Leaf [8-11] : Internal은 0, Leaf는 1 저장 
                - Number of keys [12-15] : key 개수 저장
                - Right Sibling Page Number [120-127] : 다음 Leaf Page 주소 저장, 없으면 0
            - 각각 Record : 128 Bytes (Key (8) + Value (120))
            - Order = 32로 고정 
        
        4. Internal page (indexing internal/leaf page)
            이해를 더 쉽게 하기 위해 Leaf와 대조하며 서술했다.
            - Page Header [0-127] : Leaf와 동일 
            - Right Sibling Page Number 대신 one more page number to interpret key ranges
            - Record 대신 Key, Page 저장 : 16 Bytes (Key (8) + Page number (8))
            - Order = 249로 고정 

## 2.1 Naive on-disc B+ Tree Designs 

### 2.1.1 struct node 에서 struct page로 변화 

on-disk B+ Tree는 page를 기반으로 작동하기 때문에 

기본적으로 변화가 필요한 부분이다. 

과제 명세 ([과제 명세가 정리된 위치](#203-자세한-구조-설명-size-및-구성-요소))에 따르면 

Page Type마다 저장하는 정보가 상당히 다르기에 

다음과 같이 분류하여 정의해야 될 것 같다. 

```c

//Header page
typedef struct Header_page {
	pagenum_t Free_page_number; //첫번째 아직 쓰이지 않은 페이지, 없으면 0
	pagenum_t Root_page_number; //root page 저장
    pagenum_t Number_of_pages; //현재 존재하는 페이지 수
    char Reserved[4072]; // size 조정
} Header_page;


//Free page
typedef struct Free_page {
	pagenum_t Next_free_page_Number; //다음 Free page, 없으면 0
    char Not_used[4088]; // size 조정
} Header_page;

//Leaf page, Internal page
//두 종류의 page는 공통적으로 Page_Header를 가지고 있어 먼저 정의한다.

typedef struct Page_Header {
    pagenum_t Parent page Number; // 부모 페이지 저장
    int Is_Leaf; // Leaf 여부 저장
    int Number_of_keys; //key 개수 저장 
    pagenum_t one_more_page_number; //Leaf에서는 다른 Leaf, Internal에서는 leftmost child page
} Page_Header;

//추가적으로 Leaf에서는 Record
//Internal에서는 key를 검색하게 하기 위한 Index를 저장하므로 
//편의를 위해 각각 구조체를 정의한다.


//type : key => integer & value => string, 128 Bytes(8+120)
typedef struct Record {
    long long key; 
    char value[120];
} Record;

// 16 Bytes Key (8) + Page number (8)
typedef struct Index {
    long long key; 
    pagenum_t ;
} Record;

typedef struct Internal_Page {
    Page_Header h; // 페이지 헤더 
    Record[31]; // Order 32
} Internal_Page;

typedef struct Leaf_Page {
    Page_Header h; // 페이지 헤더 
    Index[248]; // Order 249
} Leaf_Page;
```

### 2.1.2 추가되는 함수들

**Data Manager API**

```c
1. int open_table (char *pathname);
//  • <pathname> 경로에 존재하는 파일을 열거나 없으면 생성한다.
//  • 성공하면 unique한 테이블 id를, 실패하면 음수값을 반환한다.

2. int db_insert (int64_t key, char * value);
//  • <key>,<value> 쌍의 record를 file의 적절한 위치에 저장한다.
//  • 성공하면 0, 실패하면 0이 아닌 값을 반환한다. 

3. int db_find (int64_t key, char * ret_val);
//  • <key>에 해당하는 'value'를 찾는다.
//  • 해당하는 value가 존재하면 ret_val에 저장 후 0을 반환하고, 
//    존재하지 않으면 0이아닌 값을 반환한다.
//  • ret_val에 대한 메모리 할당은 caller 함수에서 일어나야 된다.
    
4. int db_delete (int64_t key);
//  • <key>에 해당하는 'record'를 찾고 삭제한다.
//  • 성공하면 0, 실패하면 0이 아닌 값을 반환한다.
```
**Page Manager API**

```c
typedef uint64_t pagenum_t; //8 Bytes를 담을 수 있는 자료형

pagenum_t file_alloc_page();
// Free page 중에서 하나를 할당한다. 

void file_free_page(pagenum_t pagenum);
// 해당 페이지를 초기화하고 Free List에 추가 

void file_read_page(pagenum_t pagenum, page_t* dest);
// on-disk page를 in-memory page 구조로 읽어낸다.

void file_write_page(pagenum_t pagenum, const page_t* src);
// in-memory page를 on-disk page로 저장한다. 

//추가로 Delayed Merge 관련 함수 : Merge를 Order에 상관없이 늦춘다.
```