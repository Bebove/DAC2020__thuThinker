#include "dac.h"
//#include <malloc.h>
uint16 IMG[imagesize];
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

void fold_w3(uint512 w3[500][3][3])
{
	int size;
	size=3*3*500*32;

	double temp;
	temp=-6;

	double data;
	data=12/(double)size;
	for(int m = 0; m < 3; m++)
	{
		for(int n = 0; n < 3; n++)
		{
			for(int co = 0; co < 500; co++)
			{
				for(int ci = 0; ci < 32; ci++)
				{
					w3[co][m][n].range(10 + ci*16, ci*16)=((wt_type)temp).range(10, 0);
					temp=data+temp;

				}

			}
		}
	}
}

void fold_w1(uint256 w1[500][16])
{
	int size;
	size=16*500*16;

	double temp;
	temp=-6;

	double data;
	data=12/(double)size;

	for(int n = 0; n < 16; n++)
	{
			for(int co = 0; co < 500; co++)
			{
				for(int ci = 0; ci < 16; ci++)
				{
					w1[co][n].range(10 + ci*16, ci*16)=((wt_type)temp).range(10, 0);
					temp=data+temp;

				}

			}
	}

}


void fold_BS(uint256 bias_port[500][5])
{
	int size;
	size=16*500*5;

	double temp;
	temp=-6;

	double data;
	data=12/(double)size;

	for(int n = 0; n < 5; n++)
	{
			for(int co = 0; co < 500; co++)
			{
				for(int ci = 0; ci < 16; ci++)
				{
					w1[co][n].range(10 + ci*16, ci*16)=((wt_type)temp).range(10, 0);
					temp=data+temp;

				}

			}
	}

}

int main()
{

	fold_data(IMG);
	fold_w3(w3);
	fold_BS(bias_port);
	//fold_w1(w1);

    SkyNet(	 IMG ,w3,w1,bias_port,debug);
    return 0;
}
