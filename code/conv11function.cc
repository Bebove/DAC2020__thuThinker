#include "dac.h"
#include <stdio.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <math.h>
#include <fstream>
#include <hls_math.h>
#include <string.h>
using namespace std;







///////////////////
fm_type compute_engine_16(wt_type w0,  fm_type b0,
					  wt_type w1,  fm_type b1,
					  wt_type w2,  fm_type b2,
					  wt_type w3,  fm_type b3,
					  wt_type w4,  fm_type b4,
					  wt_type w5,  fm_type b5,
					  wt_type w6,  fm_type b6,
					  wt_type w7,  fm_type b7,
					  wt_type w8,  fm_type b8,
					  wt_type w9,  fm_type b9,
					  wt_type w10, fm_type b10,
					  wt_type w11, fm_type b11,
					  wt_type w12, fm_type b12,
					  wt_type w13, fm_type b13,
					  wt_type w14, fm_type b14,
					  wt_type w15, fm_type b15)
{
	ap_fixed<32,10, AP_RND, AP_SAT> mul0, mul1, mul2,  mul3,  mul4,  mul5,  mul6,  mul7;
	ap_fixed<32,10, AP_RND, AP_SAT> mul8, mul9, mul10, mul11, mul12, mul13, mul14, mul15;
	ap_fixed<32,10, AP_RND, AP_SAT> add0, add1, add2, add3,  add4,  add5,  add6;
	ap_fixed<32,10, AP_RND, AP_SAT> add7, add8, add9, add10, add11, add12, add13, add14;

	mul0  = w0  * b0;
	mul1  = w1  * b1;
	mul2  = w2  * b2;
	mul3  = w3  * b3;
	mul4  = w4  * b4;
	mul5  = w5  * b5;
	mul6  = w6  * b6;
	mul7  = w7  * b7;
	mul8  = w8  * b8;
	mul9  = w9  * b9;
	mul10 = w10 * b10;
	mul11 = w11 * b11;
	mul12 = w12 * b12;
	mul13 = w13 * b13;
	mul14 = w14 * b14;
	mul15 = w15 * b15;

	add0 = mul0  + mul1;
	add1 = mul2  + mul3;
	add2 = mul4  + mul5;
	add3 = mul6  + mul7;
	add4 = mul8  + mul9;
	add5 = mul10 + mul11;
	add6 = mul12 + mul13;
	add7 = mul14 + mul15;

	add8  = add0 + add1;
	add9  = add2 + add3;
	add10 = add4 + add5;
	add11 = add6 + add7;

	add12 = add8  + add9;
	add13 = add10 + add11;

	add14 = add12 + add13;

	return add14;

}



inline fm_type relu_single( fm_type d ,int relu) {
	if(relu ==1){
	if( d > 6 )
		return 6;
	if( d < 0 )
		return 0;}
	else{
		if(relu==2){
			if( d < 0 )
					{return 0;}
		}

	}
	return d;
}

void CONV_1x1(fm_type bottom[16][50][82],
			  fm_type top[16][50][82],
			  wt_type weights[16][16],int relu)
{
//to :choose which out channel to accumulate
//ti :choose which in  channel data to use
#pragma HLS array_partition variable=top dim=1 complete
#pragma HLS array_partition variable=bottom dim=1 complete

#pragma HLS array_partition variable=weights dim=1 complete


#pragma HLS array_partition variable=weight dim=1 complete
#pragma HLS array_partition variable=weight dim=2 complete



		for(int h = 0; h < 50; h++)
		{
			for(int w = 0; w < 82; w++)
			{
#pragma HLS pipeline II=1
				for(int coo = 0; coo < 16; coo++)
				{
#pragma HLS unroll
					fm_type temp;
					temp= compute_engine_16(
							weights[coo][0],   relu_single(bottom[0][h][w],relu),
							weights[coo][1],   relu_single(bottom[1][h][w],relu),
							weights[coo][2],   relu_single(bottom[2][h][w],relu),
							weights[coo][3],   relu_single(bottom[3][h][w],relu),
							weights[coo][4],   relu_single(bottom[4][h][w],relu),
							weights[coo][5],   relu_single(bottom[5][h][w],relu),
							weights[coo][6],   relu_single(bottom[6][h][w],relu),
							weights[coo][7],   relu_single(bottom[7][h][w],relu),
							weights[coo][8],   relu_single(bottom[8][h][w],relu),
							weights[coo][9],   relu_single(bottom[9][h][w],relu),
							weights[coo][10],  relu_single(bottom[10][h][w],relu),
							weights[coo][11],  relu_single(bottom[11][h][w],relu),
							weights[coo][12],  relu_single(bottom[12][h][w],relu),
							weights[coo][13],  relu_single(bottom[13][h][w],relu),
							weights[coo][14],  relu_single(bottom[14][h][w],relu),
							weights[coo][15],  relu_single(bottom[15][h][w],relu));
					//if((float)top[coo][h][w]+(float)temp>=32) cout<<(float)top[coo][h][w]+(float)temp<<" overflow\n";
					top[coo][h][w] +=temp;
				}
			}
		}
}



