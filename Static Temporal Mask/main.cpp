#include <iostream>
#include <vector>
#include <time.h>

#include <opencv/cv.h>
#include <opencv/ml.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

using namespace std;

/**
 * @param frameBuffer An ordered list of buffered frames, ordered from oldest to newest.
 * @param temporalMask A grayscale image relative temporal offset at each pixel.
 */
void generateTimeDistortedImage(vector<IplImage*> frameBuffer, IplImage* temporalMask, IplImage* &result)
{
    IplImage* currentFrame;
    IplImage* currentMask;
    IplImage* remainingMask;
	
    unsigned long frameBufferSize = frameBuffer.size();
    if (frameBufferSize <= 0)
    {
        result = cvCreateImage(cvGetSize(temporalMask), IPL_DEPTH_8U, 3);
    }
    else
    {
        cvCopy(frameBuffer.at(frameBufferSize - 1), result);
		
        if (frameBufferSize > 1)
        {
            remainingMask = cvCloneImage(temporalMask);
            currentMask = cvCreateImage(cvGetSize(temporalMask), IPL_DEPTH_8U, 1);
			
            for (int i = 1; i < frameBufferSize; i++)
            {
                currentFrame = frameBuffer.at(i);
				
                int thresholdValue = (frameBufferSize - i - 1) * 255.0 / frameBufferSize;
                cvThreshold(remainingMask, currentMask, thresholdValue, 255, CV_THRESH_BINARY);
                cvThreshold(remainingMask, remainingMask, thresholdValue, 255, CV_THRESH_TOZERO_INV);
				
                cvCopy(currentFrame, result, currentMask);
            }
			
            cvReleaseImage(&currentMask);
            cvReleaseImage(&remainingMask);
        }
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
	
    CvCapture* capture = cvCreateCameraCapture(0);
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
		if (c == 27) break;
	}
	
    cvReleaseImage(&temporalMask);
	cvReleaseCapture(&capture);
	cvDestroyWindow(OUT_WINDOW_NAME);
	
	return 0;
}


