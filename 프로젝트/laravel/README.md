# Laravel 공부

## 설치 순서 

- apache 설치 

- php 설치

- composer 설치

- laravel 설치

- laravel new 디렉토리명

- 설정파일 -> config 폴더에 여러가지 

- 디렉토리 권한  storage 와 bootstrap/cache

-  php artisan key:generate

-  config/app.php 살펴보기 (time zone / locale )

--- 

## 기본 설정   

- 라라벨 버젼 확인 : php artisan --version

- config 디렉토리 

- 'timezone' => 'Asia/Seoul',

- 서버 시작: php artisan serve

- 점검 모드: php artisan down --render (/views/errors/503.blade.php)

- 점검 모드 해제: php artisan up

- php artisan help {명령어} : 명령어 설명 

## [디렉토리 구조](https://laravel.kr/docs/7.x/structure)

## [서비스 컨테이너](https://laravel.kr/docs/7.x/container)

    - [DB 연결](https://laravel.kr/docs/7.x/database#configuration)

        - extension=mysqli (이거하는데 1시간 걸림)

        - mysql 설치함 (이거 안 필요한듯)


