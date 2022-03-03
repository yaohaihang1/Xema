#pragma once

#ifdef _WIN32 
#define DF_SDK_API __declspec(dllexport)

#elif __linux
#define DF_SDK_API 
#endif
 


/***************************************************************************************/

extern "C"
{

	//返回码
	//0: 成功; -1:失败; -2:未获取相机分辨率分配内存

	//相机标定参数结构体
	struct CalibrationParam
	{
		//相机内参
		float intrinsic[3 * 3];
		//相机外参
		float extrinsic[4 * 4];
		//相机畸变，只用前5个
		float distortion[1 * 12];//<k1,k2,p1,p2,k3,k4,k5,k6,s1,s2,s3,s4>

	};


	//函数名： DfConnect
	//功能： 连接相机
	//输入参数： camera_id（相机ip地址）
	//输出参数： 无
	//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
	DF_SDK_API int DfConnect(const char* camera_id);

	//函数名： DfGetCameraResolution
	//功能： 获取相机分辨率
	//输入参数： 无
	//输出参数： width(图像宽)、height(图像高)
	//返回值： 类型（int）:返回0表示获取参数成功;返回-1表示获取参数失败.
	DF_SDK_API int  DfGetCameraResolution(int* width, int* height);

	//函数名： DfCaptureData
	//功能： 采集一帧数据并阻塞至返回状态
	//输入参数： exposure_num（曝光次数）：设置值为1为单曝光，大于1为多曝光模式（具体参数在相机gui中设置）.
	//输出参数： timestamp(时间戳)
	//返回值： 类型（int）:返回0表示获取采集数据成功;返回-1表示采集数据失败.
	DF_SDK_API int DfCaptureData(int exposure_num, char* timestamp);

	//函数名： DfGetDepthData
	//功能： 采集点云数据并阻塞至返回结果
	//输入参数：无
	//输出参数： depth(深度图)
	//返回值： 类型（int）:返回0表示获取数据成功;返回-1表示采集数据失败.
	DF_SDK_API int DfGetDepthData(unsigned short* depth);


	//函数名： DfGetBrightnessData
	//功能： 采集点云数据并阻塞至返回结果
	//输入参数：无
	//输出参数： brightness(亮度图)
	//返回值： 类型（int）:返回0表示获取数据成功;返回-1表示采集数据失败.
	DF_SDK_API int DfGetBrightnessData(unsigned char* brightness);

	//函数名： DfGetHeightMapData
	//功能： 采集点云数据并阻塞至返回结果
	//输入参数：无
	//输出参数： height_map(高度映射图)
	//返回值： 类型（int）:返回0表示获取数据成功;返回-1表示采集数据失败.
	DF_SDK_API int DfGetHeightMapData(float* height_map);

	//函数名： DfGetPointcloudData
	//功能： 采集点云数据并阻塞至返回结果
	//输入参数：无
	//输出参数： point_cloud(点云)
	//返回值： 类型（int）:返回0表示获取数据成功;返回-1表示采集数据失败.
	DF_SDK_API int DfGetPointcloudData(float* point_cloud);

	//函数名： DfConnect
	//功能： 断开相机连接
	//输入参数： camera_id（相机ip地址）
	//输出参数： 无
	//返回值： 类型（int）:返回0表示断开成功;返回-1表示断开失败.
	DF_SDK_API int DfDisconnect(const char* camera_id);

	//函数名： DfGetCalibrationParam
	//功能： 获取相机标定参数
	//输入参数： 无
	//输出参数： calibration_param（相机标定参数结构体）
	//返回值： 类型（int）:返回0表示获取标定参数成功;返回-1表示获取标定参数失败.
	DF_SDK_API int DfGetCalibrationParam(struct CalibrationParam* calibration_param);


	/***************************************************************************************************************************************************************/
	 

}


