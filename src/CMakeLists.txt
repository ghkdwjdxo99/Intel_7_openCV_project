cmake_minimum_required(VERSION 3.10)   # 최소 CMake 버전
project(fingertipsAdd2)                # 프로젝트 이름

# OpenCV 패키지 찾기
find_package(OpenCV REQUIRED)

# 실행 파일 생성, openCV 라이브러리 연결
add_executable(fingertipsAdd2 fingertipsAdd2.cpp)
target_link_libraries(fingertipsAdd2 ${OpenCV_LIBS})
