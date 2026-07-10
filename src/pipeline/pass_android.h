/*
 * pass_android.h — Android APK/XAPK semantic pass.
 *
 * Runs inside the normal codebase-memory indexing pipeline over already-extracted
 * artifacts. It does not unpack APK/XAPK files or call external RE tools.
 */
#ifndef CBM_PIPELINE_PASS_ANDROID_H
#define CBM_PIPELINE_PASS_ANDROID_H

#include "pipeline/pipeline_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

int cbm_pipeline_pass_android(cbm_pipeline_ctx_t *ctx, const cbm_file_info_t *files, int file_count);

#ifdef __cplusplus
}
#endif

#endif /* CBM_PIPELINE_PASS_ANDROID_H */
