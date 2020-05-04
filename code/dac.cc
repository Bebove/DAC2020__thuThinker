#include "dac.h"
#include <stdio.h>
#include <ap_int.h>
#include <ap_fixed.h>
#include <math.h>
#include <fstream>
#include <hls_math.h>
#include <string.h>
using namespace std;
fm_type fm_buf1[80][50][82];
fm_type fm_buf2[80][50][82];
fm_type fm_buf3[80][50][82];





wt_type wt_buf3[32][32][3][3];
wt_type dwt_buf3[96][3][3];
wt_type wt_buf1[16][16];
bs_type bias[80];
bs_type bias2[80];


/*







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



*/




 
 

void Thinker(	uint16 image_in_raw_pad[imagesize],
				uint512		w_port_3x3[500][3][3],
				uint256     w_port_1x1[500][16],
				uint256     bias_port[500][5],

				uint16 ddr1 [imagesize],
				uint16 debug[2])
{
 	
#pragma HLS INTERFACE m_axi depth=3*(320+2)*(192+2) 	port=image_in_raw_pad			offset=slave	bundle=INPUT
#pragma HLS INTERFACE m_axi depth=500*3*3 	port=w_port_3x3			offset=slave	bundle=INPUT
	//16000 3x3 kernel
	//every uint512 store 32 11bit number, so we need 500*3*3. the uint32 store input channel data
#pragma HLS INTERFACE m_axi depth=500 	port=w_port_1x1			offset=slave	bundle=INPUT
	//every uint256 store 16 number, so 1 index is a full 16x16x1x1 data
#pragma HLS INTERFACE m_axi depth=500 	port=bias_port			offset=slave	bundle=INPUT
	//So we use a ddr of size at least 48*(20*8)*(12*8)
#pragma HLS INTERFACE m_axi depth=3*(320+2)*(192+2) 	port=ddr1			offset=slave	bundle=INPUT


#pragma HLS INTERFACE s_axilite register	port=return
#pragma HLS INTERFACE m_axi depth=2			port=debug				offset=slave	bundle=OUTPUT



#pragma HLS ALLOCATION instances=CONV_1x1			 		limit=1 function
#pragma HLS ALLOCATION instances=dw_conv_2			 		limit=1 function



	//layer 307 310
	load_weight_conv1x1(wt_buf1, w_port_1x1[0]);   //load  weight for conv1x1 307  		,   	which is store at the index 0
	load_dwweight_conv3x3(dwt_buf3, w_port_3x3,0); //load  weight for dwconv3x3 310		,	    which is store at the index 0,1,2

	load_bias_from_axi(bias, bias_port[0]);        //load  bias   for conv1x1 ,	    which is store at the index 0
	load_bias_from_axi(bias2, bias_port[1]);       //load  bias   for conv3x3 ,	    which is store at the index 1



	int offsetx;
	int offsety;

	int relu11=0;
	int relu33=1;
	int channelnumber=6;
	int channeloffset=0;

	for(int x=0;x<8;x++)
	{
		for(int y=0;y<8;y++)
		{
			//x: h index, x*48*320=offset
			//y: w index, y*80=offset
			if(x==4){offsetx=1;}
			else{offsetx=0;}

			if(y==4){offsety=1;}
			else{offsety=0;}

			//load_img(fm_buf1, image_in_raw_pad, x,  y,  offsetx,  offsety); //load the first small part of image
			aload_img(fm_buf1, image_in_raw_pad, x,  y,  offsetx,  offsety,3,0,
								(320+2)*2,
								(192+2)*2,
								82,
								50);
			set_bias_conv1x1( fm_buf2, bias,x,y);
			CONV_1x1(fm_buf1,fm_buf2,wt_buf1,0,0,relu11);

			set_dwbias_conv3x3(fm_buf3,bias2);
			dw_conv_2(fm_buf2,fm_buf3,dwt_buf3,6);


			deload_img(fm_buf3, ddr1, x,  y,  offsetx,  offsety,channelnumber,channeloffset,relu33,
								(160+2)*2,
								(96+2)*2,
								40,
								24);

		}
	}



	//layer 313
	load_bias_from_axi(bias, bias_port[2]);       //load  bias   for conv1x1 ,	    which is store at the index 2



	debug[0]=0.1;

}
