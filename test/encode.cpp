﻿#include "encode.h"
#include "iostream" 

DF_Encode::DF_Encode()
{
}


DF_Encode::~DF_Encode()
{
}

bool DF_Encode::unwrapBase2Kmap(cv::Mat wrap_map, cv::Mat k1_map, cv::Mat k2_map, cv::Mat& unwrap_map)
{
	if (wrap_map.empty() || k1_map.empty() || k2_map.empty())
	{
		return false;
	}
	int nr = wrap_map.rows;
	int nc = wrap_map.cols;


	cv::Mat unwrap(nr, nc, CV_32F, cv::Scalar(0));


	for (int r = 0; r < nr; r++)
	{
		uchar* ptr_k1 = k1_map.ptr<uchar>(r);
		uchar* ptr_k2 = k2_map.ptr<uchar>(r);
		float* ptr_wrap = wrap_map.ptr<float>(r);
		float* ptr_unwrap = unwrap.ptr<float>(r);

		for (int c = 0; c < nc; c++)
		{

			int k2 = (ptr_k2[c] + 1) / 2;

			if (ptr_wrap[c] < CV_PI / 2.0)
			{
				ptr_unwrap[c] = ptr_wrap[c] + 2 * CV_PI * k2;
			}
			else if (ptr_wrap[c] < 3 * CV_PI / 2.0)
			{

				ptr_unwrap[c] = ptr_wrap[c] + 2 * CV_PI * ptr_k1[c];
			}
			else
			{
				ptr_unwrap[c] = ptr_wrap[c] + 2 * CV_PI * (k2 - 1);
			}
		}


	}

	unwrap_map = unwrap.clone();


	return false;
}

bool DF_Encode::computePhaseShift(std::vector<cv::Mat> patterns, cv::Mat& wrap_map, cv::Mat& confidence_map, cv::Mat& average_map, cv::Mat& brightness_map, cv::Mat& mask_map)
{
	if (patterns.empty())
	{
		return false;
	}

	int nr = patterns[0].rows;
	int nc = patterns[0].cols;

	cv::Mat wrap(nr, nc, CV_32F, cv::Scalar(0));
	cv::Mat confidence(nr, nc, CV_32F, cv::Scalar(0));
	cv::Mat average(nr, nc, CV_8U, cv::Scalar(0));
	cv::Mat brightness(nr, nc, CV_8U, cv::Scalar(0));
	cv::Mat mask(nr, nc, CV_8U, cv::Scalar(0));

	switch (patterns.size())
	{
	case 6:
	{
#pragma omp parallel for
		for (int r = 0; r < nr; r++)
		{
			uchar* ptr0 = patterns[0 + 3].ptr<uchar>(r);
			uchar* ptr1 = patterns[1 + 3].ptr<uchar>(r);
			uchar* ptr2 = patterns[2 + 3].ptr<uchar>(r);
			uchar* ptr3 = patterns[3 - 3].ptr<uchar>(r);
			uchar* ptr4 = patterns[4 - 3].ptr<uchar>(r);
			uchar* ptr5 = patterns[5 - 3].ptr<uchar>(r);

			uchar* ptr_m = mask.ptr<uchar>(r);
			uchar* ptr_avg = average.ptr<uchar>(r);
			uchar* ptr_b = brightness.ptr<uchar>(r);
			float* ptr_con = confidence.ptr<float>(r);
			float* ptr_wrap = wrap.ptr<float>(r);

			for (int c = 0; c < nc; c++)
			{
				int exposure_num = 0;

				if (255 == ptr0[c])
				{
					exposure_num++;
				}
				if (255 == ptr1[c])
				{
					exposure_num++;
				}
				if (255 == ptr2[c])
				{
					exposure_num++;
				}
				if (255 == ptr3[c])
				{
					exposure_num++;
				}
				if (255 == ptr4[c])
				{
					exposure_num++;
				}
				if (255 == ptr5[c])
				{
					exposure_num++;
				}


				float b = ptr0[c] * std::sin(0 * CV_2PI / 6.0) + ptr1[c] * std::sin(1 * CV_2PI / 6.0) + ptr2[c] * std::sin(2 * CV_2PI / 6.0)
					+ ptr3[c] * std::sin(3 * CV_2PI / 6.0) + ptr4[c] * std::sin(4 * CV_2PI / 6.0) + ptr5[c] * std::sin(5 * CV_2PI / 6.0);

				float a = ptr0[c] * std::cos(0 * CV_2PI / 6.0) + ptr1[c] * std::cos(1 * CV_2PI / 6.0) + ptr2[c] * std::cos(2 * CV_2PI / 6.0)
					+ ptr3[c] * std::cos(3 * CV_2PI / 6.0) + ptr4[c] * std::cos(4 * CV_2PI / 6.0) + ptr5[c] * std::cos(5 * CV_2PI / 6.0);

				float ave = (ptr0[c] + ptr1[c] + ptr2[c] + ptr3[c] + ptr4[c] + ptr5[c]) / 6.0;

				float r = std::sqrt(a * a + b * b);

				ptr_avg[c] = ave + 0.5;
				ptr_con[c] = r / 3.0;
				ptr_b[c] = ave + 0.5 + r / 3.0;

				/***********************************************************************/

				if (exposure_num > 3)
				{
					ptr_m[c] = 0;
					ptr_wrap[c] = -1;
				}
				else
				{
					ptr_wrap[c] = CV_PI + std::atan2(a, b);
				}


			}
		}
	}
	break;
	default:
		break;
	}




	/*****************************************************************************************************************************/

	confidence_map = confidence.clone();
	wrap_map = wrap.clone();
	brightness_map = brightness.clone();
	average_map = average.clone();
	mask_map = mask.clone();

	return true;
}

