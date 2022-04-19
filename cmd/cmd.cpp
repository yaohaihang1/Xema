﻿#include "../sdk/open_cam3d.h"
#include "../firmware/system_config_settings.h"
#include "../firmware/protocol.h"
#include "../firmware/version.h"
#include "../test/solution.h"
#include "opencv2/opencv.hpp"
#include <windows.h>
#include <assert.h>
#include <fstream>
#include <string.h>
#include "getopt.h" 
#include "../test/LookupTableFunction.h"


const char* help_info =
"Examples:\n\
\n\
1.Get temperature:\n\
open_cam3d.exe --get-temperature --ip 192.168.x.x\n\
\n\
2.Get brightness image:\n\
open_cam3d.exe --get-brightness --ip 192.168.x.x --path ./brightness.bmp\n\
\n\
3.Get point cloud:\n\
open_cam3d.exe --get-pointcloud --ip 192.168.x.x --path ./point_cloud.xyz\n\
\n\
4.Get Frame 01:\n\
open_cam3d.exe --get-frame-01 --ip 192.168.x.x --path ./frame_01\n\
\n\
5.Get Frame 03:\n\
open_cam3d.exe --get-frame-03 --ip 192.168.x.x --path ./frame_03\n\
\n\
6.Get Frame Hdr:\n\
open_cam3d.exe --get-frame-hdr --ip 192.168.x.x --path ./frame_hdr\n\
\n\
7.Get calibration parameters: \n\
open_cam3d.exe --get-calib-param --ip 192.168.x.x --path ./param.txt\n\
\n\
8.Set calibration parameters: \n\
open_cam3d.exe--set-calib-param --ip 192.168.x.x --path ./param.txt\n\
\n\
9.Get raw images (Mode 01): \n\
open_cam3d.exe --get-raw-01 --ip 192.168.x.x --path ./raw01_image_dir\n\
\n\
10.Get raw images (Mode 02): \n\
open_cam3d.exe --get-raw-02 --ip 192.168.x.x --path ./raw02_image_dir\n\
\n\
11.Get raw images (Mode 03): \n\
open_cam3d.exe --get-raw-03 --ip 192.168.x.x --path ./raw03_image_dir\n\
\n\
12.Enable checkerboard: \n\
open_cam3d.exe --enable-checkerboard --ip 192.168.x.x\n\
\n\
13.Disable checkerboard: \n\
open_cam3d.exe --disable-checkerboard --ip 192.168.x.x\n\
\n\
14.Get Repetition Frame 03: \n\
open_cam3d.exe --get-repetition-frame-03 --count 6 --ip 192.168.x.x --path ./frame03_repetition\n\
15.Load pattern data: \n\
open_cam3d.exe --load-pattern-data --ip 192.168.x.x\n\
\n\
16.Program pattern data: \n\
open_cam3d.exe --program-pattern-data --ip 192.168.x.x\n\
\n\
17.Get network bandwidth: \n\
open_cam3d.exe --get-network-bandwidth --ip 192.168.x.x\n\
\n\
18.Get firmware version: \n\
open_cam3d.exe --get-firmware-version --ip 192.168.x.x\n\
\n\
19.Test camera calibration parameters: \n\
open_cam3d.exe --test-calib-param --use plane --ip 192.168.x.x --path ./capture\n\
\n\
20.Set calibration lookTable: \n\
open_cam3d.exe --set-calib-looktable --ip 192.168.x.x --path ./param.txt\n\
21.Set Generate Brightness Param: \n\
open_cam3d.exe --set-generate-brigntness-param --ip 192.168.x.x --model 1 --exposure 12000\n\
22.Get Generate Brightness Param: \n\
open_cam3d.exe --get-generate-brigntness-param --ip 192.168.x.x\n\
23.Set Camera Exposure Param: \n\
open_cam3d.exe --set-camera-exposure-param --ip 192.168.x.x --exposure 12000\n\
24.Get Camera Exposure Param: \n\
open_cam3d.exe --get-camera-exposure-param --ip 192.168.x.x\n\
";

