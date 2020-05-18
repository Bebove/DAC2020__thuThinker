#include "dac.h"
#include <stdio.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <math.h>
#include <fstream>
#include <hls_math.h>
#include <string.h>
using namespace std;
fm_type fm_buf1[16][50][82];
fm_type fm_buf2[16][50][82];
fm_type fm_buf3[16][50][82];

wt_type dwt_buf3[16][3][3];  //33buffer
wt_type wt_buf1[16][16];     //11buffer
wt_type wt_buf1a[16][16];
bs_type bias[16];            //bias buffer
bs_type bias2[16];
bs_type bias3[16];




bs_type big_bias[30][16];   //big buffer
wt_type wt_buf_big[30][16][16];
wt_type dwt_buf3_big[30][16][3][3];


 

void Thinker(	uint16 image_in_raw_pad[imagesize],
				uint256		w_port_3x3[500][3][3],
				uint256     w_port_1x1[500][16],
				uint256     bias_port[500],


				uint256 ddrdebug [ddrsize][30],
				uint256 ddrdebug_2 [ddrsize][30],
				uint16 debug[2])
{
 	
#pragma HLS INTERFACE m_axi depth=3*(320+2)*(192+2) 	port=image_in_raw_pad			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=500*3*3 	port=w_port_3x3			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=500 	port=w_port_1x1			offset=slave	bundle=INPUT
	//every uint256 store 16 number, so 1 index is a full 16x16x1x1 data
#pragma HLS INTERFACE m_axi depth=500 	port=bias_port			offset=slave	bundle=INPUT
	//So we use a ddr of size at least 48*(20*8)*(12*8)


#pragma HLS INTERFACE m_axi depth=150000	port=ddrdebug			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=150000	port=ddrdebug_2			offset=slave	bundle=INPUT

#pragma HLS INTERFACE s_axilite register	port=return
#pragma HLS INTERFACE m_axi depth=2			port=debug				offset=slave	bundle=OUTPUT



#pragma HLS ALLOCATION instances=CONV_1x1			 		limit=1 function
#pragma HLS ALLOCATION instances=dw_conv_2			 		limit=3 function
#pragma HLS ALLOCATION instances=dw_conv_1			 		limit=3 function
#pragma HLS ALLOCATION instances=deload_img			 		limit=1 function
#pragma HLS ALLOCATION instances=load_weight_conv1x1			 		limit=1 function
#pragma HLS ALLOCATION instances=load_bias_from_axi			 		limit=1 function

//////////////////////////////////////////////////////////////////////////////////////////////////layer 307-310//////////////////////////////////////////////////////////////////////////////////
	//layer 307 310
	load_weight_conv1x1(wt_buf1, w_port_1x1[0]);   //load  weight for conv1x1 307  		,   	which is store at the index 0
	load_dwweight_conv3x3(dwt_buf3, w_port_3x3[0]); //load  weight for dwconv3x3 310		,	    which is store at the index 0,1,2

	load_bias_from_axi(bias, bias_port[0]);        //load  bias   for conv1x1 ,	    which is store at the index 0
	load_bias_from_axi(bias2, bias_port[1]);       //load  bias   for conv3x3 ,	    which is store at the index 1





	//IO variable:
		int ddr_channelX16_index=0;
		int allw=2*((320/2)+2);
		int w=40;
		int h=24;
		int offseth;
		int offsetw;
	for(int x=0;x<8;x++)
	{
		for(int y=0;y<8;y++)
		{
			//x: h index, x*48*320=offset
			//y: w index, y*80=offset

			aload_img(fm_buf1, image_in_raw_pad, x,  y,  x/4,  y/4,3,0,
								(320+2)*2,
								(192+2)*2,
								82,
								50);
			set_bias_conv1x1( fm_buf2, bias,x,y,4,true);
			CONV_1x1(fm_buf1,fm_buf2,wt_buf1,0); //level 1

			set_dwbias_conv3x3(fm_buf3,bias2);
			dw_conv_2(fm_buf2,fm_buf3,dwt_buf3,1); //level 2

			offsetw=2*(y/4)+1+y*w;    //begin from which windex ,in the whole featuremap
			offseth=2*(x/4)+1+x*h;    //begin from which hindex ,in the whole featuremap

			deload_img(fm_buf3, ddrdebug,
										ddr_channelX16_index,
										offsetw,
										offseth,

										w,
										h,
										allw
										);
		}
	}


//////////////////////////////////////////////////////////////////////////////////////////////////layer 313-318//////////////////////////////////////////////////////////////////////////////////
	load_bias_from_axi(bias, bias_port[2]);   //load  bias  for the first conv1x1 ,	 which is store at the index 2
	load_bias_from_axi(bias2,bias_port[3]); //bias for dw conv3x3
	load_bias_from_axi(bias3,bias_port[4]); //bias for the second conv1x1
	load_weight_conv1x1(wt_buf1,w_port_1x1[1]);
	load_weight_conv1x1(wt_buf1a,w_port_1x1[2]);
	load_dwweight_conv3x3(dwt_buf3,w_port_3x3[1]);
	
	//IO variable:
		ddr_channelX16_index=0;
		allw=2*((320/2)+2);
		w=82;
		h=50;


	for(int x=0;x<4;x++){
		for(int y=0;y<4;y++){

			offsetw=2*(y/2)+y*80;
			offseth=2*(x/2)+x*48;
			aload_img_2(fm_buf1, ddrdebug,
										ddr_channelX16_index,
										offsetw,
										offseth,

										w,
										h,
										allw
										);
			set_bias_conv1x1(fm_buf2,bias,x,y,2,true);
			CONV_1x1(fm_buf1,fm_buf2,wt_buf1,1); //level 3

			set_dwbias_conv3x3(fm_buf3,bias2);
			dw_conv_1(fm_buf2,fm_buf3,dwt_buf3,0); //level 4

			set_bias_conv1x1(fm_buf2,bias3,x,y,2,false);
			CONV_1x1(fm_buf3,fm_buf2,wt_buf1a,1); // level 5 no relu
			offsetw=2*(y/2)+y*80+1;
			offseth=2*(x/2)+x*48+1; 					 //this is offset for padding
			deload_img(fm_buf2, ddrdebug_2,
										ddr_channelX16_index,
										offsetw,
										offseth,

										80,
										48,
										allw
										);
		}
	}



///////////////////////////////////////////////////////////////////////////////////////////////layer 320-326//////////////////////////////////////////////////////////////////////////////////////////////
	//320:
		load_bias_from_axi(big_bias[0], bias_port[5]);    //bias for layer320 1-16
		load_bias_from_axi(big_bias[1], bias_port[6]);    //bias for layer320 17-32
		load_bias_from_axi(big_bias[2], bias_port[7]);    //bias for layer320 33-48
		load_weight_conv1x1( wt_buf_big[0], w_port_1x1[3]);  //the weight for 1x1 lay320 :output channel 1-16
		load_weight_conv1x1( wt_buf_big[1], w_port_1x1[4]);  //the weight for 1x1 lay320 :output channel 17-32
		load_weight_conv1x1( wt_buf_big[2], w_port_1x1[5]);  //the weight for 1x1 lay320 :output channel 33-48
	//323:
		load_bias_from_axi(big_bias[3], bias_port[8]);    //bias for layer323 1-16
		load_bias_from_axi(big_bias[4], bias_port[9]);    //bias for layer323 17-32
		load_bias_from_axi(big_bias[5], bias_port[10]);   //bias for layer323 33-48
		load_dwweight_conv3x3(dwt_buf3_big[0],w_port_3x3[2]);//this load full 48 channel 3x3 weight
		load_dwweight_conv3x3(dwt_buf3_big[1],w_port_3x3[3]);//this load full 48 channel 3x3 weight
		load_dwweight_conv3x3(dwt_buf3_big[2],w_port_3x3[4]);//this load full 48 channel 3x3 weight
	//326:
		load_bias_from_axi(big_bias[6], bias_port[11]);   //bias for layer326 0-16
		load_weight_conv1x1( wt_buf_big[3], w_port_1x1[6]);//the weight for 1x1 lay326 :output channel 1-16
		load_weight_conv1x1( wt_buf_big[4], w_port_1x1[7]);//the weight for 1x1 lay326 :output channel 17-32
		load_weight_conv1x1( wt_buf_big[5], w_port_1x1[8]);//the weight for 1x1 lay326 :output channel 33-48     //but mode is not same. this is [16][48]  seperate to    3* [16][16]


	//IO variable:
		ddr_channelX16_index=0;
		allw=2*((320/2)+2);
		w=82;
		h=50;

		int wafter=40;
		int hafter=24;
		int allwafter=2*((320/4)+2);
		initial_ddr(ddrdebug,
								1,
								2*((320/4)+2),
								2*((192/4)+2)
									);
	for(int x=0;x<4;x++){
		for(int y=0;y<4;y++){

			offsetw=2*(y/2)+y*80;
			offseth=2*(x/2)+x*48;
			aload_img_2(fm_buf1, ddrdebug_2,
										ddr_channelX16_index,
										offsetw,
										offseth,

										w,
										h,
										allw
										);              //channel is 8, fm_buf1 0-7 is data
			set_bias_conv1x1(fm_buf3,big_bias[6],x,y,2,false);

			//0-15:
			set_bias_conv1x1(fm_buf2,	  big_bias[0],x,y,2,true);     //16  bias is load
			CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[0],0);     //full 16 output channel for 320
			set_dwbias_conv3x3(fm_buf1,   big_bias[3]);             //16  bias is load
			dw_conv_2(fm_buf2,fm_buf1,dwt_buf3_big[0],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);     //full 16 output channel for 320

			//15-31:
			set_bias_conv1x1(fm_buf2,	  big_bias[1],x,y,2,true);     //16  bias is load
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[1],0);     //full 16 output channel for 320
			set_dwbias_conv3x3(fm_buf1,   big_bias[4]);             //16  bias is load
			dw_conv_2(fm_buf2,fm_buf1,dwt_buf3_big[1],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[4],1);     //full 16 output channel for 320

			//32-47:
			set_bias_conv1x1(fm_buf2,     big_bias[2],x,y,2,true);     //16  bias is load
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],0);     //full 16 output channel for 320
			set_dwbias_conv3x3(fm_buf1,   big_bias[5]);             //16  bias is load
			dw_conv_2(fm_buf2,fm_buf1,dwt_buf3_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[5],1);     //full 16 output channel for 320

			offsetw=2*(y/2)+1+y*wafter;    //begin from which windex ,in the whole featuremap
			offseth=2*(x/2)+1+x*hafter;    //begin from which hindex ,in the whole featuremap

			deload_img(fm_buf3, ddrdebug,
										ddr_channelX16_index,
										offsetw,
										offseth,

										wafter,
										hafter,
										allwafter
										);


			}               //still have ddr problem
		}





	debug[0]=0.1;

}
