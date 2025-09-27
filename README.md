# 조각조각 (Intel_7_openCV_project)

## 프로젝트 설명
- 웹캠이나 로컬 이미지를 사용해 나만의 퍼즐을 만드는 데스크톱 애플리케이션입니다.
- OepnCV로 퍼즐 조각을 자르고, Qt GUI를 통해 퍼즐을 맞출 수 있습니다.

## 핵심 기능
- 5x5 또는 8x8 그리드를 선택해 원하는 난이도로 퍼즐 생성
- 웹캠 캡처 또는 이미지를 불러와 즉시 퍼즐로 변환
- 퍼즐 조각 드래그 앤 드롭, 원본 보기 등 플레이 보조 기능
- 플레이 타이머 및 성공/실패 다이얼로그로 진행 상황 피드백 제공
- OpenCV 마스크 기반 조각 추출로 실제 퍼즐과 유사한 조각 모양 구현

## 구성 요소
- `puzzle/`: Qt 기반 메인 애플리케이션 (퍼즐 생성·편집·플레이 UI, 다이얼로그, 자원 파일 포함)
- `puzzle/images/`: 기본 퍼즐 마스크, 캡처 이미지, 퍼즐 조각이 저장되는 리소스 폴더

## 기술 스택
- Qt(C++)
- OpenCV(이미지 캡처 및 마스킹)

## 흐름도
<img width="1672" height="410" alt="image" src="https://github.com/user-attachments/assets/f0a9ecd4-7021-4dcc-97f5-3759246098a0" />

## 시연 영상
https://github.com/user-attachments/assets/ef41ac75-5be2-4608-a524-de82b3b2a481

## 발표 자료
[OpenCV_Project_team3(조각조각).pdf](https://github.com/user-attachments/files/22571474/OpenCV_Project_team3.pdf)


## 빌드 & 실행 방법
1. Qt와 OpenCV 개발 환경을 준비합니다.
2. Qt Creator 또는 명령행에서 `puzzle/puzzle.pro`를 열어 빌드합니다.
3. 빌드된 실행 파일을 통해 퍼즐 타입을 고르고, 이미지 캡처/선택 → 퍼즐 생성 → 플레이 순서로 진행합니다.
4. `puzzle.cpp` 예제를 단독으로 실행하려면 OpenCV가 링크되도록 컴파일한 뒤 웹캠과 `puzzle_mask.png` 파일을 같은 디렉터리에 둡니다.

## 참고 사항
- 퍼즐 마스크 이미지를 교체하면 다른 모양의 조각으로 퍼즐을 만들 수 있습니다.
- 캡처한 이미지와 조각은 `images/` 하위 폴더에 반복 사용을 위해 자동 저장됩니다.
