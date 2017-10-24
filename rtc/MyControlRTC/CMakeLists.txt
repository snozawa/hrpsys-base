set(comp_sources MyControlRTC.cpp MyControlRTCService_impl.cpp)
set(libs hrpModel-3.1 hrpUtil-3.1 hrpsysBaseStub)
add_library(MyControlRTC SHARED ${comp_sources})
target_link_libraries(MyControlRTC ${libs})
set_target_properties(MyControlRTC PROPERTIES PREFIX "")

add_executable(MyControlRTCComp MyControlRTCComp.cpp ${comp_sources})
target_link_libraries(MyControlRTCComp ${libs})

set(target MyControlRTC MyControlRTCComp)

install(TARGETS ${target}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
)
