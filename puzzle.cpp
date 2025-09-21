#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
using namespace cv;
using namespace std;

struct PuzzlePiece {
    Mat img;   // 조각 이미지
    Mat mask;  // 조각 마스크
    Point pos; // 정답 위치
};

// 퍼즐 마스크에서 contour 기반으로 조각 추출
vector<PuzzlePiece> makePuzzlePieces(const Mat& img, const Mat& maskTemplate) {
    vector<PuzzlePiece> pieces;

    // 1. 그레이스케일 + 이진화
    Mat gray, binary;
    if (maskTemplate.channels() == 3)
        cvtColor(maskTemplate, gray, COLOR_BGR2GRAY);
    else
        gray = maskTemplate.clone();

    threshold(gray, binary, 200, 255, THRESH_BINARY);

    // 2. 조각별 contour 찾기
    vector<vector<Point>> contours;
    findContours(binary, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    for (size_t i = 0; i < contours.size(); i++) {
        Mat mask = Mat::zeros(img.size(), CV_8UC1);
        drawContours(mask, contours, (int)i, Scalar(255), FILLED);

        // 원본 이미지에서 퍼즐 조각 추출
        Mat piece;
        img.copyTo(piece, mask);

        Rect bbox = boundingRect(contours[i]);

        PuzzlePiece pp;
        pp.img = piece(bbox).clone();
        pp.mask = mask(bbox).clone();
        pp.pos = bbox.tl();
        pieces.push_back(pp);
    }

    return pieces;
}

// 퍼즐 그리기
Mat drawPuzzle(const Mat& bg, const vector<PuzzlePiece>& pieces, bool randomPos) {
    Mat canvas(bg.size(), bg.type(), Scalar(200,200,200));
    RNG rng((uint64)getTickCount());

    for (auto& p : pieces) {
        Point pos;
        if (randomPos) {
            pos.x = rng.uniform(0, bg.cols - p.img.cols);
            pos.y = rng.uniform(0, bg.rows - p.img.rows);
        } else {
            pos = p.pos;
        }

        Rect target(pos.x, pos.y, p.img.cols, p.img.rows);
        p.img.copyTo(canvas(target), p.mask);
    }
    return canvas;
}

int main() {
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "❌ 웹캠 열기 실패!" << endl;
        return -1;
    }

    Mat maskTemplate = imread("puzzle_mask.png");
    if (maskTemplate.empty()) {
        cerr << "❌ puzzle_mask.png 열기 실패!" << endl;
        return -1;
    }

    Mat frame;
    vector<PuzzlePiece> lastPieces;
    int imgCount = 0;

    cout << "웹캠 실행 중...\n"
         << "'c' = 캡처 & 퍼즐 생성\n"
         << "Enter = 정답 보기\n"
         << "'q' = 종료\n";

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        imshow("Webcam", frame);

        int key = waitKey(30);
        if (key == 'c') {
            string filename = "capture_" + to_string(imgCount++) + ".jpg";
            imwrite(filename, frame);
            cout << "📸 캡처됨: " << filename << endl;

            imshow("Captured Image", frame);

            // 마스크도 frame 크기에 맞게 리사이즈
            Mat maskResized;
            resize(maskTemplate, maskResized, frame.size());

            lastPieces = makePuzzlePieces(frame, maskResized);

            Mat puzzleRand = drawPuzzle(frame, lastPieces, true);
            imshow("Puzzle Pieces", puzzleRand);
        }
        else if (key == 13) { // Enter
            if (!lastPieces.empty()) {
                Mat puzzleSolved = drawPuzzle(frame, lastPieces, false);
                imshow("Puzzle Solution", puzzleSolved);
                cout << "✅ 정답 퍼즐 창 띄움" << endl;
            } else {
                cout << "⚠️ 먼저 'c'로 퍼즐을 생성하세요!" << endl;
            }
        }
        else if (key == 'q' || key == 27) {
            break;
        }
    }

    cap.release();
    destroyAllWindows();
    return 0;
}
