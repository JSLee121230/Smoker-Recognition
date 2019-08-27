#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv/cv.h"

#include <iostream>
#include <stdio.h>
#include<math.h>


char* ASC ( double DEC);

int motion_Threshold =7;

CvRect smoker_face;

int MV_up ;
int MV_down ;

int up_c=0;
int down_c=0;
int smoke_c=0;


int block = 9 ;
int search =7 ;
int sum ;
int SAD ;

int Detect=0;
int FACE=0; 
int SMOKE=0;
int UP=0;
int DOWN=0;

int P_SMOKE=0;
int P_UP=0;
int P_DOWN=0;

int DOWN_NUM=0;
int UP_NUM=0;
int SMOKE_NUM=0;

CvPoint reference_point ;
CvPoint current_point ;
CvPoint motion_vector ;
CvPoint* vector = &motion_vector;

struct motion{
	int start_x;
	int start_y;
	int end_x;
	int end_y;
};



int i;
int j;
int a;
int b;
int x;
int y;

CvRect *face ;


int main() {
	IplImage *frame;
	int num=0;
	int frame_num=0;




	/* CvCapture* capture = cvCaptureFromCAM(0); */
	CvCapture* capture = cvCreateFileCapture("D:/1.avi");

	frame = cvQueryFrame(capture);
	frame = cvQueryFrame(capture);


	IplImage *dst =  cvCreateImage(cvGetSize(frame),8,3);

	IplImage *smoker =  cvCreateImage(cvGetSize(frame),8,3);

	IplImage *gray =  cvCreateImage(cvGetSize(frame),8,1);

	IplImage *reference =  cvCreateImage(cvGetSize(frame),8,1);

	IplImage *YUV = cvCreateImage(cvGetSize(frame),8,3);

	IplImage *skin =  cvCreateImage(cvGetSize(frame),8,1);

	IplImage *Sub_plane = cvCreateImage(cvGetSize(frame),8,1);
	
	IplImage *copy_image = cvCreateImage(cvGetSize(frame),8,1);

	IplImage *copy2_image = cvCreateImage(cvGetSize(frame),8,1);

	IplImage * reference_frame = cvCreateImage(cvGetSize(frame),8,1);


	// 하르 파라미터 //
	int value =5;

	CvHaarClassifierCascade* cascade_face =0;

	const char *classifer = "C://opencv/sources/data/haarcascades/haarcascade_frontalface_alt2.xml";

	cascade_face =(CvHaarClassifierCascade*) cvLoad(classifer,NULL,NULL,NULL);

	if(!cascade_face){
		printf("error : cascade error!!");
		return -1;
	}

	CvMemStorage* storage_face =0;
	storage_face = cvCreateMemStorage(0);

	if(!storage_face){
		printf("error : storage error!!");
		return -1;
	}
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0f, 1.0f, 0, 2, 8);
	char face_number[10]="1 face";


	CvRect *r =0;
	int i =0;

	CvSeq *faces=0;



	//smoke pixel
	CvMemStorage *point_mem;
	point_mem = cvCreateMemStorage(0);
	CvSeq *point ;
	point = cvCreateSeq(0,sizeof(CvSeq),sizeof(CvPoint),point_mem);

	CvMemStorage *pixel_mem;
	pixel_mem = cvCreateMemStorage(0);
	CvSeq *pixel ;
	pixel = cvCreateSeq(0,sizeof(CvSeq),sizeof(CvPoint),pixel_mem);

	int start_x =0;
	int start_y =0;
	int end_x =0;
	int end_y =0;

	int smoke_size =0;

	int window =13;

	motion range;

	while(frame = cvQueryFrame(capture)) {
		frame_num++;

		if(Detect < motion_Threshold)
		{
			
		cvCopy(frame,dst);

		cvCvtColor(frame,gray,CV_RGB2GRAY);
	
		// face Dection 
		if(frame_num%20){

			faces = cvHaarDetectObjects(gray,cascade_face,storage_face,1.2,2,CV_HAAR_DO_CANNY_PRUNING,cvSize(80,80));



			if(faces->total != 0 ){
				i=0;

				r=(CvRect*) cvGetSeqElem(faces,i);

				face = r;
				if(faces->total<10)
					face_number[0] = 48+(char)i+1; // 찾은 얼굴에 번호 입력

				cvRectangle(dst,cvPoint(r->x,r->y),cvPoint(r->x+r->width,r->y+r->height), cvScalar(0,0,255),3,CV_AA,0);

				cvPutText(dst,&face_number[0],cvPoint(r->x,r->y-3),&font,cvScalar(0,0,255));	

				start_x =face->x - face->width;

				start_y =face->y - face->height;

				end_x =face->x+(face->width*2);

				end_y =face->y+(face->height);

				cvRectangle(dst,cvPoint(start_x,start_y),cvPoint(end_x,end_y), cvScalar(0,255,255),3,CV_AA,0);

				FACE = 1;

			}	
		}


		cvCvtColor(frame,YUV,CV_BGR2YCrCb);

		cvInRangeS(YUV,cvScalar(0,132,76),cvScalar(255,174,128),skin);

		// motion range//
		
		MV_up=0;
		MV_down=0; 

		range.start_x = start_x; 
		range.start_y = end_y;
		range.end_x = end_x;
		range.end_y = gray->height;

		for(j = range.start_y; j < range.end_y ; j +=block)
			for( i=range.start_x; i < range.end_x; i +=block)
			{ 

				SAD = 255 * 64 ;
				reference_point = cvPoint(i,j);
				for( b = -search ; b < search ; b++){
					for(a = - search ; a < search ; a++){

						sum =0 ;
						for(int y = 0 ; y < block ; y++ ){
							for(int x = 0 ; x <block ; x ++){

								int index_R = (j + y + b) * gray->width + (i + x + a) ;
								int index_C = (j + y) * gray->width + (i + x) ; 

								sum += abs((unsigned char)gray->imageData[index_C] - (unsigned char)reference->imageData[index_R]) ;


							}
						}
						if(b==-search && a==-search)
						{
							SAD = sum;
							reference_point = cvPoint(i+a,j+b);
							current_point = cvPoint(i,j);
							motion_vector = cvPoint(a,b);
						}
						if(SAD > sum){
							SAD = sum;
							reference_point = cvPoint(i+a,j+b);
							current_point = cvPoint(i,j);
							motion_vector = cvPoint(a,b);
						}


					}
				}
				vector = &motion_vector;

				if(vector->y > 3 ){

					vector = &current_point;

					int index = vector->y * gray->width + vector->x;

					if((unsigned char)skin->imageData[index]!=0){

// 						cvLine(dst,current_point,reference_point,cvScalar(0,0,255),1,8);
// 
// 						cvRectangle(dst,reference_point,reference_point,cvScalar(0,0,255),2,8,0);
// 
// 						cvRectangle(dst,current_point,cvPoint(vector->x+block,vector->y+block),cvScalar(255,0,0),2,8,0);

						MV_up++;
						
					}
				}
				else if(vector->y < -3){
					vector = &current_point;

					int index = vector->y * gray->width + vector->x;

					if((unsigned char)skin->imageData[index]!=0){

// 						cvLine(dst,current_point,reference_point,cvScalar(0,0,255),1,8);
// 
// 						cvRectangle(dst,reference_point,reference_point,cvScalar(0,0,255),2,8,0);
// 
// 						cvRectangle(dst,current_point,cvPoint(vector->x+block,vector->y+block),cvScalar(0,255,0),2,8,0);

						MV_down++;

					}
				}
			}
			if(MV_up + MV_down > 40){

			if(MV_up > MV_down){
 	//			printf("UP ! \n");
				UP = 1;
				DOWN =0;
			
			}
			else if(MV_up < MV_down){
	//			printf( "down ! \n");
				DOWN = 1 ;
				UP =0;
			
				}

			}
		

			cvCopy(gray,reference);

			//		Smoke detection!!

			cvSub(gray,reference_frame,Sub_plane);

			cvSmooth(Sub_plane,Sub_plane,2,7,7,0,0);

			cvCreateTrackbar("TH","Test",&value,255,0);

			cvThreshold(Sub_plane,Sub_plane,value,255,CV_THRESH_BINARY);
		
			cvClearSeq(point);

			cvClearSeq(pixel);

			for(int j = start_y  ; j < Sub_plane->height ; j ++)

				for(int i = start_x ; i <  Sub_plane->width ; i ++){

					int index = j * Sub_plane->widthStep + i ;

					if((unsigned char)Sub_plane->imageData[index]!=0)

						Sub_plane->imageData[index] -= (unsigned char)skin->imageData[index];

				}


				if(start_x<0)

					start_x =0;

				if(start_y<0)

					start_y =0;

				for(int j = start_y ; j < end_y ; j +=window)

					for(int i = start_x ; i <  end_x ; i +=window){

						int count =0;

						for( int y =0; y < window ; y ++ ){

							for(int x =0; x< window ; x++){

								int index = (j+y) * Sub_plane->widthStep + (i+x) ;

								if((unsigned char)Sub_plane->imageData[index]!=0)

									count++;
							}
							if(count> window*window*0.9){

								cvRectangle(dst,cvPoint(i,j),cvPoint(i+window,j+window),cvScalar(255,0,0),2,8,0);

								/*cvSeqPush(point,&cvPoint(i,j));*/
							}
						}
					}

					if(point->total!=0){
							for( int i = 0 ; i < point->total ; i++){
								CvPoint* C_pixel = (CvPoint*)cvGetSeqElem(point,i);
	
	
								for(int j = 0; j < point -> total ; j++){
									CvPoint* N_pixel = (CvPoint*)cvGetSeqElem(point,j);
	
									int s =  N_pixel-> x- C_pixel->x ;
									int v =  N_pixel->y - C_pixel->y;
	
									if( s==window && v==0 ){
										cvSeqPush(pixel,C_pixel);
	
									}
	
									else if(  s==0 && v==window  ){
										cvSeqPush(pixel,C_pixel);
	
									}
								}
							}
						}
	
						if(pixel->total> smoke_size )  {
	
							for( int i = 0 ; i <pixel->total ; i ++){
								CvPoint* r= (CvPoint*)cvGetSeqElem(pixel,i);
								cvRectangle(dst,cvPoint(r->x,r->y),cvPoint(r->x+window,r->y+window),cvScalar(255,255,0),2,8,0);
	
							}
	
		//					printf("smoke!!\n");

						SMOKE =1;

					}
					else
					{
						cvCopy(copy2_image,reference_frame);
						SMOKE =0 ;
						
					}

					//cvShowImage("Test",Sub_plane);
					cvShowImage("dst",dst); 

					/*cvCopy(copy_image,copy2_image);*/
				 if(frame_num % 20)
					cvCopy(gray,copy2_image);

				//	cvShowImage("skin",skin);


			if(cvWaitKey(33) >= 27)
				break;
			
			cvShowImage("src",frame);
			cvShowImage("dst",dst);
			printf("     UP   DOWN SMOKE     up-C down-C smoke-c\n ");
			printf("%5d%5d%5d     %5d%5d%7d\n",UP,DOWN,SMOKE,up_c,down_c,smoke_c);


			if(P_DOWN^DOWN==1){

				DOWN_NUM++;
				
				if(DOWN_NUM%2 == 0)
					down_c++;
			}
			if(P_UP^UP==1){

				UP_NUM++;
				
				if(UP_NUM%2 == 0)
					up_c++;
			}
			if(P_SMOKE^SMOKE==1){

				SMOKE_NUM++;

				if(SMOKE_NUM%2 == 0)
					smoke_c++;
			}

			P_DOWN =DOWN;
			P_UP = UP;
			P_SMOKE = SMOKE;




			Detect = FACE * (up_c) * (down_c) * (smoke_c) ;
			
			if(Detect > motion_Threshold)
			{
				cvSetImageROI(frame,*face);
				cvSetImageROI(smoker,*face);

				cvCopy(frame,smoker);
				cvSaveImage("smoker.jpg",frame);
				cvResetImageROI(frame);
				cvResetImageROI(smoker);
				
			}

			}





		else{

			IplImage *PLA = cvLoadImage("smoker.jpg");

			cvShowImage("smoker",PLA);

			cvShowImage("src",frame);

			cvCopy(frame,dst); 
			
			cvCvtColor(frame,gray,CV_RGB2GRAY);

			// face Dection 

			CvSeq *faces=0;
			faces=cvHaarDetectObjects(gray,cascade_face,storage_face,1.2,2,CV_HAAR_DO_CANNY_PRUNING,cvSize(80,80));

			CvFont font;
			cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 1.0f, 1.0f, 0, 2, 8);
			char face_n_smoker[10]="Smoker";


			if(faces->total!=0){

				CvRect *r =0;

				int i =0;

				r=(CvRect*) cvGetSeqElem(faces,i);

				face = r;

				cvRectangle(dst,cvPoint(r->x,r->y),cvPoint(r->x+r->width,r->y+r->height), cvScalar(0,0,255),3,CV_AA,0);

				cvPutText(dst,&face_n_smoker[0],cvPoint(r->x,r->y-3),&font,cvScalar(0,0,255));	


				FACE = 1;

			}	

			if(cvWaitKey(33) >= 27)
				break;
			printf("SMOKER! \n ");
			
			cvShowImage("dst",dst);

			

			}
	}
	cvReleaseCapture(&capture);

	return 0; 
}



