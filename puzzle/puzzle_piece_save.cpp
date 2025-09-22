#include "puzzle_piece_save.h"
#include <random>

// (유지) 기본 생성자
puzzle_piece_save::puzzle_piece_save() {}

// ⭐ 수정: 보더 터치 여부 확인 헬퍼
static inline bool touchesBorder(const cv::Rect& r, const cv::Size& sz) {
    return (r.x == 0) || (r.y == 0) ||
           (r.x + r.width  == sz.width) ||
           (r.y + r.height == sz.height);
}

// ⭐ 수정: 퍼즐 조각 분리 로직을 “선=경계, 내부=연결요소” 방식으로 변경
std::vector<PuzzlePiece> puzzle_piece_save::makePuzzlePieces(const cv::Mat &src, const cv::Mat &mask)
{
    std::vector<PuzzlePiece> pieces;

    if (src.empty() || mask.empty()) return pieces;

    // 1) 마스크를 그레이스케일로 확보
    cv::Mat gray;
    if (mask.channels() == 1) gray = mask.clone();
    else                      cv::cvtColor(mask, gray, cv::COLOR_BGR2GRAY);

    // 2) “퍼즐 선”을 검출 (선=어두움). 선을 흰색으로 분리
    //    선(검정) 쪽이 255가 되도록 역이진화
    cv::Mat lineBin;
    cv::threshold(gray, lineBin, 128, 255, cv::THRESH_BINARY_INV);

    // 3) 내부 채움 마스크 = 선을 제외한 영역(=조각 내부 + 외부 배경)
    cv::Mat fillMask;
    cv::bitwise_not(lineBin, fillMask); // 선=0, 그 외=255

    // 4) 연결요소(255 영역) 라벨링
    cv::Mat labels, stats;
    cv::Mat centroids;
    int nLabels = cv::connectedComponentsWithStats(fillMask, labels, stats, centroids, 8, CV_32S);

    // 5) 라벨 1..n-1 순회. “이미지 외곽을 터치하는 큰 외부 배경 컴포넌트”는 제외
    for (int k = 1; k < nLabels; ++k) {
        int left   = stats.at<int>(k, cv::CC_STAT_LEFT);
        int top    = stats.at<int>(k, cv::CC_STAT_TOP);
        int width  = stats.at<int>(k, cv::CC_STAT_WIDTH);
        int height = stats.at<int>(k, cv::CC_STAT_HEIGHT);
        int area   = stats.at<int>(k, cv::CC_STAT_AREA);

        // 너무 작은 노이즈 제거
        if (area < 20) continue;

        cv::Rect r(left, top, width, height);

        // 외부 배경(프레임 경계와 닿는 컴포넌트) 스킵
        if (touchesBorder(r, fillMask.size())) continue;

        // 6) 이 라벨만 흰색(255)인 조각 마스크 생성
        cv::Mat pieceMaskFull(fillMask.size(), CV_8UC1, cv::Scalar(0));
        pieceMaskFull.setTo(255, (labels == k)); // 해당 라벨 위치만 255

        // ROI로 축소
        cv::Mat alpha = pieceMaskFull(r).clone();

        // 7) 원본에서 해당 ROI 추출
        cv::Mat colorROI = src(r).clone();
        if (colorROI.channels() == 1) {
            cv::cvtColor(colorROI, colorROI, cv::COLOR_GRAY2BGR);
        }

        // 8) RGBA 병합 (알파 = 조각 내부 255, 외부 0)
        cv::Mat rgba;
        std::vector<cv::Mat> ch;
        cv::split(colorROI, ch);           // B,G,R
        ch.push_back(alpha);               // A
        cv::merge(ch, rgba);               // BGRA

        PuzzlePiece pp;
        pp.img  = rgba;                    // ⭐ RGBA로 보관
        pp.mask = alpha;                   // (선택) 알파 원본
        pp.pos  = r.tl();                  // 원래 위치(정답 좌표)
        pieces.push_back(pp);
    }

    return pieces;
}

// (유지) 퍼즐 조립
cv::Mat puzzle_piece_save::drawPuzzle(const cv::Mat &src, const std::vector<PuzzlePiece> &pieces, bool shuffle)
{
    cv::Mat canvas(src.size(), src.type(), cv::Scalar::all(0));

    std::vector<cv::Point> positions;
    positions.reserve(pieces.size());
    for (const auto& p : pieces) positions.push_back(p.pos);

    if (shuffle) {
        std::mt19937 rng{std::random_device{}()};
        std::shuffle(positions.begin(), positions.end(), rng);
    }

    for (size_t i = 0; i < pieces.size(); ++i) {
        const cv::Mat &pimg = pieces[i].img;          // RGBA
        cv::Mat bgr, a;
        if (pimg.channels() == 4) {
            std::vector<cv::Mat> ch; cv::split(pimg, ch);
            a = ch[3];
            cv::merge(std::vector<cv::Mat>{ch[0],ch[1],ch[2]}, bgr);
        } else {
            bgr = pieces[i].img.clone();
            a   = cv::Mat(pimg.size(), CV_8UC1, cv::Scalar(255));
        }

        cv::Rect dst(positions[i].x, positions[i].y, bgr.cols, bgr.rows);
        cv::Mat roi = canvas(dst);
        bgr.copyTo(roi, a);
    }
    return canvas;
}
