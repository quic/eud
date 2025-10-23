AUTOMAKE_OPTIONS = subdir-objects

if SUBPROJECT_BUILD
noinst_LTLIBRARIES = libeud.la
else 
lib_LTLIBRARIES = libeud.la
if WINDOWS_ENV
if BUILD_DLL
libeud_la_LDFLAGS = -shared -no-undefined -lusb-1.0 -avoid-version
endif
endif
endif


library_includedir = inc 

libeud_la_SOURCES =  			\
    src/eud_error_defines.cpp 	\
	src/usb.cpp              	\
	src/device_manager.cpp    	\
	src/general_api_processing.cpp 	\
	src/rw_childprocess.cpp  	\
	src/eud.cpp              	\
	src/eud_api.cpp          	\
	src/jtag_eud.cpp          	\
	src/jtag_api.cpp          	\
	src/com_eud.cpp          	\
	src/com_api.cpp           	\
	src/ctl_eud.cpp          	\
	src/ctl_api.cpp          	\
	src/trc_eud.cpp          	\
	src/trc_api.cpp        		\
	src/swd_eud.cpp          	\
	src/swd_api.cpp
#	src/jtag_test.cpp		

libeud_la_CPPFLAGS = -Wall -Wno-narrowing -I $(library_includedir)

