
bmdir			=	.

lib_LTLIBRARIES		=	libbmwrapper.la

libbmwrapper_la_SOURCES	=	$(bm_sources) $(bm_headers)

libbmwrapper_la_CXXFLAGS=	$(AM_CXXFLAGS)			\
				-I$(bmdir)

libbmwrapper_la_LIBADD	=	-lxmlrpc_client++ -lxmlrpc++ -lxmlrpc -lboost_system -lboost_program_options -ljson

libbmwrapper_la_LDFLAGS =       --no-undefined

bm_sources		=	BitMessage.cpp			\
				base64.cpp		\
				Network.cpp		\
				XmlRPC.cpp		\
				BitMessageQueue.cpp

bm_headers		=	BitMessage.h			\
				TR1_Wrapper.hpp			\
				Threading.h			\
				base64.h			\
				Network.h		\
				XmlRPC.h		\
				MainConfigParser.h		\
				BitMessageQueue.h		\
				MsgQueue.h		\
				WorkQueue.h