bool DF_Encode::grayCodeToBinCode(std::vector<bool> gray_code, std::vector<bool>& bin_code)
{
	if (gray_code.empty())
	{
		return false;
	}

	bin_code.push_back(gray_code[0]);

	for (int i = 1; i < gray_code.size(); i++)
	{
		bool val = bin_code[i - 1] ^ gray_code[i];
		bin_code.push_back(val);
	}

	return true;
}

bool DF_Encode::decodeGrayCode(std::vector<cv::Mat> patterns, cv::Mat average_brightness, cv::Mat& k1_map, cv::Mat& k2_map)
{
	//bin threshold

	int nr = average_brightness.rows;
	int nc = average_brightness.cols;

	//std::vector<std::vector<bool>> gray_code_list;
	//threshold bin
	std::vector<cv::Mat> bin_patterns;
	for (int i = 0; i < patterns.size(); i++)
	{
		cv::Mat bin_mat(nr, nc, CV_8U, cv::Scalar(0));

		for (int r = 0; r < nr; r++)
		{
			uchar* ptr_bin = bin_mat.ptr<uchar>(r);
			uchar* ptr_avg = average_brightness.ptr<uchar>(r);
			uchar* ptr_gray = patterns[i].ptr<uchar>(r);

			for (int c = 0; c < nc; c++)
			{
				if (ptr_gray[c] < ptr_avg[c])
				{
					ptr_bin[c] = 0;
				}
				else
				{
					ptr_bin[c] = 255;
				}
			}
		}
		bin_patterns.push_back(bin_mat.clone());
	}

	cv::Mat k1(nr, nc, CV_8U, cv::Scalar(0));
	cv::Mat k2(nr, nc, CV_8U, cv::Scalar(0));


	for (int r = 0; r < nr; r++)
	{
		uchar* ptr_k1 = k1.ptr<uchar>(r);
		uchar* ptr_k2 = k2.ptr<uchar>(r);

		for (int c = 0; c < nc; c++)
		{
			std::vector<bool> gray_code_list;
			std::vector<bool> bin_code_list;

			for (int i = 0; i < bin_patterns.size(); i++)
			{
				uchar val = bin_patterns[i].at<uchar>(r, c);

				if (255 == val)
				{
					gray_code_list.push_back(true);
				}
				else
				{
					gray_code_list.push_back(false);
				}
			}

			grayCodeToBinCode(gray_code_list, bin_code_list);

			uchar k_2 = 0;
			uchar k_1 = 0;
			for (int i = 0; i < bin_code_list.size(); i++)
			{
				k_2 += bin_code_list[i] * std::pow(2, bin_code_list.size() - i - 1);

				if (i < bin_code_list.size() - 1)
				{
					k_1 += bin_code_list[i] * std::pow(2, bin_code_list.size() - i - 2);
				}
			}
			ptr_k2[c] = k_2;
			ptr_k1[c] = k_1;
		}

	}


	k1_map = k1.clone();
	k2_map = k2.clone();


	return true;
}


/**************************************************************************************************************/

bool DF_Encode::mergePatterns(std::vector<std::vector<cv::Mat>> patterns_list, std::vector<cv::Mat>& patterns)
{
	if (patterns_list.empty())
	{
		return false;
	}

	int nr = patterns_list[0][0].rows;
	int nc = patterns_list[0][0].cols;
	int patterns_num = patterns_list[0].size();

	for (int i = 0; i < patterns_num; i++)
	{
		cv::Mat pattern(nr, nc, CV_16U, cv::Scalar(0));

		for (int r = 0; r < nr; r++)
		{

			for (int c = 0; c < nc; c++)
			{
				for (int l_i = 0; l_i < patterns_list.size(); l_i++)
				{
					pattern.at<ushort>(r, c) += patterns_list[l_i][i].at<uchar>(r, c);
				}
			}

		}


		patterns.push_back(pattern.clone());
	}

}

