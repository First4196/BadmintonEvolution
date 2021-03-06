#include <iostream>
#include <fstream>
#include <random>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

struct Color{
    Scalar blue = Scalar(255,0,0);
    Scalar green = Scalar(0,255,0);
    Scalar red = Scalar(0,0,255);
    Scalar yellow = Scalar(0,255,255);
    Scalar white = Scalar(255,255,255);
}color;

map<string, string> fileExt{
    {"input", "avi"},
    {"player", "txt"}
};

string getPath(string what, string input){
    assert(fileExt.find(what)!=fileExt.end());
    return "data/" + what +"/" + what + input + "." + fileExt[what];
}

int WIDTH = 512;
int HEIGHT = 512;

void showImage(string windowName, const Mat &image, int wait = 0){
    namedWindow(windowName, WINDOW_AUTOSIZE );
    imshow(windowName, image);
    waitKey(wait);
}

void process(string input, map<string,int> options){
   
    VideoCapture inputVideo(getPath("input", input));
    assert(inputVideo.isOpened());

    ifstream playerfile;
    playerfile.open(getPath("player",input));

    Mat frame;
    int tagFrameNumber, tag;
    Rect boundingBoxN, boundingBoxS;
    Point2f feetN, feetS;

    for(int frameNumber=0;; frameNumber++){
        inputVideo >> frame;
        if(frame.empty()){
            break;
        }
        playerfile >> tagFrameNumber >> tag;
        playerfile >> boundingBoxN.x >> boundingBoxN.y;
        playerfile >> boundingBoxN.width >> boundingBoxN.height;
        playerfile >> feetN.x >> feetN.y;
        playerfile >> boundingBoxS.x >> boundingBoxS.y;
        playerfile >> boundingBoxS.width >> boundingBoxS.height;
        playerfile >> feetS.x >> feetS.y;
                
        assert(frameNumber==tagFrameNumber);
        if(tag){
            rectangle(frame, boundingBoxN, color.blue, 2);
            circle(frame, feetN, 2, color.blue, 2);
            rectangle(frame, boundingBoxS, color.blue, 2);
            circle(frame, feetS, 2, color.blue, 2);
        }
        showImage(input, frame, options["fast"] ? 1 : 20);
    }
    destroyWindow(input);

}

int main(int argc, char** argv ){

    srand(time(NULL));

    vector<string> optionArgs;
    vector<string> inputArgs;
    for(int i=1; i<argc; i++){
        string arg(argv[i]);
        if(arg[0] == '-'){
            optionArgs.push_back(arg.substr(1));
        }
        else{
            inputArgs.push_back(arg);
        }
    }

    map<string,int> options{
        {"fast", false}
    };

    for(string option : optionArgs){
        if(options.find(option)!=options.end()){
            options[option] = true;
        }
    }

    for(string input : inputArgs){
        process(input, options);
    }

    return 0;
}