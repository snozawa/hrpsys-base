// -*-C++-*-
#ifndef MYCONTROLRTCSERVICE_IMPL_H
#define MYCONTROLRTCSERVICE_IMPL_H

#include "hrpsys/idl/MyControlRTCService.hh"

using namespace OpenHRP;

class MyControlRTC;

class MyControlRTCService_impl 
  : public virtual POA_OpenHRP::MyControlRTCService,
    public virtual PortableServer::RefCountServantBase
{
public:
  MyControlRTCService_impl();
  virtual ~MyControlRTCService_impl();

  ::CORBA::Boolean setMyControlRTCParam(const ::OpenHRP::MyControlRTCService::MyControlRTCParam& i_param);
  ::CORBA::Boolean getMyControlRTCParam(::OpenHRP::MyControlRTCService::MyControlRTCParam_out i_param);

  void mycontrolrtc(MyControlRTC *i_mycontrolrtc);
private:
  MyControlRTC *m_mycontrolrtc;
};				 

#endif
