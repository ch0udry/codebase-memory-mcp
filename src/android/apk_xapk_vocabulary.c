/*
 * apk_xapk_vocabulary.c — Android APK/XAPK graph vocabulary registry.
 */
#include "android/apk_xapk_vocabulary.h"

#include <string.h>

#define ENTRY(name, group, description) { name, group, description }
#define ARRAY_COUNT(a) (sizeof(a) / sizeof((a)[0]))

static const cbm_android_vocab_entry_t NODE_LABELS[] = {
    /* App and artifact structure */
    ENTRY("AndroidApp", "app", "Logical Android application being indexed."),
    ENTRY("AndroidPackage", "app", "Android package namespace."),
    ENTRY("ApkArtifact", "artifact", "APK artifact or extracted APK unit."),
    ENTRY("XapkArtifact", "artifact", "XAPK container or extracted XAPK unit."),
    ENTRY("ApkSplit", "artifact", "Split APK or config APK."),
    ENTRY("SplitConfig", "artifact", "ABI, density, language, or feature split metadata."),
    ENTRY("DexFile", "artifact", "DEX file or DEX-equivalent source unit."),
    ENTRY("OatFile", "artifact", "OAT artifact metadata if present."),
    ENTRY("VdexFile", "artifact", "VDEX artifact metadata if present."),
    ENTRY("Manifest", "manifest", "Android manifest document."),
    ENTRY("Application", "manifest", "Manifest application node."),
    ENTRY("ApplicationClass", "manifest", "Custom application class."),
    ENTRY("BuildConfig", "app", "BuildConfig-like constants and metadata."),
    ENTRY("SigningInfo", "app", "Signing metadata if available from extracted metadata."),
    ENTRY("Certificate", "app", "Certificate metadata, fingerprint, or pin reference."),
    ENTRY("AppMetadata", "app", "Generic app metadata not represented by a more specific label."),

    /* Manifest and components */
    ENTRY("Activity", "component", "Manifest-declared activity."),
    ENTRY("Service", "component", "Manifest-declared service."),
    ENTRY("BroadcastReceiver", "component", "Manifest-declared broadcast receiver."),
    ENTRY("ContentProvider", "component", "Manifest-declared content provider."),
    ENTRY("Permission", "permission", "Android permission."),
    ENTRY("UsesPermission", "permission", "Requested permission declaration."),
    ENTRY("PermissionGroup", "permission", "Permission group."),
    ENTRY("IntentFilter", "intent", "Intent filter."),
    ENTRY("IntentAction", "intent", "Intent action."),
    ENTRY("IntentCategory", "intent", "Intent category."),
    ENTRY("IntentData", "intent", "Intent data element."),
    ENTRY("DeepLink", "intent", "URI/deep-link pattern."),
    ENTRY("AppLink", "intent", "Verified app link pattern."),
    ENTRY("ManifestMetaData", "manifest", "Manifest metadata entry."),
    ENTRY("ProcessName", "manifest", "Android process declaration."),
    ENTRY("TaskAffinity", "manifest", "Task affinity declaration."),

    /* Resources and assets */
    ENTRY("Resource", "resource", "Generic Android resource."),
    ENTRY("ResourceFile", "resource", "Resource file."),
    ENTRY("ResourceString", "resource", "String resource."),
    ENTRY("ResourceId", "resource", "Resource identifier."),
    ENTRY("LayoutResource", "resource", "Layout XML resource."),
    ENTRY("DrawableResource", "resource", "Drawable resource."),
    ENTRY("MenuResource", "resource", "Menu resource."),
    ENTRY("NavigationResource", "resource", "Navigation graph resource."),
    ENTRY("XmlConfig", "resource", "XML config file, including network/security/config resources."),
    ENTRY("RawResource", "resource", "Raw resource file."),
    ENTRY("Asset", "asset", "Asset file."),
    ENTRY("FontAsset", "asset", "Font asset."),
    ENTRY("LocaleResource", "resource", "Locale-specific resource."),
    ENTRY("Theme", "resource", "Theme resource."),
    ENTRY("Style", "resource", "Style resource."),
    ENTRY("ColorResource", "resource", "Color resource."),
    ENTRY("DimensionResource", "resource", "Dimension resource."),

    /* Source and symbol mapping */
    ENTRY("JadxClass", "source", "Decompiled class from Java/Kotlin source output."),
    ENTRY("JadxMethod", "source", "Decompiled method from Java/Kotlin source output."),
    ENTRY("JadxField", "source", "Decompiled field from Java/Kotlin source output."),
    ENTRY("SmaliClass", "source", "Smali class."),
    ENTRY("SmaliMethod", "source", "Smali method."),
    ENTRY("SmaliField", "source", "Smali field."),
    ENTRY("DexClass", "source", "DEX class descriptor."),
    ENTRY("DexMethod", "source", "DEX method descriptor."),
    ENTRY("DexField", "source", "DEX field descriptor."),
    ENTRY("ObfuscatedSymbol", "obfuscation", "Symbol likely affected by obfuscation."),
    ENTRY("DeobfuscatedSymbol", "obfuscation", "Manually or tool-renamed symbol."),
    ENTRY("SourceMapping", "source", "Mapping node between source representations."),
    ENTRY("ClassDescriptor", "source", "JVM/Dalvik class descriptor."),
    ENTRY("MethodDescriptor", "source", "JVM/Dalvik method descriptor."),
    ENTRY("FieldDescriptor", "source", "JVM/Dalvik field descriptor."),
    ENTRY("PackageNamespace", "source", "Java/Kotlin package namespace."),

    /* Android framework/API usage */
    ENTRY("AndroidApiCall", "framework", "Android framework API call."),
    ENTRY("FrameworkClass", "framework", "Android framework class."),
    ENTRY("FrameworkMethod", "framework", "Android framework method."),
    ENTRY("LifecycleMethod", "framework", "Android lifecycle method."),
    ENTRY("CallbackMethod", "framework", "Callback method."),
    ENTRY("Listener", "framework", "Listener/callback registration."),
    ENTRY("Adapter", "framework", "Android adapter or UI adapter concept."),
    ENTRY("ViewModel", "framework", "ViewModel-style class."),
    ENTRY("RepositoryClass", "framework", "Repository/data access class."),
    ENTRY("Worker", "framework", "WorkManager worker or similar background worker."),
    ENTRY("JobService", "framework", "JobService."),
    ENTRY("Alarm", "framework", "AlarmManager usage."),
    ENTRY("PendingIntent", "framework", "PendingIntent usage."),
    ENTRY("NotificationChannel", "framework", "Notification channel."),
    ENTRY("BroadcastAction", "framework", "Broadcast action."),
    ENTRY("ContentUri", "framework", "Content URI."),

    /* Network/API */
    ENTRY("ApiEndpoint", "network", "HTTP endpoint or URL path."),
    ENTRY("BaseUrl", "network", "Base URL."),
    ENTRY("UrlPath", "network", "URL path."),
    ENTRY("HttpMethod", "network", "HTTP method."),
    ENTRY("NetworkClient", "network", "HTTP/network client."),
    ENTRY("HttpRequest", "network", "Request construction."),
    ENTRY("HttpResponse", "network", "Response handling."),
    ENTRY("HeaderKey", "network", "HTTP header key."),
    ENTRY("QueryParameter", "network", "Query parameter."),
    ENTRY("RequestBody", "network", "Request body model."),
    ENTRY("ResponseModel", "network", "Response model."),
    ENTRY("Interceptor", "network", "HTTP interceptor."),
    ENTRY("Authenticator", "network", "HTTP authenticator."),
    ENTRY("WebSocketEndpoint", "network", "WebSocket endpoint."),
    ENTRY("GraphQLEndpoint", "network", "GraphQL endpoint."),
    ENTRY("GrpcService", "network", "gRPC service."),
    ENTRY("SocketEvent", "network", "Socket.IO or event-stream event."),

    /* Storage/data */
    ENTRY("SharedPreferenceFile", "storage", "SharedPreferences file."),
    ENTRY("StorageKey", "storage", "Preference/DataStore/session/cache key."),
    ENTRY("Database", "storage", "Local database."),
    ENTRY("DatabaseTable", "storage", "Local database table."),
    ENTRY("DatabaseColumn", "storage", "Local database column."),
    ENTRY("SqlQuery", "storage", "SQL query."),
    ENTRY("RoomDao", "storage", "Room DAO."),
    ENTRY("RoomEntity", "storage", "Room entity."),
    ENTRY("DataStoreKey", "storage", "Jetpack DataStore key."),
    ENTRY("FileStoragePath", "storage", "File path used by app storage."),
    ENTRY("CacheDirectory", "storage", "Cache directory."),
    ENTRY("ExternalStorageUse", "storage", "External storage usage."),
    ENTRY("ContentProviderAccess", "storage", "Content provider access."),

    /* Auth/session/payment */
    ENTRY("AuthFlow", "auth", "Authentication flow candidate."),
    ENTRY("LoginFlow", "auth", "Login flow candidate."),
    ENTRY("SignupFlow", "auth", "Signup flow candidate."),
    ENTRY("SessionToken", "auth", "Session token reference."),
    ENTRY("RefreshToken", "auth", "Refresh token reference."),
    ENTRY("AccessToken", "auth", "Access token reference."),
    ENTRY("OtpFlow", "auth", "OTP flow candidate."),
    ENTRY("BiometricAuth", "auth", "Biometric authentication usage."),
    ENTRY("PaymentFlow", "payment", "Payment flow candidate."),
    ENTRY("PaymentProvider", "payment", "Payment provider reference."),
    ENTRY("PaymentToken", "payment", "Payment token reference."),
    ENTRY("Wallet", "payment", "Wallet feature reference."),
    ENTRY("PromoCode", "commerce", "Promo code reference."),
    ENTRY("Coupon", "commerce", "Coupon reference."),
    ENTRY("LoyaltyPoint", "commerce", "Loyalty point reference."),

    /* Location/maps/delivery */
    ENTRY("LocationUse", "location", "Generic location use."),
    ENTRY("GpsProvider", "location", "GPS provider."),
    ENTRY("FusedLocationProvider", "location", "Fused location provider."),
    ENTRY("Geofence", "location", "Geofence usage."),
    ENTRY("MapView", "location", "Map view or map SDK usage."),
    ENTRY("RoutePolyline", "location", "Route/polyline usage."),
    ENTRY("AddressModel", "delivery", "Address model."),
    ENTRY("CityModel", "delivery", "City model."),
    ENTRY("ZoneModel", "delivery", "Zone model."),
    ENTRY("DeliveryArea", "delivery", "Delivery area model/concept."),
    ENTRY("RiderLocation", "delivery", "Rider location model/concept."),
    ENTRY("CustomerLocation", "delivery", "Customer location model/concept."),
    ENTRY("RestaurantLocation", "delivery", "Restaurant location model/concept."),
    ENTRY("DistanceCalculation", "delivery", "Distance calculation logic."),
    ENTRY("EtaCalculation", "delivery", "ETA calculation logic."),

    /* Security/integrity */
    ENTRY("CryptoUse", "security", "Generic crypto use."),
    ENTRY("CipherUse", "security", "Cipher API use."),
    ENTRY("HashUse", "security", "Hash API use."),
    ENTRY("SignatureUse", "security", "Signature API use."),
    ENTRY("KeyStoreUse", "security", "Android/Java keystore use."),
    ENTRY("CertificatePinning", "security", "Certificate pinning evidence."),
    ENTRY("NetworkSecurityConfig", "security", "Network security config."),
    ENTRY("RootCheck", "security", "Root-detection evidence."),
    ENTRY("EmulatorCheck", "security", "Emulator-detection evidence."),
    ENTRY("DebuggerCheck", "security", "Debugger-detection evidence."),
    ENTRY("HookingCheck", "security", "Hooking/instrumentation-detection evidence."),
    ENTRY("TamperCheck", "security", "Tamper-detection evidence."),
    ENTRY("IntegrityCheck", "security", "Generic integrity check."),
    ENTRY("PlayIntegrityUse", "security", "Play Integrity API use."),
    ENTRY("SafetyNetUse", "security", "SafetyNet API use."),
    ENTRY("AntiFridaSignal", "security", "Frida-detection signal."),
    ENTRY("AntiXposedSignal", "security", "Xposed-detection signal."),

    /* Dynamic behavior */
    ENTRY("ReflectionUse", "dynamic", "Generic reflection use."),
    ENTRY("ClassForNameUse", "dynamic", "Class.forName-style usage."),
    ENTRY("MethodInvokeUse", "dynamic", "Reflective method invocation."),
    ENTRY("DynamicDexLoad", "dynamic", "Dynamic dex loading."),
    ENTRY("DexClassLoaderUse", "dynamic", "DexClassLoader usage."),
    ENTRY("PathClassLoaderUse", "dynamic", "PathClassLoader usage."),
    ENTRY("NativeLibraryLoad", "dynamic", "Native library load."),
    ENTRY("JniMethod", "dynamic", "JNI method."),
    ENTRY("WebViewUse", "dynamic", "WebView usage."),
    ENTRY("JavaScriptBridge", "dynamic", "WebView JavaScript bridge."),
    ENTRY("JavaScriptInterface", "dynamic", "addJavascriptInterface target."),
    ENTRY("RuntimeCommand", "dynamic", "Runtime command execution evidence."),

    /* Obfuscation */
    ENTRY("ObfuscatedClass", "obfuscation", "Class likely affected by obfuscation."),
    ENTRY("ObfuscatedMethod", "obfuscation", "Method likely affected by obfuscation."),
    ENTRY("ObfuscatedField", "obfuscation", "Field likely affected by obfuscation."),
    ENTRY("NameCluster", "obfuscation", "Cluster of similarly named obfuscated symbols."),
    ENTRY("StringCluster", "obfuscation", "Cluster of related string evidence."),
    ENTRY("ControlFlowHotspot", "obfuscation", "Method or class with dense control flow."),
    ENTRY("HighFanInMethod", "obfuscation", "Method with high inbound call degree."),
    ENTRY("HighFanOutMethod", "obfuscation", "Method with high outbound call degree."),
    ENTRY("BridgeMethod", "obfuscation", "Bridge method."),
    ENTRY("SyntheticMethod", "obfuscation", "Synthetic method."),
    ENTRY("AnonymousClass", "obfuscation", "Anonymous class."),
    ENTRY("GeneratedClass", "obfuscation", "Generated class."),

    /* Business/domain documentation candidates */
    ENTRY("BusinessFlow", "business", "Business flow candidate inferred from client-side evidence."),
    ENTRY("BusinessEntity", "business", "Business entity candidate."),
    ENTRY("BusinessRule", "business", "Business rule candidate."),
    ENTRY("FeatureFlag", "business", "Feature flag."),
    ENTRY("ExperimentFlag", "business", "Experiment or A/B flag."),
    ENTRY("Screen", "business", "Screen/UI route candidate."),
    ENTRY("ViewModelFlow", "business", "ViewModel-backed flow candidate."),
    ENTRY("UseCaseFlow", "business", "Use-case/interactor flow candidate."),
    ENTRY("RepositoryFlow", "business", "Repository/data flow candidate."),

    /* Evidence */
    ENTRY("Evidence", "evidence", "Generic evidence item."),
    ENTRY("EvidenceSource", "evidence", "Source artifact containing evidence."),
    ENTRY("MatchedPattern", "evidence", "Pattern/rule that matched evidence."),
    ENTRY("SourceLocation", "evidence", "File and line/range evidence."),
    ENTRY("ExtractionToolOutput", "evidence", "Reverse-engineering tool output metadata.")
};

