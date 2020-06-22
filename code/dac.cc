//layer 2
//layer3version
//version of only 424 modify
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
fm_type fm_buf4[16][50][82];
fm_type fm_buf5[16][50][82];
fm_type fm_buf6[16][50][82];
fm_type fm_buf7[16][50][82];
fm_type fm_buf8[16][50][82];



bs_type big_bias[14][16];   //big buffer 0-13
wt_type wt_buf_big[18][16][16];//0-17
wt_type dwt_buf3_big[6][16][3][3];//0-5

wt_type     big_w33_buffer[12][16][16][3][3];
 

void Thinker(	uint16 image_in_raw_pad[imagesize],
				uint256		w_port_3x3[170][3][3],
				uint256     w_port_1x1[1000][16],
				uint256     bias_port[500],
				uint256		w_port_3x3_2[13*4][16][3][3],

				uint256 ddrdebug [ddrsize][ddrsize_dp],
				uint256 ddrdebug_2 [ddrsize][ddrsize_dp],
				uint256 ddrdebug_3 [ddrsize][ddrsize_dp],
				uint256 ddrdebug_4 [ddrsize][ddrsize_dp],
				uint256 temp1 [ddrsize][ddrsize_dp])
{
 	
#pragma HLS INTERFACE m_axi depth=3*(320+2)*(192+2) 	port=image_in_raw_pad			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=170*3*3 	port=w_port_3x3			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=1000 	port=w_port_1x1			offset=slave	bundle=INPUT
	//every uint256 store 16 number, so 1 index is a full 16x16x1x1 data
#pragma HLS INTERFACE m_axi depth=500 	port=bias_port			offset=slave	bundle=INPUT
	//So we use a ddr of size at least 48*(20*8)*(12*8)
#pragma HLS INTERFACE m_axi depth=13*4*16*9 	port=w_port_3x3_2			offset=slave	bundle=INPUT

#pragma HLS INTERFACE m_axi depth=65600	port=ddrdebug			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=65600	port=ddrdebug_2			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=65600	port=ddrdebug_3			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=65600	port=ddrdebug_4			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=65600	port=temp1			offset=slave	bundle=INPUT
#pragma HLS INTERFACE s_axilite register	port=return




#pragma HLS ALLOCATION instances=CONV_1x1			 		limit=1 function
#pragma HLS ALLOCATION instances=dw_conv_2			 		limit=1 function
#pragma HLS ALLOCATION instances=dw_conv_1			 		limit=1 function
#pragma HLS ALLOCATION instances=deload_img			 		limit=1 function
#pragma HLS ALLOCATION instances=load_weight_conv1x1			 		limit=1 function
#pragma HLS ALLOCATION instances=load_bias_from_axi			 		limit=1 function


#pragma HLS ALLOCATION instances=conv3x3_501			 		limit=1 function
#pragma HLS ALLOCATION instances=conv3x3_502			 		limit=1 function
#pragma HLS ALLOCATION instances=clearpad_for_502			 		limit=1 function
#pragma HLS ALLOCATION instances=conv3x3_499			 		limit=1 function
//////////////////////////////////////////////////////////////////////////////////////////////////layer 307-310//////////////////////////////////////////////////////////////////////////////////
	//layer 307 310
	load_weight_conv1x1(wt_buf_big[0], w_port_1x1[0]);   //load  weight for conv1x1 307  		,   	which is store at the index 0
	load_dwweight_conv3x3(dwt_buf3_big[0], w_port_3x3[0]); //load  weight for dwconv3x3 310		,	    which is store at the index 0,1,2

	load_bias_from_axi(big_bias[0], bias_port[0]);        //load  bias   for conv1x1 ,	    which is store at the index 0
	load_bias_from_axi(big_bias[1], bias_port[1]);       //load  bias   for conv3x3 ,	    which is store at the index 1

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
			set_bias_conv1x1( fm_buf2, big_bias[0],x,y,4,true);
			CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[0],0); //level 1

			set_dwbias_conv3x3(fm_buf3,big_bias[1]);
			dw_conv_2(fm_buf2,fm_buf3,dwt_buf3_big[0],1); //level 2

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
	load_bias_from_axi(big_bias[0], bias_port[2]);   //load  bias  for the first conv1x1 ,	 which is store at the index 2
	load_bias_from_axi(big_bias[1],bias_port[3]); //bias for dw conv3x3
	load_bias_from_axi(big_bias[2],bias_port[4]); //bias for the second conv1x1
	load_weight_conv1x1(wt_buf_big[0],w_port_1x1[1]);
	load_weight_conv1x1(wt_buf_big[1],w_port_1x1[2]);
	load_dwweight_conv3x3(dwt_buf3_big[0],w_port_3x3[1]);
	
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
			set_bias_conv1x1(fm_buf2,big_bias[0],x,y,2,true);
			CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[0],1); //level 3

			set_dwbias_conv3x3(fm_buf3,big_bias[1]);
			dw_conv_1(fm_buf2,fm_buf3,dwt_buf3_big[0],0); //level 4

			set_bias_conv1x1(fm_buf2,big_bias[2],x,y,2,false);
			CONV_1x1(fm_buf3,fm_buf2,wt_buf_big[1],1); // level 5 no relu
			offsetw=1+2*(y/2)+y*80;
			offseth=1+2*(x/2)+x*48; 					 //this is offset for padding
			deload_img(fm_buf2, ddrdebug_2,
										ddr_channelX16_index,
										offsetw,
										offseth,

										80,
										50,
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
					set_dwbias_conv3x3(fm_buf4,   big_bias[3]);             //16  bias is load
					dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[0],1);
					CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[3],1);

					//15-31:
					set_bias_conv1x1(fm_buf2,	  big_bias[1],x,y,2,true);     //16  bias is load
					CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[1],0);     //full 16 output channel for 320
					set_dwbias_conv3x3(fm_buf4,   big_bias[4]);             //16  bias is load
					dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[1],1);
					CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[4],1);
					//32-47:
					set_bias_conv1x1(fm_buf2,     big_bias[2],x,y,2,true);     //16  bias is load
					CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],0);     //full 16 output channel for 320
					set_dwbias_conv3x3(fm_buf4,   big_bias[5]);             //16  bias is load
					dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[2],1);
					CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[5],1);
					deload_img(fm_buf3, ddrdebug,
												0,
												2*(y/2)+y*40+1,
												2*(x/2)+x*24+1,

												40,
												24,
												allwafter
												);
					//offsetw=y*wafter;    //begin from which windex ,in the whole featuremap
					//offseth=x*hafter;    //begin from which hindex ,in the whole featuremap
					}               //still have ddr problem
				}


///////////////////////////////////////////////////////////////////////////////////////////////layer 328-331//////////////////////////////////////////////////////////////////////////////////////////////	
		//328:
			load_bias_from_axi(big_bias[0], bias_port[12]);    //bias for layer328 1-16
			load_bias_from_axi(big_bias[1], bias_port[13]);    //bias for layer328 17-32
			load_bias_from_axi(big_bias[2], bias_port[14]);    //bias for layer328 33-48
			load_bias_from_axi(big_bias[3], bias_port[15]);    //bias for layer328 49-64
			load_bias_from_axi(big_bias[4], bias_port[16]);    //bias for layer328 64-80
			load_bias_from_axi(big_bias[5], bias_port[17]);    //bias for layer328 81-96
			load_weight_conv1x1( wt_buf_big[0], w_port_1x1[9]);  //the weight for 1x1 lay328 :output channel 1-16
			load_weight_conv1x1( wt_buf_big[1], w_port_1x1[10]);  //the weight for 1x1 lay328 :output channel 17-32
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[11]);  //the weight for 1x1 lay328 :output channel 33-48
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[12]);  //the weight for 1x1 lay328 :output channel 49-64
			load_weight_conv1x1( wt_buf_big[4], w_port_1x1[13]);  //the weight for 1x1 lay328 :output channel 64-80
			load_weight_conv1x1( wt_buf_big[5], w_port_1x1[14]);  //the weight for 1x1 lay328 :output channel 81-96
		//331:
			load_bias_from_axi(big_bias[6], bias_port[18]);    //bias for layer331 1-16
			load_bias_from_axi(big_bias[7], bias_port[19]);    //bias for layer331 17-32
			load_bias_from_axi(big_bias[8], bias_port[20]);    //bias for layer331 33-48
			load_bias_from_axi(big_bias[9], bias_port[21]);    //bias for layer331 49-64
			load_bias_from_axi(big_bias[10], bias_port[22]);    //bias for layer331 64-80
			load_bias_from_axi(big_bias[11], bias_port[23]);    //bias for layer331 81-96
			load_dwweight_conv3x3(dwt_buf3_big[0],w_port_3x3[5]);//this load full 48 channel 3x3 weight
			load_dwweight_conv3x3(dwt_buf3_big[1],w_port_3x3[6]);//this load full 48 channel 3x3 weight
			load_dwweight_conv3x3(dwt_buf3_big[2],w_port_3x3[7]);//this load full 48 channel 3x3 weight
			load_dwweight_conv3x3(dwt_buf3_big[3],w_port_3x3[8]);//this load full 48 channel 3x3 weight
			load_dwweight_conv3x3(dwt_buf3_big[4],w_port_3x3[9]);//this load full 48 channel 3x3 weight
			load_dwweight_conv3x3(dwt_buf3_big[5],w_port_3x3[10]);//this load full 48 channel 3x3 weight
		//334:
			load_bias_from_axi(big_bias[12], bias_port[24]);   //bias for layer334 0-16
			load_weight_conv1x1( wt_buf_big[6], w_port_1x1[15]);  //the weight for 1x1 lay334 :output channel 1-16
			load_weight_conv1x1( wt_buf_big[7], w_port_1x1[16]);  //the weight for 1x1 lay334 :output channel 17-32
			load_weight_conv1x1( wt_buf_big[8], w_port_1x1[17]);  //the weight for 1x1 lay334 :output channel 33-48
			load_weight_conv1x1( wt_buf_big[9], w_port_1x1[18]);  //the weight for 1x1 lay334 :output channel 49-64
			load_weight_conv1x1( wt_buf_big[10], w_port_1x1[19]);  //the weight for 1x1 lay334 :output channel 64-80
			load_weight_conv1x1( wt_buf_big[11], w_port_1x1[20]);  //the weight for 1x1 lay334 :output channel 81-96


			for(int x=0;x<2;x++){
				for(int y=0;y<2;y++){
					offsetw=2*y+y*80;
					offseth=2*x+x*48;
					aload_img_2(fm_buf1, ddrdebug,
												0,
												offsetw,
												offseth,

												82,
												50,
												2*((320/4)+2)
												);
					set_bias_conv1x1(fm_buf3,big_bias[12],x,y,1,false);
					//0-15:
					set_bias_conv1x1(fm_buf2,	  big_bias[0],x,y,1,true);     //16  bias is load
					CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[0],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[6]);             //16  bias is load
					dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[0],1);
					CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6],1);

					//16-32:
					set_bias_conv1x1(fm_buf2,	  big_bias[1],x,y,1,true);     //16  bias is load
					CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[1],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[7]);             //16  bias is load
					dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[1],1);
					CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[7],1);

					//33-48:
					set_bias_conv1x1(fm_buf2,	  big_bias[2],x,y,1,true);     //16  bias is load
					CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[2],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[8]);             //16  bias is load
					dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[2],1);
					CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[8],1);

					//49-64:
					set_bias_conv1x1(fm_buf2,	  big_bias[3],x,y,1,true);     //16  bias is load
					CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[3],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[9]);             //16  bias is load
					dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[3],1);
					CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[9],1);

					//65-80:
					set_bias_conv1x1(fm_buf2,	  big_bias[4],x,y,1,true);     //16  bias is load
					CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[10]);             //16  bias is load
					dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[4],1);
					CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[10],1);

					//81-96:
					set_bias_conv1x1(fm_buf2,	  big_bias[5],x,y,1,true);     //16  bias is load
					CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[5],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[11]);             //16  bias is load
					dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[5],1);
					CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[11],1);

					deload_img(fm_buf3, ddrdebug,
												0,
												2*y+y*80+1,
												2*x+x*48+1,

												80,
												48,
												2*((320/4)+2)
												);
				}
			}
//////////////////////////////////////////////////////////////////////////////	layer 337 343 ///////////////////////////////////////////////////////////////////////////////////////
			//337:
			for(int i=0;i<6;i++)
			{
				load_bias_from_axi(big_bias[i], bias_port[i+25]);
				load_weight_conv1x1( wt_buf_big[i], w_port_1x1[i+21]);
			}
			//340:
			for(int i=0;i<6;i++)
			{
				load_bias_from_axi(big_bias[i+6], bias_port[i+25+6]);

			}
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[11]);
			load_dwweight_conv3x3( dwt_buf3_big[1], w_port_3x3[12]);
			load_dwweight_conv3x3( dwt_buf3_big[2], w_port_3x3[13]);
			load_dwweight_conv3x3( dwt_buf3_big[3], w_port_3x3[14]);
			load_dwweight_conv3x3( dwt_buf3_big[4], w_port_3x3[15]);
			load_dwweight_conv3x3( dwt_buf3_big[5], w_port_3x3[16]);
			//343:
			for(int i=0;i<6;i++)
			{

				load_weight_conv1x1( wt_buf_big[i+6], w_port_1x1[i+27]);
			}
			load_bias_from_axi(big_bias[12], bias_port[37]);

			initial_ddr(ddrdebug_2,
									1,
									2*((320/8)+2),
									2*((192/8)+2)
									);
			for(int x=0;x<2;x++)
			{
				for(int y=0;y<2;y++)
				{
					offsetw=2*y+y*80;
					offseth=2*x+x*48;
					aload_img_2(fm_buf1, ddrdebug,
												0,
												offsetw,
												offseth,

												82,
												50,
												2*((320/4)+2)
												);
					set_bias_conv1x1(fm_buf3,big_bias[12],x,y,1,false);


						set_bias_conv1x1(fm_buf2,	  big_bias[0],x,y,1,true);     //16  bias is load
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[0],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+0]);             //16  bias is load
						dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[0],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+0],1);

						set_bias_conv1x1(fm_buf2,	  big_bias[1],x,y,1,true);     //16  bias is load
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[1],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+1]);             //16  bias is load
						dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[1],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+1],1);

						set_bias_conv1x1(fm_buf2,	  big_bias[2],x,y,1,true);     //16  bias is load
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[2],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+2]);             //16  bias is load
						dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[2],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+2],1);

						set_bias_conv1x1(fm_buf2,	  big_bias[3],x,y,1,true);     //16  bias is load
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[3],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+3]);             //16  bias is load
						dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[3],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+3],1);

						set_bias_conv1x1(fm_buf2,	  big_bias[4],x,y,1,true);     //16  bias is load
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[4],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+4]);             //16  bias is load
						dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[4],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+4],1);

						set_bias_conv1x1(fm_buf2,	  big_bias[5],x,y,1,true);     //16  bias is load
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[5],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+5]);             //16  bias is load
						dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[5],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+5],1);


					deload_img(fm_buf3, ddrdebug_2,
												0,
												2*y+y*40+1,
												2*x+x*24+1,

												40,
												24,
												2*((320/8)+2)
												);
				}
			}

//////////////////////////////////////////////////////////////////////////////	layer 345-351 ///////////////////////////////////////////////////////////////////////////////////////
			//345:
			for(int i=0;i<6;i++)
			{
				load_bias_from_axi(big_bias[i], bias_port[i+38]);
				load_weight_conv1x1( wt_buf_big[i], w_port_1x1[i+33]);
			}
			//348:
			for(int i=0;i<6;i++)
			{
				load_bias_from_axi(big_bias[i+6], bias_port[i+44]);


			}
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[0+17]);
			load_dwweight_conv3x3( dwt_buf3_big[1], w_port_3x3[1+17]);
			load_dwweight_conv3x3( dwt_buf3_big[2], w_port_3x3[2+17]);
			load_dwweight_conv3x3( dwt_buf3_big[3], w_port_3x3[3+17]);
			load_dwweight_conv3x3( dwt_buf3_big[4], w_port_3x3[4+17]);
			load_dwweight_conv3x3( dwt_buf3_big[5], w_port_3x3[5+17]);
			//351:
			for(int i=0;i<6;i++)
			{

				load_weight_conv1x1( wt_buf_big[i+6], w_port_1x1[i+39]);
			}
			load_bias_from_axi(big_bias[12], bias_port[50]);

			for(int x=0;x<2;x++)
			{
				for(int y=0;y<2;y++)
				{
					offsetw=2*y+y*40;
					offseth=2*x+x*24;
					aload_img_2(fm_buf1, ddrdebug_2,
												0,
												offsetw,
												offseth,

												42,
												26,
												2*((320/8)+2)
												);
					set_bias_conv1x1(fm_buf3,big_bias[12],x,y,1,false);


						set_bias_conv1x1(fm_buf2,	  big_bias[0],x,y,1,true);     //16  bias is load
						clear_pad(fm_buf2,40,24);
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[0],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+0]);             //16  bias is load
						dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[0],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+0],1);

						set_bias_conv1x1(fm_buf2,	  big_bias[1],x,y,1,true);     //16  bias is load
						clear_pad(fm_buf2,40,24);
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[1],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+1]);             //16  bias is load
						dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[1],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+1],1);

						set_bias_conv1x1(fm_buf2,	  big_bias[2],x,y,1,true);     //16  bias is load
						clear_pad(fm_buf2,40,24);
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[2],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+2]);             //16  bias is load
						dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[2],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+2],1);

						set_bias_conv1x1(fm_buf2,	  big_bias[3],x,y,1,true);     //16  bias is load
						clear_pad(fm_buf2,40,24);
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[3],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+3]);             //16  bias is load
						dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[3],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+3],1);

						set_bias_conv1x1(fm_buf2,	  big_bias[4],x,y,1,true);     //16  bias is load
						clear_pad(fm_buf2,40,24);
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[4],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+4]);             //16  bias is load
						dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[4],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+4],1);

						set_bias_conv1x1(fm_buf2,	  big_bias[5],x,y,1,true);     //16  bias is load
						clear_pad(fm_buf2,40,24);
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[5],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+5]);             //16  bias is load
						dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[5],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+5],1);


					deload_img(fm_buf3, ddrdebug_2,
												0,
												2*y+y*40+1,
												2*x+x*24+1,

												40,
												24,
												2*((320/8)+2)
												);
				}
			}

