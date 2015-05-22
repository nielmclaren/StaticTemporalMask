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

  maskImage.loadImage("mask.bmp");
  assert(maskImage.width == frameWidth);
  assert(maskImage.height == frameHeight);

  drawImage.allocate(frameWidth, frameHeight, OF_IMAGE_COLOR);

  inputPixels = new unsigned char[frameCount * frameWidth * frameHeight * 3];
  inputPixelsStartIndex = -1;
}

void ofApp::update(){
  videoGrabber.update();
  if (videoGrabber.isFrameNew()) {
    if (++inputPixelsStartIndex >= frameCount) inputPixelsStartIndex = 0;

    unsigned char* p = inputPixels + inputPixelsStartIndex * frameWidth * frameHeight * 3;
    memcpy(
        p,
        videoGrabber.getPixels(),
        frameWidth * frameHeight * 3);

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
  }
}

void ofApp::draw(){
  if (drawImage.isAllocated()) {
    drawImage.draw(screenWidth, 0, -screenWidth, screenHeight);
  }
}

void ofApp::keyPressed(int key){

}

void ofApp::keyReleased(int key){
  switch (key) {
    case 'r':
      ofSaveFrame();
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

void ofApp::dragEvent(ofDragInfo dragInfo){

}
