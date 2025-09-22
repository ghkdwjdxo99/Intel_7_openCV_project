#ifndef PUZZLE_PIECE_SAVE_H
#define PUZZLE_PIECE_SAVE_H

#include <opencv2/opencv.hpp>
#include <vector>

// 퍼즐 조각을 담는 구조체
struct PuzzlePiece {
    cv::Mat img;    // 퍼즐 조각 이미지
    cv::Mat mask;   // 퍼즐 조각 마스크 (투명 배경 생성용)
    cv::Point pos;  // 퍼즐 원래 위치
};

class puzzle_piece_save
{
public:
    puzzle_piece_save();

    // 퍼즐 분할 함수
    static std::vector<PuzzlePiece> makePuzzlePieces(const cv::Mat &src, const cv::Mat &mask);

    // 퍼즐 전체를 그리는 함수 (정답 이미지, 섞은 이미지)
    static cv::Mat drawPuzzle(const cv::Mat &src, const std::vector<PuzzlePiece> &pieces, bool shuffle);

};

#endif // PUZZLE_PIECE_SAVE_H