void help_with_version(const char* help);
bool depthTransformPointcloud(cv::Mat depth_map, cv::Mat& point_cloud_map);
int get_frame_01(const char* ip, const char* frame_path);
int get_frame_03(const char* ip, const char* frame_path);
int get_repetition_frame_03(const char* ip,int count, const char* frame_path);
int get_frame_hdr(const char* ip, const char* frame_path);
void save_frame(float* depth_buffer, unsigned char* bright_buffer, const char* frame_path);
void save_images(const char* raw_image_dir, unsigned char* buffer, int image_size, int image_num);
void save_point_cloud(float* point_cloud_buffer, const char* pointcloud_path);
void save_color_point_cloud(float* point_cloud_buffer, unsigned char* brightness_buffer, const char* pointcloud_path);
int on_dropped(void* param);
int get_brightness(const char* ip, const char* image_path);
int get_pointcloud(const char* ip, const char* pointcloud_path);
int get_raw_01(const char* ip, const char* raw_image_dir);
int get_raw_02(const char* ip, const char* raw_image_dir);
int get_raw_03(const char* ip, const char* raw_image_dir);
int get_calib_param(const char* ip, const char* calib_param_path);
int set_calib_param(const char* ip, const char* calib_param_path);
int set_generate_brightness_param(const char* ip, int model, float exposure);
int get_generate_brightness_param(const char* ip, int &model, float &exposure);
int set_camera_exposure_param(const char* ip, float exposure);
int get_camera_exposure_param(const char* ip, float &exposure);
int set_calib_looktable(const char* ip, const char* calib_param_path);
int test_calib_param(const char* ip, const char* result_path);
int get_temperature(const char* ip);
int enable_checkerboard(const char* ip);
int disable_checkerboard(const char* ip);
int load_pattern_data(const char* ip);
int program_pattern_data(const char* ip);
int get_network_bandwidth(const char* ip);
int get_firmware_version(const char* ip);

extern int optind, opterr, optopt;
extern char* optarg;

enum opt_set
{
	IP,
	PATH,
	COUNT,
	GET_TEMPERATURE,
	GET_CALIB_PARAM,
	SET_CALIB_PARAM,
	SET_CALIB_LOOKTABLE,
	TEST_CALIB_PARAM,
	USE,
	GET_RAW_01,
	GET_RAW_02,
	GET_RAW_03,
	GET_POINTCLOUD,
	GET_FRAME_01,
	GET_FRAME_03,
	GET_REPETITION_FRAME_03,
	GET_FRAME_HDR,
	GET_BRIGHTNESS,
	HELP,
	ENABLE_CHECKER_BOARD,
	DISABLE_CHECKER_BOARD,
	LOAD_PATTERN_DATA,
	PROGRAM_PATTERN_DATA,
	GET_NETWORK_BANDWIDTH,
	GET_FIRMWARE_VERSION,
	SET_GENERATE_BRIGHTNESS,
	GET_GENERATE_BRIGHTNESS,
	MODEL,
	EXPOSURE,
	SET_CAMERA_EXPOSURE,
	GET_CAMERA_EXPOSURE,
};

