//layer 2
//layer3version

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
fm_type fm_buf9[16][50][82];
fm_type fm_buf10[16][50][82];
fm_type fm_buf11[16][50][82];
fm_type fm_buf12[16][50][82];

fm_type fm_buf13[16][50][82];
fm_type fm_buf14[16][50][82];
fm_type fm_buf15[16][50][82];
fm_type fm_buf16[16][50][82];
fm_type fm_buf17[16][50][82];
wt_type dwt_buf3[16][3][3];  //33buffer
wt_type wt_buf1[16][16];     //11buffer
wt_type wt_buf1a[16][16];
bs_type bias[16];            //bias buffer
bs_type bias2[16];
bs_type bias3[16];




bs_type big_bias[65][16];   //big buffer
wt_type wt_buf_big[300][16][16];
wt_type dwt_buf3_big[30][16][3][3];

wt_type     big_w33_buffer[12][16][16][3][3];
 

void Thinker(	uint16 image_in_raw_pad[imagesize],
				uint256		w_port_3x3[500][3][3],
				uint256     w_port_1x1[1000][16],
				uint256     bias_port[500],
				uint256		w_port_3x3_2[20][16][3][3],

				uint256 ddrdebug [ddrsize][30],
				uint256 ddrdebug_2 [ddrsize][30],
				uint256 ddrdebug_3 [ddrsize][30],
				uint256 ddrdebug_4 [ddrsize][30],
				uint16 debug[2])
{
 	
#pragma HLS INTERFACE m_axi depth=3*(320+2)*(192+2) 	port=image_in_raw_pad			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=500*3*3 	port=w_port_3x3			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=1000 	port=w_port_1x1			offset=slave	bundle=INPUT
	//every uint256 store 16 number, so 1 index is a full 16x16x1x1 data
#pragma HLS INTERFACE m_axi depth=500 	port=bias_port			offset=slave	bundle=INPUT
	//So we use a ddr of size at least 48*(20*8)*(12*8)


#pragma HLS INTERFACE m_axi depth=150000	port=ddrdebug			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=150000	port=ddrdebug_2			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=150000	port=ddrdebug_3			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=150000	port=ddrdebug_4			offset=slave	bundle=INPUT
#pragma HLS INTERFACE s_axilite register	port=return
#pragma HLS INTERFACE m_axi depth=2			port=debug				offset=slave	bundle=OUTPUT



#pragma HLS ALLOCATION instances=CONV_1x1			 		limit=1 function
#pragma HLS ALLOCATION instances=dw_conv_2			 		limit=3 function
#pragma HLS ALLOCATION instances=dw_conv_1			 		limit=3 function
#pragma HLS ALLOCATION instances=deload_img			 		limit=1 function
#pragma HLS ALLOCATION instances=load_weight_conv1x1			 		limit=1 function
#pragma HLS ALLOCATION instances=load_bias_from_axi			 		limit=1 function


#pragma HLS ALLOCATION instances=conv3x3_501			 		limit=1 function
#pragma HLS ALLOCATION instances=conv3x3_502			 		limit=1 function
#pragma HLS ALLOCATION instances=clearpad_for_502			 		limit=1 function
#pragma HLS ALLOCATION instances=conv3x3_499			 		limit=1 function
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
			load_bias_from_axi(big_bias[15], bias_port[37]);

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
					set_bias_conv1x1(fm_buf3,big_bias[15],x,y,1,false);


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
								set_bias_conv1x1(fm_buf5,big_bias[13],x,y,1,false);
									int i=0;
									set_bias_conv1x1(fm_buf2,	  big_bias[i],x,y,1,true);     //16  bias is load
									clear_pad(fm_buf2,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[i],0);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6+i]);             //16  bias is load
									dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[i],1);
									CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+i],1);
									CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[12+i],1);
									i=1+i;
									set_bias_conv1x1(fm_buf2,	  big_bias[i],x,y,1,true);     //16  bias is load
									clear_pad(fm_buf2,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[i],0);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6+i]);             //16  bias is load
									dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[i],1);
									CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+i],1);
									CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[12+i],1);

									i=1+i;
									set_bias_conv1x1(fm_buf2,	  big_bias[i],x,y,1,true);     //16  bias is load
									clear_pad(fm_buf2,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[i],0);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6+i]);             //16  bias is load
									dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[i],1);
									CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+i],1);
									CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[12+i],1);

									i=1+i;
									set_bias_conv1x1(fm_buf2,	  big_bias[i],x,y,1,true);     //16  bias is load
									clear_pad(fm_buf2,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[i],0);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6+i]);             //16  bias is load
									dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[i],1);
									CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+i],1);
									CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[12+i],1);

									i=1+i;
									set_bias_conv1x1(fm_buf2,	  big_bias[i],x,y,1,true);     //16  bias is load
									clear_pad(fm_buf2,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[i],0);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6+i]);             //16  bias is load
									dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[i],1);
									CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+i],1);
									CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[12+i],1);

									i=1+i;
									set_bias_conv1x1(fm_buf2,	  big_bias[i],x,y,1,true);     //16  bias is load
									clear_pad(fm_buf2,40,24);
									CONV_1x1(fm_buf1,fm_buf2,	wt_buf_big[i],0);
									set_dwbias_conv3x3(fm_buf4,   big_bias[6+i]);             //16  bias is load
									dw_conv_2(fm_buf2,fm_buf4,dwt_buf3_big[i],1);
									CONV_1x1(fm_buf4,fm_buf3,   wt_buf_big[6+i],1);
									CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[12+i],1);
									deload_img(fm_buf3, ddrdebug_3,
																0,
																2*y+y*20+1,
																2*x+x*12+1,

																20,
																12,
																2*((320/16)+2)
																);
									deload_img(fm_buf5, ddrdebug_3,
																1,
																2*y+y*20+1,
																2*x+x*12+1,

																20,
																12,
																2*((320/16)+2)
																);


							}
			}


//////////////////////////////////////////////////////////////////////////////	layer 371-377 ///////////////////////////////////////////////////////////////////////////////////////
			//371 weight and bias :
				//weight
				for(int i=0;i<12;i++)
				{
					load_weight_conv1x1( wt_buf_big[i], w_port_1x1[i+75]);
				}
				for(int i=0;i<12;i++)
				{
					load_weight_conv1x1( wt_buf_big[i+12], w_port_1x1[i+75+12]);
				}
				//bias
				for(int i=0;i<12;i++)
				{
					load_bias_from_axi( big_bias[i], bias_port[i+78]);
				}

			//374 weight and bias:
				for(int i=0;i<12;i++)
				{
					load_bias_from_axi(big_bias[i+12], bias_port[i+90]);
				}
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[0+35]);
				load_dwweight_conv3x3( dwt_buf3_big[1], w_port_3x3[1+35]);
				load_dwweight_conv3x3( dwt_buf3_big[2], w_port_3x3[2+35]);
				load_dwweight_conv3x3( dwt_buf3_big[3], w_port_3x3[3+35]);
				load_dwweight_conv3x3( dwt_buf3_big[4], w_port_3x3[4+35]);
				load_dwweight_conv3x3( dwt_buf3_big[5], w_port_3x3[5+35]);
				load_dwweight_conv3x3( dwt_buf3_big[6], w_port_3x3[6+35]);
				load_dwweight_conv3x3( dwt_buf3_big[7], w_port_3x3[7+35]);
				load_dwweight_conv3x3( dwt_buf3_big[8], w_port_3x3[8+35]);
				load_dwweight_conv3x3( dwt_buf3_big[9], w_port_3x3[9+35]);
				load_dwweight_conv3x3( dwt_buf3_big[10], w_port_3x3[10+35]);
				load_dwweight_conv3x3( dwt_buf3_big[11], w_port_3x3[11+35]);
			//377 weight and bias:

				for(int i=0;i<12;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+12+12], w_port_1x1[i+99]);
				}
				for(int i=0;i<12;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+12+24], w_port_1x1[i+111]);
				}
				load_bias_from_axi(big_bias[26], bias_port[102]);
				load_bias_from_axi(big_bias[27], bias_port[103]);

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


			set_bias_conv1x1(fm_buf5,big_bias[26],1,1,1,false);
			set_bias_conv1x1(fm_buf6,big_bias[27],1,1,1,false);

			int i=-1;
			i++;
				set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
				CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
			CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
			i++;
				set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
				CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
			CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
			i++;
				set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
				CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
			CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
			i++;
				set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
				CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
			CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
			i++;
				set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
				CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
			CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
			i++;
				set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
				CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
			CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
			i++;
				set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
				CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
			CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
			i++;
				set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
				CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
			CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
			i++;
				set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
				CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
			CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
			i++;
				set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
				CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
			CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
			i++;
				set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
				CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
			CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
			i++;
				set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
				clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
				set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
				dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
				CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
			CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);

			clear_pad3(fm_buf5,2*((320/16)+2),2*((192/16)+2));
			clear_pad3(fm_buf6,2*((320/16)+2),2*((192/16)+2));
			deload_img(fm_buf5, ddrdebug_3,
										0,
										1,
										1,
										2*((320/16)+2)-1,
										2*((192/16)+2)-1,
										2*((320/16)+2)
										);
			deload_img(fm_buf6, ddrdebug_3,
										1,
										1,
										1,
										2*((320/16)+2)-1,
										2*((192/16)+2)-1,
										2*((320/16)+2)
										);
//////////////////////////////////////////////////////////////////////////////	layer 380-386 ///////////////////////////////////////////////////////////////////////////////////////
			//380 weight and bias :
				//weight
				for(int i=0;i<12;i++)
				{
					load_weight_conv1x1( wt_buf_big[i], w_port_1x1[i+123]);
				}
				for(int i=0;i<12;i++)
				{
					load_weight_conv1x1( wt_buf_big[i+12], w_port_1x1[i+123+12]);
				}
				//bias
				for(int i=0;i<12;i++)
				{
					load_bias_from_axi( big_bias[i], bias_port[i+104]);
				}

			//383 weight and bias:
				for(int i=0;i<12;i++)
				{
					load_bias_from_axi(big_bias[i+12], bias_port[i+116]);
				}
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[0+47]);
				load_dwweight_conv3x3( dwt_buf3_big[1], w_port_3x3[1+47]);
				load_dwweight_conv3x3( dwt_buf3_big[2], w_port_3x3[2+47]);
				load_dwweight_conv3x3( dwt_buf3_big[3], w_port_3x3[3+47]);
				load_dwweight_conv3x3( dwt_buf3_big[4], w_port_3x3[4+47]);
				load_dwweight_conv3x3( dwt_buf3_big[5], w_port_3x3[5+47]);
				load_dwweight_conv3x3( dwt_buf3_big[6], w_port_3x3[6+47]);
				load_dwweight_conv3x3( dwt_buf3_big[7], w_port_3x3[7+47]);
				load_dwweight_conv3x3( dwt_buf3_big[8], w_port_3x3[8+47]);
				load_dwweight_conv3x3( dwt_buf3_big[9], w_port_3x3[9+47]);
				load_dwweight_conv3x3( dwt_buf3_big[10], w_port_3x3[10+47]);
				load_dwweight_conv3x3( dwt_buf3_big[11], w_port_3x3[11+47]);
			//386 weight and bias:

				for(int i=0;i<12;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+12+12], w_port_1x1[i+147]);
				}
				for(int i=0;i<12;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+12+24], w_port_1x1[i+159]);
				}
				load_bias_from_axi(big_bias[26], bias_port[128]);
				load_bias_from_axi(big_bias[27], bias_port[129]);

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


				set_bias_conv1x1(fm_buf5,big_bias[26],1,1,1,false);
				set_bias_conv1x1(fm_buf6,big_bias[27],1,1,1,false);

				i=-1;
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);

				clear_pad3(fm_buf5,2*((320/16)+2),2*((192/16)+2));
				clear_pad3(fm_buf6,2*((320/16)+2),2*((192/16)+2));
				deload_img(fm_buf5, ddrdebug_3,
											0,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);
				deload_img(fm_buf6, ddrdebug_3,
											1,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);

//////////////////////////////////////////////////////////////////////////////	layer 389 392 395 ///////////////////////////////////////////////////////////////////////////////////////
			//389 weight and bias :
				//weight
				for(int i=0;i<12;i++)
				{
					load_weight_conv1x1( wt_buf_big[i], w_port_1x1[i+171]);
				}
				for(int i=0;i<12;i++)
				{
					load_weight_conv1x1( wt_buf_big[i+12], w_port_1x1[i+171+12]);
				}
				//bias
				for(int i=0;i<12;i++)
				{
					load_bias_from_axi( big_bias[i], bias_port[i+130]);
				}

			//392 weight and bias:
				for(int i=0;i<12;i++)
				{
					load_bias_from_axi(big_bias[i+12], bias_port[i+142]);
				}
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[0+59]);
				load_dwweight_conv3x3( dwt_buf3_big[1], w_port_3x3[1+59]);
				load_dwweight_conv3x3( dwt_buf3_big[2], w_port_3x3[2+59]);
				load_dwweight_conv3x3( dwt_buf3_big[3], w_port_3x3[3+59]);
				load_dwweight_conv3x3( dwt_buf3_big[4], w_port_3x3[4+59]);
				load_dwweight_conv3x3( dwt_buf3_big[5], w_port_3x3[5+59]);
				load_dwweight_conv3x3( dwt_buf3_big[6], w_port_3x3[6+59]);
				load_dwweight_conv3x3( dwt_buf3_big[7], w_port_3x3[7+59]);
				load_dwweight_conv3x3( dwt_buf3_big[8], w_port_3x3[8+59]);
				load_dwweight_conv3x3( dwt_buf3_big[9], w_port_3x3[9+59]);
				load_dwweight_conv3x3( dwt_buf3_big[10], w_port_3x3[10+59]);
				load_dwweight_conv3x3( dwt_buf3_big[11], w_port_3x3[11+59]);
			//395 weight and bias:

				for(int i=0;i<12;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+12+12], w_port_1x1[i+195]);
				}
				for(int i=0;i<12;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+12+24], w_port_1x1[i+207]);
				}
				load_bias_from_axi(big_bias[26], bias_port[154]);
				load_bias_from_axi(big_bias[27], bias_port[155]);

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


				set_bias_conv1x1(fm_buf5,big_bias[26],1,1,1,false);
				set_bias_conv1x1(fm_buf6,big_bias[27],1,1,1,false);

				i=-1;
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);

				clear_pad3(fm_buf5,2*((320/16)+2),2*((192/16)+2));
				clear_pad3(fm_buf6,2*((320/16)+2),2*((192/16)+2));
				deload_img(fm_buf5, ddrdebug_3,
											0,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);
				deload_img(fm_buf6, ddrdebug_3,
											1,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);


