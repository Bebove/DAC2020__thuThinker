#include "dac.h"
#include "tb.h"
#include <bitset>
#include <cmath>


// Those are the ports which should not be changed unless the Thinker IO is changed
float  IMG[imagesize];
uint256 ddrdebug [ddrsize][ddrsize_dp];
uint256 ddrdebug_2 [ddrsize][ddrsize_dp];
uint256 w3[170][3][3];   //170
uint256 w1[1000][16];
uint256 bias_port[500];   //420
uint256		w_port_3x3_2[13*4][16][3][3];   //13*4
uint16  debug[2];
uint256 ddrdebug_3 [ddrsize][ddrsize_dp];
uint256 ddrdebug_4 [ddrsize][ddrsize_dp];
uint256 temp1 [ddrsize][ddrsize_dp];

//Those are the vars which is not related to Thinker itself and related to debug
double temp_img[imagesize];
#define ddrsize_tb   249872//(320+2)*(192+2)*4;
double ddrimg[48*ddrsize_tb];//match the largest feature map
uint16 check[imagesize];//correct data
#define imgsize_l2 381024//6*(96+2)*2*(160+2)*2
#define ifcheck 0











//Those are functions to fold data
void fold_data(float IMG[imagesize],const char *filepath,int fmsize=imagesize)
{

	int ip1;
	ip1=imagesize;
	std::ifstream file(filepath,std::ios::in | std::ios::binary);
	file.read((char *)(&temp_img),fmsize*sizeof(double));
	file.close();
	int i=0;

	while(i<ip1)
	{
		IMG[i]=temp_img[i];
		i=i+1;
	}

}

