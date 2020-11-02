- [컴퓨터 네트워크](#컴퓨터-네트워크)
  - [1. Introduction](#1-introduction)
    - [1.1. What's the Internet?](#11-whats-the-internet)
      - [1.1.1 구성 요소](#111-구성-요소)
      - [1.1.2 서비스 측면](#112-서비스-측면)
    - [1.2 Network edge (end systems, access networks, links)](#12-network-edge-end-systems-access-networks-links)
      - [1.2.1 access network](#121-access-network)
      - [1.2.2 Physical media](#122-physical-media)
    - [1.3. Network core(packet switching, circuit switching, network structure)](#13-network-corepacket-switching-circuit-switching-network-structure)
      - [1.3.1 Packet-switching](#131-packet-switching)
      - [1.3.2 Circuit switching](#132-circuit-switching)
      - [1.3.3 다중화](#133-다중화)
      - [1.3.4 Network of network](#134-network-of-network)
      - [1.3.5 Access Network의 연결 문제](#135-access-network의-연결-문제)
    - [1.4. Delay, loss, throughput in network](#14-delay-loss-throughput-in-network)
      - [1.4.1 Processing delay](#141-processing-delay)
      - [1.4.2 Queuing delay](#142-queuing-delay)
      - [1.4.3 Transmission delay](#143-transmission-delay)
      - [1.4.4 Propagation delay](#144-propagation-delay)
      - [1.4.5 Specific Queueing delay](#145-specific-queueing-delay)
      - [1.4.6 처리율 (throughput)](#146-처리율-throughput)
    - [1.5 Protocol layers, service models(OSL Model)](#15-protocol-layers-service-modelsosl-model)
      - [1.5.1 Protocol layers](#151-protocol-layers)
      - [1.5.2 Encapsulation](#152-encapsulation)
    - [1.6 Networks under attack: security](#16-networks-under-attack-security)
    - [1.7 History](#17-history)
  - [2. Application Layer](#2-application-layer)
    - [2.1 Principles of network applications](#21-principles-of-network-applications)
      - [2.1.1 어플리케이션 구조 (Application architecture)](#211-어플리케이션-구조-application-architecture)
      - [2.1.2 프로세스간 통신 (Processes communicating)](#212-프로세스간-통신-processes-communicating)
      - [2.1.3 Transport 서비스](#213-transport-서비스)
      - [2.1.4 Internet transport protocols services](#214-internet-transport-protocols-services)
      - [2.1.5 App-layer protocol defines](#215-app-layer-protocol-defines)
    - [2.2 Web and HTTP](#22-web-and-http)
      - [2.2.1 HTTP 개요](#221-http-개요)
      - [2.2.2 비지속 연결과 지속 연결(HTTP)](#222-비지속-연결과-지속-연결http)
      - [2.2.3 HTTP Message Format](#223-http-message-format)
      - [2.2.4 Cookies](#224-cookies)
      - [2.2.5 Web Caches(=proxy server)](#225-web-cachesproxy-server)
    - [2.3 Electronic mail](#23-electronic-mail)
      - [2.3.1 구성 요소](#231-구성-요소)
      - [2.3.2 과정](#232-과정)
      - [2.3.3 Mail access protocols](#233-mail-access-protocols)
    - [2.4 DNS](#24-dns)
      - [2.4.1 제공하는 서비스](#241-제공하는-서비스)
      - [2.4.2 분산 계층](#242-분산-계층)
    - [2.5 P2P applications](#25-p2p-applications)
      - [2.5.1 BitTorrent](#251-bittorrent)
      - [2.5.2 File Distribution Time](#252-file-distribution-time)
    - [2.6 Video streaming and content distribution networks](#26-video-streaming-and-content-distribution-networks)
      - [2.6.1 요구사항](#261-요구사항)
      - [2.6.2 DASH(Dynamic Adaptive Streaming over HTTP)](#262-dashdynamic-adaptive-streaming-over-http)
    - [2.7 Socket programming with UDP and TCP](#27-socket-programming-with-udp-and-tcp)
    - [3.4 Principles of reliable data transfer](#34-principles-of-reliable-data-transfer)
      - [3.4.1 rdt 1.0](#341-rdt-10)
      - [3.4.2 rdt 2.0 : Channel with bit errors](#342-rdt-20--channel-with-bit-errors)
      - [3.4.3 rdt 2.1 : sender, handles ACK/NAK 오류](#343-rdt-21--sender-handles-acknak-오류)
      - [3.4.4 rdt 3.0 : channels with errors && loss](#344-rdt-30--channels-with-errors--loss)


# 컴퓨터 네트워크


말 그대로 컴퓨터 네트워크에 대해 배우는 수업 

## 1. Introduction

***

### 1.1. What's the Internet?

#### 1.1.1 구성 요소
        
    - Hosts (= end systems) : 인터넷에 연결된 모든 장치 
    
    - End systems는 communication link/ packet switch로 연결되어 있다.
      - 다른 종단 시스템으로 보낼 데이터를 segment로 나구고 각 segment header에 붙인다. -> packet
      - packet switches
          - routers, switches

    - Internet: “network of networks” = interconnected ISP
    - Protocols
    - Internet standards

#### 1.1.2 서비스 측면 

    distributed applications


### 1.2 Network edge (end systems, access networks, links)

#### 1.2.1 access network 

    - digital subscriber line(DSL) : 전화선

        - 전화, 인터넷 -> DSL -> spiltter -> DSLAM(in CO), 하나의 링크가 3개의 링크인 것처럼 행동

    - cable network : TV 선

        - TV, 인터넷 -> cable modem -> splitter -> cable headend (CMTS)

        - frequency division multiplexing

        - HFC(hybrid fiber coax)

        - DSLAM, CMTS -> 아날로그를 디지털로 바꿈


    - LAN

        - Ethernet
            - Ethernet switch 

        - Wifi  
            - access point 제공

        - Wide-area wireless access(LTE, 3G)

#### 1.2.2 Physical media

    - coaxial cable
        - bidirectional
    - fiber optic cable 


### 1.3. Network core(packet switching, circuit switching, network structure)

transmission rate : R = L/총 delay

#### 1.3.1 Packet-switching

    - store-and-forward
    - end-end delay : N*L/R 
    - queuing delay
    - routing / fowarding 
    - 많은 사용자가 사용할 수 있음, 낭비가 없음

#### 1.3.2 Circuit switching 

    - circuit을 예약/확보 후 전송
    - 주로 전화 네트워크가 이용 
    - 일정한 속도 보장 

#### 1.3.3 다중화

    - FDM
        - 일정 대역폭을 고정 제공

    - TDM
        - 일정한 프레임에 시간 슬롯을 설정 

#### 1.3.4 Network of network 

End system -> access network -> ISP -> access network -> End system

#### 1.3.5 Access Network의 연결 문제 

    1. 서로서로 연결 -> 말도 안 됨. 비용이 너무 많이 듦
    2. one global transit ISP? -> 돈을 벌기 위해 + 경쟁을 위해 회사가 각자 global transit ISP 만듦 
    + IXP : peering의 만남의 장소 
    + peering
    + PoP
    + Content Provider Network(Google)

### 1.4. Delay, loss, throughput in network

노드 처리 -> 큐잉 -> 전송 -> 전파 

nodaling processing -> queueing -> transmission -> propagation

#### 1.4.1 Processing delay

    어디로 갈지+오류 여부 처리하는 시간, 일반적으로 msec 

#### 1.4.2 Queuing delay 

    앞에 기다리는 패킷 없으면 0, 가득 차 있으면 drop 

#### 1.4.3 Transmission delay 

    L/R, 패킷의 모든 비트를 링크로 밀어내는 데 필요한 시간 
    R : link bandwidth(bps)

#### 1.4.4 Propagation delay

    D/S : 다른 라우터로 가는데 지연되는 시간 
    D: 실제 링크 길이
    S: 전파 속도 

#### 1.4.5 Specific Queueing delay

    Traffic Intensity: La/R

    1보다 작 : delay small
    같거나 큼 : delay large

#### 1.4.6 처리율 (throughput)

    - instantaneous
    - average

    - 처리율 : min{Rc, Rs}
    - bottleneck link : 작은거

### 1.5 Protocol layers, service models(OSL Model)

#### 1.5.1 Protocol layers

    Application : 
        message 교환 (HTTP, SMTP, FTP), 네트워크 소스에 접근 허가
    Presentation : 
        복호화, 암호화
    Session :
        message 만들고 처리
    Transport : (=segment)zj
        TCP : 연결지향형, 애플리케이션 메세지 전달 보장, 흐름제어, 혼잡제어, 긴 메세지->짧
        UDP : 비연결형, 신뢰성, 흐름제어, 혼잡제어 x
    Network :
        datagram의 routing 책임짐
    Link :
        노드를 링크를 통해 전송, packet = frame 
    Physical :
        각 비트를 한 노드에서 다음 노드로 이동


#### 1.5.2 Encapsulation

    Application : message 생성  (M)
    + Transport : 적절한 어플리케이션으로 보내는 정보 + 오류 검출 비트 (segment) (Ht)
    + Network : 출발지와 종단 시스템 주소 와 동일한 헤더 (datagram) (Hn)
    + Link : 자신의 헤더 정보 (Hl)
    + 

**즉 상위계층 정보 + pay-load field를 갖는다.**

### 1.6 Networks under attack: security

    - malware
    - DoS
    - Packet Sniffing
    - Ip Spoofing

### 1.7 History 

    - Cerf and Kahn’s internetworking principles


## 2. Application Layer

### 2.1 Principles of network applications

core 장비에서 실행되는 소프트웨어를 작성할 필요가 없다.

#### 2.1.1 어플리케이션 구조 (Application architecture)

    - 서버 클라이언트 구조
        - 서버 : 항상 실행, 고정 IP, 많은 사용자 -> 데이터 센터 
        - 클라이언트 : 클라이언트끼리는 통신 x

    - P2P 구조 
        - 호스트끼리 서로 통신
        - self-scalablitiy(자가 확장성)이 높다.

#### 2.1.2 프로세스간 통신 (Processes communicating)

    - socket을 기반으로 작동
        - 어떤 프로세스로 접근할지 결정 with 호스트 주소 + port
        - application과 transport 계층 사이 위치
        - Application Programming Interface 
    - client process, server process

#### 2.1.3 Transport 서비스

    - 신뢰적 데이터 전송(data integrity) : 
        버리는 거 없이 전송
        loss-tolerant application -> 비디오
    - 처리량(throughput) :
        요구되는 처리율 보장
        bandwidth-sensitive application <-> elastic application
    - 시간(timing) :
        상호작용이 필요한 application
    - 보안(security) : 
        암호화/복호화

#### 2.1.4 Internet transport protocols services

    - TCP : 
        - 연결지향형 서비스(connection-oriented) : hand shaking, full-duplex
        - reliable transport
        - flow control : 받는 사람의 속도를 초과하여 보내지 않는다.
        - congestion control : 네트워크가 복잡하면 기다렸다 보낸다.
        - timing, throughput, security x
  
    - UDP
        - 비연결성
        - 도착 보장 x
        - 순서 뒤바뀔 수 있음


    - SSL
        - Data integrity
        - Encryption
        - End-point authentication


#### 2.1.5 App-layer protocol defines

    - types of messages exchanged
    - message syntax
    - message semantics
    - rules

    - open protocols : HTTP, SMTP
    - proprietary protocols : skype

### 2.2 Web and HTTP

On - Demand 방식

#### 2.2.1 HTTP 개요 

**HTTP protocol -> socket -> TCP protocol**

    - 서버 프로그램과 클라이언트 프로그램으로 구성 
    - Web Page : 객체(object)들로 구성 (HTML, IMG, AUDIO 등)
    
**URL : hostname / pathname**

    www.someschool.edu/someDept/pic.gif

HTTP 는 stateless protocol이다. -> 전 상태를 저장하다 오류시 복구하기 어렵다.

#### 2.2.2 비지속 연결과 지속 연결(HTTP)

    No-persistent HTTP :
        connection fetch 후 close
        multiple download -> multiple connections
        2RTT(round-trip time) + file transmssion time

    Persistent HTTP :
        하나의 TCP 커넥션 재사용 (pipelining)

#### 2.2.3 HTTP Message Format

    1. Request 

    - request line   
    {방법} {Pathname}  {버젼}{CR}{LF}     
    CR: 캐리지 끝 명시(\r), LF: 줄끝 명시 (\n)    
  
    예시: GET /index.html HTTP/1.1 \r\n    

    - header linese   
    {Header Field Name} {value}{CR}{LF}   
    
    예시: Host: www-net.cs.umass.edu\r\n   

    - body   
    {body 내용}  {CR}{LF}   

    2. Response 
    - status line
    {버전} {상태코드} {CR}{LF}   
    예시: HTTP/1.1 200 OK\r\n    

    - header lines
    {Header Field Name} {value}{CR}{LF}    
 
    - requested HTML file
    {HTML 내용} {CR}{LF}   

**Method** 

    - POST : Body에 담아 전송
    - GET : URL에 담아 전송
    - PUT/DELETE :1.1에서 추가

#### 2.2.4 Cookies

구성요소 

    1. cookie header line (res)
    2. cookie header line (req)
    3. 쿠키 파일 by browser
    4. back-end database


#### 2.2.5 Web Caches(=proxy server)

    - 첫 번째 요청 때 프록시 서버에 저장해서 다른 같은 요청을 더 빨리 처리
    - access link의 트래픽을 줄여준다.
    - 효율적인 delivery

delay 계산 

    원래 : (lan delay) + access link delay + internet delay

    cache hit rate = 0.4
    (0.6) * delay from origin servers a+ 0.4*(delay when at cache)

conditional GET 

    If-modified-since를 헤더 필드에 추가해 변경시 data 전송 with 304 
    아니면 헤더만 전송 with 200

### 2.3 Electronic mail

#### 2.3.1 구성 요소 

    - user agents
    - mail servers
        - mailbox
        - message queue
    - SMTP(simple mail transfer protocol)


#### 2.3.2 과정

    UA compose message 
    -> 자기 Mail Server(message queue) 
    -> 다른 사람 Mail Server 와 TCP 연결 
    -> message 전달 
    -> 다른사람 Mail Server (message queue) 
    -> 읽기


#### 2.3.3 Mail access protocols

    - POP(Post Office Protocol)
    - IMAP(Internet Mail Access Protocol)
    - HTTP


### 2.4 DNS

#### 2.4.1 제공하는 서비스 

    - Distributed Database
    - Applicatio-layer protocol (port 53)

    - Host aliasing : canonical, alias names
    - Mail sever aliasing
    - load distributuion
    - centralize? 아래 문제 때문에 안됨 
        - single point of failure : 하나 망가지면 끝
        - traffic volume : 하나의 서버에 몰림
        - distant centralized database : 서버와의 거리
        - maintenance
    
#### 2.4.2 분산 계층 

**Distributed, hierarchical database**

    root DNS -> TLD(top level domain) DNS -> authoritative DNS

    local DNS : (ISP마다 가지고 있다.) = default DNS , caching도 함

**DNS name Resolution**

    - iterated 
    - recursive


### 2.5 P2P applications

#### 2.5.1 BitTorrent

**Swarming Pool**

    - A file is chopped into small pieces, called chunks

    Component

    - Web server
    - The .torrent file
        - name
        - size
        - checksum
    - Tracker
    - Peers

    Policy

    - Rarest First
    - tit-for-tat
    - optimistically unchoke

#### 2.5.2 File Distribution Time

    Dc-s> max{NF/us,F/dmin}

    DP2P> max{F/us,F/dmin,,NF/(us+ Σui)}

### 2.6 Video streaming and content distribution networks


Problem : 
    - scaling : 확장성
    - heterogeneity : 유저마다 다른 환경

Solution :
    - distributed, application-level infrastructure -> CDN

#### 2.6.1 요구사항

    - video
    - digital image         
    - coding :
        - spatial
        - temporal

#### 2.6.2 DASH(Dynamic Adaptive Streaming over HTTP)


### 2.7 Socket programming with UDP and TCP



### 3.4 Principles of reliable data transfer

transport layer -> network의 unreliable을 reliable로 바꿈 

rdt_send(data) -> transport layer (send side) -> udt_send() 
-> rdt_rcv() -> transport layer(receive side) -> deliver_data()

FSM : 설명을 위한 다이어그램(?)

state -> {event causing action} / {action} -> state2

#### 3.4.1 rdt 1.0 

    - sender : wait -> rdt_send() / make_pkt(), udt_send() -> wait

    - receiver : wait -> rdt_rcv() /extract(), deliver_data -> wait

#### 3.4.2 rdt 2.0 : Channel with bit errors

check : checksum to detect bit errors 

recover : resend
    - acknowledgements(ACKs)
    - negative acknowledgements (NAKs)

sender : wait -> rdt_send()/ make_pkt(), udt_send() -> wait(ACK/NAK) ->rdc_rcv() && isNAK() / udt_send() -> rdt_rcv() && isACK() / ^(nothing)

receiver : 
wait -> rdt_rcv() && corrupt()/ udt_send(NAK) -> wait 
wait -> rdt_rcv() && notcorrupt() -> extract(), deliver_data(), udt_send(ACK) -> wait 



#### 3.4.3 rdt 2.1 : sender, handles ACK/NAK 오류 

rdt 2.0 fatal flow : ACK/NAK 에 오류가 있을 경우

- 모르겠으면 그냥 다시 보낼까? duplicate 생김
- sequence number을 패킷에 저장하자. -> 중복이면 버려

state 증가 -> seq 증가 

#### 3.4.4 rdt 3.0 : channels with errors && loss

타이머 추가 


