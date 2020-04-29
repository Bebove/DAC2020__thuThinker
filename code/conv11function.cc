#include "dac.h"
#include <stdio.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <math.h>
#include <fstream>
#include <hls_math.h>
#include <string.h>
using namespace std;

void conv1x1(fm_type (&in_buf)[80][49][81],
		fm_type (&out_buf)[80][49][81],
		wt_type (&weight)[16][16],
		uint4 to, uint4 ti){
    for(int h=0,hi=0; h<49; h++){
        for(int w=0,wi=0; w<81; w++){
#pragma HLS PIPELINE
            for(int cho=0; cho<16; cho++){
                for(int chi=0;chi<16;chi++){
                    out_buf[cho+to][h][w]+=weight[cho][chi]*in_buf[chi+ti][h][w];
                    //out_buf[cho][h][w]+=weight[cho][chi]*in_buf[chi][h][w];
                    //cout<<"cho="<<cho<<" chi="<<chi<<" hi="<<hi<<" wi="<<wi<<"\n";
					//cout<<weight[cho][chi]<<"  "<<in_buf[chi][hi][wi]<<"\n";
                    //cout<<out_buf[cho][h][w]<<"\n";
                    //system("pause");
                }
            }
        }
    }
}


void set_bias_conv1x1( fm_type buf[80][49][81], bs_type bias[80])
{
#pragma HLS array_partition variable=buf dim=1 complete
//#pragma HLS array_partition variable=bias dim=1 complete
	for(int h = 1; h <= 49; h+=1) {
		for(int w = 1; w <= 81; w++) {
#pragma HLS pipeline
			for(int c = 0; c < 80; c++) {
#pragma HLS unroll
				buf[c][h  ][w] = bias[c];
			}
		}
	}
}




void load_weight_conv1x1( wt_type dest[16][16], uint256 src[16])
{
	for(int co = 0; co < 16; co++)
	{
#pragma HLS pipeline
		uint256 DATA = 0;
		DATA.range(255, 0) = src[co].range(255, 0);
		for(int ci = 0; ci < 16; ci++)
		{
#pragma HLS unroll
			dest[co][ci].range(10, 0) = DATA.range(10 + ci*16, ci*16);
		}

	}
}
