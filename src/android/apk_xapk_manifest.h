/*
 * apk_xapk_manifest.h — AndroidManifest.xml semantic graph extraction.
 *
 * Input is already-extracted, text XML. This module does not unpack APKs or
 * call Android tooling.
 */
#ifndef CBM_ANDROID_APK_XAPK_MANIFEST_H
#define CBM_ANDROID_APK_XAPK_MANIFEST_H

#include "graph_buffer/graph_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int apps;
    int manifests;
    int components;
    int permissions;
    int intent_filters;
    int deep_links;
} cbm_android_manifest_stats_t;

int cbm_android_manifest_emit(cbm_gbuf_t *gbuf, const char *project_name, const char *rel_path,
                              const char *source, int source_len,
                              cbm_android_manifest_stats_t *stats);

#ifdef __cplusplus
}
#endif

#endif /* CBM_ANDROID_APK_XAPK_MANIFEST_H */
