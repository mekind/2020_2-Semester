# 형수님의 데이터 베이스 시스템 설계 및 응용
A+을 받아보자

***
- [형수님의 데이터 베이스 시스템 설계 및 응용](#형수님의-데이터-베이스-시스템-설계-및-응용)
  - [Lecture 1 : Course Overview](#lecture-1--course-overview)
  - [Lecture 2 : Introduction to SQL](#lecture-2--introduction-to-sql)
  - [Lecture 4 - DiskFilesBuffers Part1](#lecture-4---diskfilesbuffers-part1)
    - [4.1 Architecture of a DBMS](#41-architecture-of-a-dbms)
      - [4.1.1 Layer 설계](#411-layer-설계)
      - [4.1.2 독립적인 Layer 설계의 장점](#412-독립적인-layer-설계의-장점)
      - [4.1.3 Transaction Proccess Concept](#413-transaction-proccess-concept)
    - [4.2 STORAGE MEDIA](#42-storage-media)
  - [Lecture 5 - DiskFilesBuffers Part2](#lecture-5---diskfilesbuffers-part2)
    - [5.1 DATABASE FILES](#51-database-files)
    - [5.2 PAGE LAYOUT](#52-page-layout)
    - [5.3 RECORD LAYOUT](#53-record-layout)
  - [Lecture 6 File Organizations (Files and Index Management)](#lecture-6-file-organizations-files-and-index-management)
    - [6.1 Access Patterns](#61-access-patterns)
    - [6.2 Cost Model for Analysis](#62-cost-model-for-analysis)
  - [Lecture 7 Static Tree Indexes (Files and Index Management)](#lecture-7-static-tree-indexes-files-and-index-management)
    - [7.1 INDEX Tree, ISAM](#71-index-tree-isam)
      - [7.1.1 생각해 볼 점](#711-생각해-볼-점)
      - [7.1.2 변경 사항](#712-변경-사항)
      - [7.1.3 특징](#713-특징)
  - [Lecture 8 Dynamic Tree Indexes (Files and Index Management)](#lecture-8-dynamic-tree-indexes-files-and-index-management)
    - [8.1 Index Tree, B+ Tree](#81-index-tree-b-tree)
      - [8.1.1 특징](#811-특징)
      - [8.1.2 유사한 기능을 하는 구조 : RB-Tree](#812-유사한-기능을-하는-구조--rb-tree)
      - [8.1.3 자세한 기능](#813-자세한-기능)
      - [8.2 Bulk Loading : 한 번에 많은 insert](#82-bulk-loading--한-번에-많은-insert)

## Lecture 1 : Course Overview

- DBMS 정의

    database + management system

    database: data with schema 

    management: concurrency 병행성(동시성)

- Data models & the relational data model

    Data model : collection of concepts for describing data (ex. R-E relation)

    - E-R (entity-relation)


- schemas & data independence

    Schema : 특정 데이터 모델로 설명하는 데이터 

    - Physcial, Logical, Exteranl Schema   

    Data Independence : Applications do not need to worry about how the data is structured and stored

--- 
 - Transaction (TXN, 가장 꽃)

    - Atomicity : An action either completes entirely or not at all

    - Consistency : An action results in a state which conforms to all integrity constraints

 - Concurrency & locking

 - Atomicity & logging

--- 

## Lecture 2 : Introduction to SQL

- SQL introduction & schema definitions

    A database management system interacts with its environment (users) via the well-defined interface (SQL).

    - DDL : Create/alter/delete tables 
    - DML : Insert/delete/modify tuples

    Data type:

    - Atomic types

    - Key : minimal subset of attributes

    - NULL 

    - FOREIGN KEY

- Basic single-table queries

- Multi-table queries



## Lecture 4 - DiskFilesBuffers Part1

### 4.1 Architecture of a DBMS

#### 4.1.1 Layer 설계

    1. SQL Client : User가 직접 사용하는 층 

    2. Query Parsing & Optimization : 
        - SQL은 인간에게 가까운 언어이기 때문에 컴퓨터에 가까운 언어로 바꾸기 위한 첫 단계 (Relational Query Plan)

    3. Relational Operators :
        - Plan에 맞게 순서 변경하여 다시 2로 전달 
        - 사실 Optimization에서 다 처리하게 설계해도 됨

    4. Files and Index Management :
        - Table 접근 시작, 내가 알고 있는 테이블은 logical -> 실제 테이블은 physical 
        - Record가 어느 (Page, offset)에 어딨는지를 알려줘야 된다. (Pagination)
        - Address Mapping 

    5. Buffer Management : 
        - 메모리와 디스크의 속도 차를 효율적으로 완충 
        
    6. Disk Space Management :
        - Page가 어느 디스크 어느 파일에 존재? (filenum, offset)
        - Record에 대해 명확하게 몇 바이트 쓸 지 계산
        - os를 건너뛰고 disk에 바로 써도 되지만 아주 어려운 기술, 초보자는 open 시스템 콜이나 사용해라
        - page가 연속이라 생각하고 설계 실제 disk는 신경 쓰지 말자
        - Mapping table 만들자

    7. Database : Data들의 집합체 
        - Unorderd Heap Files



#### 4.1.2 독립적인 Layer 설계의 장점
  
아래에서 무슨 일이 일어나는지 전혀 신경 쓸 필요가 없음

#### 4.1.3 Transaction Proccess Concept

    - Concurrency Control
    - Recovery 


### 4.2 STORAGE MEDIA

    1. Disk : Stable Storage 
        - Magnetic Disk
        - 용량만 놓고 본다면 가장 경제적인 저장 공간

    2. Storage Hierarchy
        - Registers
        - On-chip Cache
        - On-Board Cache
        - RAM : For currently used data
        - SSD : Varies by deployment, Sometimes the Database or Cache
        - Disk : Database and backups/logs, Secondary & tertiary storage

    3. Components of a Disk
        - Arm을 늘리면서 속도를 높혔다. 
            - Seek time
            - Rotataion delay
            - Transfer time

    4. SSD : Notes on Flash
        - Wear Leveling : 한 곳에 계속 안 쓰게 만듦

    5. Design 철학 : Design Rationale 
        - 정답인 디자인은 없다. 
        - 문제 주 원인, 그를 해결할 가설 -> 디자인 
    
    6. Disks and Files
        - DBMS interfaces는 바이트 단위가 아닌 Block Level(page)에서 읽고 쓰기를 진행한다. (느려서?)
        - Block = Unit of transfer for disk read/write
        - Page = Fixed size contiguous chunk of memory



## Lecture 5 - DiskFilesBuffers Part2

###  5.1 DATABASE FILES

    1. Unordered Heap Files
        – keep track of the pages in a file
        – keep track of free space on pages
        – keep track of the records on a page
        - header page를 만들어서 관리하자!

### 5.2 PAGE LAYOUT


    1. Page Basics: The Header
        - 파일과 마찬가지로 헤더를 만들어서 관리 
        - 다음은 무조건 포함해야 된다.
            – Number of records
            – Free space
            – Maybe a next/last pointer
            – Bitmaps, Slot Table…
        
    2. 고려 사항 : 
        – Record length (fixed or variable) : 어떤 자료형을 얼마나 쓸 것인지
            - Fixed : Header에 Bitmap을 사용하여 저장
            - Variable : Footer에 Slot을 사용하여 저장
        – Page packing (packed or unpacked) : 삭제로 인한 빈 공간에 대해 재정렬할 것인지
            - 공간이 더 이상 없을 때 실행하자 (시간이 많이 소요됨)

### 5.3 RECORD LAYOUT

    1. Record Formats
        - Relational Model
        - 스키마를 저장하는 다른 테이블 존재 
        - Goals
            – 메모리와 디스크에서 저장
            – 빠른 접근

    2. Fixed Length : system catalog사용 아주 간단

    3. Variable Length : 
        - 어디까지가 col 정보? :  // 각각의 디자인에 대한 tradeoff(장단점) 철학을 이해하는 것이 중요 
            - Comma Separated Values (CSV) : ,를 사용하여 구분 
            - Offset을 만들기 (길이 적기)
            - Header를 아예 만들어 버리기 -> 현재 대부분 사용중 

    


## Lecture 6 File Organizations (Files and Index Management)

Review: Indirection 철학은 아주 중요하다. (Data와 그에 대한 Meta-Data)


### 6.1 Access Patterns

    1. Add/Remove particular recordId: Easy (Cost?)

    2. Scan: Easy (Cost?)

    3. Find a record?
        - Given a recordId: (PageId, Slot)?
        - Matching username = “sarahmanning”?

### 6.2 Cost Model for Analysis

용어 정의 

    • B: The number of data blocks in the file
    • R: Number of records per block
    • D: (Average) time to read/write disk block

Heap Files VS Sorted Files

| 비교 대상        | Heap Files      | Sorted Files        |
| ---------------- | --------------- | ------------------- |
| Scan all records | B * D           | B * D               |
| Equality Search  | 0.5* B * D      | (log2B) * D         |
| Range Search     | B * D           | ((log2B)+pages) * D |
| Insert           | 2 * D           | ((log2B)+B) * D     |
| Delete           | (0.5 * B+1) * D | ((log2B)+B) * D     |


## Lecture 7 Static Tree Indexes (Files and Index Management)

### 7.1 INDEX Tree, ISAM

#### 7.1.1 생각해 볼 점

    slot은 record의 page 내에서 이동은 잘 처리해 주나 page 간의 이동은 어렵다. 
     

#### 7.1.2 변경 사항

    (PageId, SlotId) -> (PageId, SearchKey)

#### 7.1.3 특징 

    1. Lookup: 여러 operations 가능
    2. Search Key: 아무 컬럼의 부분 집합
    3. Data Entries: (k, recordID) -> 자기가 정하는거임
    4. Many Types: B+-Tree, Hash, R-Tree, GiST, ...
    5. Modification: 빠른 삽입과 삭제 
    6. Fan-Out = Branching Factor
    7. 정적인 data에 대해서는 강하나 삽입, 삭제시 생기는 overflow page로 기능 저하

## Lecture 8 Dynamic Tree Indexes (Files and Index Management)

### 8.1 Index Tree, B+ Tree

     삽입, 삭제시 생기는 overflow page로 기능 저하를 해결하기 위해 고안

     Structure Modification Operaiton(SMO)

#### 8.1.1 특징 

    – Always Balanced
    – Support efficient insertion & deletion
    – Both API calls are expensive

#### 8.1.2 유사한 기능을 하는 구조 : RB-Tree

    AVL Tree 보다 rotaion이 적은 구조 (AVL page 변화가 너무 많아 쓰지 않는다.)

    높이가 skewed 되기는 한다.


#### 8.1.3 자세한 기능

    이미 코드 분석 과제를 해서 정리 x

    하나 착각한 부분 : delete한다고 꼭 key를 internal node에서 지울 필요 없다.


#### 8.2 Bulk Loading : 한 번에 많은 insert 

    split을 적절한 비대칭으로 진행 = order가 차지 않아도 split 진행 


