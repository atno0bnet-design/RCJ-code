#include "opencv2/opencv.hpp"
#include <lccv.hpp>
#include <vector>
#include <algorithm>

using namespace std;
using namespace cv;

Scalar getAverageBGR(const Mat& frame, const RotatedRect& box) {
   
    Mat mask = Mat::zeros(frame.size(), CV_8UC1);
    Point2f pts[4];
    box.points(pts);
    vector<Point> poly;
    for (int i = 0; i < 4; i++) poly.push_back(pts[i]);
    fillConvexPoly(mask, poly, Scalar(255));
    return mean(frame, mask);   
}
bool contour_compare(const vector<Point> &a,const vector<Point> &b){
	return contourArea(a)>contourArea(b);
}
int main() {
    lccv::PiCamera cam;
    
    cam.options->camera       = 0;
    cam.options->video_width  = 1640;
    cam.options->video_height = 1232;
    cam.options->framerate    = 30;
    cam.options->verbose      = false;
    
    cam.startVideo();

    Mat frame;
    Mat gray, blurred, edges;
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	vector<vector<Point>> contours,green_cont;
    namedWindow("Frame");
    namedWindow("Edges");

    while (true) {
        if (!cam.getVideoFrame(frame, 1000)) {
            cout << "CAM ERROR" << endl;
            break;
        }

        if (frame.empty()) {
            cout << "Frame empty" << endl;
            break;
        }
        resize(frame,frame,Size(frame.cols/4,frame.rows/4));
        imshow("imggggg", frame);
        Mat img = frame.clone();
        Mat green = frame.clone();
        cvtColor(green,green,COLOR_BGR2HSV);
        cvtColor(frame, frame, COLOR_BGR2GRAY);
        medianBlur(frame,frame,7);
        
        //threhshold for green for sr
        //inRange(green,Scalar(75,175,95), Scalar(87,255,180),green);
        //threshold green for home
        inRange(green,Scalar(77,168,59),Scalar(86,255,167), green);
        //threshold for line sr
        //inRange(frame, Scalar(47, 27, 18), Scalar(92, 172,130), frame);
        //home threshold for line
        inRange(frame, Scalar(30,12,13),Scalar(106,63,40),frame);
        
        findContours(frame,contours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
        vector<Point> line;
        if(contours.size()>0){
			line = *max_element(contours.begin(), contours.end(), contour_compare);
			drawContours(img,vector<vector<Point>>(1,line),0,Scalar(0,0,255), 3);
		}
        cout<< contours.size() << endl;
		
		findContours(green, green_cont,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
		
		drawContours(img,green_cont,-1,Scalar(0,255,0), 3);
        imshow("Frame", frame);
        imshow("Green", green);
        imshow("color+contour", img);
      

        int key = waitKey(1);
        if (key == 'q') break;
    }

    destroyAllWindows();
    cam.stopVideo();
    return 0;
}

