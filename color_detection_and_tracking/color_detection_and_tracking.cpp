// color_detection_and_tracking.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include <opencv/cv.h>
#include <opencv/highgui.h>

IplImage* imgTracking;
int lastX = -1;
int lastY = -1;


//int maxrendahtreshold = 0124;
//int slideposisirendah = 150;

//int maxtinggitreshold = 1024;
//int slideposisitinggi = 250;

int lowH=0;
int lowS=0;
int lowV=0;

int upH=180;
int upS=256;
int upV=256;


//Fungsi treshold gambar HSV dan membuat gambar menjadi biner
IplImage* GetThresholdedImage(IplImage* imgHSV){       
    IplImage* imgThresh=cvCreateImage(cvGetSize(imgHSV),IPL_DEPTH_8U, 1);
    cvInRangeS(imgHSV, cvScalar(lowH,lowS,lowV), cvScalar(upH,upS,upV), imgThresh); 
    return imgThresh;
}
void settingWindows(){
	cvNamedWindow("Video");
	cvNamedWindow("Video2");
	cvNamedWindow("Konfigurasi");

	cvCreateTrackbar("LowH","Konfigurasi",&lowH,180,NULL);
	cvCreateTrackbar("LowS","Konfigurasi",&lowS,256,NULL);
	cvCreateTrackbar("LowV","Konfigurasi",&lowV,256,NULL);
	
	cvCreateTrackbar("UpH","Konfigurasi",&upH,180,NULL);
	cvCreateTrackbar("UpS","Konfigurasi",&upS,256,NULL);
	cvCreateTrackbar("UpV","Konfigurasi",&upV,256,NULL);
}

void trackObject(IplImage* imgThresh){
   
	// Kalkulasi momen untuk image yg telah di konvert
    CvMoments *moments = (CvMoments*)malloc(sizeof(CvMoments));
    cvMoments(imgThresh, moments, 1);
    double moment10 = cvGetSpatialMoment(moments, 1, 0);
    double moment01 = cvGetSpatialMoment(moments, 0, 1);
    double area = cvGetCentralMoment(moments, 0, 0);

     // jika area <1000, saya memperhitungkan tidak ada objek di gambar karena akan mengganggu, dan areanya tidak nol
	if(area>1000){
		// Menghitung posisi objek
        int posX = moment10/area;
        int posY = moment01/area;        
        
       if(lastX>=0 && lastY>=0 && posX>=0 && posY>=0)
        {
			// Menggambar garis berwarna sesuai dengan HSV di Konfigurasi, dari titik pertama ke titik terakhir
            cvLine(imgTracking, cvPoint(posX, posY), cvPoint(lastX, lastY), cvScalar(lowH,lowS,lowV), 4);
        }

         lastX = posX;
        lastY = posY;
    }

     free(moments); 
} 


int main(){
   
      CvCapture* capture =0;       
      capture = cvCaptureFromCAM(0);
      if(!capture){
         printf("Capture failure\n");
         return -1;
      }
      IplImage* frame=0;
	  
      frame = cvQueryFrame(capture);           
      if(!frame) return -1;
   
	  //membuat gambar yang kosong dan menjalankan imgTracking yang memiliki ukuran CvWindow yang sama dari yang asli
     imgTracking=cvCreateImage(cvGetSize(frame),IPL_DEPTH_8U, 3);
     cvZero(imgTracking); //menyembunyikan warna yang tidak terdeteksi menjadi hitam
	 //cvCreateTrackbar("Low Threshold", "Video2", &slideposisirendah, maxrendahtreshold, onLowThresholdSlide);
 
	// Create the high threshold slider
   	//cvCreateTrackbar("High Threshold", "Video2", &slideposisitinggi, maxtinggitreshold, onHighThresholdSlide);
	 settingWindows();
	 cvNamedWindow("Konfigurasi");
     cvNamedWindow("Video");
     cvNamedWindow("Video2",CV_WINDOW_AUTOSIZE);

	 //mengulangi  setiap frame dari video
      while(true){

            frame = cvQueryFrame(capture);           
            if(!frame) break;
            frame=cvCloneImage(frame); 
            
            cvSmooth(frame, frame, CV_GAUSSIAN,3,3);//menghaluskan gambar yg asli menggunakan kernel Gaussian

            IplImage* imgHSV = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3); 
            cvCvtColor(frame, imgHSV, CV_BGR2HSV); //merubah format warna RGB ke Grayscale
            IplImage* imgThresh = GetThresholdedImage(imgHSV);
          
            cvSmooth(imgThresh, imgThresh, CV_GAUSSIAN,3,3);//menghaluskan gambar biner menggunakan kernel gaussian
            
			//membuat posisi jalur objek
           trackObject(imgThresh);

		   //Menambahkan jalur gambar dan frame
           cvAdd(frame, imgTracking, frame);

           cvShowImage("Video", imgThresh);           
           cvShowImage("Video2", frame);
           
       
           cvReleaseImage(&imgHSV);
           cvReleaseImage(&imgThresh);            
           cvReleaseImage(&frame);

            int c = cvWaitKey(10);
            
            if((char)c==27 ) break;      
      }

      cvDestroyAllWindows() ;
      cvReleaseImage(&imgTracking);
      cvReleaseCapture(&capture);     

      return 0;
}