static struct option long_options[] =
{
	{"ip",required_argument,NULL,IP},
	{"path", required_argument, NULL, PATH},
	{"count", required_argument, NULL, COUNT},
	{"use", required_argument, NULL, USE},
	{"model", required_argument, NULL, MODEL},
	{"exposure", required_argument, NULL, EXPOSURE},
	{"get-temperature",no_argument,NULL,GET_TEMPERATURE},
	{"get-calib-param",no_argument,NULL,GET_CALIB_PARAM},
	{"set-calib-param",no_argument,NULL,SET_CALIB_PARAM},
	{"set-calib-looktable",no_argument,NULL,SET_CALIB_LOOKTABLE},
	{"test-calib-param",no_argument,NULL,TEST_CALIB_PARAM},
	{"get-raw-01",no_argument,NULL,GET_RAW_01},
	{"get-raw-02",no_argument,NULL,GET_RAW_02},
	{"get-raw-03",no_argument,NULL,GET_RAW_03},
	{"get-pointcloud",no_argument,NULL,GET_POINTCLOUD},
	{"get-frame-01",no_argument,NULL,GET_FRAME_01},
	{"get-frame-03",no_argument,NULL,GET_FRAME_03},
	{"get-repetition-frame-03",no_argument,NULL,GET_REPETITION_FRAME_03},
	{"get-frame-hdr",no_argument,NULL,GET_FRAME_HDR},
	{"get-brightness",no_argument,NULL,GET_BRIGHTNESS},
	{"help",no_argument,NULL,HELP},
	{"enable-checkerboard",no_argument,NULL,ENABLE_CHECKER_BOARD},
	{"disable-checkerboard",no_argument,NULL,DISABLE_CHECKER_BOARD},
	{"load-pattern-data",no_argument,NULL,LOAD_PATTERN_DATA},
	{"program-pattern-data",no_argument,NULL,PROGRAM_PATTERN_DATA},
	{"get-network-bandwidth",no_argument,NULL,GET_NETWORK_BANDWIDTH},
	{"get-firmware-version",no_argument,NULL,GET_FIRMWARE_VERSION},
	{"set-generate-brightness-param",no_argument,NULL,SET_GENERATE_BRIGHTNESS},
	{"get-generate-brightness-param",no_argument,NULL,GET_GENERATE_BRIGHTNESS},
	{"set-camera-exposure-param",no_argument,NULL,SET_CAMERA_EXPOSURE},
	{"get-camera-exposure-param",no_argument,NULL,GET_CAMERA_EXPOSURE},
};


const char* camera_id;
const char* path;
const char* repetition_count;
const char* use_command;
const char* c_model;
const char* c_exposure;
int command = HELP;

struct CameraCalibParam calibration_param_;


int main(int argc, char* argv[])
{
	int c = 0;

	while (EOF != (c = getopt_long(argc, argv, "i:h", long_options, NULL)))
	{
		switch (c)
		{
		case IP:
			camera_id = optarg;
			break;
		case PATH:
			path = optarg;
			break;
		case COUNT:
			repetition_count = optarg;
			break;
		case USE:
			use_command = optarg;
			break;
		case MODEL:
			c_model = optarg;
			break;
		case EXPOSURE:
			c_exposure = optarg;
			break;
		case '?':
			printf("unknow option:%c\n", optopt);
			break;
		default:
			command = c;
			break;
		}
	}

	switch (command)
	{
	case HELP:
		help_with_version(help_info);
		break;
	case GET_TEMPERATURE:
		get_temperature(camera_id);
		break;
	case GET_CALIB_PARAM:
		get_calib_param(camera_id, path);
		break;
	case SET_CALIB_PARAM:
		set_calib_param(camera_id, path);
		break;
	case SET_CALIB_LOOKTABLE:
		set_calib_looktable(camera_id, path);
		break;
	case TEST_CALIB_PARAM:
		test_calib_param(camera_id, path);
		break;
	case GET_RAW_01:
		get_raw_01(camera_id, path);
		break;
	case GET_RAW_02:
		get_raw_02(camera_id, path);
		break;
	case GET_RAW_03:
		get_raw_03(camera_id, path);
		break;
	case GET_FRAME_01:
		get_frame_01(camera_id, path);
		break;
	case GET_FRAME_03:
		get_frame_03(camera_id, path);
		break;
	case GET_REPETITION_FRAME_03:
	{ 
		int num = std::atoi(repetition_count);
		get_repetition_frame_03(camera_id, num, path);
	}
		break;
	case GET_FRAME_HDR:
		get_frame_hdr(camera_id, path);
		break;
	case GET_POINTCLOUD:
		get_pointcloud(camera_id, path);
		break;
	case GET_BRIGHTNESS:
		get_brightness(camera_id, path);
		break;
	case ENABLE_CHECKER_BOARD:
		enable_checkerboard(camera_id);
		break;
	case DISABLE_CHECKER_BOARD:
		disable_checkerboard(camera_id);
		break;
	case LOAD_PATTERN_DATA:
		load_pattern_data(camera_id);
		break;
	case PROGRAM_PATTERN_DATA:
		program_pattern_data(camera_id);
		break;
	case GET_NETWORK_BANDWIDTH:
		get_network_bandwidth(camera_id);
		break;
	case GET_FIRMWARE_VERSION:
		get_firmware_version(camera_id);
		break;
	case SET_GENERATE_BRIGHTNESS:
	{
		int model = std::atoi(c_model);
		float exposure = std::atof(c_exposure);
		set_generate_brightness_param(camera_id, model, exposure);
	} 
		break;
	case GET_GENERATE_BRIGHTNESS:
	{
		int model = 0;
		float exposure = 0;
		get_generate_brightness_param(camera_id, model, exposure);
	}
	break;
	case SET_CAMERA_EXPOSURE:
	{
		float exposure = std::atof(c_exposure);
		set_camera_exposure_param(camera_id, exposure);
	}
	break;
	case GET_CAMERA_EXPOSURE:
	{
		float exposure = 0;
		get_camera_exposure_param(camera_id, exposure);
	}
	break;
	default:
		break;
	}

	return 0;
}

