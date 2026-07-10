# Android APK/XAPK graph vocabulary

This document defines a path-agnostic Android APK/XAPK vocabulary for `codebase-memory-mcp`.

The purpose is to make already-extracted Android app projects queryable as a graph after external reverse-engineering tools have produced readable artifacts such as Java/Kotlin source, smali, manifest XML, resources, assets, native library listings, or split APK folders.

This vocabulary does not define an APK/XAPK unpacker, decompiler, dynamic analysis tool, malware verdict system, bypass system, or fixed directory layout. It only defines graph labels, edge types, properties, categories, and evidence conventions that fit the existing `codebase-memory-mcp` model of node labels, edge types, and JSON properties.

For command examples after indexing an already-extracted Android folder, see [Android APK/XAPK query examples](android-apk-xapk-query-examples.md).

## Scope

The Android vocabulary is for repositories or folders that may contain any mix of:

- AndroidManifest.xml files
- Decompiled Java or Kotlin source
- Smali files
- Resource XML files
- Assets
- Native library filenames or metadata
- Split APK/XAPK extracted content
- Tool-produced notes or metadata

The vocabulary must remain:

- Path-agnostic: do not require a specific folder name such as `jadx-src`, `apktool-out`, or `smali_classes2`.
- Tool-agnostic: do not require one specific reverse-engineering tool.
- Evidence-first: every inferred Android-specific node or edge should carry source/evidence properties.
- Query-first: labels, edge types, and properties should make documentation and code navigation easier.

## Common Android properties

Use these property keys consistently on Android-specific nodes and edges when available.

| Property | Meaning |
| --- | --- |
| `android_kind` | Specific Android concept, such as `activity`, `endpoint`, `storage_key`, or `root_check`. |
| `category` | Human-useful category for grouping, such as `network`, `storage`, `auth`, or `order_tracking`. |
| `source_file` | Relative file path where the evidence was found. |
| `source_format` | Evidence format: `manifest`, `xml_resource`, `jadx_java`, `jadx_kotlin`, `smali`, `dex_metadata`, `asset`, `native_lib`, `tool_output`, or `unknown`. |
| `source_tool` | Tool that produced the source artifact if known, for example `jadx`, `apktool`, `baksmali`, or `unknown`. |
| `extractor` | Name of the codebase-memory extractor or rule that produced the node or edge. |
| `confidence` | `high`, `medium`, or `low`. |
| `evidence_count` | Number of evidence points supporting the node or edge. |
| `package_name` | Android application package name if known. |
| `split_name` | Split APK/config name if known. |
| `dex_name` | DEX source name if known, such as `classes.dex` or `classes2.dex`. |
| `start_line` | Start line for evidence when available. |
| `end_line` | End line for evidence when available. |
| `value_preview` | Short safe preview of a discovered value. |
| `value_hash` | Hash of a value when storing the full value is undesirable. |
| `reason` | Short machine-readable reason for the inference. |
| `notes` | Human-readable note when needed. |

Confidence meanings:

- `high`: direct declaration or exact API/pattern evidence, such as manifest component declaration or Retrofit annotation.
- `medium`: strong code pattern, repeated string evidence, descriptor match, or framework API usage.
- `low`: weak naming/string/category inference that needs manual review.

## Android app and artifact labels

Use these labels for app-level and artifact-level structure.

| Label | Description |
| --- | --- |
| `AndroidApp` | Logical Android application being indexed. |
| `AndroidPackage` | Android package namespace. |
| `ApkArtifact` | APK artifact or extracted APK unit. |
| `XapkArtifact` | XAPK container or extracted XAPK unit. |
| `ApkSplit` | Split APK or config APK. |
| `SplitConfig` | ABI, density, language, or feature split metadata. |
| `DexFile` | DEX file or DEX-equivalent source unit. |
| `OatFile` | OAT artifact metadata if present. |
| `VdexFile` | VDEX artifact metadata if present. |
| `Manifest` | Android manifest document. |
| `Application` | Manifest application node. |
| `ApplicationClass` | Custom application class. |
| `BuildConfig` | BuildConfig-like constants and metadata. |
| `SigningInfo` | Signing metadata if available from extracted metadata. |
| `Certificate` | Certificate metadata, fingerprint, or pin reference. |
| `AppMetadata` | Generic app metadata not represented by a more specific label. |
| `Evidence` | Evidence node used to explain a graph inference. |

Common edges:

