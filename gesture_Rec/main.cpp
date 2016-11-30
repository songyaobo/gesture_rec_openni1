#include <QCoreApplication>
#include <stdlib.h>
#include <iostream>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <XnCppWrapper.h>
#include <vector>
#include <math.h>

using namespace std;

vector<XnFloat> start_x,start_y,start_z;
static int i=0;
static float eps=0.0000000000000001;

void initial(){
    start_x.clear();
    start_y.clear();
    start_z.clear();
    i=0;
}

ostream& operator <<(ostream& out,const XnPoint3D& rPoint){
    out<<"("<<rPoint.X<<","<<rPoint.Y<<","<<rPoint.Z<<")";
    return out;
}

struct SNode
{
  char*                sGestureToUse;
  char*                sGestureToPress;
  xn::DepthGenerator   mDepth;
  xn::HandsGenerator   mHand;
  xn::GestureGenerator mGesture;
  xn::ImageGenerator mImage;
  IplImage* drawPadImg;
};

void XN_CALLBACK_TYPE GestureRecognized( xn::GestureGenerator& generator,
                                         const XnChar* strGesture,
                                         const XnPoint3D* pIDPosition,
                                         const XnPoint3D* pEndPosition,
                                         void* pCookie )
{
  SNode* pNodes = ((SNode*)pCookie);
  IplImage* refimage=pNodes->drawPadImg;
  CvFont font;
  cvInitFont( &font, CV_FONT_VECTOR0,1, 1, 0, 3, 5);

  if( strcmp( strGesture, pNodes->sGestureToPress ) == 0 )
  {
    cout << "click" << endl;
    memset(refimage->imageData,255,640*480*3);
    cvPutText(refimage, "click" , cvPoint(100, 100), &font, CV_RGB(0,255,255));
  }
  else if( strcmp( strGesture, pNodes->sGestureToUse ) == 0 )
  {
    cout << "Start moving" << endl;
    generator.RemoveGesture(strGesture);
    pNodes->mHand.StartTracking( *pIDPosition );
  }
}

void XN_CALLBACK_TYPE GProgress( xn::GestureGenerator &generator,
                                 const XnChar *strGesture,
                                 const XnPoint3D *pPosition,
                                 XnFloat fProgress,
                                 void *pCookie )
{
}

void XN_CALLBACK_TYPE HandCreate( xn::HandsGenerator& generator,
                                  XnUserID nId,
                                  const XnPoint3D* pPosition,
                                  XnFloat fTime,
                                  void* pCookie )
{
  SNode* pNodes = ((SNode*)pCookie);
  IplImage* refimage=pNodes->drawPadImg;
  CvFont font;
  cvInitFont( &font, CV_FONT_VECTOR0,1, 1, 0, 3, 5);

  memset(refimage->imageData,255,640*480*3);
  cvPutText(refimage, "New Hand" , cvPoint(100, 100), &font, CV_RGB(255,255,255));
  cout << "New Hand: " << nId << " detected!\n";

  pNodes->mGesture.AddGesture( pNodes->sGestureToPress, NULL );
}

