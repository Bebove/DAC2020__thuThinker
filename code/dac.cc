#include "dac.h"

fm_type fm_buf1[80][49][81];
fm_type fm_buf2[80][49][81];
fm_type fm_buf3[80][49][81];
wt_type wt_buf3[80][80][3][3];
wt_type wt_buf1[80][80];
bs_type bias[80];


void load_img(fm_type img_buf[80][49][81], uint16 image_in_raw_pad_burst[3*(320+4)*(192+4)*4],
							int col, int row, int offset_h = 0, int offset_w = 0)
{
	uint16* image_in_raw_pad_burst_ptr;
	image_in_raw_pad_burst_ptr=image_in_raw_pad_burst;
	for(int i = 0; i < 49; i++) {
		for(int j = 0; j < 81; j++) {
#pragma HLS pipeline
				img_buf[0][i][j].range(8, 0) = image_in_raw_pad_burst_ptr[j].range(8, 0);
		}
		image_in_raw_pad_burst_ptr += (320+4)*2;

	}

}

void SkyNet(	uint16 image_in_raw_pad[3*(320+4)*(192+4)*4])
{
#pragma HLS INTERFACE m_axi depth=3*(320+4)*(192+4) 	port=image_in_raw_pad			offset=slave	bundle=INPUT



    load_img( fm_buf1, image_in_raw_pad, 0,  0,  0,  0);
}
