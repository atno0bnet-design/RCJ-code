      
#include "opencv2/opencv.hpp"
#include <lccv.hpp>
#include <vector>
#include <algorithm>
#include <math.h>
#include <string.h>
#include <thread>
#include <chrono>
#include <termios.h>
#include <fcntl.h>
#include <sys/signal.h>
#include <sstream>


using namespace std::chrono_literals;
using namespace std;
using namespace cv;


#define h_low 108
#define h_high 148
#define s_low 84
#define s_high 184
#define v_low 16
#define v_high 218

#define BAUDRATE B115200
#define MODEMDEVICE "/dev/ttyAMA0"

int counter =0; 
int uart0_filestream = -2;

char buff[100];
char rx_buff[100];
int rx_length = 0;
double kp = 0.77;
int RS =30;
int LS = 30;

int targetSpeed = 30;

void init(){
	uart0_filestream = open(MODEMDEVICE, O_RDWR, O_NOCTTY,O_NDELAY);
	if(uart0_filestream == -1){
		printf("Unable to open UART\n");
		exit(-1);
	}
	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);
}

void sendSpeed(int scenario, int FL, int BL, int FR, int BR){
	sprintf(buff, "[%d,%d,%d,%d,%d]", scenario, FL, BL, FR, BR);
	int count = write(uart0_filestream,&buff[0],strlen(buff));
	if(count < 0){
		printf("Error sending teh message");		
	}
	
	
	
	
	this_thread::sleep_for(200ms);
}


