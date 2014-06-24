set(NAME bmwrapper)

set(SRC
  BitMessage.cpp
  BitMessageQueue.cpp
  XmlRPC.cpp
  base64.cpp
)

include_directories(SYSTEM
  ${PROJECT_SOURCE_DIR}/deps/jsoncpp/include
)

link_directories(
  ${CMAKE_BINARY_DIR}/lib
)

add_library(${NAME} SHARED ${SRC})

target_link_libraries(${NAME}
  ${Boost_LIBRARIES}
  ${XMLRPC_LIBRARIES}
  jsoncpp
  ${LIBBMWRAPPER_SYSTEM_LIBRARIES}
)

add_library(${NAME}-static STATIC ${SRC})

target_link_libraries(${NAME}-static
  ${Boost_LIBRARIES}
  ${XMLRPC_LIBRARIES}
  jsoncpp
  ${LIBBMWRAPPER_SYSTEM_LIBRARIES}
)

set_target_properties(${NAME}-static PROPERTIES OUTPUT_NAME ${NAME})

install(FILES BitMessage.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)
install(FILES base64.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)
install(FILES BitMessageQueue.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)
install(FILES MsgQueue.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)
install(FILES Threading.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)
install(FILES TR1_Wrapper.hpp DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)
install(FILES XmlRPC.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)


INSTALL(TARGETS ${NAME}-static ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
INSTALL(TARGETS ${NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
