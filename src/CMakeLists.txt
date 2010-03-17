include_directories(${CMAKE_CURRENT_BINARY_DIR}) # For config.h
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DPACKAGE_VERSION="${VERSION}")

# Basic all-the-time sources
set(SOURCES
  ASRFactory.cpp
  ByteOrder.cpp
  Cepstrum.cpp
  ComplexPeriodogram.cpp
  ComplexSample.cpp
  Component.cpp
  Concatenate.cpp
  Delta.cpp
  Divide.cpp
  Energy.cpp
  EnergyNorm.cpp
  Extract.cpp
  FilePath.cpp
  FileSink.cpp
  Frame.cpp
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
  Periodogram.cpp
  Pixmap.cpp
  SNRSpectrum.cpp
  ScreenSink.cpp
  Select.cpp
  Signal.cpp
  SocketSink.cpp
  SocketSource.cpp
  SocketTee.cpp
  SpeakerIDSocketSource.cpp
  SpectralSubtract.cpp
  StreamSocketSource.cpp
  Subtract.cpp
  Tokenise.cpp
  TracterObject.cpp
  TransverseFilter.cpp
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

# Kiss is compulsory for now...
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

# ALSA is a system thng, depends on whether pkgconfig found it
if(ALSA_FOUND)
  list(APPEND SOURCES ALSASource.cpp)
  list(APPEND HEADERS ALSASource.h  )
  add_definitions(-DHAVE_ALSA)
  include_directories(${ALSA_INCLUDE_DIRS})
#  link_directories(${ALSA_LIBRARY_DIRS})
endif(ALSA_FOUND)

# SndFile pretty much like ALSA
if(ALSA_FOUND)
  list(APPEND SOURCES SndFileSource.cpp)
  list(APPEND HEADERS SndFileSource.h  )
  add_definitions(-DHAVE_SNDFILE)
  include_directories(${SNDFILE_INCLUDE_DIRS})
#  link_directories(${SNDFILE_LIBRARY_DIRS})
endif(ALSA_FOUND)

# libresample. This gets build in place, possibly an LGPL violation.
if(LIBRESAMPLE_FOUND) 
  list(APPEND SOURCES ResampleLRS.cpp)
  list(APPEND SOURCES
    ${LIBRESAMPLE_DIR}/src/resample.c
    ${LIBRESAMPLE_DIR}/src/resamplesubs.c
    ${LIBRESAMPLE_DIR}/src/filterkit.c    
    )
  include_directories(
    ${LIBRESAMPLE_DIR}/src
    ${LIBRESAMPLE_DIR}/include
    )
  add_definitions(-DHAVE_RESAMPLE)
endif(LIBRESAMPLE_FOUND)

# RtAudio.  Again, build in place, although I think this one is OK.
if(RTAUDIO_FOUND)
  list(APPEND SOURCES RtAudioSource.cpp)
  list(APPEND HEADERS RtAudioSource.h  )
  list(APPEND SOURCES ${RTAUDIO_DIR}/RtAudio.cpp)
  add_definitions(-DHAVE_RTAUDIO -D__LINUX_ALSA__) # Not portable!
  include_directories(${RTAUDIO_DIRS})
endif(RTAUDIO_FOUND)

# SPTK
if(SPTK_FOUND)
  list(APPEND SOURCES MCep.cpp)
  list(APPEND HEADERS MCep.h  )
  add_definitions(-DHAVE_SPTK)
  include_directories(${SPTK_INCLUDE_DIRS})
endif(SPTK_FOUND)

# A little wordy, but allows static lib also with the same syntax
add_library(shared-lib SHARED ${SOURCES})
set_target_properties(shared-lib
  PROPERTIES OUTPUT_NAME   "tracter"
             PUBLIC_HEADER "${HEADERS}"
  )

target_link_libraries(shared-lib
  ${TORCH3_LIBRARIES}
  ${HTK_LIBRARIES}
  ${BSAPI_LIBRARIES}
  ${ALSA_LIBRARIES}
  ${SNDFILE_LIBRARIES}
  ${SPTK_LIBRARIES}
  )

add_executable(extracter extracter.cpp)
add_executable(recorder recorder.cpp)

add_executable(socketserver socketserver.cpp)
add_executable(socketclient socketclient.cpp)

target_link_libraries(extracter shared-lib)
target_link_libraries(recorder  shared-lib)
target_link_libraries(socketserver shared-lib)
target_link_libraries(socketclient shared-lib)

install(
  TARGETS extracter recorder shared-lib
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/tracter
  )