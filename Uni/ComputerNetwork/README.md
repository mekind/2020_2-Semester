# 컴퓨터 네트워크


말 그대로 컴퓨터 네트워크에 대해 배우는 수업 

## Chapter 1. Introduction

***

### What's the Internet?

    1.1 구성 요소
        
        - Edgehosts (= end systems) 2020년, 250억 예상

    communication links

    packet switches

    internet : "network of networks"

    protocols

    Internet standards

### What’s a protocol?

### Network edge: hosts, access net, physical media

### Network core: packet/circuit switching, Internet structure

### Performance: loss, delay, throughput

### Security

### Protocol layers, service models

### History


TCP 연결 후 HTTP 연결

이제 TCP 필요없으니까 종료 

1. persistent

2. non-persistent 

## 09.29

### 1. Request 

    - request line   
    {방법} {URL}  {버젼}{CR}{LF}     
    CR: 캐리지 끝 명시(\r), LF: 줄끝 명시 (\n)    
  
    예시: GET /index.html HTTP/1.1 \r\n    

    - header linese   
    {Header Field Name} {value}{CR}{LF}   
    
    예시: Host: www-net.cs.umass.edu\r\n   

    - body   
    {body 내용}  {CR}{LF}   

### 2. response 
    - status line
    {버전} {상태코드} {CR}{LF}   
    예시: HTTP/1.1 200 OK\r\n    

    - header lines
    {Header Field Name} {value}{CR}{LF}    
 
    - requested HTML file
    {HTML 내용} {CR}{LF}   