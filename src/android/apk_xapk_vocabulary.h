/*
 * apk_xapk_vocabulary.h — Android APK/XAPK graph vocabulary registry.
 *
 * This registry is intentionally path-agnostic and extractor-agnostic. It is
 * for already-extracted Android app artifacts produced by external RE tools.
 *
 * It does not unpack APK/XAPK files, decompile code, run dynamic analysis,
 * classify malware, or encode bypass behavior. It only centralizes graph
 * labels, edge types, property keys, source formats, confidence values, and
 * documentation categories that are valid in codebase-memory's existing graph
 * model: node label strings, edge type strings, and JSON properties.
 */
#ifndef CBM_ANDROID_APK_XAPK_VOCABULARY_H
#define CBM_ANDROID_APK_XAPK_VOCABULARY_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CBM_ANDROID_VOCAB_NODE_LABEL = 1,
    CBM_ANDROID_VOCAB_EDGE_TYPE = 2,
    CBM_ANDROID_VOCAB_PROPERTY = 3,
    CBM_ANDROID_VOCAB_CATEGORY = 4,
    CBM_ANDROID_VOCAB_SOURCE_FORMAT = 5,
    CBM_ANDROID_VOCAB_CONFIDENCE = 6
} cbm_android_vocab_kind_t;

typedef struct {
    const char *name;
    const char *group;
    const char *description;
} cbm_android_vocab_entry_t;

/* Node labels for Android APK/XAPK graph facts. */
const cbm_android_vocab_entry_t *cbm_android_vocab_node_labels(size_t *count);

/* Edge types for Android APK/XAPK graph relationships. */
const cbm_android_vocab_entry_t *cbm_android_vocab_edge_types(size_t *count);

/* JSON property keys for Android APK/XAPK nodes and edges. */
const cbm_android_vocab_entry_t *cbm_android_vocab_properties(size_t *count);

/* Documentation/business categories useful for grouping evidence. */
const cbm_android_vocab_entry_t *cbm_android_vocab_categories(size_t *count);

/* Evidence source formats accepted by the Android vocabulary. */
const cbm_android_vocab_entry_t *cbm_android_vocab_source_formats(size_t *count);

/* Standard confidence values. */
const cbm_android_vocab_entry_t *cbm_android_vocab_confidence_values(size_t *count);

/* Generic lookup helpers. */
bool cbm_android_vocab_is_node_label(const char *name);
bool cbm_android_vocab_is_edge_type(const char *name);
bool cbm_android_vocab_is_property(const char *name);
bool cbm_android_vocab_is_category(const char *name);
bool cbm_android_vocab_is_source_format(const char *name);
bool cbm_android_vocab_is_confidence(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* CBM_ANDROID_APK_XAPK_VOCABULARY_H */
