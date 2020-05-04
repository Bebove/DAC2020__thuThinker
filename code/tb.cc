#include "dac.h"

uint16 IMG[imagesize];
uint16 ddr1[imagesize];
uint512 w3[500][3][3];
uint256 w1[500][16];
uint16 debug[2];
uint256     bias_port[500][5];
void fold_data(uint16 IMG[imagesize])
{
	int ip1;
	ip1=imagesize;
	float data;
	data=8/float(ip1);
	float temp;
	temp=-4;

	//fm_type *temp_img ;
	//temp_img = (fm_type *)malloc(3*(320+4)*(192+4)*4* sizeof(fm_type));
	int i;
	i=0;
	while(i<ip1)
	{
		IMG[i].range(8, 0)=((fm_type)temp).range(8, 0);
		temp=temp+data;
		i=i+1;
	}

}








//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void fold_w1_toport(uint256 w1[500][16])
{
	//w_port_1x1[500][16],
	//choose 1 index, get 16 uint256.
	//every uint256 is 16 weight.


	//307 layer : index 0
	float temp[6][3][1][1];
	std::ifstream ifs_param("C:\\Users\\f\\Desktop\\github\\DAC2020__thuThinker\\weightdata\\w1\\307.bin", std::ios::in | std::ios::binary);
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
	std::ifstream ifs_param("C:\\Users\\f\\Desktop\\github\\DAC2020__thuThinker\\weightdata\\w1\\307bs.bin", std::ios::in | std::ios::binary);
	ifs_param.read((char*)(temp), 6 * sizeof(float));
	ifs_param.close();
	for(int ci = 0; ci < 6; ci++)
	{
		bias_port[0][0].range(wt_lenth + ci*16, ci*16)=((bs_type)temp[ci]).range(wt_lenth, 0);
	}



	//310 layer : index 1
	float temp2[6];
	std::ifstream ifs_param1("C:\\Users\\f\\Desktop\\github\\DAC2020__thuThinker\\weightdata\\w1\\310bs.bin", std::ios::in | std::ios::binary);
	ifs_param1.read((char*)(temp2), 6 * sizeof(float));
	ifs_param1.close();
	for(int ci = 0; ci < 6; ci++)
	{
		bias_port[1][0].range(wt_lenth + ci*16, ci*16)=((bs_type)temp2[ci]).range(wt_lenth, 0);
	}



	//313 layer : index 2
	float temp3[16];
	std::ifstream ifs_param2("C:\\Users\\f\\Desktop\\github\\DAC2020__thuThinker\\weightdata\\w1\\313bs.bin", std::ios::in | std::ios::binary);
	ifs_param2.read((char*)(temp3), 16 * sizeof(float));
	ifs_param2.close();
	for(int ci = 0; ci < 16; ci++)
	{
		bias_port[2][0].range(wt_lenth + ci*16, ci*16)=((bs_type)temp3[ci]).range(wt_lenth, 0);
	}

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
	std::ifstream ifs_param("C:\\Users\\f\\Desktop\\github\\DAC2020__thuThinker\\weightdata\\w1\\310.bin", std::ios::in | std::ios::binary);
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

int main()
{

	fold_data(IMG);



	fold_w3_toport(w3);
	fold_BS_toport(bias_port);
	fold_w1_toport(w1);
    Thinker(	 IMG ,w3,w1,bias_port,ddr1,debug);
    return 0;
}
