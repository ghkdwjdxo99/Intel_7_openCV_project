#include <opencv2/opencv.hpp>
#include <iostream>
#include <deque>
#include <numeric>
using namespace cv;
using namespace std;

static inline double angleBetween(const Point& s, const Point& f, const Point& e) {
    double a = norm(s - f), b = norm(e - f), c = norm(e - s);
    if (a < 1e-5 || b < 1e-5) return 180.0;
    double cosv = (a*a + b*b - c*c) / (2*a*b);
    cosv = max(-1.0, min(1.0, cosv));
    return acos(cosv) * 180.0 / CV_PI;
}

int main() {
    VideoCapture cap(0, cv::CAP_V4L2); // 카메라 인덱스 환경에 맞게 수정 필요
    if (!cap.isOpened()) { cerr << "Camera open failed!\n"; return -1; }

    const Scalar YCrCb_low(0, 133, 77), YCrCb_high(255, 173, 127);
    Mat frame, ycrcb, mask, morph;

    // 손가락 개수 안정화를 위한 history
    deque<int> countHistory;
    const int histSize = 5;

    while (true) {
        if (!cap.read(frame) || frame.empty()) {
            cerr << "Frame capture failed!" << endl;
            continue;
        }

        // 피부 마스크
        cvtColor(frame, ycrcb, COLOR_BGR2YCrCb);
        inRange(ycrcb, YCrCb_low, YCrCb_high, mask);
        Mat k = getStructuringElement(MORPH_ELLIPSE, Size(5,5));
        morphologyEx(mask, morph, MORPH_OPEN, k);
        morphologyEx(morph, morph, MORPH_CLOSE, k, Point(-1,-1), 2);

        // 가장 큰 컨투어
        vector<vector<Point>> contours;
        findContours(morph.clone(), contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        Mat out = frame.clone();

        if (!contours.empty()) {
            size_t mi=0; double ma=0;
            for (size_t i=0;i<contours.size();++i){ double a=contourArea(contours[i]); if(a>ma){ma=a; mi=i;} }

            if (ma > 1000 && contours[mi].size() >= 5) {
                vector<Point> cnt = contours[mi], approx;
                approxPolyDP(cnt, approx, 0.01 * arcLength(cnt, true), true);

                // 중심점
                Moments mm = moments(approx);
                Point2f c( (float)(mm.m10/max(mm.m00,1e-5)), (float)(mm.m01/max(mm.m00,1e-5)) );
                circle(out, c, 4, Scalar(255,0,0), FILLED);

                // Hull + Defects
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

                drawContours(out, vector<vector<Point>>{approx}, -1, Scalar(0,255,0), 2);

                // 결함 기반 끝점 후보
                vector<Point> tips;
                for (const auto& d : defects) {
                    int s=d[0], e=d[1], f=d[2];
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
                    if (p.y >= c.y) continue; // 손목 아래 제거
                    bool keep = true;
                    for (const auto& q : uniq) if (norm(p - q) < minDist) { keep = false; break; }
                    if (keep) uniq.push_back(p);
                }

                // ---------------------------------
                // 손가락 개수 안정화 (moving average)
                // ---------------------------------
                int fingerCount = (int)uniq.size();
                countHistory.push_back(fingerCount);
                if (countHistory.size() > histSize) countHistory.pop_front();
                int avgCount = accumulate(countHistory.begin(), countHistory.end(), 0) / (int)countHistory.size();

                // -------------------------------
                // 제스처 판별 (세그폴트 방지 조건 추가)
                // -------------------------------
                string action = "None";

                if (avgCount == 0) {
                    action = "Fist -> Undo Last Step";
                }
                else if (avgCount >= 4) {
                    action = "All Fingers Open -> Clear Canvas";
                }
                else if (avgCount == 1 && uniq.size() >= 1) {
                    // 가장 위쪽 손가락만 남김
                    Point topFinger = *min_element(uniq.begin(), uniq.end(), [](Point a, Point b){ return a.y < b.y; });
                    circle(out, topFinger, 7, Scalar(0,255,255), FILLED);
                    action = "One Finger -> Mouse Pointer";
                }
                else if (avgCount == 2 && uniq.size() >= 2) {
                    Point p1 = uniq[0], p2 = uniq[1];
                    double dist = norm(p1 - p2);
                    double angle = atan2(abs(p1.y - p2.y), abs(p1.x - p2.x)) * 180.0 / CV_PI;

                    if (dist > 100 && angle > 60) {
                        action = "V Sign -> Open Color Picker";
                    } else {
                        action = "Index + Thumb -> Drawing Mode";
                    }
                    circle(out, p1, 7, Scalar(0,255,255), FILLED);
                    circle(out, p2, 7, Scalar(0,255,255), FILLED);
                }

                // 화면에 동작 출력
                putText(out, action, Point(50,50), FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0,255,0), 2);
            }
        }

        imshow("Hand + Gestures", out);

        int kkey = waitKey(1) & 0xFF;
        if (kkey == 'q' || kkey == 27) break; // ESC or q 종료
    }
    return 0;
}