bool DF_Encode::sixStepPhaseShift(std::vector<cv::Mat> patterns, cv::Mat& wrap_map, cv::Mat& mask, cv::Mat& confidence)
{
	if (6 != patterns.size())
	{
		return false;
	}

	cv::Mat img0 = patterns[0 + 3];
	cv::Mat img1 = patterns[1 + 3];
	cv::Mat img2 = patterns[2 + 3];
	cv::Mat img3 = patterns[3 - 3];
	cv::Mat img4 = patterns[4 - 3];
	cv::Mat img5 = patterns[5 - 3];


	cv::Mat result(img1.rows, img1.cols, CV_64F, cv::Scalar(-1));
	cv::Mat confidence_map(img1.rows, img1.cols, CV_64F, cv::Scalar(0));

	if (!mask.data)
	{
		mask = cv::Mat(img1.rows, img1.cols, CV_8U, cv::Scalar(255));
	}

	int nl = img1.rows;
	int nc = img1.cols * img1.channels();


	if (img0.isContinuous())
	{
		if (img1.isContinuous())
		{
			if (img2.isContinuous())
			{
				if (img3.isContinuous())
				{
					if (img4.isContinuous())
					{
						if (img5.isContinuous())
						{

							if (mask.isContinuous())
							{
								nc = nc * nl;
								nl = 1;
							}

						}
					}
				}
			}

		}
	}

	if (CV_16U == img0.type())
	{

#pragma omp parallel for
		for (int r = 0; r < nl; r++)
		{

			ushort* ptr0 = img0.ptr<ushort>(r);
			ushort* ptr1 = img1.ptr<ushort>(r);
			ushort* ptr2 = img2.ptr<ushort>(r);
			ushort* ptr3 = img3.ptr<ushort>(r);
			ushort* ptr4 = img4.ptr<ushort>(r);
			ushort* ptr5 = img5.ptr<ushort>(r);
			uchar* ptr_m = mask.ptr<uchar>(r);
			double* ptr_con = confidence_map.ptr<double>(r);

			double* optr = result.ptr<double>(r);
			for (int c = 0; c < nc; c++)
			{
				int exposure_num = 0;
				if (ptr_m[c])
				{
					//double a = ptr4[j] - ptr2[j];
					//double b = ptr1[j] - ptr3[j];
					if (255 == ptr0[c])
					{
						exposure_num++;
					}

					if (255 == ptr1[c])
					{
						exposure_num++;
					}
					if (255 == ptr2[c])
					{
						exposure_num++;
					}
					if (255 == ptr3[c])
					{
						exposure_num++;
					}
					if (255 == ptr4[c])
					{
						exposure_num++;
					}

					if (255 == ptr5[c])
					{
						exposure_num++;
					}


					double b = ptr0[c] * std::sin(0 * CV_2PI / 6.0) + ptr1[c] * std::sin(1 * CV_2PI / 6.0) + ptr2[c] * std::sin(2 * CV_2PI / 6.0)
						+ ptr3[c] * std::sin(3 * CV_2PI / 6.0) + ptr4[c] * std::sin(4 * CV_2PI / 6.0) + ptr5[c] * std::sin(5 * CV_2PI / 6.0);


					double a = ptr0[c] * std::cos(0 * CV_2PI / 6.0) + ptr1[c] * std::cos(1 * CV_2PI / 6.0) + ptr2[c] * std::cos(2 * CV_2PI / 6.0)
						+ ptr3[c] * std::cos(3 * CV_2PI / 6.0) + ptr4[c] * std::cos(4 * CV_2PI / 6.0) + ptr5[c] * std::cos(5 * CV_2PI / 6.0);

					double r = std::sqrt(a * a + b * b);

					//if (r > 255)
					//{
					//	r = 255;
					//}

					ptr_con[c] = r;

					/***********************************************************************/

					if (exposure_num > 3)
					{
						ptr_m[c] = 0;
						ptr_con[c] = 0;
						optr[c] = -1;
					}
					else
					{
						optr[c] = CV_PI + std::atan2(a, b);
					}

				}
			}
		}


		/*****************************************************************************************************************************/

		confidence = confidence_map.clone();

		wrap_map = result.clone();

		return true;
	}
	else if (CV_8U == img0.type())
	{

#pragma omp parallel for
		for (int r = 0; r < nl; r++)
		{

			uchar* ptr0 = img0.ptr<uchar>(r);
			uchar* ptr1 = img1.ptr<uchar>(r);
			uchar* ptr2 = img2.ptr<uchar>(r);
			uchar* ptr3 = img3.ptr<uchar>(r);
			uchar* ptr4 = img4.ptr<uchar>(r);
			uchar* ptr5 = img5.ptr<uchar>(r);
			uchar* ptr_m = mask.ptr<uchar>(r);
			double* ptr_con = confidence_map.ptr<double>(r);

			double* optr = result.ptr<double>(r);
			for (int c = 0; c < nc; c++)
			{
				int exposure_num = 0;
				if (ptr_m[c])
				{
					//double a = ptr4[j] - ptr2[j];
					//double b = ptr1[j] - ptr3[j];
					if (255 == ptr0[c])
					{
						exposure_num++;
					}

					if (255 == ptr1[c])
					{
						exposure_num++;
					}
					if (255 == ptr2[c])
					{
						exposure_num++;
					}
					if (255 == ptr3[c])
					{
						exposure_num++;
					}
					if (255 == ptr4[c])
					{
						exposure_num++;
					}

					if (255 == ptr5[c])
					{
						exposure_num++;
					}


					double b = ptr0[c] * std::sin(0 * CV_2PI / 6.0) + ptr1[c] * std::sin(1 * CV_2PI / 6.0) + ptr2[c] * std::sin(2 * CV_2PI / 6.0)
						+ ptr3[c] * std::sin(3 * CV_2PI / 6.0) + ptr4[c] * std::sin(4 * CV_2PI / 6.0) + ptr5[c] * std::sin(5 * CV_2PI / 6.0);


					double a = ptr0[c] * std::cos(0 * CV_2PI / 6.0) + ptr1[c] * std::cos(1 * CV_2PI / 6.0) + ptr2[c] * std::cos(2 * CV_2PI / 6.0)
						+ ptr3[c] * std::cos(3 * CV_2PI / 6.0) + ptr4[c] * std::cos(4 * CV_2PI / 6.0) + ptr5[c] * std::cos(5 * CV_2PI / 6.0);

					double r = std::sqrt(a * a + b * b);

					//if (r > 255)
					//{
					//	r = 255;
					//}

					ptr_con[c] = r;

					/***********************************************************************/

					if (exposure_num > 3)
					{
						ptr_m[c] = 0;
						ptr_con[c] = 0;
						optr[c] = -1;
					}
					else
					{
						optr[c] = CV_PI + std::atan2(a, b);
					}

				}
			}
		}


		/*****************************************************************************************************************************/

		confidence = confidence_map.clone();

		wrap_map = result.clone();

		return true;
	}

	return false;
}

