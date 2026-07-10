/*
 * apk_xapk_resources.h — Android resource/asset graph extraction.
 *
 * Input is already-extracted files. This module does not unpack APK/XAPK files
 * or call Android tooling.
 */
#ifndef CBM_ANDROID_APK_XAPK_RESOURCES_H
#define CBM_ANDROID_APK_XAPK_RESOURCES_H

#include "graph_buffer/graph_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int resource_files;
    int string_resources;
    int layout_resources;
    int menu_resources;
    int navigation_resources;
    int drawable_resources;
    int xml_configs;
    int raw_resources;
    int assets;
    int native_libraries;
    int api_endpoints;
} cbm_android_resource_stats_t;

int cbm_android_resources_emit_file(cbm_gbuf_t *gbuf, const char *project_name, const char *rel_path,
                                    const char *source, int source_len,
                                    cbm_android_resource_stats_t *stats);

#ifdef __cplusplus
}
#endif

#endif /* CBM_ANDROID_APK_XAPK_RESOURCES_H */
