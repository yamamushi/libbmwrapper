# libbmwrapper Scons build script
#
# You must have "scons" installed to perform this build.
#
# This build script is subject to frequent changes.
#
#


env = Environment()
env['STATIC_AND_SHARED_OBJECTS_ARE_THE_SAME']=1

env.ParseConfig( 'xmlrpc-c-config c++2 client --cflags --libs' )
env.Append(CPPPATH = ['/usr/local/include/','src','src/crypto','src/jsoncpp','src/network','src/queue'])
env.Append(LIBPATH = ['/usr/local/lib/'])
env.Append(LIBS = ['xmlrpc_client++','boost_system'])
#env.Append(CXXFLAGS = ['-std=c++11','-stdlib=libc++'])
env.Append(CXXFLAGS = ['-std=c++11'])


sources = Split("""
src/jsoncpp/jsoncpp.cpp
src/queue/BitMessageQueue.cpp
src/network/Network.cpp
src/network/BitMessage.cpp
src/network/XmlRPC.cpp
src/crypto/base64.cpp
""")

object_list = env.Object(source = sources)

sharedbmwrapper = env.SharedLibrary('bmwrapper', source = object_list)
staticbmwrapper = env.StaticLibrary('bmwrapper', source = object_list)

#Default('bmwrapper')


# Installation Target

# env.Install("/usr/local/lib/", '')
# env.Alias('install', ['/usr/local/bin'])