- `CONTAINS`
- `HAS_PACKAGE`
- `HAS_SPLIT`
- `HAS_DEX`
- `HAS_MANIFEST`
- `USES_APPLICATION_CLASS`
- `HAS_SIGNING_INFO`
- `HAS_CERTIFICATE`
- `EVIDENCE_FROM`

## Manifest and component labels

Use these labels for AndroidManifest-derived structure.

| Label | Description |
| --- | --- |
| `Activity` | Manifest-declared activity. |
| `Service` | Manifest-declared service. |
| `BroadcastReceiver` | Manifest-declared broadcast receiver. |
| `ContentProvider` | Manifest-declared content provider. |
| `Permission` | Android permission. |
| `UsesPermission` | Requested permission declaration. |
| `PermissionGroup` | Permission group. |
| `IntentFilter` | Intent filter. |
| `IntentAction` | Intent action. |
| `IntentCategory` | Intent category. |
| `IntentData` | Intent data element. |
| `DeepLink` | URI/deep-link pattern. |
| `AppLink` | Verified app link pattern. |
| `ManifestMetaData` | Manifest metadata entry. |
| `ProcessName` | Android process declaration. |
| `TaskAffinity` | Task affinity declaration. |

Edges:

- `DECLARES_COMPONENT`
- `REQUESTS_PERMISSION`
- `DEFINES_PERMISSION`
- `HAS_INTENT_FILTER`
- `HANDLES_ACTION`
- `HANDLES_CATEGORY`
- `HANDLES_DATA`
- `EXPOSES_DEEPLINK`
- `EXPOSES_APPLINK`
- `HAS_METADATA`
- `RUNS_IN_PROCESS`
- `HAS_TASK_AFFINITY`
- `PROTECTED_BY_PERMISSION`
- `EXPORTED_AS`
- `ENABLED_AS`

Important component properties:

- `component_name`
- `qualified_name`
- `exported`
- `enabled`
- `permission`
- `process`
- `task_affinity`
- `launch_mode`
- `scheme`
- `host`
- `port`
- `path`
- `path_prefix`
- `path_pattern`
- `mime_type`
- `auto_verify`

## Resource and asset labels

Use these labels for app resources and assets.

| Label | Description |
| --- | --- |
| `Resource` | Generic Android resource. |
| `ResourceFile` | Resource file. |
| `ResourceString` | String resource. |
| `ResourceId` | Resource identifier. |
| `LayoutResource` | Layout XML resource. |
| `DrawableResource` | Drawable resource. |
| `MenuResource` | Menu resource. |
| `NavigationResource` | Navigation graph resource. |
| `XmlConfig` | XML config file, including network/security/config resources. |
| `RawResource` | Raw resource file. |
| `Asset` | Asset file. |
| `FontAsset` | Font asset. |
| `LocaleResource` | Locale-specific resource. |
| `Theme` | Theme resource. |
| `Style` | Style resource. |
| `ColorResource` | Color resource. |
| `DimensionResource` | Dimension resource. |

Edges:

- `HAS_RESOURCE`
- `CONTAINS_RESOURCE`
- `REFERENCES_RESOURCE`
- `USES_STRING`
- `USES_LAYOUT`
- `USES_DRAWABLE`
- `USES_STYLE`
- `USES_THEME`
- `USES_ASSET`
- `USES_XML_CONFIG`

Important properties:

- `resource_type`
- `resource_name`
- `resource_id`
- `locale`
- `value_preview`
- `value_hash`
- `is_sensitive_candidate`

## Source and symbol mapping labels

Use these labels to connect Java/Kotlin/JADX/smali/DEX views of the same code.

| Label | Description |
| --- | --- |
| `JadxClass` | Decompiled class from Java/Kotlin source output. |
| `JadxMethod` | Decompiled method from Java/Kotlin source output. |
| `JadxField` | Decompiled field from Java/Kotlin source output. |
| `SmaliClass` | Smali class. |
| `SmaliMethod` | Smali method. |
| `SmaliField` | Smali field. |
| `DexClass` | DEX class descriptor. |
| `DexMethod` | DEX method descriptor. |
| `DexField` | DEX field descriptor. |
| `ObfuscatedSymbol` | Symbol likely affected by obfuscation. |
| `DeobfuscatedSymbol` | Manually or tool-renamed symbol. |
| `SourceMapping` | Mapping node between source representations. |
| `ClassDescriptor` | JVM/Dalvik class descriptor. |
| `MethodDescriptor` | JVM/Dalvik method descriptor. |
| `FieldDescriptor` | JVM/Dalvik field descriptor. |
| `PackageNamespace` | Java/Kotlin package namespace. |

