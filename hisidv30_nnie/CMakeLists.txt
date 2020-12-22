cmake_minimum_required(VERSION 2.8)
project(nnie-lite-seaway)

SET(CMAKE_CXX_STANDARD 11)

#ADD_DEFINITIONS(-D __DEBUG__)  # is Debug?

# ------------------------------- FLAGS -----------------------------
SET(CMAKE_C_COMPILER arm-himix200-linux-gcc)
SET(CMAKE_CXX_COMPILER arm-himix200-linux-g++)
SET(CMAKE_C_STRIP arm-himix200-linux-strip)

SET(CMAKE_C_FLAGS_PUBLIC "-mcpu=cortex-a7 -mfloat-abi=softfp -mfpu=neon-vfpv4 -ffunction-sections -mno-unaligned-access -fno-aggressive-loop-optimizations -mapcs-frame -rdynamic")
#SET(CMAKE_C_FLAGS_DEBUG "-Wall -ggdb3 -DNM_DEBUG ${CMAKE_C_FLAGS_PUBLIC}")
SET(CMAKE_C_FLAGS_RELEASE  "-Wall -O3  ${CMAKE_C_FLAGS_PUBLIC}")


SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ./)

# --------------------------------
SET(THIRD_PART ./third_party)
# Hisi
SET(HI_ROOT ${THIRD_PART}/hisi)
SET(HI_INCLUDE ${HI_ROOT}/include)
SET(HI_LIB_DIR ${HI_ROOT}/lib)
# opencv
SET(OPENCV_ROOT ./third_party/opencv-3.4.1/install_hisi)
SET(OPENCV_INCLUDE ${OPENCV_ROOT}/include)
SET(OPENCV_LIB_DIR ${OPENCV_ROOT}/lib)
FILE(GLOB OPENCV_LIBS "${OPENCV_LIB_DIR}/*")

# ------------ lib ----------------
LINK_LIBRARIES(mpi.a)
SET(HI_LIBS hdmi.a isp.a _hiawb.a _hiae.a _hidehaze.a upvqe.a dnvqe.a md.a
        _hildci.a _hidrc.a securec.a nnie.a ive.a tde.a VoiceEngine.a  
	sns_imx307.a sns_imx307_2l.a sns_imx327.a sns_imx327_2l.a sns_imx335.a sns_imx458.a sns_mn34220.a sns_os05a.a)

INCLUDE_DIRECTORIES(inc  ${OPENCV_INCLUDE} ${HI_INCLUDE})
LINK_DIRECTORIES( ${OPENCV_LIB_DIR} ${HI_LIB_DIR})

FILE(GLOB SRC "src/*.cpp" "src/*c")
SET(NET_SRC ./src/Net.cpp)


ADD_EXECUTABLE(main ${SRC} main.cpp)
TARGET_LINK_LIBRARIES(main  ${HI_LIBS} ${OPENCV_LIBS} -lstdc++ -lpthread -lm -ldl )