/**************************************************************************************/
//函数名： DfConnectNet（区别于DfConnect:带注册的连接，DfConnectNet：不带注册直接连接）
//功能： 连接相机
//输入参数： ip
//输出参数： 无
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfConnectNet(const char* ip);

//函数名： DfDisconnectNet
//功能： 断开相机
//输入参数：无
//输出参数： 无
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfDisconnectNet();

//DF_SDK_API int DfGetCameraResolution(int* width, int* height);

//函数名： DfGetCameraData
//功能： 获取一帧相机数据
//输入参数：无
//输出参数： 无
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfGetCameraData(
	short* depth, int depth_buf_size,
	unsigned char* brightness, int brightness_buf_size,
	short* point_cloud, int point_cloud_buf_size,
	unsigned char* confidence, int confidence_buf_size);

//函数名： GetBrightness
//功能： 获取一个亮度图数据
//输入参数：brightness_buf_size（亮度图尺寸sizeof(unsigned char) * width * height）
//输出参数：brightness
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int GetBrightness(unsigned char* brightness, int brightness_buf_size);

//函数名： DfGetCameraRawData01
//功能： 采集一组相移图，一共24幅四步相移条纹图
//输入参数：raw_buf_size（24张8位图的尺寸）
//输出参数：raw
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfGetCameraRawData01(unsigned char* raw, int raw_buf_size);

//函数名： DfGetCameraRawData02
//功能： 采集一组相移图，一共37幅，24个四步相移条纹图+12个六步相移条纹图+一个亮度图
//输入参数：raw_buf_size（37张8位图的尺寸）
//输出参数：raw
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfGetCameraRawData02(unsigned char* raw, int raw_buf_size);

//函数名： DfGetCameraRawDataTest
//功能： 采集一组条纹图 
//输入参数：raw_buf_size（一组图的尺寸）
//输出参数：raw内存指针
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfGetCameraRawDataTest(unsigned char* raw, int raw_buf_size);

//函数名： DfGetCameraRawData03
//功能： 采集一组相移图，一共31幅，24个四步相移条纹图+6个垂直方向的六步相移条纹图+一个亮度图
//输入参数：raw_buf_size（31张8位图的尺寸）
//输出参数：raw
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfGetCameraRawData03(unsigned char* raw, int raw_buf_size);

//函数名： depthTransformPointcloud
//功能： 深度图转点云接口
//输入参数：depth_map（深度图）
//输出参数：point_cloud_map（点云）
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API bool depthTransformPointcloud(float* depth_map, float* point_cloud_map);

//函数名： transformPointcloud
//功能： 点云坐标系转换接口
//输入参数：rotate（旋转矩阵）、translation（平移矩阵）
//输出参数：point_cloud_map（点云）
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API bool transformPointcloud(float* point_cloud_map, float* rotate, float* translation);

//函数名： transformPointcloudInv
//功能： 点云坐标系反变换（测试接口）
DF_SDK_API bool transformPointcloudInv(float* point_cloud_map, float* rotate, float* translation);

//函数名： DfGetPointCloud
//功能： 获取点云接口，基于24张四步相移图
//输入参数：point_cloud_buf_size（点云内存大小）
//输出参数：point_cloud_map（点云）
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfGetPointCloud(float* point_cloud, int point_cloud_buf_size);

//函数名： DfGetFrame01
//功能： 获取一帧数据（亮度图+深度图），基于Raw01的相移图
//输入参数：depth_buf_size（深度图尺寸）、brightness_buf_size（亮度图尺寸）
//输出参数：depth（深度图）、brightness（亮度图）
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfGetFrame01(float* depth, int depth_buf_size,
	unsigned char* brightness, int brightness_buf_size);

//函数名： DfGetFrameHdr
//功能： 获取一帧数据（亮度图+深度图），基于Raw03相位图的HDR模式
//输入参数：depth_buf_size（深度图尺寸）、brightness_buf_size（亮度图尺寸）
//输出参数：depth（深度图）、brightness（亮度图）
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfGetFrameHdr(float* depth, int depth_buf_size,
	unsigned char* brightness, int brightness_buf_size);