Edges:

- `MAPS_TO_JADX`
- `MAPS_TO_SMALI`
- `MAPS_TO_DEX`
- `HAS_DESCRIPTOR`
- `RENAMED_AS`
- `DEOBFUSCATED_AS`
- `SAME_SYMBOL_AS`
- `DECLARES_METHOD`
- `DECLARES_FIELD`
- `BELONGS_TO_PACKAGE`

Important properties:

- `descriptor`
- `access_flags`
- `class_name`
- `method_signature`
- `return_type`
- `parameter_types`
- `register_count`
- `line_count`
- `mapping_reason`
- `mapping_confidence`

## Android framework/API labels

Use these labels for Android framework usage.

| Label | Description |
| --- | --- |
| `AndroidApiCall` | Android framework API call. |
| `FrameworkClass` | Android framework class. |
| `FrameworkMethod` | Android framework method. |
| `LifecycleMethod` | Android lifecycle method. |
| `CallbackMethod` | Callback method. |
| `Listener` | Listener/callback registration. |
| `Adapter` | Android adapter or UI adapter concept. |
| `ViewModel` | ViewModel-style class. |
| `RepositoryClass` | Repository/data access class. |
| `Worker` | WorkManager worker or similar background worker. |
| `JobService` | JobService. |
| `Alarm` | AlarmManager usage. |
| `PendingIntent` | PendingIntent usage. |
| `NotificationChannel` | Notification channel. |
| `BroadcastAction` | Broadcast action. |
| `ContentUri` | Content URI. |

Edges:

- `CALLS_ANDROID_API`
- `IMPLEMENTS_CALLBACK`
- `OVERRIDES_LIFECYCLE`
- `REGISTERS_LISTENER`
- `SCHEDULES_WORK`
- `REGISTERS_ALARM`
- `CREATES_PENDING_INTENT`
- `CREATES_NOTIFICATION_CHANNEL`
- `USES_CONTENT_URI`

## Network/API labels

Use these labels for app backend/API communication.

| Label | Description |
| --- | --- |
| `ApiEndpoint` | HTTP endpoint or URL path. |
| `BaseUrl` | Base URL. |
| `UrlPath` | URL path. |
| `HttpMethod` | HTTP method. |
| `NetworkClient` | HTTP/network client. |
| `HttpRequest` | Request construction. |
| `HttpResponse` | Response handling. |
| `HeaderKey` | HTTP header key. |
| `QueryParameter` | Query parameter. |
| `RequestBody` | Request body model. |
| `ResponseModel` | Response model. |
| `Interceptor` | HTTP interceptor. |
| `Authenticator` | HTTP authenticator. |
| `WebSocketEndpoint` | WebSocket endpoint. |
| `GraphQLEndpoint` | GraphQL endpoint. |
| `GrpcService` | gRPC service. |
| `SocketEvent` | Socket.IO or event-stream event. |

Edges:

- `CALLS_ENDPOINT`
- `BUILDS_URL`
- `USES_BASE_URL`
- `USES_HTTP_METHOD`
- `USES_NETWORK_CLIENT`
- `SETS_HEADER`
- `USES_QUERY_PARAM`
- `SENDS_BODY`
- `RECEIVES_MODEL`
- `USES_INTERCEPTOR`
- `USES_AUTHENTICATOR`
- `OPENS_WEBSOCKET`
- `EMITS_SOCKET_EVENT`
- `LISTENS_SOCKET_EVENT`
- `CALLS_GRAPHQL`
- `CALLS_GRPC`

Important properties:

- `url`
- `scheme`
- `host`
- `path`
- `method`
- `library`
- `retrofit_annotation`
- `okhttp_usage`
- `header_name`
- `parameter_name`
- `model_type`

## Storage and data labels

Use these labels for local app persistence.

| Label | Description |
| --- | --- |
| `SharedPreferenceFile` | SharedPreferences file. |
| `StorageKey` | Preference/DataStore/session/cache key. |
| `Database` | Local database. |
| `DatabaseTable` | Local database table. |
| `DatabaseColumn` | Local database column. |
| `SqlQuery` | SQL query. |
| `RoomDao` | Room DAO. |
| `RoomEntity` | Room entity. |
| `DataStoreKey` | Jetpack DataStore key. |
| `FileStoragePath` | File path used by app storage. |
| `CacheDirectory` | Cache directory. |
| `ExternalStorageUse` | External storage usage. |
| `ContentProviderAccess` | Content provider access. |

