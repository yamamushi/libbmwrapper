
bmdir			=	.

lib_LTLIBRARIES		=	libbmwrapper.la

libbmwrapper_la_SOURCES	=	$(bm_sources) $(bm_headers)

libbmwrapper_la_CXXFLAGS=	$(AM_CXXFLAGS)			\
				-I$(bmdir)			\
				-I$(bmdir)/crypto		\
				-I$(bmdir)/network		\
				-I$(bmdir)/queue		\
				-I$(bmdir)/util                 \
                                -I$(bmdir)/jsoncpp

<<<<<<< HEAD
libbmwrapper_la_LIBADD	=	-lxmlrpc_client++ -lxmlrpc++ -lxmlrpc -lboost_system -lboost_program_options -ljson
=======
libbmwrapper_la_LIBADD	=	-lxmlrpc_client++ -lxmlrpc++ -lxmlrpc -lboost_system -lboost_program_options
>>>>>>> db20b1ad09757ee6b79c2328a826e34f94f017fc

libbmwrapper_la_LDFLAGS =       --no-undefined

bm_sources		=	BitMessage.cpp			\
				crypto/base64.cpp		\
				network/Network.cpp		\
				network/XmlRPC.cpp		\
<<<<<<< HEAD
=======
				jsoncpp/jsoncpp.cpp		\
>>>>>>> db20b1ad09757ee6b79c2328a826e34f94f017fc
				queue/BitMessageQueue.cpp

bm_headers		=	BitMessage.h			\
				TR1_Wrapper.hpp			\
				Threading.h			\
				crypto/base64.h			\
				network/Network.h		\
				network/XmlRPC.h		\
				MainConfigParser.h		\
				jsoncpp/json/json-forwards.h	\
				jsoncpp/json/json.h		\
				queue/BitMessageQueue.h		\
				queue/MsgQueue.h		\
				queue/WorkQueue.h		\
				util/VectorHelp.h