//函数名： DfGetFrame03
//功能： 获取一帧数据（亮度图+深度图），基于Raw03相位图
//输入参数：depth_buf_size（深度图尺寸）、brightness_buf_size（亮度图尺寸）
//输出参数：depth（深度图）、brightness（亮度图）
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfGetFrame03(float* depth, int depth_buf_size,
	unsigned char* brightness, int brightness_buf_size);

//函数名： DfGetRepetitionFrame03
//功能： 获取一帧数据（亮度图+深度图），基于Raw03相位图，6步相移的图重复count次
//输入参数：count（重复次数）、depth_buf_size（深度图尺寸）、brightness_buf_size（亮度图尺寸）
//输出参数：depth（深度图）、brightness（亮度图）
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfGetRepetitionFrame03(int count,float* depth, int depth_buf_size,
	unsigned char* brightness, int brightness_buf_size);

//函数名： DfGetCalibrationParam
//功能：获取标定参数接口
//输入参数：无
//输出参数：calibration_param（标定参数）
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfGetCalibrationParam(struct CameraCalibParam& calibration_param);

//函数名： DfGetCalibrationParam
//功能：设置标定参数接口
//输入参数：calibration_param（标定参数）
//输出参数：无
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfSetCalibrationParam(const struct CameraCalibParam& calibration_param);

//函数名： DfGetDeviceTemperature
//功能：获取设备温度
//输入参数：无
//输出参数：temperature（摄氏度）
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfGetDeviceTemperature(float& temperature);

//函数名： DfRegisterOnDropped
//功能：注册断连回调函数
//输入参数：无
//输出参数：p_function（回调函数）
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfRegisterOnDropped(int (*p_function)(void*));

//函数名： DfGetCalibrationParam
//功能：获取相机配置参数接口
//输入参数：config_param（配置参数）
//输出参数：无
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfGetSystemConfigParam(struct SystemConfigParam& config_param);

//函数名： DfGetCalibrationParam
//功能：设置相机配置参数接口
//输入参数：config_param（配置参数）
//输出参数：无
//返回值： 类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfSetSystemConfigParam(const struct SystemConfigParam& config_param);

//函数名：  DfEnableCheckerboard
//功能：    打开光机，投影棋盘格
//输入参数：无
//输出参数：nano温度1
//返回值：  类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfEnableCheckerboard(float& temperature);

//函数名：  DfDisableCheckerboard
//功能：    关闭光机投影
//输入参数：无
//输出参数：nano温度2
//返回值：  类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfDisableCheckerboard(float& temperature);

//函数名：  DfLoadPatternData
//功能：    从光机控制板读取预先设置的42张投影图片数据--Pattern Data
//输入参数：获取Pattern Data的缓冲区大小、获取Pattern Data的缓冲区地址
//输出参数：Pattern Data数据
//返回值：  类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfLoadPatternData(int buildDataSize, char* LoadBuffer);

//函数名：  DfProgramPatternData
//功能：    将光机控制板DLP的投影图片数据--Pattern Data，由PC端写入至控制板FLASH
//输入参数：要写入的数据包地址，回读回来的数据包地址，数据包大小
//输出参数：回读的数据内容
//返回值：  类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfProgramPatternData(char* org_buffer, char* back_buffer, unsigned int pattern_size);

//函数名：  DfGetNetworkBandwidth
//功能：    获取链路的网络带宽
//输入参数：无
//输出参数：网络带宽大小
//返回值：  类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfGetNetworkBandwidth(int &speed);

//函数名：  DfGetFirmwareVersion
//功能：    获取固件版本
//输入参数：版本号缓冲区地址，缓冲区长度
//输出参数：版本号
//返回值：  类型（int）:返回0表示连接成功;返回-1表示连接失败.
DF_SDK_API int DfGetFirmwareVersion(char* pVersion, int length);