void help_with_version(const char* help)
{
	char info[100 * 1024] = {'\0'};
	char version[] = _VERSION_;
	char enter[] = "\n";

	strcpy_s(info, sizeof(enter), enter);
	strcat_s(info, sizeof(info), version);
	strcat_s(info, sizeof(info), enter);
	strcat_s(info, sizeof(info), enter);
	strcat_s(info, sizeof(info), help);

	printf_s(info);
}

int on_dropped(void* param)
{
	std::cout << "Network dropped!" << std::endl;
	return 0;
}

bool depthTransformPointcloud(cv::Mat depth_map, cv::Mat& point_cloud_map)
{
	if (depth_map.empty())
	{
		return false;
	}

	double camera_fx = calibration_param_.camera_intrinsic[0];
	double camera_fy = calibration_param_.camera_intrinsic[4];

	double camera_cx = calibration_param_.camera_intrinsic[2];
	double camera_cy = calibration_param_.camera_intrinsic[5];
  
	int nr = depth_map.rows;
	int nc = depth_map.cols;


	cv::Mat points_map(nr, nc, CV_32FC3, cv::Scalar(0, 0, 0));


	for (int r = 0; r < nr; r++)
	{

		float* ptr_d = depth_map.ptr<float>(r);
		cv::Vec3f* ptr_p = points_map.ptr<cv::Vec3f>(r);

		for (int c = 0; c < nc; c++)
		{

			if (ptr_d[c] > 0)
			{

				cv::Point3f p;
				p.z = ptr_d[c];

				p.x = (c - camera_cx) * p.z / camera_fx;
				p.y = (r - camera_cy) * p.z / camera_fy;


				ptr_p[c][0] = p.x;
				ptr_p[c][1] = p.y;
				ptr_p[c][2] = p.z;
			}


		}


	}

	point_cloud_map = points_map.clone();

	return true;
}

void save_frame(float* depth_buffer, unsigned char* bright_buffer, const char* frame_path)
{
	std::string folderPath = frame_path;

	cv::Mat depth_map(1200, 1920, CV_32F, depth_buffer);
	cv::Mat bright_map(1200, 1920, CV_8U, bright_buffer);


	std::string depth_path = folderPath + ".tiff";
	cv::imwrite(depth_path, depth_map);
	std::cout << "save depth: " << depth_path << "\n";

	std::string bright_path = folderPath + ".bmp";
	cv::imwrite(bright_path, bright_map);
	std::cout << "save brightness: " << bright_path << "\n";

	cv::Mat point_cloud_map;
	depthTransformPointcloud(depth_map, point_cloud_map);


	std::string pointcloud_path = folderPath + ".xyz";

	save_color_point_cloud((float*)point_cloud_map.data, (unsigned char*)bright_map.data, pointcloud_path.c_str());

	std::cout << "save point cloud: " << pointcloud_path << "\n";

	//struct CameraCalibParam calibration_param;
	//DfGetCalibrationParam(calibration_param);

}

