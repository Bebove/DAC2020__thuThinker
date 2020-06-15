#include "dac.h"
#include <stdio.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <math.h>
#include <fstream>
#include <hls_math.h>
#include <string.h>
using namespace std;


inline fm_type relu_single( fm_type d ,int relu) {
	if(relu ==1){
	if( d > 6 )
		return 6;
	if( d < 0 )
		return 0;}
	return d;
}







void dw_conv_2(fm_type in_buf[16][50][82],
		fm_type out_buf[16][50][82],
		wt_type weight[16][3][3],int relu)
{
#pragma HLS array_partition variable=in_buf dim=1 complete
#pragma HLS array_partition variable=out_buf dim=1 complete
#pragma HLS array_partition variable=weight dim=1 complete
	fm_type temp;
            for(int h=0,hi=0; h<24; h++,hi+=2){
                for(int w=0,wi=0; w<40; w++,wi+=2){
#pragma HLS pipeline
                    for(int ch=0; ch<16; ch++){
                    	temp=0;
                        for(int y=0;y<3;y++){
                            for (int x=0;x<3;x++){
                            	temp+=weight[ch][y][x]*relu_single(in_buf[ch][hi+y][wi+x],relu);
                    }
                } out_buf[ch][h][w]+=temp;
            }
        }
    }
}



void dw_conv_1(fm_type (&in_buf)[16][50][82],
		fm_type (&out_buf)[16][50][82],
		wt_type (&weight)[16][3][3],int relu){
#pragma HLS array_partition variable=in_buf dim=1 complete
#pragma HLS array_partition variable=out_buf dim=1 complete
#pragma HLS array_partition variable=weight dim=1 complete
	fm_type temp;
            for(int h=0; h<48; h++){
                for(int w=0; w<80; w++){
#pragma HLS PIPELINE
                    for(int ch=0; ch<16; ch++){
                    	temp=0;
                        for(int y=0;y<3;y++){
                            for (int x=0;x<3;x++){

                            	temp+=weight[ch][y][x]*relu_single(in_buf[ch][h+y][w+x],relu);
                    }
                }
                        out_buf[ch][h][w]+=temp;
                    }
        }
    }
}


void load_dwweight_conv3x3(wt_type dest[16][3][3], uint256 src[3][3])
{

	for(int m = 0; m < 3; m++)
	{
		for(int n = 0; n < 3; n++)
		{
#pragma HLS pipeline

				for(int ci = 0; ci < 16; ci++)
				{
#pragma HLS unroll

					dest[ci][m][n].range(wt_lenth, 0) =src[m][n].range(wt_lenth + ci*16, ci*16);
				}

		}
	}

}



void set_dwbias_conv3x3( fm_type buf[16][50][82], bs_type bias[16])
{
#pragma HLS array_partition variable=buf dim=1 complete
#pragma HLS array_partition variable=bias dim=1 complete

	for(int h = 0; h < 50; h+=1) {
		for(int w = 0; w < 82; w++) {
#pragma HLS pipeline
			for(int c = 0; c < 16; c++) {
#pragma HLS unroll
				buf[c][h  ][w] = bias[c];
			}
		}
	}
}