bool DF_Encode::fourStepPhaseShift(std::vector<cv::Mat> patterns, cv::Mat& wrap_map, cv::Mat& mask, cv::Mat& confidence)
{
	if (4 != patterns.size())
	{
		return false;
	}

	cv::Mat img1 = patterns[0];
	cv::Mat img2 = patterns[1];
	cv::Mat img3 = patterns[2];
	cv::Mat img4 = patterns[3];


	cv::Mat result(img1.rows, img1.cols, CV_64F, cv::Scalar(-1));
	cv::Mat confidence_map(img1.rows, img1.cols, CV_64F, cv::Scalar(0));

	if (!mask.data)
	{
		mask = cv::Mat(img1.rows, img1.cols, CV_8U, cv::Scalar(255));
	}

	int nl = img1.rows;
	int nc = img1.cols * img1.channels();

	if (img1.isContinuous())
	{
		if (img2.isContinuous())
		{
			if (img3.isContinuous())
			{
				if (img4.isContinuous())
				{
					if (mask.isContinuous())
					{
						nc = nc * nl;
						nl = 1;
					}
				}
			}
		}

	}


	if (CV_16U == img1.type())
	{

#pragma omp parallel for
		for (int i = 0; i < nl; i++)
		{
			ushort* ptr1 = img1.ptr<ushort>(i);
			ushort* ptr2 = img2.ptr<ushort>(i);
			ushort* ptr3 = img3.ptr<ushort>(i);
			ushort* ptr4 = img4.ptr<ushort>(i);
			uchar* ptr_m = mask.ptr<uchar>(i);
			double* ptr_con = confidence_map.ptr<double>(i);

			double* optr = result.ptr<double>(i);
			for (int j = 0; j < nc; j++)
			{
				int exposure_num = 0;
				if (ptr_m[j] == 255)
				{
					if (255 == ptr1[j])
					{
						exposure_num++;
					}
					if (255 == ptr2[j])
					{
						exposure_num++;
					}
					if (255 == ptr3[j])
					{
						exposure_num++;
					}
					if (255 == ptr4[j])
					{
						exposure_num++;
					}

					double a = ptr4[j] - ptr2[j];
					double b = ptr1[j] - ptr3[j];

					double r = std::sqrt(a * a + b * b) + 0.5;

					//if(r> 255)
					//{
					//	r = 255;
					//}

					ptr_con[j] = r;

					/***********************************************************************/

					if (exposure_num > 1)
					{
						ptr_m[j] = 0;
						ptr_con[j] = 0;
						optr[j] = -1;
					}
					else
					{
						optr[j] = CV_PI + std::atan2(a, b);
					}

				}
			}
		}


		/*****************************************************************************************************************************/

		confidence = confidence_map.clone();

		wrap_map = result.clone();

		return true;

	}
	else if (CV_8U == img1.type())
	{

#pragma omp parallel for
		for (int i = 0; i < nl; i++)
		{
			uchar* ptr1 = img1.ptr<uchar>(i);
			uchar* ptr2 = img2.ptr<uchar>(i);
			uchar* ptr3 = img3.ptr<uchar>(i);
			uchar* ptr4 = img4.ptr<uchar>(i);
			uchar* ptr_m = mask.ptr<uchar>(i);
			double* ptr_con = confidence_map.ptr<double>(i);

			double* optr = result.ptr<double>(i);
			for (int j = 0; j < nc; j++)
			{
				int exposure_num = 0;
				if (ptr_m[j] == 255)
				{
					if (255 == ptr1[j])
					{
						exposure_num++;
					}
					if (255 == ptr2[j])
					{
						exposure_num++;
					}
					if (255 == ptr3[j])
					{
						exposure_num++;
					}
					if (255 == ptr4[j])
					{
						exposure_num++;
					}

					double a = ptr4[j] - ptr2[j];
					double b = ptr1[j] - ptr3[j];

					double r = std::sqrt(a * a + b * b) + 0.5;

					//if(r> 255)
					//{
					//	r = 255;
					//}

					ptr_con[j] = r;

					/***********************************************************************/

					if (exposure_num > 1 || ptr_con[j] < 4)
					{
						ptr_m[j] = 0;
						ptr_con[j] = 0;
						optr[j] = -1;
					}
					else
					{
						optr[j] = CV_PI + std::atan2(a, b);
					}

				}
			}
		}


		/*****************************************************************************************************************************/

		confidence = confidence_map.clone();

		wrap_map = result.clone();

		return true;
	}



	return false;
}


