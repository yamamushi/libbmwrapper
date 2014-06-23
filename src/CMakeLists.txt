set(NAME bmwrapper)

include_directories(SYSTEM
  ${PROJECT_SOURCE_DIR}/deps/jsoncpp/include
)

set(SRC
  BitMessage.cpp
  BitMessageQueue.cpp
  Network.cpp
  XmlRPC.cpp
  base64.cpp
)

add_library(${NAME} ${SRC})

target_link_libraries(${NAME}
  ${Boost_LIBRARIES}
  ${OPENTXS_AUDITOR_SYSTEM_LIBRARIES}
)