//////////////////////////////////////////////////////////////////////////////	layer 354-360 ///////////////////////////////////////////////////////////////////////////////////////
			//354:
			for(int i=0;i<6;i++)
			{
				load_bias_from_axi(big_bias[i], bias_port[i+51]);
				load_weight_conv1x1( wt_buf_big[i], w_port_1x1[i+45]);
			}
			//357:
			for(int i=0;i<6;i++)
			{
				load_bias_from_axi(big_bias[i+6], bias_port[i+57]);


			}
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[0+23]);
			load_dwweight_conv3x3( dwt_buf3_big[1], w_port_3x3[1+23]);
			load_dwweight_conv3x3( dwt_buf3_big[2], w_port_3x3[2+23]);
			load_dwweight_conv3x3( dwt_buf3_big[3], w_port_3x3[3+23]);
			load_dwweight_conv3x3( dwt_buf3_big[4], w_port_3x3[4+23]);
			load_dwweight_conv3x3( dwt_buf3_big[5], w_port_3x3[5+23]);
			//360:
			for(int i=0;i<6;i++)
			{

				load_weight_conv1x1( wt_buf_big[i+6], w_port_1x1[i+51]);
			}
			load_bias_from_axi(big_bias[12], bias_port[63]);

			for(int x=0;x<2;x++)
			{
				for(int y=0;y<2;y++)
				{
					offsetw=2*y+y*40;
					offseth=2*x+x*24;
					aload_img_2(fm_buf1, ddrdebug_2,
												0,
												offsetw,
												offseth,

												42,
												26,
												2*((320/8)+2)
												);
					set_bias_conv1x1(fm_buf3,big_bias[12],x,y,1,false);


						set_bias_conv1x1(fm_buf2,	  big_bias[0],x,y,1,true);     //16  bias is load
						clear_pad(fm_buf2,40,24);
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[0],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+0]);             //16  bias is load
						dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[0],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+0],1);

						set_bias_conv1x1(fm_buf2,	  big_bias[1],x,y,1,true);     //16  bias is load
						clear_pad(fm_buf2,40,24);
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[1],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+1]);             //16  bias is load
						dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[1],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+1],1);

						set_bias_conv1x1(fm_buf2,	  big_bias[2],x,y,1,true);     //16  bias is load
						clear_pad(fm_buf2,40,24);
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[2],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+2]);             //16  bias is load
						dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[2],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+2],1);

						set_bias_conv1x1(fm_buf2,	  big_bias[3],x,y,1,true);     //16  bias is load
						clear_pad(fm_buf2,40,24);
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[3],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+3]);             //16  bias is load
						dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[3],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+3],1);

						set_bias_conv1x1(fm_buf2,	  big_bias[4],x,y,1,true);     //16  bias is load
						clear_pad(fm_buf2,40,24);
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[4],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+4]);             //16  bias is load
						dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[4],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+4],1);

						set_bias_conv1x1(fm_buf2,	  big_bias[5],x,y,1,true);     //16  bias is load
						clear_pad(fm_buf2,40,24);
						CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[5],0);
						set_dwbias_conv3x3(fm_buf4,   big_bias[6+5]);             //16  bias is load
						dw_conv_1(fm_buf2,fm_buf4,dwt_buf3_big[5],1);
						CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+5],1);


					deload_img(fm_buf3, ddrdebug_2,
												0,
												2*y+y*40+1,
												2*x+x*24+1,

												40,
												24,
												2*((320/8)+2)
												);
				}
			}

//////////////////////////////////////////////////////////////////////////////	layer 363-369 ///////////////////////////////////////////////////////////////////////////////////////
									//363:
			for(int i=0;i<6;i++)
			{
				load_bias_from_axi(big_bias[i], bias_port[i+64]);
				load_weight_conv1x1( wt_buf_big[i], w_port_1x1[i+57]);
			}
			//366:
			for(int i=0;i<6;i++)
			{
				load_bias_from_axi(big_bias[i+6], bias_port[i+70]);
			}
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[0+29]);
			load_dwweight_conv3x3( dwt_buf3_big[1], w_port_3x3[1+29]);
			load_dwweight_conv3x3( dwt_buf3_big[2], w_port_3x3[2+29]);
			load_dwweight_conv3x3( dwt_buf3_big[3], w_port_3x3[3+29]);
			load_dwweight_conv3x3( dwt_buf3_big[4], w_port_3x3[4+29]);
			load_dwweight_conv3x3( dwt_buf3_big[5], w_port_3x3[5+29]);
			//369:
			for(int i=0;i<6;i++)
			{

				load_weight_conv1x1( wt_buf_big[i+6], w_port_1x1[i+63]);
			}
			for(int i=0;i<6;i++)
			{

				load_weight_conv1x1( wt_buf_big[i+6+6], w_port_1x1[i+69]);
			}
			load_bias_from_axi(big_bias[12], bias_port[76]);
			load_bias_from_axi(big_bias[13], bias_port[77]);
			initial_ddr(ddrdebug_3,
									2,
									2*((320/16)+2),
									2*((192/16)+2)
									);
			for(int x=0;x<2;x++)
			{
							for(int y=0;y<2;y++)
							{
								offsetw=2*y+y*40;
								offseth=2*x+x*24;
								aload_img_2(fm_buf1, ddrdebug_2,
															0,
															offsetw,
															offseth,

															42,
															26,
															2*((320/8)+2)
															);
								set_bias_conv1x1(fm_buf3,big_bias[12],x,y,1,false);
									int i=0;
									set_bias_conv1x1(fm_buf2,	  big_bias[i],x,y,1,true);     //16  bias is load
									clear_pad(fm_buf2,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[i],0);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6+i]);             //16  bias is load
									dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[i],1);
									CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+i],1);
									deload_img(fm_buf4, temp1,
																0,
																2*y+y*20,
																2*x+x*12,

																20,
																12,
																2*(320/16)+2
																);
									i=1+i;
									set_bias_conv1x1(fm_buf2,	  big_bias[i],x,y,1,true);     //16  bias is load
									clear_pad(fm_buf2,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[i],0);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6+i]);             //16  bias is load
									dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[i],1);
									CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+i],1);
									deload_img(fm_buf4, temp1,
																1,
																2*y+y*20,
																2*x+x*12,

																20,
																12,
																2*(320/16)+2
																);				
									i=1+i;
									set_bias_conv1x1(fm_buf2,	  big_bias[i],x,y,1,true);     //16  bias is load
									clear_pad(fm_buf2,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[i],0);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6+i]);             //16  bias is load
									dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[i],1);								
									CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+i],1);
									deload_img(fm_buf4, temp1,
																2,
																2*y+y*20,
																2*x+x*12,

																20,
																12,
																2*(320/16)+2
																);									
									
									i=1+i;
									set_bias_conv1x1(fm_buf2,	  big_bias[i],x,y,1,true);     //16  bias is load
									clear_pad(fm_buf2,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[i],0);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6+i]);             //16  bias is load
									dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[i],1);
									CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+i],1);
									deload_img(fm_buf4, temp1,
																3,
																2*y+y*20,
																2*x+x*12,

																20,
																12,
																2*(320/16)+2
																);
									i=1+i;
									set_bias_conv1x1(fm_buf2,	  big_bias[i],x,y,1,true);     //16  bias is load
									clear_pad(fm_buf2,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[i],0);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6+i]);             //16  bias is load
									dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[i],1);
									CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+i],1);
									deload_img(fm_buf4, temp1,
																4,
																2*y+y*20,
																2*x+x*12,

																20,
																12,
																2*(320/16)+2
																);		
									i=1+i;
									set_bias_conv1x1(fm_buf2,	  big_bias[i],x,y,1,true);     //16  bias is load
									clear_pad(fm_buf2,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[i],0);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6+i]);             //16  bias is load
									dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[i],1);
									CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+i],1);
									deload_img(fm_buf4, temp1,
																5,
																2*y+y*20,
																2*x+x*12,

																20,
																12,
																2*(320/16)+2
																);									
									
									deload_img(fm_buf3, ddrdebug_3,
																0,
																2*y+y*20+1,
																2*x+x*12+1,

																20,
																12,
																2*((320/16)+2)
																);

							}
			}
			//last 16 channel
			set_bias_conv1x1(fm_buf2,big_bias[13],1,1,1,false);
			int i=-1;
			i++;
			aload_img_2(fm_buf1, temp1,
										0,
										0,
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[12+i],1);
			i++;
			aload_img_2(fm_buf1, temp1,
										1,
										0,
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[12+i],1);
			i++;
			aload_img_2(fm_buf1, temp1,
										2,
										0,
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[12+i],1);
			i++;
			aload_img_2(fm_buf1, temp1,
										3,
										0,
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[12+i],1);
			i++;
			aload_img_2(fm_buf1, temp1,
										4,
										0,
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[12+i],1);
			i++;
			aload_img_2(fm_buf1, temp1,
										5,
										0,
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[12+i],1);
			clear_pad3(fm_buf2,2*((320/16)+2),2*((192/16)+2));																		
			deload_img(fm_buf2, ddrdebug_3,
										1,
										1,
										1,
										2*((320/16)+2)-1,
										2*((192/16)+2)-1,
										2*((320/16)+2)
										);