//////////////////////////////////////////////////////////////////////////////	layer 398 401 404 ///////////////////////////////////////////////////////////////////////////////////////
			//389 weight and bias :
				//weight
				for(int i=0;i<12;i++)
				{
					load_weight_conv1x1( wt_buf_big[i], w_port_1x1[i+219]);
				}
				for(int i=0;i<12;i++)
				{
					load_weight_conv1x1( wt_buf_big[i+12], w_port_1x1[i+219+12]);
				}
				//bias
				for(int i=0;i<12;i++)
				{
					load_bias_from_axi( big_bias[i], bias_port[i+156]);
				}

			//401 weight and bias:
				for(int i=0;i<12;i++)
				{
					load_bias_from_axi(big_bias[i+12], bias_port[i+168]);
				}
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[0+71]);
				load_dwweight_conv3x3( dwt_buf3_big[1], w_port_3x3[1+71]);
				load_dwweight_conv3x3( dwt_buf3_big[2], w_port_3x3[2+71]);
				load_dwweight_conv3x3( dwt_buf3_big[3], w_port_3x3[3+71]);
				load_dwweight_conv3x3( dwt_buf3_big[4], w_port_3x3[4+71]);
				load_dwweight_conv3x3( dwt_buf3_big[5], w_port_3x3[5+71]);
				load_dwweight_conv3x3( dwt_buf3_big[6], w_port_3x3[6+71]);
				load_dwweight_conv3x3( dwt_buf3_big[7], w_port_3x3[7+71]);
				load_dwweight_conv3x3( dwt_buf3_big[8], w_port_3x3[8+71]);
				load_dwweight_conv3x3( dwt_buf3_big[9], w_port_3x3[9+71]);
				load_dwweight_conv3x3( dwt_buf3_big[10], w_port_3x3[10+71]);
				load_dwweight_conv3x3( dwt_buf3_big[11], w_port_3x3[11+71]);
			//404 weight and bias:

				for(int i=0;i<12;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+12+12], w_port_1x1[i+243]);
				}
				for(int i=0;i<12;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+12+24], w_port_1x1[i+255]);
				}
				for(int i=0;i<12;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+12+36], w_port_1x1[i+267]);
				}				
				load_bias_from_axi(big_bias[26], bias_port[180]);
				load_bias_from_axi(big_bias[27], bias_port[181]);
				load_bias_from_axi(big_bias[28], bias_port[182]);

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

				set_bias_conv1x1(fm_buf5,big_bias[26],1,1,1,false);
				set_bias_conv1x1(fm_buf6,big_bias[27],1,1,1,false);
				set_bias_conv1x1(fm_buf7,big_bias[28],1,1,1,false);

				i=-1;
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[12+36+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[12+36+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[12+36+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[12+36+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[12+36+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[12+36+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[12+36+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[12+36+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[12+36+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[12+36+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[12+36+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+12],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[12+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[24+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[12+24+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[12+36+i],1);

				clear_pad3(fm_buf5,2*((320/16)+2),2*((192/16)+2));
				clear_pad3(fm_buf6,2*((320/16)+2),2*((192/16)+2));
				clear_pad3(fm_buf7,2*((320/16)+2),2*((192/16)+2));
				deload_img(fm_buf5, ddrdebug_3,
											0,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);
				deload_img(fm_buf6, ddrdebug_3,
											1,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);
				deload_img(fm_buf7, ddrdebug_3,
											2,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);

//////////////////////////////////////////////////////////////////////////////	layer 406 409 412 ///////////////////////////////////////////////////////////////////////////////////////
			//406 weight and bias :
				//weight
				for(int i=0;i<18;i++)
				{
					load_weight_conv1x1( wt_buf_big[i], w_port_1x1[i+279]);
				}
				for(int i=0;i<18;i++)
				{
					load_weight_conv1x1( wt_buf_big[i+18], w_port_1x1[i+279+18]);
				}
				for(int i=0;i<18;i++)
				{
					load_weight_conv1x1( wt_buf_big[i+36], w_port_1x1[i+279+36]);
				}				
				//bias
				for(int i=0;i<18;i++)
				{
					load_bias_from_axi( big_bias[i], bias_port[i+183]);
				}

			//409 weight and bias:
				for(int i=0;i<18;i++)
				{
					load_bias_from_axi(big_bias[i+18], bias_port[i+201]);
				}
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[0+83]);
				load_dwweight_conv3x3( dwt_buf3_big[1], w_port_3x3[1+83]);
				load_dwweight_conv3x3( dwt_buf3_big[2], w_port_3x3[2+83]);
				load_dwweight_conv3x3( dwt_buf3_big[3], w_port_3x3[3+83]);
				load_dwweight_conv3x3( dwt_buf3_big[4], w_port_3x3[4+83]);
				load_dwweight_conv3x3( dwt_buf3_big[5], w_port_3x3[5+83]);
				load_dwweight_conv3x3( dwt_buf3_big[6], w_port_3x3[6+83]);
				load_dwweight_conv3x3( dwt_buf3_big[7], w_port_3x3[7+83]);
				load_dwweight_conv3x3( dwt_buf3_big[8], w_port_3x3[8+83]);
				load_dwweight_conv3x3( dwt_buf3_big[9], w_port_3x3[9+83]);
				load_dwweight_conv3x3( dwt_buf3_big[10], w_port_3x3[10+83]);
				load_dwweight_conv3x3( dwt_buf3_big[11], w_port_3x3[11+83]);
				load_dwweight_conv3x3( dwt_buf3_big[12], w_port_3x3[12+83]);
				load_dwweight_conv3x3( dwt_buf3_big[13], w_port_3x3[13+83]);
				load_dwweight_conv3x3( dwt_buf3_big[14], w_port_3x3[14+83]);
				load_dwweight_conv3x3( dwt_buf3_big[15], w_port_3x3[15+83]);
				load_dwweight_conv3x3( dwt_buf3_big[16], w_port_3x3[16+83]);
				load_dwweight_conv3x3( dwt_buf3_big[17], w_port_3x3[17+83]);
			//412 weight and bias:

				for(int i=0;i<18;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+54], w_port_1x1[i+333]);
				}
				for(int i=0;i<18;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+18+54], w_port_1x1[i+351]);
				}
				for(int i=0;i<18;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+36+54], w_port_1x1[i+369]);
				}
				load_bias_from_axi(big_bias[36], bias_port[219]);
				load_bias_from_axi(big_bias[37], bias_port[220]);
				load_bias_from_axi(big_bias[38], bias_port[221]);

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
				aload_img_2(fm_buf8, ddrdebug_3,
											2,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);

				set_bias_conv1x1(fm_buf5,big_bias[36],1,1,1,false);
				set_bias_conv1x1(fm_buf6,big_bias[37],1,1,1,false);
				set_bias_conv1x1(fm_buf7,big_bias[38],1,1,1,false);

				i=-1;
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);

				clear_pad3(fm_buf5,2*((320/16)+2),2*((192/16)+2));
				clear_pad3(fm_buf6,2*((320/16)+2),2*((192/16)+2));
				clear_pad3(fm_buf7,2*((320/16)+2),2*((192/16)+2));
				deload_img(fm_buf5, ddrdebug_3,
											0,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);
				deload_img(fm_buf6, ddrdebug_3,
											1,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);
				deload_img(fm_buf7, ddrdebug_3,
											2,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);

//////////////////////////////////////////////////////////////////////////////	layer 415 418 421 ///////////////////////////////////////////////////////////////////////////////////////
			//415 weight and bias :
				//weight
				for(int i=0;i<18;i++)
				{
					load_weight_conv1x1( wt_buf_big[i], w_port_1x1[i+387]);
				}
				for(int i=0;i<18;i++)
				{
					load_weight_conv1x1( wt_buf_big[i+18], w_port_1x1[i+387+18]);
				}
				for(int i=0;i<18;i++)
				{
					load_weight_conv1x1( wt_buf_big[i+36], w_port_1x1[i+387+36]);
				}				
				//bias
				for(int i=0;i<18;i++)
				{
					load_bias_from_axi( big_bias[i], bias_port[i+222]);
				}

			//418 weight and bias:
				for(int i=0;i<18;i++)
				{
					load_bias_from_axi(big_bias[i+18], bias_port[i+240]);
				}
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[0+101]);
				load_dwweight_conv3x3( dwt_buf3_big[1], w_port_3x3[1+101]);
				load_dwweight_conv3x3( dwt_buf3_big[2], w_port_3x3[2+101]);
				load_dwweight_conv3x3( dwt_buf3_big[3], w_port_3x3[3+101]);
				load_dwweight_conv3x3( dwt_buf3_big[4], w_port_3x3[4+101]);
				load_dwweight_conv3x3( dwt_buf3_big[5], w_port_3x3[5+101]);
				load_dwweight_conv3x3( dwt_buf3_big[6], w_port_3x3[6+101]);
				load_dwweight_conv3x3( dwt_buf3_big[7], w_port_3x3[7+101]);
				load_dwweight_conv3x3( dwt_buf3_big[8], w_port_3x3[8+101]);
				load_dwweight_conv3x3( dwt_buf3_big[9], w_port_3x3[9+101]);
				load_dwweight_conv3x3( dwt_buf3_big[10], w_port_3x3[10+101]);
				load_dwweight_conv3x3( dwt_buf3_big[11], w_port_3x3[11+101]);
				load_dwweight_conv3x3( dwt_buf3_big[12], w_port_3x3[12+101]);
				load_dwweight_conv3x3( dwt_buf3_big[13], w_port_3x3[13+101]);
				load_dwweight_conv3x3( dwt_buf3_big[14], w_port_3x3[14+101]);
				load_dwweight_conv3x3( dwt_buf3_big[15], w_port_3x3[15+101]);
				load_dwweight_conv3x3( dwt_buf3_big[16], w_port_3x3[16+101]);
				load_dwweight_conv3x3( dwt_buf3_big[17], w_port_3x3[17+101]);
			//421 weight and bias:

				for(int i=0;i<18;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+54], w_port_1x1[i+441]);
				}
				for(int i=0;i<18;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+18+54], w_port_1x1[i+459]);
				}
				for(int i=0;i<18;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+36+54], w_port_1x1[i+477]);
				}
				load_bias_from_axi(big_bias[36], bias_port[258]);
				load_bias_from_axi(big_bias[37], bias_port[259]);
				load_bias_from_axi(big_bias[38], bias_port[260]);

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
				aload_img_2(fm_buf8, ddrdebug_3,
											2,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);

				set_bias_conv1x1(fm_buf5,big_bias[36],1,1,1,false);
				set_bias_conv1x1(fm_buf6,big_bias[37],1,1,1,false);
				set_bias_conv1x1(fm_buf7,big_bias[38],1,1,1,false);

				i=-1;
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);

				clear_pad3(fm_buf5,2*((320/16)+2),2*((192/16)+2));
				clear_pad3(fm_buf6,2*((320/16)+2),2*((192/16)+2));
				clear_pad3(fm_buf7,2*((320/16)+2),2*((192/16)+2));


				deload_img(fm_buf5, ddrdebug_3,
											0,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);
				deload_img(fm_buf6, ddrdebug_3,
											1,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);
				deload_img(fm_buf7, ddrdebug_3,
											2,
											1,
											1,
											2*((320/16)+2)-1,
											2*((192/16)+2)-1,
											2*((320/16)+2)
											);



