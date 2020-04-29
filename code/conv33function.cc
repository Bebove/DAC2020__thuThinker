#include "dac.h"
#include <stdio.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <math.h>
#include <fstream>
#include <hls_math.h>
#include <string.h>
using namespace std;

//fm_type fm_buf_for33[4][8][49][81];
void conv3x3(fm_type in_buf[80][49][81],
		fm_type out_buf[8][49][81],
		wt_type weight[8][32][3][3]
		){
#pragma HLS array_partition variable=in_buf dim=1 complete
#pragma HLS array_partition variable=out_buf dim=1 complete
#pragma HLS array_partition variable=weight dim=1 complete
#pragma HLS array_partition variable=weight dim=2 complete


    for(int y=0;y<3;y++){
        for (int x=0;x<3;x++){
            for(int h=0,hi=0; h<24; h++,hi+=2){
                for(int w=0,wi=0; w<40; w++,wi+=2){
#pragma HLS PIPELINE
                    for(int cho=0; cho<8; cho++){
                        for(int chi=0;chi<32;chi++){
                            out_buf[cho][h][w]+=weight[cho][chi][y][x]*in_buf[chi][hi+y][wi+x];
                        }
                    }
                }
            }
        }
    }
}


void set_bias_conv3x3( fm_type buf[4][8][49][81], bs_type bias[80])
{
#pragma HLS array_partition variable=buf dim=1 complete
//#pragma HLS array_partition variable=bias dim=1 complete
	for(int h = 1; h <= 49; h+=1) {
		for(int w = 1; w <= 81; w++) {
#pragma HLS pipeline
			for(int c = 0; c < 4; c++) {
				for(int cd = 0; cd < 8; cd++) {
#pragma HLS unroll
				buf[c][cd][h  ][w] = bias[c*8+cd];
				}
			}
		}
	}
}

void load_weight_conv3x3( wt_type dest[4][8][32][3][3], uint512 src[500][3][3],int ofset)
{
	//this function should be able to load 32*32*3*3 weight, so the src port should have the size of 3x3x32
	for(int m = 0; m < 3; m++)
	{
		for(int n = 0; n < 3; n++)
		{
			for(int co = 0; co < 4; co++)
			{
				for(int coindex = 0; coindex < 8; coindex++)
				{
#pragma HLS pipeline
				uint512 DATA = 0;

				DATA.range(511, 0) = src[co*8+coindex+	ofset	][m][n].range(511, 0);        //the ofset function in there
				for(int ci = 0; ci < 32; ci++)
				{
#pragma HLS unroll
					dest[co][coindex][ci][m][n].range(10, 0) = DATA.range(10 + ci*16, ci*16);  //this means for 3x3 conv, every index(0-500) contains 32 3x3 weight
				}
				}

			}
		}
	}
}