Edges:

- `READS_PREFERENCE`
- `WRITES_PREFERENCE`
- `USES_DATASTORE_KEY`
- `OPENS_DATABASE`
- `READS_DATABASE`
- `WRITES_DATABASE`
- `USES_TABLE`
- `USES_COLUMN`
- `RUNS_SQL_QUERY`
- `USES_ROOM_DAO`
- `USES_ROOM_ENTITY`
- `READS_FILE`
- `WRITES_FILE`
- `USES_CACHE`
- `USES_EXTERNAL_STORAGE`
- `ACCESSES_CONTENT_PROVIDER`

Important properties:

- `key_name`
- `database_name`
- `table_name`
- `column_name`
- `query_preview`
- `operation`
- `storage_scope`
- `source_api`

## Auth, session, and payment labels

Use these labels for auth/session/payment documentation candidates.

| Label | Description |
| --- | --- |
| `AuthFlow` | Authentication flow candidate. |
| `LoginFlow` | Login flow candidate. |
| `SignupFlow` | Signup flow candidate. |
| `SessionToken` | Session token reference. |
| `RefreshToken` | Refresh token reference. |
| `AccessToken` | Access token reference. |
| `OtpFlow` | OTP flow candidate. |
| `BiometricAuth` | Biometric authentication usage. |
| `PaymentFlow` | Payment flow candidate. |
| `PaymentProvider` | Payment provider reference. |
| `PaymentToken` | Payment token reference. |
| `Wallet` | Wallet feature reference. |
| `PromoCode` | Promo code reference. |
| `Coupon` | Coupon reference. |
| `LoyaltyPoint` | Loyalty point reference. |

Edges:

- `INDICATES_AUTH_FLOW`
- `USES_SESSION_TOKEN`
- `USES_REFRESH_TOKEN`
- `USES_ACCESS_TOKEN`
- `USES_OTP`
- `USES_BIOMETRIC_AUTH`
- `INDICATES_PAYMENT_FLOW`
- `USES_PAYMENT_PROVIDER`
- `USES_PAYMENT_TOKEN`
- `USES_WALLET`
- `USES_PROMO_CODE`
- `USES_COUPON`
- `USES_LOYALTY`

## Location, maps, and delivery labels

Use these labels for food delivery, rider, restaurant, customer, and location-heavy apps.

| Label | Description |
| --- | --- |
| `LocationUse` | Generic location use. |
| `GpsProvider` | GPS provider. |
| `FusedLocationProvider` | Fused location provider. |
| `Geofence` | Geofence usage. |
| `MapView` | Map view or map SDK usage. |
| `RoutePolyline` | Route/polyline usage. |
| `AddressModel` | Address model. |
| `CityModel` | City model. |
| `ZoneModel` | Zone model. |
| `DeliveryArea` | Delivery area model/concept. |
| `RiderLocation` | Rider location model/concept. |
| `CustomerLocation` | Customer location model/concept. |
| `RestaurantLocation` | Restaurant location model/concept. |
| `DistanceCalculation` | Distance calculation logic. |
| `EtaCalculation` | ETA calculation logic. |

Edges:

- `USES_LOCATION`
- `REQUESTS_LOCATION_UPDATES`
- `USES_GEOFENCE`
- `USES_MAP`
- `DRAWS_ROUTE`
- `USES_ADDRESS`
- `USES_CITY`
- `USES_ZONE`
- `USES_DELIVERY_AREA`
- `USES_RIDER_LOCATION`
- `USES_CUSTOMER_LOCATION`
- `USES_RESTAURANT_LOCATION`
- `CALCULATES_DISTANCE`
- `CALCULATES_ETA`

Important properties:

- `provider`
- `update_interval_hint`
- `accuracy_hint`
- `permission_required`
- `model_name`
- `endpoint_hint`

## Security and integrity labels

Use these labels for defensive documentation of security-relevant checks and APIs. This vocabulary records evidence and relationships only; it must not encode bypass steps.

