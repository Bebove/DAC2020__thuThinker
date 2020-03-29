#include "dac.h"

fm_type fm_buf1[96*82*50];
fm_type fm_buf2[96*82*50];
wt_type wt_buf[480*80];
bs_type bias[480];

void conv3x3(fm_type in_buf[96*82*50],
		fm_type out_buf[96*82*50],
		wt_type weight[480*80],
		uint9 w_i, uint9 h_i ,uint9 ch_i,
		uint9 w_o, uint9 h_o ,uint9 ch_o
		){
    load_bias();//load bias to out_buf before conv
    /*
        cho: channel of output = number of fliter
        h_o,w_o: height(row),width(col) of output
        h_i,w_i: height(row),width(col) of input
        weight[a][b][c][d]：
            weight[x][y][z][q]=weight[x*(b*c*d)+y*(c*d)+z*(d)+q]
    */
    for(uint9 cho=0; cho<ch_o; cho++){
        for (uint9 h=0,hi=0; h<h_o; h++,hi+=2){
            for(uint9 w=0,wi=0; w<w_o; w++,wi+=2){
                for(uint9 chi=0;chi<ch_i;ch++){
                    for(uint9 x=0;x<3;x++){
                        for(uint9 y=0;y<3;y++){
                            uint20 index_o= (uint20)(cho*h_o*w_o+h*w_o+w);
                            uint20 index_i= (uint20)(chi*h_i*w_i+(hi+y)*w_i+(wi+x));
                            uint16 index_w= (uint16)(cho*ch_i*9+chi*9+y*3+x);
                            out_buf[index_o]+=weight[index_w]*in_buf[index_i];
                        }
                    }
                }
            }
        }
    }    

}