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
    {"court", "txt"},
    {"player", "txt"},
    {"result", "avi"}
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

Point2f cNW(0,0), cNE(200,0), cSW(0,440), cSE(200,440);
vector<Point2f> courtCorners({cNW,cNE,cSW,cSE});
vector<Vec4f> courtLines({
    // hlines
    {0,0,200,0},
    {0,25,200,25},
    {0,155,200,155},
    {0,285,200,285},
    {0,415,200,415},
    {0,440,200,440},
    // vlines
    {0,0,0,440},
    {15,0,15,440},
    {100,0,100,155},
    {100,285,100,440},
    {185,0,185,440},
    {200,0,200,440}
});
Vec4f middleCourtLine({0,220,200,220});

void drawWhiteLine(Mat &mat, Vec4f l){
    Point2f p(l[0], l[1]), q(l[2], l[3]);
    line(mat, p, q, color.white, 1, CV_AA);
}

tuple<vector<Vec4f>,Vec4f> getTransformedCourtLines(Point2f NW, Point2f NE, Point2f SW, Point2f SE){
    
    vector<Point2f> intersectionCorners({NW,NE,SW,SE});
    Mat homoMat = getPerspectiveTransform(courtCorners, intersectionCorners);
    
    vector<Point2f> courtPoints1, courtPoints2;
    courtLines.push_back(middleCourtLine); 
    for(auto &l : courtLines){
        courtPoints1.push_back({l[0],l[1]});
        courtPoints2.push_back({l[2],l[3]});
    }
    courtLines.pop_back();

    vector<Point2f> transformedCourtPoints1, transformedCourtPoints2;
    perspectiveTransform(courtPoints1, transformedCourtPoints1, homoMat);
    perspectiveTransform(courtPoints2, transformedCourtPoints2, homoMat);
    
    vector<Vec4f> transformedCourtlines;
    for(size_t i=0; i<transformedCourtPoints1.size(); i++){
        transformedCourtlines.push_back({transformedCourtPoints1[i].x,transformedCourtPoints1[i].y,transformedCourtPoints2[i].x,transformedCourtPoints2[i].y});
    }

    Vec4f transformedMiddleCourtLine = transformedCourtlines.back();
    transformedCourtlines.pop_back();

    return tuple<vector<Vec4f>,Vec4f>(transformedCourtlines, transformedMiddleCourtLine);

}

void process(string input, map<string,int> options){
   
    VideoCapture inputVideo(getPath("input", input));
    assert(inputVideo.isOpened());

    int inputVideoWidth = inputVideo.get(CAP_PROP_FRAME_WIDTH);
    int inputVideoHeight = inputVideo.get(CAP_PROP_FRAME_HEIGHT);
    auto ex = VideoWriter::fourcc('D', 'I', 'V', 'X');
    int fps = 10;
    
    VideoWriter resultVideo;
    if(options["save"]){
        resultVideo.open(getPath("result",input), ex, fps, Size(inputVideoWidth, inputVideoHeight), true);
        assert(resultVideo.isOpened());
    }

    ifstream courtfile;
    courtfile.open(getPath("court",input));

    Point2f NW, NE, SW, SE;
    courtfile >> NW.x >> NW.y;
    courtfile >> NE.x >> NE.y;
    courtfile >> SW.x >> SW.y;
    courtfile >> SE.x >> SE.y;

    vector<Vec4f> transformedCourtlines;
    Vec4f transformedMiddleCourtLine;
    tie(transformedCourtlines, transformedMiddleCourtLine) = getTransformedCourtLines(NW,NE,SW,SE);
    Mat courtImg = Mat::zeros( Size(WIDTH,HEIGHT), CV_8U);
    for(auto &l : transformedCourtlines){
        drawWhiteLine(courtImg,l);
    }
    drawWhiteLine(courtImg,transformedMiddleCourtLine);

    Mat yellowImage = Mat::zeros( Size(WIDTH, HEIGHT), CV_8UC3);
    yellowImage = color.yellow;

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
            yellowImage.copyTo(frame, courtImg);
            rectangle(frame, boundingBoxN, color.blue, 2);
            circle(frame, feetN, 2, color.blue, 2);
            rectangle(frame, boundingBoxS, color.blue, 2);
            circle(frame, feetS, 2, color.blue, 2);
        }

        if(options["show"]){
            showImage(input, frame, options["fast"] ? 1 : 20);
        }

        if(options["save"]){
            resultVideo << frame;
        }
    }

    if(options["show"]){
        destroyWindow(input);
    }

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
        {"show", false},
        {"fast", false},
        {"save", false}
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