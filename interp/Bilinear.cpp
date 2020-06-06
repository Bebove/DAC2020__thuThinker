#include <ap_fixed.h>
#include "Bilinear.h"
#include <ap_int.h>
#include <stdio.h>
#include <iostream>


//uint8 fm_input[3][80][160];
//uint8 fm_output[3][160][320];

void bilinear(
	uint8 fm_input[3][high_src][width_src],
	uint8 fm_output[3][high_dst][width_dst]
	){

	for(int ch=0;ch<3;ch++){
	//	printf("*****************");
		for(int h_dst=0;h_dst<high_dst;h_dst++){
//			printf("\n");
			for(int w_dst=0;w_dst<width_dst;w_dst++){
				fix16 h_src=(h_dst+0.5)*param_h-0.5;
				fix16 w_src=(w_dst+0.5)*param_w-0.5;
				//将投影值clip住
				if(h_src<0) h_src=0;
					else if(h_src>high_src-1) h_src=high_src-1;
				if(w_src<0) w_src=0;
					else if(w_src>width_src-1) w_src=width_src-1;
				//目标图投影
				uint9 h0=h_src;
				uint9 w0=w_src;
				uint9 h1=h0+1;
				uint9 w1=w0+1;
				//int debug0=(int)fm_input[ch][h0][w0];
				//printf("%d ",debug0);
				//计算投影点周围的四个点
				fix16	u=h_src-h0;
				fix16	v=w_src-w0;
				fm_output[ch][h_dst][w_dst]=(uint8)(fm_input[ch][h0][w0]*(1-u)*(1-v)
						+fm_input[ch][h0][w1]*(1-u)*v
						+fm_input[ch][h1][w0]*u*(1-v)
						+fm_input[ch][h1][w1]*u*v);
				//插值计算图像值
			}
		}
	}

	printf("\ntest\n");
	return;
}
