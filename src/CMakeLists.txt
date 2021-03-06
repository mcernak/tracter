include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${Boost_INCLUDE_DIRS})
add_definitions(-DPACKAGE_VERSION="${VERSION}")

# Some really basic system things; not very portable
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  add_definitions(-DHAVE_DARWIN)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_definitions(-DHAVE_LINUX)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

find_path(FPU fpu_control.h)
if(${FPU} MATCHES "FPU-NOTFOUND")
  message(STATUS "fpu_control.h not found")
else()
  message(STATUS "fpu_control.h found")
  add_definitions(-DHAVE_FPU_CONTROL)
endif()

# Basic all-the-time sources
set(SOURCES
  ASRFactory.cpp
  BoolToFloat.cpp
  ByteOrder.cpp
  Cepstrum.cpp
  Comparator.cpp
  ComplexPeriodogram.cpp
  ComplexSample.cpp
  Component.cpp
  Concatenate.cpp
  CosineTransform.cpp
  Delta.cpp
  Divide.cpp
  Energy.cpp
  EnergyNorm.cpp
  Extract.cpp
  FilePath.cpp
  FileSink.cpp
  FourierTransform.cpp
  Frame.cpp
  Gate.cpp
  Histogram.cpp
  HTKLib.cpp
  HTKSink.cpp
  HTKSource.cpp
  LinearTransform.cpp
  LNASource.cpp
  Log.cpp
  LowEnergyEnvelope.cpp
  LPCepstrum.cpp
  Mean.cpp
  MelFilter.cpp
  Minima.cpp
  MMap.cpp
  Modulation.cpp
  Noise.cpp
  NoiseVAD.cpp
  Normalise.cpp
  OverlapAdd.cpp
  Periodogram.cpp
  Pixmap.cpp
  SNRSpectrum.cpp
  ScreenSink.cpp
  Select.cpp
  SocketSink.cpp
  SocketSource.cpp
  SocketTee.cpp
  SpeakerIDSocketSource.cpp
  SpectralSubtract.cpp
  StreamSocketSource.cpp
  Subtract.cpp
  TimedLatch.cpp
  Tokenise.cpp
  TracterFPE.cpp
  TracterObject.cpp
  TransverseFilter.cpp
  Unframe.cpp
  VADGate.cpp
  VADStateMachine.cpp
  Variance.cpp
  ViterbiVAD.cpp
  ViterbiVADGate.cpp
  Window.cpp
  ZeroFilter.cpp
  )

# Prime the header list with the cpp sources, then add the lone ones
string(REGEX REPLACE ".cpp" ".h" HEADERS "${SOURCES}")
list(APPEND HEADERS
  CachedComponent.h
  FileSource.h
  Fourier.h
  FrameSink.h
  GeometricNoise.h
  Resample.h
  Sink.h
  Source.h
  )

# Things to install
set(INSTALL_TARGETS
  extracter
  static-lib
)

# Kiss is compulsory for now
# The library is not supplied built, so we add in the sources explicitly.
if(KISSFFT_FOUND)
  list(APPEND SOURCES
    FourierKiss.cpp
    ${KISSFFT_DIR}/tools/kiss_fftr.c
    ${KISSFFT_DIR}/kiss_fft.c
    )
  include_directories(${KISSFFT_DIR} ${KISSFFT_DIR}/tools)
endif(KISSFFT_FOUND)

# HTK is optional
if(HTK_FOUND)
  list(APPEND SOURCES HCopyWrapper.cpp HTKLibSource.cpp)
  list(APPEND HEADERS HCopyWrapper.h   HTKLibSource.h  )
  add_definitions(-DHAVE_HTKLIB)
  include_directories(${HTK_INCLUDE_DIRS})
endif(HTK_FOUND)

# Torch3 is optional
if(TORCH3_FOUND)
  list(APPEND SOURCES
    BlasLinear.cpp
    SKMLP.cpp
    MLP.cpp
    MLPVAD.cpp
    )
  list(APPEND HEADERS
    BlasLinear.h
    SKMLP.h
    MLP.h
    MLPVAD.h
    )
  add_definitions(-DHAVE_TORCH3)
  include_directories(${TORCH3_INCLUDE_DIRS})
  link_directories(${TORCH3_LIBRARY_DIRS})
endif(TORCH3_FOUND)

# BSAPI comes as a shared library
if(BSAPI_FOUND)
  list(APPEND SOURCES
    BSAPITransform.cpp
    BSAPIFrontEnd.cpp
    BSAPIFilterBank.cpp
    BSAPIFastVTLN.cpp
    )
  list(APPEND HEADERS
    BSAPITransform.h
    BSAPIFrontEnd.h
    BSAPIFilterBank.h
    BSAPIFastVTLN.h
    )
  add_definitions(-DHAVE_BSAPI)
  include_directories(${BSAPI_INCLUDE_DIRS})
endif(BSAPI_FOUND)

# ALSA is a system thing, depends on whether pkgconfig found it
if(ALSA_FOUND)
  list(APPEND SOURCES ALSASource.cpp)
  list(APPEND HEADERS ALSASource.h  )
  add_definitions(-DHAVE_ALSA)
  include_directories(${ALSA_INCLUDE_DIRS})
  list(APPEND PKGCONFIG_REQUIRES alsa)