static const cbm_android_vocab_entry_t EDGE_TYPES[] = {
    ENTRY("CONTAINS", "structure", "Containment relationship."),
    ENTRY("HAS_PACKAGE", "structure", "App has package namespace."),
    ENTRY("HAS_SPLIT", "structure", "App has APK split."),
    ENTRY("HAS_DEX", "structure", "App/artifact has DEX file."),
    ENTRY("HAS_MANIFEST", "structure", "App/artifact has manifest."),
    ENTRY("USES_APPLICATION_CLASS", "manifest", "Manifest/application uses an application class."),
    ENTRY("HAS_SIGNING_INFO", "app", "App has signing metadata."),
    ENTRY("HAS_CERTIFICATE", "app", "Signing metadata has certificate."),
    ENTRY("DECLARES_COMPONENT", "manifest", "Manifest declares Android component."),
    ENTRY("REQUESTS_PERMISSION", "permission", "App requests permission."),
    ENTRY("DEFINES_PERMISSION", "permission", "App defines permission."),
    ENTRY("HAS_INTENT_FILTER", "intent", "Component has intent filter."),
    ENTRY("HANDLES_ACTION", "intent", "Intent filter handles action."),
    ENTRY("HANDLES_CATEGORY", "intent", "Intent filter handles category."),
    ENTRY("HANDLES_DATA", "intent", "Intent filter handles data."),
    ENTRY("EXPOSES_DEEPLINK", "intent", "Component exposes deep link."),
    ENTRY("EXPOSES_APPLINK", "intent", "Component exposes verified app link."),
    ENTRY("HAS_METADATA", "manifest", "Node has metadata."),
    ENTRY("RUNS_IN_PROCESS", "manifest", "Component runs in process."),
    ENTRY("HAS_TASK_AFFINITY", "manifest", "Component has task affinity."),
    ENTRY("PROTECTED_BY_PERMISSION", "permission", "Component protected by permission."),
    ENTRY("EXPORTED_AS", "manifest", "Component has exported state."),
    ENTRY("ENABLED_AS", "manifest", "Component has enabled state."),
    ENTRY("HAS_RESOURCE", "resource", "App has resource."),
    ENTRY("CONTAINS_RESOURCE", "resource", "Resource file contains resource."),
    ENTRY("REFERENCES_RESOURCE", "resource", "Code/source references resource."),
    ENTRY("USES_STRING", "resource", "Code/source uses string resource."),
    ENTRY("USES_LAYOUT", "resource", "Code/source uses layout resource."),
    ENTRY("USES_DRAWABLE", "resource", "Code/source uses drawable resource."),
    ENTRY("USES_STYLE", "resource", "Code/source uses style resource."),
    ENTRY("USES_THEME", "resource", "Code/source uses theme resource."),
    ENTRY("USES_ASSET", "asset", "Code/source uses asset."),
    ENTRY("USES_XML_CONFIG", "resource", "Code/source uses XML config."),
    ENTRY("MAPS_TO_JADX", "source", "Source representation maps to JADX output."),
    ENTRY("MAPS_TO_SMALI", "source", "Source representation maps to smali output."),
    ENTRY("MAPS_TO_DEX", "source", "Source representation maps to DEX descriptor."),
    ENTRY("HAS_DESCRIPTOR", "source", "Symbol has JVM/Dalvik descriptor."),
    ENTRY("RENAMED_AS", "source", "Symbol was renamed as another symbol."),
    ENTRY("DEOBFUSCATED_AS", "source", "Symbol was deobfuscated as another symbol."),
    ENTRY("SAME_SYMBOL_AS", "source", "Two nodes represent the same underlying symbol."),
    ENTRY("DECLARES_METHOD", "source", "Class declares method."),
    ENTRY("DECLARES_FIELD", "source", "Class declares field."),
    ENTRY("BELONGS_TO_PACKAGE", "source", "Symbol belongs to package namespace."),
    ENTRY("CALLS_ANDROID_API", "framework", "Code calls Android framework API."),
    ENTRY("IMPLEMENTS_CALLBACK", "framework", "Code implements callback."),
    ENTRY("OVERRIDES_LIFECYCLE", "framework", "Code overrides lifecycle method."),
    ENTRY("REGISTERS_LISTENER", "framework", "Code registers listener."),
    ENTRY("SCHEDULES_WORK", "framework", "Code schedules work."),
    ENTRY("REGISTERS_ALARM", "framework", "Code registers alarm."),
    ENTRY("CREATES_PENDING_INTENT", "framework", "Code creates pending intent."),
    ENTRY("CREATES_NOTIFICATION_CHANNEL", "framework", "Code creates notification channel."),
    ENTRY("USES_CONTENT_URI", "framework", "Code uses content URI."),
    ENTRY("CALLS_ENDPOINT", "network", "Code calls endpoint."),
    ENTRY("BUILDS_URL", "network", "Code builds URL."),
    ENTRY("USES_BASE_URL", "network", "Code uses base URL."),
    ENTRY("USES_HTTP_METHOD", "network", "Code uses HTTP method."),
    ENTRY("USES_NETWORK_CLIENT", "network", "Code uses network client."),
    ENTRY("SETS_HEADER", "network", "Code sets HTTP header."),
    ENTRY("USES_QUERY_PARAM", "network", "Code uses query parameter."),
    ENTRY("SENDS_BODY", "network", "Code sends request body."),
    ENTRY("RECEIVES_MODEL", "network", "Code receives/deserializes model."),
    ENTRY("USES_INTERCEPTOR", "network", "Code uses network interceptor."),
    ENTRY("USES_AUTHENTICATOR", "network", "Code uses network authenticator."),
    ENTRY("OPENS_WEBSOCKET", "network", "Code opens WebSocket."),
    ENTRY("EMITS_SOCKET_EVENT", "network", "Code emits socket/event-stream event."),
    ENTRY("LISTENS_SOCKET_EVENT", "network", "Code listens for socket/event-stream event."),
    ENTRY("CALLS_GRAPHQL", "network", "Code calls GraphQL endpoint."),
    ENTRY("CALLS_GRPC", "network", "Code calls gRPC service."),
    ENTRY("READS_PREFERENCE", "storage", "Code reads SharedPreferences/DataStore key."),
    ENTRY("WRITES_PREFERENCE", "storage", "Code writes SharedPreferences/DataStore key."),
    ENTRY("USES_DATASTORE_KEY", "storage", "Code uses Jetpack DataStore key."),
    ENTRY("OPENS_DATABASE", "storage", "Code opens local database."),
    ENTRY("READS_DATABASE", "storage", "Code reads database/table."),
    ENTRY("WRITES_DATABASE", "storage", "Code writes database/table."),
    ENTRY("USES_TABLE", "storage", "Code uses database table."),
    ENTRY("USES_COLUMN", "storage", "Code uses database column."),
    ENTRY("RUNS_SQL_QUERY", "storage", "Code runs SQL query."),
    ENTRY("USES_ROOM_DAO", "storage", "Code uses Room DAO."),
    ENTRY("USES_ROOM_ENTITY", "storage", "Code uses Room entity."),
    ENTRY("READS_FILE", "storage", "Code reads file."),
    ENTRY("WRITES_FILE", "storage", "Code writes file."),
    ENTRY("USES_CACHE", "storage", "Code uses cache."),
    ENTRY("USES_EXTERNAL_STORAGE", "storage", "Code uses external storage."),
    ENTRY("ACCESSES_CONTENT_PROVIDER", "storage", "Code accesses content provider."),
    ENTRY("INDICATES_AUTH_FLOW", "auth", "Evidence indicates auth flow."),
    ENTRY("USES_SESSION_TOKEN", "auth", "Code/evidence uses session token."),
    ENTRY("USES_REFRESH_TOKEN", "auth", "Code/evidence uses refresh token."),
    ENTRY("USES_ACCESS_TOKEN", "auth", "Code/evidence uses access token."),
    ENTRY("USES_OTP", "auth", "Code/evidence uses OTP."),
    ENTRY("USES_BIOMETRIC_AUTH", "auth", "Code uses biometric auth."),
    ENTRY("INDICATES_PAYMENT_FLOW", "payment", "Evidence indicates payment flow."),
    ENTRY("USES_PAYMENT_PROVIDER", "payment", "Code/evidence uses payment provider."),
    ENTRY("USES_PAYMENT_TOKEN", "payment", "Code/evidence uses payment token."),
    ENTRY("USES_WALLET", "payment", "Code/evidence uses wallet."),
    ENTRY("USES_PROMO_CODE", "commerce", "Code/evidence uses promo code."),
    ENTRY("USES_COUPON", "commerce", "Code/evidence uses coupon."),
    ENTRY("USES_LOYALTY", "commerce", "Code/evidence uses loyalty points."),
    ENTRY("USES_LOCATION", "location", "Code uses location."),
    ENTRY("REQUESTS_LOCATION_UPDATES", "location", "Code requests location updates."),
    ENTRY("USES_GEOFENCE", "location", "Code uses geofence."),
    ENTRY("USES_MAP", "location", "Code uses map."),
    ENTRY("DRAWS_ROUTE", "location", "Code draws route/polyline."),
    ENTRY("USES_ADDRESS", "delivery", "Code/evidence uses address."),
    ENTRY("USES_CITY", "delivery", "Code/evidence uses city."),
    ENTRY("USES_ZONE", "delivery", "Code/evidence uses zone."),
    ENTRY("USES_DELIVERY_AREA", "delivery", "Code/evidence uses delivery area."),
    ENTRY("USES_RIDER_LOCATION", "delivery", "Code/evidence uses rider location."),
    ENTRY("USES_CUSTOMER_LOCATION", "delivery", "Code/evidence uses customer location."),
    ENTRY("USES_RESTAURANT_LOCATION", "delivery", "Code/evidence uses restaurant location."),
    ENTRY("CALCULATES_DISTANCE", "delivery", "Code calculates distance."),
    ENTRY("CALCULATES_ETA", "delivery", "Code calculates ETA."),
    ENTRY("USES_CRYPTO", "security", "Code uses crypto."),
    ENTRY("USES_CIPHER", "security", "Code uses cipher."),
    ENTRY("USES_HASH", "security", "Code uses hash."),
    ENTRY("USES_SIGNATURE", "security", "Code uses signature API."),
    ENTRY("USES_KEYSTORE", "security", "Code uses keystore."),
    ENTRY("PINS_CERTIFICATE", "security", "Code/evidence pins certificate."),
    ENTRY("USES_NETWORK_SECURITY_CONFIG", "security", "Code/evidence uses network security config."),
    ENTRY("CHECKS_ROOT", "security", "Code/evidence checks root state."),
    ENTRY("CHECKS_EMULATOR", "security", "Code/evidence checks emulator state."),
    ENTRY("CHECKS_DEBUGGER", "security", "Code/evidence checks debugger state."),
    ENTRY("CHECKS_HOOKING", "security", "Code/evidence checks instrumentation/hooking state."),
    ENTRY("CHECKS_TAMPER", "security", "Code/evidence checks tampering."),
    ENTRY("USES_PLAY_INTEGRITY", "security", "Code/evidence uses Play Integrity."),
    ENTRY("USES_SAFETYNET", "security", "Code/evidence uses SafetyNet."),
    ENTRY("INDICATES_SECURITY_CHECK", "security", "Evidence indicates security check."),
    ENTRY("USES_REFLECTION", "dynamic", "Code uses reflection."),
    ENTRY("USES_CLASS_FOR_NAME", "dynamic", "Code uses Class.forName."),
    ENTRY("INVOKES_METHOD_REFLECTIVELY", "dynamic", "Code invokes method reflectively."),
    ENTRY("LOADS_DEX", "dynamic", "Code loads DEX dynamically."),
    ENTRY("USES_DEX_CLASS_LOADER", "dynamic", "Code uses DexClassLoader."),
    ENTRY("USES_PATH_CLASS_LOADER", "dynamic", "Code uses PathClassLoader."),
    ENTRY("LOADS_NATIVE_LIBRARY", "dynamic", "Code loads native library."),
    ENTRY("CALLS_JNI", "dynamic", "Code calls JNI."),
    ENTRY("USES_WEBVIEW", "dynamic", "Code uses WebView."),
    ENTRY("EXPOSES_JS_BRIDGE", "dynamic", "Code exposes JavaScript bridge."),
    ENTRY("EXPOSES_JS_INTERFACE", "dynamic", "Code exposes JavaScript interface."),
    ENTRY("RUNS_RUNTIME_COMMAND", "dynamic", "Code runs runtime command."),
    ENTRY("OBFUSCATED_AS", "obfuscation", "Symbol is represented as obfuscated symbol."),
    ENTRY("BELONGS_TO_NAME_CLUSTER", "obfuscation", "Symbol belongs to name cluster."),
    ENTRY("BELONGS_TO_STRING_CLUSTER", "obfuscation", "Symbol belongs to string cluster."),
    ENTRY("HAS_HIGH_FAN_IN", "obfuscation", "Method has high inbound degree."),
    ENTRY("HAS_HIGH_FAN_OUT", "obfuscation", "Method has high outbound degree."),
    ENTRY("BRIDGES_TO", "obfuscation", "Bridge method connects to target."),
    ENTRY("SYNTHETIC_FOR", "obfuscation", "Synthetic method/field for target."),
    ENTRY("GENERATED_FROM", "obfuscation", "Generated code derived from source."),
    ENTRY("INDICATES_BUSINESS_FLOW", "business", "Evidence indicates business flow."),
    ENTRY("USES_BUSINESS_ENTITY", "business", "Code/evidence uses business entity."),
    ENTRY("ENFORCES_BUSINESS_RULE", "business", "Code/evidence enforces business rule."),
    ENTRY("USES_FEATURE_FLAG", "business", "Code/evidence uses feature flag."),
    ENTRY("USES_EXPERIMENT_FLAG", "business", "Code/evidence uses experiment flag."),
    ENTRY("BACKS_SCREEN", "business", "Code/evidence backs screen."),
    ENTRY("USES_VIEWMODEL_FLOW", "business", "Code/evidence uses ViewModel flow."),
    ENTRY("USES_USECASE_FLOW", "business", "Code/evidence uses use-case flow."),
    ENTRY("USES_REPOSITORY_FLOW", "business", "Code/evidence uses repository flow."),
    ENTRY("EVIDENCE_FROM", "evidence", "Node or edge is supported by evidence."),
    ENTRY("MATCHED_BY_PATTERN", "evidence", "Evidence matched by pattern."),
    ENTRY("FOUND_IN_FILE", "evidence", "Evidence found in file."),
    ENTRY("FOUND_AT_LOCATION", "evidence", "Evidence found at source location."),
    ENTRY("DERIVED_FROM_TOOL_OUTPUT", "evidence", "Evidence derived from RE tool output.")
};

