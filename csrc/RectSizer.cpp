#include <stdio.h>
#include <ctime>
#include "RectSizer.h"

CRectSizer::CRectSizer()
{
	region_id = 0;
	this->_region = (RegionSizer *)malloc(sizeof(RegionSizer));
	this->_ROI = (RegionSizer *)malloc(sizeof(RegionSizer));
	this->_ROI->next = NULL;
	this->_ROI->InnerCount = -1;

	this->_roiRect = (ROIRect *)malloc(sizeof(ROIRect));
	this->_roiRect->id = -1;
	this->_roiRect->InnerCount = 0;
	this->_roiRect->next = NULL;
	initRegion(cvRect(0,0,0,0),this->_region,region_id);
}
CRectSizer::CRectSizer(CSeqRect *seq)
{
	this->_seqRect = seq;
	region_id = 0;
	this->_region = (RegionSizer *)malloc(sizeof(RegionSizer));
	this->_rects = NULL;
	this->_ROI = (RegionSizer *)malloc(sizeof(RegionSizer));
	this->_ROI->next = NULL;
	this->_ROI->InnerCount = -1;

	this->_roiRect = (ROIRect *)malloc(sizeof(ROIRect));
	this->_roiRect->id = -1;
	this->_roiRect->InnerCount = 0;
	this->_roiRect->next = NULL;
	initRegion(this->_seqRect->seqNode->rect,this->_region,region_id);
}
CRectSizer::~CRectSizer()
{
	free(this->_region);
}
void CRectSizer::setHSV(IplImage *img)
{
	IplImage *hsv =  cvCloneImage(img);
	/*
	IplImage *_h =  cvCreateImage(cvGetSize(inImage),8,1);
	IplImage *_s =  cvCreateImage(cvGetSize(inImage),8,1);
	IplImage *_v =  cvCreateImage(cvGetSize(inImage),8,1);
	cvSplit(hsv,_h,_s,_v,NULL);
	*/
	cvCvtColor(img, hsv, CV_BGR2HSV);	
	this->_hsv = hsv;
}
void CRectSizer::initRegion(CvRect rect,RegionSizer *region,int regionid)
{
	region->left = rect.x-EPSILON_X;
	region->right = rect.x + EPSILON_X;
	region->up =	rect.y + EPSILON_Y;
	region->down =  rect.y - EPSILON_Y;
	region->id = regionid;
	region->InnerCount = 1;
	region->per_width =  rect.width;
	region->max_y =  rect.y + rect.height;
	region->min_y = rect.y;
	region->prev = region->next = NULL;
}
void CRectSizer::initRegion(CvRect rect,RegionSizer *region,RegionSizer *regionOld)
{
	region->left = rect.x-EPSILON_X;
	region->right = rect.x + EPSILON_X;
	region->up =	rect.y + EPSILON_Y;
	region->down =  rect.y - EPSILON_Y;
	region->id = region_id;
	region->InnerCount = 1;
	region->next = NULL;
	region->per_width =  rect.width;
	region->max_y =  rect.y + rect.height;
	region->min_y = rect.y;
	RegionSizer *tmp = regionOld;
	while(tmp->next)
	{
		tmp = tmp->next;
	}
	region->prev = tmp;
	tmp->next = region;
}
RegionSizer *CRectSizer::getROI()														//�������Ҫ�ġ�
{
	SeqRect *prect = this->_seqRect->seqNode;

	while(prect) 
	{
		int average[3] = {0,0,0};
		int var[3] = {0,0,0};
		
		averageRectValue(prect,average,var);
		
		RegionSizer *cur_region = getRegion(prect->region_id);
		
		int innercount = cur_region->InnerCount;

		bool average_condition =  (average[0]>=150 && average[2]>=80) || (average[0] <= 150 && average[2]>=90 );	//��Ϊopencv��HSV�ռ��H��Χ��0~180����ɫ��H��Χ�����(0~8)��(160,180),S���ͶȺ���Ҫ��һ���Ǵ���һ��ֵ,S���;��ǻ�ɫ���ο�ֵS>80)��V�����ȣ����;��Ǻ�ɫ�����߾��ǰ�ɫ(�ο�ֵ220>V>50) http://zhidao.baidu.com/question/468254593.html 
		bool var_condition = var[0] < 1500 && var[2] > 30;
		bool count_condition = innercount>0 && innercount<10;			 				//�ַ�����
		bool height_condition = (cur_region->max_y - cur_region->min_y) > 16;
		bool condition = var_condition && average_condition && count_condition && height_condition;
		
		if(condition)
		{
			createROIRectList(prect);
		}
		prect = prect->next;
	}

	detectDistance();			//����Ҫ����������⺯��

	ROIRect *proi = this->_roiRect;
	while(proi)
	{
		setROIRect(proi);										//��proiת����curROI����
		proi = proi->next;
	}

	RegionSizer *curROI = this->_ROI;
	while(curROI!=NULL)											//��������	
	{
		CvRect tmpRect = generateConnectedRegion(curROI);
		transformXY(tmpRect,curROI);
		curROI = curROI->next;
	}
	return this->_ROI;
}
void CRectSizer::detectDistance()
{
	ROIRect *proi = this->_roiRect;
	while(proi)
	{
		
		for(int i=0;i<proi->InnerCount;i++)
		{
			proi->minDist[i] = INFINIT;
			for(int j=0;j<proi->InnerCount;j++)
			{	
				if(i!=j)
				{
					int md = abs(proi->rect[i].x - proi->rect[j].x);
					proi->minDist[i] =(proi->minDist[i] <= md) ? proi->minDist[i] : md;
				}
			}
		}
		if(proi->InnerCount == 1)
		{
			proi->minDist[0] = 0 ;
		}
		for(int k=0;k<proi->InnerCount;k++)
		{
			proi->minDist[k] -= cvRound(proi->aveWidth*4);		//������������3��Ȼ��ȥ���ö����ģ���2�Ͳ���	
			if(proi->minDist[k] > 0)							
			{
				proi->minDist[k] = ERROR_STATUS;
			}
			else
			{
				proi->minDist[k] = RIGHT_STATUS;
			}
		}
		proi = proi->next;
	}
}
ROIRect *CRectSizer::getROIRect(int id)								//����ROI��id = id��ROIRect���ϣ����_ROI->id=-1������ROI�����û�У��½�һ���ڵ�
{
	ROIRect *proi = this->_roiRect;
	if(proi->id == -1)
	{
		proi->id=id;
		return proi;
	}
	if(id == proi->id)
	{
		return proi;
	}
	while(proi->next)
	{
		proi = proi->next;
		if(id == proi->id)
		{
			return proi;
		}
	}
	ROIRect *newroi = (ROIRect *)malloc(sizeof(ROIRect));
	newroi->id=id;
	newroi->InnerCount = 0;
	newroi->next = NULL;
	proi->next = newroi;
	proi = newroi;
	return proi;
}
ROIRect *CRectSizer::getLastROIRect()								//�Ѿ�����
{
	ROIRect *proi = this->_roiRect;
	while(proi->next)
	{
		proi = proi->next;
	}
	return proi;
}
void CRectSizer::createROIRectList(SeqRect *prect)					//������¼ÿ������ľ���
{	
	SeqRect *tmpprect = prect;
	ROIRect *proi ;
	proi = getROIRect(tmpprect->region_id);// this->_roiRect;  //��ȡROI��id��region_id��ROI, �������½�
	proi->aveWidth = 0;
	proi->rect[proi->InnerCount] = tmpprect->rect;
	proi->InnerCount++;
	proi->aveWidth = (proi->aveWidth > tmpprect->rect.width) ? proi->aveWidth :tmpprect->rect.width;// cvRound((proi->aveWidth*(proi->InnerCount-1) + tmpprect->rect.width)/proi->InnerCount);
	
}
void CRectSizer::setROIRect(ROIRect *proi)
{
	RegionSizer *curROI = this->_ROI;
	RegionSizer *oldROI =  this->_ROI;
	while(curROI)
	{ 		
 
		if(curROI->InnerCount == -1)
		{ 
			curROI->id = proi->id;
			int count = setRegionBorder(curROI,proi);
			curROI->InnerCount = count;
			break;
		}
		else
		{ 
			oldROI = curROI;
			curROI = curROI->next;
			if(curROI == NULL) 
			{
				RegionSizer *regionNew = (RegionSizer *)malloc(sizeof(RegionSizer));
				regionNew->prev = oldROI;
				oldROI->next = regionNew;
				regionNew->next=NULL;

				int count = setRegionBorder(regionNew,proi);
				regionNew->InnerCount = count;
				regionNew->id = proi->id;
			}
		}
	}

}

