#include "ofApp.h"

void ofApp::setup() {
  frameCount = 128;
  frameWidth = 640;
  frameHeight = 480;
  screenWidth = ofGetWindowWidth();
  screenHeight = ofGetWindowHeight();

  hudFont.loadFont("InputMono-Regular.ttf", 12);

  videoGrabber.setVerbose(true);
  videoGrabber.setDeviceID(0);
  videoGrabber.initGrabber(frameWidth, frameHeight, false);

  drawImage.allocate(frameWidth, frameHeight, OF_IMAGE_COLOR);

  inputPixels = new unsigned char[frameCount * frameWidth * frameHeight * 3];
  inputPixelsStartIndex = -1;

  isShowingHud = true;
  isShowingMask = false;
  isShowingInset = true;
  isMirrored = false;
  isAutoAdvancing = true;

  insetScale = 0.12;

  loadXmlSettings();

  maskIndex = floor(ofRandom(maskPaths.size()));
  loadMask();

  duration = 30000;
  prevLoadMaskTime = ofGetElapsedTimeMillis();

  isServerSetup = server.setup(9092);
  server.addListener(this);

  LocalAddressGrabber localAddressGrabber;
  ipAddress = localAddressGrabber.getIpAddress();
}

void ofApp::update() {
  unsigned long long now = ofGetElapsedTimeMillis();
  if (isAutoAdvancing && now > prevLoadMaskTime + duration) {
    loadNextMask();
  }

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

  for (int i = 0; i < commands.size(); i++) {
    handleCommand(commands[i]);
  }
  commands.clear();
}

void ofApp::draw() {
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
      insetImage.draw(
          screenWidth - 10, 10,
          -screenWidth * insetScale, screenHeight * insetScale);
    }
  }
  else {
    mainImage.draw(0, 0, screenWidth, screenHeight);
    if (isShowingInset) {
      drawInsetBackground();
      insetImage.draw(
          screenWidth * (1 - insetScale) - 10, 10,
          screenWidth * insetScale, screenHeight * insetScale);
    }
  }

  if (isShowingHud) {
    stringstream ss;
    ss << "      Frame rate: " << ofToString(ofGetFrameRate(), 2) << endl
      << "      WebSocket server: " << (isServerSetup ? ipAddress : "failed") << endl
      << " (H)  HUD: true" << endl
      << " (T)  Display: " << (isShowingMask ? "mask" : "output") << endl
      << " (M)  Mirror: " << (isMirrored ? "true" : "false") << endl
      << " (I)  Inset: " << (isShowingInset ? "true" : "false") << endl
      << " (A)  Auto-advance: " << (isAutoAdvancing ? "true" : "false") << endl
      << "([/]) Duration: " << duration << " ms" << endl
      << " (B)  Prev Frame" << endl
      << " (N)  Next Frame" << endl
      << " (R)  Save Frame" << endl
      << " (L)  Load Settings" << endl
      << " (S)  Save Settings" << endl;

    ss << endl;
    for (int i = 0; i < messages.size(); i++) {
      ss << messages[i] << endl;
    }

    hudFont.drawString(ss.str(), 14, 30);
    ss.str(std::string());
  }
}

void ofApp::drawInsetBackground() {
  ofSetColor(128);
  ofRect(screenWidth * (1 - insetScale) - 11, 9, screenWidth * insetScale + 2, screenHeight * insetScale + 2);
}

void ofApp::loadNextMask() {
  if (++maskIndex >= maskPaths.size()) maskIndex = 0;
  loadMask();
}

void ofApp::loadPrevMask() {
  if (--maskIndex < 0) maskIndex = maskPaths.size() - 1;
  loadMask();
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

  prevLoadMaskTime = ofGetElapsedTimeMillis();
}

