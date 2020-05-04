#include "dac.h"
#include <stdio.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <math.h>
#include <fstream>
#include <hls_math.h>
#include <string.h>
using namespace std;
void load_img(fm_type img_buf[80][50][82], uint16 image_port[imagesize],
							int col, int row, int offset_h , int offset_w )
{
	uint16* port_pointer; //col: 192;	row: 320; 	offset_w: when row=4,do the offset;		offset_h: when col=4,do the offset
	int OFFSET_ALL=(col*48 +2* offset_h) * ((320+2)*2) + row*80 +2*offset_w;

	//CHANNEL 0
	port_pointer=image_port	+	OFFSET_ALL;
	for(int i = 0; i < 50; i++)
	{
		for(int j = 0; j < 82; j++)
		{
#pragma HLS pipeline
				img_buf[0][i][j].range(fm_lenth, 0) = port_pointer[j].range(fm_lenth, 0);
		}
		port_pointer += (320+2)*2;

	}


	//CHANNEL 1
	port_pointer=image_port+(320+2)*2*(192+2)*2+	OFFSET_ALL;
	for(int i = 0; i < 50; i++)
	{
		for(int j = 0; j < 82; j++)
		{
#pragma HLS pipeline
				img_buf[1][i][j].range(fm_lenth, 0) = port_pointer[j].range(fm_lenth, 0);
		}
		port_pointer += (320+2)*2;

	}


	//CHANNEL 2
	port_pointer=image_port+2*(320+2)*2*(192+2)*2+	OFFSET_ALL;
	for(int i = 0; i < 50; i++)
	{
		for(int j = 0; j < 82; j++)
		{
#pragma HLS pipeline
				img_buf[2][i][j].range(fm_lenth, 0) = port_pointer[j].range(fm_lenth, 0);
					//if((i%20==0)and (j%20==0))
					//{printf("%f",(float)img_buf[2][i][j]);}
		}
		port_pointer += (320+2)*2;

	}

}


void load_bias_from_axi(bs_type dest[80], uint256 src[5])
{
//#pragma HLS array_partition variable=dest dim=1 complete
	for(int co = 0; co < 5; co++)
	{
#pragma HLS pipeline
		uint256 DATA = 0;
		DATA.range(255, 0) = src[co].range(255, 0);
		for(int ci = 0; ci < 16; ci++)
		{
#pragma HLS unroll
			dest[16*co+ci].range(10, 0) = DATA.range(10 + ci*16, ci*16);  //this means for 3x3 conv, every index(0-500) contains 32 3x3 weight
		}

	}
}

inline fm_type relu_single( fm_type d ,int relu) {
	if(relu ==1){
	if( d > 6 )
		return 6;
	if( d < 0 )
		return 0;}
	return d;
}




void deload_img(fm_type img_buf[80][50][82], uint16 image_port[imagesize],
							int col, int row, int offset_h , int offset_w ,
							int channel,int channel_offset)
{
	uint16* port_pointer; //col: 192;	row: 320; 	offset_w: when row=4,do the offset;		offset_h: when col=4,do the offset
	int OFFSET_ALL=(col*24 +2* offset_h) * ((160+2)*2) + row*40 +2*offset_w;






	for(int c = 0; c < channel; c++)
	{

	port_pointer=image_port+(c+channel_offset)*(160+2)*2*(96+2)*2+	OFFSET_ALL;
	for(int i = 0; i < 24; i++)
	{for(int j = 0; j < 40; j++)
		{
#pragma HLS pipeline
				 port_pointer[j].range(fm_lenth, 0)=img_buf[c][i][j].range(fm_lenth, 0);}
		port_pointer += (160+2)*2;}
	}
}