void CRectSizer::sizeRect2Region()								//�����ο����EPSILON�ֳɲ�ͬ������
{
	SeqRect *prect = this->_seqRect->seqNode;
	while(prect)
	{
		RegionSizer *region = this->_region;
		RegionSizer *regionOld =this->_region;
		while(region)
		{
			if(inRegion(prect->rect,region))					//�ж��Ƿ���������
			{
					prect->region_id = region->id;
					break;
			}
			else												//��������ڵ�Ļ�������ɨ
			{
				regionOld = region;
				region = region->next;
			}
			if(region == NULL)									//���ײ��ڣ��½�һ������Χ
			{
				region_id ++;
				RegionSizer *regionNew = (RegionSizer *)malloc(sizeof(RegionSizer));
				initRegion(prect->rect,regionNew,regionOld);
				prect->region_id = regionNew->id;
			}
			
		}
		prect = prect->next;
	}
}
int CRectSizer::setRegionBorder(RegionSizer *region,ROIRect *proi) 
{
	int count = 0;
	for(int i=0; i<proi->InnerCount;i++)
	{
		if(0==i)
		{
			region->left	=	proi->rect[i].x - EPSILON_X;
			region->right	=	proi->rect[i].x + proi->rect[i].width +EPSILON_X;
			region->up		=	proi->rect[i].y + EPSILON_Y;
			region->down	=	proi->rect[i].y - EPSILON_Y;
			region->max_y	=	proi->rect[i].y + proi->rect[i].height;
			region->min_y	=	proi->rect[i].y;
		}
#if debug
		printf("xxx%d\n",proi->minDist[i]);
		printf("(%d,%d,%d,%d)\n",region->left,region->right,region->up,region->down);
#endif
		if(proi->minDist[i] == RIGHT_STATUS)
		{
			setRegionBorder(region,proi->rect[i]);
			count++;
		}
	}
	return count;

}
void CRectSizer::setRegionBorder(RegionSizer *region,CvRect rect)
{
			region->left  = (region->left  <= rect.x - EPSILON_X) ? (region->left ):(rect.x - EPSILON_X);
			region->right = (region->right >= rect.x + rect.width +  EPSILON_X) ? (region->right) : (rect.x + rect.width +  EPSILON_X);
			region->up	  = (region->up    >= rect.y + rect.height) ? (region->up   ):(rect.y + rect.height);
			region->down  = (region->down  <= rect.y - EPSILON_Y) ? (region->down ):(rect.y - EPSILON_Y);
			
			region->max_y = (region->max_y >= rect.y + rect.height) ? region->max_y :  rect.y + rect.height + EPSILON_Y;
			region->min_y = (region->min_y <= rect.y + rect.height) ? region->min_y :  rect.y + rect.height + EPSILON_Y;
			
			//region->per_width =(int)((region->per_width + rect.width)/2);									//����ƽ��ֵ
			region->per_width = (region->per_width >=rect.width)? (region->per_width) : rect.width;			//ȡ���ֵ����
}
bool CRectSizer::inRegion(CvRect rect,RegionSizer *region)
{
#if debug
	printf("(%d,%d)(%d,%d,%d,%d)\n",rect.x,rect.y,region->left, region->right,region->down,region->up);

#endif
	if(rect.x >= region->left && rect.x <= region->right && rect.y >= region->down && rect.y <= region->up)
	{
		region->InnerCount ++;
		setRegionBorder(region,rect);
		return true;
	}
	else
	{
		return false;
	}
}