bool DF_Encode::unwrapVariableWavelength(cv::Mat l_unwrap, cv::Mat h_wrap, double rate, cv::Mat& h_unwrap, cv::Mat& k_Mat, float threshold, cv::Mat& err_mat)
{

	if (l_unwrap.empty() || h_wrap.empty())
	{
		return false;
	}

	int nr = l_unwrap.rows;
	int nc = l_unwrap.cols;

	if (l_unwrap.isContinuous())
	{
		if (h_wrap.isContinuous())
		{
			if (k_Mat.isContinuous())
			{

				nc = nc * nr;
				nr = 1;
			}
		}

	}

	cv::Mat err_map(l_unwrap.size(), CV_64F, cv::Scalar(0.0));

	for (int r = 0; r < nr; r++)
	{
		double* l_ptr = l_unwrap.ptr<double>(r);
		double* h_ptr = h_wrap.ptr<double>(r);
		uchar* k_ptr = k_Mat.ptr<uchar>(r);
		double* h_unwrap_ptr = h_unwrap.ptr<double>(r);

		double* ptr_err = err_map.ptr<double>(r);

		for (int c = 0; c < nc; c++)
		{

			//double temp = 0.5 + l_ptr[j] / (1 * CV_PI) - h_ptr[j] / (rate * CV_PI); 

			double temp = 0.5 + (rate * l_ptr[c] - h_ptr[c]) / (2 * CV_PI);
			int k = temp;
			h_unwrap_ptr[c] = 2 * CV_PI * k + h_ptr[c];

			ptr_err[c] = fabs(h_unwrap_ptr[c] - rate * l_ptr[c]);

			k_ptr[c] = k;

			if (ptr_err[c] > threshold)
			{
				h_unwrap_ptr[c] = -10;

			}

			//int k = temp; 
			//k_ptr[j] = k; 
			//h_unwrap_ptr[j] = 2 * CV_PI * k + h_ptr[j];


		}
	}

	err_mat = err_map.clone();

	return true;
}

void DF_Encode::unwarpDualWavelength(cv::Mat l_unwrap, cv::Mat h_wrap, cv::Mat& h_unwrap, cv::Mat& k_Mat)
{


	int nr = l_unwrap.rows;
	int nc = l_unwrap.cols;

	if (l_unwrap.isContinuous())
	{
		if (h_wrap.isContinuous())
		{
			if (k_Mat.isContinuous())
			{

				nc = nc * nr;
				nr = 1;
			}
		}

	}


	for (int i = 0; i < nr; i++)
	{
		double* l_ptr = l_unwrap.ptr<double>(i);
		double* h_ptr = h_wrap.ptr<double>(i);
		uchar* k_ptr = k_Mat.ptr<uchar>(i);
		double* h_unwrap_ptr = h_unwrap.ptr<double>(i);
		for (int j = 0; j < nc; j++)
		{

			double temp = 0.5 + l_ptr[j] / (1 * CV_PI) - h_ptr[j] / (2 * CV_PI);

			int k = temp;


			k_ptr[j] = k;

			h_unwrap_ptr[j] = 2 * CV_PI * k + h_ptr[j];




		}
	}


}

bool DF_Encode::unwrapHalfWavelengthPatternsOpenmp(std::vector<cv::Mat> wrap_img_list, cv::Mat& unwrap_img, cv::Mat& mask)
{
	if (wrap_img_list.empty())
	{
		return false;
	}



	bool unwrap_filter = false;


	if (mask.data)
	{
		unwrap_filter = true;
	}

	std::vector<cv::Mat> unwrap_img_list;
	unwrap_img_list.push_back(wrap_img_list[0]);

	int nr = wrap_img_list[0].rows;
	int nc = wrap_img_list[0].cols;

	for (int w_i = 0; w_i < wrap_img_list.size() - 1; w_i++)
	{
		cv::Mat img_1 = unwrap_img_list[w_i];
		cv::Mat img_2 = wrap_img_list[w_i + 1];

		std::cout << "w_i: " << w_i;

		cv::Mat k_mat(nr, nc, CV_8U, cv::Scalar(0));
		cv::Mat unwrap_mat(nr, nc, CV_64F, cv::Scalar(0));

		unwarpDualWavelength(img_1, img_2, unwrap_mat, k_mat);
		unwrap_img_list.push_back(unwrap_mat);

	}


	float period_num = std::pow(2, unwrap_img_list.size() - 1);



	//unwrap_img = unwrap_img_list[unwrap_img_list.size() - 1].clone()/ period_num;

	unwrap_img = unwrap_img_list[unwrap_img_list.size() - 1].clone();


	return true;
}


