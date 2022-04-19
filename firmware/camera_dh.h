#pragma once
#include "GxIAPI.h" 
#include<iostream> 
 
class CameraDh
{
public:
	CameraDh();
	~CameraDh();

	bool openCamera();

	bool closeCamera(); 
	
	bool warmupCamera();

	bool captureSingleImage(char* buffer);

	bool captureSingleExposureImage(float exposure,char* buffer);

	bool captureRawPhaseImages(char* buffer);

	bool captureRawTest(int num,char* buffer);

	bool switchToSingleShotMode();

	bool switchToScanMode();

	bool getExposure(float &value);

	bool setExposure(float value);

	bool setScanExposure(float value);

	/********************************************************************/
	//gpu parallel
	bool captureFrame03ToGpu();
	
	bool captureFrame04ToGpu();

	bool captureFrame03RepetitionToGpu(int repetition_count);

private:

    GX_DEV_HANDLE hDevice_;

	bool camera_opened_state_;

	int image_shift_num_;

	//条纹扫描时，相机曝光值
	float scan_camera_exposure_;
};

