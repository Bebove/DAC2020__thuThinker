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
*/
void load_bias(fm_type out_buf[96][82][50],
		bs_type bias[480],
		uint9 w_o, uint9 h_o,uint9 ch_o
		);

void load_img();

void conv3x3(fm_type in_buf[96*82*50],
		fm_type out_buf[96*82*50],
		wt_type weight[480*80],
		uint9 w_i, uint9 h_i ,uint9 ch_i, //input feature map scale
		uint9 w_o, uint9 h_o ,uint9 ch_o  //output feature map scale
		);