void save_images(const char* raw_image_dir, unsigned char* buffer, int image_size, int image_num)
{
	std::string folderPath = raw_image_dir;
	std::string mkdir_cmd = std::string("mkdir ") + folderPath;
	system(mkdir_cmd.c_str());

	for (int i = 0; i < image_num; i++)
	{
		std::stringstream ss;
		cv::Mat image(1200, 1920, CV_8UC1, buffer + (long)(image_size * i));
		ss << std::setw(2) << std::setfill('0') << i;
		std::string filename = folderPath + "/phase" + ss.str() + ".bmp";
		cv::imwrite(filename, image);
	}
}

void save_color_point_cloud(float* point_cloud_buffer, unsigned char* brightness_buffer, const char* pointcloud_path)
{
	std::ofstream ofile;
	ofile.open(pointcloud_path);
	for (int i = 0; i < 1920 * 1200; i++)
	{
		if (point_cloud_buffer[i * 3 + 2] > 0.01)
			ofile << point_cloud_buffer[i * 3] << " " << point_cloud_buffer[i * 3 + 1] << " " << point_cloud_buffer[i * 3 + 2] << " "
			<< (int)brightness_buffer[i] << " " << (int)brightness_buffer[i] << " " << (int)brightness_buffer[i] << std::endl;
	}
	ofile.close();
}


void save_point_cloud(float* point_cloud_buffer, const char* pointcloud_path)
{
	std::ofstream ofile;
	ofile.open(pointcloud_path);
	for (int i = 0; i < 1920 * 1200; i++)
	{
		if (point_cloud_buffer[i * 3 + 2] > 0.01)
			ofile << point_cloud_buffer[i * 3] << " " << point_cloud_buffer[i * 3 + 1] << " " << point_cloud_buffer[i * 3 + 2] << std::endl;
	}
	ofile.close();
}

int get_brightness(const char* ip, const char* image_path)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	int width, height;
	DfGetCameraResolution(&width, &height);

	int image_size = width * height;

	cv::Mat brightness(cv::Size(width, height), CV_8U);
	unsigned char* brightness_buf = (unsigned char*)brightness.data;
	ret = DfGetCameraData(0, 0,
		brightness_buf, image_size,
		0, 0,
		0, 0);
	cv::imwrite(image_path, brightness);

	DfDisconnectNet();
	return 1;
}

int get_frame_hdr(const char* ip, const char* frame_path)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	int width, height;
	DfGetCameraResolution(&width, &height);


	ret = DfGetCalibrationParam(calibration_param_);

	int image_size = width * height;

	int depth_buf_size = image_size * 1 * 4;
	float* depth_buf = (float*)(new char[depth_buf_size]);

	int brightness_bug_size = image_size;
	unsigned char* brightness_buf = new unsigned char[brightness_bug_size];

	ret = DfGetFrameHdr(depth_buf, depth_buf_size, brightness_buf, brightness_bug_size);



	save_frame(depth_buf, brightness_buf, frame_path);



	delete[] depth_buf;
	delete[] brightness_buf;

	DfDisconnectNet();



	return 1;
}


int get_repetition_frame_03(const char* ip, int count, const char* frame_path)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	int width, height;
	DfGetCameraResolution(&width, &height);


	ret = DfGetCalibrationParam(calibration_param_);

	int image_size = width * height;

	int depth_buf_size = image_size * 1 * 4;
	float* depth_buf = (float*)(new char[depth_buf_size]);

	int brightness_bug_size = image_size;
	unsigned char* brightness_buf = new unsigned char[brightness_bug_size];

	ret = DfGetRepetitionFrame03(count,depth_buf, depth_buf_size, brightness_buf, brightness_bug_size);

	DfDisconnectNet();

	save_frame(depth_buf, brightness_buf, frame_path);



	delete[] depth_buf;
	delete[] brightness_buf;

}

int get_frame_03(const char* ip, const char* frame_path)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	int width, height;
	DfGetCameraResolution(&width, &height);


	ret = DfGetCalibrationParam(calibration_param_);

	int image_size = width * height;

	int depth_buf_size = image_size * 1 * 4;
	float* depth_buf = (float*)(new char[depth_buf_size]);

	int brightness_bug_size = image_size;
	unsigned char* brightness_buf = new unsigned char[brightness_bug_size];

	ret = DfGetFrame03(depth_buf, depth_buf_size, brightness_buf, brightness_bug_size);

	DfDisconnectNet();

	save_frame(depth_buf, brightness_buf, frame_path);



	delete[] depth_buf;
	delete[] brightness_buf;



	return 1;
}

