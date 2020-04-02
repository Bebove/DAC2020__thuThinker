#include <stdio.h>
#include <ap_int.h>
#include <ap_fixed.h>

typedef ap_int<8> int8;
typedef ap_uint<9> uint9;
typedef ap_uint<20> uint20;
typedef ap_uint<16> uint16;
typedef ap_fixed<32, 1, AP_RND, AP_SAT> fix_32_1;
typedef ap_fixed<11, 4, AP_RND, AP_SAT> wt_type;//weight type
typedef ap_fixed<9,  3, AP_RND, AP_SAT> fm_type;//feature map data type
typedef ap_fixed<11, 4, AP_RND, AP_SAT> bs_type;//bias type
/*
fm_type fm_buf1[96*82*50];
fm_type fm_buf2[96*82*50];
wt_type wt_buf[480*80];
bs_type bias[480];

void load_bias();
void load_weight();
void load_ddr();
void upload_img();

void conv3x3();
void conv1x1();
void dw_conv_1();
void dw_conv_2();
*/


void load_img(fm_type img_buf[80][49][81], uint16 image_in_raw_pad_burst[3*(320+4)*(192+4)*4],
							int col, int row, int offset_h , int offset_w );
void SkyNet(	uint16 image_in_raw_pad[3*(320+4)*(192+4)*4]);