static const cbm_android_vocab_entry_t PROPERTIES[] = {
    ENTRY("android_kind", "common", "Specific Android concept."),
    ENTRY("category", "common", "Grouping category."),
    ENTRY("source_file", "evidence", "Relative file path containing evidence."),
    ENTRY("source_format", "evidence", "Evidence source format."),
    ENTRY("source_tool", "evidence", "External tool that produced the artifact if known."),
    ENTRY("extractor", "evidence", "Extractor/rule that emitted the graph fact."),
    ENTRY("confidence", "evidence", "high, medium, or low confidence."),
    ENTRY("evidence_count", "evidence", "Number of evidence points."),
    ENTRY("package_name", "app", "Android package name."),
    ENTRY("split_name", "artifact", "Split APK/config name."),
    ENTRY("dex_name", "artifact", "DEX source name."),
    ENTRY("start_line", "evidence", "Evidence start line."),
    ENTRY("end_line", "evidence", "Evidence end line."),
    ENTRY("value_preview", "evidence", "Short safe preview of value."),
    ENTRY("value_hash", "evidence", "Hash of sensitive/large value."),
    ENTRY("reason", "evidence", "Machine-readable inference reason."),
    ENTRY("notes", "common", "Human-readable note."),
    ENTRY("version_name", "app", "Android versionName."),
    ENTRY("version_code", "app", "Android versionCode."),
    ENTRY("min_sdk", "app", "Minimum SDK."),
    ENTRY("target_sdk", "app", "Target SDK."),
    ENTRY("compile_sdk", "app", "Compile SDK if known."),
    ENTRY("app_label", "app", "Application label."),
    ENTRY("debuggable", "app", "Debuggable manifest flag."),
    ENTRY("allow_backup", "app", "allowBackup manifest flag."),
    ENTRY("uses_cleartext_traffic", "app", "usesCleartextTraffic flag."),
    ENTRY("network_security_config", "security", "Network security config reference."),
    ENTRY("component_name", "manifest", "Manifest component name."),
    ENTRY("qualified_name", "manifest", "Fully qualified name."),
    ENTRY("exported", "manifest", "Exported state."),
    ENTRY("enabled", "manifest", "Enabled state."),
    ENTRY("permission", "permission", "Permission string."),
    ENTRY("process", "manifest", "Android process name."),
    ENTRY("task_affinity", "manifest", "Task affinity."),
    ENTRY("launch_mode", "manifest", "Activity launch mode."),
    ENTRY("scheme", "intent", "URI scheme."),
    ENTRY("host", "intent", "URI/API host."),
    ENTRY("port", "intent", "URI port."),
    ENTRY("path", "intent", "URI/API path."),
    ENTRY("path_prefix", "intent", "URI path prefix."),
    ENTRY("path_pattern", "intent", "URI path pattern."),
    ENTRY("mime_type", "intent", "MIME type."),
    ENTRY("auto_verify", "intent", "App link autoVerify flag."),
    ENTRY("resource_type", "resource", "Resource type."),
    ENTRY("resource_name", "resource", "Resource name."),
    ENTRY("resource_id", "resource", "Resource ID."),
    ENTRY("locale", "resource", "Resource locale."),
    ENTRY("is_sensitive_candidate", "resource", "Value may be sensitive."),
    ENTRY("descriptor", "source", "JVM/Dalvik descriptor."),
    ENTRY("access_flags", "source", "Access flags."),
    ENTRY("class_name", "source", "Class name."),
    ENTRY("method_signature", "source", "Method signature."),
    ENTRY("return_type", "source", "Return type."),
    ENTRY("parameter_types", "source", "Parameter types."),
    ENTRY("register_count", "source", "Smali register count."),
    ENTRY("line_count", "source", "Source line count."),
    ENTRY("mapping_reason", "source", "Reason source mapping was made."),
    ENTRY("mapping_confidence", "source", "Confidence for source mapping."),
    ENTRY("api_class", "framework", "Android/framework API class."),
    ENTRY("api_method", "framework", "Android/framework API method."),
    ENTRY("callback_type", "framework", "Callback type."),
    ENTRY("lifecycle_stage", "framework", "Lifecycle stage."),
    ENTRY("url", "network", "URL."),
    ENTRY("method", "network", "HTTP method."),
    ENTRY("library", "network", "Network/client library."),
    ENTRY("retrofit_annotation", "network", "Retrofit annotation."),
    ENTRY("okhttp_usage", "network", "OkHttp usage pattern."),
    ENTRY("header_name", "network", "HTTP header name."),
    ENTRY("parameter_name", "network", "Parameter name."),
    ENTRY("model_type", "network", "Model type."),
    ENTRY("key_name", "storage", "Storage key name."),
    ENTRY("database_name", "storage", "Database name."),
    ENTRY("table_name", "storage", "Table name."),
    ENTRY("column_name", "storage", "Column name."),
    ENTRY("query_preview", "storage", "Safe SQL query preview."),
    ENTRY("operation", "storage", "Read/write/query operation."),
    ENTRY("storage_scope", "storage", "Storage scope."),
    ENTRY("source_api", "storage", "Source API that caused storage fact."),
    ENTRY("provider", "location", "Location/payment/provider name."),
    ENTRY("update_interval_hint", "location", "Location update interval hint."),
    ENTRY("accuracy_hint", "location", "Location accuracy hint."),
    ENTRY("permission_required", "location", "Permission required for operation."),
    ENTRY("model_name", "business", "Model/entity name."),
    ENTRY("endpoint_hint", "business", "Endpoint hint."),
    ENTRY("token_name", "auth", "Token name."),
    ENTRY("flow_type", "business", "Flow type."),
    ENTRY("api_name", "security", "Security/dynamic API name."),
    ENTRY("algorithm", "security", "Crypto algorithm."),
    ENTRY("check_type", "security", "Integrity/security check type."),
    ENTRY("matched_indicator", "security", "Matched security indicator."),
    ENTRY("target_string", "dynamic", "Dynamic target string."),
    ENTRY("library_name", "dynamic", "Native library name."),
    ENTRY("class_name_hint", "dynamic", "Class name hint."),
    ENTRY("method_name_hint", "dynamic", "Method name hint."),
    ENTRY("name_length", "obfuscation", "Symbol name length."),
    ENTRY("symbol_pattern", "obfuscation", "Symbol naming pattern."),
    ENTRY("fan_in", "obfuscation", "Inbound call degree."),
    ENTRY("fan_out", "obfuscation", "Outbound call degree."),
    ENTRY("string_count", "obfuscation", "Number of string constants."),
    ENTRY("api_call_count", "obfuscation", "Number of API calls."),
    ENTRY("is_synthetic", "obfuscation", "Synthetic flag."),
    ENTRY("is_bridge", "obfuscation", "Bridge flag."),
    ENTRY("matched_terms", "business", "Matched terms for business inference."),
    ENTRY("endpoint_count", "business", "Endpoint evidence count."),
    ENTRY("storage_key_count", "business", "Storage key evidence count."),
    ENTRY("resource_string_count", "business", "Resource string evidence count."),
    ENTRY("warning", "business", "Warning such as client_side_inference_only."),
    ENTRY("pattern_name", "evidence", "Pattern name."),
    ENTRY("matched_text_preview", "evidence", "Safe preview of matched text."),
    ENTRY("extractor_name", "evidence", "Extractor name."),
    ENTRY("created_at", "evidence", "Creation timestamp.")
};

