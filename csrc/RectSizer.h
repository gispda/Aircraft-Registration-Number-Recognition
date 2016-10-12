#ifndef __RECTSIZER_H_
#define __RECTSIZER_H_
/////////////////////////////////////////////////////
#include <cv.h>
#include "SeqRect.h"
////@@@@@@@@@@@@@@@@@@����2
#define EPSILON_X 30
#define EPSILON_Y 10
#define ERROR_STATUS 9999
#define RIGHT_STATUS -9999

typedef struct regionSize
{
	int left,right,up,down;
	int id;
	int InnerCount;
	int per_width;
	int max_y;
	int min_y;
	struct regionSize *next;
	struct regionSize *prev;
}RegionSizer;
typedef struct roiRect
{
	CvRect rect[10];
	int minDist[10];
	int aveWidth;
	int id;
	int InnerCount;
	struct roiRect *next;
}ROIRect;
class CRectSizer
{
public:
	CRectSizer();
	~CRectSizer();
	CRectSizer(CSeqRect *seq);
public:
	void sizeRect2Region();												//����Ҫ�ĺ���֮һ��size��˼�Ƿ��������ǰѾ��ηֵ�Region�����������Ǹ�����
	void setHSV(IplImage *img);											// ��ͼ���Ƶ�������˽�г�Ա_hsv
	bool averageRectValue(SeqRect *prect,int *average,int *var);		//Get Rect HSV average value and Variance value 
public:		//Region	���򼰸���Ȥ�����򣨽�����һ������Χ��
	CvRect generateConnectedRegion(RegionSizer *region);				// �ϲ���ͨ��
	int setRegionBorder(RegionSizer *region,ROIRect *proi);				// ����
	RegionSizer *getRegion(int id);										//ͨ��ID��ȡRegion
	RegionSizer *getROI();												// �жϾ����ǲ��Ǻϸ��Լ��������������ǲ��Ǻϸ񣬿ɲ����Ե���ROI
	void detectDistance();

public:		//ROI Rect����Ȥ�����Rect���ϣ�����������һЩ���μ��ϣ�
	void createROIRectList(SeqRect *prect);								//����ROI���μ�������
	ROIRect *getROIRect(int id);										//ͨ��ID��ȡ���μ���
	ROIRect *getLastROIRect();											//��ȡ���μ��������

	void setROIRect(SeqRect *prect);	//�Ѿ�����						// Ϊ���κϸ�&&������������ϸ�ľ��Σ��������Region���֣�������ROI
	void setROIRect(ROIRect *proi);										// ��roi���μ������ϳ�ROI Region
	

protected:
	void setRegionBorder(RegionSizer *region,CvRect rect);				//����Χ��չ�����rect+epsilon�������⣬���
	void initRegion(CvRect rect,RegionSizer *region,int regionid);		//��ʼ��Region
	void initRegion(CvRect rect,RegionSizer *region,RegionSizer *regionOld);	//���س�ʼ��Region
	bool inRegion(CvRect rect,RegionSizer *region);								//�ж��ǲ�����ĳ��_region
	void transformXY(CvRect rect,RegionSizer *region);							//��region�����left right up down �ֱ��Ӧrect�� x y width height
public:
	RegionSizer *_region;												//�����еľ����ж���������[û�кϲ���ͨ��]
private:
	ROIRect *_roiRect;					//�������ڵ�Rect
	RegionSizer *_ROI;					//���������Ȥ�����򣬺ϲ���ͨ��֮���

	CSeqRect *_seqRect;					//���������е�Rect
	SeqRect *_rects;					//��ʱû�ã���������������Rect��
	IplImage *_hsv;
	int region_id;						//���������id
};


#endif