//////////////////////////////////////////////////////////////////////////////	layer 424 427 430 ///////////////////////////////////////////////////////////////////////////////////////
			//424 weight and bias :
				//weight
				for(int i=0;i<18;i++)
				{
					load_weight_conv1x1( wt_buf_big[i], w_port_1x1[i+495]);
				}
				for(int i=0;i<18;i++)
				{
					load_weight_conv1x1( wt_buf_big[i+18], w_port_1x1[i+495+18]);
				}
				for(int i=0;i<18;i++)
				{
					load_weight_conv1x1( wt_buf_big[i+36], w_port_1x1[i+495+36]);
				}				
				//bias
				for(int i=0;i<18;i++)
				{
					load_bias_from_axi( big_bias[i], bias_port[i+261]);
				}

			//427 weight and bias:
				for(int i=0;i<18;i++)
				{
					load_bias_from_axi(big_bias[i+18], bias_port[i+279]);
				}
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[0+119]);
				load_dwweight_conv3x3( dwt_buf3_big[1], w_port_3x3[1+119]);
				load_dwweight_conv3x3( dwt_buf3_big[2], w_port_3x3[2+119]);
				load_dwweight_conv3x3( dwt_buf3_big[3], w_port_3x3[3+119]);
				load_dwweight_conv3x3( dwt_buf3_big[4], w_port_3x3[4+119]);
				load_dwweight_conv3x3( dwt_buf3_big[5], w_port_3x3[5+119]);
				load_dwweight_conv3x3( dwt_buf3_big[6], w_port_3x3[6+119]);
				load_dwweight_conv3x3( dwt_buf3_big[7], w_port_3x3[7+119]);
				load_dwweight_conv3x3( dwt_buf3_big[8], w_port_3x3[8+119]);
				load_dwweight_conv3x3( dwt_buf3_big[9], w_port_3x3[9+119]);
				load_dwweight_conv3x3( dwt_buf3_big[10], w_port_3x3[10+119]);
				load_dwweight_conv3x3( dwt_buf3_big[11], w_port_3x3[11+119]);
				load_dwweight_conv3x3( dwt_buf3_big[12], w_port_3x3[12+119]);
				load_dwweight_conv3x3( dwt_buf3_big[13], w_port_3x3[13+119]);
				load_dwweight_conv3x3( dwt_buf3_big[14], w_port_3x3[14+119]);
				load_dwweight_conv3x3( dwt_buf3_big[15], w_port_3x3[15+119]);
				load_dwweight_conv3x3( dwt_buf3_big[16], w_port_3x3[16+119]);
				load_dwweight_conv3x3( dwt_buf3_big[17], w_port_3x3[17+119]);
			//430 weight and bias:

				for(int i=0;i<18;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+54], w_port_1x1[i+549]);
				}
				for(int i=0;i<18;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+18+54], w_port_1x1[i+567]);
				}
				for(int i=0;i<18;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+36+54], w_port_1x1[i+585]);
				}
				for(int i=0;i<18;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+54+54], w_port_1x1[i+603]);
				}
				for(int i=0;i<18;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+72+54], w_port_1x1[i+621]);
				}								
				load_bias_from_axi(big_bias[36], bias_port[297]);
				load_bias_from_axi(big_bias[37], bias_port[298]);
				load_bias_from_axi(big_bias[38], bias_port[299]);
				load_bias_from_axi(big_bias[39], bias_port[300]);
				load_bias_from_axi(big_bias[40], bias_port[301]);

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
				aload_img_2(fm_buf8, ddrdebug_3,
											2,
											0,
											0,
											2*((320/16)+2),
											2*((192/16)+2),
											2*((320/16)+2)
											);

				set_bias_conv1x1(fm_buf5,big_bias[36],1,1,1,false);
				set_bias_conv1x1(fm_buf6,big_bias[37],1,1,1,false);
				set_bias_conv1x1(fm_buf7,big_bias[38],1,1,1,false);
				set_bias_conv1x1(fm_buf9,big_bias[39],1,1,1,false);
				set_bias_conv1x1(fm_buf10,big_bias[40],1,1,1,false);

				i=-1;
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				
				i++;
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,false);     //16  bias is load
					clear_pad2(fm_buf3,2*((320/16)+2),2*((192/16)+2));
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+18],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+36],0);                //get 16/192 output channel of 32 input channel
					set_dwbias_conv3x3(fm_buf4,   big_bias[18+i]);             //16  bias is load
					dw_conv_2(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[54+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[18+54+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[36+54+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[54+54+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[72+54+i],1);				

				clear_pad(fm_buf5,9,5);
				clear_pad(fm_buf6,9,5);
				clear_pad(fm_buf7,9,5);
				clear_pad(fm_buf9,9,5);
				clear_pad(fm_buf10,9,5);
				deload_img(fm_buf5, ddrdebug_4,
											0,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);
				deload_img(fm_buf6, ddrdebug_4,
											1,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);
				deload_img(fm_buf7, ddrdebug_4,
											2,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);
				deload_img(fm_buf9, ddrdebug_4,
											3,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);
				deload_img(fm_buf10, ddrdebug_4,
											4,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);											
	//data in ddrdebug_4 only contains one pad between two images

//////////////////////////////////////////////////////////////////////////////	layer 432 435 438 ///////////////////////////////////////////////////////////////////////////////////////
			//432 weight and bias :
				//weight
				for(int i=0;i<30;i++)
				{
					load_weight_conv1x1( wt_buf_big[i], w_port_1x1[i+639]);
				}
				for(int i=0;i<30;i++)
				{
					load_weight_conv1x1( wt_buf_big[i+30], w_port_1x1[i+639+30]);
				}
				for(int i=0;i<30;i++)
				{
					load_weight_conv1x1( wt_buf_big[i+60], w_port_1x1[i+639+60]);
				}
				for(int i=0;i<30;i++)
				{
					load_weight_conv1x1( wt_buf_big[i+90], w_port_1x1[i+639+90]);
				}
				for(int i=0;i<30;i++)
				{
					load_weight_conv1x1( wt_buf_big[i+120], w_port_1x1[i+639+120]);
				}												
				//bias
				for(int i=0;i<30;i++)
				{
					load_bias_from_axi( big_bias[i], bias_port[i+302]);
				}

			//435 weight and bias:
				for(int i=0;i<30;i++)
				{
					load_bias_from_axi(big_bias[i+30], bias_port[i+332]);
				}
				load_dwweight_conv3x3( dwt_buf3_big[0], w_port_3x3[0+137]);
				load_dwweight_conv3x3( dwt_buf3_big[1], w_port_3x3[1+137]);
				load_dwweight_conv3x3( dwt_buf3_big[2], w_port_3x3[2+137]);
				load_dwweight_conv3x3( dwt_buf3_big[3], w_port_3x3[3+137]);
				load_dwweight_conv3x3( dwt_buf3_big[4], w_port_3x3[4+137]);
				load_dwweight_conv3x3( dwt_buf3_big[5], w_port_3x3[5+137]);
				load_dwweight_conv3x3( dwt_buf3_big[6], w_port_3x3[6+137]);
				load_dwweight_conv3x3( dwt_buf3_big[7], w_port_3x3[7+137]);
				load_dwweight_conv3x3( dwt_buf3_big[8], w_port_3x3[8+137]);
				load_dwweight_conv3x3( dwt_buf3_big[9], w_port_3x3[9+137]);
				load_dwweight_conv3x3( dwt_buf3_big[10], w_port_3x3[10+137]);
				load_dwweight_conv3x3( dwt_buf3_big[11], w_port_3x3[11+137]);
				load_dwweight_conv3x3( dwt_buf3_big[12], w_port_3x3[12+137]);
				load_dwweight_conv3x3( dwt_buf3_big[13], w_port_3x3[13+137]);
				load_dwweight_conv3x3( dwt_buf3_big[14], w_port_3x3[14+137]);
				load_dwweight_conv3x3( dwt_buf3_big[15], w_port_3x3[15+137]);
				load_dwweight_conv3x3( dwt_buf3_big[16], w_port_3x3[16+137]);
				load_dwweight_conv3x3( dwt_buf3_big[17], w_port_3x3[17+137]);
				load_dwweight_conv3x3( dwt_buf3_big[18], w_port_3x3[18+137]);
				load_dwweight_conv3x3( dwt_buf3_big[19], w_port_3x3[19+137]);
				load_dwweight_conv3x3( dwt_buf3_big[20], w_port_3x3[20+137]);
				load_dwweight_conv3x3( dwt_buf3_big[21], w_port_3x3[21+137]);
				load_dwweight_conv3x3( dwt_buf3_big[22], w_port_3x3[22+137]);
				load_dwweight_conv3x3( dwt_buf3_big[23], w_port_3x3[23+137]);
				load_dwweight_conv3x3( dwt_buf3_big[24], w_port_3x3[24+137]);
				load_dwweight_conv3x3( dwt_buf3_big[25], w_port_3x3[25+137]);
				load_dwweight_conv3x3( dwt_buf3_big[26], w_port_3x3[26+137]);
				load_dwweight_conv3x3( dwt_buf3_big[27], w_port_3x3[27+137]);
				load_dwweight_conv3x3( dwt_buf3_big[28], w_port_3x3[28+137]);
				load_dwweight_conv3x3( dwt_buf3_big[29], w_port_3x3[29+137]);
			//438 weight and bias:

				for(int i=0;i<30;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+150], w_port_1x1[i+789]);
				}
				for(int i=0;i<30;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+180], w_port_1x1[i+819]);
				}
				for(int i=0;i<30;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+210], w_port_1x1[i+849]);
				}
				for(int i=0;i<30;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+240], w_port_1x1[i+879]);
				}
				for(int i=0;i<30;i++)
				{

					load_weight_conv1x1( wt_buf_big[i+270], w_port_1x1[i+909]);
				}								
				load_bias_from_axi(big_bias[60], bias_port[362]);
				load_bias_from_axi(big_bias[61], bias_port[363]);
				load_bias_from_axi(big_bias[62], bias_port[364]);
				load_bias_from_axi(big_bias[63], bias_port[365]);
				load_bias_from_axi(big_bias[64], bias_port[366]);

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
				aload_img_2(fm_buf8, ddrdebug_4,
											2,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				aload_img_2(fm_buf11, ddrdebug_4,
											3,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				aload_img_2(fm_buf12, ddrdebug_4,
											4,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);

				set_bias_conv1x1(fm_buf5,big_bias[60],1,1,1,false);
				set_bias_conv1x1(fm_buf6,big_bias[61],1,1,1,false);
				set_bias_conv1x1(fm_buf7,big_bias[62],1,1,1,false);
				set_bias_conv1x1(fm_buf9,big_bias[63],1,1,1,false);
				set_bias_conv1x1(fm_buf10,big_bias[64],1,1,1,false);

				i=-1;
				i++;	//0
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);
				i++;	//1
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);		
				i++;	//2
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);			
				i++;	//3
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);				
				i++;	//4
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);				
				i++;	//5
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);				
				i++;	//6
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);				
				i++;	//7
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);				
				i++;	//8
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);				
				i++;	//9
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);				
				i++;	//10
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);				
				i++;	//11
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);				
				i++;	//12
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);				
				i++;	//13
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);				
				i++;	//14
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);				
				i++;	//15
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);				
				i++;	//16
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);				
				i++;	//17
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);				
				i++;	//18
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);
				i++;	//19
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);
				i++;	//20
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);
				i++;	//21
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);
				i++;	//22
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);
				i++;	//23
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);
				i++;	//24
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);
				i++;	//25
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);
				i++;	//26
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);
				i++;	//27
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);
				i++;	//28
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);
				i++;	//29
					set_bias_conv1x1(fm_buf3,	  big_bias[i],1,1,1,true);     //16  bias is load
					clear_pad(fm_buf3,10,6);
					clear_pad(fm_buf3,21,13);
					CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[i],0);
					CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[i+30],0);
					CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[i+60],0);                //get 16/192 output channel of 32 input channel
					CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[i+90],0);
					CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[i+120],0);
					set_dwbias_conv3x3(fm_buf4,   big_bias[30+i]);             //16  bias is load
					dw_conv_1(fm_buf3,fm_buf4,dwt_buf3_big[i],1);
					CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[150+i],1);
				CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[180+i],1);
				CONV_1x1(fm_buf4,fm_buf7,   wt_buf_big[210+i],1);
				CONV_1x1(fm_buf4,fm_buf9,   wt_buf_big[240+i],1);
				CONV_1x1(fm_buf4,fm_buf10,   wt_buf_big[270+i],1);

				clear_pad(fm_buf5,9,5);
				clear_pad(fm_buf6,9,5);
				clear_pad(fm_buf7,9,5);
				clear_pad(fm_buf9,9,5);
				clear_pad(fm_buf10,9,5);
				deload_img(fm_buf5, ddrdebug_4,
											0,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);
				deload_img(fm_buf6, ddrdebug_4,
											1,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);
				deload_img(fm_buf7, ddrdebug_4,
											2,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);
				deload_img(fm_buf9, ddrdebug_4,
											3,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);
				deload_img(fm_buf10, ddrdebug_4,
											4,
											1,
											1,
											2*(320/32)+1,
											2*(192/32)+1,
											2*(320/32)+3
											);

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
				aload_img_2(fm_buf8, ddrdebug_4,
											2,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				aload_img_2(fm_buf11, ddrdebug_4,
											3,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);
				aload_img_2(fm_buf12, ddrdebug_4,
											4,
											0,
											0,
											2*((320/32)+2)-1,
											2*((192/32)+2)-1,
											2*((320/32)+2)-1
											);

				set_bias_conv1x1(fm_buf3,	  big_bias[0],1,1,1,true);     //16  bias is load
				set_bias_conv1x1(fm_buf4,	  big_bias[1],1,1,1,true);     //16  bias is load
				clear_pad(fm_buf3,10,6);
				clear_pad(fm_buf3,21,13);
				clear_pad(fm_buf4,10,6);
				clear_pad(fm_buf4,21,13);				
				CONV_1x1(fm_buf1,fm_buf3,	wt_buf_big[0],0);
				CONV_1x1(fm_buf2,fm_buf3,	wt_buf_big[1],0);
				CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[2],0);                //get 16/192 output channel of 32 input channel
				CONV_1x1(fm_buf11,fm_buf3,	wt_buf_big[3],0);
				CONV_1x1(fm_buf12,fm_buf3,	wt_buf_big[4],0);
				
				CONV_1x1(fm_buf1,fm_buf4,	wt_buf_big[5],0);
				CONV_1x1(fm_buf2,fm_buf4,	wt_buf_big[6],0);
				CONV_1x1(fm_buf8,fm_buf4,	wt_buf_big[7],0);                //get 16/192 output channel of 32 input channel
				CONV_1x1(fm_buf11,fm_buf4,	wt_buf_big[8],0);
				CONV_1x1(fm_buf12,fm_buf4,	wt_buf_big[9],0);

			/*	std::ifstream ifs_param("C:/Users/f/Desktop/DAC2020__thuThinker/test_data/temp1.bin", std::ios::in | std::ios::binary);
				ifs_param.read((char*)(**fm_buf3), 16 * 50 * 82 * sizeof(fm_type));
				ifs_param.close();
				std::ifstream ifs_param2("C:/Users/f/Desktop/DAC2020__thuThinker/test_data/temp2.bin", std::ios::in | std::ios::binary);
				ifs_param2.read((char*)(**fm_buf4), 16 * 50 * 82 * sizeof(fm_type));
				ifs_param2.close();
*/
//////////////////////////////////////////////////////////////////////////////	layer 502 523 526 523 525 ///////////////////////////////////////////////////////////////////////////////////////
								//input : 32channel  size 0.5^5 of 2*imgsize  : 2*((320/32)+2)-1 2*((192/32)+2)-1
								//ddr_img :ddr4

								//502: weight :32*32:3*3  bias 32 no relu , st=1
								//LOAD BIAS FOR LAYER 502
				initial_ddr(ddrdebug_4,
										6,
										320+2,
										192+2
										);
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
								load_bias_from_axi(big_bias[2], bias_port[2+index_502b]);  //521
								load_bias_from_axi(big_bias[3], bias_port[3+index_502b]);  //
								load_weight_conv3x3( big_w33_buffer[4],w_port_3x3_2[4+index_502w]); // opchannel16 inchannel16
								load_weight_conv3x3( big_w33_buffer[5],w_port_3x3_2[5+index_502w]); // opchannel16 inchannel32
								load_weight_conv3x3( big_w33_buffer[6],w_port_3x3_2[6+index_502w]); // opchannel32 inchannel16
								load_weight_conv3x3( big_w33_buffer[7],w_port_3x3_2[7+index_502w]); // opchannel32 inchannel32
								set_dwbias_conv3x3(fm_buf5,   big_bias[2]);
								set_dwbias_conv3x3(fm_buf6,   big_bias[3]);
								conv3x3_502(fm_buf1,fm_buf5,big_w33_buffer[4]);
								conv3x3_502(fm_buf2,fm_buf5,big_w33_buffer[5]);
								conv3x3_502(fm_buf1,fm_buf6,big_w33_buffer[6]);
								conv3x3_502(fm_buf2,fm_buf6,big_w33_buffer[7]);
								clearpad_for_502(fm_buf5);               //clear x=6 y=10
								clearpad_for_502(fm_buf6);
								//1,2->5,6
								load_bias_from_axi(big_bias[4], bias_port[4+index_502b]);  //523
								load_bias_from_axi(big_bias[5], bias_port[5+index_502b]);  //
								load_weight_conv3x3( big_w33_buffer[8],w_port_3x3_2[8+index_502w]); // opchannel16 inchannel16
								load_weight_conv3x3( big_w33_buffer[9],w_port_3x3_2[9+index_502w]); // opchannel16 inchannel32
								load_weight_conv3x3( big_w33_buffer[10],w_port_3x3_2[10+index_502w]); // opchannel32 inchannel16
								load_weight_conv3x3( big_w33_buffer[11],w_port_3x3_2[11+index_502w]); // opchannel32 inchannel32
								set_dwbias_conv3x3(fm_buf7,   big_bias[4]);
								set_dwbias_conv3x3(fm_buf8,   big_bias[5]);
								conv3x3_502(fm_buf1,fm_buf7,big_w33_buffer[8]);
								conv3x3_502(fm_buf2,fm_buf7,big_w33_buffer[9]);
								conv3x3_502(fm_buf1,fm_buf8,big_w33_buffer[10]);
								conv3x3_502(fm_buf2,fm_buf8,big_w33_buffer[11]);
								clearpad_for_502(fm_buf7);               //clear x=6 y=10
								clearpad_for_502(fm_buf8);
								//1,2->7,8
								load_bias_from_axi(big_bias[6], bias_port[6+index_502b]);  //525
								set_bias_conv1x1(fm_buf9,	  big_bias[6],1,1,1,true);     //16  bias is load
								clear_pad(fm_buf9,10,6);
								clear_pad(fm_buf9,21,13);
								load_weight_conv1x1( wt_buf_big[0], w_port_1x1[949]);
								load_weight_conv1x1( wt_buf_big[1], w_port_1x1[950]);
								CONV_1x1(fm_buf5,fm_buf9,	wt_buf_big[0],2);
								CONV_1x1(fm_buf6,fm_buf9,	wt_buf_big[1],2);
								//5,6->9
								load_bias_from_axi(big_bias[7], bias_port[7+index_502b]);  //525
								load_bias_from_axi(big_bias[8], bias_port[8+index_502b]);  //525
								set_bias_conv1x1(fm_buf5,	  big_bias[7],1,1,1,true);     //16  bias is load
								set_bias_conv1x1(fm_buf6,	  big_bias[8],1,1,1,true);     //16  bias is load
								clear_pad(fm_buf5,10,6);
								clear_pad(fm_buf5,21,13);
								clear_pad(fm_buf6,10,6);
								clear_pad(fm_buf6,21,13);
								load_weight_conv1x1( wt_buf_big[0], w_port_1x1[951]);
								load_weight_conv1x1( wt_buf_big[1], w_port_1x1[952]);
								load_weight_conv1x1( wt_buf_big[2], w_port_1x1[953]);
								load_weight_conv1x1( wt_buf_big[3], w_port_1x1[954]);
								CONV_1x1(fm_buf7,fm_buf5,	wt_buf_big[0],2);
								CONV_1x1(fm_buf8,fm_buf5,	wt_buf_big[1],2);
								CONV_1x1(fm_buf7,fm_buf6,	wt_buf_big[2],2);
								CONV_1x1(fm_buf8,fm_buf6,	wt_buf_big[3],2);
								//7,8->5,6
								deload_img(fm_buf5, ddrdebug_4,
															0,
															0,
															0,
															2*((320/32)+2)-1,
															2*((192/32)+2)-1,
															2*((320/32)+2)-1
															);
								deload_img(fm_buf6, ddrdebug_4,
															1,
															0,
															0,
															2*((320/32)+2)-1,
															2*((192/32)+2)-1,
															2*((320/32)+2)-1
															);                       //526:channel 0,1  525:channel 2 ,两图之间间隔为1
								deload_img(fm_buf9, ddrdebug_4,
															2,
															0,
															0,
															2*((320/32)+2)-1,
															2*((192/32)+2)-1,
															2*((320/32)+2)-1
															);                       //526:channel 0,1  525:channel 2, 两图之间间隔为1

