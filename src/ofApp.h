#pragma once

#include "ofMain.h"

#include "ofxOpenCv.h"
#include "ofxMacamPs3Eye.h"
#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp{

  public:
    void setup();
    void update();
    void draw();

    void loadMask();
    void loadMask(string filename);
    void drawInsetBackground();
    void loadXmlSettings();
    void saveXmlSettings();

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

  private:
    std::vector<string> maskPaths;
    int maskIndex;

    int frameCount;
    int frameWidth;
    int frameHeight;
    int screenWidth;
    int screenHeight;

    //*
    ofxMacamPs3Eye videoGrabber;
    /*/
    ofVideoGrabber videoGrabber;
    //*/

    ofxCvColorImage colorImg;
    ofxCvGrayscaleImage grayscaleImg;

    unsigned char* inputPixels;
    int inputPixelsStartIndex;

    ofImage maskImage;
    ofImage drawImage;

    bool isShowingMask;
    bool isShowingInset;
    bool isMirrored;

    float insetScale;
};
