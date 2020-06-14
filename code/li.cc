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
#pragma HLS array_partition variable=fm_input dim=1 complete
#pragma HLS array_partition variable=fm_output dim=1 complete

	//0,1,2,3,4
	//0,2,4,6,8

		for(int w=0;w<10;w++)
		{
			for(int h=0;h<6;h++)
			{
#pragma HLS pipeline
				for(int c=0;c<16;c++)
				{
#pragma HLS unroll
				fm_output[c][2*h][2*w]=fm_input[c][h][w];
				fm_output[c][2*h+1][2*w]=fm_input[c][h][w];
				fm_output[c][2*h][2*w+1]=fm_input[c][h][w];
				fm_output[c][2*h+1][2*w+1]=fm_input[c][h][w];
			}
		}
	}

}

void load_oneimageto_ddr_1 (
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
void load_oneimageto_ddr_2 (
			fm_type fm_buf1[16][50][82],int choose,fm_type fm_buf2[16][50][82]
		  )
{
#pragma HLS array_partition variable=fm_buf1 dim=1 complete
#pragma HLS array_partition variable=fm_buf2 dim=1 complete
	int index_x[4]={1,23,1,23};
	int index_y[4]={1,1,15,15};
	int w=index_x[choose];
	int h=index_y[choose];


		for(int x=0;x<20;x++){
			for(int y=0;y<12;y++){
#pragma HLS pipeline
				for(int c=0;c<16;c++){
#pragma HLS unroll
				fm_buf2[c][y][x]=fm_buf1[c][y+h][x+w];
			}

		}
	}




}
void bilinear_2(
		fm_type fm_input[16][50][82],
		fm_type fm_output[16][50][82]
	)
{
#pragma HLS array_partition variable=fm_input dim=1 complete
#pragma HLS array_partition variable=fm_output dim=1 complete
	//0,1,2,3,4
	//0,2,4,6,8

		for(int w=0;w<20;w++)
		{
			for(int h=0;h<12;h++)
			{
#pragma HLS pipeline
				for(int c=0;c<16;c++)
				{
#pragma HLS unroll
				fm_output[c][2*h][2*w]=fm_input[c][h][w];
				fm_output[c][2*h+1][2*w]=fm_input[c][h][w];
				fm_output[c][2*h][2*w+1]=fm_input[c][h][w];
				fm_output[c][2*h+1][2*w+1]=fm_input[c][h][w];
			}
		}
	}

}
void bilinear_3(
		fm_type fm_input[16][50][82],
		fm_type fm_output[16][50][82]
	)
{
#pragma HLS array_partition variable=fm_input dim=1 complete
#pragma HLS array_partition variable=fm_output dim=1 complete
	//0,1,2,3,4
	//0,2,4,6,8

		for(int w=0;w<40;w++)
		{
			for(int h=0;h<24;h++)
			{
#pragma HLS pipeline
				for(int c=0;c<16;c++)
				{
#pragma HLS unroll
				fm_output[c][2*h][2*w]=fm_input[c][h][w];
				fm_output[c][2*h+1][2*w]=fm_input[c][h][w];
				fm_output[c][2*h][2*w+1]=fm_input[c][h][w];
				fm_output[c][2*h+1][2*w+1]=fm_input[c][h][w];
			}
		}
	}

}
