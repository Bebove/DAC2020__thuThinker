
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
typedef cv::Vec<char, 3> Ve3us;




float dw1_conv_3x3_weight[3][3][3];
float dw1_conv_3x3_bias[3];



float val_dw1_conv_3x3_weight[3][3][3];
float val_dw1_conv_3x3_bias[3];
void load_weights()
{
	std::ifstream ifs_param("F:\\FPGA\\SkyNet-master\\FPGA\\HLS\\weights_floating.bin", std::ios::in | std::ios::binary);
	ifs_param.read((char*)(**dw1_conv_3x3_weight), 3 * 3 * 3 * sizeof(float));
	ifs_param.read((char*)dw1_conv_3x3_bias, 3 * sizeof(float));
	ifs_param.close();


}
void write_weight()
{
	std::cout << "The weight to store" << std::endl;
	std::cout << dw1_conv_3x3_weight[1][1][1] << std::endl;
	std::cout << dw1_conv_3x3_bias[1] << std::endl;
	std::ofstream cp_ifs_param("F:\\FPGA\\SkyNet-master\\FPGA\\HLS\\test\\weights_floating_cp.bin", std::ios::out | std::ios::binary);
	cp_ifs_param.write((char*)(dw1_conv_3x3_weight), 3 * 3 * 3 * sizeof(float));
	cp_ifs_param.write((char*)dw1_conv_3x3_bias, 3 * sizeof(float));
	cp_ifs_param.close();
}
void val_weight_data()
{
	std::ifstream val_ifs_param("F:\\FPGA\\SkyNet-master\\FPGA\\HLS\\test\\weights_floating_cp.bin", std::ios::in | std::ios::binary);
	val_ifs_param.read((char*)(**val_dw1_conv_3x3_weight), 3 * 3 * 3 * sizeof(float));
	val_ifs_param.read((char*)val_dw1_conv_3x3_bias, 3 * sizeof(float));
	val_ifs_param.close();
	std::cout << "" << std::endl;
	std::cout << "val_weight_store" << std::endl;
	std::cout << dw1_conv_3x3_weight[1][1][1] << std::endl;
	std::cout << val_dw1_conv_3x3_bias[1] << std::endl;
}




typedef float uint8;

float dw1_conv_1x1_weight[48][3];
float dw1_conv_1x1_bias[48];
float dw1_conv_1x1_output[48][160][320];

float dw1_max_pool_output[48][80][160];

float dw2_conv_3x3_weight[48][3][3];
float dw2_conv_3x3_bias[48];
float dw2_conv_3x3_output[48][80][160];

float dw2_conv_1x1_weight[96][48];
float dw2_conv_1x1_bias[96];
float dw2_conv_1x1_output[96][80][160];

float dw2_max_pool_output[96][40][80];

float dw3_conv_3x3_weight[96][3][3];
float dw3_conv_3x3_bias[96];
float dw3_conv_3x3_output[96][40][80];

float dw3_conv_1x1_weight[192][96];
float dw3_conv_1x1_bias[192];
float dw3_conv_1x1_output[192][40][80];

float dw3_max_pool_output[192][20][40];

float dw4_conv_3x3_weight[192][3][3];
float dw4_conv_3x3_bias[192];
float dw4_conv_3x3_output[192][20][40];

float dw4_conv_1x1_weight[384][192];
float dw4_conv_1x1_bias[384];
float dw4_conv_1x1_output[384][20][40];

float dw5_conv_3x3_weight[384][3][3];
float dw5_conv_3x3_bias[384];
float dw5_conv_3x3_output[384][20][40];

float dw5_conv_1x1_weight[512][384];
float dw5_conv_1x1_bias[512];
float dw5_conv_1x1_output[512][20][40];

// cat dw3(ch:192 -> 768) and dw5(ch:512)
float dw6_conv_3x3_input[1280][20][40];
float dw6_conv_3x3_weight[1280][3][3];
float dw6_conv_3x3_bias[1280];
float dw6_conv_3x3_output[1280][20][40];

float dw6_conv_1x1_weight[96][1280];
float dw6_conv_1x1_bias[96];
float dw6_conv_1x1_output[96][20][40];

float pw7_conv_1x1_weight[10][96];
float pw7_conv_1x1_output[10][20][40];


// reordered weights for the mysterious dw3(192->768)
float dw6_conv_3x3_weight_reo[1280][3][3];
float dw6_conv_3x3_bias_reo[1280];
float dw6_conv_3x3_input_reo[1280][20][40];
float dw6_conv_3x3_output_reo[1280][20][40];

