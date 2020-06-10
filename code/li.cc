#include <ap_fixed.h>
#include <ap_int.h>
#include <stdio.h>
#include <iostream>
#include "dac.h"



void bilinear_1(
		fm_type fm_input[16][50][82],
		fm_type fm_output[16][50][82]
	)
{
float high_dst=192/16;
float width_dst=320/16;
float high_src=192/32;
float width_src =320/32;
float param_h 	=1/2;
float param_w =	1/2;
#pragma HLS array_partition variable=fm_input dim=1 complete
#pragma HLS array_partition variable=fm_output dim=1 complete
float h_src;
float w_src;
		int h0;
		int w0;
		int h1;
		int w1;
		int	u;
		int	v;
		for(int h_dst=0;h_dst<high_dst;h_dst++){
			for(int w_dst=0;w_dst<width_dst;w_dst++){
#pragma HLS pipeline II=1
				h_src=(h_dst+0.5)*param_h-0.5;
				w_src=(w_dst+0.5)*param_w-0.5;

				//��ͶӰֵclipס
				if(h_src<0) h_src=0;
					else if(h_src>high_src-1) h_src=high_src-1;
				if(w_src<0) w_src=0;
					else if(w_src>width_src-1) w_src=width_src-1;
				//Ŀ��ͼͶӰ

				h0=h_src;  //tobe opti
				w0=w_src;  //tobe opti
				h1=h0+1;
				w1=w0+1;
				u=h_src-h0;
				v=w_src-w0;

				for(int ch=0;ch<16;ch++){
#pragma HLS unroll
				fm_output[ch][h_dst][w_dst]=(fm_input[ch][h0][w0]*(1-u)*(1-v)
						+fm_input[ch][h0][w1]*(1-u)*v
						+fm_input[ch][h1][w0]*u*(1-v)
						+fm_input[ch][h1][w1]*u*v);
				//��ֵ����ͼ��ֵ
			}
		}
	}
	printf("\ntest\n");

}


void load_oneimageto_ddr (
			fm_type fm_buf1[16][50][82],int choose,fm_type fm_buf2[16][50][82]
		  )
{
#pragma HLS array_partition variable=fm_buf1 dim=1 complete
#pragma HLS array_partition variable=fm_buf2 dim=1 complete
	int index_x[4]={1,12,1,12};
	int index_y[4]={1,1,8,8};
	int w=index_x[choose];
	int h=index_y[choose];


		for(int x=0;x<10;x++){
			for(int y=0;y<6;y++){
#pragma HLS pipeline
				for(int c=0;c<16;c++){
#pragma HLS unroll
				fm_buf2[c][y][x]=fm_buf1[c][y+h][x+w];
			}

		}
	}




}
