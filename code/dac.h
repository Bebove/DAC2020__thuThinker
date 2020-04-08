#include <stdio.h>
#include <ap_int.h>
#include <ap_fixed.h>
#define imagesize 749616//3*(320+2)*(192+2)*4;



typedef ap_int<8> int8;
typedef ap_uint<2> uint2;
typedef ap_uint<5> uint5;
typedef ap_uint<4> uint4;
typedef ap_uint<6> uint6;
typedef ap_uint<9> uint9;
typedef ap_uint<20> uint20;
typedef ap_uint<16> uint16;
typedef ap_fixed<32, 1, AP_RND, AP_SAT> fix_32_1;
typedef ap_fixed<11, 4, AP_RND, AP_SAT> wt_type;//weight type
typedef ap_fixed<9,  3, AP_RND, AP_SAT> fm_type;//feature map data type
typedef ap_fixed<11, 4, AP_RND, AP_SAT> bs_type;//bias type
/*
fm_type fm_buf1[80][49][81];
fm_type fm_buf2[80][49][81];
fm_type fm_buf3[80][49][81];
wt_type wt_buf3[80][80][3][3];
wt_type wt_buf1[80][80];
bs_type bias[80];

void load_bias();
void load_weight();
void load_ddr();
void upload_img();
*/


void load_img(fm_type img_buf[80][49][81], uint16 image_in_raw_pad_burst[imagesize],
							int col, int row, int offset_h , int offset_w );
void SkyNet(	uint16 image_in_raw_pad[imagesize]);

void conv3x3(fm_type (&in_buf)[80][49][81],
		fm_type (&out_buf)[80][49][81],
		wt_type (&weight)[32][32][3][3]
		);
void conv1x1(fm_type (&in_buf)[80][49][81],
		fm_type (&out_buf)[80][49][81],
		wt_type (&weight)[16][16],
		uint4 to, uint4 ti);
void dw_conv_1(fm_type (&in_buf)[80][49][81],
		fm_type (&out_buf)[80][49][81],
		wt_type (&weight)[80][3][3]);
void dw_conv_2(fm_type (&in_buf)[80][49][81],
		fm_type (&out_buf)[80][49][81],
		wt_type (&weight)[80][3][3]);