float dw6_conv_1x1_weight_reo[96][1280];
float dw6_conv_1x1_bias_reo[96];
float dw6_conv_1x1_output_reo[96][20][40];

char   image_raw_g_burst[3 * 162 * 2 * 322 * 2];
char   r_image_raw_g_burst[162 * 2 * 322 * 2];
char   g_image_raw_g_burst[162 * 2 * 322 * 2];
char   b_image_raw_g_burst[162 * 2 * 322 * 2];


char   rr_image_raw_g_burst[322 * 2][162 * 2];
char   rg_image_raw_g_burst[322 * 2][162 * 2];
char   rb_image_raw_g_burst[322 * 2][162 * 2];
void load_img(const char* img_g)
{
	
	std::ifstream ifs_image_raw_g(img_g, std::ios::in | std::ios::binary);
	ifs_image_raw_g.read(image_raw_g_burst, 3 * 162 * 2 * 322 * 2 * sizeof(uint8));
	int i;
	i = 0;
	while(i!=208656)
	{
		r_image_raw_g_burst[i] = image_raw_g_burst[i];
		i = i + 1;
	}
	while (i != 2*208656)
	{
		g_image_raw_g_burst[i%208656] = image_raw_g_burst[i];
		i = i + 1;
	}
	while (i != 3*208656)
	{
		b_image_raw_g_burst[i % 208656] = image_raw_g_burst[i];
		i = i + 1;
	}










	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int j;
	i = -1;
	j = 0;
	while (j!= 208656) {
		if (j % (322 * 2) == 0)
		{
			i = i + 1;
		}
		rr_image_raw_g_burst[j % (322 * 2)][i] = r_image_raw_g_burst[j];
		j = j + 1;

	}
	i = -1;
	j = 0;
	while (j != 208656) {
		if (j % (322 * 2) == 0)
		{
			i = i + 1;
		}
		rg_image_raw_g_burst[j % (322 * 2)][i] = g_image_raw_g_burst[j];
		j = j + 1;

	}
	i = -1;
	j = 0;
	while (j != 208656) {
		if (j % (322 * 2) == 0)
		{
			i = i + 1;
		}
		rb_image_raw_g_burst[j % (322 * 2)][i] =b_image_raw_g_burst[j];
		j = j + 1;

	}
	

















	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	cv::Mat grHistogram(322 * 2, 162 * 2, CV_8SC3, cv::Scalar('?','i', '?'));
	//cv::namedWindow("3", cv::WINDOW_AUTOSIZE);
	
	//cv::imshow("3", grHistogram);
	//std::cout << grHistogram.at<Ve3us>(1, 2)[1] << std::endl;
	i = 0;
	while (i != 322 * 2) {
		j = 0;
		while (j != 162 * 2)
		{
			grHistogram.at<Ve3us>(i, j)[2] = rr_image_raw_g_burst[i][j];
			j=j+1;
		}
		i = i + 1;
	}

	i = 0;
	while (i != 322 * 2) {
		j = 0;
		while (j != 162 * 2)
		{
			grHistogram.at<Ve3us>(i, j)[0] = rb_image_raw_g_burst[i][j];
			j = j + 1;
		}
		i = i + 1;
	}

	i = 0;
	while (i != 322 * 2) {
		j = 0;
		while (j != 162 * 2)
		{
			grHistogram.at<Ve3us>(i, j)[1] = rg_image_raw_g_burst[i][j];
			//std::cout << grHistogram.at<Ve3us>(i, j)[2] << std::endl;
			j = j + 1;
		}
		i = i + 1;
	}
	//cv::namedWindow("3", cv::WINDOW_AUTOSIZE);
	//cv::imshow("3", grHistogram);
	//std::cout << grHistogram.at<Ve3us>(1, 2)[1] << std::endl;
	cv::imwrite("9.PNG", grHistogram);
}
void write_img()
{

}
void val_img_data()
{

}



