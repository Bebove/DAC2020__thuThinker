#include "dac.h"
#include <bitset>

#define imgpath "F:/hls/thinker/code/test_data/data/img.bin"
#define l1 "F:/hls/thinker/code/test_data/data/conv_1.bin"
#define l1_relu "F:/hls/thinker/code/test_data/data/clip_1.bin"
#define l2 "F:/hls/thinker/code/test_data/data/conv_2.bin"

#define imgsize_l2 381024//6*(96+2)*2*(160+2)*2

uint16 IMG[imagesize];
uint16 ddr1[imagesize];
uint512 w3[500][3][3];
uint256 w1[500][16];
uint16 debug[2];
uint256   bias_port[500][5];
uint16 check[imagesize];//correct data
double temp_img[imagesize];

void fold_data(uint16 IMG[imagesize],const char *filepath,int fmsize=imagesize)
{
	int ip1;
	ip1=imagesize;
	std::ifstream file(filepath,std::ios::in | std::ios::binary);
	file.read((char *)(&temp_img),fmsize*sizeof(double));
	file.close();
	int i=0;
	/*
	cout<<temp_img[375]<<'\n';
	cout<<temp_img[784]<<'\n';
	cout<<temp_img[972]<<'\n';
	cout<<temp_img[1023]<<'\n';
	cout<<temp_img[5431]<<'\n';
	cout<<temp_img[8920]<<'\n';
	cout<<temp_img[11021]<<'\n';
	cout<<temp_img[80202]<<'\n';
	cout<<temp_img[112342]<<'\n';
	cout<<temp_img[248222]<<'\n';
	cout<<"***********************\n";*/
	while(i<ip1)
	{
		IMG[i].range(8, 0)=((fm_type)temp_img[i]).range(8, 0);
		i=i+1;
	}
	/*cout<<IMG[375]<<'\n';
	cout<<IMG[784]<<'\n';
	cout<<IMG[972]<<'\n';
	cout<<IMG[1023]<<'\n';
	cout<<IMG[5431]<<'\n';
	cout<<IMG[8920]<<'\n';
	cout<<IMG[11021]<<'\n';
	cout<<IMG[80202]<<'\n';
	cout<<IMG[112342]<<'\n';
	cout<<IMG[248222]<<'\n';
	cout<<"***********************\n";*/
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void fold_w1_toport(uint256 w1[500][16])
{
	//w_port_1x1[500][16],
	//choose 1 index, get 16 uint256.
	//every uint256 is 16 weight.


	//307 layer : index 0
	float temp[6][3][1][1];
	std::ifstream ifs_param("F:/hls/thinker/code/test_data/modle_param/l1w.bin", std::ios::in | std::ios::binary);
	ifs_param.read((char*)(**temp), 6 * 3 * 1 * sizeof(float));
	ifs_param.close();
	for(int co = 0; co< 6; co++)
	{
		for(int ci = 0; ci < 3; ci++)
		{
			w1[0][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp[co][ci][0][0]).range(wt_lenth, 0);
		}
	}


    /*
	printf("%f",temp[1][0][0][0]);
	wt_type tss;
	tss.range(wt_lenth, 0)=w1[0][1].range(wt_lenth  ,0);
	printf("%f",(float)(tss));
	*/
}

void fold_BS_toport(uint256 bias_port[500][5])
{
	//307 layer : index 0
	float temp[6];
	std::ifstream ifs_param("F:/hls/thinker/code/test_data/modle_param/l1b.bin", std::ios::in | std::ios::binary);
	ifs_param.read((char*)(temp), 6 * sizeof(float));
	ifs_param.close();
	for(int ci = 0; ci < 6; ci++)
	{
		bias_port[0][0].range(wt_lenth + ci*16, ci*16)=((bs_type)temp[ci]).range(wt_lenth, 0);
	}



	//310 layer : index 1
	float temp2[6];
	std::ifstream ifs_param1("F:/hls/thinker/code/test_data/modle_param/l2b.bin", std::ios::in | std::ios::binary);
	ifs_param1.read((char*)(temp2), 6 * sizeof(float));
	ifs_param1.close();
	for(int ci = 0; ci < 6; ci++)
	{
		bias_port[1][0].range(wt_lenth + ci*16, ci*16)=((bs_type)temp2[ci]).range(wt_lenth, 0);
	}


/*
	//313 layer : index 2
	float temp3[16];
	std::ifstream ifs_param2("C:\\Users\\f\\Desktop\\github\\DAC2020__thuThinker\\weightdata\\w1\\313bs.bin", std::ios::in | std::ios::binary);
	ifs_param2.read((char*)(temp3), 16 * sizeof(float));
	ifs_param2.close();
	for(int ci = 0; ci < 16; ci++)
	{
		bias_port[2][0].range(wt_lenth + ci*16, ci*16)=((bs_type)temp3[ci]).range(wt_lenth, 0);
	}
*/
/*
		printf("%f",temp[1]);
		wt_type tss;
		tss.range(wt_lenth, 0)=bias_port[0][0].range(wt_lenth +16 ,16);
		printf("%f",(float)(tss));
*/
}

void fold_w3_toport(uint512 w3[500][3][3])
{
	//310 layer : index 0,1,2
	float temp[6][6][3][3];
	std::ifstream ifs_param("F:/hls/thinker/code/test_data/modle_param/l2w.bin", std::ios::in | std::ios::binary);
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

void check_l1(){
	const char *f=l2;
	fold_data(check,f,imgsize_l2);
	for(int i=0;i<imgsize_l2;i++){
		if(ddr1[i]/8!=check[i]/8){
			cout<<"not match at "<<i<<'\n';
			cout<<"real:"<<bitset<16>(check[i])<<'\n';
			cout<<"ddr:"<<bitset<16>(ddr1[i])<<'\n';
			//return;
		}
	}
	//cout<<" Correct ";
}

int main()
{
	const char *img_path=imgpath;
	fold_data(IMG,img_path);
	fold_w3_toport(w3);
	fold_BS_toport(bias_port);
	fold_w1_toport(w1);
    Thinker(IMG ,w3,w1,bias_port,ddr1,debug);
    check_l1();
    return 0;
}
