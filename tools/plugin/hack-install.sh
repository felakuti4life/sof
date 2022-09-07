#!/bin/bash

# hacky installler for modules on host to ALSA plugin dir

DEST_DIR="/usr/lib/x86_64-linux-gnu/alsa-lib"

#cp ./sof_ep/install/lib/libsof_drc_avx2.so
cp ./sof_ep/install/lib/libsof_mixer_avx2.so ${DEST_DIR}/libsof-37c006bcaa127c419a9789282e321a76.so
#cp ./sof_ep/install/lib/libsof_asrc_avx2.so
#cp ./sof_ep/install/lib/libsof_crossover_avx2.so
#cp ./sof_ep/install/lib/libsof_tdfb_avx2.so
#cp ./sof_ep/install/lib/libsof_src_avx2.so
#cp ./sof_ep/install/lib/libsof_dcblock_avx2.so
#cp ./sof_ep/install/lib/libsof_eq-fir_avx2.so
cp ./sof_ep/install/lib/libsof_volume_avx2.so ${DEST_DIR}/libsof-7e677eb7f45f8841af14fba8bdbf8682.so
#cp ./sof_ep/install/lib/libsof_multiband_drc_avx2.so
#cp ./sof_ep/install/lib/libsof_eq-iir_avx2.so

#file read and write is same module
cp ./libsof_mod_file.so ${DEST_DIR}/libsof-270db0c2bcff5041a51a245c79c5e54b.so
#ln -s ${DEST_DIR}/libsof-270db0c2bcff5041a51a245c79c5e54b.so ${DEST_DIR}/libsof-270db0c2bcff5041a51a245c79c5e54b.so
