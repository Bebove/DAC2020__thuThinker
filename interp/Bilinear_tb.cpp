#include <stdio.h>
#include "Bilinear.h"
#include <hls_opencv.h>
#include <iostream>
#include "hls_video.h"
#include <fstream>
#define input_image "F:/hls/Interpolation/image/input.jpg"
#define src_image "F:/hls/Interpolation/image/src.jpg"
#define debug_image "F:/hls/Interpolation/image/debug.jpg"
#define output_image "F:/hls/Interpolation/image/out.jpg"

using namespace std;
#define weight_file "F:/hls/Interpolation/image/weight.txt"
using namespace cv;
using namespace std;
//IplImage *src= cvLoadImage(input_image);

//cvCreateImage();
//CvSize s;

fix8 weight[12][high_dst][width_dst];
uint8 fm_input[3][high_src][width_src];
uint8 fm_output[3][high_dst][width_dst];

Mat src_size(){
	Mat srcimg=cv::imread(src_image,IMREAD_COLOR);
	Mat img;
	resize(srcimg, img, cv::Size(width_src,high_src));
	return img;
}

void save_img(uint8 fm_img[3][high_dst][width_dst]){
	cv::Mat img_dst(high_dst ,width_dst, CV_8UC3);
	for(int ch=0;ch<3;ch++){
		for(int h=0;h<high_dst;h++){
			for(int w=0;w<width_dst;w++){
				img_dst.at<Vec3b>(h,w)[ch]=(unsigned char)fm_img[ch][h][w];
			}
		}
	}
	cv::imwrite(output_image, img_dst);
}

void load_weight(){
	ifstream file(weight_file);
	for(int ch=0;ch<12;ch++){
		for(int h=0;h<high_dst;h++){
			for(int w=0;w<width_dst;w++){
				file>>weight[ch][h][w];
			}
		}
	}
	file.close();
}

int main(){

	//Mat img=src_size();
	Mat img=cv::imread(input_image,IMREAD_COLOR);
	Vec3i *img_ptr;
	unsigned char* rgb;

		for(int ch=0;ch<3;ch++){
			for(int h=0;h<img.rows;h++){
				for(int w=0;w<img.cols;w++){
					fm_input[ch][h][w]=img.at<Vec3b>(h,w)[ch];
				}
			}
		}

	//load_weight();
	//bilinear(fm_input,fm_output,weight);
	nearest(fm_input,fm_output);
	save_img(fm_output);

	return 0;
}
