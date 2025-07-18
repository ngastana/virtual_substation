/*
 * iec60870_point_map.c
 *
 *  Created on: Feb 26, 2021
 *      Author: Tecnalia
 */
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "iec60870_point_map.h"

// PointMap pointMapping;

boolIec60870PointMap boolMapping;
anagIec60870PointMap anagMapping;

void initializeArrayPointMapValues(void)
{
// Single values initialization
	for (int i = 0 ; i< 8; i++) {
		anagMapping.anagVals[i] = 10.0	;
	}   
// Measurements initialization
	for (int i = 0 ; i< 19; i++) {
		boolMapping.boolVals[i] = true;	
	}   
		     
}




// void initializePointMapValues(void)
// {
// 	pointMapping.POINT_28600_VALUE = true;//
// 	pointMapping.POINT_28602_VALUE = false;//
// 	pointMapping.POINT_28604_VALUE = true;//
// 	pointMapping.POINT_28606_VALUE = false;//
// 	pointMapping.POINT_28608_VALUE = true;//
// 	pointMapping.POINT_28610_VALUE = false;//
// 	//pointMapping.POINT_28612_VALUE = true;
// 	//pointMapping.POINT_28614_VALUE = true;
// 	//pointMapping.POINT_28616_VALUE = true;
// 	//pointMapping.POINT_28618_VALUE = true;
// 	//pointMapping.POINT_28620_VALUE = true;
// 	//pointMapping.POINT_28622_VALUE = true;
// 	//pointMapping.POINT_28624_VALUE = true;
// 	//pointMapping.POINT_28626_VALUE = false;
// 	//pointMapping.POINT_28628_VALUE = true;
// 	//pointMapping.POINT_28630_VALUE = true;
// 	//pointMapping.POINT_28632_VALUE = true;
// 	//pointMapping.POINT_28634_VALUE = true;
// 	pointMapping.POINT_28636_VALUE = true;//
// 	pointMapping.POINT_28638_VALUE = false;//
// 	pointMapping.POINT_28640_VALUE = true;//
// 	pointMapping.POINT_28642_VALUE = false;//
// 	pointMapping.POINT_28644_VALUE = true;//
// 	pointMapping.POINT_28646_VALUE = false;//
// 	pointMapping.POINT_28648_VALUE = true;//
// 	pointMapping.POINT_28650_VALUE = true;//
// 	pointMapping.POINT_28652_VALUE = true;
// 	pointMapping.POINT_28654_VALUE = true;//
// 	pointMapping.POINT_28656_VALUE = false;//
// 	pointMapping.POINT_28658_VALUE = true;//
// 	pointMapping.POINT_28660_VALUE = true; //
// 	pointMapping.POINT_28662_VALUE = false;//
// 
// 	// Measurement initialization
// 	pointMapping.POINT_6241_VALUE  = 58.1;
// 	pointMapping.POINT_28960_VALUE = 233.4;
// 	pointMapping.POINT_28961_VALUE = 53.7;
// 	pointMapping.POINT_28962_VALUE = 22.2;
// 	pointMapping.POINT_28963_VALUE = 138;
// 	pointMapping.POINT_6240_VALUE =  113.0;
// 	pointMapping.POINT_28972_VALUE = 238.7;
// 	pointMapping.POINT_28973_VALUE = 112.9;
// 	pointMapping.POINT_28974_VALUE = 4.6;
// 	pointMapping.POINT_28975_VALUE = 303;
// 	pointMapping.POINT_28976_VALUE = 234.3;
// 	pointMapping.POINT_28977_VALUE = 234.3;
// 	pointMapping.POINT_28978_VALUE = 60.01;
// 	pointMapping.POINT_28979_VALUE = 60.03;
// 	pointMapping.POINT_28980_VALUE = 231.5;
// 	pointMapping.POINT_28981_VALUE = 170.9;
// 	pointMapping.POINT_28982_VALUE = 16.7;
// 	pointMapping.POINT_28983_VALUE = 414;
// 	pointMapping.POINT_28984_VALUE = 60.02;
// }