| Label | Description |
| --- | --- |
| `CryptoUse` | Generic crypto use. |
| `CipherUse` | Cipher API use. |
| `HashUse` | Hash API use. |
| `SignatureUse` | Signature API use. |
| `KeyStoreUse` | Android/Java keystore use. |
| `CertificatePinning` | Certificate pinning evidence. |
| `NetworkSecurityConfig` | Network security config. |
| `RootCheck` | Root-detection evidence. |
| `EmulatorCheck` | Emulator-detection evidence. |
| `DebuggerCheck` | Debugger-detection evidence. |
| `HookingCheck` | Hooking/instrumentation-detection evidence. |
| `TamperCheck` | Tamper-detection evidence. |
| `IntegrityCheck` | Generic integrity check. |
| `PlayIntegrityUse` | Play Integrity API use. |
| `SafetyNetUse` | SafetyNet API use. |
| `AntiFridaSignal` | Frida-detection signal. |
| `AntiXposedSignal` | Xposed-detection signal. |

Edges:

- `USES_CRYPTO`
- `USES_CIPHER`
- `USES_HASH`
- `USES_SIGNATURE`
- `USES_KEYSTORE`
- `PINS_CERTIFICATE`
- `USES_NETWORK_SECURITY_CONFIG`
- `CHECKS_ROOT`
- `CHECKS_EMULATOR`
- `CHECKS_DEBUGGER`
- `CHECKS_HOOKING`
- `CHECKS_TAMPER`
- `USES_PLAY_INTEGRITY`
- `USES_SAFETYNET`
- `INDICATES_SECURITY_CHECK`

Important properties:

- `api_name`
- `algorithm`
- `check_type`
- `matched_indicator`

## Dynamic behavior labels

Use these labels for reflection, native, dynamic loading, WebView, and similar runtime boundaries.

| Label | Description |
| --- | --- |
| `ReflectionUse` | Generic reflection use. |
| `ClassForNameUse` | `Class.forName`-style usage. |
| `MethodInvokeUse` | Reflective method invocation. |
| `DynamicDexLoad` | Dynamic dex loading. |
| `DexClassLoaderUse` | DexClassLoader usage. |
| `PathClassLoaderUse` | PathClassLoader usage. |
| `NativeLibraryLoad` | Native library load. |
| `JniMethod` | JNI method. |
| `WebViewUse` | WebView usage. |
| `JavaScriptBridge` | WebView JavaScript bridge. |
| `JavaScriptInterface` | `addJavascriptInterface` target. |
| `RuntimeCommand` | Runtime command execution evidence. |

Edges:

- `USES_REFLECTION`
- `USES_CLASS_FOR_NAME`
- `INVOKES_METHOD_REFLECTIVELY`
- `LOADS_DEX`
- `USES_DEX_CLASS_LOADER`
- `USES_PATH_CLASS_LOADER`
- `LOADS_NATIVE_LIBRARY`
- `CALLS_JNI`
- `USES_WEBVIEW`
- `EXPOSES_JS_BRIDGE`
- `EXPOSES_JS_INTERFACE`
- `RUNS_RUNTIME_COMMAND`

Important properties:

- `api_name`
- `target_string`
- `library_name`
- `class_name_hint`
- `method_name_hint`

## Obfuscation labels

Use these labels for graph-level navigation of obfuscated apps.

| Label | Description |
| --- | --- |
| `ObfuscatedClass` | Class likely affected by obfuscation. |
| `ObfuscatedMethod` | Method likely affected by obfuscation. |
| `ObfuscatedField` | Field likely affected by obfuscation. |
| `NameCluster` | Cluster of similarly named obfuscated symbols. |
| `StringCluster` | Cluster of related string evidence. |
| `ControlFlowHotspot` | Method or class with dense control flow. |
| `HighFanInMethod` | Method with high inbound call degree. |
| `HighFanOutMethod` | Method with high outbound call degree. |
| `BridgeMethod` | Bridge method. |
| `SyntheticMethod` | Synthetic method. |
| `AnonymousClass` | Anonymous class. |
| `GeneratedClass` | Generated class. |

Edges:

- `OBFUSCATED_AS`
- `BELONGS_TO_NAME_CLUSTER`
- `BELONGS_TO_STRING_CLUSTER`
- `HAS_HIGH_FAN_IN`
- `HAS_HIGH_FAN_OUT`
- `BRIDGES_TO`
- `SYNTHETIC_FOR`
- `GENERATED_FROM`

Important properties:

- `name_length`
- `symbol_pattern`
- `fan_in`
- `fan_out`
- `string_count`
- `api_call_count`
- `is_synthetic`
- `is_bridge`

## Business/domain labels

Use these labels as evidence-backed documentation candidates. They are not server-side truth.

