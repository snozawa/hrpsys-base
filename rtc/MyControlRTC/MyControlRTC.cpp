// -*- C++ -*-
/*!
 * @file  MyControlRTC.cpp
 * @brief virtual force sensor component
 * $Date$
 *
 * $Id$
 */

#include "MyControlRTC.h"
#include <rtm/CorbaNaming.h>
#include <hrpModel/ModelLoaderUtil.h>
#include <hrpUtil/MatrixSolvers.h>
#include <hrpModel/Sensor.h>

typedef coil::Guard<coil::Mutex> Guard;

// Module specification
// <rtc-template block="module_spec">
static const char* mycontrolrtc_spec[] =
  {
    "implementation_id", "MyControlRTC",
    "type_name",         "MyControlRTC",
    "description",       "my control rtc",
    "version",           HRPSYS_PACKAGE_VERSION,
    "vendor",            "AIST",
    "category",          "example",
    "activity_type",     "DataFlowComponent",
    "max_instance",      "10",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables
    "conf.default.debugLevel", "0",
    ""
  };
// </rtc-template>

MyControlRTC::MyControlRTC(RTC::Manager* manager)
  : RTC::DataFlowComponentBase(manager),
    // <rtc-template block="initializer">
    m_qCurrentIn("qCurrent", m_qCurrent),
    m_rpyIn("rpy", m_rpy),
    m_MyControlRTCServicePort("MyControlRTCService"),
    // </rtc-template>
    m_debugLevel(0)
{
  m_service0.mycontrolrtc(this);
}

MyControlRTC::~MyControlRTC()
{
}



RTC::ReturnCode_t MyControlRTC::onInitialize()
{
  std::cerr << "[" << m_profile.instance_name << "] onInitialize()" << std::endl;
  // <rtc-template block="bind_config">
  // Bind variables and configuration variable
  bindParameter("debugLevel", m_debugLevel, "0");
  
  // </rtc-template>

  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers
  addInPort("qCurrent", m_qCurrentIn);
  addInPort("rpy", m_rpyIn);

  // Set OutPort buffer
  
  // Set service provider to Ports
  m_MyControlRTCServicePort.registerProvider("service0", "MyControlRTCService", m_service0);
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  addPort(m_MyControlRTCServicePort);
  
  // </rtc-template>

  // CONFファイルからdt(制御周期[s]を読み込む)
  RTC::Properties& prop = getProperties();
  coil::stringTo(m_dt, prop["dt"].c_str());

  // ROBOTモデルをModelLoaderから取得
  m_robot = hrp::BodyPtr(new hrp::Body());
  RTC::Manager& rtcManager = RTC::Manager::instance();
  std::string nameServer = rtcManager.getConfig()["corba.nameservers"];
  int comPos = nameServer.find(",");
  if (comPos < 0){
      comPos = nameServer.length();
  }
  nameServer = nameServer.substr(0, comPos);
  RTC::CorbaNaming naming(rtcManager.getORB(), nameServer.c_str());
  if (!loadBodyFromModelLoader(m_robot, prop["model"].c_str(),
			       CosNaming::NamingContext::_duplicate(naming.getRootContext())
	  )){
      std::cerr << "[" << m_profile.instance_name << "] failed to load model[" << prop["model"] << "]" << std::endl;
      return RTC::RTC_ERROR;
  }

  // 初期化
  // mask_joint_flags.size(m_robot->numJoints());
  // mask_joint_angles.size(m_robot->numJoints());
  // for (size_t i = 0; m_robot->numJoints(); i++) {
  //     mask_joint_angles(i) = 0.0;
  //     mask_joint_flags(i) = false;
  // }

  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t MyControlRTC::onFinalize()
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t MyControlRTC::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t MyControlRTC::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

RTC::ReturnCode_t MyControlRTC::onActivated(RTC::UniqueId ec_id)
{
  std::cerr << "[" << m_profile.instance_name<< "] onActivated(" << ec_id << ")" << std::endl;
  return RTC::RTC_OK;
}

RTC::ReturnCode_t MyControlRTC::onDeactivated(RTC::UniqueId ec_id)
{
  std::cerr << "[" << m_profile.instance_name<< "] onDeactivated(" << ec_id << ")" << std::endl;
  return RTC::RTC_OK;
}

#define DEBUGP ((m_debugLevel==1 && loop%200==0) || m_debugLevel > 1 )
RTC::ReturnCode_t MyControlRTC::onExecute(RTC::UniqueId ec_id)
{
  //std::cout << m_profile.instance_name<< ": onExecute(" << ec_id << ")" << std::endl;
  static int loop = 0;
  loop ++;

  // InPort
  if (m_qCurrentIn.isNew()) {
      m_qCurrentIn.read();
      for ( unsigned int i = 0; i < m_robot->numJoints(); i++ ){
          m_robot->joint(i)->q = m_qCurrent.data[i];
      }
  }

  Guard guard(m_mutex);
  m_robot->calcForwardKinematics();

  // OutPort
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t MyControlRTC::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t MyControlRTC::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t MyControlRTC::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t MyControlRTC::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t MyControlRTC::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/


bool MyControlRTC::setMyControlRTCParam (const ::OpenHRP::MyControlRTCService::MyControlRTCParam &i_param)
{
    std::cerr << "[" << m_profile.instance_name << "] setMyControlRTCParam()" << std::endl;
    Guard guard(m_mutex);
    return true;
};

bool MyControlRTC::getMyControlRTCParam (::OpenHRP::MyControlRTCService::MyControlRTCParam& i_param)
{
    std::cerr << "[" << m_profile.instance_name << "] getMyControlRTCParam()" << std::endl;
    Guard guard(m_mutex);
    return true;
};

extern "C"
{

  void MyControlRTCInit(RTC::Manager* manager)
  {
    RTC::Properties profile(mycontrolrtc_spec);
    manager->registerFactory(profile,
                             RTC::Create<MyControlRTC>,
                             RTC::Delete<MyControlRTC>);
  }

};