//////////////////////////////////////////////////////////////////////////////	layer 371-377 ///////////////////////////////////////////////////////////////////////////////////////

			//conv://////////////////////////////////////////////////////////////////////
			aload_img_2(fm_buf1, ddrdebug_3,
										0,
										0,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
			aload_img_2(fm_buf2, ddrdebug_3,
										1,
										0,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);

			i=-1;
			i++;
			load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+75]);
			load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+75+12]);
			load_bias_from_axi( big_bias[0], bias_port[i+78]);
			load_bias_from_axi(big_bias[1], bias_port[i+90]);
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+35]);
				set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										0,
										(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);			

			i++;
			load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+75]);
			load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+75+12]);
			load_bias_from_axi( big_bias[0], bias_port[i+78]);
			load_bias_from_axi(big_bias[1], bias_port[i+90]);
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+35]);
				set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										1,
										(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);	

			i++;
			load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+75]);
			load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+75+12]);
			load_bias_from_axi( big_bias[0], bias_port[i+78]);
			load_bias_from_axi(big_bias[1], bias_port[i+90]);
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+35]);
				set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										2,
										(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);	

			i++;
			load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+75]);
			load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+75+12]);
			load_bias_from_axi( big_bias[0], bias_port[i+78]);
			load_bias_from_axi(big_bias[1], bias_port[i+90]);
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+35]);
				set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										3,
										(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);	

			i++;
			load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+75]);
			load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+75+12]);
			load_bias_from_axi( big_bias[0], bias_port[i+78]);
			load_bias_from_axi(big_bias[1], bias_port[i+90]);
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+35]);
				set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										4,
										(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);	

			i++;
			load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+75]);
			load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+75+12]);
			load_bias_from_axi( big_bias[0], bias_port[i+78]);
			load_bias_from_axi(big_bias[1], bias_port[i+90]);
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+35]);
				set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										5,
										(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);	

			i++;
			load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+75]);
			load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+75+12]);
			load_bias_from_axi( big_bias[0], bias_port[i+78]);
			load_bias_from_axi(big_bias[1], bias_port[i+90]);
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+35]);
				set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										0,
										2*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);	

			i++;
			load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+75]);
			load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+75+12]);
			load_bias_from_axi( big_bias[0], bias_port[i+78]);
			load_bias_from_axi(big_bias[1], bias_port[i+90]);
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+35]);
				set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										1,
										2*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);	

			i++;
			load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+75]);
			load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+75+12]);
			load_bias_from_axi( big_bias[0], bias_port[i+78]);
			load_bias_from_axi(big_bias[1], bias_port[i+90]);
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+35]);
				set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										2,
										2*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);	

			i++;
			load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+75]);
			load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+75+12]);
			load_bias_from_axi( big_bias[0], bias_port[i+78]);
			load_bias_from_axi(big_bias[1], bias_port[i+90]);
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+35]);
				set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										3,
										2*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);	

			i++;
			load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+75]);
			load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+75+12]);
			load_bias_from_axi( big_bias[0], bias_port[i+78]);
			load_bias_from_axi(big_bias[1], bias_port[i+90]);
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+35]);
				set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										4,
										2*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);	

			i++;
			load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+75]);
			load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+75+12]);
			load_bias_from_axi( big_bias[0], bias_port[i+78]);
			load_bias_from_axi(big_bias[1], bias_port[i+90]);
			load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+35]);
				set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										5,
										2*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);	

			//conv1x1
			load_bias_from_axi(big_bias[0], bias_port[102]);
			load_bias_from_axi(big_bias[1], bias_port[103]);

			set_bias_conv1x1(fm_buf2,big_bias[0],1,1,1,false);
			set_bias_conv1x1(fm_buf3,big_bias[1],1,1,1,false);
			i=-1;
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+99]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+111]);
			aload_img_2(fm_buf1, temp1,
										0,
										(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+99]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+111]);
			aload_img_2(fm_buf1, temp1,
										1,
										(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+99]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+111]);
			aload_img_2(fm_buf1, temp1,
										2,
										(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+99]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+111]);
			aload_img_2(fm_buf1, temp1,
										3,
										(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+99]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+111]);
			aload_img_2(fm_buf1, temp1,
										4,
										(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+99]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+111]);
			aload_img_2(fm_buf1, temp1,
										5,
										(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+99]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+111]);
			aload_img_2(fm_buf1, temp1,
										0,
										2*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);																								
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+99]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+111]);
			aload_img_2(fm_buf1, temp1,
										1,
										2*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+99]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+111]);
			aload_img_2(fm_buf1, temp1,
										2,
										2*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+99]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+111]);
			aload_img_2(fm_buf1, temp1,
										3,
										2*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+99]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+111]);
			aload_img_2(fm_buf1, temp1,
										4,
										2*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);							
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+99]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+111]);
			aload_img_2(fm_buf1, temp1,
										5,
										2*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			clear_pad3(fm_buf2,2*((320/16)+2),2*((192/16)+2));
			clear_pad3(fm_buf3,2*((320/16)+2),2*((192/16)+2));
			deload_img(fm_buf2, ddrdebug_3,
										0,
										1,
										1,
										2*((320/16)+2)-1,
										2*((192/16)+2)-1,
										2*((320/16)+2)
										);
			deload_img(fm_buf3, ddrdebug_3,
										1,
										1,
										1,
										2*((320/16)+2)-1,
										2*((192/16)+2)-1,
										2*((320/16)+2)
										);			


//////////////////////////////////////////////////////////////////////////////	layer 380-386 ///////////////////////////////////////////////////////////////////////////////////////

		//conv://////////////////////////////////////////////////////////////////////
				aload_img_2(fm_buf1, ddrdebug_3,
											0,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);
				aload_img_2(fm_buf2, ddrdebug_3,
											1,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);

				i=-1;
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+123]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+123+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+104]);
				load_bias_from_axi(big_bias[1], bias_port[i+116]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+47]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										0,
										3*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);					

				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+123]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+123+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+104]);
				load_bias_from_axi(big_bias[1], bias_port[i+116]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+47]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										1,
										3*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);

				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+123]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+123+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+104]);
				load_bias_from_axi(big_bias[1], bias_port[i+116]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+47]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										2,
										3*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);

				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+123]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+123+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+104]);
				load_bias_from_axi(big_bias[1], bias_port[i+116]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+47]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										3,
										3*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);

				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+123]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+123+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+104]);
				load_bias_from_axi(big_bias[1], bias_port[i+116]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+47]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										4,
										3*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);

				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+123]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+123+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+104]);
				load_bias_from_axi(big_bias[1], bias_port[i+116]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+47]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										5,
										3*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);

				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+123]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+123+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+104]);
				load_bias_from_axi(big_bias[1], bias_port[i+116]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+47]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										0,
										4*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);

				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+123]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+123+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+104]);
				load_bias_from_axi(big_bias[1], bias_port[i+116]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+47]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										1,
										4*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);

				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+123]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+123+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+104]);
				load_bias_from_axi(big_bias[1], bias_port[i+116]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+47]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										2,
										4*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);

				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+123]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+123+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+104]);
				load_bias_from_axi(big_bias[1], bias_port[i+116]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+47]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										3,
										4*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);

				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+123]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+123+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+104]);
				load_bias_from_axi(big_bias[1], bias_port[i+116]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+47]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										4,
										4*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);

				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+123]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+123+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+104]);
				load_bias_from_axi(big_bias[1], bias_port[i+116]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+47]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										5,
										4*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
		//part 2 conv1x1
			load_bias_from_axi(big_bias[0], bias_port[128]);
			load_bias_from_axi(big_bias[1], bias_port[129]);
			set_bias_conv1x1(fm_buf2,big_bias[0],1,1,1,false);
			set_bias_conv1x1(fm_buf3,big_bias[1],1,1,1,false);
			i=-1;
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+147]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+159]);			
			aload_img_2(fm_buf1, temp1,
										0,
										3*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);	
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+147]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+159]);			
			aload_img_2(fm_buf1, temp1,
										1,
										3*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);	
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+147]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+159]);			
			aload_img_2(fm_buf1, temp1,
										2,
										3*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);	
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+147]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+159]);			
			aload_img_2(fm_buf1, temp1,
										3,
										3*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);	
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+147]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+159]);			
			aload_img_2(fm_buf1, temp1,
										4,
										3*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);							
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+147]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+159]);			
			aload_img_2(fm_buf1, temp1,
										5,
										3*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+147]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+159]);			
			aload_img_2(fm_buf1, temp1,
										0,
										4*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+147]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+159]);			
			aload_img_2(fm_buf1, temp1,
										1,
										4*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+147]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+159]);			
			aload_img_2(fm_buf1, temp1,
										2,
										4*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+147]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+159]);			
			aload_img_2(fm_buf1, temp1,
										3,
										4*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+147]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+159]);			
			aload_img_2(fm_buf1, temp1,
										4,
										4*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+147]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+159]);			
			aload_img_2(fm_buf1, temp1,
										5,
										4*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
				clear_pad3(fm_buf2,2*((320/16)+2),2*((192/16)+2));
				clear_pad3(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				deload_img(fm_buf2, ddrdebug_3,
											0,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);
				deload_img(fm_buf3, ddrdebug_3,
											1,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);																										

//////////////////////////////////////////////////////////////////////////////	layer 389 392 395 ///////////////////////////////////////////////////////////////////////////////////////

		//conv://////////////////////////////////////////////////////////////////////
				aload_img_2(fm_buf1, ddrdebug_3,
											0,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);
				aload_img_2(fm_buf2, ddrdebug_3,
											1,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);

				i=-1;
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+171]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+171+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+130]);
				load_bias_from_axi(big_bias[1], bias_port[i+142]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+59]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										0,
										5*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+171]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+171+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+130]);
				load_bias_from_axi(big_bias[1], bias_port[i+142]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+59]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										1,
										5*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+171]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+171+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+130]);
				load_bias_from_axi(big_bias[1], bias_port[i+142]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+59]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										2,
										5*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+171]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+171+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+130]);
				load_bias_from_axi(big_bias[1], bias_port[i+142]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+59]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										3,
										5*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+171]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+171+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+130]);
				load_bias_from_axi(big_bias[1], bias_port[i+142]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+59]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										4,
										5*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+171]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+171+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+130]);
				load_bias_from_axi(big_bias[1], bias_port[i+142]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+59]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										5,
										5*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+171]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+171+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+130]);
				load_bias_from_axi(big_bias[1], bias_port[i+142]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+59]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										0,
										6*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+171]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+171+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+130]);
				load_bias_from_axi(big_bias[1], bias_port[i+142]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+59]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										1,
										6*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+171]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+171+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+130]);
				load_bias_from_axi(big_bias[1], bias_port[i+142]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+59]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										2,
										6*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+171]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+171+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+130]);
				load_bias_from_axi(big_bias[1], bias_port[i+142]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+59]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										3,
										6*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+171]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+171+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+130]);
				load_bias_from_axi(big_bias[1], bias_port[i+142]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+59]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										4,
										6*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+171]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+171+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+130]);
				load_bias_from_axi(big_bias[1], bias_port[i+142]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+59]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										5,
										6*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);

		//part2
			load_bias_from_axi(big_bias[0], bias_port[154]);
			load_bias_from_axi(big_bias[1], bias_port[155]);
			set_bias_conv1x1(fm_buf2,big_bias[0],1,1,1,false);
			set_bias_conv1x1(fm_buf3,big_bias[1],1,1,1,false);			
			i=-1;
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+195]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+207]);		
			aload_img_2(fm_buf1, temp1,
										0,
										5*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+195]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+207]);		
			aload_img_2(fm_buf1, temp1,
										1,
										5*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+195]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+207]);		
			aload_img_2(fm_buf1, temp1,
										2,
										5*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+195]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+207]);		
			aload_img_2(fm_buf1, temp1,
										3,
										5*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+195]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+207]);		
			aload_img_2(fm_buf1, temp1,
										4,
										5*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+195]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+207]);		
			aload_img_2(fm_buf1, temp1,
										5,
										5*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+195]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+207]);		
			aload_img_2(fm_buf1, temp1,
										0,
										6*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+195]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+207]);		
			aload_img_2(fm_buf1, temp1,
										1,
										6*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+195]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+207]);		
			aload_img_2(fm_buf1, temp1,
										2,
										6*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+195]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+207]);		
			aload_img_2(fm_buf1, temp1,
										3,
										6*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+195]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+207]);		
			aload_img_2(fm_buf1, temp1,
										4,
										6*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
			i++;
			load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+195]);
			load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+207]);		
			aload_img_2(fm_buf1, temp1,
										5,
										6*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
			CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
			CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);																														
				clear_pad3(fm_buf2,2*((320/16)+2),2*((192/16)+2));
				clear_pad3(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				deload_img(fm_buf2, ddrdebug_3,
											0,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);
				deload_img(fm_buf3, ddrdebug_3,
											1,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);	

//////////////////////////////////////////////////////////////////////////////	layer 398 401 404 ///////////////////////////////////////////////////////////////////////////////////////

		//conv://////////////////////////////////////////////////////////////////////
				aload_img_2(fm_buf1, ddrdebug_3,
											0,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);
				aload_img_2(fm_buf2, ddrdebug_3,
											1,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);

				initial_ddr(ddrdebug_3,3,2*((320/16)+2),2*((192/16)+2));

				i=-1;
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+219]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+219+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+156]);
				load_bias_from_axi(big_bias[1], bias_port[i+168]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+71]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										0,
										7*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+219]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+219+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+156]);
				load_bias_from_axi(big_bias[1], bias_port[i+168]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+71]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										1,
										7*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+219]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+219+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+156]);
				load_bias_from_axi(big_bias[1], bias_port[i+168]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+71]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										2,
										7*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+219]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+219+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+156]);
				load_bias_from_axi(big_bias[1], bias_port[i+168]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+71]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										3,
										7*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+219]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+219+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+156]);
				load_bias_from_axi(big_bias[1], bias_port[i+168]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+71]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										4,
										7*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+219]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+219+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+156]);
				load_bias_from_axi(big_bias[1], bias_port[i+168]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+71]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										5,
										7*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+219]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+219+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+156]);
				load_bias_from_axi(big_bias[1], bias_port[i+168]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+71]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										0,
										8*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+219]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+219+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+156]);
				load_bias_from_axi(big_bias[1], bias_port[i+168]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+71]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										1,
										8*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+219]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+219+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+156]);
				load_bias_from_axi(big_bias[1], bias_port[i+168]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+71]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										2,
										8*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+219]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+219+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+156]);
				load_bias_from_axi(big_bias[1], bias_port[i+168]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+71]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										3,
										8*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+219]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+219+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+156]);
				load_bias_from_axi(big_bias[1], bias_port[i+168]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+71]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										4,
										8*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+219]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+219+12]);
				load_bias_from_axi( big_bias[0], bias_port[i+156]);
				load_bias_from_axi(big_bias[1], bias_port[i+168]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+71]);
					set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[0],1);
			deload_img(fm_buf4, temp1,
										5,
										8*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);

		//part2
				load_bias_from_axi(big_bias[0], bias_port[180]);
				load_bias_from_axi(big_bias[1], bias_port[181]);
				load_bias_from_axi(big_bias[2], bias_port[182]);
				set_bias_conv1x1(fm_buf2,big_bias[0],1,1,1,false);
				set_bias_conv1x1(fm_buf3,big_bias[1],1,1,1,false);
				set_bias_conv1x1(fm_buf4,big_bias[2],1,1,1,false);
			i=-1;
			i++;
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+243]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+255]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+267]);
			aload_img_2(fm_buf1, temp1,
										0,
										7*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[4],1);
			i++;
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+243]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+255]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+267]);
			aload_img_2(fm_buf1, temp1,
										1,
										7*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[4],1);
			i++;
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+243]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+255]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+267]);
			aload_img_2(fm_buf1, temp1,
										2,
										7*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[4],1);
			i++;
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+243]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+255]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+267]);
			aload_img_2(fm_buf1, temp1,
										3,
										7*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[4],1);
			i++;
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+243]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+255]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+267]);
			aload_img_2(fm_buf1, temp1,
										4,
										7*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[4],1);
			i++;
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+243]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+255]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+267]);
			aload_img_2(fm_buf1, temp1,
										5,
										7*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[4],1);												
			i++;
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+243]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+255]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+267]);
			aload_img_2(fm_buf1, temp1,
										0,
										8*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[4],1);
			i++;
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+243]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+255]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+267]);
			aload_img_2(fm_buf1, temp1,
										1,
										8*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[4],1);
			i++;
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+243]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+255]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+267]);
			aload_img_2(fm_buf1, temp1,
										2,
										8*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[4],1);
			i++;
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+243]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+255]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+267]);
			aload_img_2(fm_buf1, temp1,
										3,
										8*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[4],1);
			i++;
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+243]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+255]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+267]);
			aload_img_2(fm_buf1, temp1,
										4,
										8*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[4],1);
			i++;
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+243]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+255]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+267]);
			aload_img_2(fm_buf1, temp1,
										5,
										8*(42*26),
										0,
										2*(320/16)+2,
										2*(192/16)+2,
										2*(320/16)+2
										);
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[4],1);

				clear_pad3(fm_buf2,2*((320/16)+2),2*((192/16)+2));
				clear_pad3(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				clear_pad3(fm_buf4,2*((320/16)+2),2*((192/16)+2));
				deload_img(fm_buf2, ddrdebug_3,
											0,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);
				deload_img(fm_buf3, ddrdebug_3,
											1,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);
				deload_img(fm_buf4, ddrdebug_3,
											2,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);

//////////////////////////////////////////////////////////////////////////////	layer 406 409 412 ///////////////////////////////////////////////////////////////////////////////////////

		//conv://////////////////////////////////////////////////////////////////////
				aload_img_2(fm_buf1, ddrdebug_3,
											0,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);
				aload_img_2(fm_buf2, ddrdebug_3,
											1,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);
				aload_img_2(fm_buf3, ddrdebug_3,
											2,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);

				i=-1;
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										0,
										9*(42*26),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);					
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										1,
										9*(42*26),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										2,
										9*(42*26),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										3,
										9*(42*26),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										4,
										9*(42*26),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										5,
										9*(42*26),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										0,
										9*(42*26)+44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										1,
										9*(42*26)+44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										2,
										9*(42*26)+44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										3,
										9*(42*26)+44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										4,
										9*(42*26)+44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										5,
										9*(42*26)+44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										0,
										9*(42*26)+2*44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										1,
										9*(42*26)+2*44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										2,
										9*(42*26)+2*44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										3,
										9*(42*26)+2*44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										4,
										9*(42*26)+2*44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+279]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+279+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+279+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+183]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										5,
										9*(42*26)+2*44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	

		//part2
				load_bias_from_axi(big_bias[0], bias_port[219]);
				load_bias_from_axi(big_bias[1], bias_port[220]);
				set_bias_conv1x1(fm_buf3,big_bias[0],1,1,1,false);
				set_bias_conv1x1(fm_buf4,big_bias[1],1,1,1,false);
			i=-1;
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										0,
										9*(42*26),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										0,
										9*(42*26)+3*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										1,
										9*(42*26),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										1,
										9*(42*26)+3*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										2,
										9*(42*26),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										2,
										9*(42*26)+3*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										3,
										9*(42*26),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										3,
										9*(42*26)+3*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										4,
										9*(42*26),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										4,
										9*(42*26)+3*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										5,
										9*(42*26),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										5,
										9*(42*26)+3*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										0,
										9*(42*26)+44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										0,
										9*(42*26)+4*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										1,
										9*(42*26)+44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										1,
										9*(42*26)+4*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										2,
										9*(42*26)+44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										2,
										9*(42*26)+4*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										3,
										9*(42*26)+44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										3,
										9*(42*26)+4*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										4,
										9*(42*26)+44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										4,
										9*(42*26)+4*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										5,
										9*(42*26)+44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										5,
										9*(42*26)+4*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										0,
										9*(42*26)+2*44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										0,
										9*(42*26)+5*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										1,
										9*(42*26)+2*44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										1,
										9*(42*26)+5*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										2,
										9*(42*26)+2*44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										2,
										9*(42*26)+5*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										3,
										9*(42*26)+2*44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										3,
										9*(42*26)+5*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										4,
										9*(42*26)+2*44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										4,
										9*(42*26)+5*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
			i++;
				load_bias_from_axi(big_bias[1], bias_port[i+201]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+83]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+333]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+351]);
			aload_img_2(fm_buf1, temp1,
										5,
										9*(42*26)+2*44*28,
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										5,
										9*(42*26)+5*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);																																																												
				clear_pad3(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				clear_pad3(fm_buf4,2*((320/16)+2),2*((192/16)+2));
				deload_img(fm_buf3, ddrdebug_3,
											0,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);
				deload_img(fm_buf4, ddrdebug_3,
											1,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);										
		//part 3
				load_bias_from_axi(big_bias[2], bias_port[221]);
				set_bias_conv1x1(fm_buf2,big_bias[2],1,1,1,false);
				i=-1;
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										0,
										9*(42*26)+3*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										1,
										9*(42*26)+3*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										2,
										9*(42*26)+3*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										3,
										9*(42*26)+3*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										4,
										9*(42*26)+3*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										5,
										9*(42*26)+3*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										0,
										9*(42*26)+4*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										1,
										9*(42*26)+4*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										2,
										9*(42*26)+4*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										3,
										9*(42*26)+4*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										4,
										9*(42*26)+4*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										5,
										9*(42*26)+4*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										0,
										9*(42*26)+5*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										1,
										9*(42*26)+5*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										2,
										9*(42*26)+5*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										3,
										9*(42*26)+5*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										4,
										9*(42*26)+5*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+369]);
			aload_img_2(fm_buf1, temp1,
										5,
										9*(42*26)+5*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[5],1);

				clear_pad3(fm_buf2,2*((320/16)+2),2*((192/16)+2));
				deload_img(fm_buf2, ddrdebug_3,
											2,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);				

//////////////////////////////////////////////////////////////////////////////	layer 415 418 421 ///////////////////////////////////////////////////////////////////////////////////////

		//conv://////////////////////////////////////////////////////////////////////
				aload_img_2(fm_buf1, ddrdebug_3,
											0,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);
				aload_img_2(fm_buf2, ddrdebug_3,
											1,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);
				aload_img_2(fm_buf3, ddrdebug_3,
											2,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);

				i=-1;
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										0,
										12*(42*26)+6*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										1,
										12*(42*26)+6*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										2,
										12*(42*26)+6*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										3,
										12*(42*26)+6*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										4,
										12*(42*26)+6*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										5,
										12*(42*26)+6*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										0,
										12*(42*26)+7*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										1,
										12*(42*26)+7*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										2,
										12*(42*26)+7*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										3,
										12*(42*26)+7*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										4,
										12*(42*26)+7*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										5,
										12*(42*26)+7*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										0,
										12*(42*26)+8*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										1,
										12*(42*26)+8*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										2,
										12*(42*26)+8*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										3,
										12*(42*26)+8*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										4,
										12*(42*26)+8*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+387]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+387+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+387+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+222]);
					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
			deload_img(fm_buf4, temp1,
										5,
										12*(42*26)+8*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
		//part2
				load_bias_from_axi(big_bias[0], bias_port[258]);
				load_bias_from_axi(big_bias[1], bias_port[259]);
				set_bias_conv1x1(fm_buf3,big_bias[0],1,1,1,false);
				set_bias_conv1x1(fm_buf4,big_bias[1],1,1,1,false);
				i=-1;
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										0,
										12*(42*26)+6*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										0,
										12*(42*26)+9*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										1,
										12*(42*26)+6*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										1,
										12*(42*26)+9*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										2,
										12*(42*26)+6*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										2,
										12*(42*26)+9*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										3,
										12*(42*26)+6*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										3,
										12*(42*26)+9*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										4,
										12*(42*26)+6*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										4,
										12*(42*26)+9*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										5,
										12*(42*26)+6*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										5,
										12*(42*26)+9*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);																																																		
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										0,
										12*(42*26)+7*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										0,
										12*(42*26)+10*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										1,
										12*(42*26)+7*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										1,
										12*(42*26)+10*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										2,
										12*(42*26)+7*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										2,
										12*(42*26)+10*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										3,
										12*(42*26)+7*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										3,
										12*(42*26)+10*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										4,
										12*(42*26)+7*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										4,
										12*(42*26)+10*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										5,
										12*(42*26)+7*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										5,
										12*(42*26)+10*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);	
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										0,
										12*(42*26)+8*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										0,
										12*(42*26)+11*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										1,
										12*(42*26)+8*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										1,
										12*(42*26)+11*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										2,
										12*(42*26)+8*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										2,
										12*(42*26)+11*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										3,
										12*(42*26)+8*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										3,
										12*(42*26)+11*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										4,
										12*(42*26)+8*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										4,
										12*(42*26)+11*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				i++;
				load_bias_from_axi(big_bias[1], bias_port[i+240]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+101]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+441]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+459]);
			aload_img_2(fm_buf1, temp1,
										5,
										12*(42*26)+8*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);
				set_dwbias_conv3x3(fm_buf2,   big_bias[1]);             //16  bias is load
				dw_conv_1(fm_buf1,fm_buf2,dwt_buf3_big[0],1);
				CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[3],1);
				CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],1);
			deload_img(fm_buf2, temp1,
										5,
										12*(42*26)+11*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);											

				clear_pad3(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				clear_pad3(fm_buf4,2*((320/16)+2),2*((192/16)+2));
				deload_img(fm_buf3, ddrdebug_3,
											0,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);
				deload_img(fm_buf4, ddrdebug_3,
											1,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);								
		//part3
				load_bias_from_axi(big_bias[2], bias_port[260]);
				set_bias_conv1x1(fm_buf2,big_bias[2],1,1,1,false);
				i=-1;
				i++;
			aload_img_2(fm_buf1, temp1,
										0,
										12*(42*26)+9*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										1,
										12*(42*26)+9*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										2,
										12*(42*26)+9*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										3,
										12*(42*26)+9*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										4,
										12*(42*26)+9*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										5,
										12*(42*26)+9*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);																
				i++;
			aload_img_2(fm_buf1, temp1,
										0,
										12*(42*26)+10*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										1,
										12*(42*26)+10*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										2,
										12*(42*26)+10*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										3,
										12*(42*26)+10*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										4,
										12*(42*26)+10*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										5,
										12*(42*26)+10*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										0,
										12*(42*26)+11*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										1,
										12*(42*26)+11*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										2,
										12*(42*26)+11*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										3,
										12*(42*26)+11*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										4,
										12*(42*26)+11*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);
				i++;
			aload_img_2(fm_buf1, temp1,
										5,
										12*(42*26)+11*(44*28),
										0,
										2*((320/16)+2),
										2*((192/16)+2),
										2*((320/16)+2)
										);				
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+477]);
				CONV_1x1(fm_buf1,fm_buf2,wt_buf_big[5],1);				

				clear_pad3(fm_buf2,2*((320/16)+2),2*((192/16)+2));
				deload_img(fm_buf2, ddrdebug_3,
											2,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);



