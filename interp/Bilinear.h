#include <ap_fixed.h>
#include <ap_int.h>
#include <stdio.h>

typedef ap_uint<8> uint8;
typedef ap_uint<9> uint9;
//typedef int uint8;
//typedef int uint9;
//typedef float fix16;
//typedef float fix9;
typedef ap_fixed<16,10,AP_RND, AP_SAT> fix16;
typedef ap_ufixed<9,9,AP_RND, AP_SAT> fix9;
typedef ap_ufixed<8,1,AP_RND, AP_SAT> fix8;

#define high_dst 	160
#define width_dst 	320
#define high_src 	80 //rows
#define width_src 	160 //cols
#define param_h 	high_src/high_dst
#define param_w 	width_src/width_dst

void bilinear(
		uint8 fm_input[3][high_src][width_src],
		uint8 fm_output[3][high_dst][width_dst],
		fix8 weight[12][high_dst][width_dst]
	);
void nearest(
	uint8 fm_input[3][high_src][width_src],
	uint8 fm_output[3][high_dst][width_dst]
	);