void load_full_weight()
{
	std::ifstream ifs_param("weights_floating.bin", std::ios::in | std::ios::binary);

	///////////// Read Weights ///////////////////////
	ifs_param.read((char*)(**dw1_conv_3x3_weight), 3 * 3 * 3 * sizeof(float));
	ifs_param.read((char*)dw1_conv_3x3_bias, 3 * sizeof(float));
	ifs_param.read((char*)(*dw1_conv_1x1_weight), 48 * 3 * sizeof(float));
	ifs_param.read((char*)dw1_conv_1x1_bias, 48 * sizeof(float));
	ifs_param.read((char*)(**dw2_conv_3x3_weight), 48 * 3 * 3 * sizeof(float));
	ifs_param.read((char*)dw2_conv_3x3_bias, 48 * sizeof(float));
	ifs_param.read((char*)(*dw2_conv_1x1_weight), 96 * 48 * sizeof(float));
	ifs_param.read((char*)dw2_conv_1x1_bias, 96 * sizeof(float));
	ifs_param.read((char*)(**dw3_conv_3x3_weight), 96 * 3 * 3 * sizeof(float));
	ifs_param.read((char*)dw3_conv_3x3_bias, 96 * sizeof(float));
	ifs_param.read((char*)(*dw3_conv_1x1_weight), 192 * 96 * sizeof(float));
	ifs_param.read((char*)dw3_conv_1x1_bias, 192 * sizeof(float));
	ifs_param.read((char*)(**dw4_conv_3x3_weight), 192 * 3 * 3 * sizeof(float));
	ifs_param.read((char*)dw4_conv_3x3_bias, 192 * sizeof(float));
	ifs_param.read((char*)(*dw4_conv_1x1_weight), 384 * 192 * sizeof(float));
	ifs_param.read((char*)dw4_conv_1x1_bias, 384 * sizeof(float));
	ifs_param.read((char*)(*dw5_conv_3x3_weight), 384 * 3 * 3 * sizeof(float));
	ifs_param.read((char*)dw5_conv_3x3_bias, 384 * sizeof(float));
	ifs_param.read((char*)(*dw5_conv_1x1_weight), 512 * 384 * sizeof(float));
	ifs_param.read((char*)dw5_conv_1x1_bias, 512 * sizeof(float));
	ifs_param.read((char*)(*dw6_conv_3x3_weight), 1280 * 3 * 3 * sizeof(float));
	ifs_param.read((char*)dw6_conv_3x3_bias, 1280 * sizeof(float));
	ifs_param.read((char*)(*dw6_conv_1x1_weight), 96 * 1280 * sizeof(float));
	ifs_param.read((char*)dw6_conv_1x1_bias, 96 * sizeof(float));
	ifs_param.read((char*)(*pw7_conv_1x1_weight), 10 * 96 * sizeof(float));
	ifs_param.close();
}

float image[3][160][320];
char  image_raw1[3 ][ 160 ][ 320];
void test_golden(const char* img0) {
	
	std::ifstream ifs_image_raw_g1(img0, std::ios::in | std::ios::binary);
	ifs_image_raw_g1.read(**image_raw1, 3 * 160 * 320 * sizeof(uint8));
	///////////////// IMAGE NORM ///////////////////
	



	cv::Mat grHistogram(320, 160, CV_8UC3, cv::Scalar('?', 'i', '?'));
	int i = 0;
	int j = 0;
	while (i != 320) {
		j = 0;
		while (j != 160)
		{
			grHistogram.at<cv::Vec3b>(i, j)[0] = image_raw1[2][j][i];
			j = j + 1;
		}
		i = i + 1;
	}

	i = 0;
	while (i != 320) {
		j = 0;
		while (j != 160)
		{
			grHistogram.at<cv::Vec3b>(i, j)[1] = image_raw1[1][j][i];
			j = j + 1;
		}
		i = i + 1;
	}

	i = 0;
	while (i != 320) {
		j = 0;
		while (j != 160)
		{
			grHistogram.at<cv::Vec3b>(i, j)[2] = image_raw1[0][j][i];
			//std::cout << grHistogram.at<Ve3us>(i, j)[2] << std::endl;
			j = j + 1;
		}
		i = i + 1;
	}
	//cv::namedWindow("3", cv::WINDOW_AUTOSIZE);
	//cv::imshow("3", grHistogram);
	//std::cout << grHistogram.at<Ve3us>(1, 2)[1] << std::endl;
	cv::imwrite("17.PNG", grHistogram);
}

int main()
{
	//load_weights();
	//write_weight();
	//val_weight_data();
	//


	//
	//load_img("F:\\FPGA\\SkyNet-master\\FPGA\\HLS\\test_image_bins\\stitched_0_3.bin");
	//write_img();
	//val_img_data();

	/*
	load_full_weight();*/
	test_golden("F:\\FPGA\\SkyNet-master\\FPGA\\HLS\\test_image_bins\\2.bin");
}