int get_frame_01(const char* ip, const char* frame_path)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	int width, height;
	DfGetCameraResolution(&width, &height);


	ret = DfGetCalibrationParam(calibration_param_);

	int image_size = width * height;

	int depth_buf_size = image_size * 1 * 4;
	float* depth_buf = (float*)(new char[depth_buf_size]);

	int brightness_bug_size = image_size;
	unsigned char* brightness_buf = new unsigned char[brightness_bug_size];

	ret = DfGetFrame01(depth_buf, depth_buf_size, brightness_buf, brightness_bug_size);



	save_frame(depth_buf, brightness_buf, frame_path);



	delete[] depth_buf;
	delete[] brightness_buf;

	DfDisconnectNet();

	return 1;
}

int get_pointcloud(const char* ip, const char* pointcloud_path)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	int width, height;
	DfGetCameraResolution(&width, &height);

	int image_size = width * height;

	int point_cloud_buf_size = image_size * 3 * 4;
	float* point_cloud_buf = (float*)(new char[point_cloud_buf_size]);

	ret = DfGetPointCloud(point_cloud_buf, point_cloud_buf_size);

	save_point_cloud(point_cloud_buf, pointcloud_path);

	delete[] point_cloud_buf;

	DfDisconnectNet();
	return 1;
}

int get_raw_01(const char* ip, const char* raw_image_dir)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	int capture_num = 24;

	int width, height;
	DfGetCameraResolution(&width, &height);

	int image_size = width * height;

	unsigned char* raw_buf = new unsigned char[(long)(image_size * capture_num)];

	ret = DfGetCameraRawData01(raw_buf, image_size * capture_num);

	save_images(raw_image_dir, raw_buf, image_size, capture_num);

	delete[] raw_buf;

	DfDisconnectNet();
	return 1;

}


int get_raw_03(const char* ip, const char* raw_image_dir)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	int width, height;
	DfGetCameraResolution(&width, &height);

	int image_size = width * height;

	unsigned char* raw_buf = new unsigned char[(long)(image_size * 31)];

	ret = DfGetCameraRawData03(raw_buf, image_size * 31);

	save_images(raw_image_dir, raw_buf, image_size, 31);

	delete[] raw_buf;

	DfDisconnectNet();
	return 1;
}

int get_raw_02(const char* ip, const char* raw_image_dir)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	int width, height;
	DfGetCameraResolution(&width, &height);

	int capture_num = 37;

	int image_size = width * height;

	unsigned char* raw_buf = new unsigned char[(long)(image_size * capture_num)];

	ret = DfGetCameraRawDataTest(raw_buf, image_size * capture_num);

	save_images(raw_image_dir, raw_buf, image_size, capture_num);

	delete[] raw_buf;

	DfDisconnectNet();
	return 1;
}


int test_calib_param(const char* ip, const char* result_path)
{ 
	std::string cmd(use_command);

	if ("plane" == cmd)
	{
		std::cout << "plane"<<std::endl;

		struct CameraCalibParam calibration_param_;
		DfSolution solution_machine_;
		std::vector<cv::Mat> patterns_;

		bool ret = solution_machine_.captureMixedVariableWavelengthPatterns(std::string(ip), patterns_);

		if (!ret)
		{
			std::cout << "采集图像出错！" << std::endl;
			return false;
		}

		ret = solution_machine_.getCameraCalibData(std::string(ip), calibration_param_);

		if (!ret)
		{
			std::cout << "获取标定数据出错！" << std::endl;
		}


		solution_machine_.testCalibrationParamBasePlane(patterns_, calibration_param_, std::string(result_path));
	}
	else if("board" == cmd)
	{
		std::cout << "board" << std::endl; 

		struct CameraCalibParam calibration_param_;
		DfSolution solution_machine_;
		std::vector<cv::Mat> patterns_;

		bool ret = solution_machine_.captureMixedVariableWavelengthPatterns(std::string(ip), patterns_);

		if (!ret)
		{
			std::cout << "采集图像出错！"<<std::endl;
			return false;
		}

		ret = solution_machine_.getCameraCalibData(std::string(ip), calibration_param_);

		if (!ret)
		{
			std::cout << "获取标定数据出错！" << std::endl;
		}


		solution_machine_.testCalibrationParamBaseBoard(patterns_, calibration_param_, std::string(result_path));
	}

	return 1;
}

