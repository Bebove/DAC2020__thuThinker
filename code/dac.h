#include <stdio.h>
#include <ap_int.h>
#include <ap_fixed.h>

typedef ap_int<8> int8;
typedef ap_uint<9> uint9;
typedef ap_fixed<32, 1, AP_RND, AP_SAT> fix_32_1;
typedef int bs_type;//bias
typedef fix_32_1 scale_type;//scale type
typedef int8 wt_type;//weight
typedef int8 fm_type;//feature map data type
typedef int fms_type;//feature map for scale type
/*
fm_type fm_buf[96][82][50];
fms_type fms_buf[96][82][50];
wt_type wt_buf[480*80];
scale_type scale[480];
bs_type bias[480];
*/
void conv_pre(fms_type out_buf[96][82][50],
		bs_type bias[480],
		uint9 w, uint9 h ,uint9 ch
		);
/*
void conv(fm_type in_buf[96][82][50],
		fms_type out_buf[96][82][50],
		uint9 w, uint9 h ,uint9 ch
		);
*/
void Conv(ap_fixed<32,16, AP_RND, AP_SAT> re[2],
		ap_uint<16> c[2],

		ap_uint<8> a[2],
		ap_uint<8> b[2],
		ap_fixed<32,16, AP_RND, AP_SAT> h[2]);
