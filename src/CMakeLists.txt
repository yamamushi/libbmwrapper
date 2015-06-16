set(NAME bmwrapper)

set(SRC
  BitMessage.cpp
  BitMessageQueue.cpp
  Filesystem.cpp
  XmlRPC.cpp
  base64.cpp
  bmfec.cpp
  sha256.cpp
)

include_directories(SYSTEM
  ${PROJECT_SOURCE_DIR}/deps/jsoncpp/include
  ${PROJECT_SOURCE_DIR}/deps/fecpp/src
)

link_directories(
  ${CMAKE_BINARY_DIR}/lib
)

add_library(${NAME} SHARED ${SRC})

target_link_libraries(${NAME}
  ${Boost_LIBRARIES}
  ${XMLRPC_LIBRARIES}
  jsoncpp
  fecpp
  ${LIBBMWRAPPER_SYSTEM_LIBRARIES}
)

add_library(${NAME}-static STATIC ${SRC})

target_link_libraries(${NAME}-static
  ${Boost_LIBRARIES}
  ${XMLRPC_LIBRARIES}
  jsoncpp
  fecpp
  ${LIBBMWRAPPER_SYSTEM_LIBRARIES}
)

set_target_properties(${NAME}-static PROPERTIES OUTPUT_NAME ${NAME})

install(FILES BitMessage.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)
install(FILES base64.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)
install(FILES BitMessageQueue.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)
install(FILES MsgQueue.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)
install(FILES BMThreading.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)
install(FILES TR1_Wrapper.hpp DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)
install(FILES XmlRPC.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)
install(FILES bmfec.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)
install(FILES Filesystem.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)
install(FILES sha256.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include/bmwrapper)


INSTALL(TARGETS bmwrapper-static ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
INSTALL(TARGETS bmwrapper LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
