#include "dac.h"
#include <stdio.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <math.h>
#include <fstream>
#include <hls_math.h>
#include <string.h>
using namespace std;
fm_type fm_buf1[80][49][81];
fm_type fm_buf2[80][49][81];
fm_type fm_buf3[80][49][81];





wt_type wt_buf3[32][32][3][3];
wt_type dwt_buf3[96][3][3];
wt_type wt_buf1[16][16];


//these buffer is for 3x3 undepth conv. just splite the 32 into 4*8, no other change
fm_type fm_buf_for33[4][8][49][81];
wt_type wt_buf3_for33[4][8][32][3][3];

void load_img(fm_type img_buf[80][49][81], uint16 image_port[imagesize],
							int col, int row, int offset_h = 0, int offset_w = 0)
{
	uint16* port_pointer; //col: 192;	row: 320; 	offset_w: when row=4,do the offset;		offset_h: when col=4,do the offset
	int OFFSET_ALL=(col*48 +2* offset_h)*(320+2)*2 + row*80 +2*offset_w;

	//CHANNEL 0
	port_pointer=image_port	+	OFFSET_ALL;
	for(int i = 0; i < 49; i++)
	{
		for(int j = 0; j < 81; j++)
		{
#pragma HLS pipeline
				img_buf[0][i][j].range(8, 0) = port_pointer[j].range(8, 0);
		}
		port_pointer += (320+2)*2;

	}


	//CHANNEL 1
	port_pointer=image_port+(320+2)*2*(192+2)*2+	OFFSET_ALL;
	for(int i = 0; i < 49; i++)
	{
		for(int j = 0; j < 81; j++)
		{
#pragma HLS pipeline
				img_buf[1][i][j].range(8, 0) = port_pointer[j].range(8, 0);
		}
		port_pointer += (320+2)*2;

	}


	//CHANNEL 2
	port_pointer=image_port+2*(320+2)*2*(192+2)*2+	OFFSET_ALL;
	for(int i = 0; i < 49; i++)
	{
		for(int j = 0; j < 81; j++)
		{
#pragma HLS pipeline
				img_buf[2][i][j].range(8, 0) = port_pointer[j].range(8, 0);
					//if((i%20==0)and (j%20==0))
					//{printf("%f",(float)img_buf[2][i][j]);}
		}
		port_pointer += (320+2)*2;

	}

}

void load_dwweight_conv3x3(wt_type dest[96][3][3], uint512 src[500][3][3],int ofset)
{
//should be able to load 80x3x3 weight. so, we need 3 index to store one layer. The redundancy should be 0.
	for(int m = 0; m < 3; m++)
	{
		for(int n = 0; n < 3; n++)
		{
			for(int co = 0; co < 3; co++)
			{
#pragma HLS pipeline
				uint512 DATA = 0;
				DATA.range(511, 0) = src[co+	ofset	][m][n].range(511, 0);        //the ofset function in there
				for(int ci = 0; ci < 32; ci++)
				{
#pragma HLS unroll
					dest[ci+32*co][m][n].range(10, 0) = DATA.range(10 + ci*16, ci*16);  //this means for 3x3 conv, every index(0-500) contains 32 3x3 weight
				}

			}
		}
	}
}

bs_type bias[80];
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

void set_dwbias_conv3x3( fm_type buf[80][49][81], bs_type bias[80])
{
#pragma HLS array_partition variable=buf dim=1 complete
//#pragma HLS array_partition variable=bias dim=1 complete
	for(int h = 1; h <= 49; h+=2) {
		for(int w = 1; w <= 81; w++) {
#pragma HLS pipeline
			for(int c = 0; c < 80; c++) {
#pragma HLS unroll
				buf[c][h  ][w] = bias[c];
			}
		}
	}
}

void dw_conv_1(fm_type (&in_buf)[80][49][81],
		fm_type (&out_buf)[80][49][81],
		wt_type (&weight)[80][3][3]){
    for(uint2 y=0;y<3;y++){
        for (uint2 x=0;x<3;x++){
            for(uint6 h=0; h<47; h++){
                for(uint6 w=0; w<79; w++){
#pragma HLS PIPELINE
                    for(uint6 ch=0; ch<80; ch++){
                        out_buf[ch][h][w]+=weight[ch][y][x]*in_buf[ch][h+y][w+x];
                        //cout<<weight[ch][y][x]<<"  "<<in_buf[ch][h+y][w+x]<<"\n";
                        //cout<<out_buf[ch][h][w]<<"\n";
                        //system("pause");
                    }
                }
            }
        }
    }
}

void dw_conv_2(fm_type (&in_buf)[80][49][81],
		fm_type (&out_buf)[80][49][81],
		wt_type (&weight)[80][3][3]){
    for(uint2 y=0;y<3;y++){
        for (uint2 x=0;x<3;x++){
            for(uint6 h=0,hi=0; h<24; h++,hi+=2){
                for(uint6 w=0,wi=0; w<40; w++,wi+=2){
#pragma HLS PIPELINE
                    for(uint6 ch=0; ch<80; ch++){
                        out_buf[ch][h][w]+=weight[ch][y][x]*in_buf[ch][hi+y][wi+x];
                    }
                }
            }
        }
    }
}








void test(fm_type fm_buf3[80][49][81])
{
	printf("%f",(float)fm_buf3[0][22][22]);
	printf("%f",(float)fm_buf3[5][22][22]);
}




 
 

void SkyNet(	uint16 image_in_raw_pad[imagesize],
				uint512		w_port_3x3[500][3][3],
				uint256     w_port_1x1[500][16],
				uint256     bias_port[500][5],
				uint16 debug[2])
{
 	
#pragma HLS INTERFACE m_axi depth=3*(320+2)*(192+2) 	port=image_in_raw_pad			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=500*3*3 	port=w_port_3x3			offset=slave	bundle=INPUT
	//16000 3x3 kernel
	//every uint512 store 32 11bit number, so we need 500*3*3. the uint32 store input channel data
#pragma HLS INTERFACE m_axi depth=500 	port=w_port_1x1			offset=slave	bundle=INPUT
	//every uint256 store 16 number, so 1 index is a full 16x16x1x1 data
#pragma HLS INTERFACE m_axi depth=500 	port=bias_port			offset=slave	bundle=INPUT
	//every uint256 store 16 number ,so 1 index is a full 80 data


#pragma HLS INTERFACE s_axilite register	port=return
#pragma HLS INTERFACE m_axi depth=2			port=debug				offset=slave	bundle=OUTPUT
#pragma HLS ALLOCATION instances=conv1x1			 		limit=1 function

	load_weight_conv1x1(wt_buf1, w_port_1x1[1]);   //load all full weight for conv1x1
	load_bias_from_axi(bias, bias_port[0]);              //load all bias 		for conv1x1



	for(int y=0;y<4;y++)
	{
		for(int x=0;x<4;x++)
		{
			load_img(fm_buf1, image_in_raw_pad, x,  y,  0,  0); //load the first small part of image
			set_bias_conv1x1( fm_buf2, bias);               //set  all bias 		for conv3x3
			conv1x1(fm_buf1,fm_buf2,wt_buf1,1,1);
			/*
			for(int i=0;i<4;i++)
			{
#pragma HLS unroll
			    conv3x3(fm_buf1,fm_buf_for33[i],wt_buf3_for33[i]);
			}
			*/

		}
	}


	
    //first layer








	debug[0]=0.1;

}