int get_calib_param(const char* ip, const char* calib_param_path)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	struct CameraCalibParam calibration_param;
	DfGetCalibrationParam(calibration_param);
	std::ofstream ofile;
	ofile.open(calib_param_path);
	for (int i = 0; i < sizeof(calibration_param) / sizeof(float); i++)
	{
		ofile << ((float*)(&calibration_param))[i] << std::endl;
	}
	ofile.close();

	DfDisconnectNet();
	return 1;
}


int set_calib_looktable(const char* ip, const char* calib_param_path)
{
	/*************************************************************************************************/
  
	struct CameraCalibParam calibration_param;
	std::ifstream ifile;
	ifile.open(calib_param_path);
	for (int i = 0; i < sizeof(calibration_param) / sizeof(float); i++)
	{
		ifile >> ((float*)(&calibration_param))[i];
		std::cout << ((float*)(&calibration_param))[i] << std::endl;
	}
	ifile.close();
	std::cout << "Read Param"<<std::endl;
	LookupTableFunction looktable_machine;
	looktable_machine.setCalibData(calibration_param);
	//looktable_machine.readCalibData(calib_param_path);
	cv::Mat xL_rotate_x;
	cv::Mat xL_rotate_y;
	cv::Mat rectify_R1;
	cv::Mat pattern_mapping;


	std::cout << "Start Generate LookTable Param" << std::endl;
	bool ok = looktable_machine.generateLookTable(xL_rotate_x, xL_rotate_y, rectify_R1, pattern_mapping);

	std::cout << "Finished Generate LookTable Param: "<< ok << std::endl;

	xL_rotate_x.convertTo(xL_rotate_x, CV_32F);
	xL_rotate_y.convertTo(xL_rotate_y, CV_32F);
	rectify_R1.convertTo(rectify_R1, CV_32F);
	pattern_mapping.convertTo(pattern_mapping, CV_32F);
 
	/**************************************************************************************************/

	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}



	DfSetCalibrationLookTable(calibration_param, (float*)xL_rotate_x.data, (float*)xL_rotate_y.data, (float*)rectify_R1.data, (float*)pattern_mapping.data);

	 

	DfDisconnectNet();
	return 1;
}


int get_camera_exposure_param(const char* ip, float& exposure)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}


	DfGetParamCameraExposure(exposure);


	DfDisconnectNet();


	std::cout << "camera exposure: " << exposure << std::endl;
}

int set_camera_exposure_param(const char* ip, float exposure)
{
	if (exposure < 20 || exposure> 1000000)
	{
		std::cout << "exposure param out of range!" << std::endl;
		return 0;
	}


	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}
	

	DfSetParamCameraExposure(exposure);


	DfDisconnectNet();


	std::cout << "camera exposure: " << exposure << std::endl;

	return 1;
}


int get_generate_brightness_param(const char* ip, int& model, float& exposure)
{

	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	DfGetParamGenerateBrightness(model, exposure); 

	DfDisconnectNet();


	std::cout << "model: " << model << std::endl;
	std::cout << "exposure: " << exposure << std::endl;
	return 1;
}

int set_generate_brightness_param(const char* ip, int model, float exposure)
{
	if (exposure < 20 || exposure> 1000000)
	{
		std::cout << "exposure param out of range!" << std::endl;
		return 0;
	}

	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	DfSetParamGenerateBrightness(model, exposure);
  

	DfDisconnectNet(); 

	std::cout << "model: " << model << std::endl;
	std::cout << "exposure: " << exposure << std::endl;
	return 1;
}