bool DF_Encode::unwrapVariableWavelengthPatterns(std::vector<cv::Mat> wrap_img_list, std::vector<double> rate_list, cv::Mat& unwrap_img, cv::Mat& mask)
{
	if (wrap_img_list.empty())
	{
		return false;
	}
	if (wrap_img_list.size() != rate_list.size() + 1)
	{
		return false;
	}

	std::vector<float> threshold_list;

	for (int i = 0; i < rate_list.size(); i++)
	{
		threshold_list.push_back(CV_PI);
	}


	if (threshold_list.size() >= 3)
	{
		threshold_list[0] = CV_PI;
		threshold_list[1] = CV_PI;
		threshold_list[2] = 1.5;
	}


	int nr = wrap_img_list[0].rows;
	int nc = wrap_img_list[0].cols;

	bool unwrap_filter = false;

	if (mask.data)
	{
		unwrap_filter = true;
	}

	cv::Mat h_unwrap_map(nr, nc, CV_64F, cv::Scalar(0));

	cv::Mat err_map_l(nr, nc, CV_64F, cv::Scalar(0));
	cv::Mat err_map_h(nr, nc, CV_64F, cv::Scalar(0));

	cv::Mat unwrap_map = wrap_img_list[0];

	cv::Mat k_mat(nr, nc, CV_8U, cv::Scalar(0));

	for (int g_i = 1; g_i < wrap_img_list.size(); g_i++)
	{
		cv::Mat wrap_map = wrap_img_list[g_i];
		cv::Mat h_unwrap_map(nr, nc, CV_64F, cv::Scalar(0));
		cv::Mat err_map;

		unwrapVariableWavelength(unwrap_map, wrap_map, rate_list[g_i - 1], h_unwrap_map, k_mat, threshold_list[g_i - 1], err_map);

		unwrap_map = h_unwrap_map.clone();
	}

	unwrap_img = unwrap_map.clone();

	return true;
}

bool DF_Encode::unwrapVariableWavelengthPatternsBaseConfidence(std::vector<cv::Mat> wrap_img_list, std::vector<double> rate_list, cv::Mat& unwrap_img, cv::Mat& mask)
{
	if (wrap_img_list.empty())
	{
		return false;
	}
	if (wrap_img_list.size() != rate_list.size() + 1)
	{
		return false;
	}

	std::vector<float> threshold_list;

	for (int i = 0; i < rate_list.size(); i++)
	{
		threshold_list.push_back(CV_PI);
	}


	if (threshold_list.size() >= 3)
	{
		threshold_list[0] = CV_PI;
		threshold_list[1] = CV_PI;
		threshold_list[2] = 1.5;
	}


	int nr = wrap_img_list[0].rows;
	int nc = wrap_img_list[0].cols;

	bool unwrap_filter = false;

	if (mask.data)
	{
		unwrap_filter = true;
	}

	cv::Mat h_unwrap_map(nr, nc, CV_64F, cv::Scalar(0));

	cv::Mat err_map_l(nr, nc, CV_64F, cv::Scalar(0));
	cv::Mat err_map_h(nr, nc, CV_64F, cv::Scalar(0));

	cv::Mat unwrap_map = wrap_img_list[0];

	cv::Mat k_mat(nr, nc, CV_8U, cv::Scalar(0));

	for (int g_i = 1; g_i < wrap_img_list.size(); g_i++)
	{
		cv::Mat wrap_map = wrap_img_list[g_i];
		cv::Mat h_unwrap_map(nr, nc, CV_64F, cv::Scalar(0));
		cv::Mat err_map;

		unwrapVariableWavelength(unwrap_map, wrap_map, rate_list[g_i - 1], h_unwrap_map, k_mat, threshold_list[g_i - 1], err_map);

		const double fisher_rates[] = { -6.61284856e-06, 4.52035763e-06, -1.16182132e-05 };//[-6.61284856e-06  4.52035763e-06 -1.16182132e-05 -2.89004663e-05]
		double fisher_temp = fisher_rates[g_i - 1];
		for (int r = 0; r < mask.rows; r += 1)
		{
			double* mask_ptr = mask.ptr<double>(r);
			double* err_ptr = err_map.ptr<double>(r);
			for (int c = 0; c < mask.cols; c += 1)
			{
				mask_ptr[c] += err_ptr[c] * fisher_temp;
			}
		}

		unwrap_map = h_unwrap_map.clone();
	}

	unwrap_img = unwrap_map.clone();

	cv::Mat neighborhoodCharacteristicsR(unwrap_map.size(), CV_64F, cv::Scalar(0));
	// 注意避坑：邻域的计算当中应当避免出现的问题是
	for (int r = 0; r < neighborhoodCharacteristicsR.rows; r += 1)
	{
		double* neighborhoodR_Ptr = neighborhoodCharacteristicsR.ptr<double>(r);
		double* data_ptr = unwrap_map.ptr<double>(r);
		double* mask_ptr = mask.ptr<double>(r);

		int numR = 0, numC = 0;

		for (int c = 1; c < neighborhoodCharacteristicsR.cols - 1; c += 1)
		{
			// 在此处需要循环找到非-10的值
			while (data_ptr[c] == -10 && c < neighborhoodCharacteristicsR.cols - 1)
			{
				c += 1;
				mask_ptr[c] = -10;
			}
			numC = c;
			while (data_ptr[c + 1] == -10 && c < neighborhoodCharacteristicsR.cols - 1)
			{
				c += 1;
				mask_ptr[c] = -10;
			}
			numR = c + 1;
			neighborhoodR_Ptr[c] = data_ptr[numR] - data_ptr[numC];
		}
	}
	unwrap_img = unwrap_map.clone();

	// 根据右减左来写代码

	for (int r = 0; r < neighborhoodCharacteristicsR.rows; r += 1)
	{
		double* neighborPtr = neighborhoodCharacteristicsR.ptr<double>(r);
		for (int c = 0; c < neighborhoodCharacteristicsR.cols; c += 1)
		{
			if (neighborPtr[c] < -1)
			{
				neighborPtr[c] = -1;
			}
			else if (neighborPtr[c] > 1)
			{
				neighborPtr[c] = 1;
			}
			neighborPtr[c] = neighborPtr[c] * (-0.5) + 0.5;
		}
	}

	cv::Mat kernal = (cv::Mat_<double>(1, 7) << 1, 1, 1, 1, 1, 1, 1);
	cv::dilate(neighborhoodCharacteristicsR, neighborhoodCharacteristicsR, kernal, cv::Point(0, 0), 1);

	for (int r = 0; r < neighborhoodCharacteristicsR.rows; r += 1)
	{
		double* neighborPtr = neighborhoodCharacteristicsR.ptr<double>(r);
		double* maskPtr = mask.ptr<double>(r);
		for (int c = 0; c < neighborhoodCharacteristicsR.cols; c += 1)
		{
			maskPtr[c] += neighborPtr[c] * (-2.89004663e-05);

		}
	}

	return true;
}


