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
#define ddrsize   150000//3*(320+2)*(192+2)*4;
#define wt_lenth 10//
#define fm_lenth 8//

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
typedef ap_fixed<11, 3, AP_RND, AP_SAT> wt_type;//weight type
typedef ap_fixed<9,  3, AP_RND, AP_SAT> fm_type;//feature map data type
typedef ap_fixed<11, 3, AP_RND, AP_SAT> bs_type;//bias type


void Thinker(	uint16 image_in_raw_pad[imagesize],
			    uint512	w_port_3x3[500][3][3],
				uint256     w_port_1x1[500][16],
				uint256     bias_port[500][5],
				uint256 ddrdebug [ddrsize][30],
				uint256 ddrdebug_2 [ddrsize][30],
				uint16 debug[2]);


//function for dw3x3:
void load_dwweight_conv3x3(wt_type dest[96][3][3], uint512 src[500][3][3],int ofset);
void set_dwbias_conv3x3( fm_type buf[80][50][82], bs_type bias[80]);
void dw_conv_1(fm_type (&in_buf)[80][50][82],
		fm_type (&out_buf)[80][50][82],
		wt_type (&weight)[96][3][3],int wise,int relu);
void dw_conv_2(fm_type (&in_buf)[80][50][82],
		fm_type (&out_buf)[80][50][82],
		wt_type (&weight)[96][3][3],int wise,int relu);

//function for 3x3:
void load_weight_conv3x3( wt_type dest[4][8][32][3][3], uint512 src[500][3][3],int ofset);
void set_bias_conv3x3( fm_type buf[4][8][49][81], bs_type bias[80]);
void conv3x3(fm_type in_buf[80][49][81],
		fm_type out_buf[8][49][81],
		wt_type weight[8][32][3][3]
		);


//function for 1x1:
void CONV_1x1(fm_type bottom[80][50][82],
			  fm_type top[80][50][82],
			  wt_type weights[16][16],int to,int ti,int relu);
void load_weight_conv1x1( wt_type dest[16][16], uint256 src[16]);
void set_bias_conv1x1( fm_type buf[80][50][82], bs_type bias[80], int x, int y,int number_ofpart_for_one_image,bool pad);

void chear_pad(int x,int y, fm_type buff[80][50][82],int round);

//global
void load_bias_from_axi(bs_type dest[80], uint256 src[5]);

void aload_img(fm_type img_buf[80][50][82], uint16 image_port[imagesize],
							int col, int row, int offset_h , int offset_w ,
							int channel,int channel_offset,
							int all_image_w,
							int all_image_h,
							int buffer_w,
							int buffer_h);
void aload_img_2(fm_type img_buf[80][50][82], uint256 image_port[ddrsize][30],   //use 30 uint256 to store  480 channel
							int howmany256,
							int offsetw,
							int offseth,

							int w,    //80
							int h,    //48
							int allw
							);

void deload_img(fm_type img_buf[80][50][82], uint256 image_port[ddrsize][30],   //use 30 uint256 to store  480 channel
							int howmany256,
							int offsetw,
							int offseth,

							int w,
							int h,
							int allw
							);
