 # SPDX-License-Identifier: BSD-3-Clause

 add_subdirectory(signal_processing)
 add_subdirectory(third_party)

 add_local_sources(sof ring_buffer.c)
 add_local_sources(sof dot_product_with_scale.c)
