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
static float eps=0.0000001;

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
  //CVMouse    mVMouse;
};

void XN_CALLBACK_TYPE GestureRecognized( xn::GestureGenerator& generator,
                                         const XnChar* strGesture,
                                         const XnPoint3D* pIDPosition,
                                         const XnPoint3D* pEndPosition,
                                         void* pCookie )
{
  SNode* pNodes = ((SNode*)pCookie);
  if( strcmp( strGesture, pNodes->sGestureToPress ) == 0 )
  {
    cout << "click" << endl;
  }
  else if( strcmp( strGesture, pNodes->sGestureToUse ) == 0 )
  {
    cout << "Start moving" << endl;
    generator.RemoveGesture(strGesture);
   // cout<<strGesture<<"from"<<*pIDPosition<<"to"<<*pEndPosition<<endl;
    pNodes->mHand.StartTracking( *pIDPosition );
  }
}

void XN_CALLBACK_TYPE HandCreate( xn::HandsGenerator& generator,
                                  XnUserID nId,
                                  const XnPoint3D* pPosition,
                                  XnFloat fTime,
                                  void* pCookie )
{
  SNode* pNodes = ((SNode*)pCookie);
  cout << "New Hand: " << nId << " detected!\n";
 // cout << pPosition->X << "/" << pPosition->Y << "/" << pPosition->Z << endl;

  pNodes->mGesture.AddGesture( pNodes->sGestureToPress, NULL );

//  XnPoint3D  wPos;
//  pNodes->mDepth.ConvertRealWorldToProjective( 1, pPosition, &wPos );
//  cout << wPos.X << "/" << wPos.Y << endl;
//  i++;
//  start_x.push_back( pPosition->X);
//  start_y.push_back(pPosition->Y);
}

void XN_CALLBACK_TYPE HandUpdate( xn::HandsGenerator& generator,
                                  XnUserID nId,
                                  const XnPoint3D* pPosition,
                                  XnFloat fTime,
                                  void* pCookie )
{
  SNode* pNodes = ((SNode*)pCookie);
//  XnPoint3D  wPos;
//  pNodes->mDepth.ConvertRealWorldToProjective( 1, pPosition, &wPos );
//  cout << wPos.X << "/" << wPos.Y << endl;


  i++;
  start_x.push_back( pPosition->X);
  start_y.push_back(pPosition->Y);
  if(start_x.size()>5&&abs(start_x.at(i-1)-start_x.at(i-2))==0&&abs(start_y.at(i-1)-start_y.at(i-2))==0){
      XnFloat k=(start_y.at(i-1)-start_y.at(0))/((start_x.at(i-1)-start_x.at(0))+eps);
      if(k!=0){
          if(abs(k)<1){

             // cout<<abs(k)<<endl;
    //          memset(refimage->imageData,255,640*480*3);
    //          cvPutText(refimage, "horizontal!" , cvPoint(100, 100), &font, CV_RGB(0,255,0));
              start_x.clear();
              start_y.clear();
              i=0;
              cout<<"horizontal"<<endl;
          }
          else{
              //cout<<abs(k)<<endl;
    //          memset(refimage->imageData,255,640*480*3);
    //          cvPutText(refimage, "vertical!" ,cvPoint(100, 100), &font, CV_RGB(255,0,0));
              start_x.clear();
              start_y.clear();
              i=0;
              cout<<"vertical"<<endl;
          }
      }
//      else{
//          cout<<endl;
//      }

  }
}


void XN_CALLBACK_TYPE HandDestroy( xn::HandsGenerator& generator,
                                   XnUserID nId,
                                   XnFloat fTime,
                                   void* pCookie )
{
  SNode* pNodes = ((SNode*)pCookie);
  cout << "Lost Hand: " << nId << endl;
  pNodes->mGesture.AddGesture( pNodes->sGestureToUse, NULL );
  pNodes->mGesture.RemoveGesture( pNodes->sGestureToPress );
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // 1. initial context
    xn::Context mContext;
    mContext.Init();

    XnStatus res;

    // 2a. create OpenNI nodes
    SNode mNodes;
    res=mNodes.mDepth.Create( mContext );
    res=mNodes.mGesture.Create( mContext );
    res=mNodes.mHand.Create( mContext );

    // 2b set some variable
    mNodes.mHand.SetSmoothing( 0.5f );
    mNodes.sGestureToPress = "Click";
    mNodes.sGestureToUse = "RaiseHand";

    // 3. set callback of gesture generator

    mNodes.mGesture.AddGesture( mNodes.sGestureToUse, NULL );
    mNodes.mGesture.AddGesture( mNodes.sGestureToPress, NULL );

    XnCallbackHandle hGesture;
    mNodes.mGesture.RegisterGestureCallbacks( GestureRecognized, NULL,
                                              &mNodes, hGesture );




    // 4 set callback of hand generator
    XnCallbackHandle hHand;
    mNodes.mHand.RegisterHandCallbacks( HandCreate, HandUpdate, HandDestroy,
                                        &mNodes, hHand );

    // 5. start generate data, and enter main loop to update data
    mContext.StartGeneratingAll();
    while( true )
      mContext.WaitAndUpdateAll();

    // 7. stop and shutdown
    mContext.StopGeneratingAll();
    mContext.Release();

    return a.exec();
}
