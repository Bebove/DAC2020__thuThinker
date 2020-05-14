#include "dac.h"
#include <bitset>
// Those are the ports which should not be changed unless the Thinker IO is changed
uint16  IMG[imagesize];
uint256 ddrdebug [ddrsize][30];
uint256 ddrdebug_2 [ddrsize][30];
uint512 w3[500][3][3];
uint256 w1[500][16];
uint256 bias_port[500][5];
uint16  debug[2];


//Those are the paths which should be changed according to your env
/*#define imgpath "F:/hls/thinker/code/test_data/data/img.bin"
#define l1 "F:/hls/thinker/code/test_data/data/conv_1.bin"
#define l1_relu "F:/hls/thinker/code/test_data/data/clip_1.bin"
#define l2 "F:/hls/thinker/code/test_data/data/conv_2.bin"*/
#define imgpath "C:\\Users\\f\\Desktop\\github\\DAC2020__thuThinker\\verify\\test_data\\dataimg.bin"
#define w3_310 "C:\\Users\\f\\Desktop\\github\\DAC2020__thuThinker\\weightdata\\w1\\310.bin"
#define bs_307 "C:\\Users\\f\\Desktop\\github\\DAC2020__thuThinker\\weightdata\\w1\\307bs.bin"
#define bs_310 "C:\\Users\\f\\Desktop\\github\\DAC2020__thuThinker\\weightdata\\w1\\310bs.bin"
#define bs_313 "C:\\Users\\f\\Desktop\\github\\DAC2020__thuThinker\\weightdata\\w1\\313bs.bin"
#define w1_307 "C:\\Users\\f\\Desktop\\github\\DAC2020__thuThinker\\weightdata\\w1\\307.bin"


//Those are the vars which is not related to Thinker itself and related to debug
double temp_img[imagesize];
uint16 check[imagesize];//correct data
#define imgsize_l2 381024//6*(96+2)*2*(160+2)*2
#define ifcheck 0











//Those are functions to fold data
void fold_data(uint16 IMG[imagesize],const char *filepath,int fmsize=imagesize)
{

	int ip1;
	ip1=imagesize;
	std::ifstream file(filepath,std::ios::in | std::ios::binary);
	file.read((char *)(&temp_img),fmsize*sizeof(double));
	file.close();
	int i=0;

	while(i<ip1)
	{
		IMG[i].range(fm_lenth, 0)=((fm_type)temp_img[i]).range(fm_lenth, 0);
		i=i+1;
	}

}

void fold_w1_toport(uint256 w1[500][16])
{
	//w_port_1x1[500][16],
	//choose 1 index, get 16 uint256.
	//every uint256 is 16 weight.


	//307 layer : index 0
	float temp[6][3][1][1];
	std::ifstream ifs_param(w1_307, std::ios::in | std::ios::binary);
	ifs_param.read((char*)(**temp), 6 * 3 * 1 * sizeof(float));
	ifs_param.close();
	for(int co = 0; co< 6; co++)
	{
		for(int ci = 0; ci < 3; ci++)
		{
			w1[0][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp[co][ci][0][0]).range(wt_lenth, 0);
		}
	}

}

void fold_BS_toport(uint256 bias_port[500][5])
{
	//307 layer : index 0
	float temp[6];
	std::ifstream ifs_param(bs_307, std::ios::in | std::ios::binary);
	ifs_param.read((char*)(temp), 6 * sizeof(float));
	ifs_param.close();
	for(int ci = 0; ci < 6; ci++)
	{
		bias_port[0][0].range(wt_lenth + ci*16, ci*16)=((bs_type)temp[ci]).range(wt_lenth, 0);
	}



	//310 layer : index 1
	float temp2[6];
	std::ifstream ifs_param1(bs_310, std::ios::in | std::ios::binary);
	ifs_param1.read((char*)(temp2), 6 * sizeof(float));
	ifs_param1.close();
	for(int ci = 0; ci < 6; ci++)
	{
		bias_port[1][0].range(wt_lenth + ci*16, ci*16)=((bs_type)temp2[ci]).range(wt_lenth, 0);
	}



	//313 layer : index 2
	float temp3[16];
	std::ifstream ifs_param2(bs_313, std::ios::in | std::ios::binary);
	ifs_param2.read((char*)(temp3), 16 * sizeof(float));
	ifs_param2.close();
	for(int ci = 0; ci < 16; ci++)
	{
		bias_port[2][0].range(wt_lenth + ci*16, ci*16)=((bs_type)temp3[ci]).range(wt_lenth, 0);
	}


}

void fold_w3_toport(uint512 w3[500][3][3])
{
	//310 layer : index 0,1,2
	float temp[6][6][3][3];
	std::ifstream ifs_param(w3_310, std::ios::in | std::ios::binary);
	ifs_param.read((char*)(**temp), 6 * 6 * 9 * sizeof(float));
	ifs_param.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int i=0;i<6;i++)
			{
				w3[0][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp[0][i][x][y])).range(wt_lenth, 0);
			}

		}
	}

/*
			printf("%f",temp[0][1][1][1]);
			wt_type tss;
			tss.range(wt_lenth, 0)=w3[0][1][1].range(wt_lenth +16 ,16);
			printf("%f",(float)(tss));
*/
}








//Those are functions only for debug
void deload_img(fm_type img_buf[80][50][82], uint16 image_port[imagesize],
							int col, int row, int offset_h , int offset_w ,
							int channel,int channel_offset,int relu,
							int all_image_w=(160+2)*2,
							int all_image_h=(96+2)*2,
							int buffer_w=40,
							int buffer_h=24)
{
	uint16* port_pointer; //col: 192;	row: 320; 	offset_w: when row=4,do the offset;		offset_h: when col=4,do the offset
	int OFFSET_ALL=(col*buffer_h +2* offset_h) * (all_image_w) + row*buffer_w +2*offset_w;
	OFFSET_ALL = OFFSET_ALL+ 1 + all_image_w;// this add pad ////////////////////////////////////////// important

	for(int c = 0; c < channel; c++)
	{

	port_pointer=image_port+(c+channel_offset)*all_image_w*all_image_h+	OFFSET_ALL;
	for(int i = 0; i < buffer_h; i++)
	{

		for(int j = 0; j < buffer_w; j++)
		{
#pragma HLS pipeline II=2
				 //port_pointer[j].range(fm_lenth, 0)=relu_single(img_buf[c][i][j],relu).range(fm_lenth, 0);}
				port_pointer[j].range(fm_lenth, 0)=img_buf[c][i][j].range(fm_lenth, 0);
		}
		port_pointer += all_image_w;}
	}
}






int main()
{

	const char *img_path=imgpath;
	fold_data(IMG,img_path);



	fold_w3_toport(w3);
	fold_BS_toport(bias_port);
	fold_w1_toport(w1);
    Thinker(	 IMG ,w3,w1,bias_port,ddrdebug,ddrdebug_2,debug);

    return 0;
}