void XN_CALLBACK_TYPE HandUpdate( xn::HandsGenerator& generator,
                                  XnUserID nId,
                                  const XnPoint3D* pPosition,
                                  XnFloat fTime,
                                  void* pCookie )
{
    //图形化界面
    SNode* pNodes = ((SNode*)pCookie);
    IplImage* refimage=pNodes->drawPadImg;
    CvFont font;
    cvInitFont( &font, CV_FONT_VECTOR0,1, 1, 0, 3, 5);

  i++;
  start_x.push_back( pPosition->X);
  start_y.push_back(pPosition->Y);
  start_z.push_back(pPosition->Z);
  if(start_x.size()>5&&abs(start_x.at(i-1)-start_x.at(i-2))==0&&abs(start_y.at(i-1)-start_y.at(i-2))==0){
      XnFloat k=(start_y.at(i-1)-start_y.at(0))/((start_x.at(i-1)-start_x.at(0))+eps);
      if(k!=0){
          if(abs(k)<1){
              if((start_x.at(i-1)-start_x.at(0))>0){
                  initial();
                  cout<<"++++horizontal++++"<<endl;
                  memset(refimage->imageData,255,640*480*3);
                  cvPutText(refimage, "++++horizontal++++" , cvPoint(100, 100), &font, CV_RGB(0,0,255));
              }
              else {
                  initial();
                  cout<<"----horizontal----"<<endl;
                  memset(refimage->imageData,255,640*480*3);
                  cvPutText(refimage, "----horizontal----" , cvPoint(100, 100), &font, CV_RGB(0,255,0));
              }
          }
          else{
               if((start_y.at(i-1)-start_y.at(0))>0){
                   initial();
                   cout<<"++++vertical++++"<<endl;
                   memset(refimage->imageData,255,640*480*3);
                   cvPutText(refimage, "++++vertical++++" , cvPoint(100, 100), &font, CV_RGB(255,0,0));
               }
              else{
                   initial();
                   cout<<"----vertical------"<<endl;
                   memset(refimage->imageData,255,640*480*3);
                   cvPutText(refimage, "----vertical------" , cvPoint(100, 100), &font, CV_RGB(255,0,255));
               }
          }
      }
  }
}

void XN_CALLBACK_TYPE HandDestroy( xn::HandsGenerator& generator,
                                   XnUserID nId,
                                   XnFloat fTime,
                                   void* pCookie )
{ 
  SNode* pNodes = ((SNode*)pCookie);
  IplImage* refimage=pNodes->drawPadImg;
  CvFont font;
  cvInitFont( &font, CV_FONT_VECTOR0,1, 1, 0, 3, 5);

  memset(refimage->imageData,255,640*480*3);
  cvPutText(refimage, "Lost Hand" , cvPoint(100, 100), &font, CV_RGB(255,0,255));

  cout << "Lost Hand " << nId << endl;
  pNodes->mGesture.AddGesture( pNodes->sGestureToUse, NULL );
}

void clearImg(IplImage* inputimg)
{
    CvFont font;
    cvInitFont( &font, CV_FONT_VECTOR0,1, 1, 0, 3, 5);
    memset(inputimg->imageData,255,640*480*3);
}

int main(int argc, char *argv[])
{
    // 1. initial
    xn::Context mContext;
    mContext.Init();

    XnStatus res;
     char key=0;
     cvNamedWindow("Gesture",1);

    // 2a. create OpenNI nodes
    SNode mNodes;
    res=mNodes.mDepth.Create( mContext );
    res=mNodes.mGesture.Create( mContext );
    res=mNodes.mHand.Create( mContext );
    res=mNodes.mImage.Create(mContext);
    mNodes.drawPadImg=cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,3);

     xn::ImageMetaData imgMD;

    // 2b set some variable
    mNodes.mHand.SetSmoothing( 3.25f );//click影响的核心参数
    mNodes.sGestureToPress = "Click";
    mNodes.sGestureToUse = "RaiseHand";

    // 3. set callback of gesture generator
    mNodes.mGesture.AddGesture( mNodes.sGestureToUse, NULL );
    mNodes.mGesture.AddGesture( mNodes.sGestureToPress, NULL );

    XnCallbackHandle hGesture;
    mNodes.mGesture.RegisterGestureCallbacks( GestureRecognized, GProgress,
                                              &mNodes, hGesture );

    // 4 set callback of hand generator
    XnCallbackHandle hHand;
    mNodes.mHand.RegisterHandCallbacks( HandCreate, HandUpdate, HandDestroy,
                                        &mNodes, hHand );

    // 5. start generate data, and enter main loop to update data
    mContext.StartGeneratingAll();

    while( (key!=27) && !(res = mContext.WaitAndUpdateAll())  )
    {
        if(key=='c')
        {
            clearImg(mNodes.drawPadImg);
        }

        mNodes.mImage.GetMetaData(imgMD);

        cvShowImage("Gesture",mNodes.drawPadImg);

        key=cvWaitKey(20);
    }

    cvDestroyWindow("Gesture");
    cvReleaseImage(&mNodes.drawPadImg);

    // 7. stop and shutdown
    mContext.StopGeneratingAll();
    mContext.Release();

    return 0;
}
