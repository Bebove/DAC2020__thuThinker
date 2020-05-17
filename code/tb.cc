#include "dac.h"
#include "tb.h"
#include <bitset>
#include <cmath>
// Those are the ports which should not be changed unless the Thinker IO is changed
uint16  IMG[imagesize];
uint256 ddrdebug [ddrsize][30];
uint256 ddrdebug_2 [ddrsize][30];
uint512 w3[500][3][3];
uint256 w1[500][16];
uint256 bias_port[500][5];
uint16  debug[2];


//Those are the vars which is not related to Thinker itself and related to debug
double temp_img[imagesize];
#define ddrsize_tb   249872//(320+2)*(192+2)*4;
double ddrimg[6*ddrsize_tb];//match the largest feature map
uint16 check[imagesize];//correct data
#define imgsize_l2 381024//6*(96+2)*2*(160+2)*2
#define ifcheck 0


//Those are functions to fold data
void fold_data(uint16 IMG[imagesize],const char *filepath,int fmsize=imagesize)
{

	int ip1;
	double error;
	ip1=fmsize;
	std::ifstream file(filepath,std::ios::in | std::ios::binary);
	file.read((char *)(&temp_img),fmsize*sizeof(double));
	file.close();
	int i=0;

	while(i<ip1)
	{
		fm_type temp_data=(fm_type)temp_img[i];
		IMG[i].range(fm_lenth, 0)=((fm_type)temp_img[i]).range(fm_lenth, 0);
		error+=fabs((temp_img[i]-(double)temp_data))/4;
		i=i+1;
	}
	error=error/ip1*100;
	cout<<"input image transfer error is "<<error<<"%\n";
}

void check_ddr(uint256 ddr [ddrsize][30],const char *filepath, int allch, int allh, int allw, int level){
	int fm_size=allh*allw;
	int ip1=allch*fm_size;
	double error=0;
	fm_type temp_data;

	std::ifstream file(filepath,std::ios::in | std::ios::binary);
	file.read((char *)(&ddrimg),ip1*sizeof(double));
	file.close();
	int i=0;

	// to find max value and min value in ddrimg, aim to normalized the error
	double max=0;
	double min=0;
	for(int i=0;i<ip1;i++){
		if(ddrimg[i]<min) min=ddrimg[i];
		if(ddrimg[i]>max) max=ddrimg[i];
	}
	//cout<<"max value "<<max<<"  min value "<<min<<" of level "<<level<<'\n';

	//compare ddrimg and compute result and give error value
	for(int i=0;i<fm_size;i++){
		int w=i%allw;
		int h=i/allw;
		for(int ch=0;ch<allch;ch++){
			int p=ch%16;
			int c=ch/16;
			//fm_type temp_data=(fm_type)ddrimg[ch*fm_size+h*allw+w];
			temp_data.range(fm_lenth,0)=ddr[i][c].range(16*p+fm_lenth,16*p);
			error+=fabs((ddrimg[ch*fm_size+h*allw+w]-(double)temp_data))/(max-min);//compute error
		}
	}
	error=error/ip1*100;
	cout<<"the error of level "<<level<<" is "<<error<<"%\n";

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
	float temp1[16][6][1][1];
	std::ifstream ifs_param1(w1_313, std::ios::in | std::ios::binary);
	ifs_param1.read((char*)(**temp1), 16 * 6 * 1 * sizeof(float));
	ifs_param1.close();
	for(int co = 0; co< 16; co++)
	{
		for(int ci = 0; ci < 6; ci++)
		{
			w1[1][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp1[co][ci][0][0]).range(wt_lenth, 0);
		}
	}
	float temp2[8][16][1][1];
	std::ifstream ifs_param2(w1_318, std::ios::in | std::ios::binary);
	ifs_param2.read((char*)(**temp2), 16 * 8 * 1 * sizeof(float));
	ifs_param2.close();
	for(int co = 0; co< 8; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			w1[2][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp2[co][ci][0][0]).range(wt_lenth, 0);
		}
	}

	float temp3[48][8][1][1];
	std::ifstream ifs_param3(w1_320, std::ios::in | std::ios::binary);
	ifs_param3.read((char*)(**temp3), 48 * 8 * 1 * sizeof(float));
	ifs_param3.close();
	for(int ic = 0; ic< 3;  ic++)
	{
		for(int co = 0; co< 16; co++)
		{
			for(int ci = 0; ci < 8; ci++)
			{
				w1[3+ic][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp3[co+16*ic][ci][0][0]).range(wt_lenth, 0);
			}
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

	//315 layer : index 3
	float temp4[16];
	std::ifstream ifs_param4(bs_315, std::ios::in | std::ios::binary);
	ifs_param4.read((char*)(temp4), 16 * sizeof(float));
	ifs_param4.close();
	for(int ci = 0; ci < 16; ci++)
	{
		bias_port[3][0].range(wt_lenth + ci*16, ci*16)=((bs_type)temp4[ci]).range(wt_lenth, 0);
	}
	//318 layer : index 4
	float temp5[8];
	std::ifstream ifs_param5(bs_318, std::ios::in | std::ios::binary);
	ifs_param5.read((char*)(temp5), 8 * sizeof(float));
	ifs_param5.close();
	for(int ci = 0; ci < 8; ci++)
	{
		bias_port[4][0].range(wt_lenth + ci*16, ci*16)=((bs_type)temp5[ci]).range(wt_lenth, 0);
	}
	//320 layer : index 5
	float temp6[48];
	std::ifstream ifs_param6(bs_320, std::ios::in | std::ios::binary);
	ifs_param6.read((char*)(temp6), 48 * sizeof(float));
	ifs_param6.close();
	for(int co = 0; co < 3; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[5][co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp6[ci+16*co]).range(wt_lenth, 0);
		}
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
	float temp1[16][16][3][3];
	std::ifstream ifs_param1(w3_315, std::ios::in | std::ios::binary);
	ifs_param1.read((char*)(**temp1), 16 * 16 * 9 * sizeof(float));
	ifs_param1.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int i=0;i<16;i++)
			{
				w3[1][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp1[0][i][x][y])).range(wt_lenth, 0);
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



int main()
{
	const char *img_path=imgpath;
	fold_data(IMG,img_path);

	fold_w3_toport(w3);
	fold_BS_toport(bias_port);
	fold_w1_toport(w1);

    Thinker(	 IMG ,w3,w1,bias_port,ddrdebug,ddrdebug_2,debug);
    check_ddr(ddrdebug,conv2,6,(96+2)*2,(160+2)*2,2);
    check_ddr(ddrdebug_2,conv5,8,(96+2)*2,(160+2)*2,5);
    return 0;
}