//////////////////////////////////////////////////////////////////////////////	layer 424 427 430 ///////////////////////////////////////////////////////////////////////////////////////
									initial_ddr6(temp1,6,
											(23)*3,
                                            2*((192/16)+2)
											);			
					for(int i=0;i<18;i++)
					{
						//load bias for 16 opchannel
						load_bias_from_axi(big_bias[0], bias_port[i+261]);
						set_bias_conv1x1(fm_buf2,	  big_bias[0],1,1,1,false);     //16  bias is load
						for(int j=0;j<3;j++)
						{
							//load img
							aload_img_2(fm_buf1, ddrdebug_3,
														j,
														0,
														0,
														2*((320/16)+2),
														2*((192/16)+2),
														2*((320/16)+2)
														);
							//load weight for 16i op, 16j inp
							load_weight_conv1x1( wt_buf_big[0], w_port_1x1[495+18*j+i]);											
							CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],0);														
						}
						clear_pad(fm_buf2,20,12);
						clear_pad(fm_buf2,21,13);	
                        clear_pad(fm_buf2,-1,-1);	
						clear_pad(fm_buf2,42,26);	
                        //dw33
						load_bias_from_axi(big_bias[1], bias_port[i+279]);
                        set_dwbias_conv3x3(fm_buf3,   big_bias[1]);             //16  bias is load
                        load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
                        dw_conv_2(fm_buf2,fm_buf3,dwt_buf3_big[0],1);  
						int c=i/6;      //6*3
						int offsetx=(i%6)/3;  //哪一行
						int offsety=(i%6)%3;  //那一列       
                        deload_img(fm_buf3, temp1,
                                                    c,
                                                    offsety*(23)+1,
                                                    offsetx*(15)+1,
                                                    21,
                                                    13,
                                                    (23)*3
                                                    );  						
                                            																
					}										
                    for(int i=0;i<5;i++)
                    {
                        //load bias
                        //set bias for 16 op_channel
                        load_bias_from_axi( big_bias[0], bias_port[i+297]);
                        set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);                          
                        for(int j=0;j<18;j++)
                        {
                            //load 16in channel image
							int c=j/6;  
							int offsetx=(j%6)/3;  //哪一行
							int offsety=(j%6)%3;  //那一列  							
                            aload_img_2(fm_buf1, temp1,
														c,
														offsety*(2*((320/32)+2)-1),
														offsetx*(2*((192/32)+2)-1),
                                                        2*((320/32)+2)-1,
                                                        2*((192/32)+2)-1,
                                                        (2*((320/32)+2)-1)*3
                                                        );                            
                            //load 16in channel weight
                            load_weight_conv1x1( wt_buf_big[0], w_port_1x1[549+j+18*i]);	                                          
                            //do the conv and add to one buffer
                            CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],1);
                        }
                        //deload
                            clear_pad(fm_buf4,10,6);
                            clear_pad(fm_buf4,21,13);	
                            clear_pad(fm_buf4,-1,-1);                        
                        deload_img(fm_buf4, ddrdebug_4,
                                                    i,
                                                    0,
                                                    0,
                                                    2*(320/32)+3,
                                                    2*(192/32)+3,
                                                    2*(320/32)+3
                                                    );	                        
                    }  

			
			/*				
				load_bias_from_axi(big_bias[0], bias_port[297]);
				load_bias_from_axi(big_bias[1], bias_port[298]);
				load_bias_from_axi(big_bias[2], bias_port[299]);
				load_bias_from_axi(big_bias[3], bias_port[300]);
				load_bias_from_axi(big_bias[4], bias_port[301]);

				//conv://////////////////////////////////////////////////////////////////////
				aload_img_2(fm_buf1, ddrdebug_3,
											0,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);
				aload_img_2(fm_buf2, ddrdebug_3,
											1,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);
				aload_img_2(fm_buf3, ddrdebug_3,
											2,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);

				set_bias_conv1x1(fm_buf6,big_bias[0],1,1,1,false);
				set_bias_conv1x1(fm_buf7,big_bias[1],1,1,1,false);
				set_bias_conv1x1(fm_buf8,big_bias[2],1,1,1,false);

				i=-1;
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											0,
											0,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											1,
											0,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);	
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											2,
											0,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);				
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											3,
											0,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);			
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											4,
											0,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);			
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											5,
											0,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);			
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											0,
											(20+1)*(12+1),
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);			
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											1,
											(20+1)*(12+1),
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);			
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											2,
											(20+1)*(12+1),
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);		
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											3,
											(20+1)*(12+1),
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);			
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											4,
											(20+1)*(12+1),
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);		
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											5,
											(20+1)*(12+1),
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);			
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											0,
											(20+1)*(12+1)*2,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);			
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											1,
											(20+1)*(12+1)*2,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);			
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											2,
											(20+1)*(12+1)*2,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);				
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											3,
											(20+1)*(12+1)*2,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);		
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											4,
											(20+1)*(12+1)*2,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);				
				i++;
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+495]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+495+18]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+495+36]);
				load_bias_from_axi( big_bias[0], bias_port[i+261]);
				load_bias_from_axi(big_bias[1], bias_port[i+279]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+119]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+549]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+567]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+585]);

					set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf4,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf4,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf5,   big_bias[1]);             //16  bias is load
					dw_conv_2(fm_buf4,fm_buf5,dwt_buf3_big[0],1);
				deload_img(fm_buf5, temp1,
											5,
											(20+1)*(12+1)*2,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[3],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[4],1);
				CONV_1x1(fm_buf5,fm_buf8,   wt_buf_big[5],1);			

				clear_pad(fm_buf6,9,5);
				clear_pad(fm_buf7,9,5);
				clear_pad(fm_buf8,9,5);

				deload_img(fm_buf6, ddrdebug_4,
											0,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);
				deload_img(fm_buf7, ddrdebug_4,
											1,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);
				deload_img(fm_buf8, ddrdebug_4,
											2,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);
	// out index 9-10
				set_bias_conv1x1(fm_buf6,big_bias[3],1,1,1,false);
				set_bias_conv1x1(fm_buf7,big_bias[4],1,1,1,false);

				i=-1;
				i++;
				aload_img_2(fm_buf5, temp1,
											0,
											0,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);
				i++;

				aload_img_2(fm_buf5, temp1,
											1,
											0,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);		
				i++;

				aload_img_2(fm_buf5, temp1,
											2,
											0,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);				
				i++;

				aload_img_2(fm_buf5, temp1,
											3,
											0,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);				
				i++;

				aload_img_2(fm_buf5, temp1,
											4,
											0,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);				
				i++;

				aload_img_2(fm_buf5, temp1,
											5,
											0,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);			
				i++;

				aload_img_2(fm_buf5, temp1,
											0,
											(20+1)*(12+1),
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);			
				i++;

				aload_img_2(fm_buf5, temp1,
											1,
											(20+1)*(12+1),
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);			
				i++;

				aload_img_2(fm_buf5, temp1,
											2,
											(20+1)*(12+1),
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);		
				i++;
				aload_img_2(fm_buf5, temp1,
											3,
											(20+1)*(12+1),
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);			
				i++;
				aload_img_2(fm_buf5, temp1,
											4,
											(20+1)*(12+1),
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);			
				i++;
				aload_img_2(fm_buf5, temp1,
											5,
											(20+1)*(12+1),
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);			
				i++;
				aload_img_2(fm_buf5, temp1,
											0,
											(20+1)*(12+1)*2,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);		
				i++;
				aload_img_2(fm_buf5, temp1,
											1,
											(20+1)*(12+1)*2,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);			
				i++;
				aload_img_2(fm_buf5, temp1,
											2,
											(20+1)*(12+1)*2,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);				
				i++;
				aload_img_2(fm_buf5, temp1,
											3,
											(20+1)*(12+1)*2,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);		
				i++;
				aload_img_2(fm_buf5, temp1,
											4,
											(20+1)*(12+1)*2,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);				
				i++;
				aload_img_2(fm_buf5, temp1,
											5,
											(20+1)*(12+1)*2,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+603]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+621]);											
				CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[0],1);
				CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[1],1);				


				clear_pad(fm_buf6,9,5);
				clear_pad(fm_buf7,9,5);
				deload_img(fm_buf6, ddrdebug_4,
											3,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);
				deload_img(fm_buf7, ddrdebug_4,
											4,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);											
	//data in ddrdebug_4 only contains one pad between two images
*/
//////////////////////////////////////////////////////////////////////////////	layer 432 435 438 ///////////////////////////////////////////////////////////////////////////////////////
									initial_ddr6(temp1,6,
											3*(2*((320/32)+2)-1),
                                            2*(2*((192/32)+2)-1)
											);

					for(int i=0;i<30;i++)
					{
						//load bias for 16 opchannel
						load_bias_from_axi(big_bias[0], bias_port[i+302]);
						set_bias_conv1x1(fm_buf2,	  big_bias[0],1,1,1,false);     //16  bias is load
						for(int j=0;j<5;j++)
						{
							//load img
							aload_img_2(fm_buf1, ddrdebug_4,
														j,
														0,
														0,
														2*((320/32)+2)-1,
														2*((192/32)+2)-1,
														2*((320/32)+2)-1
														);
							//load weight for 16i op, 16j inp
							load_weight_conv1x1( wt_buf_big[0], w_port_1x1[639+30*j+i]);											
							CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],0);														
						}
						clear_pad(fm_buf2,10,6);
						clear_pad(fm_buf2,21,13);	
                        clear_pad(fm_buf2,-1,-1);	
                        //dw33
						load_bias_from_axi(big_bias[1], bias_port[i+332]);
                        set_dwbias_conv3x3(fm_buf3,   big_bias[1]);             //16  bias is load
                        load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
                        dw_conv_1(fm_buf2,fm_buf3,dwt_buf3_big[0],1);  
						int c=i/6;  
						int offsetx=(i%6)/3;  //哪一行
						int offsety=(i%6)%3;  //那一列       
                        deload_img(fm_buf3, temp1,
                                                    c,
                                                    offsety*(2*((320/32)+2)-1)+1,
                                                    offsetx*(2*((192/32)+2)-1)+1,
                                                    21,
                                                    13,
                                                    (2*((320/32)+2)-1)*3
                                                    );  						
                                            																
					}	
                    
                    //438   
                    for(int i=0;i<5;i++)
                    {
                        //load bias
                        //set bias for 16 op_channel
                        load_bias_from_axi( big_bias[0], bias_port[i+362]);
                        set_bias_conv1x1(fm_buf4,	  big_bias[0],1,1,1,false);                          
                        for(int j=0;j<30;j++)
                        {
                            //load 16in channel image
							int c=j/6;  
							int offsetx=(j%6)/3;  //哪一行
							int offsety=(j%6)%3;  //那一列  							
                            aload_img_2(fm_buf1, temp1,
														c,
														offsety*(2*((320/32)+2)-1),
														offsetx*(2*((192/32)+2)-1),
                                                        2*((320/32)+2)-1,
                                                        2*((192/32)+2)-1,
                                                        (2*((320/32)+2)-1)*3
                                                        );                            
                            //load 16in channel weight
                            load_weight_conv1x1( wt_buf_big[0], w_port_1x1[789+j+30*i]);	                                          
                            //do the conv and add to one buffer
                            CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],1);
                        }
                        //deload
                            clear_pad(fm_buf4,10,6);
                            clear_pad(fm_buf4,21,13);	
                            clear_pad(fm_buf4,-1,-1);                        
                        deload_img(fm_buf4, ddrdebug_4,
                                                    i,
                                                    0,
                                                    0,
                                                    2*(320/32)+3,
                                                    2*(192/32)+3,
                                                    2*(320/32)+3
                                                    );	                        
                    }   
				/*			
				load_bias_from_axi(big_bias[0], bias_port[362]);
				//conv://////////////////////////////////////////////////////////////////////
				aload_img_2(fm_buf1, ddrdebug_4,
											0,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				aload_img_2(fm_buf2, ddrdebug_4,
											1,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				aload_img_2(fm_buf3, ddrdebug_4,
											2,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				aload_img_2(fm_buf4, ddrdebug_4,
											3,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				aload_img_2(fm_buf5, ddrdebug_4,
											4,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);

				set_bias_conv1x1(fm_buf8,big_bias[0],1,1,1,false);

				i=-1;
				i++;	//0
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											0,
											21*13*3,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);
				i++;	//1
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											1,
											21*13*3,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);	
				i++;	//2
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											2,
											21*13*3,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);			
				i++;	//3
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											3,
											21*13*3,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);		
				i++;	//4
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											4,
											21*13*3,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);		
				i++;	//5
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											5,
											21*13*3,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);			
				i++;	//6
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											0,
											21*13*4,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);		
				i++;	//7
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											1,
											21*13*4,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);				
				i++;	//8
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											2,
											21*13*4,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);				
				i++;	//9
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											3,
											21*13*4,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);				
				i++;	//10
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											4,
											21*13*4,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);				
				i++;	//11
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											5,
											21*13*4,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);				
				i++;	//12
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											0,
											21*13*5,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);			
				i++;	//13
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											1,
											21*13*5,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);				
				i++;	//14
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											2,
											21*13*5,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);			
				i++;	//15
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											3,
											21*13*5,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);			
				i++;	//16
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											4,
											21*13*5,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);				
				i++;	//17
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											5,
											21*13*5,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);			
				i++;	//18
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											0,
											21*13*6,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);
				i++;	//19
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											1,
											21*13*6,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);
				i++;	//20
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											2,
											21*13*6,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);
				i++;	//21
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											3,
											21*13*6,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);
				i++;	//22
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											4,
											21*13*6,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);
				i++;	//23
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											5,
											21*13*6,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);
				i++;	//24
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											0,
											21*13*7,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);
				i++;	//25
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											1,
											21*13*7,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);
				i++;	//26
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											2,
											21*13*7,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);
				i++;	//27
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											3,
											21*13*7,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);
				i++;	//28
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											4,
											21*13*7,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);
				i++;	//29
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+639]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+639+30]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+639+60]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+639+90]);
				load_weight_conv1x1( wt_buf_big[4], w_port_1x1[i+639+120]);
				load_bias_from_axi( big_bias[0], bias_port[i+302]);
				load_bias_from_axi(big_bias[1], bias_port[i+332]);
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[i+137]);
				load_weight_conv1x1( wt_buf_big[5], w_port_1x1[i+789]);

					set_bias_conv1x1(fm_buf6,	  big_bias[0],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf6,10,6);
					clear_pad(fm_buf6,21,13);
					CONV_1x1(fm_buf1,fm_buf6,	wt_buf_big[0],0);
					CONV_1x1(fm_buf2,fm_buf6,	wt_buf_big[1],0);
					CONV_1x1(fm_buf3,fm_buf6,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf4,fm_buf6,	wt_buf_big[3],0);
					CONV_1x1(fm_buf5,fm_buf6,	wt_buf_big[4],0);
					set_dwbias_conv3x3(fm_buf7,   big_bias[1]);             //16  bias is load
					dw_conv_1(fm_buf6,fm_buf7,dwt_buf3_big[0],1);
				deload_img(fm_buf7, temp1,
											5,
											21*13*7,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);					
					CONV_1x1(fm_buf7,fm_buf8,   wt_buf_big[5],1);

				clear_pad(fm_buf8,9,5);
				deload_img(fm_buf8, ddrdebug_4,
											0,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);

				//index 2-6

				load_bias_from_axi(big_bias[1], bias_port[363]);
				load_bias_from_axi(big_bias[2], bias_port[364]);
				load_bias_from_axi(big_bias[3], bias_port[365]);
				load_bias_from_axi(big_bias[4], bias_port[366]);
				set_bias_conv1x1(fm_buf2,big_bias[1],1,1,1,false);
				set_bias_conv1x1(fm_buf3,big_bias[2],1,1,1,false);
				set_bias_conv1x1(fm_buf4,big_bias[3],1,1,1,false);
				set_bias_conv1x1(fm_buf5,big_bias[4],1,1,1,false);
				i=-1;
				i++;	//0
				aload_img_2(fm_buf1, temp1,
											0,
											21*13*3,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);
				i++;	//1
				aload_img_2(fm_buf1, temp1,
											1,
											21*13*3,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);	
				i++;	//2
				aload_img_2(fm_buf1, temp1,
											2,
											21*13*3,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);				
				i++;	//3
				aload_img_2(fm_buf1, temp1,
											3,
											21*13*3,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//4
				aload_img_2(fm_buf1, temp1,
											4,
											21*13*3,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//5
				aload_img_2(fm_buf1, temp1,
											5,
											21*13*3,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//6
				aload_img_2(fm_buf1, temp1,
											0,
											21*13*4,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//7
				aload_img_2(fm_buf1, temp1,
											1,
											21*13*4,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//8
				aload_img_2(fm_buf1, temp1,
											2,
											21*13*4,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//9
				aload_img_2(fm_buf1, temp1,
											3,
											21*13*4,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//10
				aload_img_2(fm_buf1, temp1,
											4,
											21*13*4,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//11
				aload_img_2(fm_buf1, temp1,
											5,
											21*13*4,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//12
				aload_img_2(fm_buf1, temp1,
											0,
											21*13*5,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);				
				i++;	//13
				aload_img_2(fm_buf1, temp1,
											1,
											21*13*5,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//14
				aload_img_2(fm_buf1, temp1,
											2,
											21*13*5,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//15
				aload_img_2(fm_buf1, temp1,
											3,
											21*13*5,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//16
				aload_img_2(fm_buf1, temp1,
											4,
											21*13*5,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//17
				aload_img_2(fm_buf1, temp1,
											5,
											21*13*5,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);				
				i++;	//18
				aload_img_2(fm_buf1, temp1,
											0,
											21*13*6,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);				
				i++;	//19
				aload_img_2(fm_buf1, temp1,
											1,
											21*13*6,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);				
				i++;	//20
				aload_img_2(fm_buf1, temp1,
											2,
											21*13*6,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//21
				aload_img_2(fm_buf1, temp1,
											3,
											21*13*6,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//22
				aload_img_2(fm_buf1, temp1,
											4,
											21*13*6,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//23
				aload_img_2(fm_buf1, temp1,
											5,
											21*13*6,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//24
				aload_img_2(fm_buf1, temp1,
											0,
											21*13*7,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);				
				i++;	//25
				aload_img_2(fm_buf1, temp1,
											1,
											21*13*7,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//26
				aload_img_2(fm_buf1, temp1,
											2,
											21*13*7,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//27
				aload_img_2(fm_buf1, temp1,
											3,
											21*13*7,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//28
				aload_img_2(fm_buf1, temp1,
											4,
											21*13*7,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					
				i++;	//29
				aload_img_2(fm_buf1, temp1,
											5,
											21*13*7,
											0,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+1
											);
				load_weight_conv1x1( wt_buf_big[0], w_port_1x1[i+819]);
				load_weight_conv1x1( wt_buf_big[1], w_port_1x1[i+849]);
				load_weight_conv1x1( wt_buf_big[2], w_port_1x1[i+879]);
				load_weight_conv1x1( wt_buf_big[3], w_port_1x1[i+909]);											
				CONV_1x1(fm_buf1,fm_buf2,   wt_buf_big[0],1);
				CONV_1x1(fm_buf1,fm_buf3,   wt_buf_big[1],1);
				CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[2],1);
				CONV_1x1(fm_buf1,fm_buf5,   wt_buf_big[3],1);					

				clear_pad(fm_buf2,9,5);
				clear_pad(fm_buf3,9,5);
				clear_pad(fm_buf4,9,5);
				clear_pad(fm_buf5,9,5);
				deload_img(fm_buf2, ddrdebug_4,
											1,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);
				deload_img(fm_buf3, ddrdebug_4,
											2,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);
				deload_img(fm_buf4, ddrdebug_4,
											3,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);
				deload_img(fm_buf5, ddrdebug_4,
											4,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);

*/