void ofApp::loadXmlSettings() {
  ofxXmlSettings settings;
  settings.loadFile("settings.xml");

  isShowingHud = getBooleanAttribute(settings, "isShowingHud", isShowingHud);
  isMirrored = getBooleanAttribute(settings, "isMirrored", isMirrored);
  isShowingMask = getBooleanAttribute(settings, "isShowingMask", isShowingMask);
  isShowingInset = getBooleanAttribute(settings, "isShowingInset", isShowingInset);
  isAutoAdvancing = getBooleanAttribute(settings, "isAutoAdvancing", isAutoAdvancing);
  duration = settings.getAttribute("settings", "duration", duration);

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

  settings.setAttribute("settings", "isShowingHud", isShowingHud ? "true" : "false", 0);
  settings.setAttribute("settings", "isMirrored", isMirrored ? "true" : "false", 0);
  settings.setAttribute("settings", "isShowingMask", isShowingMask ? "true" : "false", 0);
  settings.setAttribute("settings", "isShowingInset", isShowingInset ? "true" : "false", 0);
  settings.setAttribute("settings", "isAutoAdvancing", isAutoAdvancing ? "true" : "false", 0);
  settings.setAttribute("settings", "duration", floor(duration), 0);

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

void ofApp::handleCommand(string command) {
  if (command == "toggleHud") {
    isShowingHud = !isShowingHud;
    saveXmlSettings();
  }
  else if (command == "toggleMirrored") {
    isMirrored = !isMirrored;
    saveXmlSettings();
  }
  else if (command == "toggleMask") {
    isShowingMask = !isShowingMask;
    saveXmlSettings();
  }
  else if (command == "toggleInset") {
    isShowingInset = !isShowingInset;
    saveXmlSettings();
  }
  else if (command == "toggleAutoAdvance") {
    isAutoAdvancing = !isAutoAdvancing;
    saveXmlSettings();
  }
  else if (command == "saveFrame") {
    ofSaveFrame();
  }
  else if (command == "prevMask") {
    loadPrevMask();
  }
  else if (command == "nextMask") {
    loadNextMask();
  }
  else if (command == "decDuration") {
    duration -= 2500;
    if (duration < 2500) duration = 2500;
    saveXmlSettings();
  }
  else if (command == "incDuration") {
    duration += 2500;
    saveXmlSettings();
  }
}

bool ofApp::getBooleanAttribute(ofxXmlSettings settings, string attributeName, bool defaultValue) {
  return settings.getAttribute("settings", attributeName, defaultValue ? "true" : "false", 0) != "false";
}

void ofApp::keyPressed(int key) {
}

void ofApp::keyReleased(int key) {
  switch (key) {
    case 'h': handleCommand("toggleHud"); break;
    case 'm': handleCommand("toggleMirrored"); break;
    case 't': handleCommand("toggleMask"); break;
    case 'i': handleCommand("toggleInset"); break;
    case 'a': handleCommand("toggleAutoAdvance"); break;
    case 'r': handleCommand("saveFrame"); break;
    case 'n': handleCommand("nextMask"); break;
    case 'b': handleCommand("prevMask"); break;
    case '[': handleCommand("decDuration"); break;
    case ']': handleCommand("incDuration"); break;
    case 'l':
      loadXmlSettings();
      break;
  }
}

void ofApp::mouseMoved(int x, int y) {

}

void ofApp::mouseDragged(int x, int y, int button) {

}

void ofApp::mousePressed(int x, int y, int button) {

}

void ofApp::mouseReleased(int x, int y, int button) {

}

void ofApp::windowResized(int w, int h) {

}

void ofApp::gotMessage(ofMessage msg) {

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

void ofApp::onConnect(ofxLibwebsockets::Event& args) {
  cout << "on connected" << endl;
}

void ofApp::onOpen(ofxLibwebsockets::Event& args) {
  cout << "new connection open" << endl;
  messages.push_back("New connection from " + args.conn.getClientIP() + ", " + args.conn.getClientName());
}

void ofApp::onClose(ofxLibwebsockets::Event& args) {
  cout << "on close" << endl;
  messages.push_back("Connection closed");
}

void ofApp::onIdle(ofxLibwebsockets::Event& args) {
  cout << "on idle" << endl;
}

void ofApp::onMessage(ofxLibwebsockets::Event& args) {
  cout << "got message '" << args.message << "'" << endl;

  // trace out string messages or JSON messages!
  if (!args.json.isNull()) {
    messages.push_back("New message: " + args.json.toStyledString() + " from " + args.conn.getClientName());
  }
  else {
    messages.push_back("New message: " + args.message + " from " + args.conn.getClientName());
  }

  if (messages.size() > NUM_MESSAGES) messages.erase(messages.begin());

  // echo server = send message right back!
  args.conn.send(args.message);

  commands.push_back(args.message);
}

void ofApp::onBroadcast(ofxLibwebsockets::Event& args) {
  cout << "got broadcast " << args.message << endl;
}
