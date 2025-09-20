#include <opencv2/opencv.hpp>
#include <iostream>
#include <deque>
#include <numeric>
using namespace cv;
using namespace std;

// 두 점 사이 각도 계산
static inline double angleBetween(const Point& s, const Point& f, const Point& e) {
    double a = norm(s - f), b = norm(e - f), c = norm(e - s);
    if (a < 1e-5 || b < 1e-5) return 180.0;
    double cosv = (a*a + b*b - c*c) / (2*a*b);
    cosv = max(-1.0, min(1.0, cosv));
    return acos(cosv) * 180.0 / CV_PI;
}

int main() {
    VideoCapture cap(0, cv::CAP_V4L2);
    if (!cap.isOpened()) {
        cerr << "Camera open failed!\n";
        return -1;
    }

    const Scalar YCrCb_low(0, 133, 77);
    const Scalar YCrCb_high(255, 173, 127);

    Mat frame, ycrcb, maskSkin, handOnly;
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(5, 5));

    deque<int> countHistory;
    const int histSize = 5;

    while (true) {
        if (!cap.read(frame) || frame.empty()) {
            cerr << "Frame capture failed!" << endl;
            continue;
        }

        // 피부색 마스크 생성
        cvtColor(frame, ycrcb, COLOR_BGR2YCrCb);
        inRange(ycrcb, YCrCb_low, YCrCb_high, maskSkin);

        // 마스크 정제 (잡음 제거) - 너무 강하게 하지 않음
        morphologyEx(maskSkin, maskSkin, MORPH_OPEN, kernel);
        morphologyEx(maskSkin, maskSkin, MORPH_CLOSE, kernel);
        GaussianBlur(maskSkin, maskSkin, Size(3, 3), 0);

        // Hand Only 이미지 (손만 추출, 배경=검정)
        handOnly = Mat::zeros(frame.size(), frame.type());
        frame.copyTo(handOnly, maskSkin);

        // 제스처 분석용 컨투어
        vector<vector<Point>> contours;
        findContours(maskSkin.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        string action = "None";

        if (!contours.empty()) {
            // 가장 큰 컨투어 선택 = 손
            size_t maxIdx = 0;
            double maxArea = 0;
            for (size_t i = 0; i < contours.size(); i++) {
                double area = contourArea(contours[i]);
                if (area > maxArea) { maxArea = area; maxIdx = i; }
            }
            vector<Point> cnt = contours[maxIdx];

            if (cnt.size() >= 5) {
                vector<Point> approx;
                approxPolyDP(cnt, approx, 0.01 * arcLength(cnt, true), true);

                // 중심점
                Moments mm = moments(approx);
                Point2f c((float)(mm.m10 / max(mm.m00, 1e-5)),
                          (float)(mm.m01 / max(mm.m00, 1e-5)));
                circle(handOnly, c, 4, Scalar(255, 0, 0), FILLED);

                // Convex Hull + Defects
                vector<int> hullIdx;
                convexHull(approx, hullIdx, false, false);
                vector<Vec4i> defects;
                if (hullIdx.size() > 3) {
                    try {
                        convexityDefects(approx, hullIdx, defects);
                    } catch (const cv::Exception& e) {
                        cerr << "convexityDefects error: " << e.what() << endl;
                        defects.clear();
                    }
                }

                // 손끝 후보
                vector<Point> tips;
                for (const auto& d : defects) {
                    int s = d[0], e = d[1], f = d[2];
                    float depth = d[3] / 256.0f;
                    Point ps = approx[s], pe = approx[e], pf = approx[f];
                    double ang = angleBetween(ps, pf, pe);
                    if (depth > 10.0f && ang < 90.0) {
                        tips.push_back(ps);
                        tips.push_back(pe);
                    }
                }

                // 중복 제거 + 손목 제거
                vector<Point> uniq;
                const int minDist = 20;
                for (const auto& p : tips) {
                    if (p.y >= c.y) continue;
                    bool keep = true;
                    for (const auto& q : uniq)
                        if (norm(p - q) < minDist) { keep = false; break; }
                    if (keep) uniq.push_back(p);
                }

                // 손가락 개수 안정화
                int fingerCount = (int)uniq.size();
                countHistory.push_back(fingerCount);
                if (countHistory.size() > histSize) countHistory.pop_front();
                int avgCount = accumulate(countHistory.begin(), countHistory.end(), 0) /
                               (int)countHistory.size();

                // 제스처 판별
                if (avgCount == 0) {
                    action = "Fist -> Undo One";
                }
                else if (avgCount >= 4) {
                    action = "All Fingers -> Clear All";
                }
                else if (avgCount == 2 && uniq.size() >= 2) {
                    Point p1 = uniq[0], p2 = uniq[1];
                    double dist = norm(p1 - p2);
                    int yDiff = abs(p1.y - p2.y);

                    if (p1.y < c.y && p2.y < c.y && yDiff < 50 && dist > 80) {
                        action = "V Sign -> Color Select";
                    } else {
                        action = "Index+Thumb -> Drawing Mode";
                    }
                }
                else if (avgCount == 1 && uniq.size() >= 1) {
                    action = "One Finger -> Mouse Pointer";
                }

                // 손가락 끝 좌표 표시
                for (const auto& p : uniq) {
                    circle(handOnly, p, 8, Scalar(0, 255, 255), FILLED);
                }
            }
        }

        // 결과 출력
        putText(handOnly, action, Point(50, 50), FONT_HERSHEY_SIMPLEX,
                1.0, Scalar(0, 0, 255), 2);

        imshow("Original", frame);
        imshow("Skin Mask", maskSkin);
        imshow("Hand Only", handOnly);

        int k = waitKey(1) & 0xFF;
        if (k == 'q' || k == 27) break;
    }

    return 0;
}
