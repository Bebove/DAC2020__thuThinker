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







void dw_conv_2(fm_type (&in_buf)[80][50][82],
		fm_type (&out_buf)[80][50][82],
		wt_type (&weight)[96][3][3],int wise,int relu)
{
#pragma HLS array_partition variable=in_buf dim=1 complete
#pragma HLS array_partition variable=out_buf dim=1 complete
#pragma HLS array_partition variable=weight dim=1 complete
    for(int y=0;y<3;y++){
        for (int x=0;x<3;x++){
            for(int h=0,hi=0; h<24; h++,hi+=2){
                for(int w=0,wi=0; w<40; w++,wi+=2){
#pragma HLS pipeline
                    for(int ch=0; ch<wise; ch++){
#pragma HLS unroll
                        out_buf[ch][h][w]+=weight[ch][y][x]*relu_single(in_buf[ch][hi+y][wi+x],relu);
                    }
                }
            }
        }
    }
}


void dw_conv_1(fm_type (&in_buf)[80][50][82],
		fm_type (&out_buf)[80][50][82],
		wt_type (&weight)[96][3][3],int wise,int relu){
#pragma HLS array_partition variable=in_buf dim=1 complete
#pragma HLS array_partition variable=out_buf dim=1 complete
#pragma HLS array_partition variable=weight dim=1 complete
    for(int y=0;y<3;y++){
        for (int x=0;x<3;x++){
            for(int h=0; h<48; h++){
                for(int w=0; w<80; w++){
#pragma HLS PIPELINE
                    for(int ch=0; ch<wise; ch++){
#pragma HLS unroll
                        out_buf[ch][h][w]+=weight[ch][y][x]*relu_single(in_buf[ch][h+y][w+x],relu);
                    }
                }
            }
        }
    }
}


void load_dwweight_conv3x3(wt_type dest[96][3][3], uint512 src[500][3][3],int ofset)
{
//should be able to load 80x3x3 weight. so, we need 3 index to store one layer. The redundancy should be 0.
	uint512 DATA = 0;
	for(int co = 0; co < 3; co++)
	{
	for(int m = 0; m < 3; m++)
	{
		for(int n = 0; n < 3; n++)
		{
#pragma HLS pipeline
				//DATA.range(511, 0) = src[co+	ofset	][m][n].range(511, 0);        //the ofset function in there
				for(int ci = 0; ci < 32; ci++)
				{
#pragma HLS unroll
					//dest[ci+32*co][m][n].range(10, 0) = DATA.range(10 + ci*16, ci*16);  //this means for 3x3 conv, every index(0-500) contains 32 3x3 weight
					dest[ci+32*co][m][n].range(10, 0) =src[co+	ofset	][m][n].range(10 + ci*16, ci*16);
				}

			}
		}
	}
	//printf("%f",(float)(dest[1][1][1]));
}



void set_dwbias_conv3x3( fm_type buf[80][50][82], bs_type bias[80])
{
#pragma HLS array_partition variable=buf dim=1 complete
#pragma HLS array_partition variable=bias dim=1 complete

	for(int h = 0; h < 50; h+=1) {
		for(int w = 0; w < 82; w++) {
#pragma HLS pipeline
			for(int c = 0; c < 80; c++) {
#pragma HLS unroll
				buf[c][h  ][w] = bias[c];
			}
		}
	}
}
