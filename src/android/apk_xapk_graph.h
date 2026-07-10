/*
 * apk_xapk_graph.h — Small Android graph insertion helpers.
 *
 * These helpers only emit graph facts for already-extracted Android artifacts.
 * They do not unpack APK/XAPK files, run decompilers, or change the DB schema.
 */
#ifndef CBM_ANDROID_APK_XAPK_GRAPH_H
#define CBM_ANDROID_APK_XAPK_GRAPH_H

#include "graph_buffer/graph_buffer.h"

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *key;
    const char *value;
} cbm_android_graph_prop_t;

int cbm_android_graph_qn(char *buf, size_t buf_size, const char *kind, const char *a,
                         const char *b, const char *c);

int cbm_android_graph_props(char *buf, int buf_size, const char *source_format,
                            const char *confidence, const char *evidence_path,
                            const char *android_kind, const cbm_android_graph_prop_t *extra,
                            int extra_count);

int64_t cbm_android_graph_upsert_node(cbm_gbuf_t *gbuf, const char *label, const char *name,
                                      const char *qualified_name, const char *file_path,
                                      int start_line, int end_line, const char *properties_json);

int64_t cbm_android_graph_insert_edge(cbm_gbuf_t *gbuf, int64_t source_id, int64_t target_id,
                                      const char *type, const char *properties_json);

#ifdef __cplusplus
}
#endif

#endif /* CBM_ANDROID_APK_XAPK_GRAPH_H */
