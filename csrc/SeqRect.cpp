#include <stdio.h>
#include <ctime>
#include "cv.h"
#include "highgui.h"
#include "SeqRect.h"
 

CSeqRect::CSeqRect()
{
	seqNode = CreateSeqRect();
	this->FeaWidth = INFINIT;
	this->FeaHeight = INFINIT;
	firstNode = true;
	calcArea();
	bug=0;
}
CSeqRect::CSeqRect(int fea_width,int fea_height)
{
	seqNode = CreateSeqRect();
	this->FeaWidth = fea_width;
	this->FeaHeight = fea_height;
	firstNode = true;
	calcArea();
	bug=0;
}
CSeqRect::CSeqRect(CvRect rect)
{
	seqNode = CreateSeqRect(rect);
	this->FeaWidth = INFINIT;
	this->FeaHeight = INFINIT;
	firstNode = true;
	calcArea();
	bug=0;
}
CSeqRect::~CSeqRect()
{
	free(this->seqNode);
}
SeqRect *CSeqRect::CreateSeqRect()
{
	SeqRect *r ;
	r = (SeqRect*)malloc(sizeof(SeqRect));
	r->next = NULL;
	r->out_next = NULL;
	r->out_prev = NULL;
	r->first_out_layer = NULL;//��ʱû�õ�
	r->prev = NULL;
	return r;
}
SeqRect *CSeqRect::CreateSeqRect(CvRect rect)
{
	SeqRect *r;
	r = (SeqRect*)malloc(sizeof(SeqRect));
	r->rect = rect;
	r->next = NULL;
	r->out_next = NULL;
	r->out_prev = NULL;
	r->first_out_layer = NULL;
	r->prev = NULL;
	return r;
}
void CSeqRect::AddNode(CvRect rect)									//��ӽڵ����Ҫ�������������ڴ˴�����//Ҫ���ڲ��
{ 
	SeqRect *newRect =  CreateSeqRect(rect);						//Ҫ��ӵ��½ڵ�

	SeqRect *prect;		//��ʱ�����ڵ�
	SeqRect *oldrect;
	prect = oldrect = this->seqNode;
	bool isAbnormal = IsRectAbnormal(rect);
	while(prect != NULL && !firstNode)
	{ 
		if(isAbnormal)
		{	
			break;
		}
		if(IsRect1In2(prect->rect,rect))
		{	 
			InsertOutEnd(prect,newRect);				//�ڸ���������ڵ�
			break;
		}
		else if(IsRect1In2(rect,prect->rect))
		{
			ReplaceBy2(prect,newRect);					//�滻�����ϵĽڵ㣬�������Ľڵ��Ƶ�����
			break;
		} 
		else				//������������ڵ�
		{
			oldrect = prect;
			prect = prect->next;
			if(prect == NULL)
			{
				InsertInineEnd(oldrect,newRect);
			}
		}
	}
	if(firstNode && !isAbnormal)							//�öδ���ִֻ��һ��
	{
		this->seqNode->rect = rect;
		firstNode = false;
	}
}
void CSeqRect::AddNode(CvRect rect,bool simple)							//�����������	//��Region����		
{ 
	SeqRect *newRect =  CreateSeqRect(rect);					 

	SeqRect *prect;		//��ʱ�����ڵ�
	SeqRect *oldrect;
	prect = oldrect = this->seqNode;
	bool isAbnormal =IsRectAbnormal(rect);// (rect.width <= 7);				//��Ҫͨ�û�
	//printf("xx==>%d,%d\n",this->FeaWidth,this->FeaHeight);
	while(prect != NULL && !firstNode)
	{ 
		if(isAbnormal)
		{ //printf("ax\n(%d,%d,%d,%d)\n",rect.x,rect.y,rect.width,rect.height);
			break;
		}
		//printf("a1,(%d,%d,%d,%d)\n",rect.x,rect.y,rect.width,rect.height);
		if(IsRect1In2(prect->rect,rect,simple))
		{//printf("a2\n");
			bool replaceFirst = false;
			if(prect == this->seqNode)
				replaceFirst = true;
			ReplaceBy2(prect,newRect);
			if(replaceFirst)
				this->seqNode = newRect;
			break;
		}
		else if(IsRect1In2(rect,prect->rect,simple))
		{  //printf("a3\n");
			InsertOutEnd(prect,newRect);
			break;
		} 
		else				//zhuliancha
		{	//printf("a4\n");
			oldrect = prect;
			prect = prect->next;
			if(prect == NULL)
			{//printf("null\n");
				InsertInOrder(this->seqNode,newRect);
			}
		}
	} 
	if(firstNode && !isAbnormal)
	{  //printf("a8\n(%d,%d,%d,%d)\n",rect.x,rect.y,rect.width,rect.height);
		this->seqNode->rect = rect;
		firstNode = false;
	}
	while(this->seqNode->prev)
	{ //printf("a9\n");
		this->seqNode = this->seqNode->prev;
	}
}
/////@@@@@@@@@@@@@@@@����1
bool CSeqRect::IsRectAbnormal(CvRect rect)
{	
	bool positive = (((rect.x -1) < 0) || ((rect.y -1) < 0 ));					//rect���궼������
	bool outImage = ( (rect.x + rect.width  +2 >= this->FeaWidth) || (rect.y + rect.height +2 >= this->FeaHeight) );   //��ͼ��Χ��
	bool min2limit = (rect.width < 10 && rect.height < 10 );
	bool minlimit = (rect.height < 2 || rect.width <2);
	//bool areax = ((rect.width * rect.height) * 1.0 / this->_area) > 0.1;
	return (minlimit ||min2limit || outImage || positive );
	
#if 0

	

	bool radio_abnormal = false;
	if( rect.width >=PADDING_OFFSET && rect.height >=PADDING_OFFSET )
		radio_abnormal = (rect.width > rect.height ? (rect.width / rect.height) : (rect.height / rect.width) ) > 2;		//�����С��2������
	else
		radio_abnormal = true;
	bool area_ratio = (calcRatio(rect)-RATIO_AREA) >= 0;				//�������
#endif
	return false;
	//return (positive || inImage );
}
bool CSeqRect::IsRect1In2(CvRect rect1,CvRect rect2)					//rect1�Ƿ������rect2
{																		//��Ҫ���Ǽ�С�߽����

	bool x_con = (rect1.x >= rect2.x);
	bool y_con = (rect1.y >= rect2.y);
	bool width_con = (rect1.x + rect1.width <= rect2.x + rect2.width  );
	bool height_con = (rect1.y + rect1.height <= rect2.y + rect2.height );
/*
	bool x_con = (rect1.x >= rect2.x + MARGIN_OFFSET);
	bool y_con = (rect1.y >= rect2.y + MARGIN_OFFSET);
	bool width_con = (rect1.x + rect1.width  + MARGIN_OFFSET<= rect2.x + rect2.width);
	bool height_con = (rect1.y + rect1.height  + MARGIN_OFFSET <= rect2.y + rect2.height);
*/	//��ǰ������ž���
	//������ ������ţ���಻��Ͳ��������
	//��ô�����಻��
	// =>��Sizer���洦��

	return x_con && y_con && width_con && height_con;
}

