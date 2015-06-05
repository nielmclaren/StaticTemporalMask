#pragma once

#include "ofMain.h"

#include "ofxLibwebsockets.h"
#include "ofxMacamPs3Eye.h"
#include "ofxOpenCv.h"
#include "ofxXmlSettings.h"

#include "LocalAddressGrabber.h"

#define NUM_MESSAGES 20 // how many past messages we want to keep

class ofApp : public ofBaseApp{

  public:
    void setup();
    void update();
    void draw();

    void loadMask();
    void loadNextMask();
    void loadPrevMask();
    void loadMask(string filename);
    void drawInsetBackground();
    void loadXmlSettings();
    void saveXmlSettings();
    void handleCommand(string message);
    bool getBooleanAttribute(ofxXmlSettings settings, string attributeName, bool defaultValue);

    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);

    // Websocket methods.
    void onConnect(ofxLibwebsockets::Event& args);
    void onOpen(ofxLibwebsockets::Event& args);
    void onClose(ofxLibwebsockets::Event& args);
    void onIdle(ofxLibwebsockets::Event& args);
    void onMessage(ofxLibwebsockets::Event& args);
    void onBroadcast(ofxLibwebsockets::Event& args);

  private:
    std::vector<string> maskPaths;
    int maskIndex;

    int frameCount;
    int frameWidth;
    int frameHeight;
    int screenWidth;
    int screenHeight;

    ofTrueTypeFont hudFont;

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

    bool isShowingHud;
    bool isShowingMask;
    bool isShowingInset;
    bool isMirrored;
    bool isAutoAdvancing;

    float insetScale;

    double duration;
    unsigned long long prevLoadMaskTime;

    ofxLibwebsockets::Server server;

    bool isServerSetup;

    vector<string> commands;
    vector<string> messages;

    string ipAddress;
};
