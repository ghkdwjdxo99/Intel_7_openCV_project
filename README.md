## 프로젝트 설명
- 웹캠이나 로컬 이미지를 사용해 나만의 퍼즐을 만드는 데스크톱 애플리케이션입니다.
- OepnCV로 퍼즐 조각을 자르고, Qt GUI를 통해 퍼즐을 맞출 수 있습니다.

## 핵심 하이라이트
- 개발 인원 : 3명
- 기간 : 9/19 ~ 9/25 (7일)
- 기술 : Qt(C++), OpenCV
- 목표 : 사용자가 즉시 사진만의 퍼즐을 만들고, 즐길 수 있는 앱 구축

## 트러블 슈팅
1. 특정 조각의 위치가 부정확하게 배치
<img width="380" height="420" alt="image" src="https://github.com/user-attachments/assets/7ade3b9b-955b-4f4d-b8e7-b9b683dd25e3" />
  
  - 증상
    - 일부 퍼즐 조각이 슬롯 중심에 스냅되지 않음
  
  - 원인
    - `puzzle/playpage.cpp:477`에서 `_ay` 좌표를 파싱할 때 정규식 `QRegularExpression re_ay("_ay(-?\\d+)\\.")`가 파일명(`piece_0_ax44_ay43_fx100.png`)에 존재하지 않는 마침표(`.`)를 찾아 매칭 실패했고, `ay`가 `-1`로 남으면서 기본값(`scaled.height()/2`)이 사용돼 위치가 틀어짐
  
  - 해결
    - 후행 문자 조건을 묶어 `QRegularExpression re_ay("_ay(-?\\d+)(?=\\D|$)")`로 교체해 숫자 뒤에 비숫자 문자가 와도 정상적으로 동작하도록 수정. 같은 방식으로 `_ax` 정규식도 보완해 추후 파일명 포맷 변화에도 안정적으로 동작
   
## 핵심 기능
- 5x5 또는 8x8 그리드를 선택해 원하는 난이도로 퍼즐 생성
- 웹캠 캡처 또는 이미지를 불러와 즉시 퍼즐로 변환
- 퍼즐 조각 드래그 앤 드롭, 원본 보기 등 플레이 보조 기능
- 플레이 타이머 및 성공/실패 다이얼로그로 진행 상황 피드백 제공
- OpenCV 마스크 기반 조각 추출로 실제 퍼즐과 유사한 조각 모양 구현

## 구성 요소
- `puzzle/`: Qt 기반 메인 애플리케이션 (퍼즐 생성·편집·플레이 UI, 다이얼로그, 자원 파일 포함)
- `puzzle/images/`: 기본 퍼즐 마스크, 캡처 이미지, 퍼즐 조각이 저장되는 리소스 폴더

## 기술적 구현
- **OpenCV 파이프라인**: 퍼즐 마스크 이미지를 이진화 → contour 추출 → 원본 이미지에서 동일 영역 복사 → 조각 `Mat`·마스크 저장으로 재활용
- **Qt UI/UX**: `QStackedWidget`으로 페이지 전환 관리, Graphics View에서 조각 드래그, 커스텀 다이얼로그(Success/Fail/Solution)로 피드백 제공
- **리소스 관리**: `images/` 폴더에 캡처 이미지와 조각을 저장해 플레이 전환 시 자동으로 불러오도록 구성

## 아키텍처 & 흐름
<img width="1672" height="410" alt="project flow" src="https://github.com/user-attachments/assets/f0a9ecd4-7021-4dcc-97f5-3759246098a0" />

1. 사용자 입력으로 퍼즐 타입 선택
2. 웹캠 캡처 또는 이미지 불러오기 → 캡처 이미지 저장
3. 퍼즐 마스크 기반으로 조각 생성 후 `images/piece_image/`에 저장
4. 플레이 페이지에서 조각 배치 및 타이머 동작
5. 완료 시 성공/실패 다이얼로그로 결과 안내

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
