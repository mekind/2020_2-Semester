프로젝트 생성
django-admin startproject mysite

원하는 디텍토리 생성 
python manage.py startapp polls
+ URLconf 사용을 위해 urls.py 생성 


새로운 라우트 추가 path('/dir', include(urls.py))

path 함수 path( route, view, (kwargs), name)

path(' ', views.xxx, name=' ')

시간 설정 및 setting.py 

 python manage.py migrate


모델 설계 
apps.py setting에 추가

python manage.py makemigrations polls --model 활성화
python manage.py sqlmigrate polls 0001 -- 실행하는 sql 명령 보여줌
python manage.py migrate -- tabel 만듦


관리자 생성 
python manage.py createsuperuser

하드코딩 view 추가 
 templates 폴더 생성 

static 폴더 생성