void set_bias_conv1x1( fm_type buf[16][50][82], bs_type bias[16], int x, int y,int number_ofpart_for_one_image,bool pad)
{
#pragma HLS array_partition variable=buf dim=1 complete
#pragma HLS array_partition variable=bias dim=1 complete
//this function only work when buffer is smaller than feature map



	bool x_up=	(x%number_ofpart_for_one_image==0);
	bool x_down=(x%number_ofpart_for_one_image==number_ofpart_for_one_image-1);
	bool y_up=	(y%number_ofpart_for_one_image==0);
	bool y_down=(y%number_ofpart_for_one_image==number_ofpart_for_one_image-1);


	if (x_up and pad){
		for(int w = 0; w < 82; w++){
#pragma HLS pipeline
			for(int c = 0; c < 16; c++){
#pragma HLS unroll
				buf[c][0][w]=0;                              //第一行
			}
		}
	}
	else{
		for(int w = 0; w < 82; w++){
#pragma HLS pipeline
			for(int c = 0; c < 16; c++){
#pragma HLS unroll
				buf[c][0][w]=bias[c];
			}
		}
	}

	if (x_down and pad ){
		for(int w = 0; w < 82; w++){
#pragma HLS pipeline
			for(int c = 0; c < 16; c++){
#pragma HLS unroll
				buf[c][49][w]=0;                            //最后一行
			}
		}
	}
	else{
		for(int w = 0; w < 82; w++){
#pragma HLS pipeline
			for(int c = 0; c < 16; c++){
#pragma HLS unroll
				buf[c][49][w]=bias[c];
			}
		}
	}




	if (y_up and pad){
		for(int h = 0; h < 50; h++){
#pragma HLS pipeline
			for(int c = 0; c < 16; c++){
#pragma HLS unroll
				buf[c][h][0]=0;                           //第一列
			}
		}
	}
	else{
		for(int h = 0; h < 50; h++){
#pragma HLS pipeline
			for(int c = 0; c < 16; c++){
#pragma HLS unroll
				buf[c][h][0]=bias[c];
			}
		}
	}
	if (y_down and pad){
		for(int h = 0; h < 50; h++){
#pragma HLS pipeline
			for(int c = 0; c < 16; c++){
#pragma HLS unroll
				buf[c][h][81]=0;                            //最后一列
			}
		}
	}
	else{
		for(int h = 0; h < 50; h++){
#pragma HLS pipeline
			for(int c = 0; c < 16; c++){
#pragma HLS unroll
				buf[c][h][81]=bias[c];
			}
		}
	}




	for(int h = 1; h < 49; h++) {
		for(int w = 1; w < 81; w++) {
#pragma HLS pipeline
			for(int c = 0; c < 16; c++) {
#pragma HLS unroll
				buf[c][h  ][w] = bias[c];
			}
		}
	}
	bool xuyu = (x_up or y_up) and pad;
	bool xuyd = (x_up or y_down) and pad;
	bool xdyu = (x_down or y_up) and pad;
	bool xdyd = (x_down or y_down) and pad;

	if(xuyu){			for(int c = 0; c < 16; c++){
#pragma HLS unroll
				buf[c][0][0]=0;
			}}
	if(xuyd){			for(int c = 0; c < 16; c++){
#pragma HLS unroll
				buf[c][0][81]=0;
			}}
	if(xdyu){			for(int c = 0; c < 16; c++){
#pragma HLS unroll
				buf[c][49][0]=0;
			}}
	if(xdyd){			for(int c = 0; c < 16; c++){
#pragma HLS unroll
				buf[c][49][81]=0;
			}}

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
			dest[co][ci].range(wt_lenth, 0) = DATA.range(wt_lenth + ci*16, ci*16);
		}

	}
}

void clear_pad(fm_type buf[16][50][82],int w, int h){
	for(int c=0;c<16;c++){

		for(int i=0;i<50;i++){
#pragma HLS pipeline
			buf[c][i][w+1]=0;
		}
		for(int j=0;j<82;j++){
#pragma HLS pipeline
			buf[c][h+1][j]=0;
		}
	}
}
void clear_pad2(fm_type buf[16][50][82],int w, int h)

{
	int h_w=w/2;
	int h_h=h/2;
	for(int c=0;c<16;c++){

		for(int i=0;i<h;i++){
#pragma HLS pipeline
			buf[c][i][0]=0;
			buf[c][i][w-1]=0;
			buf[c][i][h_w]=0;
			buf[c][i][h_w-1]=0;
		}
		for(int j=0;j<w;j++){
#pragma HLS pipeline
			buf[c][h-1][j]=0;
			buf[c][h_h][j]=0;
			buf[c][h_h-1][j]=0;
			buf[c][0][j]=0;
		}
	}
}
void clear_pad3(fm_type buf[16][50][82],int w, int h)

{
	int h_w=w/2;  //(320/16)+2
	int h_h=h/2;  //(192/16)+2
	for(int c=0;c<16;c++){

		for(int i=0;i<h-1;i++){
#pragma HLS pipeline

			buf[c][i][h_w-1+h_w-1-1+1]=0;
			buf[c][i][h_w-1]=0;     //320/16+1
			buf[c][i][h_w-1-1]=0;   //320/16
		}
		for(int j=0;j<w-1;j++){
#pragma HLS pipeline

			buf[c][h_h-1+h_h-1-1+1][j]=0;
			buf[c][h_h-1][j]=0;
			buf[c][h_h-1-1][j]=0;

		}
	}
}