bool DF_Encode::unwrapVariableWavelengthPatternsOpenmp(std::vector<cv::Mat> wrap_img_list, std::vector<double> rate_list, cv::Mat& unwrap_img, cv::Mat& mask)
{

	if (wrap_img_list.empty())
	{
		return false;
	}
	if (3 != wrap_img_list.size())
	{
		return false;
	}

	if (2 != rate_list.size())
	{
		return false;
	}

	int nr = wrap_img_list[0].rows;
	int nc = wrap_img_list[0].cols;

	bool unwrap_filter = false;

	if (mask.data)
	{
		unwrap_filter = true;
	}

	cv::Mat h_unwrap_map(nr, nc, CV_64F, cv::Scalar(0));

	cv::Mat err_map_l(nr, nc, CV_64F, cv::Scalar(0));
	cv::Mat err_map_h(nr, nc, CV_64F, cv::Scalar(0));

#pragma omp parallel for
	for (int r = 0; r < nr; r++)
	{
		double* ptr_0 = wrap_img_list[0].ptr<double>(r);
		double* ptr_1 = wrap_img_list[1].ptr<double>(r);
		double* ptr_2 = wrap_img_list[2].ptr<double>(r);

		double* ptr_err_l = err_map_l.ptr<double>(r);
		double* ptr_err_h = err_map_h.ptr<double>(r);

		uchar* ptr_mask = mask.ptr<uchar>(r);

		double* ptr_h = h_unwrap_map.ptr<double>(r);

		for (int c = 0; c < nc; c++)
		{

			double temp = 0.5 + (rate_list[0] * ptr_0[c] - ptr_1[c]) / (CV_PI);
			int k = temp;
			ptr_h[c] = CV_PI * k + ptr_1[c];

			if (unwrap_filter)
			{
				double error = fabs(ptr_h[c] - ptr_0[c] * rate_list[0]);
				ptr_err_l[c] = error * 1;
				//backup 0.5
				if (error > 1.0)
				{
					ptr_h[c] = 0;
					ptr_mask[c] = 0;
				}
			}




			/******************************************************************/
			temp = 0.5 + (rate_list[1] * ptr_h[c] - ptr_2[c]) / (CV_PI);
			k = temp;

			double old_ptr_h = ptr_h[c];
			ptr_h[c] = CV_PI * k + ptr_2[c];

			if (unwrap_filter)
			{
				double error = fabs(ptr_h[c] - old_ptr_h * rate_list[1]);
				ptr_err_h[c] = error * 1;
				//backup 0.2
				if (error > 0.4)
				{
					ptr_h[c] = 0;
					ptr_mask[c] = 0;
				}
			}


			/********************************************************************************/
		}

	}

	unwrap_img = h_unwrap_map.clone();

	//unwrap_img = unwrap_img / 32;

	return true;
}



bool DF_Encode::computePhaseBaseSixStep(std::vector<cv::Mat> patterns, std::vector<cv::Mat>& wrap_maps, cv::Mat& mask_img, cv::Mat& confidence)
{
	std::vector<cv::Mat> wrap_img_list;
	std::vector<cv::Mat> confidence_map_list;
	std::vector<int> number_list;



#pragma omp parallel for
	for (int i = 0; i < patterns.size() - 1; i += 6)
	{
		cv::Mat wrap_img;
		cv::Mat confidence;

		std::vector<cv::Mat> phase_list(patterns.begin() + i, patterns.begin() + i + 6);
		sixStepPhaseShift(phase_list, wrap_img, mask_img, confidence);


#pragma omp critical
		{
			number_list.push_back(i / 6);
			wrap_img_list.push_back(wrap_img);
			confidence_map_list.push_back(confidence);
		}


	}


	std::vector<cv::Mat> sort_img_list;
	sort_img_list.resize(wrap_img_list.size());

	std::vector<cv::Mat> sort_confidencce_list;
	sort_confidencce_list.resize(confidence_map_list.size());

	for (int i = 0; i < wrap_img_list.size(); i++)
	{

		sort_img_list[number_list[i]] = wrap_img_list[i];
		sort_confidencce_list[number_list[i]] = confidence_map_list[i];
	}

	wrap_maps = sort_img_list;

	cv::Mat confid_map = sort_confidencce_list[0].clone();

	int nr = sort_confidencce_list[0].rows;
	int nc = sort_confidencce_list[0].cols;

	//for (int r = 0; r < nr; r++)
	//{

	//	double* ptr_0 = sort_confidencce_list[0].ptr<double>(r);
	//	double* ptr_1 = sort_confidencce_list[1].ptr<double>(r);
	//	double* ptr_2 = sort_confidencce_list[2].ptr<double>(r);

	//	double* ptr_confid = confid_map.ptr<double>(r);

	//	for (int c = 0; c < nc; c++)
	//	{

	//		double max_v = 0;
	//		if (ptr_0[c] > ptr_1[c])
	//		{
	//			max_v = ptr_0[c];
	//		}
	//		else
	//		{
	//			max_v = ptr_1[c];
	//		}
	//		if (max_v < ptr_2[c])
	//		{
	//			max_v = ptr_2[c];
	//		}
	//		ptr_confid[c] = max_v;

	//	}
	//}

	confidence = confid_map.clone();


	return true;
}