endif(ALSA_FOUND)

# SndFile pretty much like ALSA
if(SNDFILE_FOUND)
  list(APPEND SOURCES SndFileSource.cpp SndFileSink.cpp)
  list(APPEND HEADERS SndFileSource.h   SndFileSink.h  )
  add_definitions(-DHAVE_SNDFILE)
  include_directories(${SNDFILE_INCLUDE_DIRS})
  list(APPEND PKGCONFIG_REQUIRES sndfile)

  # codec and recorder rely on SndFile
  add_executable(codec codec.cpp)
  add_executable(recorder recorder.cpp)
  target_link_libraries(codec static-lib)
  target_link_libraries(recorder  static-lib pthread)
  list(APPEND INSTALL_TARGETS recorder)
endif(SNDFILE_FOUND)

# libresample
if(LIBRESAMPLE_FOUND) 
  list(APPEND SOURCES ResampleLRS.cpp)
  add_definitions(-DHAVE_RESAMPLE)
  include_directories(${LIBRESAMPLE_INCLUDE_DIRS})
endif(LIBRESAMPLE_FOUND)

# RtAudio
if(RTAUDIO_FOUND)
  list(APPEND SOURCES RtAudioSource.cpp)
  list(APPEND HEADERS RtAudioSource.h  )
  add_definitions(-DHAVE_RTAUDIO)
  include_directories(${RTAUDIO_INCLUDE_DIRS})
endif(RTAUDIO_FOUND)

# SPTK
if(SPTK_FOUND)
  list(APPEND SOURCES MCep.cpp)
  list(APPEND HEADERS MCep.h  )
  add_definitions(-DHAVE_SPTK)
  include_directories(${SPTK_INCLUDE_DIRS})
endif(SPTK_FOUND)

# PulseAudio is another pkgconfig one
if(PULSEAUDIO_FOUND)
  list(APPEND SOURCES PulseAudioSource.cpp) # PulseAudioSink.cpp)
  list(APPEND HEADERS PulseAudioSource.h)   # PulseAudioSink.h  )
  add_definitions(-DHAVE_PULSEAUDIO)
  include_directories(${PULSEAUDIO_INCLUDE_DIRS})
  list(APPEND PKGCONFIG_REQUIRES libpulse-simple)
endif(PULSEAUDIO_FOUND)

# A little wordy, but allows static lib also with the same syntax
set(TARGET_LIBS
  ${TORCH3_LIBRARIES}
  ${HTK_LIBRARIES}
  ${BSAPI_LIBRARIES}
  ${ALSA_LIBRARIES}
  ${SNDFILE_LIBRARIES}
  ${SPTK_LIBRARIES}
  ${RTAUDIO_LIBRARIES}
  ${LIBRESAMPLE_LIBRARIES}
  ${Boost_LIBRARIES}
  ${PULSEAUDIO_LIBRARIES}
)

# Static library
add_library(static-lib STATIC ${SOURCES})
set_target_properties(static-lib
  PROPERTIES OUTPUT_NAME   "tracter"
             PUBLIC_HEADER "${HEADERS}"
  )
target_link_libraries(static-lib ${TARGET_LIBS})

# Shared library
set(USE_SHARED $ENV{USE_SHARED}
  CACHE BOOLEAN "Whether to build shared library"
  )
if (USE_SHARED)
  add_library(shared-lib SHARED ${SOURCES})
  set_target_properties(shared-lib
    PROPERTIES OUTPUT_NAME   "tracter"
    PUBLIC_HEADER "${HEADERS}"
    )
  target_link_libraries(shared-lib ${TARGET_LIBS})
  list(APPEND INSTALL_TARGETS shared-lib)
endif (USE_SHARED)

add_executable(extracter extracter.cpp)

#add_executable(testfile testfile.c)
#add_executable(creature creature.cpp)
#add_executable(fft fft.cpp)
#add_executable(socketserver socketserver.cpp)
#add_executable(socketclient socketclient.cpp)
#add_executable(lnadump lnadump.cpp)
#add_executable(parser parser.cpp)

# These link static for the time being.  Could be changed.
target_link_libraries(extracter static-lib pthread)
#target_link_libraries(testfile static-lib)
#target_link_libraries(creature static-lib)
#target_link_libraries(fft static-lib)
#target_link_libraries(socketserver static-lib)
#target_link_libraries(socketclient static-lib)
#target_link_libraries(lnadump static-lib)
#target_link_libraries(parser static-lib)

# CMake install line
install(
  TARGETS ${INSTALL_TARGETS}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/tracter
  )

# This feels like a hack.
string(REGEX REPLACE ";" " -I" TORCH3_CFLAGS "-I${TORCH3_INCLUDE_DIRS}")
string(REGEX REPLACE ";" "," TRACTER_REQUIRES "${PKGCONFIG_REQUIRES}")

# pkgconfig install lines
set(prefix ${CMAKE_INSTALL_PREFIX})
set(exec_prefix "\${prefix}")
set(libdir "\${exec_prefix}/lib")
set(includedir "\${prefix}/include/tracter")
configure_file(tracter.pc.in ${CMAKE_CURRENT_BINARY_DIR}/tracter.pc)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/tracter.pc
  DESTINATION lib/pkgconfig
  )
