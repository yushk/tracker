#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/tracking.hpp>
#include <opencv2/tracking/tracker.hpp>
#include <iostream>
#include <utils.h>

using namespace cv;
using namespace std;

//Global variables
Rect2d box;
Rect2d oribox;
bool drawing_box = false;
bool gotBB = false;
bool tl = false;
bool rep = false;
bool fromfile=false;
string video;

// void readBB(char* file){
//   ifstream bb_file (file);
//   string line;
//   getline(bb_file,line);
//   istringstream linestream(line);
//   string x1,y1,x2,y2;
//   getline (linestream,x1, ',');
//   getline (linestream,y1, ',');
//   getline (linestream,x2, ',');
//   getline (linestream,y2, ',');
//   int x = atoi(x1.c_str());// = (int)file["bb_x"];
//   int y = atoi(y1.c_str());// = (int)file["bb_y"];
//   int w = atoi(x2.c_str())-x;// = (int)file["bb_w"];
//   int h = atoi(y2.c_str())-y;// = (int)file["bb_h"];
//   box = Rect(x,y,w,h);
// }
//bounding box mouse callback
void mouseHandler(int event, int x, int y, int flags, void *param){
  switch( event ){
  case CV_EVENT_MOUSEMOVE:
    if (drawing_box){
        box.width = x-box.x;
        box.height = y-box.y;
    }
    break;
  case CV_EVENT_LBUTTONDOWN:
    drawing_box = true;
    box = Rect( x, y, 0, 0 );
    break;
  case CV_EVENT_LBUTTONUP:
    drawing_box = false;
    if( box.width < 0 ){
        box.x += box.width;
        box.width *= -1;
    }
    if( box.height < 0 ){
        box.y += box.height;
        box.height *= -1;
    }
    gotBB = true;
    break;
  }
}
void print_help(char** argv){
  printf("use:\n     %s -p /path/parameters.yml\n",argv[0]);
  printf("-s    source video\n-b        bounding box file\n-tl  track and learn\n-r     repeat\n");
}
void read_options(int argc, char** argv,VideoCapture& capture,FileStorage &fs){
  for (int i=0;i<argc;i++){
      if (strcmp(argv[i],"-b")==0){
          if (argc>i){
            //   readBB(argv[i+1]);
              gotBB = true;
          }
          else
            print_help(argv);
      }
      if (strcmp(argv[i],"-s")==0){
          if (argc>i){
              video = string(argv[i+1]);
              capture.open(video);
              fromfile = true;
          }
          else
            print_help(argv);

      }
      if (strcmp(argv[i],"-p")==0){
          if (argc>i){
              fs.open(argv[i+1], FileStorage::READ);
          }
          else
            print_help(argv);
      }
      if (strcmp(argv[i],"-tl")==0){
          tl = true;
      }
      if (strcmp(argv[i],"-r")==0){
          rep = true;
      }
  }
}
void drawText(Mat & image){
    putText(image, "Hello OpenCV",
            Point(20, 50),
            FONT_HERSHEY_COMPLEX, 1, // font face and scale
            Scalar(255, 255, 255), // white
            1, LINE_AA); // line thickness and type
}

int main(int argc, char * argv[])
{
    cout << "Built with OpenCV " << CV_VERSION << endl;
    Mat image;
    VideoCapture capture;
    capture.open(0);
    FileStorage fs;
  //Read options
  read_options(argc,argv,capture,fs);
  //Init camera
  if (!capture.isOpened())
  {
	cout << "capture device failed to open!" << endl;
    return 1;
  }
  //Register mouse callback to draw the bounding box
  cvNamedWindow("TLD",CV_WINDOW_AUTOSIZE);
  cvSetMouseCallback( "TLD", mouseHandler, NULL );
    Mat frame;
  Mat last_gray;
  Mat first;
  if (fromfile){
      capture >> frame;
      cvtColor(frame, last_gray, CV_RGB2GRAY);
      frame.copyTo(first);
  }else{
      capture.set(CV_CAP_PROP_FRAME_WIDTH,680);
      capture.set(CV_CAP_PROP_FRAME_HEIGHT,480);
  }
 ///Initialization
GETBOUNDINGBOX:
  while(!gotBB)
  {
    if (!fromfile){
      capture >> frame;
    }
    else
      first.copyTo(frame);
    cvtColor(frame, last_gray, CV_RGB2GRAY);
    drawBox(frame,box);
    imshow("TLD", frame);
    if (cvWaitKey(33) == 'q')
	    return 0;
  }
  if (min(box.width,box.height)<(int)fs.getFirstTopLevelNode()["min_win"]){
      cout << "Bounding box too small, try again." << endl;
      gotBB = false;
      goto GETBOUNDINGBOX;
  }
  //Remove callback
  cvSetMouseCallback( "TLD", NULL, NULL );
  printf("Initial Bounding Box = x:%d y:%d h:%d w:%d\n",box.x,box.y,box.width,box.height);
// can change to BOOSTING, MIL, KCF (OpenCV 3.1), TLD, MEDIANFLOW, or GOTURN (OpenCV 3.2)
    //使用TrackerMIL跟踪
Ptr<Tracker> tracker= Tracker::create("MIL");
//Ptr<TrackerTLD> tracker= TrackerTLD::create();
//Ptr<TrackerKCF> tracker = TrackerKCF::create();
//Ptr<TrackerMedianFlow> tracker = TrackerMedianFlow::create();
//Ptr<TrackerBoosting> tracker= TrackerBoosting::create();
  //Output file
FILE  *bb_file = fopen("bounding_boxes.txt","w");
oribox = box;
tracker->init(frame,box);
  
REPEAT:
  while(capture.read(frame)){
    //get frame
    // cvtColor(frame, current_gray, CV_RGB2GRAY);
    // //Process Frame
    // tld.processFrame(last_gray,current_gray,pts1,pts2,pbox,status,tl,bb_file);
    // //Draw Points
    // if (status){
    //   drawPoints(frame,pts1);
    //   drawPoints(frame,pts2,Scalar(0,255,0));
    //   drawBox(frame,pbox);
    //   detections++;
    // }
    //Display
    tracker->update(frame,box);
    drawBox(frame,box);
    drawBox(frame,oribox);
    
    drawText(frame);
    imshow("TLD", frame);
    //swap points and images
    // swap(last_gray,current_gray);
    // pts1.clear();
    // pts2.clear();
    // frames++;
    // printf("Detection rate: %d/%d\n",detections,frames);
    if (cvWaitKey(33) == 'q')
      break;
  }
  if (rep){
    rep = false;
    tl = false;
    fclose(bb_file);
    bb_file = fopen("final_detector.txt","w");
    //capture.set(CV_CAP_PROP_POS_AVI_RATIO,0);
    capture.release();
    capture.open(video);
    goto REPEAT;
  }
  fclose(bb_file);
  return 0;

    // if(capture.isOpened()){
    //     cout << "Capture is opened" << endl;
    //     for(;;)
    //     {
    //         capture >> image;
    //         if(image.empty())
    //             break;
    //         drawText(image);
    //         imshow("Sample", image);
    //         if(waitKey(10) >= 0)
    //             break;
    //     }
    // }else{
    //     cout << "No capture" << endl;
    //     image = Mat::zeros(480, 640, CV_8UC1);
    //     drawText(image);
    //     imshow("Sample", image);
    //     waitKey(0);
    // }
    // return 0;
}






