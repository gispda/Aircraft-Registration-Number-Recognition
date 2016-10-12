/*****************************************************************
** Copyright 2008-2010 SDL of GUCAS.		All rights reserved **
**																**
** version 1.0													**
** Date: DEC 30th, 2010		by LiangJixiang						**
**																**
** src	 file for		 feature extraction						**
**																**
******************************************************************/
#include "IntegralFea.h"
#include <math.h>

#include <afx.h>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

/////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//һ��С�������������ݶȵķ���
//�����Dx,Dy�Ƿ�ָ����һ�����ص���Ǽ����ݶȷ����dx��dy
//���ڼ���(HAAR,MSO)�鷽��,Dx(Dy)ָ����ˮƽ(��ֱ)����Ĳ�.
int f(int Dx, int Dy)
{
	float Div;
    if(Dx != 0)
	{
		Div = (float) Dy / (float) Dx;
		/////////////////////////////////////////
		if(Div >= 0 && Div < 0.3639)              //tan 20 = 0.3639
			return 0;
		else if(Div >= 0.3639 && Div < 0.8391)        //tan 40 = 0.8391
			return 1;
		else if(Div >= 0.8391 && Div < 1.7321)        //tan 60 = 1.7321
			return 2;
		else if(Div >= 1.7321 && Div < 5.6713)        //tan 80 = 5.6713
			return 3;
		else if(Div >= 5.6713 || Div <= -5.6713)      //tan 100 = -5.6713
			return 4;
		else if(Div >= -5.6713 && Div < -1.7321)      //tan 120 = -1.7321
			return 5;
		else if(Div >= -1.7321 && Div < -0.8391)      //tan 140 = -0.8391
			return 6;
		else if(Div >= -0.8391 && Div < -0.3639)      //tan 160 = -0.3639
			return 7;
		else //if(Div >= -0.3639 && Div < 0)
			return 8;
	}
	else
	{
		if(Dx == 0 && Dy != 0)
			return 4;                  //ע��bug!
	    else //if(Dx == 0 && Dy == 0)
			return 0;                  //�����Ժ�С���ǲ��ܺ���
	}
}

//////////////////////////////////////////////////////////////////////////////
//�����Ĳ��� ��ʼ��һ������
bool InitiateMSOFea(MSOFea &fea)
{
	/////////////////////////////////////
	fea.active  = 1;
	fea.orient  = 0;
	fea.x1      = 0;
	fea.x2      = 0;
	fea.y1      = 0;
	fea.y2      = 0;
	fea.dim     = 9;

	///////////////////////////////////////	
	for(int i = 0; i < BINS; i++)	fea.fea[i] = 0.0;

	return true;
}
////////////////////////////////////////////////////////////////////////////
//copy һ������
bool CopyMSOFea(MSOFea &src, MSOFea &tgt)
{
	tgt.active  = src.active;
	tgt.orient  = src.orient;
	tgt.x1      = src.x1    ;
	tgt.x2      = src.x2    ;
	tgt.y1      = src.y1    ;
	tgt.y2      = src.y2    ;	

	for(int i = 0; i < BINS; i++)
	tgt.fea[i] = src.fea[i];

	return true;
}
///////////////////////////////////////////////////////////////////
//����һ���������ϵ����ݽṹ
FeaSet* AllocFeaSet(int cell_num, int adjacent=1, int fea_type = HOG)
{
	FeaSet *set= new FeaSet;

	if(cell_num <=0) return NULL;

	set->cellfea     = (CellFea*)malloc(cell_num*sizeof(CellFea));
	if(set->cellfea == NULL){
		printf("Can not malloc !\n");
		return 0;
	}
	set->cell_num    = cell_num;
	set->adjacent_num= adjacent;
	set->fea_type    = fea_type;
	for(int i = 0; i < cell_num; i++)
		InitiateMSOFea(set->cellfea[i]);		
	return set;
}
///////////////////////////////////////////////////////////////////
//�Ӹ�ʽ�����������ļ��������������Ľṹ
FeaSet* ReadFeaSetFromFile(char path[255])
{
	int fea_type,cell_num,adjacent_num,cell_dim;
	int c;

	FILE * fp = fopen(path,"r");

	FeaSet *set;

	if(NULL ==fp) return NULL;

	char tmp[255];

	fscanf(fp,"%s %d",tmp,&fea_type);
	fscanf(fp,"%s %d",tmp,&cell_num);
	fscanf(fp,"%s %d",tmp,&adjacent_num);
	fscanf(fp,"%s %d",tmp,&cell_dim);

	set = AllocFeaSet(cell_num,adjacent_num,fea_type);

	for(c = 0; c<cell_num; c++)
	{
		fscanf(fp,"%d %d %d %d",
			&(set->cellfea[c].x1),&(set->cellfea[c].y1),
			&(set->cellfea[c].x2),&(set->cellfea[c].y2));

		set->cellfea[c].dim   = cell_dim;
		set->cellfea[c].active=1; 
		set->cellfea[c].orient=-1;
	}
	fclose(fp);

	return set;
}
///////////////////////////////////////////////////////////////////
//�ͷ�һ�������ṹ
bool  FreeFeaSet(FeaSet *set)
{
	if(NULL == set) return 0;
	if(NULL!=set) free(set->cellfea);
	set->cell_num =1;
	set->adjacent_num =1;

	free(set);

	set = NULL;

	return 1;
}
/////////////////////////////////////////////////////////////////////////
//img �����룬�����ǲ�ɫ���Ҷ�ͼ�񣬻����ݶȷ���ͼ��
//IntergralImage �ǻ���ͼ
//1   2
//4   3
/////////////////////////////////////////////////////////////////////////
bool IntegralImage_char(void *pImage, long *IntergralImage, int wid, int hei, int nBand)
{
	int h,w,b;
	long loc,loc1,loc2,loc3,loc4;
	
	
	unsigned char *img = (unsigned char *)pImage;
	
	//����ͼIntergralImage��ʼ��
	for(h =0; h < wid*hei*nBand; h++)IntergralImage[h] = 0;
	
	//��ʼ����һ������
	for(b= 0; b< nBand; b++)	IntergralImage[b]  =  (img[b]);
	
	//��ʼ����һ�л���ͼ
	for(w = 1; w < wid; w++){
		loc = w * nBand;
		
		for(b = 0; b< nBand; b++)
			IntergralImage[loc+b] = IntergralImage[loc-nBand+b] +img[loc+b];
	}

	//��ʼ����һ�л���ͼ
	for(h = 1; h < hei; h++){
		loc = h*wid*nBand;

		for(b = 0; b< nBand; b++)
		IntergralImage[loc+b] = IntergralImage[loc-wid*nBand] +img[loc+b];
	}

	//�������л���ͼ
	for(h = 1; h < hei; h++){
		for(w = 1; w < wid; w++){
			
			loc3 = (h*wid+w)*nBand;
			loc1 = ((h-1)*wid+w-1)*nBand;
			loc2 = ((h-1)*wid+w)*nBand;
			loc4 = (h*wid+w-1)*nBand;
			
			for(b= 0; b< nBand; b++){		 
				IntergralImage[loc3+b] = IntergralImage[loc4+b] +IntergralImage[loc2+b]
										- IntergralImage[loc1+b]+img[loc3+b];
			}			
	}}	

	return 1;
}