//////////////////////////////////////////////////////////////////////////////	layer 444 ///////////////////////////////////////////////////////////////////////////////////////
			//444 weight and bias:

				for(int i=0;i<5;i++)
				{

					load_weight_conv1x1( wt_buf_big[i], w_port_1x1[i+939]);
				}
				for(int i=0;i<5;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+5], w_port_1x1[i+944]);
				}							
				load_bias_from_axi(big_bias[0], bias_port[367]);
				load_bias_from_axi(big_bias[1], bias_port[368]);
				set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,true);     //16  bias is load
				set_bias_conv1x1(fm_buf4,	  big_bias[1],1,1,1,true);     //16  bias is load
				

				//conv://////////////////////////////////////////////////////////////////////
				aload_img_2(fm_buf1, ddrdebug_4,
											0,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
				CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[5],0);


				aload_img_2(fm_buf2, ddrdebug_4,
											1,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);
				CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[6],0);


				aload_img_2(fm_buf1, ddrdebug_4,
											2,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
				CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[7],0);                //get 16/192 output channel of 32 input channel


				aload_img_2(fm_buf1, ddrdebug_4,
											3,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[3],0);
				CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[8],0);



				aload_img_2(fm_buf1, ddrdebug_4,
											4,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[4],0);
				CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[9],0);   

				clear_pad(fm_buf3,10,6);
				clear_pad(fm_buf3,21,13);
				clear_pad(fm_buf4,10,6);
				clear_pad(fm_buf4,21,13);					
																																				
//////////////////////////////////////////////////////////////////////////////	layer 502 523 526 523 525 ///////////////////////////////////////////////////////////////////////////////////////
								//input : 32channel  size 0.5^5 of 2*imgsize  : 2*((320/32)+2)-1 2*((192/32)+2)-1
				initial_ddr6(ddrdebug_4,
										6,
										2*((320/32)+2)-1,
										2*((192/32)+2)-1
										);
				//deload buffer3,4 to ddrdebug4 4,5 (444 re)
				deload_img(fm_buf3, ddrdebug_4,
											4,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				deload_img(fm_buf4, ddrdebug_4,
											5,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				//502: the header
								int index_502b=369;
								int index_502w=0;
								int index_11;
								load_bias_from_axi(big_bias[0], bias_port[0+index_502b]);  //502
								load_bias_from_axi(big_bias[1], bias_port[1+index_502b]);  //
								load_weight_conv3x3( big_w33_buffer[0],w_port_3x3_2[0+index_502w]); //502 opchannel16 inchannel16
								load_weight_conv3x3( big_w33_buffer[1],w_port_3x3_2[1+index_502w]); //502 opchannel16 inchannel32
								load_weight_conv3x3( big_w33_buffer[2],w_port_3x3_2[2+index_502w]); //502 opchannel32 inchannel16
								load_weight_conv3x3( big_w33_buffer[3],w_port_3x3_2[3+index_502w]); //502 opchannel32 inchannel32
								set_dwbias_conv3x3(fm_buf1,   big_bias[0]);
								set_dwbias_conv3x3(fm_buf2,   big_bias[1]);
								conv3x3_502(fm_buf3,fm_buf1,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
								conv3x3_502(fm_buf4,fm_buf1,big_w33_buffer[1]);
								conv3x3_502(fm_buf3,fm_buf2,big_w33_buffer[2]);
								conv3x3_502(fm_buf4,fm_buf2,big_w33_buffer[3]);
								clearpad_for_502(fm_buf1);               //clear x=6 y=10
								clearpad_for_502(fm_buf2);
								//3,4->1,2
				//deload 502 to ddrdebug4 2,3 (502 re)
								deload_img(fm_buf1, ddrdebug_4,
															2,
															0,
															0,
															2*((320/32)+2)-1,
															2*((192/32)+2)-1,
															2*((320/32)+2)-1
															);
								deload_img(fm_buf2, ddrdebug_4,
															3,
															0,
															0,
															2*((320/32)+2)-1,
															2*((192/32)+2)-1,
															2*((320/32)+2)-1
															);																
				//523: branch 1
								load_bias_from_axi(big_bias[4], bias_port[4+index_502b]);  //523
								load_bias_from_axi(big_bias[5], bias_port[5+index_502b]);  //
								load_weight_conv3x3( big_w33_buffer[8],w_port_3x3_2[8+index_502w]); // opchannel16 inchannel16
								load_weight_conv3x3( big_w33_buffer[9],w_port_3x3_2[9+index_502w]); // opchannel16 inchannel32
								load_weight_conv3x3( big_w33_buffer[10],w_port_3x3_2[10+index_502w]); // opchannel32 inchannel16
								load_weight_conv3x3( big_w33_buffer[11],w_port_3x3_2[11+index_502w]); // opchannel32 inchannel32
								set_dwbias_conv3x3(fm_buf3,   big_bias[4]);
								set_dwbias_conv3x3(fm_buf4,   big_bias[5]);
								conv3x3_502(fm_buf1,fm_buf3,big_w33_buffer[8]);
								conv3x3_502(fm_buf2,fm_buf3,big_w33_buffer[9]);
								conv3x3_502(fm_buf1,fm_buf4,big_w33_buffer[10]);
								conv3x3_502(fm_buf2,fm_buf4,big_w33_buffer[11]);
								clearpad_for_502(fm_buf3);               //clear x=6 y=10
								clearpad_for_502(fm_buf4);
								//1,2->3,4		
				//526: op1
								load_bias_from_axi(big_bias[7], bias_port[7+index_502b]);  //525
								load_bias_from_axi(big_bias[8], bias_port[8+index_502b]);  //525
								set_bias_conv1x1(fm_buf1,	  big_bias[7],1,1,1,true);     //16  bias is load
								set_bias_conv1x1(fm_buf2,	  big_bias[8],1,1,1,true);     //16  bias is load
								clear_pad(fm_buf1,10,6);
								clear_pad(fm_buf1,21,13);
								clear_pad(fm_buf2,10,6);
								clear_pad(fm_buf2,21,13);
								load_weight_conv1x1( wt_buf_big[0], w_port_1x1[951]);
								load_weight_conv1x1( wt_buf_big[1], w_port_1x1[952]);
								load_weight_conv1x1( wt_buf_big[2], w_port_1x1[953]);
								load_weight_conv1x1( wt_buf_big[3], w_port_1x1[954]);
								CONV_1x1(fm_buf3,fm_buf1,	wt_buf_big[0],2);
								CONV_1x1(fm_buf4,fm_buf1,	wt_buf_big[1],2);
								CONV_1x1(fm_buf3,fm_buf2,	wt_buf_big[2],2);
								CONV_1x1(fm_buf4,fm_buf2,	wt_buf_big[3],2);
								//3,4->1,2
				//aload for 521:
								aload_img_2(fm_buf3, ddrdebug_4,
															2,
															0,
															0,
															2*((320/32)+2)-1,
															2*((192/32)+2)-1,
															2*((320/32)+2)-1
															);
								aload_img_2(fm_buf4, ddrdebug_4,
															3,
															0,
															0,
															2*((320/32)+2)-1,
															2*((192/32)+2)-1,
															2*((320/32)+2)-1
															);	
				//initial ddr 	
				initial_ddr(ddrdebug_4,
										3,
										2*((320/32)+2)-1,
										2*((192/32)+2)-1
										);
				//deload for 526							
								deload_img(fm_buf1, ddrdebug_4,
															0,
															0,
															0,
															2*((320/32)+2)-1,
															2*((192/32)+2)-1,
															2*((320/32)+2)-1
															);
								deload_img(fm_buf2, ddrdebug_4,
															1,
															0,
															0,
															2*((320/32)+2)-1,
															2*((192/32)+2)-1,
															2*((320/32)+2)-1
															);                       //where 526:channel 0,1	
									
				//521: branch 2:
								load_bias_from_axi(big_bias[2], bias_port[2+index_502b]);  //521
								load_bias_from_axi(big_bias[3], bias_port[3+index_502b]);  //
								load_weight_conv3x3( big_w33_buffer[4],w_port_3x3_2[4+index_502w]); // opchannel16 inchannel16
								load_weight_conv3x3( big_w33_buffer[5],w_port_3x3_2[5+index_502w]); // opchannel16 inchannel32
								load_weight_conv3x3( big_w33_buffer[6],w_port_3x3_2[6+index_502w]); // opchannel32 inchannel16
								load_weight_conv3x3( big_w33_buffer[7],w_port_3x3_2[7+index_502w]); // opchannel32 inchannel32
								set_dwbias_conv3x3(fm_buf1,   big_bias[2]);
								set_dwbias_conv3x3(fm_buf2,   big_bias[3]);
								conv3x3_502(fm_buf3,fm_buf1,big_w33_buffer[4]);
								conv3x3_502(fm_buf4,fm_buf1,big_w33_buffer[5]);
								conv3x3_502(fm_buf3,fm_buf2,big_w33_buffer[6]);
								conv3x3_502(fm_buf4,fm_buf2,big_w33_buffer[7]);
								clearpad_for_502(fm_buf1);               //clear x=6 y=10
								clearpad_for_502(fm_buf2);
								//3,4->1,2
				//525: op 2:
								load_bias_from_axi(big_bias[6], bias_port[6+index_502b]);  //525
								set_bias_conv1x1(fm_buf3,	  big_bias[6],1,1,1,false);     //16  bias is load
								load_weight_conv1x1( wt_buf_big[0], w_port_1x1[949]);
								load_weight_conv1x1( wt_buf_big[1], w_port_1x1[950]);
								CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],2);
								CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],2);
								clear_pad(fm_buf3,10,6);
								clear_pad(fm_buf3,-1,-1);
								clear_pad(fm_buf3,21,13);
								deload_img(fm_buf3, ddrdebug_4,
															2,
															0,
															0,
															2*((320/32)+2)-1,
															2*((192/32)+2)-1,
															2*((320/32)+2)-1
															);                       //where 525:channel 2, ��ͼ֮����Ϊ1
								//1,2->3				




///////////////////////////////////////////////////////////////////////  layer 443 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////  layer 443 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////  layer 443 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////  layer 443 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////  layer 443 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////  layer 443 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

								aload_img_2(fm_buf1, ddrdebug_3,
															0,
															0,
															0,
															2*((320/16)+2),
															2*((192/16)+2),
															2*((320/16)+2)
															);
							aload_img_2(fm_buf2, ddrdebug_3,
															1,
															0,
															0,
															2*((320/16)+2),
															2*((192/16)+2),
															2*((320/16)+2)
															);
							aload_img_2(fm_buf3, ddrdebug_3,
															2,
															0,
															0,
															2*((320/16)+2),
															2*((192/16)+2),
															2*((320/16)+2)
															);

							load_weight_conv1x1( wt_buf_big[0], w_port_1x1[0+955]);
							load_weight_conv1x1( wt_buf_big[1], w_port_1x1[1+955]);
							load_weight_conv1x1( wt_buf_big[2], w_port_1x1[2+955]);

							load_weight_conv1x1( wt_buf_big[3], w_port_1x1[3+955]);
							load_weight_conv1x1( wt_buf_big[4], w_port_1x1[4+955]);
							load_weight_conv1x1( wt_buf_big[5], w_port_1x1[5+955]);
							initial_ddr(ddrdebug_3,
													2,
													2*((320/16)+2),
													2*((192/16)+2)
													);
							//bias
							for(int i=0;i<2;i++)
							{
								load_bias_from_axi( big_bias[i], bias_port[i+378]);
							}
							set_bias_conv1x1(fm_buf4,big_bias[0],1,1,1,false);
							CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[0],0);
							CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[1],0);
							CONV_1x1(fm_buf3,fm_buf4,   wt_buf_big[2],0);
							clearpad_for_501(fm_buf4);
							deload_img(fm_buf4, ddrdebug_3,
														4,
														0,
														0,
														2*((320/16)+2),
														2*((192/16)+2),
														2*((320/16)+2)
														);	


							set_bias_conv1x1(fm_buf4,big_bias[1],1,1,1,false);				
							CONV_1x1(fm_buf1,fm_buf4,   wt_buf_big[3],0);
							CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[4],0);
							CONV_1x1(fm_buf3,fm_buf4,   wt_buf_big[5],0);
							clearpad_for_501(fm_buf4);
							deload_img(fm_buf4, ddrdebug_3,
														5,
														0,
														0,
														2*((320/16)+2),
														2*((192/16)+2),
														2*((320/16)+2)
														);
////////////////////////////////////////////////////////   bilinear the output of 444(in buffer3 and 4) and add to the ddrdebug_3///////////////////////////////////////////////////////
				aload_img_2(fm_buf3, ddrdebug_4,
											4,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				aload_img_2(fm_buf4, ddrdebug_4,
											5,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
							//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1
									load_oneimageto_ddr_1(fm_buf3,0,fm_buf1);
									bilinear_1(
											fm_buf1,
												fm_buf2
											);
									deload_img(fm_buf2, ddrdebug_3,
																4,
																1,
																1,
																20,
																12,
																2*((320/16)+2)
																);
									//3->1->2->ddr							
									load_oneimageto_ddr_1(fm_buf4,0,fm_buf1);															
									bilinear_1(
											fm_buf1,
												fm_buf2
											);														
									deload_img(fm_buf2, ddrdebug_3,
																5,
																1,
																1,
																20,
																12,
																2*((320/16)+2)
																);
									//4->1->2->ddr
							//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2					
									load_oneimageto_ddr_1(fm_buf3,1,fm_buf1);
									bilinear_1(
											fm_buf1,
												fm_buf2
											);		
									deload_img(fm_buf2, ddrdebug_3,
																4,
																1+2+20,
																1,
																20,
																12,
																2*((320/16)+2)
																);														
									load_oneimageto_ddr_1(fm_buf4,1,fm_buf1);
									bilinear_1(
											fm_buf1,
												fm_buf2
											);

									deload_img(fm_buf2, ddrdebug_3,
																5,
																1+2+20,
																1,
																20,
																12,
																2*((320/16)+2)
																);
							//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3
									load_oneimageto_ddr_1(fm_buf3,2,fm_buf1);
									bilinear_1(
											fm_buf1,
												fm_buf2
											);		
									deload_img(fm_buf2, ddrdebug_3,
																4,
																1,
																1+2+12,
																20,
																12,
																2*((320/16)+2)
																);													
									load_oneimageto_ddr_1(fm_buf4,2,fm_buf1);
									bilinear_1(
											fm_buf1,
												fm_buf2
											);

									deload_img(fm_buf2, ddrdebug_3,
																5,
																1,
																1+2+12,
																20,
																12,
																2*((320/16)+2)
																);						
							//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4
									load_oneimageto_ddr_1(fm_buf3,3,fm_buf1);
									bilinear_1(
											fm_buf1,
												fm_buf2
											);		
									deload_img(fm_buf2, ddrdebug_3,
																4,
																1+2+20,
																1+2+12,
																20,
																12,
																2*((320/16)+2)
																);													
									load_oneimageto_ddr_1(fm_buf4,3,fm_buf1);
									bilinear_1(
											fm_buf1,
												fm_buf2
											);

									deload_img(fm_buf2, ddrdebug_3,
																5,
																1+2+20,
																1+2+12,
																20,
																12,
																2*((320/16)+2)
																);	
///////////////////////////////////////////////////////////           layer 501 515 517 519 520 ///////////////////////////////////////////////////////////////
							//load ddrdebug_3 to buffer1,2
							aload_img_2(fm_buf1, ddrdebug_3,
														4,
														0,
														0,
														2*((320/16)+2),
														2*((192/16)+2),
														2*((320/16)+2)
														);
							aload_img_2(fm_buf2, ddrdebug_3,
														5,
														0,
														0,
														2*((320/16)+2),
														2*((192/16)+2),
														2*((320/16)+2)
														);
							//load for 501 and conv to buffer3,4:
								initial_ddr(ddrdebug_3,
														4,
														2*((320/16)+2),
														2*((192/16)+2)
														);

								index_502b=381;
								index_502w=12;
								load_bias_from_axi(big_bias[0], bias_port[0+index_502b]);  //501
								load_bias_from_axi(big_bias[1], bias_port[1+index_502b]);  //
								load_weight_conv3x3( big_w33_buffer[0],w_port_3x3_2[0+index_502w]); //502 opchannel16 inchannel16
								load_weight_conv3x3( big_w33_buffer[1],w_port_3x3_2[1+index_502w]); //502 opchannel16 inchannel32
								load_weight_conv3x3( big_w33_buffer[2],w_port_3x3_2[2+index_502w]); //502 opchannel32 inchannel16
								load_weight_conv3x3( big_w33_buffer[3],w_port_3x3_2[3+index_502w]); //502 opchannel32 inchannel32
								set_dwbias_conv3x3(fm_buf3,   big_bias[0]);
								set_dwbias_conv3x3(fm_buf4,   big_bias[1]);
								conv3x3_501(fm_buf1,fm_buf3,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
								conv3x3_501(fm_buf2,fm_buf3,big_w33_buffer[1]);
								conv3x3_501(fm_buf1,fm_buf4,big_w33_buffer[2]);
								conv3x3_501(fm_buf2,fm_buf4,big_w33_buffer[3]);
								clearpad_for_501(fm_buf3);               //clear x=6 y=10
								clearpad_for_501(fm_buf4);
								//deload for 501 to 3,2
								deload_img(fm_buf3, ddrdebug_3,
															2,
															0,
															0,
															2*((320/16)+2),
															2*((192/16)+2),
															2*((320/16)+2)
															);
								deload_img(fm_buf4, ddrdebug_3,
															3,
															0,
															0,
															2*((320/16)+2),
															2*((192/16)+2),
															2*((320/16)+2)
															);							
							//517 
								//3,4->1,2
								load_bias_from_axi(big_bias[4], bias_port[4+index_502b]);  //523
								load_bias_from_axi(big_bias[5], bias_port[5+index_502b]);  //
								load_weight_conv3x3( big_w33_buffer[8],w_port_3x3_2[8+index_502w]); // opchannel16 inchannel16
								load_weight_conv3x3( big_w33_buffer[9],w_port_3x3_2[9+index_502w]); // opchannel16 inchannel32
								load_weight_conv3x3( big_w33_buffer[10],w_port_3x3_2[10+index_502w]); // opchannel32 inchannel16
								load_weight_conv3x3( big_w33_buffer[11],w_port_3x3_2[11+index_502w]); // opchannel32 inchannel32
								set_dwbias_conv3x3(fm_buf1,   big_bias[4]);
								set_dwbias_conv3x3(fm_buf2,   big_bias[5]);
								conv3x3_501(fm_buf3,fm_buf1,big_w33_buffer[8]);
								conv3x3_501(fm_buf4,fm_buf1,big_w33_buffer[9]);
								conv3x3_501(fm_buf3,fm_buf2,big_w33_buffer[10]);
								conv3x3_501(fm_buf4,fm_buf2,big_w33_buffer[11]);
								clearpad_for_501(fm_buf1);               //clear x=6 y=10
								clearpad_for_501(fm_buf2);



							//520 1,2->3,4
								load_bias_from_axi(big_bias[7], bias_port[7+index_502b]);  //525
								load_bias_from_axi(big_bias[8], bias_port[8+index_502b]);  //525
								set_bias_conv1x1(fm_buf3,	  big_bias[7],1,1,1,true);     //16  bias is load
								set_bias_conv1x1(fm_buf4,	  big_bias[8],1,1,1,true);     //16  bias is load
								load_weight_conv1x1( wt_buf_big[0], w_port_1x1[964]);
								load_weight_conv1x1( wt_buf_big[1], w_port_1x1[965]);
								load_weight_conv1x1( wt_buf_big[2], w_port_1x1[966]);
								load_weight_conv1x1( wt_buf_big[3], w_port_1x1[967]);
								CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],2);
								CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],2);
								CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[2],2);
								CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[3],2);
								clearpad_for_501(fm_buf4);
								clearpad_for_501(fm_buf3);
							//aload for 515
								aload_img_2(fm_buf1, ddrdebug_3,
															2,
															0,
															0,
															2*((320/16)+2),
															2*((192/16)+2),
															2*((320/16)+2)
															);
								aload_img_2(fm_buf2, ddrdebug_3,
															3,
															0,
															0,
															2*((320/16)+2),
															2*((192/16)+2),
															2*((320/16)+2)
															);	
							//initial ddr and deload for 520
								initial_ddr(ddrdebug_3,
														3,
														2*((320/16)+2),
														2*((192/16)+2)
														);

								deload_img(fm_buf3, ddrdebug_3,
															0,
															0,
															0,
															2*((320/16)+2),
															2*((192/16)+2),
															2*((320/16)+2)
															);                                     //where 520 ,s =2
								deload_img(fm_buf4, ddrdebug_3,
															1,
															0,
															0,
															2*((320/16)+2),
															2*((192/16)+2),
															2*((320/16)+2)
															);

							//515: 1,2->3,4
								load_bias_from_axi(big_bias[2], bias_port[2+index_502b]);  //521
								load_bias_from_axi(big_bias[3], bias_port[3+index_502b]);  //
								load_weight_conv3x3( big_w33_buffer[4],w_port_3x3_2[4+index_502w]); // opchannel16 inchannel16
								load_weight_conv3x3( big_w33_buffer[5],w_port_3x3_2[5+index_502w]); // opchannel16 inchannel32
								load_weight_conv3x3( big_w33_buffer[6],w_port_3x3_2[6+index_502w]); // opchannel32 inchannel16
								load_weight_conv3x3( big_w33_buffer[7],w_port_3x3_2[7+index_502w]); // opchannel32 inchannel32
								set_dwbias_conv3x3(fm_buf3,   big_bias[2]);
								set_dwbias_conv3x3(fm_buf4,   big_bias[3]);
								conv3x3_501(fm_buf1,fm_buf3,big_w33_buffer[4]);
								conv3x3_501(fm_buf2,fm_buf3,big_w33_buffer[5]);
								conv3x3_501(fm_buf1,fm_buf4,big_w33_buffer[6]);
								conv3x3_501(fm_buf2,fm_buf4,big_w33_buffer[7]);
								clearpad_for_501(fm_buf3);               //clear x=6 y=10
								clearpad_for_501(fm_buf4);
							//519 3,4->1
								load_bias_from_axi(big_bias[6], bias_port[6+index_502b]);  //525
								set_bias_conv1x1(fm_buf1,	  big_bias[6],1,1,1,true);     //16  bias is load
								load_weight_conv1x1( wt_buf_big[0], w_port_1x1[962]);
								load_weight_conv1x1( wt_buf_big[1], w_port_1x1[963]);
								CONV_1x1(fm_buf3,fm_buf1,	wt_buf_big[0],2);
								CONV_1x1(fm_buf4,fm_buf1,	wt_buf_big[1],2);
								clearpad_for_501(fm_buf1);

								deload_img(fm_buf1, ddrdebug_3,
															2,
															0,
															0,
															2*((320/16)+2),
															2*((192/16)+2),
															2*((320/16)+2)
															);                                    //where 519 ,s=2

