// Grab.cpp
/*
	Note: Before getting started, Basler recommends reading the Programmer's Guide topic
	in the pylon C++ API documentation that gets installed with pylon.
	If you are upgrading to a higher major version of pylon, Basler also
	strongly recommends reading the Migration topic in the pylon C++ API documentation.

	This sample illustrates how to grab and process images using the CInstantCamera class.
	The images are grabbed and processed asynchronously, i.e.,
	while the application is processing a buffer, the acquisition of the next buffer is done
	in parallel.

	The CInstantCamera class uses a pool of buffers to retrieve image data
	from the camera device. Once a buffer is filled and ready,
	the buffer can be retrieved from the camera object for processing. The buffer
	and additional image data are collected in a grab result. The grab result is
	held by a smart pointer after retrieval. The buffer is automatically reused
	when explicitly released or when the smart pointer object is destroyed.
*/

// Include files to use the PYLON API.
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif
#include <opencv2/opencv.hpp>

// Namespace for using pylon objects.
using namespace Pylon;
// Namespace for using GenApi objects.
using namespace GenApi;
// Namespace for using cout.
using namespace std;
using namespace cv;

// Number of images to be grabbed.
static const uint32_t c_countOfImagesToGrab = 100;

int main(int argc, char* argv[])
{
	// The exit code of the sample application.
	int exitCode = 0;

	// Before using any pylon methods, the pylon runtime must be initialized. 
	PylonInitialize();
	Mat frame;
	try
	{
		// Create an instant camera object with the camera device found first.
		CInstantCamera camera(CTlFactory::GetInstance().CreateFirstDevice());

		INodeMap& nodemap = camera.GetNodeMap();

		// Close auto gain
		CFloatParameter gain(nodemap, "Gain");
		CEnumParameter gainAuto(nodemap, "GainAuto");
		if (gainAuto.CanSetValue("off")) {
			gainAuto.SetValue("off");
		}

		// Close auto balance white
		CEnumParameter bwAuto(nodemap, "BalanceWhiteAuto");
		if (bwAuto.CanSetValue("off")) {
			bwAuto.SetValue("off");
		}

		// Get parameter: exposure time
		CFloatParameter expoTime(nodemap, "ExposureTime");
		CEnumParameter expoAuto(nodemap, "ExposureAuto");
		if (expoAuto.CanSetValue("off")) {
			expoAuto.SetValue("off");
		}
		// set initial value and increase step
		float expoValue = 0.0f, expoStep = 5e4;

		// Print the model name of the camera.
		cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;

		// The parameter MaxNumBuffer can be used to control the count of buffers
		// allocated for grabbing. The default value of this parameter is 10.
		camera.MaxNumBuffer = 5;

		// Start the grabbing of c_countOfImagesToGrab images.
		// The camera device is parameterized with a default configuration which
		// sets up free-running continuous acquisition.
		camera.StartGrabbing(c_countOfImagesToGrab);

		// This smart pointer will receive the grab result data.
		CGrabResultPtr ptrGrabResult;
		
		/// new image that convert to cv::Mat
		CImageFormatConverter formatConverter;
		formatConverter.OutputPixelFormat = PixelType_BGR8packed;
		CPylonImage pylonImage;
		
		// Camera.StopGrabbing() is called automatically by the RetrieveResult() method
		// when c_countOfImagesToGrab images have been retrieved.
		char c;
		while (c = waitKey(1) && camera.IsGrabbing())
		{
			// Wait for an image and then retrieve it. A timeout of 5000 ms is used.
			camera.RetrieveResult(5000, ptrGrabResult, TimeoutHandling_ThrowException);

			// Image grabbed successfully?
			if (ptrGrabResult->GrabSucceeded())
			{
				// print current gain and expo
				cout << "gain value: " << gain.GetValue() << endl;
				std::cout << "current expo time: " << expoValue << endl;
				// Access the image data.
				cout << "SizeX: " << ptrGrabResult->GetWidth() << endl;
				cout << "SizeY: " << ptrGrabResult->GetHeight() << endl;
				const uint8_t *pImageBuffer = (uint8_t *)ptrGrabResult->GetBuffer();
				cout << "Gray value of first pixel: " << (uint32_t)pImageBuffer[0] << endl << endl;

//#ifdef PYLON_WIN_BUILD
				// Display the grabbed image.
				//Pylon::DisplayImage(1, ptrGrabResult);
//#endif
				///convert to cv::Mat
				formatConverter.Convert(pylonImage, ptrGrabResult);
				frame = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(), CV_8UC3, (uint8_t *)pylonImage.GetBuffer());
				/// show
				resize(frame, frame, Size(frame.cols / 2, frame.rows / 2));
				imshow("OpenCV Display Window", frame);
				// save image
				imwrite(".//results//"+to_string(expoValue) + ".bmp", frame);
				waitKey(100);
				// set expo value for next grab
				expoValue += expoStep;
				if (expoValue > 1e6) break;
				expoTime.SetValue(expoValue);
			}
			else
			{
				cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
			}
		}
	}
	catch (const GenericException &e)
	{
		// Error handling.
		cerr << "An exception occurred." << endl
			<< e.GetDescription() << endl;
		exitCode = 1;
	}

	// Comment the following two lines to disable waiting on exit.
	cerr << endl << "Press Enter to exit." << endl;
	while (cin.get() != '\n');

	// Releases all pylon resources. 
	PylonTerminate();

	return exitCode;
}
