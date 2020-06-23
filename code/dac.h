#include <stdio.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <math.h>
#include <fstream>
#include <hls_math.h>
#include <string.h>


#include <cstddef>
#include <stdio.h>
#include <math.h>
#include "hls_stream.h"
#include <iostream>
#include <cmath>
using namespace std;
#define imagesize 749616//3*(320+2)*(192+2)*4;
#define ddrsize   65600//3*(320+2)*(192+2)*4;
#define ddrsize_dp 6
typedef ap_int<8> int8;
typedef ap_uint<2> uint2;
typedef ap_uint<5> uint5;
typedef ap_uint<4> uint4;
typedef ap_uint<6> uint6;
typedef ap_uint<9> uint9;
typedef ap_uint<20> uint20;
typedef ap_uint<16> uint16;
typedef ap_uint<512> uint512;
typedef ap_uint<256> uint256;
typedef ap_fixed<32, 1, AP_RND, AP_SAT> fix_32_1;

#define wt_lenth 13//
#define fm_lenth 13//
#define bs_lenth 13//

typedef ap_fixed<14, 6, AP_RND, AP_SAT> wt_type;//weight type
typedef ap_fixed<14, 6, AP_RND, AP_SAT> fm_type;//feature map data type
typedef ap_fixed<14, 6, AP_RND, AP_SAT> bs_type;//bias type


void Thinker(	float image_in_raw_pad[imagesize],
			    uint256	w_port_3x3[170][3][3],
				uint256     w_port_1x1[1000][16],
				uint256     bias_port[500],
				uint256		w_port_3x3_2[13*4][16][3][3],
				uint256 ddrdebug [ddrsize][ddrsize_dp],
				uint256 ddrdebug_2 [ddrsize][ddrsize_dp],
				uint256 ddrdebug_3 [ddrsize][ddrsize_dp],
				uint256 ddrdebug_4 [ddrsize][ddrsize_dp],
				uint256 temp1 [ddrsize][ddrsize_dp],
				float boxs[4][5]);


//function for dw3x3:
void load_dwweight_conv3x3(wt_type dest[16][3][3], uint256 src[3][3]);
void set_dwbias_conv3x3( fm_type buf[16][50][82], bs_type bias[16]);
void dw_conv_1(fm_type (&in_buf)[16][50][82],
		fm_type (&out_buf)[16][50][82],
		wt_type (&weight)[16][3][3],int relu);
void dw_conv_2(fm_type in_buf[16][50][82],
		fm_type out_buf[16][50][82],
		wt_type weight[16][3][3],int relu);

//function for 3x3:
void load_weight_conv3x3( wt_type dest[16][16][3][3], uint256 src[16][3][3]);
void conv3x3_502(fm_type in_buf[16][50][82],
		fm_type out_buf[16][50][82],
		wt_type weight[16][16][3][3]
		);
void clearpad_for_502(fm_type in_buf[16][50][82]);
void conv3x3_501(fm_type in_buf[16][50][82],
		fm_type out_buf[16][50][82],
		wt_type weight[16][16][3][3]
		);
void clearpad_for_501(fm_type in_buf[16][50][82]);
void clearpad_for_500(fm_type in_buf[16][50][82]);
void conv3x3_499(fm_type in_buf[16][50][82],
		fm_type out_buf[16][50][82],
		wt_type weight[16][16][3][3]
		);
void clearpad_for_499(fm_type in_buf[16][50][82]);
//function for 1x1:
void CONV_1x1(fm_type bottom[16][50][82],
			  fm_type top[16][50][82],
			  wt_type weights[16][16],int relu);
void load_weight_conv1x1( wt_type dest[16][16], uint256 src[16]);
void set_bias_conv1x1( fm_type buf[16][50][82], bs_type bias[16], int x, int y,int number_ofpart_for_one_image,bool pad);

void chear_pad(int x,int y, fm_type buff[80][50][82],int round);

//global
void load_bias_from_axi(bs_type dest[16], uint256 src);

void aload_img(fm_type img_buf[16][50][82], float image_port[imagesize],
							int col, int row, int offset_h , int offset_w ,
							int channel,int channel_offset,
							int all_image_w,
							int all_image_h,
							int buffer_w,
							int buffer_h);
void aload_img_2(fm_type img_buf[16][50][82], uint256 image_port[ddrsize][ddrsize_dp],   //use 30 uint256 to store  480 channel
							int ddr_channelX16_index,
							int offsetw,
							int offseth,

							int w,    //80
							int h,    //48
							int allw
							);

void deload_img(fm_type img_buf[16][50][82], uint256 image_port[ddrsize][ddrsize_dp],   //use 30 uint256 to store  480 channel
							int ddr_channelX16_index,
							int offsetw,
							int offseth,

							int w,    //80
							int h,    //48
							int allw
							);
void initial_ddr(uint256 image_port[ddrsize][ddrsize_dp],
							int ddr_channelX16_index,
							int allw,
							int allh
							);
void initial_ddr6(uint256 image_port[ddrsize][ddrsize_dp],
		int ddr_channelX16_index,
		int allw,
		int allh
		);
void clear_pad(fm_type buf[16][50][82],int w, int h);
void clear_pad2(fm_type buf[16][50][82],int w, int h);
void clear_pad3(fm_type buf[16][50][82],int w, int h);
void bilinear_1(
		fm_type fm_input[16][50][82],
		fm_type fm_output[16][50][82]
	);
void bilinear_2(
		fm_type fm_input[16][50][82],
		fm_type fm_output[16][50][82]
	);
void bilinear_3(
		fm_type fm_input[16][50][82],
		fm_type fm_output[16][50][82]
	);
void load_oneimageto_ddr_1 (
			fm_type fm_buf1[16][50][82],int choose,fm_type fm_buf2[16][50][82]
		  );
void load_oneimageto_ddr_2 (
			fm_type fm_buf1[16][50][82],int choose,fm_type fm_buf2[16][50][82]
		  );
////////////////////////
void findmax1(fm_type img_buf[16][50][82],
			 int  hwc_img1[3],
			 int  hwc_img2[3],
			 int  hwc_img3[3],
			 int  hwc_img4[3]);//�??小，�??下找4张图
void findbox1(fm_type img_buf[16][50][82],
			fm_type img_buf2[16][50][82],
			int imgmax_index[3],
			int which,
			float box[5]);			 
void findmax2(fm_type img_buf[16][50][82],
			 int  hwc_img1[3],
			 int  hwc_img2[3],
			 int  hwc_img3[3],
			 int  hwc_img4[3]);
void findmax3(fm_type img_buf[16][50][82],
			  int hwc_img1[3]);		
void findmax4(fm_type img_buf[16][50][82],
			  int hwc_img1[3]);		
int findmax(float score[4]);  	 			