void fold_w1_toport(uint256 w1[500][16])
{
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

	float temp3[48][8][1][1];//index 3,4,5
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

	float temp4[16][48][1][1];//index 6,7,8
	std::ifstream ifs_param4(w1_326, std::ios::in | std::ios::binary);
	ifs_param4.read((char*)(**temp4), 48 * 16 * 1 * sizeof(float));
	ifs_param4.close();
	for(int ci = 0; ci < 16; ci++)
	{
		for(int ic = 0; ic< 3;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[6+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp4[ci][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}

	float temp5[96][16][1][1];//index 9,10,11,12,13,14
	std::ifstream ifs_param5(w1_328, std::ios::in | std::ios::binary);
	ifs_param5.read((char*)(**temp5), 96 * 16 * 1 * sizeof(float));
	ifs_param5.close();
	for(int ic = 0; ic< 6;  ic++)
	{
		for(int co = 0; co< 16; co++)
		{
			for(int ci = 0; ci < 16; ci++)
			{
				w1[9+ic][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp5[co+16*ic][ci][0][0]).range(wt_lenth, 0);
			}
		}
	}

	float temp6[16][96][1][1];//index 15,16,17,18,19,20
	std::ifstream ifs_param6(w1_334, std::ios::in | std::ios::binary);
	ifs_param6.read((char*)(**temp6), 16 * 96 * 1 * sizeof(float));
	ifs_param6.close();
	for(int ci = 0; ci < 16; ci++)
	{
		for(int ic = 0; ic< 6;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[15+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp6[ci][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}

	float temp7[96][16][1][1];//index 21-26
	std::ifstream ifs_param7(w1_337, std::ios::in | std::ios::binary);
	ifs_param7.read((char*)(**temp7), 96 * 16 * 1 * sizeof(float));
	ifs_param7.close();
	for(int ic = 0; ic< 6;  ic++)
	{
		for(int co = 0; co< 16; co++)
		{
			for(int ci = 0; ci < 16; ci++)
			{
				w1[21+ic][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp7[co+16*ic][ci][0][0]).range(wt_lenth, 0);
			}
		}
	}

	float temp8[16][96][1][1];//index 27-32
	std::ifstream ifs_param8(w1_343, std::ios::in | std::ios::binary);
	ifs_param8.read((char*)(**temp8), 16 * 96 * 1 * sizeof(float));
	ifs_param8.close();
	for(int ci = 0; ci < 16; ci++)
	{
		for(int ic = 0; ic< 6;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[27+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp8[ci][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}

	float temp9[96][16][1][1];//index 33-38
	std::ifstream ifs_param9(w1_345, std::ios::in | std::ios::binary);
	ifs_param9.read((char*)(**temp9), 96 * 16 * 1 * sizeof(float));
	ifs_param9.close();
	for(int ic = 0; ic< 6;  ic++)
	{
		for(int co = 0; co< 16; co++)
		{
			for(int ci = 0; ci < 16; ci++)
			{
				w1[33+ic][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp9[co+16*ic][ci][0][0]).range(wt_lenth, 0);
			}
		}
	}

	float temp10[16][96][1][1];//index 39-44
	std::ifstream ifs_param10(w1_351, std::ios::in | std::ios::binary);
	ifs_param10.read((char*)(**temp10), 16 * 96 * 1 * sizeof(float));
	ifs_param10.close();
	for(int ci = 0; ci < 16; ci++)
	{
		for(int ic = 0; ic< 6;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[39+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp10[ci][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}

	//float temp7[96][16][1][1];//index 45-50
	std::ifstream ifs_param11(w1_354, std::ios::in | std::ios::binary);
	ifs_param11.read((char*)(**temp7), 96 * 16 * 1 * sizeof(float));
	ifs_param11.close();
	for(int ic = 0; ic< 6;  ic++)
	{
		for(int co = 0; co< 16; co++)
		{
			for(int ci = 0; ci < 16; ci++)
			{
				w1[45+ic][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp7[co+16*ic][ci][0][0]).range(wt_lenth, 0);
			}
		}
	}

	//float temp8[16][96][1][1];//index 51-56
	std::ifstream ifs_param12(w1_360, std::ios::in | std::ios::binary);
	ifs_param12.read((char*)(**temp8), 16 * 96 * 1 * sizeof(float));
	ifs_param12.close();
	for(int ci = 0; ci < 16; ci++)
	{
		for(int ic = 0; ic< 6;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[51+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp8[ci][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}
	//float temp7[96][16][1][1];//index 57-62
	std::ifstream ifs_param13(w1_363, std::ios::in | std::ios::binary);
	ifs_param13.read((char*)(**temp7), 96 * 16 * 1 * sizeof(float));
	ifs_param13.close();
	for(int ic = 0; ic< 6;  ic++)
	{
		for(int co = 0; co< 16; co++)
		{
			for(int ci = 0; ci < 16; ci++)
			{
				w1[57+ic][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp7[co+16*ic][ci][0][0]).range(wt_lenth, 0);
			}
		}
	}
	float temp12[32][96][1][1];//index 63-68  and  69-74
	std::ifstream ifs_param14(w1_369, std::ios::in | std::ios::binary);
	ifs_param14.read((char*)(**temp12), 32 * 96 * 1 * sizeof(float));
	ifs_param14.close();
		for(int ci = 0; ci < 16; ci++)
		{
			for(int ic = 0; ic< 6;  ic++)
			{
				for(int co = 0; co< 16; co++)
				{

					w1[63+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp12[ci][co+16*ic][0][0]).range(wt_lenth, 0);
				}
			}
		}
		for(int ci = 0; ci < 16; ci++)
		{
			for(int ic = 0; ic< 6;  ic++)
			{
				for(int co = 0; co< 16; co++)
				{

					w1[69+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp12[ci+16][co+16*ic][0][0]).range(wt_lenth, 0);
				}
			}
		}

	float temp13[192][32][1][1];//index 75-86  and   87-98
	std::ifstream ifs_param15(w1_371, std::ios::in | std::ios::binary);
	ifs_param15.read((char*)(**temp13), 192 * 32 * 1 * sizeof(float));
	ifs_param15.close();
		for(int ic = 0; ic< 12;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{
				for(int ci = 0; ci < 16; ci++)
				{
					w1[75+ic][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp13[co+16*ic][ci][0][0]).range(wt_lenth, 0);
				}
			}
		}
		for(int ic = 0; ic< 12;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{
				for(int ci = 0; ci < 16; ci++)
				{
					w1[87+ic][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp13[co+16*ic][ci+16][0][0]).range(wt_lenth, 0);
				}
			}
		}

	float temp14[32][192][1][1];//index 99-110  and  111-122
	std::ifstream ifs_param16(w1_377, std::ios::in | std::ios::binary);
	ifs_param16.read((char*)(**temp14), 32 * 192 * 1 * sizeof(float));
	ifs_param16.close();
		for(int ci = 0; ci < 16; ci++)
		{
			for(int ic = 0; ic< 12;  ic++)
			{
				for(int co = 0; co< 16; co++)
				{

					w1[99+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp14[ci][co+16*ic][0][0]).range(wt_lenth, 0);
				}
			}
		}
		for(int ci = 0; ci < 16; ci++)
		{
			for(int ic = 0; ic< 12;  ic++)
			{
				for(int co = 0; co< 16; co++)
				{

					w1[111+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp14[ci+16][co+16*ic][0][0]).range(wt_lenth, 0);
				}
			}
		}

	//float temp13[192][32][1][1];//index 123-134  and   135-146
	std::ifstream ifs_param17(w1_380, std::ios::in | std::ios::binary);
	ifs_param17.read((char*)(**temp13), 192 * 32 * 1 * sizeof(float));
	ifs_param17.close();
		for(int ic = 0; ic< 12;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{
				for(int ci = 0; ci < 16; ci++)
				{
					w1[123+ic][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp13[co+16*ic][ci][0][0]).range(wt_lenth, 0);
				}
			}
		}
		for(int ic = 0; ic< 12;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{
				for(int ci = 0; ci < 16; ci++)
				{
					w1[135+ic][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp13[co+16*ic][ci+16][0][0]).range(wt_lenth, 0);
				}
			}
		}


	//float temp14[32][192][1][1];//index 147-158  and  159-170
	std::ifstream ifs_param18(w1_386, std::ios::in | std::ios::binary);
	ifs_param18.read((char*)(**temp14), 32 * 192 * 1 * sizeof(float));
	ifs_param18.close();
		for(int ci = 0; ci < 16; ci++)
		{
			for(int ic = 0; ic< 12;  ic++)
			{
				for(int co = 0; co< 16; co++)
				{

					w1[147+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp14[ci][co+16*ic][0][0]).range(wt_lenth, 0);
				}
			}
		}
		for(int ci = 0; ci < 16; ci++)
		{
			for(int ic = 0; ic< 12;  ic++)
			{
				for(int co = 0; co< 16; co++)
				{

					w1[159+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp14[ci+16][co+16*ic][0][0]).range(wt_lenth, 0);
				}
			}
		}

	//float temp13[192][32][1][1];//index 171-182  and   183-194
	std::ifstream ifs_param19(w1_389, std::ios::in | std::ios::binary);
	ifs_param19.read((char*)(**temp13), 192 * 32 * 1 * sizeof(float));
	ifs_param19.close();
		for(int ic = 0; ic< 12;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{
				for(int ci = 0; ci < 16; ci++)
				{
					w1[171+ic][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp13[co+16*ic][ci][0][0]).range(wt_lenth, 0);
				}
			}
		}
		for(int ic = 0; ic< 12;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{
				for(int ci = 0; ci < 16; ci++)
				{
					w1[183+ic][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp13[co+16*ic][ci+16][0][0]).range(wt_lenth, 0);
				}
			}
		}

	//float temp14[32][192][1][1];//index 195-206  and  207-218
	std::ifstream ifs_param20(w1_395, std::ios::in | std::ios::binary);
	ifs_param20.read((char*)(**temp14), 32 * 192 * 1 * sizeof(float));
	ifs_param20.close();
		for(int ci = 0; ci < 16; ci++)
		{
			for(int ic = 0; ic< 12;  ic++)
			{
				for(int co = 0; co< 16; co++)
				{

					w1[195+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp14[ci][co+16*ic][0][0]).range(wt_lenth, 0);
				}
			}
		}
		for(int ci = 0; ci < 16; ci++)
		{
			for(int ic = 0; ic< 12;  ic++)
			{
				for(int co = 0; co< 16; co++)
				{

					w1[207+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp14[ci+16][co+16*ic][0][0]).range(wt_lenth, 0);
				}
			}
		}

	//float temp13[192][32][1][1];//index 219-230  and  231-242
	std::ifstream ifs_param21(w1_398, std::ios::in | std::ios::binary);
	ifs_param21.read((char*)(**temp13), 192 * 32 * 1 * sizeof(float));
	ifs_param21.close();
	int ic_max=12;
	for(int p=0;p<2;p++)
	{
		for(int ic = 0; ic< ic_max;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{
				for(int ci = 0; ci < 16; ci++)
				{
					w1[219+ic+p*ic_max][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp13[co+16*ic][ci+16*p][0][0]).range(wt_lenth, 0);
				}
			}
		}
	}

	float temp17[48][192][1][1];//index 243-254  and  255-266 and 267-278
	std::ifstream ifs_param22(w1_404, std::ios::in | std::ios::binary);
	ifs_param22.read((char*)(**temp17), 48 * 192 * 1 * sizeof(float));
	ifs_param22.close();
	ic_max=12;
	for(int p=0;p<3;p++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			for(int ic = 0; ic< ic_max;  ic++)
			{
				for(int co = 0; co< 16; co++)
				{

					w1[243+ic+p*ic_max][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp17[ci+16*p][co+16*ic][0][0]).range(wt_lenth, 0);
				}
			}
		}
	}

	float temp15[288][48][1][1];//index 279-296  and  297-314 315-332
	std::ifstream ifs_param23(w1_406, std::ios::in | std::ios::binary);
	ifs_param23.read((char*)(**temp15), 288 * 48 * 1 * sizeof(float));
	ifs_param23.close();
	ic_max=18;
	for(int p=0;p<3;p++)
	{
		for(int ic = 0; ic< ic_max;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{
				for(int ci = 0; ci < 16; ci++)
				{
					w1[279+ic+p*ic_max][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp15[co+16*ic][ci+16*p][0][0]).range(wt_lenth, 0);
				}
			}
		}
	}

	float temp16[48][288][1][1];//index 333-350 351-368 369-386
	std::ifstream ifs_param24(w1_412, std::ios::in | std::ios::binary);
	ifs_param24.read((char*)(**temp16), 48 * 288 * 1 * sizeof(float));
	ifs_param24.close();
	ic_max=18;
	for(int p=0;p<3;p++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			for(int ic = 0; ic< ic_max;  ic++)
			{
				for(int co = 0; co< 16; co++)
				{

					w1[333+ic+p*ic_max][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp16[ci+16*p][co+16*ic][0][0]).range(wt_lenth, 0);
				}
			}
		}
	}

	//float temp15[288][48][1][1];//index 387-404  and  405-422 423-440
	std::ifstream ifs_param25(w1_415, std::ios::in | std::ios::binary);
	ifs_param25.read((char*)(**temp15), 288 * 48 * 1 * sizeof(float));
	ifs_param25.close();
	ic_max=18;
	for(int p=0;p<3;p++)
	{
		for(int ic = 0; ic< ic_max;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{
				for(int ci = 0; ci < 16; ci++)
				{
					w1[387+ic+p*ic_max][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp15[co+16*ic][ci+16*p][0][0]).range(wt_lenth, 0);
				}
			}
		}
	}

	//float temp16[48][288][1][1];//index 441-458 459-476 477-494
	std::ifstream ifs_param26(w1_421, std::ios::in | std::ios::binary);
	ifs_param26.read((char*)(**temp16), 48 * 288 * 1 * sizeof(float));
	ifs_param26.close();
	ic_max=18;
	for(int p=0;p<3;p++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			for(int ic = 0; ic< ic_max;  ic++)
			{
				for(int co = 0; co< 16; co++)
				{

					w1[441+ic+p*ic_max][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp16[ci+16*p][co+16*ic][0][0]).range(wt_lenth, 0);
				}
			}
		}
	}

	//float temp15[288][48][1][1];//index 495-512  and  513-530 531-548
	std::ifstream ifs_param27(w1_424, std::ios::in | std::ios::binary);
	ifs_param27.read((char*)(**temp15), 288 * 48 * 1 * sizeof(float));
	ifs_param27.close();
	ic_max=18;
	for(int p=0;p<3;p++)
	{
		for(int ic = 0; ic< ic_max;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{
				for(int ci = 0; ci < 16; ci++)
				{
					w1[495+ic+p*ic_max][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp15[co+16*ic][ci+16*p][0][0]).range(wt_lenth, 0);
				}
			}
		}
	}

	float temp18[80][288][1][1];//index 549-566 567-584 585-602 603-620 621-638
	std::ifstream ifs_param28(w1_430, std::ios::in | std::ios::binary);
	ifs_param28.read((char*)(**temp18), 80 * 288 * 1 * sizeof(float));
	ifs_param28.close();
	ic_max=18;
	for(int p=0;p<5;p++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			for(int ic = 0; ic< ic_max;  ic++)
			{
				for(int co = 0; co< 16; co++)
				{

					w1[549+ic+p*ic_max][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp18[ci+16*p][co+16*ic][0][0]).range(wt_lenth, 0);
				}
			}
		}
	}

	float temp19[480][80][1][1];//index 639-668  and  669-698 699-728 729-758 759-788
	std::ifstream ifs_param29(w1_432, std::ios::in | std::ios::binary);
	ifs_param29.read((char*)(**temp19), 480 * 80 * 1 * sizeof(float));
	ifs_param29.close();
	ic_max=30;
	for(int p=0;p<5;p++)
	{
		for(int ic = 0; ic< ic_max;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{
				for(int ci = 0; ci < 16; ci++)
				{
					w1[639+ic+p*ic_max][co].range(wt_lenth + ci*16, ci*16)=((wt_type)temp19[co+16*ic][ci+16*p][0][0]).range(wt_lenth, 0);
				}
			}
		}
	}

	float temp20[80][480][1][1];//index 789-818 819-848 849-878 879-908 909-938
	std::ifstream ifs_param30(w1_438, std::ios::in | std::ios::binary);
	ifs_param30.read((char*)(**temp20), 80 * 480 * 1 * sizeof(float));
	ifs_param30.close();
	ic_max=30;
	for(int p=0;p<5;p++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			for(int ic = 0; ic< ic_max;  ic++)
			{
				for(int co = 0; co< 16; co++)
				{

					w1[789+ic+p*ic_max][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp20[ci+16*p][co+16*ic][0][0]).range(wt_lenth, 0);
				}
			}
		}
	}

	float temp21[32][80][1][1];//index 939-943 944-948
	std::ifstream ifs_param31(w1_444, std::ios::in | std::ios::binary);
	ifs_param31.read((char*)(**temp21), 32 * 80* 1 * sizeof(float));
	ifs_param31.close();
	ic_max=5;
	for(int p=0;p<2;p++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			for(int ic = 0; ic< ic_max;  ic++)
			{
				for(int co = 0; co< 16; co++)
				{

					w1[939+ic+p*ic_max][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp21[ci+16*p][co+16*ic][0][0]).range(wt_lenth, 0);
				}
			}
		}
	}
	float temp22[6][32][1][1];//index 949 950
	std::ifstream ifs_param40(w1_525, std::ios::in | std::ios::binary);
	ifs_param40.read((char*)(**temp22), 32 * 6 * 1 * sizeof(float));
	ifs_param40.close();
	for(int ci = 0; ci < 6; ci++)
	{
		for(int ic = 0; ic< 2;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[949+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp22[ci][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}
	float temp23[24][32][1][1];//index 951 952   953 954
	std::ifstream ifs_param41(w1_526, std::ios::in | std::ios::binary);
	ifs_param41.read((char*)(**temp23), 32 * 24 * 1 * sizeof(float));
	ifs_param41.close();
	for(int ci = 0; ci < 16; ci++)
	{
		for(int ic = 0; ic< 2;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[951+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp23[ci][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}
	for(int ci = 0; ci < 8; ci++)
	{
		for(int ic = 0; ic< 2;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[953+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp23[ci+16][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}
	float temp25[32][48][1][1];//index 955 956 957    958 959 960
	std::ifstream ifs_param42(w1_443, std::ios::in | std::ios::binary);
	ifs_param42.read((char*)(**temp25), 32 * 48 * 1 * sizeof(float));
	ifs_param42.close();
	for(int ci = 0; ci < 16; ci++)
	{
		for(int ic = 0; ic< 3;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[955+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp25[ci][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}
	for(int ci = 0; ci < 16; ci++)
	{
		for(int ic = 0; ic< 3;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[958+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp25[ci+16][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}
	//float temp22[6][32][1][1];//index 962 963
	std::ifstream ifs_param43(w1_519, std::ios::in | std::ios::binary);
	ifs_param43.read((char*)(**temp22), 32 * 6 * 1 * sizeof(float));
	ifs_param43.close();
	for(int ci = 0; ci < 6; ci++)
	{
		for(int ic = 0; ic< 2;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[962+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp22[ci][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}
	//float temp23[24][32][1][1];//index 964 965   966 967
	std::ifstream ifs_param44(w1_520, std::ios::in | std::ios::binary);
	ifs_param44.read((char*)(**temp23), 32 * 24 * 1 * sizeof(float));
	ifs_param44.close();
	for(int ci = 0; ci < 16; ci++)
	{
		for(int ic = 0; ic< 2;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[964+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp23[ci][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}
	for(int ci = 0; ci < 8; ci++)
	{
		for(int ic = 0; ic< 2;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[966+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp23[ci+16][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}
	float temp221[32][16][1][1];//index 968 969
	std::ifstream ifs_param45(w1_442, std::ios::in | std::ios::binary);
	ifs_param45.read((char*)(**temp221), 32 * 16 * 1 * sizeof(float));
	ifs_param45.close();
	for(int ci = 0; ci < 16; ci++)
	{

			for(int co = 0; co< 16; co++)
			{

				w1[968][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp221[ci][co][0][0]).range(wt_lenth, 0);
			}

	}
	for(int ci = 0; ci < 16; ci++)
	{

			for(int co = 0; co< 16; co++)
			{

				w1[969][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp221[ci+16][co][0][0]).range(wt_lenth, 0);
			}

	}
	//float temp22[6][32][1][1];//index 971 972
	std::ifstream ifs_param47(w1_513, std::ios::in | std::ios::binary);
	ifs_param47.read((char*)(**temp22), 32 * 6 * 1 * sizeof(float));
	ifs_param47.close();
	for(int ci = 0; ci < 6; ci++)
	{
		for(int ic = 0; ic< 2;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[971+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp22[ci][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}
	//float temp23[24][32][1][1];//index 973 974   975 976
	std::ifstream ifs_param48(w1_514, std::ios::in | std::ios::binary);
	ifs_param48.read((char*)(**temp23), 32 * 24 * 1 * sizeof(float));
	ifs_param48.close();
	for(int ci = 0; ci < 16; ci++)
	{
		for(int ic = 0; ic< 2;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[973+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp23[ci][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}
	for(int ci = 0; ci < 8; ci++)
	{
		for(int ic = 0; ic< 2;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[975+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp23[ci+16][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}
	//float temp221[32][16][1][1];//index 978 979
	std::ifstream ifs_param451(w1_441, std::ios::in | std::ios::binary);
	ifs_param451.read((char*)(**temp221), 32 * 16 * 1 * sizeof(float));
	ifs_param451.close();
	for(int ci = 0; ci < 16; ci++)
	{

			for(int co = 0; co< 16; co++)
			{

				w1[978][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp221[ci][co][0][0]).range(wt_lenth, 0);
			}

	}
	for(int ci = 0; ci < 16; ci++)
	{

			for(int co = 0; co< 16; co++)
			{

				w1[979][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp221[ci+16][co][0][0]).range(wt_lenth, 0);
			}

	}
	//float temp22[6][32][1][1];//index 981 982
	std::ifstream ifs_param452(w1_507, std::ios::in | std::ios::binary);
	ifs_param452.read((char*)(**temp22), 32 * 6 * 1 * sizeof(float));
	ifs_param452.close();
	for(int ci = 0; ci < 6; ci++)
	{
		for(int ic = 0; ic< 2;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[981+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp22[ci][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}
	//float temp23[24][32][1][1];//index 983 974   975 976
	std::ifstream ifs_param148(w1_508, std::ios::in | std::ios::binary);
	ifs_param148.read((char*)(**temp23), 32 * 24 * 1 * sizeof(float));
	ifs_param148.close();
	for(int ci = 0; ci < 16; ci++)
	{
		for(int ic = 0; ic< 2;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[983+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp23[ci][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}
	for(int ci = 0; ci < 8; ci++)
	{
		for(int ic = 0; ic< 2;  ic++)
		{
			for(int co = 0; co< 16; co++)
			{

				w1[985+ic][ci].range(wt_lenth + co*16, co*16)=((wt_type)temp23[ci+16][co+16*ic][0][0]).range(wt_lenth, 0);
			}
		}
	}
}

void fold_BS_toport(uint256 bias_port[500])
{
	//307 layer : index 0
	float temp[6];
	std::ifstream ifs_param(bs_307, std::ios::in | std::ios::binary);
	ifs_param.read((char*)(temp), 6 * sizeof(float));
	ifs_param.close();
	for(int ci = 0; ci < 6; ci++)
	{
		bias_port[0].range(wt_lenth + ci*16, ci*16)=((bs_type)temp[ci]).range(wt_lenth, 0);
	}



	//310 layer : index 1
	float temp2[6];
	std::ifstream ifs_param1(bs_310, std::ios::in | std::ios::binary);
	ifs_param1.read((char*)(temp2), 6 * sizeof(float));
	ifs_param1.close();
	for(int ci = 0; ci < 6; ci++)
	{
		bias_port[1].range(wt_lenth + ci*16, ci*16)=((bs_type)temp2[ci]).range(wt_lenth, 0);
	}



	//313 layer : index 2
	float temp3[16];
	std::ifstream ifs_param2(bs_313, std::ios::in | std::ios::binary);
	ifs_param2.read((char*)(temp3), 16 * sizeof(float));
	ifs_param2.close();
	for(int ci = 0; ci < 16; ci++)
	{
		bias_port[2].range(wt_lenth + ci*16, ci*16)=((bs_type)temp3[ci]).range(wt_lenth, 0);
	}

	//315 layer : index 3
	float temp4[16];
	std::ifstream ifs_param4(bs_315, std::ios::in | std::ios::binary);
	ifs_param4.read((char*)(temp4), 16 * sizeof(float));
	ifs_param4.close();
	for(int ci = 0; ci < 16; ci++)
	{
		bias_port[3].range(wt_lenth + ci*16, ci*16)=((bs_type)temp4[ci]).range(wt_lenth, 0);
	}
	//318 layer : index 4
	float temp5[8];
	std::ifstream ifs_param5(bs_318, std::ios::in | std::ios::binary);
	ifs_param5.read((char*)(temp5), 8 * sizeof(float));
	ifs_param5.close();
	for(int ci = 0; ci < 8; ci++)
	{
		bias_port[4].range(wt_lenth + ci*16, ci*16)=((bs_type)temp5[ci]).range(wt_lenth, 0);
	}

	//320 layer : index 5,6,7
	float temp6[48];
	std::ifstream ifs_param6(bs_320, std::ios::in | std::ios::binary);
	ifs_param6.read((char*)(temp6), 48 * sizeof(float));
	ifs_param6.close();
	for(int co = 0; co < 3; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[5+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp6[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//323 layer : index 8,9,10
	float temp7[48];
	std::ifstream ifs_param7(bs_323, std::ios::in | std::ios::binary);
	ifs_param7.read((char*)(temp7), 48 * sizeof(float));
	ifs_param7.close();
	for(int co = 0; co < 3; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[8+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp7[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//318 layer : index 11
	float temp8[16];
	std::ifstream ifs_param8(bs_326, std::ios::in | std::ios::binary);
	ifs_param8.read((char*)(temp8), 16 * sizeof(float));
	ifs_param8.close();
	for(int ci = 0; ci < 16; ci++)
	{
		bias_port[11].range(wt_lenth + ci*16, ci*16)=((bs_type)temp8[ci]).range(wt_lenth, 0);
	}

	//328 layer : index 12-17
	float temp9[96];
	std::ifstream ifs_param9(bs_328, std::ios::in | std::ios::binary);
	ifs_param9.read((char*)(temp9), 96 * sizeof(float));
	ifs_param9.close();
	for(int co = 0; co < 6; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[12+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp9[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//331 layer : index 18-23
	float temp10[96];
	std::ifstream ifs_param10(bs_331, std::ios::in | std::ios::binary);
	ifs_param10.read((char*)(temp10), 96 * sizeof(float));
	ifs_param10.close();
	for(int co = 0; co < 6; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[18+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp10[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//334 layer : index 24
	float temp11[16];
	std::ifstream ifs_param11(bs_334, std::ios::in | std::ios::binary);
	ifs_param11.read((char*)(temp11), 16 * sizeof(float));
	ifs_param11.close();
	for(int ci = 0; ci < 16; ci++)
	{
		bias_port[24].range(wt_lenth + ci*16, ci*16)=((bs_type)temp11[ci]).range(wt_lenth, 0);
	}

	//337 layer : index 25-30
	float temp12[96];
	std::ifstream ifs_param12(bs_337, std::ios::in | std::ios::binary);
	ifs_param12.read((char*)(temp12), 96 * sizeof(float));
	ifs_param12.close();
	for(int co = 0; co < 6; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[25+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp12[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//340 layer : index 31-36
	float temp13[96];
	std::ifstream ifs_param13(bs_340, std::ios::in | std::ios::binary);
	ifs_param13.read((char*)(temp13), 96 * sizeof(float));
	ifs_param13.close();
	for(int co = 0; co < 6; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[31+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp13[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//343 layer : index 37
	float temp14[16];
	std::ifstream ifs_param14(bs_343, std::ios::in | std::ios::binary);
	ifs_param14.read((char*)(temp14), 16 * sizeof(float));
	ifs_param14.close();
	for(int ci = 0; ci < 16; ci++)
	{
		bias_port[37].range(wt_lenth + ci*16, ci*16)=((bs_type)temp14[ci]).range(wt_lenth, 0);
	}

	//345 layer : index 38-43
	float temp15[96];
	std::ifstream ifs_param15(bs_345, std::ios::in | std::ios::binary);
	ifs_param15.read((char*)(temp15), 96 * sizeof(float));
	ifs_param15.close();
	for(int co = 0; co < 6; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[38+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp15[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//348 layer : index 44-49
	float temp16[96];
	std::ifstream ifs_param16(bs_348, std::ios::in | std::ios::binary);
	ifs_param16.read((char*)(temp16), 96 * sizeof(float));
	ifs_param16.close();
	for(int co = 0; co < 6; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[44+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp16[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//351 layer : index 50
	float temp17[16];
	std::ifstream ifs_param17(bs_351, std::ios::in | std::ios::binary);
	ifs_param17.read((char*)(temp17), 16 * sizeof(float));
	ifs_param17.close();
	for(int ci = 0; ci < 16; ci++)
	{
		bias_port[50].range(wt_lenth + ci*16, ci*16)=((bs_type)temp17[ci]).range(wt_lenth, 0);
	}

	//354 layer : index 51-56
	//float temp12[96];
	std::ifstream ifs_param18(bs_354, std::ios::in | std::ios::binary);
	ifs_param18.read((char*)(temp12), 96 * sizeof(float));
	ifs_param18.close();
	for(int co = 0; co < 6; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[51+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp12[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//357 layer : index 57-62
	//float temp13[96];
	std::ifstream ifs_param19(bs_357, std::ios::in | std::ios::binary);
	ifs_param19.read((char*)(temp13), 96 * sizeof(float));
	ifs_param19.close();
	for(int co = 0; co < 6; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[57+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp13[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//360 layer : index 63
	//float temp14[16];
	std::ifstream ifs_param20(bs_360, std::ios::in | std::ios::binary);
	ifs_param20.read((char*)(temp14), 16 * sizeof(float));
	ifs_param20.close();
	for(int ci = 0; ci < 16; ci++)
	{
		bias_port[63].range(wt_lenth + ci*16, ci*16)=((bs_type)temp14[ci]).range(wt_lenth, 0);
	}
	//363 layer : index 64-69
	//float temp12[96];
	std::ifstream ifs_param21(bs_363, std::ios::in | std::ios::binary);
	ifs_param21.read((char*)(temp12), 96 * sizeof(float));
	ifs_param21.close();
	for(int co = 0; co < 6; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[64+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp12[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//366 layer : index 70-75
	//float temp13[96];
	std::ifstream ifs_param22(bs_366, std::ios::in | std::ios::binary);
	ifs_param22.read((char*)(temp13), 96 * sizeof(float));
	ifs_param22.close();
	for(int co = 0; co < 6; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[70+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp13[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//369 layer : index 76-77
	float temp20[32];
	std::ifstream ifs_param23(bs_369, std::ios::in | std::ios::binary);
	ifs_param23.read((char*)(temp20), 32 * sizeof(float));
	ifs_param23.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[76+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//371 layer : index 78-89
	float temp21[192];
	std::ifstream ifs_param24(bs_371, std::ios::in | std::ios::binary);
	ifs_param24.read((char*)(temp21), 192 * sizeof(float));
	ifs_param24.close();
	for(int co = 0; co < 12; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[78+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp21[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//374 layer : index 90-101
	//float temp21[192];
	std::ifstream ifs_param25(bs_374, std::ios::in | std::ios::binary);
	ifs_param25.read((char*)(temp21), 192 * sizeof(float));
	ifs_param25.close();
	for(int co = 0; co < 12; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[90+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp21[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//377 layer : index 102 103
	//float temp20[32];
	std::ifstream ifs_param26(bs_377, std::ios::in | std::ios::binary);
	ifs_param26.read((char*)(temp20), 32 * sizeof(float));
	ifs_param26.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[102+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//380 layer : index 104-115
	//float temp21[192];
	std::ifstream ifs_param27(bs_380, std::ios::in | std::ios::binary);
	ifs_param27.read((char*)(temp21), 192 * sizeof(float));
	ifs_param27.close();
	for(int co = 0; co < 12; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[104+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp21[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//383 layer : index 116-127
	//float temp21[192];
	std::ifstream ifs_param28(bs_383, std::ios::in | std::ios::binary);
	ifs_param28.read((char*)(temp21), 192 * sizeof(float));
	ifs_param28.close();
	for(int co = 0; co < 12; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[116+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp21[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//386 layer : index 128 129
	//float temp20[32];
	std::ifstream ifs_param29(bs_386, std::ios::in | std::ios::binary);
	ifs_param29.read((char*)(temp20), 32 * sizeof(float));
	ifs_param29.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[128+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//389 layer : index 130-141
	//float temp21[192];
	std::ifstream ifs_param30(bs_389, std::ios::in | std::ios::binary);
	ifs_param30.read((char*)(temp21), 192 * sizeof(float));
	ifs_param30.close();
	for(int co = 0; co < 12; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[130+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp21[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//392 layer : index 142-153
	//float temp21[192];
	std::ifstream ifs_param31(bs_392, std::ios::in | std::ios::binary);
	ifs_param31.read((char*)(temp21), 192 * sizeof(float));
	ifs_param31.close();
	for(int co = 0; co < 12; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[142+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp21[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//395 layer : index 154 155
	//float temp20[32];
	std::ifstream ifs_param32(bs_395, std::ios::in | std::ios::binary);
	ifs_param32.read((char*)(temp20), 32 * sizeof(float));
	ifs_param32.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[154+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//398 layer : index 156-167
	//float temp21[192];
	std::ifstream ifs_param33(bs_398, std::ios::in | std::ios::binary);
	ifs_param33.read((char*)(temp21), 192 * sizeof(float));
	ifs_param33.close();
	for(int co = 0; co < 12; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[156+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp21[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//401 layer : index 168-179
	//float temp21[192];
	std::ifstream ifs_param34(bs_401, std::ios::in | std::ios::binary);
	ifs_param34.read((char*)(temp21), 192 * sizeof(float));
	ifs_param34.close();
	for(int co = 0; co < 12; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[168+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp21[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//404 layer : index 180-182
	float temp22[48];
	std::ifstream ifs_param35(bs_404, std::ios::in | std::ios::binary);
	ifs_param35.read((char*)(temp22), 48 * sizeof(float));
	ifs_param35.close();
	for(int co = 0; co < 3; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[180+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp22[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//406 layer : index 183-200
	float temp23[288];
	std::ifstream ifs_param36(bs_406, std::ios::in | std::ios::binary);
	ifs_param36.read((char*)(temp23), 288 * sizeof(float));
	ifs_param36.close();
	for(int co = 0; co < 18; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[183+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp23[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//409 layer : index 201-218
	//float temp23[288];
	std::ifstream ifs_param37(bs_409, std::ios::in | std::ios::binary);
	ifs_param37.read((char*)(temp23), 288 * sizeof(float));
	ifs_param37.close();
	for(int co = 0; co < 18; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[201+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp23[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//412 layer : index 219-221
	//float temp22[48];
	std::ifstream ifs_param38(bs_412, std::ios::in | std::ios::binary);
	ifs_param38.read((char*)(temp22), 48 * sizeof(float));
	ifs_param38.close();
	for(int co = 0; co < 3; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[219+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp22[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//415 layer : index 222-239
	//float temp23[288];
	std::ifstream ifs_param39(bs_415, std::ios::in | std::ios::binary);
	ifs_param39.read((char*)(temp23), 288 * sizeof(float));
	ifs_param39.close();
	for(int co = 0; co < 18; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[222+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp23[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//418 layer : index 240-257
	//float temp23[288];
	std::ifstream ifs_param40(bs_418, std::ios::in | std::ios::binary);
	ifs_param40.read((char*)(temp23), 288 * sizeof(float));
	ifs_param40.close();
	for(int co = 0; co < 18; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[240+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp23[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//421 layer : index 258-260
	//float temp22[48];
	std::ifstream ifs_param41(bs_421, std::ios::in | std::ios::binary);
	ifs_param41.read((char*)(temp22), 48 * sizeof(float));
	ifs_param41.close();
	for(int co = 0; co < 3; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[258+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp22[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//424 layer : index 261-278
	//float temp23[288];
	std::ifstream ifs_param42(bs_424, std::ios::in | std::ios::binary);
	ifs_param42.read((char*)(temp23), 288 * sizeof(float));
	ifs_param42.close();
	for(int co = 0; co < 18; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[261+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp23[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//427 layer : index 279-296
	//float temp23[288];
	std::ifstream ifs_param43(bs_427, std::ios::in | std::ios::binary);
	ifs_param43.read((char*)(temp23), 288 * sizeof(float));
	ifs_param43.close();
	for(int co = 0; co < 18; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[279+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp23[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//430 layer : index 297-301
	float temp24[80];
	std::ifstream ifs_param44(bs_430, std::ios::in | std::ios::binary);
	ifs_param44.read((char*)(temp24), 80 * sizeof(float));
	ifs_param44.close();
	for(int co = 0; co < 5; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[297+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp24[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//432 layer : index 302-331
	float temp25[480];
	std::ifstream ifs_param45(bs_432, std::ios::in | std::ios::binary);
	ifs_param45.read((char*)(temp25), 480 * sizeof(float));
	ifs_param45.close();
	for(int co = 0; co < 30; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[302+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp25[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//435 layer : index 332-361
	//float temp25[480];
	std::ifstream ifs_param46(bs_435, std::ios::in | std::ios::binary);
	ifs_param46.read((char*)(temp25), 480 * sizeof(float));
	ifs_param46.close();
	for(int co = 0; co < 30; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[332+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp25[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//438 layer : index 362-366
	//float temp24[80];
	std::ifstream ifs_param47(bs_438, std::ios::in | std::ios::binary);
	ifs_param47.read((char*)(temp24), 80 * sizeof(float));
	ifs_param47.close();
	for(int co = 0; co < 5; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[362+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp24[ci+16*co]).range(wt_lenth, 0);
		}
	}

	//444 layer : index 367 368
	//float temp20[32];
	std::ifstream ifs_param48(bs_444, std::ios::in | std::ios::binary);
	ifs_param48.read((char*)(temp20), 32 * sizeof(float));
	ifs_param48.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[367+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//502 layer : index 369 370
	//float temp20[32];
	std::ifstream ifs_param49(bs_502, std::ios::in | std::ios::binary);
	ifs_param49.read((char*)(temp20), 32 * sizeof(float));
	ifs_param49.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[369+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//521 layer 371 372
	//float temp20[32];
	std::ifstream ifs_param491(bs_521, std::ios::in | std::ios::binary);
	ifs_param491.read((char*)(temp20), 32 * sizeof(float));
	ifs_param491.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[371+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//523 layer 373 374
	//float temp20[32];
	std::ifstream ifs_param4911(bs_523, std::ios::in | std::ios::binary);
	ifs_param4911.read((char*)(temp20), 32 * sizeof(float));
	ifs_param4911.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[373+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//525 layer 375
	float temp26[6];
	std::ifstream ifs_param4921(bs_525, std::ios::in | std::ios::binary);
	ifs_param4921.read((char*)(temp26), 6 * sizeof(float));
	ifs_param4921.close();
	for(int co = 0; co < 1; co++)
	{
		for(int ci = 0; ci < 6; ci++)
		{
			bias_port[375+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp26[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//525 layer 376 377
	float temp27[24];
	std::ifstream ifs_param4922(bs_526, std::ios::in | std::ios::binary);
	ifs_param4922.read((char*)(temp27), 24 * sizeof(float));
	ifs_param4922.close();
	for(int co = 0; co < 1; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[376+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp27[ci+16*co]).range(wt_lenth, 0);
		}
	}
	for(int co = 0; co < 1; co++)
	{
		for(int ci = 0; ci < 8; ci++)
		{
			bias_port[377+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp27[ci+16]).range(wt_lenth, 0);
		}
	}
	//443 layer 378 379
	//float temp20[32];
	std::ifstream ifs_param4913(bs_443, std::ios::in | std::ios::binary);
	ifs_param4913.read((char*)(temp20), 32 * sizeof(float));
	ifs_param4913.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[378+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//501 layer : index 381 382
	//float temp20[32];
	std::ifstream ifs_param4914(bs_501, std::ios::in | std::ios::binary);
	ifs_param4914.read((char*)(temp20), 32 * sizeof(float));
	ifs_param4914.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[381+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//515 layer : index 383 384
	//float temp20[32];
	std::ifstream ifs_param4915(bs_515, std::ios::in | std::ios::binary);
	ifs_param4915.read((char*)(temp20), 32 * sizeof(float));
	ifs_param4915.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[383+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//517 layer : index 385 386
	//float temp20[32];
	std::ifstream ifs_param4916(bs_517, std::ios::in | std::ios::binary);
	ifs_param4916.read((char*)(temp20), 32 * sizeof(float));
	ifs_param4916.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[385+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//519 layer 387
	//float temp26[6];
	std::ifstream ifs_param5921(bs_519, std::ios::in | std::ios::binary);
	ifs_param5921.read((char*)(temp26), 6 * sizeof(float));
	ifs_param5921.close();
	for(int co = 0; co < 1; co++)
	{
		for(int ci = 0; ci < 6; ci++)
		{
			bias_port[387+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp26[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//520 layer 388 389
	//float temp27[24];
	std::ifstream ifs_param5922(bs_520, std::ios::in | std::ios::binary);
	ifs_param5922.read((char*)(temp27), 24 * sizeof(float));
	ifs_param5922.close();
	for(int co = 0; co < 1; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[388+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp27[ci+16*co]).range(wt_lenth, 0);
		}
	}
	for(int co = 0; co < 1; co++)
	{
		for(int ci = 0; ci < 8; ci++)
		{
			bias_port[389+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp27[ci+16]).range(wt_lenth, 0);
		}
	}
	//442 layer : index 390 391
	//float temp20[32];
	std::ifstream ifs_param5923(bs_442, std::ios::in | std::ios::binary);
	ifs_param5923.read((char*)(temp20), 32 * sizeof(float));
	ifs_param5923.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[390+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//500 layer : index 392 393
	//float temp20[32];
	std::ifstream ifs_param5933(bs_500, std::ios::in | std::ios::binary);
	ifs_param5933.read((char*)(temp20), 32 * sizeof(float));
	ifs_param5933.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[392+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//509 layer : index 394 395
	//float temp20[32];
	std::ifstream ifs_param5934(bs_509, std::ios::in | std::ios::binary);
	ifs_param5934.read((char*)(temp20), 32 * sizeof(float));
	ifs_param5934.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[394+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//513 layer 396
	//float temp26[6];
	std::ifstream ifs_param5944(bs_513, std::ios::in | std::ios::binary);
	ifs_param5944.read((char*)(temp26), 6 * sizeof(float));
	ifs_param5944.close();
	for(int co = 0; co < 1; co++)
	{
		for(int ci = 0; ci < 6; ci++)
		{
			bias_port[396+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp26[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//511 layer : index 397 398
	//float temp20[32];
	std::ifstream ifs_param5954(bs_511, std::ios::in | std::ios::binary);
	ifs_param5954.read((char*)(temp20), 32 * sizeof(float));
	ifs_param5954.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[397+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//514 layer 400 401
	//float temp27[24];
	std::ifstream ifs_param5955(bs_514, std::ios::in | std::ios::binary);
	ifs_param5955.read((char*)(temp27), 24 * sizeof(float));
	ifs_param5955.close();
	for(int co = 0; co < 1; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[400+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp27[ci+16*co]).range(wt_lenth, 0);
		}
	}
	for(int co = 0; co < 1; co++)
	{
		for(int ci = 0; ci < 8; ci++)
		{
			bias_port[401+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp27[ci+16]).range(wt_lenth, 0);
		}
	}
	//441 layer : index 402 403
	//float temp20[32];
	std::ifstream ifs_param6923(bs_441, std::ios::in | std::ios::binary);
	ifs_param6923.read((char*)(temp20), 32 * sizeof(float));
	ifs_param6923.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[402+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//499 layer : index 404 405
	//float temp20[32];
	std::ifstream ifs_param6924(bs_499, std::ios::in | std::ios::binary);
	ifs_param6924.read((char*)(temp20), 32 * sizeof(float));
	ifs_param6924.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[404+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//503 layer : index 406 407
	//float temp20[32];
	std::ifstream ifs_param6934(bs_503, std::ios::in | std::ios::binary);
	ifs_param6934.read((char*)(temp20), 32 * sizeof(float));
	ifs_param6934.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[406+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//507 layer 408
	//float temp26[6];
	std::ifstream ifs_param6935(bs_507, std::ios::in | std::ios::binary);
	ifs_param6935.read((char*)(temp26), 6 * sizeof(float));
	ifs_param6935.close();
	for(int co = 0; co < 1; co++)
	{
		for(int ci = 0; ci < 6; ci++)
		{
			bias_port[408+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp26[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//505 layer : index 410 411
	//float temp20[32];
	std::ifstream ifs_param69314(bs_505, std::ios::in | std::ios::binary);
	ifs_param69314.read((char*)(temp20), 32 * sizeof(float));
	ifs_param69314.close();
	for(int co = 0; co < 2; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[410+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp20[ci+16*co]).range(wt_lenth, 0);
		}
	}
	//518 layer 412 413
	//float temp27[24];
	std::ifstream ifs_param15955(bs_508, std::ios::in | std::ios::binary);
	ifs_param15955.read((char*)(temp27), 24 * sizeof(float));
	ifs_param15955.close();
	for(int co = 0; co < 1; co++)
	{
		for(int ci = 0; ci < 16; ci++)
		{
			bias_port[412+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp27[ci+16*co]).range(wt_lenth, 0);
		}
	}
	for(int co = 0; co < 1; co++)
	{
		for(int ci = 0; ci < 8; ci++)
		{
			bias_port[413+co].range(wt_lenth + ci*16, ci*16)=((bs_type)temp27[ci+16]).range(wt_lenth, 0);
		}
	}



	//zero:
	for(int ci = 0; ci < 16; ci++)
		{
			bias_port[420].range(wt_lenth + ci*16, ci*16)=((bs_type)0).range(wt_lenth, 0);
		}

}

void fold_w3_toport(uint256 w3[170][3][3])
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
	//315layer : index 3,4,5
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

	//323 layer : index 2,3,4
	float temp2[48][48][3][3];
	std::ifstream ifs_param2(w3_323, std::ios::in | std::ios::binary);
	ifs_param2.read((char*)(**temp2), 48 * 48 * 9 * sizeof(float));
	ifs_param2.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int IDX=0;IDX<3;IDX++)
			{
				for(int i=0;i<16;i++)
				{
					w3[2+IDX][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp2[0][i+16*IDX][x][y])).range(wt_lenth, 0);
				}

			}
		}
	}

	//331 layer : index 5,6,7,8,9,10
	float temp3[96][96][3][3];
	std::ifstream ifs_param3(w3_331, std::ios::in | std::ios::binary);
	ifs_param3.read((char*)(**temp3), 96 * 96 * 9 * sizeof(float));
	ifs_param3.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int IDX=0;IDX<6;IDX++)
			{
				for(int i=0;i<16;i++)
				{
					w3[5+IDX][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp3[0][i+16*IDX][x][y])).range(wt_lenth, 0);
				}

			}
		}
	}

	//340 layer : index 11-16
	float temp4[96][96][3][3];
	std::ifstream ifs_param4(w3_340, std::ios::in | std::ios::binary);
	ifs_param4.read((char*)(**temp4), 96 * 96 * 9 * sizeof(float));
	ifs_param4.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int IDX=0;IDX<6;IDX++)
			{
				for(int i=0;i<16;i++)
				{
					w3[11+IDX][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp4[0][i+16*IDX][x][y])).range(wt_lenth, 0);
				}

			}
		}
	}

	//348 layer : index 17-22
	float temp5[96][96][3][3];
	std::ifstream ifs_param5(w3_348, std::ios::in | std::ios::binary);
	ifs_param5.read((char*)(**temp5), 96 * 96 * 9 * sizeof(float));
	ifs_param5.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int IDX=0;IDX<6;IDX++)
			{
				for(int i=0;i<16;i++)
				{
					w3[17+IDX][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp5[0][i+16*IDX][x][y])).range(wt_lenth, 0);
				}

			}
		}
	}

	//357 layer : index 23-28
	//float temp4[96][96][3][3];
	std::ifstream ifs_param6(w3_357, std::ios::in | std::ios::binary);
	ifs_param6.read((char*)(**temp4), 96 * 96 * 9 * sizeof(float));
	ifs_param6.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int IDX=0;IDX<6;IDX++)
			{
				for(int i=0;i<16;i++)
				{
					w3[23+IDX][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp4[0][i+16*IDX][x][y])).range(wt_lenth, 0);
				}

			}
		}
	}
	//366 layer : index 29-34
	//float temp4[96][96][3][3];
	std::ifstream ifs_param7(w3_366, std::ios::in | std::ios::binary);
	ifs_param7.read((char*)(**temp4), 96 * 96 * 9 * sizeof(float));
	ifs_param7.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int IDX=0;IDX<6;IDX++)
			{
				for(int i=0;i<16;i++)
				{
					w3[29+IDX][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp4[0][i+16*IDX][x][y])).range(wt_lenth, 0);
				}

			}
		}
	}
	//374 layer : index 35-46
	float temp6[192][3][3];
	std::ifstream ifs_param8(w3_374, std::ios::in | std::ios::binary);
	ifs_param8.read((char*)(**temp6), 192  * 9 * sizeof(float));
	ifs_param8.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int IDX=0;IDX<12;IDX++)
			{
				for(int i=0;i<16;i++)
				{
					w3[35+IDX][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp6[i+16*IDX][x][y])).range(wt_lenth, 0);
				}

			}
		}
	}
	//383 layer : index 47-58
	//float temp6[192][3][3];
	std::ifstream ifs_param9(w3_383, std::ios::in | std::ios::binary);
	ifs_param9.read((char*)(**temp6), 192  * 9 * sizeof(float));
	ifs_param9.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int IDX=0;IDX<12;IDX++)
			{
				for(int i=0;i<16;i++)
				{
					w3[47+IDX][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp6[i+16*IDX][x][y])).range(wt_lenth, 0);
				}

			}
		}
	}
	//392 layer : index 59-70
	//float temp6[192][3][3];
	std::ifstream ifs_param10(w3_392, std::ios::in | std::ios::binary);
	ifs_param10.read((char*)(**temp6), 192  * 9 * sizeof(float));
	ifs_param10.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int IDX=0;IDX<12;IDX++)
			{
				for(int i=0;i<16;i++)
				{
					w3[59+IDX][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp6[i+16*IDX][x][y])).range(wt_lenth, 0);
				}

			}
		}
	}

	//401 layer : index 71-82
	//float temp6[192][3][3];
	std::ifstream ifs_param11(w3_401, std::ios::in | std::ios::binary);
	ifs_param11.read((char*)(**temp6), 192  * 9 * sizeof(float));
	ifs_param11.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int IDX=0;IDX<12;IDX++)
			{
				for(int i=0;i<16;i++)
				{
					w3[71+IDX][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp6[i+16*IDX][x][y])).range(wt_lenth, 0);
				}

			}
		}
	}

	//409 layer : index 83-100
	float temp7[288][3][3];
	std::ifstream ifs_param12(w3_409, std::ios::in | std::ios::binary);
	ifs_param12.read((char*)(**temp7), 288  * 9 * sizeof(float));
	ifs_param12.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int IDX=0;IDX<18;IDX++)
			{
				for(int i=0;i<16;i++)
				{
					w3[83+IDX][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp7[i+16*IDX][x][y])).range(wt_lenth, 0);
				}

			}
		}
	}

	//418 layer : index 101-118
	//float temp7[288][3][3];
	std::ifstream ifs_param13(w3_418, std::ios::in | std::ios::binary);
	ifs_param13.read((char*)(**temp7), 288  * 9 * sizeof(float));
	ifs_param13.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int IDX=0;IDX<18;IDX++)
			{
				for(int i=0;i<16;i++)
				{
					w3[101+IDX][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp7[i+16*IDX][x][y])).range(wt_lenth, 0);
				}

			}
		}
	}

	//427 layer : index 119-136
	//float temp7[288][3][3];
	std::ifstream ifs_param14(w3_427, std::ios::in | std::ios::binary);
	ifs_param14.read((char*)(**temp7), 288  * 9 * sizeof(float));
	ifs_param14.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int IDX=0;IDX<18;IDX++)
			{
				for(int i=0;i<16;i++)
				{
					w3[119+IDX][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp7[i+16*IDX][x][y])).range(wt_lenth, 0);
				}

			}
		}
	}

	//435 layer : index 137-166
	float temp8[480][3][3];
	std::ifstream ifs_param15(w3_435, std::ios::in | std::ios::binary);
	ifs_param15.read((char*)(**temp8), 480  * 9 * sizeof(float));
	ifs_param15.close();
	for(int x=0;x<3;x++)
	{
		for(int y=0;y<3;y++)
		{
			for(int IDX=0;IDX<30;IDX++)
			{
				for(int i=0;i<16;i++)
				{
					w3[137+IDX][x][y].range(wt_lenth + i*16, i*16)=((wt_type)(temp8[i+16*IDX][x][y])).range(wt_lenth, 0);
				}

			}
		}
	}

}



void fold_w3_2_toport(uint256 w_port_3x3_2[13*4][16][3][3])
{
	//502 layer : index 0,1,2,3
	float temp[32][32][3][3];
	std::ifstream ifs_param(w3_502, std::ios::in | std::ios::binary);
	ifs_param.read((char*)(***temp), 32 * 32 * 9 * sizeof(float));
	ifs_param.close();
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+2][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out+16][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	std::ifstream ifs_param1(w3_521, std::ios::in | std::ios::binary);
	ifs_param1.read((char*)(***temp), 32 * 32 * 9 * sizeof(float));
	ifs_param1.close();
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+2+4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out+16][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	std::ifstream ifs_param2(w3_523, std::ios::in | std::ios::binary);
	ifs_param2.read((char*)(***temp), 32 * 32 * 9 * sizeof(float));
	ifs_param2.close();
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+2*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+2+2*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out+16][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	//501 layer : index 0,1,2,3
	//float temp[32][32][3][3];
	std::ifstream ifs_param4(w3_501, std::ios::in | std::ios::binary);
	ifs_param4.read((char*)(***temp), 32 * 32 * 9 * sizeof(float));
	ifs_param4.close();
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+3*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+2+3*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out+16][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	//515 layer
	std::ifstream ifs_param5(w3_515, std::ios::in | std::ios::binary);
	ifs_param5.read((char*)(***temp), 32 * 32 * 9 * sizeof(float));
	ifs_param5.close();
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+4*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+2+4*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out+16][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	//517 layer
	std::ifstream ifs_param6(w3_517, std::ios::in | std::ios::binary);
	ifs_param6.read((char*)(***temp), 32 * 32 * 9 * sizeof(float));
	ifs_param6.close();
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+5*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+2+5*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out+16][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	//500 layer
	std::ifstream ifs_param7(w3_500, std::ios::in | std::ios::binary);
	ifs_param7.read((char*)(***temp), 32 * 32 * 9 * sizeof(float));
	ifs_param7.close();
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+6*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+2+6*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out+16][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	//509 layer
	std::ifstream ifs_param8(w3_509, std::ios::in | std::ios::binary);
	ifs_param8.read((char*)(***temp), 32 * 32 * 9 * sizeof(float));
	ifs_param8.close();
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+7*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+2+7*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out+16][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	//511 layer
	std::ifstream ifs_param9(w3_511, std::ios::in | std::ios::binary);
	ifs_param9.read((char*)(***temp), 32 * 32 * 9 * sizeof(float));
	ifs_param9.close();
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+8*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+2+8*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out+16][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	//499 layer
	std::ifstream ifs_param91(w3_499, std::ios::in | std::ios::binary);
	ifs_param91.read((char*)(***temp), 32 * 32 * 9 * sizeof(float));
	ifs_param91.close();
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+9*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+2+9*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out+16][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	//503 layer
	std::ifstream ifs_param92(w3_503, std::ios::in | std::ios::binary);
	ifs_param92.read((char*)(***temp), 32 * 32 * 9 * sizeof(float));
	ifs_param92.close();
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+10*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+2+10*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out+16][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	//505 layer
	std::ifstream ifs_param94(w3_505, std::ios::in | std::ios::binary);
	ifs_param94.read((char*)(***temp), 32 * 32 * 9 * sizeof(float));
	ifs_param94.close();
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+11*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
	for (int round=0;round<2;round++)
	{
		for(int channel_out=0;channel_out<16;channel_out++)
		{
			for(int channel_in=0;channel_in<16;channel_in++)
			{
				for(int x=0;x<3;x++)
				{
					for(int y=0;y<3;y++)
					{
						w_port_3x3_2[round+2+11*4][channel_out][x][y].range(wt_lenth + channel_in*16, channel_in*16)=((wt_type)(temp[channel_out+16][channel_in+round*16][x][y])).range(wt_lenth, 0);
					}
				}

			}
		}
	}
}



//this is debug function
void check_ddr(uint256 ddr [ddrsize][ddrsize_dp],const char *filepath, int allch, int allh, int allw, int level,int m){
	int fm_size=allh*allw;
	int ip1=allch*fm_size;
	double error=0;
	double error_max=0;
	int ch_p,h_p,w_p;
	double compute_value,real_value;
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
	//double allbin=0;
	double RMSD=0;
	double allbin=0;
	//compare ddrimg and compute result and give error value
	for(int i=0;i<fm_size;i++){
		int w=i%allw;
		int h=i/allw;
		for(int ch=0;ch<allch;ch++){
			int p=ch%16;
			int c=ch/16;
			//fm_type temp_data=(fm_type)ddrimg[ch*fm_size+h*allw+w];
			temp_data.range(fm_lenth,0)=ddr[i][c+m].range(16*p+fm_lenth,16*p);
			//temp_data=temp_data/2;
			RMSD+=(ddrimg[ch*fm_size+h*allw+w]-((double)temp_data)) * (ddrimg[ch*fm_size+h*allw+w]-((double)temp_data));
			allbin+=ddrimg[ch*fm_size+h*allw+w] * ddrimg[ch*fm_size+h*allw+w];

			//find the worst data point
			error=abs(ddrimg[ch*fm_size+h*allw+w]-(double)temp_data);
			//if(ch==0&&h==1&&w==1) cout<<"test:"<<(float)temp_data<<'\n';
			//printf("%f",ddrimg[ch*fm_size+h*allw+w]);
			//printf("%f\n",(double)temp_data);
			if(error>error_max){
				ch_p=ch;
				h_p=h;
				w_p=w;
				error_max=error;
				compute_value= (double)temp_data;
				real_value=ddrimg[ch*fm_size+h*allw+w];
			}
		}
	}

	double NRMSE=RMSD/(allbin);
	cout<<"**********************"<<level<<'\n';
	cout<<"the NRMSE  of layer "<<level<<" is "<<NRMSE<<"\n";
	cout<<" the worst data point is\n";
	cout<<"channel: "<<ch_p<<" h: "<<h_p<<" w: "<<w_p<<'\n';
	cout<<"real_value: "<<real_value<<" compute value: "<<compute_value<<'\n';

}


int main()
{

	const char *img_path=imgpath;
	fold_data(IMG,img_path);



	fold_w3_toport(w3);
	fold_BS_toport(bias_port);
	fold_w1_toport(w1);
	fold_w3_2_toport(w_port_3x3_2);
			/////
/*
 uint256 w3[170][3][3];   //170
uint256 w1[1000][16];
uint256 bias_port[500];   //420
uint256		w_port_3x3_2[13*4][16][3][3];   //13*4

 */

			std::ofstream ifs_param133("C:/Users/f/Desktop/DAC2020__thuThinker/test_data/w3.bin", std::ios::out | std::ios::binary);
			ifs_param133.write((char*)(w3), 170 * 9 * sizeof(uint256));
			ifs_param133.close();

			std::ofstream ifs_param1331("C:/Users/f/Desktop/DAC2020__thuThinker/test_data/w1.bin", std::ios::out | std::ios::binary);
			ifs_param1331.write((char*)(w1), 1000 * 16 * sizeof(uint256));
			ifs_param1331.close();

			std::ofstream ifs_param13311("C:/Users/f/Desktop/DAC2020__thuThinker/test_data/bs.bin", std::ios::out | std::ios::binary);
			ifs_param13311.write((char*)(bias_port), 500  * sizeof(uint256));
			ifs_param13311.close();

			std::ofstream ifs_param133111("C:/Users/f/Desktop/DAC2020__thuThinker/test_data/w_port_3x3_2.bin", std::ios::out | std::ios::binary);
			ifs_param133111.write((char*)(w_port_3x3_2), 13*4*16*3*3  * sizeof(uint256));
			ifs_param133111.close();
			/////
	float boxs[4][5];
	return(0);
    Thinker(	 IMG ,w3,w1,bias_port,w_port_3x3_2,ddrdebug,ddrdebug_2,ddrdebug_3,ddrdebug_4,temp1,boxs);
    int n=2;
    int h=(192/n+2)*2;
    int w=(320/n+2)*2;

    n=32;
    //check_ddr(ddrdebug_4, 		add440,	80, 	2*((320/32)+2)-1,2*((192/32)+2)-1,	526,0);

	check_ddr(ddrdebug_4, 		cv526,	24, 	2*((320/32)+2)-1,2*((192/32)+2)-1,	526,0);
	check_ddr(ddrdebug_4, 		cv525,	 6, 	2*((320/32)+2)-1,2*((192/32)+2)-1,	525,2);

	//check_ddr(ddrdebug_3, 		cv501,  32, 	2*((320/16)+2),	 2*((192/16)+2),	520,0);

	check_ddr(ddrdebug_3, 		cv520,  24, 	2*((320/16)+2),	 2*((192/16)+2),	520,0);
	check_ddr(ddrdebug_3,	 	cv519,   6, 	2*((320/16)+2),	 2*((192/16)+2),	519,2);



	//check_ddr(ddrdebug_2, 		cv500,  32, 	2*((320/8)+2),	 2*((192/8)+2),		500,4);
	check_ddr(ddrdebug_2,	 	cv514,   24, 	2*((320/8)+2),	 2*((192/8)+2),		514,0);
	check_ddr(ddrdebug_2,	 	cv513,   6, 	2*((320/8)+2),	 2*((192/8)+2),		513,2);




	check_ddr(ddrdebug, 		cv508,	24, 	2*((320/4)+2),	 2*((192/4)+2),		508,0);
	check_ddr(ddrdebug, 		cv507,	 6, 	2*((320/4)+2),	 2*((192/4)+2),		507,2);
	/*the NRMSE  of layer 526 is 0.00967133
 the worst data point is
channel: 12 h: 22 w: 14
real_value: 0 compute value: 0.292969
**********************525
the NRMSE  of layer 525 is 0.000518878*/
    return 0;
}
