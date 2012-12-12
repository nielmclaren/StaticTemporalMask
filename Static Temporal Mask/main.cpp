#include <iostream>
#include <vector>
#include <time.h>

#include <opencv/cv.h>
#include <opencv/ml.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

using namespace std;
using namespace cv;

/**
 * @param frameBuffer An ordered list of buffered frames, ordered from oldest to newest.
 * @param mask A grayscale image relative temporal offset at each pixel.
 */
void generateTimeDistortedImage(vector<IplImage*> frameBuffer, IplImage* mask, IplImage* &result) {
	unsigned long bufferSize = frameBuffer.size();
	
	if (bufferSize < 2) return;
	
	int numRows = result->height;
	int numCols = result->width;
	int numChannels = result->nChannels;
	
	unsigned char *maskData = reinterpret_cast<unsigned char *>(mask->imageData);
	unsigned char *resultData = reinterpret_cast<unsigned char *>(result->imageData);
	
	for (int r = 1; r < numRows; r++) {
		for (int c = 0; c < numCols; c++) {
			float fraction = 1 - maskData[c] / 256.0;
			int index = (bufferSize - 1) * fraction;
			float remainder = (bufferSize - 1) * fraction - index;
			
			for (int ch = 0; ch < numChannels; ch++) {
				IplImage* frame = frameBuffer[index];
				unsigned char *frameData = reinterpret_cast<unsigned char *>(frame->imageData);
				frameData += r * frame->widthStep;
				
				IplImage* nextFrame = frameBuffer[index + 1];
				unsigned char *nextFrameData = reinterpret_cast<unsigned char *>(nextFrame->imageData);
				nextFrameData += r * nextFrame->widthStep;
				
				resultData[c * numChannels + ch] =
					frameData[c * numChannels + ch] * (1 - remainder)
					+ nextFrameData[c * numChannels + ch] * remainder;
			}
		}
		
		maskData += mask->widthStep;
		resultData += result->widthStep;
	}
}

int main()
{
    const unsigned int FRAME_BUFFER_SIZE = 40;
    const char* OUT_WINDOW_NAME = "StaticTemporalMask";
	cvNamedWindow(OUT_WINDOW_NAME, CV_WINDOW_AUTOSIZE);
	
	IplImage* frame;
	IplImage* temporalMaskFile;
	IplImage* temporalMask;
	IplImage* timeDistortedImage;
	vector<IplImage*> frameBuffer;
	
    CvCapture* capture = cvCaptureFromCAM(0);
    //CvCapture* capture = cvCreateFileCapture("clock.avi");
	
    temporalMaskFile = cvLoadImage("temporal_mask01.jpg");
    temporalMask = cvCreateImage(cvGetSize(temporalMaskFile), IPL_DEPTH_8U, 1);
    cvCvtColor(temporalMaskFile, temporalMask, CV_BGR2GRAY);
    cvReleaseImage(&temporalMaskFile);
	
    timeDistortedImage = cvCreateImage(cvGetSize(temporalMask), IPL_DEPTH_8U, 3);
	
	while (1)
    {
		frame = cvQueryFrame(capture);
		if (!frame) break;
		
        cvConvertImage(frame, frame, CV_CVTIMG_FLIP);
        cvFlip(frame, frame, -1);
		frameBuffer.push_back(cvCloneImage(frame));
		if (frameBuffer.size() > FRAME_BUFFER_SIZE)
		{
		    cvReleaseImage(&frameBuffer.front());
		    frameBuffer.erase(frameBuffer.begin());
		}
		
		generateTimeDistortedImage(frameBuffer, temporalMask, timeDistortedImage);
		
        cvShowImage(OUT_WINDOW_NAME, timeDistortedImage);
		
		// wait for ESC
		char c = cvWaitKey(33);
		if (c == 27) break;	}
	
	cvReleaseImage(&temporalMask);
	cvReleaseCapture(&capture);
	cvDestroyWindow(OUT_WINDOW_NAME);
	
	return 0;
}