///////////////////////////////////////////////////////////////////////1,2fixed  layer 442 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////1,2fixed  layer 442 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////1,2fixed  layer 442 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////1,2fixed  layer 442 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////1,2fixed  layer 442 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

													//9,10,3,4
													//5,6
													load_weight_conv1x1( wt_buf_big[0], w_port_1x1[0+968]);
													load_weight_conv1x1( wt_buf_big[1], w_port_1x1[1+968]);
													//bias
													for(int i=0;i<2;i++)
													{
														load_bias_from_axi( big_bias[i], bias_port[i+390]);
													}
													initial_ddr6(temp1,
																				2,
																				2*((320/8)+2),
																				2*((192/8)+2)
																				);
													//img 1://img 1://img 1://img 1://img 1://img 1://img 1://img 1://img 1://img 1://img 1://img 1:
														aload_img_2(fm_buf3, ddrdebug_2,
																						0,
																						0,
																						0,
																						((320/8)+2),
																						((192/8)+2),
																						2*((320/8)+2)
																						);     //load image 1 to buffer1
														set_bias_conv1x1(fm_buf1,big_bias[0],1,1,1,false);
														set_bias_conv1x1(fm_buf2,big_bias[1],1,1,1,false);
														CONV_1x1(fm_buf3,fm_buf1,   wt_buf_big[0],0); //
														CONV_1x1(fm_buf3,fm_buf2,   wt_buf_big[1],0); //
														clear_pad(fm_buf1, 40,  24);
														clear_pad(fm_buf2, 40,  24);
														clear_pad(fm_buf1, -1,  -1);
														clear_pad(fm_buf2, -1,  -1);
														deload_img(fm_buf1, temp1,
																0,
																0,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);
														deload_img(fm_buf2, temp1,
																1,
																0,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);

													//img 2://img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2
														aload_img_2(fm_buf3, ddrdebug_2,
																						0,
																						42,
																						0,
																						((320/8)+2),
																						((192/8)+2),
																						2*((320/8)+2)
																						);     //load image 1 to buffer1
														set_bias_conv1x1(fm_buf1,big_bias[0],1,1,1,false);
														set_bias_conv1x1(fm_buf2,big_bias[1],1,1,1,false);
														CONV_1x1(fm_buf3,fm_buf1,   wt_buf_big[0],0); //
														CONV_1x1(fm_buf3,fm_buf2,   wt_buf_big[1],0); //
														clear_pad(fm_buf1, 40,  24);
														clear_pad(fm_buf2, 40,  24);
														clear_pad(fm_buf1, -1,  -1);
														clear_pad(fm_buf2, -1,  -1);


														deload_img(fm_buf1, temp1,
																0,
																42,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);
														deload_img(fm_buf2, temp1,
																1,
																42,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);

													//img 3://img 3://img 3://img 3://img 3://img 3://img 3://img 3://img 3://img 3://img 3://img 3://img 3:
														aload_img_2(fm_buf3, ddrdebug_2,
																						0,
																					0,
																					26,
																					((320/8)+2),
																					((192/8)+2),
																					2*((320/8)+2)
																						);     //load image 1 to buffer1
														set_bias_conv1x1(fm_buf1,big_bias[0],1,1,1,false);
														set_bias_conv1x1(fm_buf2,big_bias[1],1,1,1,false);
														CONV_1x1(fm_buf3,fm_buf1,   wt_buf_big[0],0); //
														CONV_1x1(fm_buf3,fm_buf2,   wt_buf_big[1],0); //
														clear_pad(fm_buf1, 40,  24);
														clear_pad(fm_buf2, 40,  24);
														clear_pad(fm_buf1, -1,  -1);
														clear_pad(fm_buf2, -1,  -1);


														deload_img(fm_buf1, temp1,
																0,
																					0,
																					26,
																					((320/8)+2),
																					((192/8)+2),
																					2*((320/8)+2)
																);
														deload_img(fm_buf2, temp1,
																1,
																					0,
																					26,
																					((320/8)+2),
																					((192/8)+2),
																					2*((320/8)+2)
																);
													//img 4://img 4://img 4://img 4://img 4://img 4://img 4://img 4://img 4://img 4://img 4://img 4:
														aload_img_2(fm_buf3, ddrdebug_2,
																						0,
															42,
															26,
																					((320/8)+2),
																					((192/8)+2),
																					2*((320/8)+2)
																						);     //load image 1 to buffer1
														set_bias_conv1x1(fm_buf1,big_bias[0],1,1,1,false);
														set_bias_conv1x1(fm_buf2,big_bias[1],1,1,1,false);
														CONV_1x1(fm_buf3,fm_buf1,   wt_buf_big[0],0); //
														CONV_1x1(fm_buf3,fm_buf2,   wt_buf_big[1],0); //
														clear_pad(fm_buf1, 40,  24);
														clear_pad(fm_buf2, 40,  24);
														clear_pad(fm_buf1, -1,  -1);
														clear_pad(fm_buf2, -1,  -1);


														deload_img(fm_buf1, temp1,
																0,
															42,
															26,
																					((320/8)+2),
																					((192/8)+2),
																					2*((320/8)+2)
																);
														deload_img(fm_buf2, temp1,
																1,
															42,
															26,
																					((320/8)+2),
																					((192/8)+2),
																					2*((320/8)+2)
																);													
///////////////////////////////////////////////////////  sample for buffer1,2 ////////////////////////////////
							//load img1 from buf1 to buf3
							//load img1 from buf2 to buf4
							//////////////////////////////////////////////////
							aload_img_2(fm_buf1, ddrdebug_3,
														4,
														0,
														0,
														2*((320/16)+2),
														2*((192/16)+2),
														2*((320/16)+2)
														);
							aload_img_2(fm_buf2, ddrdebug_3,
														5,
														0,
														0,
														2*((320/16)+2),
														2*((192/16)+2),
														2*((320/16)+2)
														);	
							//img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1:						
								load_oneimageto_ddr_2(fm_buf1,0,fm_buf3);
								//1->3->4
								bilinear_2(
										fm_buf3,
											fm_buf4
										);	
								deload_img(fm_buf4, temp1,
															0,
															1,
															1,
															40,
															24,
															2*((320/8)+2)
															);															
								//2->3->4
								load_oneimageto_ddr_2(fm_buf2,0,fm_buf3);
								bilinear_2(
										fm_buf3,
											fm_buf4
										);
								deload_img(fm_buf4, temp1,
															1,
															1,
															1,
															40,
															24,
															2*((320/8)+2)
															);
							//img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2:
								load_oneimageto_ddr_2(fm_buf1,1,fm_buf3);
								//1->3->4
								bilinear_2(
										fm_buf3,
											fm_buf4
										);	
								deload_img(fm_buf4, temp1,
															0,
															1+2+40,
															1,
															40,
															24,
															2*((320/8)+2)
															);															
								//2->3->4
								load_oneimageto_ddr_2(fm_buf2,1,fm_buf3);
								bilinear_2(
										fm_buf3,
											fm_buf4
										);
								deload_img(fm_buf4, temp1,
															1,
															1+2+40,
															1,
															40,
															24,
															2*((320/8)+2)
															);							
							//img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3:
								load_oneimageto_ddr_2(fm_buf1,2,fm_buf3);
								//1->3->4
								bilinear_2(
										fm_buf3,
											fm_buf4
										);	
								deload_img(fm_buf4, temp1,
															0,
															1,
															1+2+24,
															40,
															24,
															2*((320/8)+2)
															);															
								//2->3->4
								load_oneimageto_ddr_2(fm_buf2,2,fm_buf3);
								bilinear_2(
										fm_buf3,
											fm_buf4
										);
								deload_img(fm_buf4, temp1,
															1,
															1,
															1+2+24,
															40,
															24,
															2*((320/8)+2)
															);								
							//img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4:
								load_oneimageto_ddr_2(fm_buf1,3,fm_buf3);
								//1->3->4
								bilinear_2(
										fm_buf3,
											fm_buf4
										);	
								deload_img(fm_buf4, temp1,
															0,
														1+2+40,
														1+2+24,
															40,
															24,
															2*((320/8)+2)
															);															
								//2->3->4
								load_oneimageto_ddr_2(fm_buf2,3,fm_buf3);
								bilinear_2(
										fm_buf3,
											fm_buf4
										);
								deload_img(fm_buf4, temp1,
															1,
														1+2+40,
														1+2+24,
															40,
															24,
															2*((320/8)+2)
															);							
