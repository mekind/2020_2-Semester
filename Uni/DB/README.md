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
        - 

    8. 22


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

    5. Design 철학 : Design Rational 
        - 정답인 디자인은 없다. 
        - 문제 주 원인, 그를 해결할 가설 -> 디자인 
    
    6. Disks and Files
        - DBMS interfaces는 바이트 단위가 아닌 Block Level(page)에서 읽고 쓰기를 진행한다. (느려서?)
        - Block = Unit of transfer for disk read/write
        - Page = Fixed size contiguous chunk of memory
    7. 222
    8. 22


## Lecture 5 - DiskFilesBuffers Part2