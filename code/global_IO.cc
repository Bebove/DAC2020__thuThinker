#include "dac.h"
#include <stdio.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <math.h>
#include <fstream>
#include <hls_math.h>
#include <string.h>
using namespace std;



void load_bias_from_axi(bs_type dest[16], uint256 src)
{
//#pragma HLS array_partition variable=dest dim=1 complete

		for(int ci = 0; ci < 16; ci++)
		{
#pragma HLS unroll
			dest[ci].range(bs_lenth, 0) = src.range(bs_lenth + ci*16, ci*16);  //this means for 3x3 conv, every index(0-500) contains 32 3x3 weight
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


void aload_img(fm_type img_buf[16][50][82], uint16 image_port[imagesize],
							int col, int row, int offset_h , int offset_w ,
							int channel,int channel_offset,
							int all_image_w=(160+2)*2,
							int all_image_h=(96+2)*2,
							int buffer_w=40,
							int buffer_h=24)
{
	uint16* port_pointer; //col: 192;	row: 320; 	offset_w: when row=4,do the offset;		offset_h: when col=4,do the offset
	int OFFSET_ALL=(col*(buffer_h-2) +2* offset_h) * (all_image_w) + row*(buffer_w-2) +2*offset_w;
	//It's the offset of a block start address relative to the start address of the whole channel 

	for(int c = 0; c < channel; c++)
	{

	port_pointer=image_port+(c+channel_offset)*all_image_w*all_image_h+	OFFSET_ALL;
	//channnel start address + offset_all
	for(int i = 0; i < buffer_h; i++)
	{for(int j = 0; j < buffer_w; j++)
		{
#pragma HLS pipeline
				 img_buf[c][i][j].range(fm_lenth, 0)=port_pointer[j].range(fm_lenth, 0);}
		port_pointer += all_image_w;
		//locate pointer to next row
		}
	}
}






void deload_img(fm_type img_buf[16][50][82], uint256 image_port[ddrsize][30],   //use 30 uint256 to store  480 channel
							int ddr_channelX16_index,
							int offsetw,
							int offseth,

							int w,    //80
							int h,    //48
							int allw
							)
{
	//rule:
	int offset_ALL=allw*offseth +offsetw;
	int  pointer;
	uint256 DATA;
	for(int j=0;j<h;j++)
	{
		pointer=offset_ALL	+	allw*j;
		for(int i=0;i<w;i++)
		{

#pragma HLS pipeline
				for(int p=0;p<16;p++)
				{
#pragma HLS unroll
					DATA.range(p*16+fm_lenth,p*16)=img_buf[p][j][i].range(fm_lenth,0);
				}
				image_port[pointer][ddr_channelX16_index].range(255,0)=DATA.range(255,0);

			pointer=pointer+1;
		}
	}
}


void aload_img_2(fm_type img_buf[16][50][82], uint256 image_port[ddrsize][30],   //use 30 uint256 to store  480 channel
							int ddr_channelX16_index,
							int offsetw,
							int offseth,

							int w,    //80
							int h,    //48
							int allw
							)
{
	//rule:
	int offset_ALL=allw*offseth +offsetw;
	int  pointer;
	uint256 DATA;
	for(int j=0;j<h;j++)
	{
		pointer=offset_ALL	+	allw*j;
		for(int i=0;i<w;i++)
		{

#pragma HLS pipeline
				DATA.range(255,0)=image_port[pointer][ddr_channelX16_index].range(255,0);
				for(int p=0;p<16;p++)
				{
#pragma HLS unroll
					img_buf[p][j][i].range(fm_lenth,0)=DATA.range(p*16+fm_lenth,p*16);
				}


			pointer=pointer+1;
		}

	}


}
void initial_ddr(uint256 image_port[ddrsize][30],
							int ddr_channelX16_index,
							int allw,
							int allh
							)
{
	for(int j=0;j<allh;j++)
	{

		for(int i=0;i<allw;i++)
		{
			for(int c=0;c<ddr_channelX16_index;c++)
			{
				image_port[j*allw+i][c]=0;
			}
		}

	}
}
