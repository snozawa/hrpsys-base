// -*- mode: c++; indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*-
#include <iostream>
#include "MyControlRTCService_impl.h"
#include "MyControlRTC.h"

MyControlRTCService_impl::MyControlRTCService_impl() : m_mycontrolrtc(NULL)
{
}

MyControlRTCService_impl::~MyControlRTCService_impl()
{
}


::CORBA::Boolean MyControlRTCService_impl::setMyControlRTCParam(const ::OpenHRP::MyControlRTCService::MyControlRTCParam& i_param)
{
	return m_mycontrolrtc->setMyControlRTCParam(i_param);
};

::CORBA::Boolean MyControlRTCService_impl::getMyControlRTCParam(::OpenHRP::MyControlRTCService::MyControlRTCParam_out i_param)
{
	i_param = new OpenHRP::MyControlRTCService::MyControlRTCParam();
	return m_mycontrolrtc->getMyControlRTCParam(*i_param);
};

void MyControlRTCService_impl::mycontrolrtc(MyControlRTC *i_mycontrolrtc)
{
	m_mycontrolrtc = i_mycontrolrtc;
}