bool DF_Encode::computePhaseBaseFourStep(std::vector<cv::Mat> patterns, std::vector<cv::Mat>& wrap_maps, cv::Mat& mask_img, cv::Mat& confidence)
{

	std::vector<cv::Mat> wrap_img_list;
	std::vector<cv::Mat> confidence_map_list;
	std::vector<int> number_list;



#pragma omp parallel for
	for (int i = 0; i < patterns.size() - 1; i += 4)
	{
		cv::Mat wrap_img;
		cv::Mat confidence;

		std::vector<cv::Mat> phase_list(patterns.begin() + i, patterns.begin() + i + 4);
		fourStepPhaseShift(phase_list, wrap_img, mask_img, confidence);


#pragma omp critical
		{
			number_list.push_back(i / 4);
			wrap_img_list.push_back(wrap_img);
			confidence_map_list.push_back(confidence);
		}


	}


	std::vector<cv::Mat> sort_img_list;
	sort_img_list.resize(wrap_img_list.size());

	std::vector<cv::Mat> sort_confidencce_list;
	sort_confidencce_list.resize(confidence_map_list.size());

	for (int i = 0; i < wrap_img_list.size(); i++)
	{

		sort_img_list[number_list[i]] = wrap_img_list[i];
		sort_confidencce_list[number_list[i]] = confidence_map_list[i];
	}

	wrap_maps = sort_img_list;

	cv::Mat confid_map = sort_confidencce_list[0].clone();

	int nr = sort_confidencce_list[0].rows;
	int nc = sort_confidencce_list[0].cols;

	for (int r = 0; r < nr; r++)
	{

		uchar* ptr_0 = sort_confidencce_list[0].ptr<uchar>(r);
		uchar* ptr_1 = sort_confidencce_list[1].ptr<uchar>(r);
		uchar* ptr_2 = sort_confidencce_list[2].ptr<uchar>(r);

		uchar* ptr_confid = confid_map.ptr<uchar>(r);

		for (int c = 0; c < nc; c++)
		{
			uchar min_v = 255;
			if (ptr_0[c] < ptr_1[c])
			{
				min_v = ptr_0[c];
			}
			else
			{
				min_v = ptr_1[c];
			}
			if (min_v > ptr_2[c])
			{
				min_v = ptr_2[c];
			}
			ptr_confid[c] = min_v;

			//uchar max_v = 0;
			//if (ptr_0[c] > ptr_1[c])
			//{
			//	max_v = ptr_0[c];
			//}
			//else
			//{
			//	max_v = ptr_1[c];
			//}
			//if (max_v < ptr_2[c])
			//{
			//	max_v = ptr_2[c];
			//}
			//ptr_confid[c] = max_v;

		}
	}

	confidence = confid_map.clone();


	return true;
}


bool DF_Encode::maskMap(cv::Mat mask, cv::Mat& map)
{
	if (!mask.data)
	{
		return false;
	}

	if (!map.data)
	{
		return false;
	}


	if (CV_64FC3 == map.type())
	{
		for (int r = 0; r < map.rows; r++)
		{

			cv::Vec3d* ptr_map = map.ptr<cv::Vec3d>(r);
			uchar* ptr_mask = mask.ptr<uchar>(r);

			for (int c = 0; c < map.cols; c++)
			{
				if (0 == ptr_mask[c])
				{
					ptr_map[c][0] = 0;
					ptr_map[c][1] = 0;
					ptr_map[c][2] = 0;
				}
			}

		}
	}
	else if (CV_64FC1 == map.type())
	{
		for (int r = 0; r < map.rows; r++)
		{
			double* ptr_map = map.ptr<double>(r);
			uchar* ptr_mask = mask.ptr<uchar>(r);

			for (int c = 0; c < map.cols; c++)
			{
				if (0 == ptr_mask[c])
				{
					ptr_map[c] = 0;
				}
			}

		}
	}



	return true;
}

bool DF_Encode::selectMaskBaseConfidence(cv::Mat confidence, int threshold, cv::Mat& mask)
{
	if (!confidence.data)
	{
		return false;
	}

	int nr = confidence.rows;
	int nc = confidence.cols;

	//mask = cv::Mat(nr, nc, CV_8U, cv::Scalar(0));

	for (int r = 0; r < nr; r++)
	{
		double* ptr_c = confidence.ptr<double>(r);
		uchar* ptr_m = mask.ptr<uchar>(r);

		for (int c = 0; c < nc; c++)
		{
			if (ptr_c[c] < threshold)
			{
				ptr_m[c] = 0;
			}

		}
	}

	return true;

}