///////////////////////////////////////////////////////////////////////  layer 443 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							/*	std::ifstream ifs_param33("C:/Users/f/Desktop/DAC2020__thuThinker/test_data/ddrdebug_3.bin", std::ios::in | std::ios::binary);
								ifs_param33.read((char*)(*ddrdebug_3), ddrsize * 30 * sizeof(uint256));
								ifs_param33.close(); */
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
							aload_img_2(fm_buf5, ddrdebug_3,
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

							//bias
							for(int i=0;i<2;i++)
							{
								load_bias_from_axi( big_bias[i], bias_port[i+378]);
							}
							set_bias_conv1x1(fm_buf6,big_bias[0],1,1,1,false);
							set_bias_conv1x1(fm_buf7,big_bias[1],1,1,1,false);
							clear_pad2(fm_buf6,2*((320/16)+2),2*((192/16)+2));
							clear_pad2(fm_buf7,2*((320/16)+2),2*((192/16)+2));
							CONV_1x1(fm_buf1,fm_buf6,   wt_buf_big[0],0);
							CONV_1x1(fm_buf2,fm_buf6,   wt_buf_big[1],0);
							CONV_1x1(fm_buf5,fm_buf6,   wt_buf_big[2],0);
							CONV_1x1(fm_buf1,fm_buf7,   wt_buf_big[3],0);
							CONV_1x1(fm_buf2,fm_buf7,   wt_buf_big[4],0);
							CONV_1x1(fm_buf5,fm_buf7,   wt_buf_big[5],0);
							initial_ddr(ddrdebug_3,
													2,
													2*((320/16)+2),
													2*((192/16)+2)
													);
							deload_img(fm_buf6, ddrdebug_3,
														0,
														0,
														0,
														2*((320/16)+2),
														2*((192/16)+2),
														2*((320/16)+2)
														);
							deload_img(fm_buf7, ddrdebug_3,
														1,
														0,
														0,
														2*((320/16)+2),
														2*((192/16)+2),
														2*((320/16)+2)
														);
////////////////////////////////////////////////////////   bilinear the output of 444(in buffer3 and 4) and add to the ddrdebug_3///////////////////////////////////////////////////////
							//load img1 from buf3 to buf1
							//load img1 from buf4 to buf2

							load_oneimageto_ddr_1(fm_buf3,0,fm_buf1);
							load_oneimageto_ddr_1(fm_buf4,0,fm_buf2);
							//bilinear buf1 to buf7
							//bilinear bug2 to buf8
							//store to ddr
							bilinear_1(
									fm_buf1,
										fm_buf7
									);
							bilinear_1(
									fm_buf2,
										fm_buf8
									);
							deload_img(fm_buf7, ddrdebug_3,
														0,
														1,
														1,
														20,
														12,
														2*((320/16)+2)
														);
							deload_img(fm_buf8, ddrdebug_3,
														1,
														1,
														1,
														20,
														12,
														2*((320/16)+2)
														);

							/////////////////////////////////////////////////////////////////////
							//load img1 from buf3 to buf1
							//load img1 from buf4 to buf2
							load_oneimageto_ddr_1(fm_buf3,1,fm_buf1);
							load_oneimageto_ddr_1(fm_buf4,1,fm_buf2);
							//bilinear buf1 to buf7
							//bilinear bug2 to buf8
							//store to ddr
							bilinear_1(
									fm_buf1,
										fm_buf7
									);
							bilinear_1(
									fm_buf2,
										fm_buf8
									);
							deload_img(fm_buf7, ddrdebug_3,
														0,
														1+2+20,
														1,
														20,
														12,
														2*((320/16)+2)
														);
							deload_img(fm_buf8, ddrdebug_3,
														1,
														1+2+20,
														1,
														20,
														12,
														2*((320/16)+2)
														);

							/////////////////////////////////////////////////////////////////////
							//load img1 from buf3 to buf1
							//load img1 from buf4 to buf2
							load_oneimageto_ddr_1(fm_buf3,2,fm_buf1);
							load_oneimageto_ddr_1(fm_buf4,2,fm_buf2);
							//bilinear buf1 to buf7
							//bilinear bug2 to buf8
							//store to ddr
							bilinear_1(
									fm_buf1,
										fm_buf7
									);
							bilinear_1(
									fm_buf2,
										fm_buf8
									);
							deload_img(fm_buf7, ddrdebug_3,
														0,
														1,
														1+2+12,
														20,
														12,
														2*((320/16)+2)
														);
							deload_img(fm_buf8, ddrdebug_3,
														1,
														1,
														1+2+12,
														20,
														12,
														2*((320/16)+2)
														);

							/////////////////////////////////////////////////////////////////////
							//load img1 from buf3 to buf1
							//load img1 from buf4 to buf2
							load_oneimageto_ddr_1(fm_buf3,3,fm_buf1);
							load_oneimageto_ddr_1(fm_buf4,3,fm_buf2);
							//bilinear buf1 to buf7
							//bilinear bug2 to buf8
							//store to ddr
							bilinear_1(
									fm_buf1,
										fm_buf7
									);
							bilinear_1(
									fm_buf2,
										fm_buf8
									);
							deload_img(fm_buf7, ddrdebug_3,
														0,
														1+2+20,
														1+2+12,
														20,
														12,
														2*((320/16)+2)
														);
							deload_img(fm_buf8, ddrdebug_3,
														1,
														1+2+20,
														1+2+12,
														20,
														12,
														2*((320/16)+2)
														);
///////////////////////////////////////////////////////////     1,2 fixed     layer 501 515 517 519 520 ///////////////////////////////////////////////////////////////
							//load ddrdebug_3 to buffer1,2
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
							//load for 501 and conv to buffer3,4:
							initial_ddr(ddrdebug_3,
													6,
													320+2,
													192+2
													);

							index_502b=381;
							index_502w=12;
							load_bias_from_axi(big_bias[0], bias_port[0+index_502b]);  //502
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

							//515 517

							//3,4->7,8
							load_bias_from_axi(big_bias[2], bias_port[2+index_502b]);  //521
							load_bias_from_axi(big_bias[3], bias_port[3+index_502b]);  //
							load_weight_conv3x3( big_w33_buffer[4],w_port_3x3_2[4+index_502w]); // opchannel16 inchannel16
							load_weight_conv3x3( big_w33_buffer[5],w_port_3x3_2[5+index_502w]); // opchannel16 inchannel32
							load_weight_conv3x3( big_w33_buffer[6],w_port_3x3_2[6+index_502w]); // opchannel32 inchannel16
							load_weight_conv3x3( big_w33_buffer[7],w_port_3x3_2[7+index_502w]); // opchannel32 inchannel32
							set_dwbias_conv3x3(fm_buf7,   big_bias[2]);
							set_dwbias_conv3x3(fm_buf8,   big_bias[3]);
							conv3x3_501(fm_buf3,fm_buf7,big_w33_buffer[4]);
							conv3x3_501(fm_buf4,fm_buf7,big_w33_buffer[5]);
							conv3x3_501(fm_buf3,fm_buf8,big_w33_buffer[6]);
							conv3x3_501(fm_buf4,fm_buf8,big_w33_buffer[7]);
							clearpad_for_501(fm_buf7);               //clear x=6 y=10
							clearpad_for_501(fm_buf8);


							//3,4->5,6
							load_bias_from_axi(big_bias[4], bias_port[4+index_502b]);  //523
							load_bias_from_axi(big_bias[5], bias_port[5+index_502b]);  //
							load_weight_conv3x3( big_w33_buffer[8],w_port_3x3_2[8+index_502w]); // opchannel16 inchannel16
							load_weight_conv3x3( big_w33_buffer[9],w_port_3x3_2[9+index_502w]); // opchannel16 inchannel32
							load_weight_conv3x3( big_w33_buffer[10],w_port_3x3_2[10+index_502w]); // opchannel32 inchannel16
							load_weight_conv3x3( big_w33_buffer[11],w_port_3x3_2[11+index_502w]); // opchannel32 inchannel32
							set_dwbias_conv3x3(fm_buf5,   big_bias[4]);
							set_dwbias_conv3x3(fm_buf6,   big_bias[5]);
							conv3x3_501(fm_buf3,fm_buf5,big_w33_buffer[8]);
							conv3x3_501(fm_buf4,fm_buf5,big_w33_buffer[9]);
							conv3x3_501(fm_buf3,fm_buf6,big_w33_buffer[10]);
							conv3x3_501(fm_buf4,fm_buf6,big_w33_buffer[11]);
							clearpad_for_501(fm_buf5);               //clear x=6 y=10
							clearpad_for_501(fm_buf6);

							//519 7,8->3
							load_bias_from_axi(big_bias[6], bias_port[6+index_502b]);  //525
							set_bias_conv1x1(fm_buf3,	  big_bias[6],1,1,1,true);     //16  bias is load
							clear_pad(fm_buf3,20,12);
							clear_pad(fm_buf3,21,13);
							clear_pad(fm_buf3,42,26);
							load_weight_conv1x1( wt_buf_big[0], w_port_1x1[962]);
							load_weight_conv1x1( wt_buf_big[1], w_port_1x1[963]);
							CONV_1x1(fm_buf7,fm_buf3,	wt_buf_big[0],2);
							CONV_1x1(fm_buf8,fm_buf3,	wt_buf_big[1],2);

							//520 5,6->7,8
							load_bias_from_axi(big_bias[7], bias_port[7+index_502b]);  //525
							load_bias_from_axi(big_bias[8], bias_port[8+index_502b]);  //525
							set_bias_conv1x1(fm_buf7,	  big_bias[7],1,1,1,true);     //16  bias is load
							set_bias_conv1x1(fm_buf8,	  big_bias[8],1,1,1,true);     //16  bias is load
							clear_pad(fm_buf7,20,12);
							clear_pad(fm_buf7,21,13);
							clear_pad(fm_buf7,42,26);

							clear_pad(fm_buf8,20,12);
							clear_pad(fm_buf8,21,13);
							clear_pad(fm_buf8,42,26);
							load_weight_conv1x1( wt_buf_big[0], w_port_1x1[964]);
							load_weight_conv1x1( wt_buf_big[1], w_port_1x1[965]);
							load_weight_conv1x1( wt_buf_big[2], w_port_1x1[966]);
							load_weight_conv1x1( wt_buf_big[3], w_port_1x1[967]);
							CONV_1x1(fm_buf5,fm_buf7,	wt_buf_big[0],2);
							CONV_1x1(fm_buf6,fm_buf7,	wt_buf_big[1],2);
							CONV_1x1(fm_buf5,fm_buf8,	wt_buf_big[2],2);
							CONV_1x1(fm_buf6,fm_buf8,	wt_buf_big[3],2);

							deload_img(fm_buf7, ddrdebug_3,
														0,
														0,
														0,
														2*((320/16)+2),
														2*((192/16)+2),
														2*((320/16)+2)
														);
							deload_img(fm_buf8, ddrdebug_3,
														1,
														0,
														0,
														2*((320/16)+2),
														2*((192/16)+2),
														2*((320/16)+2)
														);
							deload_img(fm_buf3, ddrdebug_3,
														2,
														0,
														0,
														2*((320/16)+2),
														2*((192/16)+2),
														2*((320/16)+2)
														);


///////////////////////////////////////////////////////////////////////  layer 442 ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						/*	std::ifstream ifs_param133("C:/Users/f/Desktop/DAC2020__thuThinker/test_data/ddrdebug_2.bin", std::ios::in | std::ios::binary);
													ifs_param133.read((char*)(*ddrdebug_2), ddrsize * 30 * sizeof(uint256));
													ifs_param133.close(); */
													aload_img_2(fm_buf9, ddrdebug_2,
																					0,
																					0,
																					0,
																					((320/8)+2),
																					((192/8)+2),
																					2*((320/8)+2)
																					);     //load image 1 to buffer1
													aload_img_2(fm_buf10, ddrdebug_2,
																					0,
																					42,
																					0,
																					((320/8)+2),
																					((192/8)+2),
																					2*((320/8)+2)
																					);     //load image 1 to buffer1
													aload_img_2(fm_buf3, ddrdebug_2,
																					0,
																					0,
																					26,
																					((320/8)+2),
																					((192/8)+2),
																					2*((320/8)+2)
																					);     //load image 1 to buffer1
													aload_img_2(fm_buf4, ddrdebug_2,
																					0,
																					42,
																					26,
																					((320/8)+2),
																					((192/8)+2),
																					2*((320/8)+2)
																					);     //load image 1 to buffer1
													initial_ddr(ddrdebug_2,
																			2,
																			2*((320/8)+2),
																			2*((192/8)+2)
																			);
													load_weight_conv1x1( wt_buf_big[0], w_port_1x1[0+968]);
													load_weight_conv1x1( wt_buf_big[1], w_port_1x1[1+968]);
													//bias
													for(int i=0;i<2;i++)
													{
														load_bias_from_axi( big_bias[i], bias_port[i+390]);
													}

													///////////img 1
													set_bias_conv1x1(fm_buf5,big_bias[0],1,1,1,true);
													set_bias_conv1x1(fm_buf6,big_bias[1],1,1,1,true);
													clear_pad(fm_buf5, 40,  24);
													clear_pad(fm_buf6, 40,  24);
													clear_pad(fm_buf5, 41,  25);
													clear_pad(fm_buf6, 41,  25);
													CONV_1x1(fm_buf9,fm_buf5,   wt_buf_big[0],0); //
													CONV_1x1(fm_buf9,fm_buf6,   wt_buf_big[1],0); //


													deload_img(fm_buf5, ddrdebug_2,
															0,
															0,
															0,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);
													deload_img(fm_buf6, ddrdebug_2,
															1,
															0,
															0,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);

													///////////img 2
													set_bias_conv1x1(fm_buf5,big_bias[0],1,1,1,true);
													set_bias_conv1x1(fm_buf6,big_bias[1],1,1,1,true);
													clear_pad(fm_buf5, 40,  24);
													clear_pad(fm_buf6, 40,  24);
													clear_pad(fm_buf5, 41,  25);
													clear_pad(fm_buf6, 41,  25);
													CONV_1x1(fm_buf10,fm_buf5,   wt_buf_big[0],0); //
													CONV_1x1(fm_buf10,fm_buf6,   wt_buf_big[1],0); //


													deload_img(fm_buf5, ddrdebug_2,
															0,
															42,
															0,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);
													deload_img(fm_buf6, ddrdebug_2,
															1,
															42,
															0,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);

													///////////img 3
													set_bias_conv1x1(fm_buf5,big_bias[0],1,1,1,true);
													set_bias_conv1x1(fm_buf6,big_bias[1],1,1,1,true);
													clear_pad(fm_buf5, 40,  24);
													clear_pad(fm_buf6, 40,  24);
													clear_pad(fm_buf5, 41,  25);
													clear_pad(fm_buf6, 41,  25);
													CONV_1x1(fm_buf3,fm_buf5,   wt_buf_big[0],0); //
													CONV_1x1(fm_buf3,fm_buf6,   wt_buf_big[1],0); //


													deload_img(fm_buf5, ddrdebug_2,
															0,
															0,
															26,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);
													deload_img(fm_buf6, ddrdebug_2,
															1,
															0,
															26,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);

													///////////img 1
													set_bias_conv1x1(fm_buf5,big_bias[0],1,1,1,true);
													set_bias_conv1x1(fm_buf6,big_bias[1],1,1,1,true);
													clear_pad(fm_buf5, 40,  24);
													clear_pad(fm_buf6, 40,  24);
													clear_pad(fm_buf5, 41,  25);
													clear_pad(fm_buf6, 41,  25);
													CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[0],0); //
													CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[1],0); //


													deload_img(fm_buf5, ddrdebug_2,
															0,
															42,
															26,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);
													deload_img(fm_buf6, ddrdebug_2,
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
							load_oneimageto_ddr_2(fm_buf1,0,fm_buf3);
							load_oneimageto_ddr_2(fm_buf2,0,fm_buf4);
							//bilinear buf3 to buf7
							//bilinear bug4 to buf8
							//store to ddr
							bilinear_2(
									fm_buf3,
										fm_buf7
									);
							bilinear_2(
									fm_buf4,
										fm_buf8
									);
							deload_img(fm_buf7, ddrdebug_2,
														0,
														1,
														1,
														40,
														24,
														2*((320/8)+2)
														);
							deload_img(fm_buf8, ddrdebug_2,
														1,
														1,
														1,
														40,
														24,
														2*((320/8)+2)
														);
							//////////////////////////////////////////////////
							load_oneimageto_ddr_2(fm_buf1,1,fm_buf3);
							load_oneimageto_ddr_2(fm_buf2,1,fm_buf4);
							//bilinear buf3 to buf7
							//bilinear bug4 to buf8
							//store to ddr
							bilinear_2(
									fm_buf3,
										fm_buf7
									);
							bilinear_2(
									fm_buf4,
										fm_buf8
									);
							deload_img(fm_buf7, ddrdebug_2,
														0,
														1+2+40,
														1,
														40,
														24,
														2*((320/8)+2)
														);
							deload_img(fm_buf8, ddrdebug_2,
														1,
														1+2+40,
														1,
														40,
														24,
														2*((320/8)+2)
														);
							//////////////////////////////////////////////////
							load_oneimageto_ddr_2(fm_buf1,2,fm_buf3);
							load_oneimageto_ddr_2(fm_buf2,2,fm_buf4);
							//bilinear buf3 to buf7
							//bilinear bug4 to buf8
							//store to ddr
							bilinear_2(
									fm_buf3,
										fm_buf7
									);
							bilinear_2(
									fm_buf4,
										fm_buf8
									);
							deload_img(fm_buf7, ddrdebug_2,
														0,
														1,
														1+2+24,
														40,
														24,
														2*((320/8)+2)
														);
							deload_img(fm_buf8, ddrdebug_2,
														1,
														1,
														1+2+24,
														40,
														24,
														2*((320/8)+2)
														);
							//////////////////////////////////////////////////
							load_oneimageto_ddr_2(fm_buf1,3,fm_buf3);
							load_oneimageto_ddr_2(fm_buf2,3,fm_buf4);
							//bilinear buf3 to buf7
							//bilinear bug4 to buf8
							//store to ddr
							bilinear_2(
									fm_buf3,
										fm_buf7
									);
							bilinear_2(
									fm_buf4,
										fm_buf8
									);
							deload_img(fm_buf7, ddrdebug_2,
														0,
														1+2+40,
														1+2+24,
														40,
														24,
														2*((320/8)+2)
														);
							deload_img(fm_buf8, ddrdebug_2,
														1,
														1+2+40,
														1+2+24,
														40,
														24,
														2*((320/8)+2)
														);
/////////////////////////////////////////////    layer 500 509 511  513 514 /////////////////////////////////

							//initial ddr
							aload_img_2(fm_buf1, ddrdebug_2,
															0,
															0,
															0,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							aload_img_2(fm_buf2, ddrdebug_2,
															0,
															42,
															0,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							aload_img_2(fm_buf3, ddrdebug_2,
															0,
															0,
															26,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							aload_img_2(fm_buf4, ddrdebug_2,
															0,
															42,
															26,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							aload_img_2(fm_buf5, ddrdebug_2,
															1,
															0,
															0,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							aload_img_2(fm_buf6, ddrdebug_2,
															1,
															42,
															0,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							aload_img_2(fm_buf7, ddrdebug_2,
															1,
															0,
															26,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							aload_img_2(fm_buf8, ddrdebug_2,
															1,
															42,
															26,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1

							//load 1 image to buffer 1,5
							//load 1 image to buffer 2,6
							//load 1 image to buffer 3,7
							//load 1 image to buffer 4,8
							//load weight and bias for 500
							index_502b=392;
							index_502w=24;
							load_bias_from_axi(big_bias[0], bias_port[0+index_502b]);  //500
							load_bias_from_axi(big_bias[1], bias_port[1+index_502b]);  //
							load_weight_conv3x3( big_w33_buffer[0],w_port_3x3_2[0+index_502w]); //502 opchannel16 inchannel16
							load_weight_conv3x3( big_w33_buffer[1],w_port_3x3_2[1+index_502w]); //502 opchannel16 inchannel32
							load_weight_conv3x3( big_w33_buffer[2],w_port_3x3_2[2+index_502w]); //502 opchannel32 inchannel16
							load_weight_conv3x3( big_w33_buffer[3],w_port_3x3_2[3+index_502w]); //502 opchannel32 inchannel32
							//img1: 1,5->9,10
							//img1: 2,6->1,5
							//img1: 3,7->2,6
							//img1: 4,8->3,7
							set_dwbias_conv3x3(fm_buf9,   big_bias[0]);
							set_dwbias_conv3x3(fm_buf10,   big_bias[1]);
							conv3x3_501(fm_buf1,fm_buf9,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
							conv3x3_501(fm_buf5,fm_buf9,big_w33_buffer[1]);
							conv3x3_501(fm_buf1,fm_buf10,big_w33_buffer[2]);
							conv3x3_501(fm_buf5,fm_buf10,big_w33_buffer[3]);
							clearpad_for_500(fm_buf9);               //clear x=6 y=10
							clearpad_for_500(fm_buf10);
							//
							set_dwbias_conv3x3(fm_buf1,   big_bias[0]);
							set_dwbias_conv3x3(fm_buf5,   big_bias[1]);
							conv3x3_501(fm_buf2,fm_buf1,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
							conv3x3_501(fm_buf6,fm_buf1,big_w33_buffer[1]);
							conv3x3_501(fm_buf2,fm_buf5,big_w33_buffer[2]);
							conv3x3_501(fm_buf6,fm_buf5,big_w33_buffer[3]);
							clearpad_for_500(fm_buf1);               //clear x=6 y=10
							clearpad_for_500(fm_buf5);
							//
							set_dwbias_conv3x3(fm_buf2,   big_bias[0]);
							set_dwbias_conv3x3(fm_buf6,   big_bias[1]);
							conv3x3_501(fm_buf3,fm_buf2,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
							conv3x3_501(fm_buf7,fm_buf2,big_w33_buffer[1]);
							conv3x3_501(fm_buf3,fm_buf6,big_w33_buffer[2]);
							conv3x3_501(fm_buf7,fm_buf6,big_w33_buffer[3]);
							clearpad_for_500(fm_buf2);               //clear x=6 y=10
							clearpad_for_500(fm_buf6);
							//
							set_dwbias_conv3x3(fm_buf3,   big_bias[0]);
							set_dwbias_conv3x3(fm_buf7,   big_bias[1]);
							conv3x3_501(fm_buf4,fm_buf3,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
							conv3x3_501(fm_buf8,fm_buf3,big_w33_buffer[1]);
							conv3x3_501(fm_buf4,fm_buf7,big_w33_buffer[2]);
							conv3x3_501(fm_buf8,fm_buf7,big_w33_buffer[3]);
							clearpad_for_500(fm_buf3);               //clear x=6 y=10
							clearpad_for_500(fm_buf7);
							//img1: 1,5->9,10
							//img1: 2,6->1,5
							//img1: 3,7->2,6
							//img1: 4,8->3,7
							/////////////////////////// to fixed /////////////
							//////// layer 509
							//img1: 9,10->4,13
							//img1: 1,5 ->8,14
							//img1: 2,6 ->11,15
							//img1: 3,7 ->12,16
							load_bias_from_axi(big_bias[2], bias_port[2+index_502b]);  //500
							load_bias_from_axi(big_bias[3], bias_port[3+index_502b]);  //
							load_weight_conv3x3( big_w33_buffer[4],w_port_3x3_2[4+index_502w]); //502 opchannel16 inchannel16
							load_weight_conv3x3( big_w33_buffer[5],w_port_3x3_2[5+index_502w]); //502 opchannel16 inchannel32
							load_weight_conv3x3( big_w33_buffer[6],w_port_3x3_2[6+index_502w]); //502 opchannel32 inchannel16
							load_weight_conv3x3( big_w33_buffer[7],w_port_3x3_2[7+index_502w]);
							//
							set_dwbias_conv3x3(fm_buf4,   big_bias[2]);
							set_dwbias_conv3x3(fm_buf13,   big_bias[3]);
							conv3x3_501(fm_buf9,fm_buf4,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
							conv3x3_501(fm_buf10,fm_buf4,big_w33_buffer[5]);
							conv3x3_501(fm_buf9,fm_buf13,big_w33_buffer[6]);
							conv3x3_501(fm_buf10,fm_buf13,big_w33_buffer[7]);
							clearpad_for_500(fm_buf4);               //clear x=6 y=10
							clearpad_for_500(fm_buf13);
							//
							set_dwbias_conv3x3(fm_buf8,   big_bias[2]);
							set_dwbias_conv3x3(fm_buf14,   big_bias[3]);
							conv3x3_501(fm_buf1,fm_buf8,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
							conv3x3_501(fm_buf5,fm_buf8,big_w33_buffer[5]);
							conv3x3_501(fm_buf1,fm_buf14,big_w33_buffer[6]);
							conv3x3_501(fm_buf5,fm_buf14,big_w33_buffer[7]);
							clearpad_for_500(fm_buf8);               //clear x=6 y=10
							clearpad_for_500(fm_buf14);
							//
							set_dwbias_conv3x3(fm_buf11,   big_bias[2]);
							set_dwbias_conv3x3(fm_buf15,   big_bias[3]);
							conv3x3_501(fm_buf2,fm_buf11,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
							conv3x3_501(fm_buf6,fm_buf11,big_w33_buffer[5]);
							conv3x3_501(fm_buf2,fm_buf15,big_w33_buffer[6]);
							conv3x3_501(fm_buf6,fm_buf15,big_w33_buffer[7]);
							clearpad_for_500(fm_buf11);               //clear x=6 y=10
							clearpad_for_500(fm_buf15);
							//
							set_dwbias_conv3x3(fm_buf12,   big_bias[2]);
							set_dwbias_conv3x3(fm_buf16,   big_bias[3]);
							conv3x3_501(fm_buf3,fm_buf12,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
							conv3x3_501(fm_buf7,fm_buf12,big_w33_buffer[5]);
							conv3x3_501(fm_buf3,fm_buf16,big_w33_buffer[6]);
							conv3x3_501(fm_buf7,fm_buf16,big_w33_buffer[7]);
							clearpad_for_500(fm_buf12);               //clear x=6 y=10
							clearpad_for_500(fm_buf16);
							//
							// layer 513
							//img1: 4,13 ->17
							//img1: 8,14 ->4
							//img1: 11,15->13
							//img1: 12,16->8
							load_bias_from_axi(big_bias[4], bias_port[4+index_502b]);  //525
							load_weight_conv1x1( wt_buf_big[0], w_port_1x1[971]);
							load_weight_conv1x1( wt_buf_big[1], w_port_1x1[972]);

							set_bias_conv1x1(fm_buf17,	  big_bias[4],1,1,1,true);     //16  bias is load
							clear_pad(fm_buf17,40,24);
							CONV_1x1(fm_buf4,fm_buf17,	wt_buf_big[0],2);
							CONV_1x1(fm_buf13,fm_buf17,	wt_buf_big[1],2);
							//
							set_bias_conv1x1(fm_buf4,	  big_bias[4],1,1,1,true);     //16  bias is load
							clear_pad(fm_buf4,40,24);
							CONV_1x1(fm_buf8,fm_buf4,	wt_buf_big[0],2);
							CONV_1x1(fm_buf14,fm_buf4,	wt_buf_big[1],2);
							//
							set_bias_conv1x1(fm_buf13,	  big_bias[4],1,1,1,true);     //16  bias is load
							clear_pad(fm_buf13,40,24);
							CONV_1x1(fm_buf11,fm_buf13,	wt_buf_big[0],2);
							CONV_1x1(fm_buf15,fm_buf13,	wt_buf_big[1],2);
							//
							set_bias_conv1x1(fm_buf8,	  big_bias[4],1,1,1,true);     //16  bias is load
							clear_pad(fm_buf8,40,24);
							CONV_1x1(fm_buf12,fm_buf8,	wt_buf_big[0],2);
							CONV_1x1(fm_buf16,fm_buf8,	wt_buf_big[1],2);
							// in ddr3
							deload_img(fm_buf17, ddrdebug_3,
															5,
															0,
															0,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							deload_img(fm_buf4, ddrdebug_3,
															5,
															42,
															0,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							deload_img(fm_buf13, ddrdebug_3,
															5,
															0,
															26,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							deload_img(fm_buf8, ddrdebug_3,
															5,
															42,
															26,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1

							//layer 511
							/////////////////////////// to fixed /////////////
							//////// layer 509
							//img1: 9,10->4,13
							//img1: 1,5 ->8,14
							//img1: 2,6 ->11,15
							//img1: 3,7 ->12,16

							load_bias_from_axi(big_bias[2], bias_port[397]);  //500
							load_bias_from_axi(big_bias[3], bias_port[397+1]);  //
							load_weight_conv3x3( big_w33_buffer[4],w_port_3x3_2[8+index_502w]); //502 opchannel16 inchannel16
							load_weight_conv3x3( big_w33_buffer[5],w_port_3x3_2[9+index_502w]); //502 opchannel16 inchannel32
							load_weight_conv3x3( big_w33_buffer[6],w_port_3x3_2[10+index_502w]); //502 opchannel32 inchannel16
							load_weight_conv3x3( big_w33_buffer[7],w_port_3x3_2[11+index_502w]);
							//
							set_dwbias_conv3x3(fm_buf4,   big_bias[2]);
							set_dwbias_conv3x3(fm_buf13,   big_bias[3]);
							conv3x3_501(fm_buf9,fm_buf4,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
							conv3x3_501(fm_buf10,fm_buf4,big_w33_buffer[5]);
							conv3x3_501(fm_buf9,fm_buf13,big_w33_buffer[6]);
							conv3x3_501(fm_buf10,fm_buf13,big_w33_buffer[7]);
							clearpad_for_500(fm_buf4);               //clear x=6 y=10
							clearpad_for_500(fm_buf13);
							//
							set_dwbias_conv3x3(fm_buf8,   big_bias[2]);
							set_dwbias_conv3x3(fm_buf14,   big_bias[3]);
							conv3x3_501(fm_buf1,fm_buf8,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
							conv3x3_501(fm_buf5,fm_buf8,big_w33_buffer[5]);
							conv3x3_501(fm_buf1,fm_buf14,big_w33_buffer[6]);
							conv3x3_501(fm_buf5,fm_buf14,big_w33_buffer[7]);
							clearpad_for_500(fm_buf8);               //clear x=6 y=10
							clearpad_for_500(fm_buf14);
							//
							set_dwbias_conv3x3(fm_buf11,   big_bias[2]);
							set_dwbias_conv3x3(fm_buf15,   big_bias[3]);
							conv3x3_501(fm_buf2,fm_buf11,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
							conv3x3_501(fm_buf6,fm_buf11,big_w33_buffer[5]);
							conv3x3_501(fm_buf2,fm_buf15,big_w33_buffer[6]);
							conv3x3_501(fm_buf6,fm_buf15,big_w33_buffer[7]);
							clearpad_for_500(fm_buf11);               //clear x=6 y=10
							clearpad_for_500(fm_buf15);
							//
							set_dwbias_conv3x3(fm_buf12,   big_bias[2]);
							set_dwbias_conv3x3(fm_buf16,   big_bias[3]);
							conv3x3_501(fm_buf3,fm_buf12,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
							conv3x3_501(fm_buf7,fm_buf12,big_w33_buffer[5]);
							conv3x3_501(fm_buf3,fm_buf16,big_w33_buffer[6]);
							conv3x3_501(fm_buf7,fm_buf16,big_w33_buffer[7]);
							clearpad_for_500(fm_buf12);               //clear x=6 y=10
							clearpad_for_500(fm_buf16);
							//
							////////////////// 514 ///////////
							//img1: 4,13 ->1,2
							//img1: 8,14 ->3,4
							//img1: 11,15->5,6
							//img1: 12,16->7,8
							load_bias_from_axi(big_bias[4], bias_port[400]);  //525
							load_bias_from_axi(big_bias[5], bias_port[401]);  //525

							load_weight_conv1x1( wt_buf_big[0], w_port_1x1[973]);
							load_weight_conv1x1( wt_buf_big[1], w_port_1x1[974]);
							load_weight_conv1x1( wt_buf_big[2], w_port_1x1[975]);
							load_weight_conv1x1( wt_buf_big[3], w_port_1x1[976]);

							set_bias_conv1x1(fm_buf1,	  big_bias[4],1,1,1,true);     //16  bias is load
							set_bias_conv1x1(fm_buf2,	  big_bias[5],1,1,1,true);     //16  bias is load
							clear_pad(fm_buf1,40,24);
							clear_pad(fm_buf2,40,24);
							CONV_1x1(fm_buf4, fm_buf1,	wt_buf_big[0],2);
							CONV_1x1(fm_buf13,fm_buf1,	wt_buf_big[1],2);
							CONV_1x1(fm_buf4, fm_buf2,	wt_buf_big[2],2);
							CONV_1x1(fm_buf13,fm_buf2,	wt_buf_big[3],2);
							//
							set_bias_conv1x1(fm_buf3,	  big_bias[4],1,1,1,true);     //16  bias is load
							set_bias_conv1x1(fm_buf4,	  big_bias[5],1,1,1,true);     //16  bias is load
							clear_pad(fm_buf3,40,24);
							clear_pad(fm_buf4,40,24);
							CONV_1x1(fm_buf8, fm_buf3,	wt_buf_big[0],2);
							CONV_1x1(fm_buf14,fm_buf3,	wt_buf_big[1],2);
							CONV_1x1(fm_buf8, fm_buf4,	wt_buf_big[2],2);
							CONV_1x1(fm_buf14,fm_buf4,	wt_buf_big[3],2);
							//
							set_bias_conv1x1(fm_buf5,	  big_bias[4],1,1,1,true);     //16  bias is load
							set_bias_conv1x1(fm_buf6,	  big_bias[5],1,1,1,true);     //16  bias is load
							clear_pad(fm_buf5,40,24);
							clear_pad(fm_buf6,40,24);
							CONV_1x1(fm_buf11, fm_buf5,	wt_buf_big[0],2);
							CONV_1x1(fm_buf15,fm_buf5,	wt_buf_big[1],2);
							CONV_1x1(fm_buf11, fm_buf6,	wt_buf_big[2],2);
							CONV_1x1(fm_buf15,fm_buf6,	wt_buf_big[3],2);
							//
							set_bias_conv1x1(fm_buf7,	  big_bias[4],1,1,1,true);     //16  bias is load
							set_bias_conv1x1(fm_buf8,	  big_bias[5],1,1,1,true);     //16  bias is load
							clear_pad(fm_buf7,40,24);
							clear_pad(fm_buf8,40,24);
							CONV_1x1(fm_buf12, fm_buf7,	wt_buf_big[0],2);
							CONV_1x1(fm_buf16,fm_buf7,	wt_buf_big[1],2);
							CONV_1x1(fm_buf12, fm_buf8,	wt_buf_big[2],2);
							CONV_1x1(fm_buf16,fm_buf8,	wt_buf_big[3],2);
							//

							deload_img(fm_buf1, ddrdebug_3,
															3,
															0,
															0,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							deload_img(fm_buf3, ddrdebug_3,
															3,
															42,
															0,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							deload_img(fm_buf5, ddrdebug_3,
															3,
															0,
															26,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							deload_img(fm_buf7, ddrdebug_3,
															3,
															42,
															26,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							deload_img(fm_buf2, ddrdebug_3,
															4,
															0,
															0,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							deload_img(fm_buf4, ddrdebug_3,
															4,
															42,
															0,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							deload_img(fm_buf6, ddrdebug_3,
															4,
															0,
															26,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
							deload_img(fm_buf8, ddrdebug_3,
															4,
															42,
															26,
															((320/8)+2),
															((192/8)+2),
															2*((320/8)+2)
															);     //load image 1 to buffer1
//////////////////////////////////////////         layer 441 //////////////////////////////////////////////////////////////
							aload_img_2(fm_buf9, ddrdebug,
															0,
															0,
															0,
															((320/4)+2),
															((192/4)+2),
															2*((320/4)+2)
															);     //load image 1 to buffer1
							aload_img_2(fm_buf10, ddrdebug,
															0,
															82,
															0,
															((320/4)+2),
															((192/4)+2),
															2*((320/4)+2)
															);     //load image 1 to buffer1
							aload_img_2(fm_buf3, ddrdebug,
															0,
															0,
															50,
															((320/4)+2),
															((192/4)+2),
															2*((320/4)+2)
															);     //load image 1 to buffer1
							aload_img_2(fm_buf4, ddrdebug,
															0,
															82,
															50,
															((320/4)+2),
															((192/4)+2),
															2*((320/4)+2)
															);     //load image 1 to buffer1
							initial_ddr(ddrdebug,
													2,
													2*((320/4)+2),
													2*((192/4)+2)
													);
							load_weight_conv1x1( wt_buf_big[0], w_port_1x1[0+978]);
							load_weight_conv1x1( wt_buf_big[1], w_port_1x1[1+978]);
							//bias
							for(int i=0;i<2;i++)
							{
								load_bias_from_axi( big_bias[i], bias_port[i+402]);
							}

							///////////img 1
							set_bias_conv1x1(fm_buf5,big_bias[0],1,1,1,true);
							set_bias_conv1x1(fm_buf6,big_bias[1],1,1,1,true);
							clear_pad(fm_buf5, 80,  48);
							clear_pad(fm_buf6, 80,  48);
							CONV_1x1(fm_buf9,fm_buf5,   wt_buf_big[0],0); //
							CONV_1x1(fm_buf9,fm_buf6,   wt_buf_big[1],0); //


							deload_img(fm_buf5, ddrdebug,
									0,
									0,
									0,
									((320/4)+2),
									((192/4)+2),
									2*((320/4)+2)
									);
							deload_img(fm_buf6, ddrdebug,
									1,
									0,
									0,
									((320/4)+2),
									((192/4)+2),
									2*((320/4)+2)
									);

							///////////img 2
							set_bias_conv1x1(fm_buf5,big_bias[0],1,1,1,true);
							set_bias_conv1x1(fm_buf6,big_bias[1],1,1,1,true);
							clear_pad(fm_buf5, 80,  48);
							clear_pad(fm_buf6, 80,  48);
							CONV_1x1(fm_buf10,fm_buf5,   wt_buf_big[0],0); //
							CONV_1x1(fm_buf10,fm_buf6,   wt_buf_big[1],0); //


							deload_img(fm_buf5, ddrdebug,
									0,
									82,
									0,
									((320/4)+2),
									((192/4)+2),
									2*((320/4)+2)
									);
							deload_img(fm_buf6, ddrdebug,
									1,
									82,
									0,
									((320/4)+2),
									((192/4)+2),
									2*((320/4)+2)
									);

							///////////img 3
							set_bias_conv1x1(fm_buf5,big_bias[0],1,1,1,true);
							set_bias_conv1x1(fm_buf6,big_bias[1],1,1,1,true);
							clear_pad(fm_buf5, 80,  48);
							clear_pad(fm_buf6, 80,  48);
							CONV_1x1(fm_buf3,fm_buf5,   wt_buf_big[0],0); //
							CONV_1x1(fm_buf3,fm_buf6,   wt_buf_big[1],0); //


							deload_img(fm_buf5, ddrdebug,
									0,
									0,
									50,
									((320/4)+2),
									((192/4)+2),
									2*((320/4)+2)
									);
							deload_img(fm_buf6, ddrdebug,
									1,
									0,
									50,
									((320/4)+2),
									((192/4)+2),
									2*((320/4)+2)
									);

							///////////img 1
							set_bias_conv1x1(fm_buf5,big_bias[0],1,1,1,true);
							set_bias_conv1x1(fm_buf6,big_bias[1],1,1,1,true);
							clear_pad(fm_buf5, 80,  48);
							clear_pad(fm_buf6, 80,  48);
							CONV_1x1(fm_buf4,fm_buf5,   wt_buf_big[0],0); //
							CONV_1x1(fm_buf4,fm_buf6,   wt_buf_big[1],0); //


							deload_img(fm_buf5, ddrdebug,
									0,
									82,
									50,
									((320/4)+2),
									((192/4)+2),
									2*((320/4)+2)
									);
							deload_img(fm_buf6, ddrdebug,
									1,
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
														aload_img_2(fm_buf3, ddrdebug_2,
																0,
																1,
																1,
																((320/8)),
																((192/8)),
																2*((320/8)+2)
																);
														aload_img_2(fm_buf4, ddrdebug_2,
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
																								fm_buf7
																							);
														bilinear_3(
																							fm_buf4,
																								fm_buf8
																							);
														deload_img(fm_buf7, ddrdebug,
																					0,
																					1,
																					1,
																					80,
																					48,
																					2*((320/4)+2)
																					);
														deload_img(fm_buf8, ddrdebug,
																					1,
																					1,
																					1,
																					80,
																					48,
																					2*((320/4)+2)
																					);
											//////////////////////////////////////////////////
														aload_img_2(fm_buf3, ddrdebug_2,
																0,
																43,
																1,
																((320/8)),
																((192/8)),
																2*((320/8)+2)
																);
														aload_img_2(fm_buf4, ddrdebug_2,
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
																								fm_buf7
																							);
														bilinear_3(
																							fm_buf4,
																								fm_buf8
																							);
														deload_img(fm_buf7, ddrdebug,
																					0,
																					1+2+80,
																					1,
																					80,
																					48,
																					2*((320/4)+2)
																					);
														deload_img(fm_buf8, ddrdebug,
																					1,
																					1+2+80,
																					1,
																					80,
																					48,
																					2*((320/4)+2)
																					);
											//////////////////////////////////////////////////
														aload_img_2(fm_buf3, ddrdebug_2,
																0,
																1,
																1+2+24,
																((320/8)),
																((192/8)),
																2*((320/8)+2)
																);
														aload_img_2(fm_buf4, ddrdebug_2,
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
																								fm_buf7
																							);
														bilinear_3(
																							fm_buf4,
																								fm_buf8
																							);
														deload_img(fm_buf7, ddrdebug,
																					0,
																					1,
																					1+2+48,
																					80,
																					48,
																					2*((320/4)+2)
																					);
														deload_img(fm_buf8, ddrdebug,
																					1,
																					1,
																					1+2+48,
																					80,
																					48,
																					2*((320/4)+2)
																					);
											//////////////////////////////////////////////////
														aload_img_2(fm_buf3, ddrdebug_2,
																0,
																1+2+40,
																1+2+24,
																((320/8)),
																((192/8)),
																2*((320/8)+2)
																);
														aload_img_2(fm_buf4, ddrdebug_2,
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
																								fm_buf7
																							);
														bilinear_3(
																							fm_buf4,
																								fm_buf8
																							);
														deload_img(fm_buf7, ddrdebug,
																					0,
																					1+2+80,
																					1+2+48,
																					80,
																					48,
																					2*((320/4)+2)
																					);
														deload_img(fm_buf8, ddrdebug,
																					1,
																					1+2+80,
																					1+2+48,
																					80,
																					48,
																					2*((320/4)+2)
																					);
////////////////////////////////////////////	499 503 505 507 508 ///////////////////////////////////////////////////

									aload_img_2(fm_buf1, ddrdebug,
																	0,
																	0,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									aload_img_2(fm_buf2, ddrdebug,
																	0,
																	82,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									aload_img_2(fm_buf3, ddrdebug,
																	0,
																	0,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									aload_img_2(fm_buf4, ddrdebug,
																	0,
																	82,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									aload_img_2(fm_buf5, ddrdebug,
																	1,
																	0,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									aload_img_2(fm_buf6, ddrdebug,
																	1,
																	82,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									aload_img_2(fm_buf7, ddrdebug,
																	1,
																	0,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									aload_img_2(fm_buf8, ddrdebug,
																	1,
																	82,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									//load 1 image to buffer 1,5
									//load 1 image to buffer 2,6
									//load 1 image to buffer 3,7
									//load 1 image to buffer 4,8
									//load weight and bias for 500
									index_502b=404;
									index_502w=36;
									load_bias_from_axi(big_bias[0], bias_port[0+index_502b]);  //500
									load_bias_from_axi(big_bias[1], bias_port[1+index_502b]);  //
									load_weight_conv3x3( big_w33_buffer[0],w_port_3x3_2[0+index_502w]); //502 opchannel16 inchannel16
									load_weight_conv3x3( big_w33_buffer[1],w_port_3x3_2[1+index_502w]); //502 opchannel16 inchannel32
									load_weight_conv3x3( big_w33_buffer[2],w_port_3x3_2[2+index_502w]); //502 opchannel32 inchannel16
									load_weight_conv3x3( big_w33_buffer[3],w_port_3x3_2[3+index_502w]); //502 opchannel32 inchannel32
									//img1: 1,5->9,10
									//img1: 2,6->1,5
									//img1: 3,7->2,6
									//img1: 4,8->3,7
									set_dwbias_conv3x3(fm_buf9,   big_bias[0]);
									set_dwbias_conv3x3(fm_buf10,   big_bias[1]);
									conv3x3_499(fm_buf1,fm_buf9,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf5,fm_buf9,big_w33_buffer[1]);
									conv3x3_499(fm_buf1,fm_buf10,big_w33_buffer[2]);
									conv3x3_499(fm_buf5,fm_buf10,big_w33_buffer[3]);
									clearpad_for_499(fm_buf9);               //clear x=6 y=10
									clearpad_for_499(fm_buf10);
									//
									set_dwbias_conv3x3(fm_buf1,   big_bias[0]);
									set_dwbias_conv3x3(fm_buf5,   big_bias[1]);
									conv3x3_499(fm_buf2,fm_buf1,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf6,fm_buf1,big_w33_buffer[1]);
									conv3x3_499(fm_buf2,fm_buf5,big_w33_buffer[2]);
									conv3x3_499(fm_buf6,fm_buf5,big_w33_buffer[3]);
									clearpad_for_499(fm_buf1);               //clear x=6 y=10
									clearpad_for_499(fm_buf5);
									//
									set_dwbias_conv3x3(fm_buf2,   big_bias[0]);
									set_dwbias_conv3x3(fm_buf6,   big_bias[1]);
									conv3x3_499(fm_buf3,fm_buf2,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf7,fm_buf2,big_w33_buffer[1]);
									conv3x3_499(fm_buf3,fm_buf6,big_w33_buffer[2]);
									conv3x3_499(fm_buf7,fm_buf6,big_w33_buffer[3]);
									clearpad_for_499(fm_buf2);               //clear x=6 y=10
									clearpad_for_499(fm_buf6);
									//
									set_dwbias_conv3x3(fm_buf3,   big_bias[0]);
									set_dwbias_conv3x3(fm_buf7,   big_bias[1]);
									conv3x3_499(fm_buf4,fm_buf3,big_w33_buffer[0]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf8,fm_buf3,big_w33_buffer[1]);
									conv3x3_499(fm_buf4,fm_buf7,big_w33_buffer[2]);
									conv3x3_499(fm_buf8,fm_buf7,big_w33_buffer[3]);
									clearpad_for_499(fm_buf3);               //clear x=6 y=10
									clearpad_for_499(fm_buf7);
									//
									/////////////////////////// to fixed /////////////
									//////// layer 503
									//img1: 9,10->4,13
									//img1: 1,5 ->8,14
									//img1: 2,6 ->11,15
									//img1: 3,7 ->12,16
									load_bias_from_axi(big_bias[2], bias_port[2+index_502b]);  //500
									load_bias_from_axi(big_bias[3], bias_port[3+index_502b]);  //
									load_weight_conv3x3( big_w33_buffer[4],w_port_3x3_2[4+index_502w]); //502 opchannel16 inchannel16
									load_weight_conv3x3( big_w33_buffer[5],w_port_3x3_2[5+index_502w]); //502 opchannel16 inchannel32
									load_weight_conv3x3( big_w33_buffer[6],w_port_3x3_2[6+index_502w]); //502 opchannel32 inchannel16
									load_weight_conv3x3( big_w33_buffer[7],w_port_3x3_2[7+index_502w]);
									//
									set_dwbias_conv3x3(fm_buf4,   big_bias[2]);
									set_dwbias_conv3x3(fm_buf13,   big_bias[3]);
									conv3x3_499(fm_buf9,fm_buf4,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf10,fm_buf4,big_w33_buffer[5]);
									conv3x3_499(fm_buf9,fm_buf13,big_w33_buffer[6]);
									conv3x3_499(fm_buf10,fm_buf13,big_w33_buffer[7]);
									clearpad_for_499(fm_buf4);               //clear x=6 y=10
									clearpad_for_499(fm_buf13);
									//
									set_dwbias_conv3x3(fm_buf8,   big_bias[2]);
									set_dwbias_conv3x3(fm_buf14,   big_bias[3]);
									conv3x3_499(fm_buf1,fm_buf8,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf5,fm_buf8,big_w33_buffer[5]);
									conv3x3_499(fm_buf1,fm_buf14,big_w33_buffer[6]);
									conv3x3_499(fm_buf5,fm_buf14,big_w33_buffer[7]);
									clearpad_for_499(fm_buf8);               //clear x=6 y=10
									clearpad_for_499(fm_buf14);
									//
									set_dwbias_conv3x3(fm_buf11,   big_bias[2]);
									set_dwbias_conv3x3(fm_buf15,   big_bias[3]);
									conv3x3_499(fm_buf2,fm_buf11,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf6,fm_buf11,big_w33_buffer[5]);
									conv3x3_499(fm_buf2,fm_buf15,big_w33_buffer[6]);
									conv3x3_499(fm_buf6,fm_buf15,big_w33_buffer[7]);
									clearpad_for_499(fm_buf11);               //clear x=6 y=10
									clearpad_for_499(fm_buf15);
									//
									set_dwbias_conv3x3(fm_buf12,   big_bias[2]);
									set_dwbias_conv3x3(fm_buf16,   big_bias[3]);
									conv3x3_499(fm_buf3,fm_buf12,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf7,fm_buf12,big_w33_buffer[5]);
									conv3x3_499(fm_buf3,fm_buf16,big_w33_buffer[6]);
									conv3x3_499(fm_buf7,fm_buf16,big_w33_buffer[7]);
									clearpad_for_499(fm_buf12);               //clear x=6 y=10
									clearpad_for_499(fm_buf16);
									//
									// layer 507
									//img1: 4,13 ->17
									//img1: 8,14 ->4
									//img1: 11,15->13
									//img1: 12,16->8
									load_bias_from_axi(big_bias[4], bias_port[4+index_502b]);  //525
									load_weight_conv1x1( wt_buf_big[0], w_port_1x1[981]);
									load_weight_conv1x1( wt_buf_big[1], w_port_1x1[982]);

									set_bias_conv1x1(fm_buf17,	  big_bias[4],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf17,40,24);
									CONV_1x1(fm_buf4,fm_buf17,	wt_buf_big[0],2);
									CONV_1x1(fm_buf13,fm_buf17,	wt_buf_big[1],2);
									//
									set_bias_conv1x1(fm_buf4,	  big_bias[4],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf4,40,24);
									CONV_1x1(fm_buf8,fm_buf4,	wt_buf_big[0],2);
									CONV_1x1(fm_buf14,fm_buf4,	wt_buf_big[1],2);
									//
									set_bias_conv1x1(fm_buf13,	  big_bias[4],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf13,40,24);
									CONV_1x1(fm_buf11,fm_buf13,	wt_buf_big[0],2);
									CONV_1x1(fm_buf15,fm_buf13,	wt_buf_big[1],2);
									//
									set_bias_conv1x1(fm_buf8,	  big_bias[4],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf8,40,24);
									CONV_1x1(fm_buf12,fm_buf8,	wt_buf_big[0],2);
									CONV_1x1(fm_buf16,fm_buf8,	wt_buf_big[1],2);
									initial_ddr(ddrdebug,
															3,
															2*((320/4)+2),
															2*((192/4)+2)
															);
									deload_img(fm_buf17, ddrdebug,
																	2,
																	0,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									deload_img(fm_buf4, ddrdebug,
																	2,
																	82,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									deload_img(fm_buf13, ddrdebug,
																	2,
																	0,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									deload_img(fm_buf8, ddrdebug,
																	2,
																	82,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									//////////////////   505 ///////////////////////
									/////////////////////////// to fixed /////////////
									//////// layer 509
									//img1: 9,10->4,13
									//img1: 1,5 ->8,14
									//img1: 2,6 ->11,15
									//img1: 3,7 ->12,16

									load_bias_from_axi(big_bias[2], bias_port[410]);  //500
									load_bias_from_axi(big_bias[3], bias_port[410+1]);  //
									load_weight_conv3x3( big_w33_buffer[4],w_port_3x3_2[8+index_502w]); //502 opchannel16 inchannel16
									load_weight_conv3x3( big_w33_buffer[5],w_port_3x3_2[9+index_502w]); //502 opchannel16 inchannel32
									load_weight_conv3x3( big_w33_buffer[6],w_port_3x3_2[10+index_502w]); //502 opchannel32 inchannel16
									load_weight_conv3x3( big_w33_buffer[7],w_port_3x3_2[11+index_502w]);
									//
									set_dwbias_conv3x3(fm_buf4,   big_bias[2]);
									set_dwbias_conv3x3(fm_buf13,   big_bias[3]);
									conv3x3_499(fm_buf9,fm_buf4,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf10,fm_buf4,big_w33_buffer[5]);
									conv3x3_499(fm_buf9,fm_buf13,big_w33_buffer[6]);
									conv3x3_499(fm_buf10,fm_buf13,big_w33_buffer[7]);
									clearpad_for_499(fm_buf4);               //clear x=6 y=10
									clearpad_for_499(fm_buf13);
									//
									set_dwbias_conv3x3(fm_buf8,   big_bias[2]);
									set_dwbias_conv3x3(fm_buf14,   big_bias[3]);
									conv3x3_499(fm_buf1,fm_buf8,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf5,fm_buf8,big_w33_buffer[5]);
									conv3x3_499(fm_buf1,fm_buf14,big_w33_buffer[6]);
									conv3x3_499(fm_buf5,fm_buf14,big_w33_buffer[7]);
									clearpad_for_499(fm_buf8);               //clear x=6 y=10
									clearpad_for_499(fm_buf14);
									//
									set_dwbias_conv3x3(fm_buf11,   big_bias[2]);
									set_dwbias_conv3x3(fm_buf15,   big_bias[3]);
									conv3x3_499(fm_buf2,fm_buf11,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf6,fm_buf11,big_w33_buffer[5]);
									conv3x3_499(fm_buf2,fm_buf15,big_w33_buffer[6]);
									conv3x3_499(fm_buf6,fm_buf15,big_w33_buffer[7]);
									clearpad_for_499(fm_buf11);               //clear x=6 y=10
									clearpad_for_499(fm_buf15);
									//
									set_dwbias_conv3x3(fm_buf12,   big_bias[2]);
									set_dwbias_conv3x3(fm_buf16,   big_bias[3]);
									conv3x3_499(fm_buf3,fm_buf12,big_w33_buffer[4]);  //in buffer1,2, index [c][x][y]  y 1-10 12-21 is data, 11 should be 0 ;  x 1-6  8-13 is data ,7 should be 0
									conv3x3_499(fm_buf7,fm_buf12,big_w33_buffer[5]);
									conv3x3_499(fm_buf3,fm_buf16,big_w33_buffer[6]);
									conv3x3_499(fm_buf7,fm_buf16,big_w33_buffer[7]);
									clearpad_for_499(fm_buf12);               //clear x=6 y=10
									clearpad_for_499(fm_buf16);
									//
									////////////////// 508 ///////////
									//img1: 4,13 ->1,2
									//img1: 8,14 ->3,4
									//img1: 11,15->5,6
									//img1: 12,16->7,8
									load_bias_from_axi(big_bias[4], bias_port[412]);  //525
									load_bias_from_axi(big_bias[5], bias_port[413]);  //525

									load_weight_conv1x1( wt_buf_big[0], w_port_1x1[983]);
									load_weight_conv1x1( wt_buf_big[1], w_port_1x1[984]);
									load_weight_conv1x1( wt_buf_big[2], w_port_1x1[985]);
									load_weight_conv1x1( wt_buf_big[3], w_port_1x1[986]);

									set_bias_conv1x1(fm_buf1,	  big_bias[4],1,1,1,true);     //16  bias is load
									set_bias_conv1x1(fm_buf2,	  big_bias[5],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf1,40,24);
									//clear_pad(fm_buf2,40,24);
									CONV_1x1(fm_buf4, fm_buf1,	wt_buf_big[0],2);
									CONV_1x1(fm_buf13,fm_buf1,	wt_buf_big[1],2);
									CONV_1x1(fm_buf4, fm_buf2,	wt_buf_big[2],2);
									CONV_1x1(fm_buf13,fm_buf2,	wt_buf_big[3],2);
									//
									set_bias_conv1x1(fm_buf3,	  big_bias[4],1,1,1,true);     //16  bias is load
									set_bias_conv1x1(fm_buf4,	  big_bias[5],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf3,40,24);
									//clear_pad(fm_buf4,40,24);
									CONV_1x1(fm_buf8, fm_buf3,	wt_buf_big[0],2);
									CONV_1x1(fm_buf14,fm_buf3,	wt_buf_big[1],2);
									CONV_1x1(fm_buf8, fm_buf4,	wt_buf_big[2],2);
									CONV_1x1(fm_buf14,fm_buf4,	wt_buf_big[3],2);
									//
									set_bias_conv1x1(fm_buf5,	  big_bias[4],1,1,1,true);     //16  bias is load
									set_bias_conv1x1(fm_buf6,	  big_bias[5],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf5,40,24);
									//clear_pad(fm_buf6,40,24);
									CONV_1x1(fm_buf11, fm_buf5,	wt_buf_big[0],2);
									CONV_1x1(fm_buf15,fm_buf5,	wt_buf_big[1],2);
									CONV_1x1(fm_buf11, fm_buf6,	wt_buf_big[2],2);
									CONV_1x1(fm_buf15,fm_buf6,	wt_buf_big[3],2);
									//
									set_bias_conv1x1(fm_buf7,	  big_bias[4],1,1,1,true);     //16  bias is load
									set_bias_conv1x1(fm_buf8,	  big_bias[5],1,1,1,true);     //16  bias is load
									//clear_pad(fm_buf7,40,24);
									//clear_pad(fm_buf8,40,24);
									CONV_1x1(fm_buf12, fm_buf7,	wt_buf_big[0],2);
									CONV_1x1(fm_buf16,fm_buf7,	wt_buf_big[1],2);
									CONV_1x1(fm_buf12, fm_buf8,	wt_buf_big[2],2);
									CONV_1x1(fm_buf16,fm_buf8,	wt_buf_big[3],2);
									//
									deload_img(fm_buf1, ddrdebug,
																	0,
																	0,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									deload_img(fm_buf3, ddrdebug,
																	0,
																	82,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									deload_img(fm_buf5, ddrdebug,
																	0,
																	0,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									deload_img(fm_buf7, ddrdebug,
																	0,
																	82,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									deload_img(fm_buf2, ddrdebug,
																	1,
																	0,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									deload_img(fm_buf4, ddrdebug,
																	1,
																	82,
																	0,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									deload_img(fm_buf6, ddrdebug,
																	1,
																	0,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1
									deload_img(fm_buf8, ddrdebug,
																	1,
																	82,
																	50,
																	((320/4)+2),
																	((192/4)+2),
																	2*((320/4)+2)
																	);     //load image 1 to buffer1

}