static const cbm_android_vocab_entry_t CATEGORIES[] = {
    ENTRY("auth", "business", "Authentication and authorization."),
    ENTRY("signup", "business", "Signup flow."),
    ENTRY("login", "business", "Login flow."),
    ENTRY("logout", "business", "Logout flow."),
    ENTRY("otp", "business", "OTP flow."),
    ENTRY("profile", "business", "Profile flow."),
    ENTRY("address", "business", "Address flow."),
    ENTRY("location", "business", "Location flow."),
    ENTRY("restaurant_listing", "business", "Restaurant listing flow."),
    ENTRY("restaurant_detail", "business", "Restaurant detail flow."),
    ENTRY("menu", "business", "Menu flow."),
    ENTRY("cart", "business", "Cart flow."),
    ENTRY("checkout", "business", "Checkout flow."),
    ENTRY("payment", "business", "Payment flow."),
    ENTRY("wallet", "business", "Wallet flow."),
    ENTRY("promo", "business", "Promo flow."),
    ENTRY("coupon", "business", "Coupon flow."),
    ENTRY("loyalty", "business", "Loyalty flow."),
    ENTRY("order_create", "business", "Order creation flow."),
    ENTRY("order_tracking", "business", "Order tracking flow."),
    ENTRY("order_cancel", "business", "Order cancellation flow."),
    ENTRY("refund", "business", "Refund flow."),
    ENTRY("rating", "business", "Rating flow."),
    ENTRY("review", "business", "Review flow."),
    ENTRY("chat", "business", "Chat flow."),
    ENTRY("support", "business", "Support flow."),
    ENTRY("rider_assignment", "business", "Rider assignment flow."),
    ENTRY("rider_tracking", "business", "Rider tracking flow."),
    ENTRY("delivery_eta", "business", "Delivery ETA flow."),
    ENTRY("fraud_check", "business", "Fraud-check evidence category."),
    ENTRY("device_integrity", "business", "Device-integrity evidence category."),
    ENTRY("notification", "business", "Notification flow."),
    ENTRY("analytics", "business", "Analytics flow."),
    ENTRY("network", "technical", "Network/API evidence."),
    ENTRY("storage", "technical", "Local storage/data evidence."),
    ENTRY("security", "technical", "Security/integrity evidence."),
    ENTRY("dynamic", "technical", "Reflection/native/dynamic boundary evidence."),
    ENTRY("obfuscation", "technical", "Obfuscation evidence."),
    ENTRY("unknown", "business", "Unknown or uncategorized flow.")
};

