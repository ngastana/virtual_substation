#ifndef IEC_60870_POINT_MAP_H_
#define IEC_60870_POINT_MAP_H_

#include <stdint.h>

typedef uint64_t nsSinceEpoch;
typedef uint64_t msSinceEpoch;
// struct Iec60870PointMap
// {
// 	// CBR and SWI Status points
// 	bool POINT_28600_VALUE;
// 	bool POINT_28602_VALUE;
// 	bool POINT_28604_VALUE;
// 	bool POINT_28606_VALUE;
// 	bool POINT_28608_VALUE;
// 	bool POINT_28610_VALUE;
// 	bool POINT_28612_VALUE;
// 	bool POINT_28614_VALUE;
// 	bool POINT_28616_VALUE;
// 	bool POINT_28618_VALUE;
// 	bool POINT_28620_VALUE;
// 	bool POINT_28622_VALUE;
// 	bool POINT_28624_VALUE;
// 	bool POINT_28626_VALUE;
// 	bool POINT_28628_VALUE;
// 	bool POINT_28630_VALUE;
// 	bool POINT_28632_VALUE;
// 	bool POINT_28634_VALUE;
// 	bool POINT_28636_VALUE;
// 	bool POINT_28638_VALUE;
// 	bool POINT_28640_VALUE;
// 	bool POINT_28642_VALUE;
// 	bool POINT_28644_VALUE;
// 	bool POINT_28646_VALUE;
// 	bool POINT_28648_VALUE;
// 	bool POINT_28650_VALUE;
// 	bool POINT_28652_VALUE;
// 	bool POINT_28654_VALUE;
// 	bool POINT_28656_VALUE;
// 	bool POINT_28658_VALUE;
// 	bool POINT_28660_VALUE;
// 	bool POINT_28662_VALUE;
// 
// 
// 	// Measurement Points
// 	float 	POINT_6240_VALUE;
// 	float 	POINT_6241_VALUE;
// 	float 	POINT_28960_VALUE;
// 	float 	POINT_28961_VALUE;
// 	float 	POINT_28962_VALUE;
// 	float 	POINT_28963_VALUE;
// 	float	POINT_28972_VALUE;
// 	float 	POINT_28973_VALUE;
// 	float 	POINT_28974_VALUE;
// 	float 	POINT_28975_VALUE;
// 	float 	POINT_28976_VALUE;
// 	float 	POINT_28977_VALUE;
// 	float 	POINT_28978_VALUE;
// 	float 	POINT_28979_VALUE;
// 	float 	POINT_28980_VALUE;
// 	float 	POINT_28981_VALUE;
// 	float 	POINT_28982_VALUE;
// 	float 	POINT_28983_VALUE;
// 	float 	POINT_28984_VALUE;
// };
// 
// typedef struct Iec60870PointMap PointMap;
// extern PointMap pointMapping;


typedef union _boolIec60870PointMap
{
    struct  {
    	bool POINT_28600_VALUE;
		bool POINT_28602_VALUE;
		bool POINT_28604_VALUE;
		bool POINT_28606_VALUE;
		bool POINT_28608_VALUE;
		bool POINT_28610_VALUE;
		bool POINT_28612_VALUE;
		bool POINT_28614_VALUE;
	} boolStruct;
    bool boolVals[8];
    
} boolIec60870PointMap;


typedef union _anagIec60870PointMap
{
    struct  {
		// Measurement Points
		float 	POINT_28960_VALUE;
		float 	POINT_28961_VALUE;
		float 	POINT_28962_VALUE;
		float 	POINT_28963_VALUE;
		float	POINT_28972_VALUE;
		float 	POINT_28973_VALUE;
		float 	POINT_28974_VALUE;
		float 	POINT_28975_VALUE;	
	} anagStruct;    
    float anagVals[9];

} anagIec60870PointMap;


extern boolIec60870PointMap boolMapping;
extern anagIec60870PointMap anagMapping;

// Function definition
void initializePointMapValues(void);
void initializeArrayPointMapValues(void);

#endif /* IEC_60870_POINT_MAP_H_ */
