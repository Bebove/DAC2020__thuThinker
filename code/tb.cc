#include "dac.h"
//#include <malloc.h>
uint16 IMG[imagesize];



void fold_data(uint16 IMG[imagesize])
{
	int ip1;
	ip1=imagesize;
	float data;
	data=4/float(ip1);
	float temp;
	temp=-2;

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

int main()
{

	fold_data(IMG);
    SkyNet(	 IMG );
    return 0;
}