////////////////////////////////////////////////////////////////////////
//���������ݶ�,����"�ݶ�ͼ"
//input: wid,hei��ͼ��ߴ�, nBandͼ����ɫͨ��������RGB��nBand=3
//��SURF����gradient ��һ��4��ͼ���С��ͼ�����dx,dy,|dx|,|dy|...
//��HOG����gradient��һ��9��ͼ���С��ͼ,���ÿ�����ص���9��bins�ϵ�ͶӰ
//    3 
//1  loc   2
//    4
bool ComputerGradient(unsigned char *img, int nBand,int *gradient, int wid, int hei)
{
	int h,w,c;
	long pos1,pos2,pos3,pos4,loc;
	int max_dx,max_dy,max;
	int orient = -1;
	int g_x,g_y;
	long img_size = wid * hei;	
	
	///////////////////////////////////////////////////////////////
	//����ÿ�����ص��ݶ�
	//gradient һ��Ҫ��ʼ��
	for(int i = 0; i < wid*hei*HOG_BINS; i++){
		gradient[i] = 0;
	}
	for(h = 1; h < hei - 1; h++)
	{
		for(w = 1; w < wid - 1; w++)
		{
			max_dx = 0;         //ע��!һ��Ҫ��cѭ�����ʼ��!
			max_dy = 0;
			max = 0;
			
			loc = h * wid + w;	
			
			for(c = 0 ; c< nBand; c++)                //��ɫ����
			{
				pos1 = (loc - 1) * nBand + c;
				pos2 = (loc + 1) * nBand + c;
				pos3 = (loc - wid) * nBand + c;
				pos4 = (loc + wid) * nBand + c;				
				g_x = (int)(img[pos2] - img[pos1]);    //�ݶ�
				g_y = (int)(img[pos4] - img[pos3]);    //�ݶ�								
				int temp = g_x*g_x + g_y*g_y;
				//�����ݶȺ������Ǹ���ɫͨ��
				if(temp > max)
				{
					max_dx = g_x;
					max_dy = g_y;
					max   = temp;					
				}
			}			
			if(abs(max_dx) <= THREHOLD && abs(max_dy) <= THREHOLD)
				continue;
			if(max_dy < 0){                                             
				max_dy = -max_dy;                               
				max_dx = -max_dx;                               
			}
			orient = f(max_dx, max_dy);                        //��������ص�ķ���	
			float magnitude = sqrt(max_dx*max_dx + max_dy*max_dy);	
			gradient[HOG_BINS * loc + orient] = magnitude;	
		} //for-w
	} //for-h
	return 1;
}
///////////////////////////////////////////////////////////////////////////////
//�����ݶ����͵Ļ���ͼ
//3   2
//1   0
////////////////////////////////////////////////////////////////////////////////
bool IntegralImage_int(int *img, long *IntergralImage, int wid, int hei, int nBand)
{
	int h,w,b;
	long loc,loc1,loc2,loc3;
	//IntergralImage ��ʼ��
	for(h =0; h < wid*hei*nBand; h++)IntergralImage[h] = 0;

	//��ʼ����һ������
	for(b = 0; b< nBand; b++)	IntergralImage[b]  =  img[b];

	//��ʼ����һ�л���ͼ
	for(w = 1; w < wid; w++)
	{
		loc = w * nBand;
		
		for(b= 0; b< nBand;b++)
		IntergralImage[loc + b] = IntergralImage[loc - nBand + b] +img[loc + b];
	}

	//��ʼ����һ�л���ͼ
	for(h = 1; h < hei; h++)
	{
		loc = h*wid*nBand;

		for(b= 0; b< nBand; b++)
		IntergralImage[loc + b] = IntergralImage[loc - wid * nBand] +img[loc + b];
	}

	//�������л���ͼ
	for(h = 1; h < hei; h++){
		for(w = 1; w < wid; w++){			
			loc = (h*wid+w)*nBand;
			loc1 = loc - nBand;
			loc2 = loc - wid*nBand;
			loc3 = loc - wid*nBand-nBand;			
			for(b= 0; b< nBand;b++){		 
				IntergralImage[loc+b] = IntergralImage[loc1 + b] +IntergralImage[loc2 + b]
					- IntergralImage[loc3 + b]+img[loc + b];
			}			
		}
	}	
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
bool ComputSmallSURFFea(long *IntegralColor, int wid, int hei, CellFea &fea,int nBand)
{

	////////////////////////////////////////////////
	if(!fea.active) return false;	
	fea.orient  = -2;// ��ʼ��

	int h,b;
	long loc[12];
	int fea_wid = fea.x2 - fea.x1 + 1;
	int fea_hei = fea.y2 - fea.y1 + 1;
	
	long up[3],down[3],left[3],right[3];
	long max_dx,max_dy,dx,dy;
	/////////////////////////////////////////////
	//���ݻ���ͼ�������� ���dx,���dy,���|dx|,���|dy|,orientation
	//ע�⵱��Ĵ�С��ż�����ĳ��Ϳ�ʱ
	//����ͨ��8�����Ǽ��㲻��dx��dy�ı���Ҫ12����!
	/////////////////////////////////////////////	
	int pos1 = fea.y1*wid;
	int pos2 = fea.y2*wid;
	
	loc[0] = pos1 + fea.x1;
	loc[1] = pos1 + fea.x2;
	loc[2] = pos2 + fea.x1;
	loc[3] = pos2 + fea.x2;
	
	loc[4] = (fea.y1 + fea.y2) / 2 * wid + fea.x1;
	loc[5] = ((fea.y1 + fea.y2) / 2 + 1) * wid + fea.x1;//+ wid;
	loc[6] = (fea.y1 + fea.y2) / 2 * wid + fea.x2;
	loc[7] = ((fea.y1 + fea.y2) / 2 + 1) * wid + fea.x2; //+ wid;
	loc[8] = pos1 + (fea.x1 + fea.x2) / 2;
	loc[9] = pos1 + (fea.x1 + fea.x2) / 2 + 1;
	loc[10]= pos2 + (fea.x1 + fea.x2) / 2;
	loc[11]= pos2 + (fea.x1 + fea.x2) / 2 + 1; 

	for(h = 0; h < 12; h++){
		loc[h] *= nBand;
	}
	
	for(b = 0; b < nBand; b++){	
	
		up[b]    = IntegralColor[loc[0]+b]+ IntegralColor[loc[6]+b] - 
			       IntegralColor[loc[1]+b]- IntegralColor[loc[4]+b];
		
		down[b]  = IntegralColor[loc[5]+b]+ IntegralColor[loc[3]+b] - 
			       IntegralColor[loc[2]+b]- IntegralColor[loc[7]+b];
		
		left[b]  = IntegralColor[loc[0]+b]+ IntegralColor[loc[10]+b] - 
			       IntegralColor[loc[2]+b]- IntegralColor[loc[8]+b];

		right[b] = IntegralColor[loc[9]+b]+ IntegralColor[loc[3]+b] - 
			       IntegralColor[loc[11]+b]- IntegralColor[loc[1]+b];
	}

	max_dx = max_dy =0;	
	
	//�ڸ�����ɫͨ����ѡ��������Ŀ��ݶ�
	for(b= 0; b<nBand; b++)
	{
		dx = (left[b] - right[b]);
		dy = (up[b]   - down[b] );

		if(abs(dx) > max_dx) max_dx = dx;
		if(abs(dy) > max_dy) max_dy = dy;		
	}
	//2011-11-24    by liang ����ƽ̹����
	int basic_t = THREHOLD*fea_wid*fea_hei/4;
	if(abs(max_dx) < basic_t && abs(max_dy) < basic_t){
		max_dx = 0;
		max_dy = 0;
	}
	//����haar-like ����
	fea.fea[0] = (max_dx) / 255.0;
	fea.fea[1] = (max_dy) / 255.0;
	fea.fea[2] = abs(max_dx) / 255.0;
	fea.fea[3] = abs(max_dy) / 255.0;
	return true;
}
bool ComputSURFFea(long *IntegralColor, int wid, int hei, CellFea &fea,int nBand)
{
	int i,c;
	int fea_wid = (fea.x2 - fea.x1 + 1)/2;
	int fea_hei = (fea.y2 - fea.y1 + 1)/2;
	CellFea Smallcell[4];
	Smallcell[0].x1 = fea.x1;
	Smallcell[0].y1 = fea.y1;
	Smallcell[0].x2 = fea.x1 + fea_wid - 1;
	Smallcell[0].y2 = fea.y1 + fea_hei - 1;
	Smallcell[1].x1 = fea.x1 + fea_wid;
	Smallcell[1].y1 = fea.y1;
	Smallcell[1].x2 = fea.x2;
	Smallcell[1].y2 = fea.y1 + fea_hei - 1;
	Smallcell[2].x1 = fea.x1;
	Smallcell[2].y1 = fea.y1 + fea_hei;
	Smallcell[2].x2 = fea.x1 + fea_wid -1;
	Smallcell[2].y2	= fea.y2;
	Smallcell[3].x1 = fea.x1 + fea_wid;
	Smallcell[3].y1 = fea.y1 + fea_hei;
	Smallcell[3].x2 = fea.x2;
	Smallcell[3].y2 = fea.y2;
	for(i = 0; i < 4; i++){
		Smallcell[i].dim = SURF_BINS;
		ComputSmallSURFFea(IntegralColor,wid,hei,Smallcell[i],nBand);
	}
	for(c = 0; c < SURF_BINS; c++){
		fea.fea[c] = 0.0;
	}
	for(c = 0; c < SURF_BINS; c++){
		for(i = 0; i < 4; i++){
			fea.fea[c] += Smallcell[i].fea[c];
		}
	}
	return 1;
}
/////////////////////////////////////////////////////////////////////////////
//�����Ѿ���λ�õ�����������һ�����Haar-like����,ͬʱ������������
//IntegralColor �ĸ�ʽsum(r),sum(g),sum(b)...
// 0  8  9 1
// 4       6
// 5       7
// 2 10 11 3
/////////////////////////////////////////////////////////////////////////////
bool ComputHAARFea(long *IntegralColor, int wid, int hei, CellFea &fea,int nBand)
{

	////////////////////////////////////////////////
	if(!fea.active) return false;	
	fea.orient  = -2;// ��ʼ��

	int h,b;
	long loc[12];
	int fea_wid = fea.x2 - fea.x1 + 1;
	int fea_hei = fea.y2 - fea.y1 + 1;
	
	long up[3],down[3],left[3],right[3];
	long max_dx,max_dy,dx,dy;
	/////////////////////////////////////////////
	//���ݻ���ͼ�������� ���dx,���dy,���|dx|,���|dy|,orientation
	//ע�⵱��Ĵ�С��ż�����ĳ��Ϳ�ʱ
	//����ͨ��8�����Ǽ��㲻��dx��dy�ı���Ҫ12����!
	/////////////////////////////////////////////	
	int pos1 = fea.y1*wid;
	int pos2 = fea.y2*wid;
	
	loc[0] = pos1 + fea.x1;
	loc[1] = pos1 + fea.x2;
	loc[2] = pos2 + fea.x1;
	loc[3] = pos2 + fea.x2;
	
	loc[4] = (fea.y1 + fea.y2) / 2 * wid + fea.x1;
	loc[5] = ((fea.y1 + fea.y2) / 2 + 1) * wid + fea.x1;//+ wid;
	loc[6] = (fea.y1 + fea.y2) / 2 * wid + fea.x2;
	loc[7] = ((fea.y1 + fea.y2) / 2 + 1) * wid + fea.x2; //+ wid;
	loc[8] = pos1 + (fea.x1 + fea.x2) / 2;
	loc[9] = pos1 + (fea.x1 + fea.x2) / 2 + 1;
	loc[10]= pos2 + (fea.x1 + fea.x2) / 2;
	loc[11]= pos2 + (fea.x1 + fea.x2) / 2 + 1; 

	for(h = 0; h < 12; h++){
		loc[h] *= nBand;
	}
	
	for(b = 0; b < nBand; b++){	
	
		up[b]    = IntegralColor[loc[0]+b]+ IntegralColor[loc[6]+b] - 
			       IntegralColor[loc[1]+b]- IntegralColor[loc[4]+b];
		
		down[b]  = IntegralColor[loc[5]+b]+ IntegralColor[loc[3]+b] - 
			       IntegralColor[loc[2]+b]- IntegralColor[loc[7]+b];
		
		left[b]  = IntegralColor[loc[0]+b]+ IntegralColor[loc[10]+b] - 
			       IntegralColor[loc[2]+b]- IntegralColor[loc[8]+b];

		right[b] = IntegralColor[loc[9]+b]+ IntegralColor[loc[3]+b] - 
			       IntegralColor[loc[11]+b]- IntegralColor[loc[1]+b];
	}

	max_dx = max_dy =0;	
	
	//�ڸ�����ɫͨ����ѡ��������Ŀ��ݶ�
	for(b= 0; b<nBand; b++)
	{
		dx = (left[b] - right[b]);
		dy = (up[b]   - down[b] );

		if(abs(dx) > max_dx) max_dx = dx;
		if(abs(dy) > max_dy) max_dy = dy;		
	}
	//2011-11-24    by liang ����ƽ̹����
	int basic_t = THREHOLD*fea_wid*fea_hei/4;
	if(abs(max_dx) < basic_t && abs(max_dy) < basic_t){
		max_dx = 0;
		max_dy = 0;
	}
	//����haar-like ����
	fea.fea[0] = abs(max_dx) / 255.0;
	fea.fea[1] = abs(max_dy) / 255.0;	
	////////////////////////////////////////////////
	//����Haar-like feature ������������,ǰ���Ƿ���
	if(fea_wid != fea_hei)  return true;
	if(max_dy < 0)                                       
	{                                             
		max_dy = -max_dy;                               
		max_dx = -max_dx;                               
	}   
	
	////////////////////////////////////////////////
	//BUG: dy �����С�Ļ�Ӧ����δ����أ�
	if( (abs(max_dx) + abs(max_dy)) / (fea_hei * fea_wid) > 0.2){
		fea.orient = f(max_dx, max_dy);
	}
	else
	{
		fea.orient = -2;
	}

	return true;
}
////////////////////////////////////////////////////////////////////////////////
//�����Ѿ���λ�õ�����������һ�����MSO������haar������ȫ����
//��ͬ��MSOֻ�п������������
//IntegralColor �ĸ�ʽsum(r),sum(g),sum(b)...
// 0  8  9 1
// 4       6
// 5       7
// 2 10 11 3
/////////////////////////////////////////////////////////////////////////////
bool ComputMSOFea(long *IntegralColor, int wid, int hei, CellFea &fea, int nBand)
{
	////////////////////////////////////////////////
	if(!fea.active) return false;	
	fea.orient  = -2;// ��ʼ��

	int fea_wid = fea.x2 - fea.x1 + 1;
	int fea_hei = fea.y2 - fea.y1 + 1;

	//������Ƿ����ֱ�ӷ���!
	if(fea_wid != fea_hei)  return false;      
	else{

		int h,b;
		long loc[12];
	
		long up[3],down[3],left[3],right[3];
		long max_dx,max_dy,dx,dy;

		/////////////////////////////////////////////
		//���ݻ���ͼ�������� ���dx,���dy
		//ע�⵱��Ĵ�С��ż�����ĳ��Ϳ�ʱ
		//����ͨ��8�����Ǽ��㲻��dx��dy�ı���Ҫ12����!
		/////////////////////////////////////////////
	
		int pos1 = fea.y1*wid;
		int pos2 = fea.y2*wid;
		loc[0] = pos1 + fea.x1;
		loc[1] = pos1 + fea.x2;
		loc[2] = pos2 + fea.x1;
		loc[3] = pos2 + fea.x2;
	
		loc[4] = (fea.y1 + fea.y2 ) / 2 * wid + fea.x1;
		loc[5] = ((fea.y1 + fea.y2 ) / 2 + 1) * wid + fea.x1;//+ wid;
		loc[6] = (fea.y1 + fea.y2 ) / 2  * wid + fea.x2;
		loc[7] = ((fea.y1 + fea.y2 ) / 2 + 1) * wid + fea.x2; //+ wid;
		loc[8] = pos1 + (fea.x1 + fea.x2 ) / 2;
		loc[9] = pos1 + (fea.x1 + fea.x2 ) / 2+ 1;
		loc[10] = pos2 + (fea.x1 + fea.x2 ) / 2;
		loc[11] = pos2 + (fea.x1 + fea.x2 ) / 2 + 1; 

		for(h = 0; h < 12; h++) loc[h] *= nBand;
	
		for(b = 0; b < nBand; b++){	
	
			up[b]    = IntegralColor[loc[0]+b]+ IntegralColor[loc[6]+b] - 
			       IntegralColor[loc[1]+b]- IntegralColor[loc[4]+b];
		
			down[b]  = IntegralColor[loc[5]+b]+ IntegralColor[loc[3]+b] - 
			       IntegralColor[loc[2]+b]- IntegralColor[loc[7]+b];
		
			left[b]  = IntegralColor[loc[0]+b]+ IntegralColor[loc[10]+b] - 
			       IntegralColor[loc[2]+b]- IntegralColor[loc[8]+b];

			right[b] = IntegralColor[loc[9]+b]+ IntegralColor[loc[3]+b] - 
			       IntegralColor[loc[11]+b]- IntegralColor[loc[1]+b];
		}

		max_dx = max_dy =0;	
	
		//�ڸ�����ɫͨ����ѡ��������Ŀ��ݶ�
		for(b= 0; b<nBand; b++)	{
			dx = (left[b] - right[b]);
			dy = (up[b]   - down[b] );
	
			if(abs(dx) > max_dx) max_dx = abs(dx);
			if(abs(dy) > max_dy) max_dy = abs(dy);		
		}
		//2011-11-24    by liang ����ƽ̹����
		int basic_t = THREHOLD*fea_wid*fea_hei/4;
		if(abs(max_dx) < basic_t && abs(max_dy) < basic_t){
			max_dx = 0;
			max_dy = 0;
		}
		fea.fea[0] = abs(max_dx) / 255.0;
		fea.fea[1] = abs(max_dy) / 255.0;	

		return true;
	}
}


/////////////////////////////////////////////////////////////////////////////
//�����Ѿ���λ�õ�����������һ����(cell)������
// 0   1
// 2   3
/////////////////////////////////////////////////////////////////////////////
bool ComputHOGFea(long *IntegralImage, int wid, int hei, CellFea &fea,int Channel)
{
	////////////////////////////////////////////////
	if(!fea.active) return false;	
	fea.orient  = -2;// ��ʼ��

	long loc[4];
	int c;
	/////////////////////////////////////////////
	//���ݻ���ͼ�������� ���dx,���dy,���|dx|,���|dy|
	/////////////////////////////////////////////	
	long pos1 = fea.y1 * wid;
	long pos2 = fea.y2 * wid;
	
	loc[0] = pos1 + fea.x1;
	loc[1] = pos1 + fea.x2;
	loc[2] = pos2 + fea.x1;
	loc[3] = pos2 + fea.x2;

	loc[0] *= Channel;
	loc[1] *= Channel;
	loc[2] *= Channel;
	loc[3] *= Channel;

	for(c = 0; c < Channel; c++){		
	fea.fea[c] =( IntegralImage[loc[0]+c] + 	IntegralImage[loc[3]+c]-
					 IntegralImage[loc[1]+c] - 	IntegralImage[loc[2]+c]);
	}
	return true;
}
////////////////////////////////////////////////////////////////////////
//����һ�������ĵ�������(HAAR\HOG\MSO\SURF)
////////////////////////////////////////////////////////////////////////
bool ExtractFeaSet(long *IntegralImage, FeaSet &fea_set, int wid, int hei, int Channel)
{
	int n;

	//���庯����ָ��,һ��������Ӧһ������
	bool (*pFun)(long *IntegralImage, int wid, int hei, CellFea &fea,int Channel);

	if      (fea_set.fea_type==HAAR)  pFun = ComputHAARFea   ; 
	else if (fea_set.fea_type==MSO )  pFun = ComputMSOFea     ;
	else if (fea_set.fea_type==SURF)  pFun = ComputSURFFea;	
	else if (fea_set.fea_type==HOG )  pFun = ComputHOGFea;

	for(n = 0; n < fea_set.cell_num; n++){
		//ͨ������ͼ����һЩ�и���λ�õ�cell�ϵ�����
		pFun(IntegralImage, wid, hei, fea_set.cellfea[n], Channel);	
	}

	//������һ��
	NormFeaSet(fea_set);
	
	return true;
}
/////////////////////////////////////////////////////////////////////////
//ר�Ź�һ�������ĺ���,��Բ�ͬ�����������в�ͬ�Ĺ�һ������
//cell���ֱ�ӹ�һ��
//����BLOCK �����ĸ����ڵĿ���и�ʽ��1,2,3,4..5,6,7,8,..����
/////////////////////////////////////////////////////////////////////////
bool scale_data(double &value, double min, double max, double lower, double upper)
{
	if(value == min) value = lower;
	else if(value == max)	value = upper;
	else
		value = lower + (upper-lower) * 
			(value- min)/ (max-min);	
	if(value > upper) value = upper;
	if(value < lower) value = lower;
	return true;
}

bool NormFeaSet(FeaSet &fea_set)
{
	int n,m,c;
	int bins,norm_bins;
	float sum;
	int adjacent_num= fea_set.adjacent_num;

	//����fea_set�и�������ȡ��������ѡ����Ӧ�Ĺ�һ������
	if(HOG == fea_set.fea_type)
	{
		bins      = HOG_BINS;
		norm_bins = HOG_BINS;
	}
	else if(HAAR == fea_set.fea_type){
		bins     = HAAR_BINS;
		norm_bins= HAAR_BINS; 
	}
	else if(MSO == fea_set.fea_type)
	{
		bins      = MSO_BINS;
		norm_bins = MSO_BINS;
	}
	else if(SURF == fea_set.fea_type)
	{
		bins	= SURF_BINS;
		norm_bins = SURF_BINS;	
	}
		
	//������cell�������ڿ��ϵ���й�һ��
	for(n = 0; n < fea_set.cell_num; n+=adjacent_num){	
		sum = 0;
		for(m = 0; m<adjacent_num; m++){
			for(c = 0; c < bins; c++){
				sum += fea_set.cellfea[m+n].fea[c] *  fea_set.cellfea[m+n].fea[c];
			}
		}
		sum = sqrt(sum);	
		for(m = 0; m<adjacent_num; m++){
			for(c = 0; c < norm_bins; c++){
				fea_set.cellfea[m+n].fea[c] = fea_set.cellfea[m+n].fea[c] / (sum+1.0);
			//	if(fea_set.cellfea[m+n].fea[c] > 0.5)
			//		fea_set.cellfea[m+n].fea[c] = 0.5;
			}
		}	
	}//for(n = 0; n < fea_num; n+=4)

	//����������ά�Ƚ�������
	double lower = -1.0, upper = 1.0;
	double mincut = 0.0, maxcut = 0.92;
	for(n = 0; n < fea_set.cell_num; n+=adjacent_num){
		for(m = 0; m<adjacent_num; m++){
			for(c = 0; c < norm_bins; c++){
				double value = fea_set.cellfea[m+n].fea[c];
				scale_data(value,mincut,maxcut,lower,upper);
				fea_set.cellfea[m+n].fea[c] = value;
			}
		}
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//for LBP
//////////////////////////////////////////////////////////////////////////////////////////////////
/*
 * Calculate the LBP histogram for an cell region on an grey image. This is an
 * optimized version of the basic 8-bit LBP operator. Note that this
 * assumes 4-byte integers. In some architectures, one must modify the
 * code to reflect a different integer size.
 * 
 * img: the image data, an array of rows*columns integers arranged in
 * a horizontal raster-scan order
 * rows: the number of rows in the image
 * columns: the number of columns in the image
 * result: an array of 256 integers. Will hold the 256-bin LBP histogram.
 */

unsigned char UI[256] = 
{0, 1, 2, 3, 4, 58, 5, 6, 7, 58, 58, 58, 8, 58, 9, 10, 11, 
58, 58, 58, 58, 58, 58, 58, 12, 58, 58, 58, 13, 58, 14, 15, 16,
 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 17,
 58, 58, 58, 58, 58, 58, 58, 18, 58, 58, 58, 19, 58, 20, 21, 22,
 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 
 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 23,
 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 24,
 58, 58, 58, 58, 58, 58, 58, 25, 58, 58, 58, 26, 58, 27, 28, 29, 
 30, 58, 31, 58, 58, 58, 32, 58, 58, 58, 58, 58, 58, 58, 33, 58,
 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 34, 58, 
 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58,
 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 35, 36,
 37, 58, 38, 58, 58, 58, 39, 58, 58, 58, 58, 58, 58, 58, 40, 58, 
 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 58, 41, 42,
 43, 58, 44, 58, 58, 58, 45, 58, 58, 58, 58, 58, 58, 58, 46, 47,
 48, 58, 49, 58, 58, 58, 50, 51, 52, 58, 53, 54, 55, 56, 57};

int compab_mask_inc2(unsigned char*p, unsigned char *center)
{
	if(*p > *center) return 1;
	return 0;
}

int ExtractLBPFea(unsigned char* img, int columns, int rows, CellFea &fea)
{
	int leap = columns*predicate;

	unsigned char *cell_pos = img + (columns*fea.y1+fea.x1);
	/*Set up a circularly indexed neighborhood using nine pointers.*/
	unsigned char
		*p0 = cell_pos,
		*p1 = p0 + predicate,
		*p2 = p1 + predicate,
		*p3 = p2 + leap,
		*p4 = p3 + leap,
		*p5 = p4 - predicate,
		*p6 = p5 - predicate,
		*p7 = p6 - leap,
		*center = p7 + predicate;

	unsigned int value;
	int pred2 = predicate << 1;
	int r,c;

	/* Clear result histogram */
	for(c = 0; c < LBP_BINS; c++)
	fea.fea[c] = 0;

	for (r=fea.y1;r<fea.y2-pred2;r++)
	{
		for (c=fea.x1;c<fea.x2-pred2;c++)
		{
			value = 0;			
			/* Unrolled loop */			
			compab_mask_inc(p0,0);
			compab_mask_inc(p1,1);
			compab_mask_inc(p2,2);
			compab_mask_inc(p3,3);
			compab_mask_inc(p4,4);
			compab_mask_inc(p5,5);
			compab_mask_inc(p6,6);
			compab_mask_inc(p7,7);			
			center++;	
			fea.fea[UI[value]]++;
		}
		p0 += pred2;
		p1 += pred2;
		p2 += pred2;
		p3 += pred2;
		p4 += pred2;
		p5 += pred2;
		p6 += pred2;
		p7 += pred2;
		
		center += pred2;
	}

	return 1;
}
///////////////////////////////////////////////////////////////////////////////////
int ExtractLBPFeaSet(unsigned char* img, FeaSet &fea_set, int columns,int rows)
{
	int n,m,c;

	float sum;
	double norm_para = 0.0,basic_t=0.0;
	int adjacent_num= fea_set.adjacent_num;

	for(n = 0; n < fea_set.cell_num; n++)
	{
		//ͨ������ͼ����һЩ�и���λ�õ�cell�ϵ�����
		ExtractLBPFea(img, columns, rows, fea_set.cellfea[n]);		
	}

	//������һ��, L1-norm sqrt �ķ�ʽ
	for(n = 0; n < fea_set.cell_num; n+=adjacent_num){			
		sum = 0;
		for(m = 0; m<adjacent_num; m++){
			//L1-sqrt ��			
			for(c = 0; c < LBP_BINS; c++){
				sum += abs(fea_set.cellfea[m+n].fea[c]);
				//sum += (fea_set.cellfea[m+n].fea[c])*(fea_set.cellfea[m+n].fea[c]);
			}		
			
		}
		sum = sqrt(sum);			
		for(m = 0; m<adjacent_num; m++){
			
			for(c = 0; c < LBP_BINS; c++){
				fea_set.cellfea[m+n].fea[c] = fea_set.cellfea[m+n].fea[c] / (sum+1.0);
			}
		}
	
	}//for(n = 0; n < fea_num; n+=4)
	return 1;	
}
//////////////////////////////////////////////////////////////////////////
//��һ������д���ض����ı��ļ�,begin_index��Ϊ���ʾ׷��������Ϊ��дһ������
//////////////////////////////////////////////////////////////////////////
bool WriteFeaToFile(CellFea fea, char path[255],int label)
{
	ofstream SaveFile(path, ios::app);
    SaveFile<<label<<" ";

	for(int d = 0; d < fea.dim; d++){
	SaveFile<<d+1<<":"<<fea.fea[d]<<" ";
	}

	//SaveFile<<"\n";

	return 1;
}
//////////////////////////////////////////////////////////////////////////
//��һ�����������ڵ���������д���ض����ı��ļ�
//////////////////////////////////////////////////////////////////////////
bool WriteFeaSetBySVMFormat(FeaSet fea_set,char path[255], int label,FILE *fp)      //2011-0727 By Liang
{
	int n,d,index;
	index = 0;
	fprintf(fp,"%d ",label);
	for( n = 0; n < fea_set.cell_num; n++){		
		for( d = 0; d < fea_set.cellfea[d].dim; d++){
			fprintf(fp,"%d:%.8g ",++index,fea_set.cellfea[n].fea[d]);
		}
	}
	fprintf(fp,"\n");
	return true;
}

bool AddFeaBySVMFormat(double fea[], int start_index, int add_fea_num,FILE *fp)
{
	int i,j;
	for(i=start_index+1,j = 0; i < (start_index+add_fea_num+1); i++,j++)
		fprintf(fp,"%d:%.8g ",i,fea[j]);
	return true;
}

bool GetWindowFea(FeaSet fea_set, double fea[])
{
	int n,d;
	int index;
	for( n = 0,index = 0; n < fea_set.cell_num; n++){
		for( d = 0; d < fea_set.cellfea[n].dim; d++){
			fea[index++] = fea_set.cellfea[n].fea[d];
		}	
	}
	return true;
}