int set_calib_param(const char* ip, const char* calib_param_path)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	struct CameraCalibParam calibration_param;
	std::ifstream ifile;
	ifile.open(calib_param_path);
	for (int i = 0; i < sizeof(calibration_param) / sizeof(float); i++)
	{
		ifile >> ((float*)(&calibration_param))[i];
		std::cout << ((float*)(&calibration_param))[i] << std::endl;
	}
	ifile.close();

	DfSetCalibrationParam(calibration_param);

	DfDisconnectNet();
	return 1;
}

int get_temperature(const char* ip)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	float temperature = 0;
	DfGetDeviceTemperature(temperature);
	std::cout << "Device temperature: " << temperature << std::endl;

	DfDisconnectNet();
	return 1;
}

// -------------------------------------------------------------------
// -- enable and disable checkerboard, by wangtong, 2022-01-27
int enable_checkerboard(const char* ip)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	float temperature = 0;
	DfEnableCheckerboard(temperature);
	std::cout << "Enable checkerboard: " << temperature << std::endl;

	DfDisconnectNet();
	return 1;
}

int disable_checkerboard(const char* ip)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	float temperature = 0;
	DfDisableCheckerboard(temperature);
	std::cout << "Disable checkerboard: " << temperature << std::endl;

	DfDisconnectNet();
	return 1;
}

// -------------------------------------------------------------------
#define PATTERN_DATA_SIZE 0xA318						// Now the pattern data build size is 0xA318 = 41752 bytes.
int load_pattern_data(const char* ip)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	char * LoadBuffer = new char[PATTERN_DATA_SIZE];

	DfLoadPatternData(PATTERN_DATA_SIZE, LoadBuffer);							

	char string[50] = { '\0' };
	FILE* fw;
	if (fopen_s(&fw, "pattern_data.dat", "wb") == 0) {
		fwrite(LoadBuffer, 1, PATTERN_DATA_SIZE, fw);
		fclose(fw);
		sprintf_s(string, "pattern_data.dat");
	}
	else {
		sprintf_s(string, "save pattern data fail");
	}

	std::cout << "Load Pattern save as:" << string << std::endl;

	delete[] LoadBuffer;

	DfDisconnectNet();
	return 1;
}

int program_pattern_data(const char* ip)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	// allocate the org pattern data & read back data buffer.
	char* pOrg = new char[PATTERN_DATA_SIZE];
	char* pBack = new char[PATTERN_DATA_SIZE];

	// read the pattern data from file into the front half of the buffer.
	FILE* fw;
	if (fopen_s(&fw, "pattern_data.dat", "rb") == 0) {
		fread_s(pOrg, PATTERN_DATA_SIZE, 1, PATTERN_DATA_SIZE, fw);
		fclose(fw);
		std::cout << "Program Pattern:" << "load file ok!" << std::endl;
	}
	else {
		std::cout << "Program Pattern:" << "load file fail..." << std::endl;
	}

	DfProgramPatternData(pOrg, pBack, PATTERN_DATA_SIZE);

	// check the program and load data be the same.
	if ( memcmp(pOrg, pBack, PATTERN_DATA_SIZE) ) {
		std::cout << "Program Pattern:" << "fail..." << std::endl;
	} else {
		std::cout << "Program Pattern:" << "ok!" << std::endl;
	}

	delete[] pOrg;
	delete[] pBack;

	DfDisconnectNet();
	return 1;
}

int get_network_bandwidth(const char* ip)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	int speed = 0;
	DfGetNetworkBandwidth(speed);
	std::cout << "Network bandwidth: " << speed << std::endl;

	DfDisconnectNet();
	return 1;
}

int get_firmware_version(const char* ip)
{
	DfRegisterOnDropped(on_dropped);

	int ret = DfConnectNet(ip);
	if (ret == DF_FAILED)
	{
		return 0;
	}

	char version[_VERSION_LENGTH_] = { '\0' };
	DfGetFirmwareVersion(version, _VERSION_LENGTH_);
	std::cout << "Firmware: " << version << std::endl;

	DfDisconnectNet();
	return 1;
}