| Label | Description |
| --- | --- |
| `BusinessFlow` | Business flow candidate inferred from client-side evidence. |
| `BusinessEntity` | Business entity candidate. |
| `BusinessRule` | Business rule candidate. |
| `FeatureFlag` | Feature flag. |
| `ExperimentFlag` | Experiment or A/B flag. |
| `Screen` | Screen/UI route candidate. |
| `ViewModelFlow` | ViewModel-backed flow candidate. |
| `UseCaseFlow` | Use-case/interactor flow candidate. |
| `RepositoryFlow` | Repository/data flow candidate. |

Recommended `BusinessFlow.category` values:

- `auth`
- `signup`
- `login`
- `logout`
- `otp`
- `profile`
- `address`
- `location`
- `restaurant_listing`
- `restaurant_detail`
- `menu`
- `cart`
- `checkout`
- `payment`
- `wallet`
- `promo`
- `coupon`
- `loyalty`
- `order_create`
- `order_tracking`
- `order_cancel`
- `refund`
- `rating`
- `review`
- `chat`
- `support`
- `rider_assignment`
- `rider_tracking`
- `delivery_eta`
- `fraud_check`
- `device_integrity`
- `notification`
- `analytics`
- `unknown`

Edges:

- `INDICATES_BUSINESS_FLOW`
- `USES_BUSINESS_ENTITY`
- `ENFORCES_BUSINESS_RULE`
- `USES_FEATURE_FLAG`
- `USES_EXPERIMENT_FLAG`
- `BACKS_SCREEN`
- `USES_VIEWMODEL_FLOW`
- `USES_USECASE_FLOW`
- `USES_REPOSITORY_FLOW`

Important properties:

- `matched_terms`
- `endpoint_count`
- `storage_key_count`
- `resource_string_count`
- `warning`

The recommended `warning` value for inferred business nodes is `client_side_inference_only`.

## Evidence labels

Use these labels to explain graph conclusions.

| Label | Description |
| --- | --- |
| `Evidence` | Generic evidence item. |
| `EvidenceSource` | Source artifact containing evidence. |
| `MatchedPattern` | Pattern/rule that matched evidence. |
| `SourceLocation` | File and line/range evidence. |
| `ExtractionToolOutput` | Reverse-engineering tool output metadata. |

Edges:

- `EVIDENCE_FROM`
- `MATCHED_BY_PATTERN`
- `FOUND_IN_FILE`
- `FOUND_AT_LOCATION`
- `DERIVED_FROM_TOOL_OUTPUT`

Important properties:

- `pattern_name`
- `matched_text_preview`
- `extractor_name`
- `source_tool`
- `created_at`

## Query classes this vocabulary should support

This vocabulary is designed so existing graph search and query features can support:

- Search by Android label.
- Search by Android edge type.
- Search by package or component name.
- Search by permission.
- Search by exported component.
- Search by endpoint host, path, method, or header.
- Search by local storage key, table, or file path.
- Search by source format: manifest, resource XML, JADX, Kotlin, smali, DEX metadata, asset, native library.
- Search by confidence.
- Search by evidence source.
- Search JADX to smali mappings.
- Search business-flow candidates.
- Search obfuscation hotspots.

Example labels useful with `search_graph`:

- `Activity`
- `Service`
- `BroadcastReceiver`
- `ContentProvider`
- `DeepLink`
- `ApiEndpoint`
- `StorageKey`
- `BusinessFlow`
- `ObfuscatedMethod`
- `RootCheck`
- `CertificatePinning`
- `WebViewUse`
- `JavaScriptInterface`

Example edge types useful with relationship filters or graph queries:

- `EXPOSES_DEEPLINK`
- `REQUESTS_PERMISSION`
- `CALLS_ENDPOINT`
- `READS_PREFERENCE`
- `WRITES_PREFERENCE`
- `USES_LOCATION`
- `CHECKS_ROOT`
- `PINS_CERTIFICATE`
- `LOADS_NATIVE_LIBRARY`
- `MAPS_TO_SMALI`
- `INDICATES_BUSINESS_FLOW`

## Non-goals

This vocabulary must not introduce:

- APK/XAPK unpacking.
- Decompiler execution.
- Fixed RE tool folder assumptions.
- Dynamic instrumentation.
- Bypass logic.
- Malware verdicts.
- Credential dumping.
- A new database model.
- A heavy UI redesign.

The package should only make extracted Android evidence graphable, searchable, and documentable.
