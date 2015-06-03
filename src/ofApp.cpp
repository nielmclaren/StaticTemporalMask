#include "ofApp.h"

void ofApp::setup(){
  ofSetLogLevel(OF_LOG_VERBOSE);

  frameCount = 128;
  frameWidth = 640;
  frameHeight = 480;
  screenWidth = ofGetWindowWidth();
  screenHeight = ofGetWindowHeight();

  videoGrabber.setVerbose(true);
  videoGrabber.setDeviceID(0);
  videoGrabber.initGrabber(frameWidth, frameHeight, false);

  drawImage.allocate(frameWidth, frameHeight, OF_IMAGE_COLOR);

  inputPixels = new unsigned char[frameCount * frameWidth * frameHeight * 3];
  inputPixelsStartIndex = -1;

  isShowingMask = false;
  isShowingInset = true;
  isMirrored = false;

  insetScale = 0.12;

  loadXmlSettings();

  maskIndex = 0;
  loadMask();
}

void ofApp::update(){
  videoGrabber.update();
  if (videoGrabber.isFrameNew()) {
    if (++inputPixelsStartIndex >= frameCount) inputPixelsStartIndex = 0;

    for (int x = 0; x < frameWidth; x++) {
      for (int y = 0; y < frameHeight; y++) {
        int i = y * frameWidth + x;
        int frameIndex = inputPixelsStartIndex + ofMap(maskImage.getPixels()[i], 0, 255, 0, frameCount - 1);
        if (frameIndex >= frameCount) frameIndex -= frameCount;

        for (int c = 0; c < 3; c++) {
          drawImage.getPixels()[i * 3 + c] =
            inputPixels[(frameIndex * frameWidth * frameHeight + i) * 3 + c];
        }
      }
    }
    drawImage.update();

    unsigned char* p = inputPixels + inputPixelsStartIndex * frameWidth * frameHeight * 3;
    memcpy(p, videoGrabber.getPixels(), frameWidth * frameHeight * 3);
  }
}

void ofApp::draw(){
  if (!maskImage.isAllocated() || !drawImage.isAllocated()) return;

  ofImage mainImage;
  ofImage insetImage;

  if (isShowingMask) {
    mainImage = maskImage;
    insetImage = drawImage;
  }
  else {
    mainImage = drawImage;
    insetImage = maskImage;
  }

  if (isMirrored) {
    mainImage.draw(screenWidth, 0, -screenWidth, screenHeight);
    if (isShowingInset) {
      drawInsetBackground();
      insetImage.draw(10 + screenWidth * insetScale, 10, -screenWidth * insetScale, screenHeight * insetScale);
    }
  }
  else {
    mainImage.draw(0, 0, screenWidth, screenHeight);
    if (isShowingInset) {
      drawInsetBackground();
      insetImage.draw(10, 10, screenWidth * insetScale, screenHeight * insetScale);
    }
  }
}

void ofApp::drawInsetBackground() {
  ofSetColor(128);
  ofRect(9, 9, screenWidth * insetScale + 2, screenHeight * insetScale + 2);
}

void ofApp::loadMask() {
  loadMask(maskPaths[maskIndex]);
}

void ofApp::loadMask(string filename) {
  maskImage.loadImage(filename);
  if (maskImage.type != OF_IMAGE_GRAYSCALE) {
      colorImg.setFromPixels(maskImage.getPixels(), maskImage.width, maskImage.height);
      grayscaleImg.allocate(maskImage.width, maskImage.height);
      grayscaleImg = colorImg;
      maskImage.setFromPixels(grayscaleImg.getPixels(), grayscaleImg.width, grayscaleImg.height, OF_IMAGE_GRAYSCALE);
  }
  maskImage.resize(frameWidth, frameHeight);
}

void ofApp::loadXmlSettings() {
  ofxXmlSettings settings;
  settings.loadFile("settings.xml");

  settings.pushTag("settings");
  settings.pushTag("masks");

  maskPaths.clear();

  int numMasks = settings.getNumTags("mask");
  for (int i = 0; i < numMasks; i++) {
    string path = settings.getAttribute("mask", "path", "", i);
    maskPaths.push_back(path);
  }

  settings.popTag(); // masks
  settings.popTag(); // settings
}

void ofApp::saveXmlSettings() {
  ofxXmlSettings settings;

  settings.addTag("settings");
  settings.pushTag("settings");

  settings.addTag("masks");
  settings.pushTag("masks");

  int numMasks = maskPaths.size();
  for (int i = 0; i < numMasks; i++) {
    settings.addTag("mask");
    settings.setAttribute("mask", "path", maskPaths[i], i);
  }

  settings.popTag(); // masks

  settings.popTag(); // settings

  settings.saveFile("settings.xml");
}

void ofApp::keyPressed(int key){
}

void ofApp::keyReleased(int key){
  switch (key) {
    case 'm':
      isMirrored = !isMirrored;
      break;
    case 't':
      isShowingMask = !isShowingMask;
      break;
    case 'i':
      isShowingInset = !isShowingInset;
      break;
    case 'r':
      ofSaveFrame();
      break;
    case 's':
      saveXmlSettings();
      break;
    case 'n':
      if (++maskIndex >= maskPaths.size()) maskIndex = 0;
      loadMask();
      break;
    case 'b':
      if (--maskIndex < 0) maskIndex = maskPaths.size() - 1;
      loadMask();
      break;
  }
}

void ofApp::mouseMoved(int x, int y ){

}

void ofApp::mouseDragged(int x, int y, int button){

}

void ofApp::mousePressed(int x, int y, int button){

}

void ofApp::mouseReleased(int x, int y, int button){

}

void ofApp::windowResized(int w, int h){

}

void ofApp::gotMessage(ofMessage msg){

}

void ofApp::dragEvent(ofDragInfo dragInfo) {
    for (int i = 0; i < dragInfo.files.size(); i++) {
        string filename = dragInfo.files[i];
        vector<string> tokens = ofSplitString(filename, ".");
        string extension = tokens[tokens.size() - 1];
        if (extension == "bmp" || extension == "jpg" || extension == "png") {
          loadMask(filename);
        }
    }
}
