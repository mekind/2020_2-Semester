# 형수님의 데이터 베이스 시스템 설계 및 응용
A+을 받아보자

***


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