/////////////////////////////////////////////    layer 500 509 511  513 514 /////////////////////////////////

							initial_ddr6(ddrdebug_2,
													6,
													2*((320/4)+2),
													2*((192/4)+2)
													);
							//load 1 image to buffer 1,5
							//load 1 image to buffer 2,6
							//load 1 image to buffer 3,7
							//load 1 image to buffer 4,8
							//load weight and bias for 500
							index_502b=392;
							index_502w=24;
							load_bias_from_axi(big_bias[0], bias_port[0+index_502b]);  //500
							load_bias_from_axi(big_bias[1], bias_port[1+index_502b]);  //
							load_weight_conv3x3( big_w33_buffer[0],w_port_3x3_2[0+index_502w]); 
							load_weight_conv3x3( big_w33_buffer[1],w_port_3x3_2[1+index_502w]); 
							load_weight_conv3x3( big_w33_buffer[2],w_port_3x3_2[2+index_502w]); 
							load_weight_conv3x3( big_w33_buffer[3],w_port_3x3_2[3+index_502w]); 
							load_bias_from_axi(big_bias[2], bias_port[2+index_502b]);  //509
							load_bias_from_axi(big_bias[3], bias_port[3+index_502b]);  //
							load_weight_conv3x3( big_w33_buffer[4],w_port_3x3_2[4+index_502w]); 
							load_weight_conv3x3( big_w33_buffer[5],w_port_3x3_2[5+index_502w]); 
							load_weight_conv3x3( big_w33_buffer[6],w_port_3x3_2[6+index_502w]); 
							load_weight_conv3x3( big_w33_buffer[7],w_port_3x3_2[7+index_502w]);	
							load_bias_from_axi(big_bias[4], bias_port[4+index_502b]);  //513
							load_weight_conv1x1( wt_buf_big[0], w_port_1x1[971]);
							load_weight_conv1x1( wt_buf_big[1], w_port_1x1[972]);	
							load_bias_from_axi(big_bias[5], bias_port[397]);  			//511
							load_bias_from_axi(big_bias[6], bias_port[397+1]);  //
							load_weight_conv3x3( big_w33_buffer[8],w_port_3x3_2[8+index_502w]); 
							load_weight_conv3x3( big_w33_buffer[9],w_port_3x3_2[9+index_502w]); 
							load_weight_conv3x3( big_w33_buffer[10],w_port_3x3_2[10+index_502w]); 
							load_weight_conv3x3( big_w33_buffer[11],w_port_3x3_2[11+index_502w]);	
							load_bias_from_axi(big_bias[7], bias_port[400]);  			//514
							load_bias_from_axi(big_bias[8], bias_port[401]);  //

							load_weight_conv1x1( wt_buf_big[2], w_port_1x1[973]);
							load_weight_conv1x1( wt_buf_big[3], w_port_1x1[974]);
							load_weight_conv1x1( wt_buf_big[4], w_port_1x1[975]);
							load_weight_conv1x1( wt_buf_big[5], w_port_1x1[976]);	
							//layer 500:																								
								//img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1:
								//load for  to 1,2
								aload_img_2(fm_buf1, temp1,
																0,
																0,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								aload_img_2(fm_buf2, temp1,
																1,
																0,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								set_dwbias_conv3x3(fm_buf3,   big_bias[0]);
								set_dwbias_conv3x3(fm_buf4,   big_bias[1]);
								conv3x3_501(fm_buf1,fm_buf3,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
								conv3x3_501(fm_buf2,fm_buf3,big_w33_buffer[1]);
								conv3x3_501(fm_buf1,fm_buf4,big_w33_buffer[2]);
								conv3x3_501(fm_buf2,fm_buf4,big_w33_buffer[3]);
								clearpad_for_500(fm_buf3);               //clear x=6 y=10
								clearpad_for_500(fm_buf4);
								deload_img(fm_buf3, ddrdebug_2,
																4,
																0,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								deload_img(fm_buf4, ddrdebug_2,
																5,
																0,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								//img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2:
								//load for  to 1,2
								aload_img_2(fm_buf1, temp1,
																0,
																42,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								aload_img_2(fm_buf2, temp1,
																1,
																42,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								set_dwbias_conv3x3(fm_buf3,   big_bias[0]);
								set_dwbias_conv3x3(fm_buf4,   big_bias[1]);
								conv3x3_501(fm_buf1,fm_buf3,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
								conv3x3_501(fm_buf2,fm_buf3,big_w33_buffer[1]);
								conv3x3_501(fm_buf1,fm_buf4,big_w33_buffer[2]);
								conv3x3_501(fm_buf2,fm_buf4,big_w33_buffer[3]);
								clearpad_for_500(fm_buf3);               //clear x=6 y=10
								clearpad_for_500(fm_buf4);
								deload_img(fm_buf3, ddrdebug_2,
																4,
																42,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								deload_img(fm_buf4, ddrdebug_2,
																5,
																42,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1	
								//img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3:
								//load for  to 1,2
								aload_img_2(fm_buf1, temp1,
																0,
																0,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								aload_img_2(fm_buf2, temp1,
																1,
																0,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								set_dwbias_conv3x3(fm_buf3,   big_bias[0]);
								set_dwbias_conv3x3(fm_buf4,   big_bias[1]);
								conv3x3_501(fm_buf1,fm_buf3,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
								conv3x3_501(fm_buf2,fm_buf3,big_w33_buffer[1]);
								conv3x3_501(fm_buf1,fm_buf4,big_w33_buffer[2]);
								conv3x3_501(fm_buf2,fm_buf4,big_w33_buffer[3]);
								clearpad_for_500(fm_buf3);               //clear x=6 y=10
								clearpad_for_500(fm_buf4);
								deload_img(fm_buf3, ddrdebug_2,
																4,
																0,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								deload_img(fm_buf4, ddrdebug_2,
																5,
																0,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1		
								//img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4:
								//load for  to 1,2
								aload_img_2(fm_buf1, temp1,
																0,
																42,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								aload_img_2(fm_buf2, temp1,
																1,
																42,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								set_dwbias_conv3x3(fm_buf3,   big_bias[0]);
								set_dwbias_conv3x3(fm_buf4,   big_bias[1]);
								conv3x3_501(fm_buf1,fm_buf3,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
								conv3x3_501(fm_buf2,fm_buf3,big_w33_buffer[1]);
								conv3x3_501(fm_buf1,fm_buf4,big_w33_buffer[2]);
								conv3x3_501(fm_buf2,fm_buf4,big_w33_buffer[3]);
								clearpad_for_500(fm_buf3);               //clear x=6 y=10
								clearpad_for_500(fm_buf4);
								deload_img(fm_buf3, ddrdebug_2,
																4,
																42,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								deload_img(fm_buf4, ddrdebug_2,
																5,
																42,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1	
							//511: branch 1
								//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1	
								//3,4->1,2
								aload_img_2(fm_buf3, ddrdebug_2,
																4,
																0,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								aload_img_2(fm_buf4, ddrdebug_2,
																5,
																0,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1								
								set_dwbias_conv3x3(fm_buf1,   big_bias[5]);
								set_dwbias_conv3x3(fm_buf2,   big_bias[6]);
								conv3x3_501(fm_buf3,fm_buf1,big_w33_buffer[8]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
								conv3x3_501(fm_buf4,fm_buf1,big_w33_buffer[9]);
								conv3x3_501(fm_buf3,fm_buf2,big_w33_buffer[10]);
								conv3x3_501(fm_buf4,fm_buf2,big_w33_buffer[11]);
								clearpad_for_500(fm_buf1);               //clear x=6 y=10
								clearpad_for_500(fm_buf2);						
								// layer 514 1,2->3,4
								set_bias_conv1x1(fm_buf3,	  big_bias[7],1,1,1,false);     //16  bias is load
								set_bias_conv1x1(fm_buf4,	  big_bias[8],1,1,1,false);     //16  bias is load							
								CONV_1x1(fm_buf1, fm_buf3,	wt_buf_big[2],2);
								CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[3],2);
								CONV_1x1(fm_buf1, fm_buf4,	wt_buf_big[4],2);
								CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[5],2);
								clearpad_for_500(fm_buf3);               //clear x=6 y=10
								clearpad_for_500(fm_buf4);
								//deload for 514 7,8->ddr2
								deload_img(fm_buf3, ddrdebug_2,
																0,
																0,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								deload_img(fm_buf4, ddrdebug_2,
																1,
																0,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1	
								//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2
								//3,4->1,2
								aload_img_2(fm_buf3, ddrdebug_2,
																4,
																42,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								aload_img_2(fm_buf4, ddrdebug_2,
																5,
																42,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1								
								set_dwbias_conv3x3(fm_buf1,   big_bias[5]);
								set_dwbias_conv3x3(fm_buf2,   big_bias[6]);
								conv3x3_501(fm_buf3,fm_buf1,big_w33_buffer[8]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
								conv3x3_501(fm_buf4,fm_buf1,big_w33_buffer[9]);
								conv3x3_501(fm_buf3,fm_buf2,big_w33_buffer[10]);
								conv3x3_501(fm_buf4,fm_buf2,big_w33_buffer[11]);
								clearpad_for_500(fm_buf1);               //clear x=6 y=10
								clearpad_for_500(fm_buf2);						
								// layer 514 1,2->3,4
								set_bias_conv1x1(fm_buf3,	  big_bias[7],1,1,1,false);     //16  bias is load
								set_bias_conv1x1(fm_buf4,	  big_bias[8],1,1,1,false);     //16  bias is load							
								CONV_1x1(fm_buf1, fm_buf3,	wt_buf_big[2],2);
								CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[3],2);
								CONV_1x1(fm_buf1, fm_buf4,	wt_buf_big[4],2);
								CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[5],2);
								clearpad_for_500(fm_buf3);               //clear x=6 y=10
								clearpad_for_500(fm_buf4);
								//deload for 514 7,8->ddr2
								deload_img(fm_buf3, ddrdebug_2,
																0,
																42,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								deload_img(fm_buf4, ddrdebug_2,
																1,
																42,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1	
								//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3
								//3,4->1,2
								aload_img_2(fm_buf3, ddrdebug_2,
																4,
																0,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								aload_img_2(fm_buf4, ddrdebug_2,
																5,
																0,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1								
								set_dwbias_conv3x3(fm_buf1,   big_bias[5]);
								set_dwbias_conv3x3(fm_buf2,   big_bias[6]);
								conv3x3_501(fm_buf3,fm_buf1,big_w33_buffer[8]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
								conv3x3_501(fm_buf4,fm_buf1,big_w33_buffer[9]);
								conv3x3_501(fm_buf3,fm_buf2,big_w33_buffer[10]);
								conv3x3_501(fm_buf4,fm_buf2,big_w33_buffer[11]);
								clearpad_for_500(fm_buf1);               //clear x=6 y=10
								clearpad_for_500(fm_buf2);						
								// layer 514 1,2->3,4
								set_bias_conv1x1(fm_buf3,	  big_bias[7],1,1,1,false);     //16  bias is load
								set_bias_conv1x1(fm_buf4,	  big_bias[8],1,1,1,false);     //16  bias is load							
								CONV_1x1(fm_buf1, fm_buf3,	wt_buf_big[2],2);
								CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[3],2);
								CONV_1x1(fm_buf1, fm_buf4,	wt_buf_big[4],2);
								CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[5],2);
								clearpad_for_500(fm_buf3);               //clear x=6 y=10
								clearpad_for_500(fm_buf4);
								//deload for 514 7,8->ddr2
								deload_img(fm_buf3, ddrdebug_2,
																0,
																0,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								deload_img(fm_buf4, ddrdebug_2,
																1,
																0,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4
								//3,4->1,2
								aload_img_2(fm_buf3, ddrdebug_2,
																4,
																42,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								aload_img_2(fm_buf4, ddrdebug_2,
																5,
																42,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1								
								set_dwbias_conv3x3(fm_buf1,   big_bias[5]);
								set_dwbias_conv3x3(fm_buf2,   big_bias[6]);
								conv3x3_501(fm_buf3,fm_buf1,big_w33_buffer[8]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
								conv3x3_501(fm_buf4,fm_buf1,big_w33_buffer[9]);
								conv3x3_501(fm_buf3,fm_buf2,big_w33_buffer[10]);
								conv3x3_501(fm_buf4,fm_buf2,big_w33_buffer[11]);
								clearpad_for_500(fm_buf1);               //clear x=6 y=10
								clearpad_for_500(fm_buf2);						
								// layer 514 1,2->3,4
								set_bias_conv1x1(fm_buf3,	  big_bias[7],1,1,1,false);     //16  bias is load
								set_bias_conv1x1(fm_buf4,	  big_bias[8],1,1,1,false);     //16  bias is load							
								CONV_1x1(fm_buf1, fm_buf3,	wt_buf_big[2],2);
								CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[3],2);
								CONV_1x1(fm_buf1, fm_buf4,	wt_buf_big[4],2);
								CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[5],2);
								clearpad_for_500(fm_buf3);               //clear x=6 y=10
								clearpad_for_500(fm_buf4);
								//deload for 514 7,8->ddr2
								deload_img(fm_buf3, ddrdebug_2,
																0,
																42,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								deload_img(fm_buf4, ddrdebug_2,
																1,
																42,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1	
							//509,513			
								//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1//img1																																																																																														
								//3,4->1,2
								aload_img_2(fm_buf3, ddrdebug_2,
																4,
																0,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								aload_img_2(fm_buf4, ddrdebug_2,
																5,
																0,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1																								
								//layer 509: img 3,4 -> 1,2
								set_dwbias_conv3x3(fm_buf1,   big_bias[2]);
								set_dwbias_conv3x3(fm_buf2,   big_bias[3]);
								conv3x3_501(fm_buf3,fm_buf1,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
								conv3x3_501(fm_buf4,fm_buf1,big_w33_buffer[5]);
								conv3x3_501(fm_buf3,fm_buf2,big_w33_buffer[6]);
								conv3x3_501(fm_buf4,fm_buf2,big_w33_buffer[7]);
								clearpad_for_500(fm_buf1);               //clear x=6 y=10
								clearpad_for_500(fm_buf2);
								//layer 513: 1,2->4				
								set_bias_conv1x1(fm_buf4,	  big_bias[4],1,1,1,false);     //16  bias is load
								CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],2);
								CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],2);	
								clearpad_for_500(fm_buf4);						
								//deload for513  4 -> ddr2
								deload_img(fm_buf4, ddrdebug_2,
																2,
																0,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2//img2
								//3,4->1,2
								aload_img_2(fm_buf3, ddrdebug_2,
																4,
																42,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								aload_img_2(fm_buf4, ddrdebug_2,
																5,
																42,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1																								
								//layer 509: img 3,4 -> 1,2
								set_dwbias_conv3x3(fm_buf1,   big_bias[2]);
								set_dwbias_conv3x3(fm_buf2,   big_bias[3]);
								conv3x3_501(fm_buf3,fm_buf1,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
								conv3x3_501(fm_buf4,fm_buf1,big_w33_buffer[5]);
								conv3x3_501(fm_buf3,fm_buf2,big_w33_buffer[6]);
								conv3x3_501(fm_buf4,fm_buf2,big_w33_buffer[7]);
								clearpad_for_500(fm_buf1);               //clear x=6 y=10
								clearpad_for_500(fm_buf2);
								//layer 513: 1,2->4				
								set_bias_conv1x1(fm_buf4,	  big_bias[4],1,1,1,false);     //16  bias is load
								CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],2);
								CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],2);	
								clearpad_for_500(fm_buf4);						
								//deload for513  4 -> ddr2
								deload_img(fm_buf4, ddrdebug_2,
																2,
																42,
																0,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1								
								//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3//img3
								//3,4->1,2
								aload_img_2(fm_buf3, ddrdebug_2,
																4,
																0,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								aload_img_2(fm_buf4, ddrdebug_2,
																5,
																0,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1																								
								//layer 509: img 3,4 -> 1,2
								set_dwbias_conv3x3(fm_buf1,   big_bias[2]);
								set_dwbias_conv3x3(fm_buf2,   big_bias[3]);
								conv3x3_501(fm_buf3,fm_buf1,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
								conv3x3_501(fm_buf4,fm_buf1,big_w33_buffer[5]);
								conv3x3_501(fm_buf3,fm_buf2,big_w33_buffer[6]);
								conv3x3_501(fm_buf4,fm_buf2,big_w33_buffer[7]);
								clearpad_for_500(fm_buf1);               //clear x=6 y=10
								clearpad_for_500(fm_buf2);
								//layer 513: 1,2->4				
								set_bias_conv1x1(fm_buf4,	  big_bias[4],1,1,1,false);     //16  bias is load
								CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],2);
								CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],2);	
								clearpad_for_500(fm_buf4);						
								//deload for513  4 -> ddr2
								deload_img(fm_buf4, ddrdebug_2,
																2,
																0,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1								
								//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4//img4		
								//3,4->1,2
								aload_img_2(fm_buf3, ddrdebug_2,
																4,
																42,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1
								aload_img_2(fm_buf4, ddrdebug_2,
																5,
																42,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1																								
								//layer 509: img 3,4 -> 1,2
								set_dwbias_conv3x3(fm_buf1,   big_bias[2]);
								set_dwbias_conv3x3(fm_buf2,   big_bias[3]);
								conv3x3_501(fm_buf3,fm_buf1,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
								conv3x3_501(fm_buf4,fm_buf1,big_w33_buffer[5]);
								conv3x3_501(fm_buf3,fm_buf2,big_w33_buffer[6]);
								conv3x3_501(fm_buf4,fm_buf2,big_w33_buffer[7]);
								clearpad_for_500(fm_buf1);               //clear x=6 y=10
								clearpad_for_500(fm_buf2);
								//layer 513: 1,2->4				
								set_bias_conv1x1(fm_buf4,	  big_bias[4],1,1,1,false);     //16  bias is load
								CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[0],2);
								CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[1],2);	
								clearpad_for_500(fm_buf4);						
								//deload for513  4 -> ddr2
								deload_img(fm_buf4, ddrdebug_2,
																2,
																42,
																26,
																((320/8)+2),
																((192/8)+2),
																2*((320/8)+2)
																);     //load image 1 to buffer1											
							
//////////////////////////////////////////         layer 441 //////////////////////////////////////////////////////////////
//////////////////////////////////////////         layer 441 //////////////////////////////////////////////////////////////
//////////////////////////////////////////         layer 441 //////////////////////////////////////////////////////////////
//////////////////////////////////////////         layer 441 //////////////////////////////////////////////////////////////
//////////////////////////////////////////         layer 441 //////////////////////////////////////////////////////////////
//////////////////////////////////////////         layer 441 //////////////////////////////////////////////////////////////
//////////////////////////////////////////         layer 441 //////////////////////////////////////////////////////////////
							load_weight_conv1x1( wt_buf_big[0], w_port_1x1[0+978]);
							load_weight_conv1x1( wt_buf_big[1], w_port_1x1[1+978]);
							//bias
							for(int i=0;i<2;i++)
							{
								load_bias_from_axi( big_bias[i], bias_port[i+402]);
							}

							//img 1//img 1//img 1//img 1//img 1//img 1//img 1//img 1//img 1//img 1//img 1//img 1//img 1//img 1//img 1//img 1//img 1//img 1
							//2->3,4
									aload_img_2(fm_buf2, ddrdebug,
																	0,
																	0,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1							
									set_bias_conv1x1(fm_buf3,big_bias[0],1,1,1,true);
									set_bias_conv1x1(fm_buf4,big_bias[1],1,1,1,true);
									//clear_pad(fm_buf5, 80,  48);
									//clear_pad(fm_buf6, 80,  48);
									CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[0],0); //
									CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[1],0); //
									deload_img(fm_buf3, temp1,
											2,
											0,
											0,
											((320/4)+2),
											((192/4)+2),
											2*((320/4)+2)
											);
									deload_img(fm_buf4, temp1,
											3,
											0,
											0,
											((320/4)+2),
											((192/4)+2),
											2*((320/4)+2)
											);

							//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2//img 2
									aload_img_2(fm_buf2, ddrdebug,
																	0,
																	82,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1							
									set_bias_conv1x1(fm_buf3,big_bias[0],1,1,1,true);
									set_bias_conv1x1(fm_buf4,big_bias[1],1,1,1,true);
									//clear_pad(fm_buf5, 80,  48);
									//clear_pad(fm_buf6, 80,  48);
									CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[0],0); //
									CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[1],0); //
									deload_img(fm_buf3, temp1,
											2,
											82,
											0,
											((320/4)+2),
											((192/4)+2),
											2*((320/4)+2)
											);
									deload_img(fm_buf4, temp1,
											3,
											82,
											0,
											((320/4)+2),
											((192/4)+2),
											2*((320/4)+2)
											);

							//img 3//img 3//img 3//img 3//img 3//img 3//img 3//img 3//img 3//img 3//img 3//img 3//img 3//img 3//img 3//img 3//img 3//img 3//img 3
									aload_img_2(fm_buf2, ddrdebug,
																	0,
																	0,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1							
									set_bias_conv1x1(fm_buf3,big_bias[0],1,1,1,true);
									set_bias_conv1x1(fm_buf4,big_bias[1],1,1,1,true);
									//clear_pad(fm_buf5, 80,  48);
									//clear_pad(fm_buf6, 80,  48);
									CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[0],0); //
									CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[1],0); //


									deload_img(fm_buf3, temp1,
											2,
											0,
											50,
											((320/4)+2),
											((192/4)+2),
											2*((320/4)+2)
											);
									deload_img(fm_buf4, temp1,
											3,
											0,
											50,
											((320/4)+2),
											((192/4)+2),
											2*((320/4)+2)
											);

							//img 4//img 4//img 4//img 4//img 4//img 4//img 4//img 4//img 4//img 4//img 4//img 4//img 4//img 4//img 4//img 4//img 4//img 4//img 4
									aload_img_2(fm_buf2, ddrdebug,
																	0,
																	82,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1							
									set_bias_conv1x1(fm_buf3,big_bias[0],1,1,1,true);
									set_bias_conv1x1(fm_buf4,big_bias[1],1,1,1,true);
									//clear_pad(fm_buf3, 80,  48);
									//clear_pad(fm_buf4, 80,  48);
									CONV_1x1(fm_buf2,fm_buf3,   wt_buf_big[0],0); //
									CONV_1x1(fm_buf2,fm_buf4,   wt_buf_big[1],0); //


									deload_img(fm_buf3, temp1,
											2,
											82,
											50,
											((320/4)+2),
											((192/4)+2),
											2*((320/4)+2)
											);
									deload_img(fm_buf4, temp1,
											3,
											82,
											50,
											((320/4)+2),
											((192/4)+2),
											2*((320/4)+2)
											);

									
////////////////////////////////////// ups for 497 //////////////////////////////////////////////////////////////////////
														//load img1 from buf1 to buf3
														//load img1 from buf2 to buf4
											//////////////////////////////////////////////////
														aload_img_2(fm_buf3, temp1,
																0,
																1,
																1,
																((320/8)),
																((192/8)),
																2*((320/8)+2)
																);
														aload_img_2(fm_buf4, temp1,
																1,
																1,
																1,
																((320/8)),
																((192/8)),
																2*((320/8)+2)
																);
														//bilinear buf3 to buf7
														//bilinear bug4 to buf8
														//store to ddr
														bilinear_3(
																							fm_buf3,
																								fm_buf1
																							);
														bilinear_3(
																							fm_buf4,
																								fm_buf2
																							);
														deload_img(fm_buf1, temp1,
																					2,
																					1,
																					1,
																					80,
																					48,
																					2*((320/4)+2)
																					);
														deload_img(fm_buf2, temp1,
																					3,
																					1,
																					1,
																					80,
																					48,
																					2*((320/4)+2)
																					);
											//////////////////////////////////////////////////
														aload_img_2(fm_buf3, temp1,
																0,
																43,
																1,
																((320/8)),
																((192/8)),
																2*((320/8)+2)
																);
														aload_img_2(fm_buf4, temp1,
																1,
																43,
																1,
																((320/8)),
																((192/8)),
																2*((320/8)+2)
																);
														//bilinear buf3 to buf7
														//bilinear bug4 to buf8
														//store to ddr
														bilinear_3(
																							fm_buf3,
																								fm_buf1
																							);
														bilinear_3(
																							fm_buf4,
																								fm_buf2
																							);
														deload_img(fm_buf1, temp1,
																					2,
																					1+2+80,
																					1,
																					80,
																					48,
																					2*((320/4)+2)
																					);
														deload_img(fm_buf2, temp1,
																					3,
																					1+2+80,
																					1,
																					80,
																					48,
																					2*((320/4)+2)
																					);
											//////////////////////////////////////////////////
														aload_img_2(fm_buf3, temp1,
																0,
																1,
																1+2+24,
																((320/8)),
																((192/8)),
																2*((320/8)+2)
																);
														aload_img_2(fm_buf4, temp1,
																1,
																1,
																1+2+24,
																((320/8)),
																((192/8)),
																2*((320/8)+2)
																);
														//bilinear buf3 to buf7
														//bilinear bug4 to buf8
														//store to ddr
														bilinear_3(
																							fm_buf3,
																								fm_buf1
																							);
														bilinear_3(
																							fm_buf4,
																								fm_buf2
																							);
														deload_img(fm_buf1, temp1,
																					2,
																					1,
																					1+2+48,
																					80,
																					48,
																					2*((320/4)+2)
																					);
														deload_img(fm_buf2, temp1,
																					3,
																					1,
																					1+2+48,
																					80,
																					48,
																					2*((320/4)+2)
																					);
											//////////////////////////////////////////////////
														aload_img_2(fm_buf3, temp1,
																0,
																1+2+40,
																1+2+24,
																((320/8)),
																((192/8)),
																2*((320/8)+2)
																);
														aload_img_2(fm_buf4, temp1,
																1,
																1+2+40,
																1+2+24,
																((320/8)),
																((192/8)),
																2*((320/8)+2)
																);
														//bilinear buf3 to buf7
														//bilinear bug4 to buf8
														//store to ddr
														bilinear_3(
																							fm_buf3,
																								fm_buf1
																							);
														bilinear_3(
																							fm_buf4,
																								fm_buf2
																							);
														deload_img(fm_buf1, temp1,
																					2,
																					1+2+80,
																					1+2+48,
																					80,
																					48,
																					2*((320/4)+2)
																					);
														deload_img(fm_buf2, temp1,
																					3,
																					1+2+80,
																					1+2+48,
																					80,
																					48,
																					2*((320/4)+2)
																					);
																					
////////////////////////////////////////////	499 503 505 507 508 ///////////////////////////////////////////////////
								//loadweight:
									//499
									index_502b=404;
									index_502w=36;
									load_bias_from_axi(big_bias[0], bias_port[0+index_502b]);  
									load_bias_from_axi(big_bias[1], bias_port[1+index_502b]);  
									load_weight_conv3x3( big_w33_buffer[0],w_port_3x3_2[0+index_502w]); 
									load_weight_conv3x3( big_w33_buffer[1],w_port_3x3_2[1+index_502w]); 
									load_weight_conv3x3( big_w33_buffer[2],w_port_3x3_2[2+index_502w]); 
									load_weight_conv3x3( big_w33_buffer[3],w_port_3x3_2[3+index_502w]); 	
									//503
									load_bias_from_axi(big_bias[2], bias_port[2+index_502b]);  //500
									load_bias_from_axi(big_bias[3], bias_port[3+index_502b]);  //
									load_weight_conv3x3( big_w33_buffer[4],w_port_3x3_2[4+index_502w]); //502 opchannel16 inchannel16
									load_weight_conv3x3( big_w33_buffer[5],w_port_3x3_2[5+index_502w]); //502 opchannel16 inchannel32
									load_weight_conv3x3( big_w33_buffer[6],w_port_3x3_2[6+index_502w]); //502 opchannel32 inchannel16
									load_weight_conv3x3( big_w33_buffer[7],w_port_3x3_2[7+index_502w]);									
									//507
									load_bias_from_axi(big_bias[4], bias_port[4+index_502b]);  //525
									load_weight_conv1x1( wt_buf_big[0], w_port_1x1[981]);
									load_weight_conv1x1( wt_buf_big[1], w_port_1x1[982]);									
									//505								
									load_bias_from_axi(big_bias[5], bias_port[410]);  //500
									load_bias_from_axi(big_bias[6], bias_port[410+1]);  //
									load_weight_conv3x3( big_w33_buffer[8],w_port_3x3_2[8+index_502w]); //502 opchannel16 inchannel16
									load_weight_conv3x3( big_w33_buffer[9],w_port_3x3_2[9+index_502w]); //502 opchannel16 inchannel32
									load_weight_conv3x3( big_w33_buffer[10],w_port_3x3_2[10+index_502w]); //502 opchannel32 inchannel16
									load_weight_conv3x3( big_w33_buffer[11],w_port_3x3_2[11+index_502w]);
									//508
									load_bias_from_axi(big_bias[7], bias_port[412]);  //525
									load_bias_from_axi(big_bias[8], bias_port[413]);  //525

									load_weight_conv1x1( wt_buf_big[2], w_port_1x1[983]);
									load_weight_conv1x1( wt_buf_big[3], w_port_1x1[984]);
									load_weight_conv1x1( wt_buf_big[4], w_port_1x1[985]);
									load_weight_conv1x1( wt_buf_big[5], w_port_1x1[986]);
									initial_ddr6(ddrdebug,
															6,
															2*((320/4)+2),
															2*((192/4)+2)
															);									
								//img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1://img1:
								//load to 1,4
									aload_img_2(fm_buf1, temp1,
																	2,
																	0,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									aload_img_2(fm_buf4, temp1,
																	3,
																	0,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1	
								//499 1,4->2,3	 fix
									set_dwbias_conv3x3(fm_buf2,   big_bias[0]);
									set_dwbias_conv3x3(fm_buf3,   big_bias[1]);
									conv3x3_499(fm_buf1,fm_buf2,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf4,fm_buf2,big_w33_buffer[1]);
									conv3x3_499(fm_buf1,fm_buf3,big_w33_buffer[2]);
									conv3x3_499(fm_buf4,fm_buf3,big_w33_buffer[3]);
									clearpad_for_499(fm_buf2);               //clear x=6 y=10
									clearpad_for_499(fm_buf3);
								//deload for 499
									deload_img(fm_buf2, ddrdebug,
																	3,
																	0,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1		
									deload_img(fm_buf3, ddrdebug,
																	4,
																	0,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1																							
								//503 2,3->1,4
									set_dwbias_conv3x3(fm_buf1,   big_bias[2]);
									set_dwbias_conv3x3(fm_buf4,   big_bias[3]);
									conv3x3_499(fm_buf2,fm_buf1,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf3,fm_buf1,big_w33_buffer[5]);
									conv3x3_499(fm_buf2,fm_buf4,big_w33_buffer[6]);
									conv3x3_499(fm_buf3,fm_buf4,big_w33_buffer[7]);
									clearpad_for_499(fm_buf1);               //clear x=6 y=10
									clearpad_for_499(fm_buf4);
								//507 1,4->2
									set_bias_conv1x1(fm_buf2,	  big_bias[4],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf8,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[0],2);
									CONV_1x1(fm_buf4,fm_buf2,	wt_buf_big[1],2);
								//deload
									deload_img(fm_buf2, ddrdebug,
																	2,
																	0,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
								//505 2,3->1,4
									aload_img_2(fm_buf2, ddrdebug,
																	3,
																	0,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1		
									aload_img_2(fm_buf3, ddrdebug,
																	4,
																	0,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1								
									set_dwbias_conv3x3(fm_buf1,   big_bias[5]);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6]);
									conv3x3_499(fm_buf2,fm_buf1,big_w33_buffer[8]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf3,fm_buf1,big_w33_buffer[9]);
									conv3x3_499(fm_buf2,fm_buf4,big_w33_buffer[10]);
									conv3x3_499(fm_buf3,fm_buf4,big_w33_buffer[11]);
									clearpad_for_499(fm_buf1);               //clear x=6 y=10
									clearpad_for_499(fm_buf4);							
								//508 1,4->2,3
									set_bias_conv1x1(fm_buf2,	  big_bias[7],1,1,1,true);     //16  bias is load
									set_bias_conv1x1(fm_buf3,	  big_bias[8],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf7,40,24);
									//clear_pad(fm_buf8,40,24);
									CONV_1x1(fm_buf1, fm_buf2,	wt_buf_big[2],2);
									CONV_1x1(fm_buf4,fm_buf2,	wt_buf_big[3],2);
									CONV_1x1(fm_buf1, fm_buf3,	wt_buf_big[4],2);
									CONV_1x1(fm_buf4,fm_buf3,	wt_buf_big[5],2);
								//deload
									deload_img(fm_buf2, ddrdebug,
																	0,
																	0,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									deload_img(fm_buf3, ddrdebug,
																	1,
																	0,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1																									
								
								//img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2://img2:
								//load to 1,4
									aload_img_2(fm_buf1, temp1,
																	2,
																	82,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									aload_img_2(fm_buf4, temp1,
																	3,
																	82,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1	
								//499 1,4->2,3	 fix
									set_dwbias_conv3x3(fm_buf2,   big_bias[0]);
									set_dwbias_conv3x3(fm_buf3,   big_bias[1]);
									conv3x3_499(fm_buf1,fm_buf2,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf4,fm_buf2,big_w33_buffer[1]);
									conv3x3_499(fm_buf1,fm_buf3,big_w33_buffer[2]);
									conv3x3_499(fm_buf4,fm_buf3,big_w33_buffer[3]);
									clearpad_for_499(fm_buf2);               //clear x=6 y=10
									clearpad_for_499(fm_buf3);
								//deload for 499
									deload_img(fm_buf2, ddrdebug,
																	3,
																	82,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1		
									deload_img(fm_buf3, ddrdebug,
																	4,
																	82,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1																							
								//503 2,3->1,4
									set_dwbias_conv3x3(fm_buf1,   big_bias[2]);
									set_dwbias_conv3x3(fm_buf4,   big_bias[3]);
									conv3x3_499(fm_buf2,fm_buf1,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf3,fm_buf1,big_w33_buffer[5]);
									conv3x3_499(fm_buf2,fm_buf4,big_w33_buffer[6]);
									conv3x3_499(fm_buf3,fm_buf4,big_w33_buffer[7]);
									clearpad_for_499(fm_buf1);               //clear x=6 y=10
									clearpad_for_499(fm_buf4);
								//507 1,4->2
									set_bias_conv1x1(fm_buf2,	  big_bias[4],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf8,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[0],2);
									CONV_1x1(fm_buf4,fm_buf2,	wt_buf_big[1],2);
								//deload
									deload_img(fm_buf2, ddrdebug,
																	2,
																	82,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
								//505 2,3->1,4
									aload_img_2(fm_buf2, ddrdebug,
																	3,
																	82,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1		
									aload_img_2(fm_buf3, ddrdebug,
																	4,
																	82,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1								
									set_dwbias_conv3x3(fm_buf1,   big_bias[5]);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6]);
									conv3x3_499(fm_buf2,fm_buf1,big_w33_buffer[8]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf3,fm_buf1,big_w33_buffer[9]);
									conv3x3_499(fm_buf2,fm_buf4,big_w33_buffer[10]);
									conv3x3_499(fm_buf3,fm_buf4,big_w33_buffer[11]);
									clearpad_for_499(fm_buf1);               //clear x=6 y=10
									clearpad_for_499(fm_buf4);							
								//508 1,4->2,3
									set_bias_conv1x1(fm_buf2,	  big_bias[7],1,1,1,true);     //16  bias is load
									set_bias_conv1x1(fm_buf3,	  big_bias[8],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf7,40,24);
									//clear_pad(fm_buf8,40,24);
									CONV_1x1(fm_buf1, fm_buf2,	wt_buf_big[2],2);
									CONV_1x1(fm_buf4,fm_buf2,	wt_buf_big[3],2);
									CONV_1x1(fm_buf1, fm_buf3,	wt_buf_big[4],2);
									CONV_1x1(fm_buf4,fm_buf3,	wt_buf_big[5],2);
								//deload
									deload_img(fm_buf2, ddrdebug,
																	0,
																	82,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									deload_img(fm_buf3, ddrdebug,
																	1,
																	82,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1										

								//img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3://img3:
								//load to 1,4
									aload_img_2(fm_buf1, temp1,
																	2,
																	0,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									aload_img_2(fm_buf4, temp1,
																	3,
																	0,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1	
								//499 1,4->2,3	 fix
									set_dwbias_conv3x3(fm_buf2,   big_bias[0]);
									set_dwbias_conv3x3(fm_buf3,   big_bias[1]);
									conv3x3_499(fm_buf1,fm_buf2,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf4,fm_buf2,big_w33_buffer[1]);
									conv3x3_499(fm_buf1,fm_buf3,big_w33_buffer[2]);
									conv3x3_499(fm_buf4,fm_buf3,big_w33_buffer[3]);
									clearpad_for_499(fm_buf2);               //clear x=6 y=10
									clearpad_for_499(fm_buf3);
								//deload for 499
									deload_img(fm_buf2, ddrdebug,
																	3,
																	0,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1		
									deload_img(fm_buf3, ddrdebug,
																	4,
																	0,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1																							
								//503 2,3->1,4
									set_dwbias_conv3x3(fm_buf1,   big_bias[2]);
									set_dwbias_conv3x3(fm_buf4,   big_bias[3]);
									conv3x3_499(fm_buf2,fm_buf1,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf3,fm_buf1,big_w33_buffer[5]);
									conv3x3_499(fm_buf2,fm_buf4,big_w33_buffer[6]);
									conv3x3_499(fm_buf3,fm_buf4,big_w33_buffer[7]);
									clearpad_for_499(fm_buf1);               //clear x=6 y=10
									clearpad_for_499(fm_buf4);
								//507 1,4->2
									set_bias_conv1x1(fm_buf2,	  big_bias[4],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf8,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[0],2);
									CONV_1x1(fm_buf4,fm_buf2,	wt_buf_big[1],2);
								//deload
									deload_img(fm_buf2, ddrdebug,
																	2,
																	0,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
								//505 2,3->1,4
									aload_img_2(fm_buf2, ddrdebug,
																	3,
																	0,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1		
									aload_img_2(fm_buf3, ddrdebug,
																	4,
																	0,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1								
									set_dwbias_conv3x3(fm_buf1,   big_bias[5]);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6]);
									conv3x3_499(fm_buf2,fm_buf1,big_w33_buffer[8]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf3,fm_buf1,big_w33_buffer[9]);
									conv3x3_499(fm_buf2,fm_buf4,big_w33_buffer[10]);
									conv3x3_499(fm_buf3,fm_buf4,big_w33_buffer[11]);
									clearpad_for_499(fm_buf1);               //clear x=6 y=10
									clearpad_for_499(fm_buf4);							
								//508 1,4->2,3
									set_bias_conv1x1(fm_buf2,	  big_bias[7],1,1,1,true);     //16  bias is load
									set_bias_conv1x1(fm_buf3,	  big_bias[8],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf7,40,24);
									//clear_pad(fm_buf8,40,24);
									CONV_1x1(fm_buf1, fm_buf2,	wt_buf_big[2],2);
									CONV_1x1(fm_buf4,fm_buf2,	wt_buf_big[3],2);
									CONV_1x1(fm_buf1, fm_buf3,	wt_buf_big[4],2);
									CONV_1x1(fm_buf4,fm_buf3,	wt_buf_big[5],2);
								//deload
									deload_img(fm_buf2, ddrdebug,
																	0,
																	0,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									deload_img(fm_buf3, ddrdebug,
																	1,
																	0,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1	

								//img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4://img4:
								//load to 1,4
									aload_img_2(fm_buf1, temp1,
																	2,
																	82,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									aload_img_2(fm_buf4, temp1,
																	3,
																	82,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1	
								//499 1,4->2,3	 fix
									set_dwbias_conv3x3(fm_buf2,   big_bias[0]);
									set_dwbias_conv3x3(fm_buf3,   big_bias[1]);
									conv3x3_499(fm_buf1,fm_buf2,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf4,fm_buf2,big_w33_buffer[1]);
									conv3x3_499(fm_buf1,fm_buf3,big_w33_buffer[2]);
									conv3x3_499(fm_buf4,fm_buf3,big_w33_buffer[3]);
									clearpad_for_499(fm_buf2);               //clear x=6 y=10
									clearpad_for_499(fm_buf3);
								//deload for 499
									deload_img(fm_buf2, ddrdebug,
																	3,
																	82,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1		
									deload_img(fm_buf3, ddrdebug,
																	4,
																	82,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1																							
								//503 2,3->1,4
									set_dwbias_conv3x3(fm_buf1,   big_bias[2]);
									set_dwbias_conv3x3(fm_buf4,   big_bias[3]);
									conv3x3_499(fm_buf2,fm_buf1,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf3,fm_buf1,big_w33_buffer[5]);
									conv3x3_499(fm_buf2,fm_buf4,big_w33_buffer[6]);
									conv3x3_499(fm_buf3,fm_buf4,big_w33_buffer[7]);
									clearpad_for_499(fm_buf1);               //clear x=6 y=10
									clearpad_for_499(fm_buf4);
								//507 1,4->2
									set_bias_conv1x1(fm_buf2,	  big_bias[4],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf8,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[0],2);
									CONV_1x1(fm_buf4,fm_buf2,	wt_buf_big[1],2);
								//deload
									deload_img(fm_buf2, ddrdebug,
																	2,
																	82,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
								//505 2,3->1,4
									aload_img_2(fm_buf2, ddrdebug,
																	3,
																	82,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1		
									aload_img_2(fm_buf3, ddrdebug,
																	4,
																	82,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1								
									set_dwbias_conv3x3(fm_buf1,   big_bias[5]);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6]);
									conv3x3_499(fm_buf2,fm_buf1,big_w33_buffer[8]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf3,fm_buf1,big_w33_buffer[9]);
									conv3x3_499(fm_buf2,fm_buf4,big_w33_buffer[10]);
									conv3x3_499(fm_buf3,fm_buf4,big_w33_buffer[11]);
									clearpad_for_499(fm_buf1);               //clear x=6 y=10
									clearpad_for_499(fm_buf4);							
								//508 1,4->2,3
									set_bias_conv1x1(fm_buf2,	  big_bias[7],1,1,1,true);     //16  bias is load
									set_bias_conv1x1(fm_buf3,	  big_bias[8],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf7,40,24);
									//clear_pad(fm_buf8,40,24);
									CONV_1x1(fm_buf1, fm_buf2,	wt_buf_big[2],2);
									CONV_1x1(fm_buf4,fm_buf2,	wt_buf_big[3],2);
									CONV_1x1(fm_buf1, fm_buf3,	wt_buf_big[4],2);
									CONV_1x1(fm_buf4,fm_buf3,	wt_buf_big[5],2);
								//deload
									deload_img(fm_buf2, ddrdebug,
																	0,
																	82,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									deload_img(fm_buf3, ddrdebug,
																	1,
																	82,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1																												
							

}