static const cbm_android_vocab_entry_t SOURCE_FORMATS[] = {
    ENTRY("manifest", "source_format", "AndroidManifest XML evidence."),
    ENTRY("xml_resource", "source_format", "Android XML resource evidence."),
    ENTRY("jadx_java", "source_format", "JADX Java source evidence."),
    ENTRY("jadx_kotlin", "source_format", "JADX Kotlin source evidence."),
    ENTRY("smali", "source_format", "Smali source evidence."),
    ENTRY("dex_metadata", "source_format", "DEX metadata evidence."),
    ENTRY("asset", "source_format", "Asset evidence."),
    ENTRY("native_lib", "source_format", "Native library evidence."),
    ENTRY("tool_output", "source_format", "External RE tool output evidence."),
    ENTRY("unknown", "source_format", "Unknown evidence format.")
};

static const cbm_android_vocab_entry_t CONFIDENCE_VALUES[] = {
    ENTRY("high", "confidence", "Direct declaration or exact API/pattern evidence."),
    ENTRY("medium", "confidence", "Strong code pattern, repeated evidence, or descriptor match."),
    ENTRY("low", "confidence", "Weak naming/string/category inference that needs manual review.")
};

static bool contains_name(const cbm_android_vocab_entry_t *entries, size_t count, const char *name) {
    if (!name) {
        return false;
    }
    for (size_t i = 0; i < count; i++) {
        if (entries[i].name && strcmp(entries[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

const cbm_android_vocab_entry_t *cbm_android_vocab_node_labels(size_t *count) {
    if (count) {
        *count = ARRAY_COUNT(NODE_LABELS);
    }
    return NODE_LABELS;
}

const cbm_android_vocab_entry_t *cbm_android_vocab_edge_types(size_t *count) {
    if (count) {
        *count = ARRAY_COUNT(EDGE_TYPES);
    }
    return EDGE_TYPES;
}

const cbm_android_vocab_entry_t *cbm_android_vocab_properties(size_t *count) {
    if (count) {
        *count = ARRAY_COUNT(PROPERTIES);
    }
    return PROPERTIES;
}

const cbm_android_vocab_entry_t *cbm_android_vocab_categories(size_t *count) {
    if (count) {
        *count = ARRAY_COUNT(CATEGORIES);
    }
    return CATEGORIES;
}

const cbm_android_vocab_entry_t *cbm_android_vocab_source_formats(size_t *count) {
    if (count) {
        *count = ARRAY_COUNT(SOURCE_FORMATS);
    }
    return SOURCE_FORMATS;
}

const cbm_android_vocab_entry_t *cbm_android_vocab_confidence_values(size_t *count) {
    if (count) {
        *count = ARRAY_COUNT(CONFIDENCE_VALUES);
    }
    return CONFIDENCE_VALUES;
}

bool cbm_android_vocab_is_node_label(const char *name) {
    return contains_name(NODE_LABELS, ARRAY_COUNT(NODE_LABELS), name);
}

bool cbm_android_vocab_is_edge_type(const char *name) {
    return contains_name(EDGE_TYPES, ARRAY_COUNT(EDGE_TYPES), name);
}

bool cbm_android_vocab_is_property(const char *name) {
    return contains_name(PROPERTIES, ARRAY_COUNT(PROPERTIES), name);
}

bool cbm_android_vocab_is_category(const char *name) {
    return contains_name(CATEGORIES, ARRAY_COUNT(CATEGORIES), name);
}

bool cbm_android_vocab_is_source_format(const char *name) {
    return contains_name(SOURCE_FORMATS, ARRAY_COUNT(SOURCE_FORMATS), name);
}

bool cbm_android_vocab_is_confidence(const char *name) {
    return contains_name(CONFIDENCE_VALUES, ARRAY_COUNT(CONFIDENCE_VALUES), name);
}