bool CSeqRect::IsRect1In2(CvRect rect1,CvRect rect2,bool simple)					//rect1�Ƿ������rect2
{																		//��Ҫ���Ǽ�С�߽����

	bool x_con = (rect1.x >= rect2.x);
	bool y_con = (rect1.y >= rect2.y);
	bool width_con = (rect1.x + rect1.width <= rect2.x + rect2.width);
	bool height_con = (rect1.y + rect1.height <= rect2.y + rect2.height);

	return x_con && y_con && width_con && height_con;
}
void CSeqRect::InsertInOrder(SeqRect *inlineRect,SeqRect *newRect)	//������û�з��ְ�����ϵ�ģ�����rect��xֵ��С����������
{
	SeqRect *prect,*oldrect;
	oldrect = prect = inlineRect;
 	while(prect)
	{  
		if(prect->rect.x > newRect->rect.x)
			break; 
		oldrect = prect;
		prect = prect->next;
	}
	if(prect == NULL)
	{  
		oldrect->next = newRect;
		newRect->prev = oldrect;
		newRect->out_next = NULL;
		newRect->out_prev = NULL;
		newRect->next = NULL;
		return;
	} 				//������ǽ�β��ô��֮ǰ�ӽڵ�
	newRect->prev = prect->prev;
	if(prect->prev)
		prect->prev->next = newRect;	//˫����������ˣ�
	newRect->next = prect; 
	prect->prev = newRect;

	newRect->out_next = NULL;
	newRect->out_prev = NULL;
}
void CSeqRect::InsertInineEnd(SeqRect *inlineRect,SeqRect *newRect)	//������û�з��ְ�����ϵ�ģ��ӵ���β
{
	SeqRect *prect;
	prect = inlineRect;
 	while(prect->next!=NULL)
	{
		prect = prect->next;
	} 
	prect->next = newRect;
	newRect->prev = prect;
	newRect->out_next = NULL;
	newRect->out_prev = NULL;
	newRect->next = NULL;
}
void CSeqRect::InsertOutEnd(SeqRect *inlineRect,SeqRect *newRect)		//�����������ϵľ��Σ�����������
{
	SeqRect *prect;
	prect = inlineRect;
	newRect->out_next = NULL;
	newRect->next = NULL;
	newRect->prev = NULL;
	while(prect->out_next!=NULL)
	{
		prect = prect->out_next;
	}
	prect->out_next = newRect;
	newRect->out_prev = prect;
	if(IsRect1In2(newRect->rect,inlineRect->rect))
	{
		if(inlineRect->first_out_layer == NULL)
		{
			inlineRect->first_out_layer = newRect;
		}
		else if(IsRect1In2(newRect->rect,inlineRect->first_out_layer->rect))
		{
			inlineRect->first_out_layer = newRect;
		}
	}
}

void CSeqRect::ReplaceBy2(SeqRect *inlineRect,SeqRect *newRect)		//��������������滻�������ϵĸýڵ�
{
	if(inlineRect->next != NULL)
		inlineRect->next->prev = newRect;
	if(inlineRect->prev != NULL)
		inlineRect->prev->next = newRect;

	newRect->next = inlineRect->next;
	newRect->prev = inlineRect->prev;

	inlineRect->next = inlineRect->prev = NULL;
	inlineRect->out_prev = newRect;
	newRect->out_next = inlineRect;
	newRect->out_prev = NULL;
	//inlineRect = newRect;
}
void CSeqRect::calcArea()
{
	this->_area = this->FeaWidth * this->FeaHeight;
}
double CSeqRect::calcRatio(CvRect rect)
{
	double ratio = ((rect.width*rect.height * 1.0) / (this->_area*1.0));
	return ratio;
}

	