bool contour_compare(const vector<Point> &a,const vector<Point> &b){
	return contourArea(a)<contourArea(b);
}
int main() {
	init();
	
	
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
	vector<vector<Point>> contours,green_cont,mid, left, right,botHaf;
    namedWindow("Frame");
    namedWindow("Edges");
    
    
    
    printf("waiting for pico\n");
    
    
    while(rx_length <= 0){
		rx_length = read(uart0_filestream,(void*)rx_buff,9);
	}
	rx_length = 0;
	
	
	int stop_motors = 0;
	
    while (true) {
        if (!cam.getVideoFrame(frame, 1000)) {
            cout << "CAM ERROR" << endl;
            break;
        }

        if (frame.empty()) {
            cout << "Frame empty" << endl;
            break;
        }
        RS = 25;
        LS = 25;
		rx_length = 0;
        resize(frame,frame,Size(frame.cols/4,frame.rows/4));
        blur(frame,frame,Size(7 ,7));
        
        Mat img = frame.clone();
        Mat display = img.clone();
        
        Mat green = frame.clone();
        cvtColor(green,green,COLOR_BGR2HSV);
        medianBlur(frame,frame,3);
        cvtColor(frame, frame, COLOR_BGR2HSV);

        
        
        //threhshold for green for sr
        inRange(green,Scalar(56, 84, 29), Scalar(103, 255,231),green);
        //threshold green for home
        //~ inRange(green,Scalar(77,168,59),Scalar(86,255,167), green);
        //threshold for line sr
        inRange(frame, Scalar(98, 74, 18), Scalar(136, 185,180), frame);
        //home threshold for line
        //threshold(frame,frame,170,255,THRESH_BINARY_INV);
        //~ inRange(frame, Scalar(h_low,s_low,v_low),Scalar(h_high,s_high,v_high),frame);
        imshow("not failure", frame);
        findContours(frame,contours,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
        vector<Point> black_line, left_line, right_line, mid_line, bot_line;
        if(contours.size()>0){
			black_line = *max_element(contours.begin(), contours.end(), contour_compare);
			drawContours(display,vector<vector<Point>>(1,black_line),0,Scalar(0,0,255), 3);
			Moments m = moments(black_line);		
			double cx = m.m10/m.m00;
			double error = ((float)m.m10/m.m00-(frame.cols/2.0))*kp;
			cout<<"cx:"<<error<<endl;
			circle(display,Point(cx,50),10,Scalar(255,255,255),-1);
			line(display,Point(img.cols/2,0),Point(img.cols/2,img.rows),Scalar(0,255,0),LINE_8);
			RS -= error;
			LS +=error;
		}
		else{
				cout<<"failure"<<endl;
		}
		

		
		findContours(green, green_cont,RETR_EXTERNAL,CHAIN_APPROX_SIMPLE);
		
		drawContours(display,green_cont,-1,Scalar(0,255,0), 3);


		
		//double greenError = (img.cols/2.0)-cx;
		
		//double scaledown = 0.0009;

		
		Rect leftSlice(Point(0,0),Point(img.cols*(1/2.0),img.rows));
		Rect midSlice(Point(img.cols*(1/5.0),0),Point(img.cols*(4/5.0),img.rows));
		Rect rightSlice(Point(img.cols*(1/2.0),0),Point(img.cols,img.rows));
		Rect botHalf(Point(0,img.rows/2),Point(img.cols,img.rows));
		
		Mat leftImg = frame(leftSlice);
		Mat midImg = frame(midSlice);
		Mat rightImg = frame(rightSlice);
		Mat bottomHalf = frame(botHalf);
		
		Mat leftDisplay = display(leftSlice);
		Mat midDisplay = display(midSlice);
		Mat rightDisplay = display(rightSlice);
		
		for(int i = 0;i<(int)green_cont.size();i++){
			int cy = 0;
			//finds centroid of the green suqare so we can orient it so it is in the center of screen
			Moments green_moment = moments(green_cont[i]);
			if(contourArea(green_cont[i])!=0){
				cy = static_cast<int>(green_moment.m01/green_moment.m00);
			}  
			
			
			if(contourArea(green_cont[i])>5000&&cy>=250){
				Rect green_square = boundingRect(green_cont[i]);
				rectangle(display,green_square,Scalar(255,0,0),2);
				Point left,top, right,bottom;
				cout<<green_square.x<<" "<<green_square.y<<endl;
				top = Point(green_square.x,green_square.y);
				top.x += green_square.width/2;
				top.y -= 30;
				//~ stop_motors = 1;
				circle(display,top,10,Scalar(255,0,0),-1);
				int color = frame.at<unsigned char>(top);
				cout<<color<<endl;
				
				if(color==255){
					left.x = green_square.x;
					left.y = green_square.y;
					if(left.x<frame.cols/2){
						cout<<"left"<<counter<<endl;
						counter++;
					}
					else{
						counter+= 2;
						cout<<"right"<<counter<<endl;
						
						
					}
				}
				else{
					cout<<"false"<<endl;
				}
			}
		}

		
		if(counter!=0){
			for(int i = 0;i<(int)green_cont.size();i++){
				int cy = 0;
			//finds centroid of the green suqare so we can orient it so it is in the center of screen
			Moments green_moment = moments(green_cont[i]);
			if(contourArea(green_cont[i])!=0){
				cy = static_cast<int>(green_moment.m01/green_moment.m00);
			}  
				if(contourArea(green_cont[i])>5000&&cy>=200){
					Rect green_square = boundingRect(green_cont[i]);
					rectangle(display,green_square,Scalar(255,0,0),2);
					Point left,top, right,bottom;
					cout<<green_square.x<<" "<<green_square.y<<endl;
					top = Point(green_square.x+70,green_square.y);
					top.x += green_square.width/2;
					top.y -= 30;
					circle(display,top,2,Scalar(255,0,0),-1);
					Vec3b color = frame.at<Vec3b>(top.y,top.x);
					cout<<"red"<<static_cast<int>(color[2])<<"green"<<static_cast<int>(color[1])<<"blue"<<static_cast<int>(color[0])<<endl;
					
					if(color[0]==255){
						left.x = green_square.x;
						left.y = green_square.y;
						if(left.x<frame.cols/2){
							cout<<"left"<<counter<<endl;
							counter++;
						}
						else{
								counter+= 2;
								cout<<"right"<<counter<<endl;
						}
					}
					else{
						cout<<"false"<<endl;
					}
				}
			}
		}

		
		
			
		//printf("counter: %d", counter);
		printf("-------%d---------\n",counter); 
		
		
		if(stop_motors){
			sendSpeed(0, 0, 0, 0, 0);
		}
		else{
			sendSpeed((counter/2), LS, LS, RS, RS);
		}
		
		
		
		if((counter%2==0||counter%2==0)&&counter!=0){
			rx_length = 0;
			while(rx_length <= 0){
				rx_length = read(uart0_filestream,(void*)rx_buff,9);
				cout<<"waiting 4 green"<<endl;
			}
		}
		else if(counter == 6){

			cout<<"middle"<<endl;
			while(rx_length <= 0){
				rx_length = read(uart0_filestream,(void*)rx_buff,9);
			}
		}
		
		else{
			
		}

		counter = 0;
		
			
		//imshow("Left", leftDisplay);
		imshow("Mid", midDisplay);
		//imshow("Right", rightDisplay);	
		
        imshow("color+contour", display);
        imshow("threshold", frame);
        int key = waitKey(1);
        if (key == 'q') break;
        else if(key == ' ') stop_motors = !stop_motors;
        //else if(key == 'g') stop_motors = 0;
    }

	sendSpeed(0, 0, 0, 0, 0);
    destroyAllWindows();
    cam.stopVideo();
    cout<<"hi"<<endl;
    return 0;
}
