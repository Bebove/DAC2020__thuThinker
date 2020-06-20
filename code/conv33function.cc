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
//fm_type fm_buf_for33[4][8][49][81];
void conv3x3_502(fm_type in_buf[16][50][82],
		fm_type out_buf[16][50][82],
		wt_type weight[16][16][3][3]
		)
{
#pragma HLS array_partition variable=in_buf dim=1 complete
#pragma HLS array_partition variable=out_buf dim=1 complete
#pragma HLS array_partition variable=weight dim=1 complete
#pragma HLS array_partition variable=weight dim=2 complete

	fm_type temp;
    for(int y=0;y<3;y++){
        for (int x=0;x<3;x++){
            for(int h=0; h<14; h++){
                for(int w=0; w<22; w++){
#pragma HLS PIPELINE
                    for(int cho=0; cho<16; cho++){
                    	temp=0;
                        for(int chi=0;chi<16;chi++){
//#pragma HLS unroll
                        	temp+=weight[cho][chi][y][x]*in_buf[chi][h+y][w+x];
                        }
                        out_buf[cho][h+1][w+1]+=temp;
                    }
                }
            }
        }
    }
}

void clearpad_for_502(fm_type in_buf[16][50][82])
{
	for(int c=0;c<16;c++){

		for(int i=0;i<14;i++){
#pragma HLS unroll
			in_buf[c][i][0]=0;
			in_buf[c][i][11]=0;
			in_buf[c][i][22]=0;
		}
		for(int j=0;j<22;j++){
#pragma HLS unroll
			in_buf[c][0][j]=0;
			in_buf[c][7][j]=0;
			in_buf[c][14][j]=0;
		}
	}

}
void conv3x3_501(fm_type in_buf[16][50][82],
		fm_type out_buf[16][50][82],
		wt_type weight[16][16][3][3]
		)
{
#pragma HLS array_partition variable=in_buf dim=1 complete
#pragma HLS array_partition variable=out_buf dim=1 complete
#pragma HLS array_partition variable=weight dim=1 complete
#pragma HLS array_partition variable=weight dim=2 complete

	fm_type temp;
    for(int y=0;y<3;y++){
        for (int x=0;x<3;x++){
            for(int h=0; h<26; h++){
                for(int w=0; w<42; w++){
#pragma HLS PIPELINE
                    for(int cho=0; cho<16; cho++){
                    	temp=0;
                        for(int chi=0;chi<16;chi++){
//#pragma HLS unroll
                        	temp+=weight[cho][chi][y][x]*in_buf[chi][h+y][w+x];
                        }
                        out_buf[cho][h+1][w+1]+=temp;
                    }
                }
            }
        }
    }
}
void conv3x3_499(fm_type in_buf[16][50][82],
		fm_type out_buf[16][50][82],
		wt_type weight[16][16][3][3]
		)
{
#pragma HLS array_partition variable=in_buf dim=1 complete
#pragma HLS array_partition variable=out_buf dim=1 complete
#pragma HLS array_partition variable=weight dim=1 complete
#pragma HLS array_partition variable=weight dim=2 complete

	fm_type temp;

    for(int y=0;y<3;y++){
        for (int x=0;x<3;x++){
            for(int h=0; h<48; h++){
                for(int w=0; w<80; w++){
#pragma HLS PIPELINE
                    for(int cho=0; cho<16; cho++){
                    	temp=0;
                        for(int chi=0;chi<16;chi++){
//#pragma HLS unroll
                        	temp+=weight[cho][chi][y][x]*in_buf[chi][h+y][w+x];
                        }
                        out_buf[cho][h+1][w+1]+=temp;
                    }
                }
            }
        }
    }
}
void clearpad_for_501(fm_type in_buf[16][50][82])
{
	for(int c=0;c<16;c++){

		for(int i=0;i<28;i++){
#pragma HLS unroll
			in_buf[c][i][0]=0;
			in_buf[c][i][21]=0;
			in_buf[c][i][22]=0;
			in_buf[c][i][43]=0;
		}
		for(int j=0;j<44;j++){
#pragma HLS unroll
			in_buf[c][0][j]=0;
			in_buf[c][13][j]=0;
			in_buf[c][14][j]=0;
			in_buf[c][27][j]=0;
		}
	}

}
void clearpad_for_500(fm_type in_buf[16][50][82])
{
	for(int c=0;c<16;c++){

		for(int i=0;i<28;i++){
#pragma HLS unroll
			in_buf[c][i][0]=0;
			in_buf[c][i][41]=0;
		}
		for(int j=0;j<44;j++){
#pragma HLS unroll
			in_buf[c][0][j]=0;
			in_buf[c][25][j]=0;
		}
	}

}
void clearpad_for_499(fm_type in_buf[16][50][82])
{
	for(int c=0;c<16;c++){

		for(int i=0;i<49;i++){
#pragma HLS unroll
			in_buf[c][i][0]=0;
			in_buf[c][i][81]=0;
		}
		for(int j=0;j<81;j++){
#pragma HLS unroll
			in_buf[c][0][j]=0;
			in_buf[c][49][j]=0;
		}
	}

}
void set_bias_conv3x3( fm_type buf[4][8][49][81], bs_type bias[80])
{
#pragma HLS array_partition variable=buf dim=1 complete
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

void load_weight_conv3x3( wt_type dest[16][16][3][3], uint256 src[16][3][3])
{

	for(int m = 0; m < 3; m++)
	{
		for(int n = 0; n < 3; n++)
		{
			for(int co = 0; co < 16; co++)
			{

#pragma HLS pipeline
				uint256 DATA = 0;
				DATA.range(255, 0) = src[co	][m][n].range(255, 0);
				for(int ci = 0; ci < 16; ci++)
				{
#pragma HLS unroll
					dest[co][ci][m][n].range(wt_lenth, 0) = DATA.range(wt_lenth + ci*16, ci*16);
				}

			}
		}
	}
}
