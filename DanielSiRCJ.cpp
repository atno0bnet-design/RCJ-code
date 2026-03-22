#include "opencv2/opencv.hpp"
#include <lccv.hpp>
#include <vector>
#include <algorithm>
#include <math.h>

using namespace std;
using namespace cv;


#define h_low 108
#define h_high 148
#define s_low 84
#define s_high 184
#define v_low 16
#define v_high 218


double Rmotor_speed = 20;
double Lmotor_speed = 20;


bool contour_compare(const vector<Point> &a,const vector<Point> &b){
	return contourArea(a)<contourArea(b);
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
    Mat slices[3];
    Mat gray, blurred, edges;
    Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3));
	vector<vector<Point>> contours,green_cont,top,mid,bot;
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
        Mat img = frame.clone();
        Mat display = img.clone();
        Mat green = frame.clone();
        cvtColor(green,green,COLOR_BGR2HSV);
                medianBlur(frame,frame,15);
        cvtColor(frame, frame, COLOR_BGR2HSV);

        
        //threhshold for green for sr
        //inRange(green,Scalar(75,175,95), Scalar(87,255,180),green);
        //threshold green for home
        inRange(green,Scalar(77,168,59),Scalar(86,255,167), green);
        //threshold for line sr
        //inRange(frame, Scalar(47, 27, 18), Scalar(92, 172,130), frame);
        //home threshold for line
        //threshold(frame,frame,170,255,THRESH_BINARY_INV);
        inRange(frame, Scalar(h_low,s_low,v_low),Scalar(h_high,s_high,v_high),frame);
        
        findContours(frame,contours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
        vector<Point> black_line;
        if(contours.size()>0){
			black_line = *max_element(contours.begin(), contours.end(), contour_compare);
			drawContours(display,vector<vector<Point>>(1,black_line),0,Scalar(0,0,255), 3);
		}
		else{
				cout<<"failure"<<endl;
		}
		

		
		findContours(green, green_cont,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
		
		drawContours(display,green_cont,-1,Scalar(0,255,0), 3);
		
		Moments m = moments(black_line);		
		
		double cx = m.m10/m.m00;
		circle(display,Point(cx,50),10,Scalar(255,255,255	),-1);
		line(display,Point(img.cols/2,0),Point(img.cols/2,img.rows),Scalar(0,255,0),LINE_8);
		
		
		
		
		double error = (img.cols/2.0)-cx;
		
		double scaledown = 0.0009;
		
		Rmotor_speed += (error*scaledown);
		Lmotor_speed += (error*-scaledown);
		
		
		cout<<"Error:"<<error<<"r:"<<Rmotor_speed<<"l:"<<Lmotor_speed<<endl;
		

		
		Rect leftSlice(Point(0,0),Point(img.cols*(2/5.0),img.rows));
		Rect midSlice(Point(img.cols*(2/5.0),0),Point(img.cols*(3/5.0),img.rows));
		Rect rightSlice(Point(img.cols*(3/5.0),0),Point(img.cols,img.rows));
		
		Rect topSlice(Point(0,0),Point(img.cols,img.rows*(1/5.0)));
		Rect middleSlice(Point(0,img.rows*(1/5.0)),Point(img.cols,img.rows*(4/5.0)));
		Rect botSlice(Point(0,img.rows*(4/5.0)),Point(img.cols,img.rows));
		
		Mat leftImg = img(leftSlice);
		Mat midImg = img(midSlice);
		Mat rightImg = img(rightSlice);
		
		Mat topImg = img(topSlice);
		Mat middleImg = img(middleSlice);
		Mat botImg = img(botSlice);
		
		Mat topDisplay = topImg.clone();
		Mat middleDisplay = middleImg.clone();
		Mat botDisplay = botImg.clone();
		
		cvtColor(topImg, topImg, COLOR_BGR2HSV);
		cvtColor(middleImg, middleImg, COLOR_BGR2HSV);
		cvtColor(botImg, botImg, COLOR_BGR2HSV);
		
		
		inRange(topImg, Scalar(h_low,s_low,v_low),Scalar(h_high,s_high,v_high),topImg);
		findContours(topImg,top,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
		
		if(top.size()>0){
			black_line = *max_element(top.begin(), top.end(), contour_compare);
			drawContours(topDisplay,vector<vector<Point>>(1,black_line),0,Scalar(0,0,255), 3);
		}
		inRange(middleImg, Scalar(h_low,s_low,v_low),Scalar(h_high,s_high,v_high),middleImg);
		findContours(middleImg,mid,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
		
		if(mid.size()>0){
			black_line = *max_element(mid.begin(), mid.end(), contour_compare);
			drawContours(middleDisplay,vector<vector<Point>>(1,black_line),0,Scalar(0,0,255), 3);
		}
		inRange(botImg, Scalar(h_low,s_low,v_high),Scalar(h_high,s_high,v_high),botImg);
		findContours(botImg,bot,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
		
		if(bot.size()>0){
			black_line = *max_element(top.begin(), top.end(), contour_compare);
			drawContours(botDisplay,vector<vector<Point>>(1,black_line),0,Scalar(0,0,255), 3);
		}
		
		
		imshow("top", topDisplay);
		imshow("middle", middleDisplay);
		imshow("bottom", botDisplay);
			
		imshow("Left", leftImg);
		imshow("Mid", midImg);
		imshow("Right", rightImg);	
		
        imshow("color+contour", display);
      

        int key = waitKey(1);
        if (key == 'q') break;
    }

    destroyAllWindows();
    cam.stopVideo();
    return 0;
}

