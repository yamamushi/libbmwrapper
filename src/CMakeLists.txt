set(NAME bmwrapper)

set(SRC
  BitMessage.cpp
  BitMessageQueue.cpp
  Network.cpp
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