RegionSizer *CRectSizer::getRegion(int id)
{
	int num = 0 ;
	RegionSizer *region = this->_region;
	while (region)
	{
		if(num == id)
			break;
		num ++;
		region = region ->next;
	}
	return region;
}
//��left right up down ��� x y width height
void CRectSizer::transformXY(CvRect rect,RegionSizer *region)
{
	region->left = rect.x;
	region->right = rect.y;
	region->up = rect.width;
	region->down = rect.height;
}

//����������ROI����
CvRect CRectSizer::generateConnectedRegion(RegionSizer *region)
{
	if(!region)
	{
		return cvRect(0,0,0,0);
	}/////@@@@@@@@@@@@@@@@����3
	int x = region->left + EPSILON_X-5 > 0 ? region->left + EPSILON_X-5  : 0;
	int y = region->down + EPSILON_Y-5 > 0 ? region->down + EPSILON_Y -5: 0;
	int width = region->right - EPSILON_X + 10 - (region->left + EPSILON_X)   ;// + region->per_width;
	int height = region->max_y + 10 - region->min_y ;
	return cvRect(x,y,width,height);
}

//��Ҫ�ȵ���SetHSV
bool CRectSizer::averageRectValue(SeqRect *prect,int *average,int *var)  //average ƽ��ֵ var ����
{
	if(!prect)
	{
		return false;
	}
	IplImage *img = this->_hsv;
	CvRect rect = prect->rect;
	int x,y;
	int sum[3]={0,0,0};
	long int sum2[3] = {0,0,0};
	int amount = 0;
	for(y=rect.y+4;y<rect.y+rect.height-4;y++)				//  ͼƬ������Ԫ�ؾ�ֵ �� ����
	{
		 unsigned char* line = (unsigned char*)(img->imageData + y*img->widthStep);
		 for( x=rect.x + 4;x<rect.x+rect.width - 4;x++)
		 {
			 sum[0] = sum[0] + line[3*x + 0];
			 sum[1] = sum[1] + line[3*x + 1];
			 sum[2] = sum[2] + line[3*x + 2];
	
			 sum2[0] = sum2[0] + line[3*x + 0] * line[3*x + 0];
			 sum2[1]= sum2[1] + line[3*x + 1] * line[3*x + 1];
			 sum2[2] = sum2[2] + line[3*x + 2] * line[3*x + 2];
			 amount++;
		 }
	}	

	prect->avarage[0] = average[0] = (int)floor(sum[0] *1.0 / (amount *1.0));
	prect->avarage[1] = average[1] = (int)floor(sum[1] *1.0 / (amount *1.0));
	prect->avarage[2] = average[2] = (int)floor(sum[2] *1.0 / (amount *1.0));
	prect->var[0] = var[0] = (int)floor(sum2[0] * 1.0 / (amount * 1.0) - (average[0] * average[0] * 1.0 ));			//���� E(X^2) - E(X)^2
	prect->var[1] = var[1] = (int)floor(sum2[1] * 1.0 / (amount * 1.0) - (average[1] * average[1] * 1.0 ));
	prect->var[2] = var[2] = (int)floor(sum2[2] * 1.0 / (amount * 1.0) - (average[2] * average[2] * 1.0 ));
	
	return true;
}


void CRectSizer::setROIRect(SeqRect *prect)		//�Ѿ�������
{
	RegionSizer *curROI = this->_ROI;
	RegionSizer *oldROI =  this->_ROI;
	while(curROI)
	{
		if(curROI->InnerCount == -1)
		{
			initRegion(prect->rect,curROI,prect->region_id);
			break;
		}
		else if(prect->region_id == curROI->id)
		{
			setRegionBorder(curROI,prect->rect);
			curROI->InnerCount = curROI->InnerCount ++ ;
			break;
		}
		else
		{ 
			oldROI = curROI;
			curROI = curROI->next;
			if(curROI == NULL) 
			{
				RegionSizer *regionNew = (RegionSizer *)malloc(sizeof(RegionSizer));
				initRegion(prect->rect,regionNew,oldROI);
				regionNew->id = prect->region_id;
			}
		}
	}
}