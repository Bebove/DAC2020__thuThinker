#include "dac.h"

/*
fm_type fm_buf1[96*82*50];
fm_type fm_buf2[96*82*50];
wt_type wt_buf[480*80];
bs_type bias[480];
*/

void conv3x3(fm_type (&in_buf)[80][49][81],
		fm_type (&out_buf)[80][49][81],
		wt_type (&weight)[32][32][3][3]
		){
    for(uint2 y=0;y<3;y++){
        for (uint2 x=0;x<3;x++){
            for(uint5 h=0,hi=0; h<24; h++,hi+=2){
                for(uint6 w=0,wi=0; w<40; w++,wi+=2){
#pragma HLS PIPELINE
                    for(uint5 cho=0; cho<32; cho++){
                        for(uint5 chi=0;chi<32;chi++){
                            out_buf[cho][h][w]+=weight[cho][chi][y][x]*in_buf[chi][hi+y][wi+x];
                        }
                    }
                }
            }
        }
    }
}

void conv1x1(fm_type (&in_buf)[80][49][81],
		fm_type (&out_buf)[80][49][81],
		wt_type (&weight)[16][16],
		uint4 to, uint4 ti){
    for(uint6 h=0,hi=0; h<49; h++){
        for(uint6 w=0,wi=0; w<81; w++){
#pragma HLS PIPELINE
            for(uint4 cho=0; cho<16; cho++){
                for(uint4 chi=0;chi<16;chi++){
                    out_buf[cho+to][h][w]+=weight[cho][chi]*in_buf[chi+ti][h][w];
                    //cout<<"cho="<<cho<<" chi="<<chi<<" hi="<<hi<<" wi="<<wi<<"\n";
					//cout<<weight[cho][chi]<<"  "<<in_buf[chi][hi][wi]<<"\n";
                    //cout<<out_buf[cho][h][w]<<"\n";
                    //system("pause");
                }
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