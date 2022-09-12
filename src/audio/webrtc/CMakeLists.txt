 # SPDX-License-Identifier: BSD-3-Clause

 target_include_directories(sof PRIVATE ${PROJECT_SOURCE_DIR}/src/include/sof/audio/webrtc)

 add_subdirectory(common_audio)
 add_subdirectory(modules)
 # add_subdirectory(rtc_base)

 add_local_sources(sof aecm_api.c)
