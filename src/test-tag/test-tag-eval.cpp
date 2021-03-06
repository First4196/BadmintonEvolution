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
    {"tag", "txt"},
    {"tag-truth", "txt"}
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
    int numberOfFrames = inputVideo.get(CV_CAP_PROP_FRAME_COUNT);

    ifstream tagfile;
    tagfile.open(getPath("tag",input));

    ifstream tagtfile;
    tagtfile.open(getPath("tag-truth",input));
        
    int frameNumber, tag;
    int frameNumbert, tagt;
    int truePositive=0, falsePositive=0;
    int trueNegative=0, falseNegative=0;

    numberOfFrames = min(numberOfFrames, 2500);

    for(int i=0; i<numberOfFrames; i++){
        tagfile >> frameNumber >> tag;
        tagtfile >> frameNumbert >> tagt;
        assert(frameNumber==frameNumbert);
        if(tag){
            if(tagt){
                truePositive++;
            }
            else{
                falsePositive++;
            }
        }
        else{
            if(tagt){
                falseNegative++;
            }
            else{
                trueNegative++;
            }
        }
    }

    cout << "Number Of Frames : " << numberOfFrames << endl;
    cout << "True  Positive   : " << truePositive << endl;
    cout << "False Positive   : " << falsePositive << endl;
    cout << "True  Negative   : " << trueNegative << endl;
    cout << "False Negative   : " << falseNegative << endl;

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