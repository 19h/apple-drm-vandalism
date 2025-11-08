#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// --- C Representations of C++ Standard Library Types ---
// These structs are designed to match the memory layout of their C++
// counterparts in the Android NDK's libc++. This is platform-specific
// and fragile but necessary for C/C++ interoperability with pre-compiled
// libraries.

/**
 * @brief C-compatible layout for std::shared_ptr.
 */
typedef struct {
    void *obj;      // Pointer to the managed object.
    void *ctrl_blk; // Pointer to the control block.
} am_shared_ptr_t;

/**
 * @brief C-compatible layout for std::string.
 * This union handles the Small String Optimization (SSO).
 */
typedef union {
    // Layout for "long" strings (heap-allocated).
    struct {
        size_t cap;
        size_t size;
        const char *data;
    } l;
    // Layout for "short" strings (stored internally).
    // The actual size is platform-dependent; 23 is a common value for 64-bit.
    struct {
        uint8_t mark;
        char s[23];
    } s;
} am_string_t;

/**
 * @brief C-compatible layout for std::vector.
 */
typedef struct {
    void *begin;
    void *end;
    void *end_capacity;
} am_vector_t;


// --- Inline Helpers for C++ Type Manipulation ---

/**
 * @brief Creates a C representation of a "long" std::string.
 * This is for strings that are too long for SSO.
 */
static inline am_string_t am_string_new(const char *str) {
    am_string_t s;
    s.l.cap = 1; // The lowest bit indicates it's a long string.
    s.l.size = strlen(str);
    s.l.data = str;
    return s;
}

/**
 * @brief Creates a C representation of a "short" std::string (using SSO).
 */
static inline am_string_t am_string_new_short(const char *str) {
    am_string_t s = {0};
    size_t len = strlen(str);
    // The lowest bit being 0 indicates a short string. The length is stored
    // in the higher bits.
    s.s.mark = (uint8_t)(len << 1);
    strncpy(s.s.s, str, sizeof(s.s.s) - 1);
    return s;
}

/**
 * @brief Creates a C representation of a std::vector with one element.
 */
static inline am_vector_t am_vector_new(void *element) {
    am_vector_t vec;
    vec.begin = element;
    vec.end = (char*)element + sizeof(void*); // Assuming vector of pointers
    vec.end_capacity = vec.end;
    return vec;
}

/**
 * @brief Safely extracts the character data from the C representation of a std::string.
 */
static inline const char *am_string_data(am_string_t *str) {
    // The lowest bit of the first byte/capacity field determines the mode.
    return (str->s.mark & 1) ? str->l.data : str->s.s;
}


// --- Function Prototypes from Apple Music Libraries ---
// These are the mangled names of C++ functions exported from the shared libraries.
// Below each block of externs is a corresponding block of human-readable macros.

// --- Context & Initialization ---
extern void _ZN17storeservicescore10DeviceGUID8instanceEv(am_shared_ptr_t *ret);
extern void _ZN17storeservicescore10DeviceGUID9configureERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_RKjRKb(void *ret, void *instance, am_string_t *s1, am_string_t *s2, unsigned int *i, uint8_t *b);
extern void _ZN14FootHillConfig6configERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE(am_string_t *s);
extern void _ZNSt6__ndk110shared_ptrIN17storeservicescore14RequestContextEE11make_sharedIJRNS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEEEEES3_DpOT_(am_shared_ptr_t *ret, am_string_t *s);
extern void _ZN17storeservicescore20RequestContextConfigC2Ev(void *this_ptr);
extern void _ZN17storeservicescore20RequestContextConfig20setBaseDirectoryPathERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *this_ptr, am_string_t *s);
extern void _ZN17storeservicescore20RequestContextConfig19setClientIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *this_ptr, am_string_t *s);
extern void _ZN17storeservicescore20RequestContextConfig20setVersionIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *this_ptr, am_string_t *s);
extern void _ZN17storeservicescore20RequestContextConfig21setPlatformIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *this_ptr, am_string_t *s);
extern void _ZN17storeservicescore20RequestContextConfig17setProductVersionERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *this_ptr, am_string_t *s);
extern void _ZN17storeservicescore20RequestContextConfig14setDeviceModelERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *this_ptr, am_string_t *s);
extern void _ZN17storeservicescore20RequestContextConfig15setBuildVersionERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *this_ptr, am_string_t *s);
extern void _ZN17storeservicescore20RequestContextConfig19setLocaleIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *this_ptr, am_string_t *s);
extern void _ZN17storeservicescore20RequestContextConfig21setLanguageIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *this_ptr, am_string_t *s);
extern void _ZN17storeservicescore20RequestContextConfig24setFairPlayDirectoryPathERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *this_ptr, am_string_t *s);
extern void _ZN17storeservicescore14RequestContext4initERKNSt6__ndk110shared_ptrINS_20RequestContextConfigEEE(void *ret, void *this_ptr, am_shared_ptr_t *config);
extern void _ZN17storeservicescore14RequestContext24setFairPlayDirectoryPathERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *this_ptr, am_string_t *s);
extern void _ZN21RequestContextManager9configureERKNSt6__ndk110shared_ptrIN17storeservicescore14RequestContextEEE(am_shared_ptr_t *ctx);
extern void _resolv_set_nameservers_for_net(unsigned netid, const char **servers, int numservers, const char *domains);

#define ssc_device_guid_instance _ZN17storeservicescore10DeviceGUID8instanceEv
#define ssc_device_guid_configure _ZN17storeservicescore10DeviceGUID9configureERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEES9_RKjRKb
#define fh_config _ZN14FootHillConfig6configERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEE
#define std_make_shared_request_context _ZNSt6__ndk110shared_ptrIN17storeservicescore14RequestContextEE11make_sharedIJRNS_12basic_stringIcNS_11char_traitsIcEENS_9allocatorIcEEEEEEES3_DpOT_
#define ssc_request_context_config_ctor _ZN17storeservicescore20RequestContextConfigC2Ev
#define ssc_request_context_config_set_base_dir _ZN17storeservicescore20RequestContextConfig20setBaseDirectoryPathERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE
#define ssc_request_context_config_set_client_id _ZN17storeservicescore20RequestContextConfig19setClientIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE
#define ssc_request_context_config_set_version_id _ZN17storeservicescore20RequestContextConfig20setVersionIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE
#define ssc_request_context_config_set_platform_id _ZN17storeservicescore20RequestContextConfig21setPlatformIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE
#define ssc_request_context_config_set_product_version _ZN17storeservicescore20RequestContextConfig17setProductVersionERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE
#define ssc_request_context_config_set_device_model _ZN17storeservicescore20RequestContextConfig14setDeviceModelERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE
#define ssc_request_context_config_set_build_version _ZN17storeservicescore20RequestContextConfig15setBuildVersionERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE
#define ssc_request_context_config_set_locale_id _ZN17storeservicescore20RequestContextConfig19setLocaleIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE
#define ssc_request_context_config_set_lang_id _ZN17storeservicescore20RequestContextConfig21setLanguageIdentifierERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE
#define ssc_request_context_config_set_fairplay_dir _ZN17storeservicescore20RequestContextConfig24setFairPlayDirectoryPathERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE
#define ssc_request_context_init _ZN17storeservicescore14RequestContext4initERKNSt6__ndk110shared_ptrINS_20RequestContextConfigEEE
#define ssc_request_context_set_fairplay_dir _ZN17storeservicescore14RequestContext24setFairPlayDirectoryPathERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE
#define ssc_request_context_manager_configure _ZN21RequestContextManager9configureERKNSt6__ndk110shared_ptrIN17storeservicescore14RequestContextEEE
#define resolv_set_nameservers_for_net _resolv_set_nameservers_for_net

// --- Authentication Flow ---
extern void _ZNSt6__ndk110shared_ptrIN17storeservicescore16AuthenticateFlowEE11make_sharedIJRNS0_INS1_14RequestContextEEEEEES3_DpOT_(am_shared_ptr_t *ret, am_shared_ptr_t *ctx);
extern void _ZN17storeservicescore16AuthenticateFlow3runEv(void *this_ptr);
extern am_shared_ptr_t *_ZNK17storeservicescore16AuthenticateFlow8responseEv(void *this_ptr);
extern int _ZNK17storeservicescore20AuthenticateResponse12responseTypeEv(void *this_ptr);

#define std_make_shared_auth_flow _ZNSt6__ndk110shared_ptrIN17storeservicescore16AuthenticateFlowEE11make_sharedIJRNS0_INS1_14RequestContextEEEEEES3_DpOT_
#define ssc_auth_flow_run _ZN17storeservicescore16AuthenticateFlow3runEv
#define ssc_auth_flow_response _ZNK17storeservicescore16AuthenticateFlow8responseEv
#define ssc_auth_response_type _ZNK17storeservicescore20AuthenticateResponse12responseTypeEv

// --- Dialog & Credential Handling ---
extern void _ZNSt6__ndk110shared_ptrIN20androidstoreservices28AndroidPresentationInterfaceEE11make_sharedIJEEES3_DpOT_(am_shared_ptr_t *ret);
extern void _ZN17storeservicescore14RequestContext24setPresentationInterfaceERKNSt6__ndk110shared_ptrINS_21PresentationInterfaceEEE(void *this_ptr, am_shared_ptr_t *pres_interface);
extern void _ZN20androidstoreservices28AndroidPresentationInterface16setDialogHandlerEPFvlNSt6__ndk110shared_ptrIN17storeservicescore14ProtocolDialogEEENS2_INS_36AndroidProtocolDialogResponseHandlerEEEE(void *this_ptr, void (*handler)(long, am_shared_ptr_t *, am_shared_ptr_t *));
extern void _ZN20androidstoreservices28AndroidPresentationInterface21setCredentialsHandlerEPFvNSt6__ndk110shared_ptrIN17storeservicescore18CredentialsRequestEEENS2_INS_33AndroidCredentialsResponseHandlerEEEE(void *this_ptr, void (*handler)(am_shared_ptr_t *, am_shared_ptr_t *));
extern am_string_t *_ZNK17storeservicescore14ProtocolDialog5titleEv(void *this_ptr);
extern am_string_t *_ZNK17storeservicescore14ProtocolDialog7messageEv(void *this_ptr);
extern am_vector_t *_ZNK17storeservicescore14ProtocolDialog7buttonsEv(void *this_ptr);
extern am_string_t *_ZNK17storeservicescore14ProtocolButton5titleEv(void *this_ptr);
extern void _ZN17storeservicescore22ProtocolDialogResponseC1Ev(void *this_ptr);
extern void _ZN17storeservicescore22ProtocolDialogResponse17setSelectedButtonERKNSt6__ndk110shared_ptrINS_14ProtocolButtonEEE(void *this_ptr, am_shared_ptr_t *button);
extern void _ZN20androidstoreservices28AndroidPresentationInterface28handleProtocolDialogResponseERKlRKNSt6__ndk110shared_ptrIN17storeservicescore22ProtocolDialogResponseEEE(void *this_ptr, long *j, am_shared_ptr_t *resp);
extern am_string_t *_ZNK17storeservicescore18CredentialsRequest5titleEv(void *this_ptr);
extern am_string_t *_ZNK17storeservicescore18CredentialsRequest7messageEv(void *this_ptr);
extern uint8_t _ZNK17storeservicescore18CredentialsRequest28requiresHSA2VerificationCodeEv(void *this_ptr);
extern void _ZN17storeservicescore19CredentialsResponseC1Ev(void *this_ptr);
extern void _ZN17storeservicescore19CredentialsResponse11setUserNameERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *this_ptr, am_string_t *s);
extern void _ZN17storeservicescore19CredentialsResponse11setPasswordERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE(void *this_ptr, am_string_t *s);
extern void _ZN17storeservicescore19CredentialsResponse15setResponseTypeENS0_12ResponseTypeE(void *this_ptr, int type);
extern void _ZN20androidstoreservices28AndroidPresentationInterface25handleCredentialsResponseERKNSt6__ndk110shared_ptrIN17storeservicescore19CredentialsResponseEEE(void *this_ptr, am_shared_ptr_t *resp);

#define std_make_shared_android_presentation_interface _ZNSt6__ndk110shared_ptrIN20androidstoreservices28AndroidPresentationInterfaceEE11make_sharedIJEEES3_DpOT_
#define ssc_request_context_set_presentation_interface _ZN17storeservicescore14RequestContext24setPresentationInterfaceERKNSt6__ndk110shared_ptrINS_21PresentationInterfaceEEE
#define api_set_dialog_handler _ZN20androidstoreservices28AndroidPresentationInterface16setDialogHandlerEPFvlNSt6__ndk110shared_ptrIN17storeservicescore14ProtocolDialogEEENS2_INS_36AndroidProtocolDialogResponseHandlerEEEE
#define api_set_credentials_handler _ZN20androidstoreservices28AndroidPresentationInterface21setCredentialsHandlerEPFvNSt6__ndk110shared_ptrIN17storeservicescore18CredentialsRequestEEENS2_INS_33AndroidCredentialsResponseHandlerEEEE
#define ssc_protocol_dialog_title _ZNK17storeservicescore14ProtocolDialog5titleEv
#define ssc_protocol_dialog_message _ZNK17storeservicescore14ProtocolDialog7messageEv
#define ssc_protocol_dialog_buttons _ZNK17storeservicescore14ProtocolDialog7buttonsEv
#define ssc_protocol_button_title _ZNK17storeservicescore14ProtocolButton5titleEv
#define ssc_protocol_dialog_response_ctor _ZN17storeservicescore22ProtocolDialogResponseC1Ev
#define ssc_protocol_dialog_response_set_selected_button _ZN17storeservicescore22ProtocolDialogResponse17setSelectedButtonERKNSt6__ndk110shared_ptrINS_14ProtocolButtonEEE
#define api_handle_protocol_dialog_response _ZN20androidstoreservices28AndroidPresentationInterface28handleProtocolDialogResponseERKlRKNSt6__ndk110shared_ptrIN17storeservicescore22ProtocolDialogResponseEEE
#define ssc_credentials_request_title _ZNK17storeservicescore18CredentialsRequest5titleEv
#define ssc_credentials_request_message _ZNK17storeservicescore18CredentialsRequest7messageEv
#define ssc_credentials_request_requires_hsa2 _ZNK17storeservicescore18CredentialsRequest28requiresHSA2VerificationCodeEv
#define ssc_credentials_response_ctor _ZN17storeservicescore19CredentialsResponseC1Ev
#define ssc_credentials_response_set_username _ZN17storeservicescore19CredentialsResponse11setUserNameERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE
#define ssc_credentials_response_set_password _ZN17storeservicescore19CredentialsResponse11setPasswordERKNSt6__ndk112basic_stringIcNS1_11char_traitsIcEENS1_9allocatorIcEEEE
#define ssc_credentials_response_set_response_type _ZN17storeservicescore19CredentialsResponse15setResponseTypeENS0_12ResponseTypeE
#define api_handle_credentials_response _ZN20androidstoreservices28AndroidPresentationInterface25handleCredentialsResponseERKNSt6__ndk110shared_ptrIN17storeservicescore19CredentialsResponseEEE

// --- DRM / FairPlay ---
extern void *_ZN21SVFootHillSessionCtrl8instanceEv(void);
extern void _ZN21SVFootHillSessionCtrl16getPersistentKeyERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_S8_S8_S8_S8_S8_S8_(am_shared_ptr_t *ret, void *instance, am_string_t *s1, am_string_t *s2, am_string_t *s3, am_string_t *s4, am_string_t *s5, am_string_t *s6, am_string_t *s7, am_string_t *s8);
extern void _ZN21SVFootHillSessionCtrl14decryptContextERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKN11SVDecryptor15SVDecryptorTypeERKb(am_shared_ptr_t *ret, void *instance, am_string_t *ckc);
extern void **_ZNK18SVFootHillPContext9kdContextEv(void *this_ptr);
extern long NfcRKVnxuKZy04KWbdFu71Ou(void *kd_context, uint32_t, void *in_buffer, void *out_buffer, size_t size);

#define fh_session_ctrl_instance _ZN21SVFootHillSessionCtrl8instanceEv
#define fh_session_ctrl_get_persistent_key _ZN21SVFootHillSessionCtrl16getPersistentKeyERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEES8_S8_S8_S8_S8_S8_S8_
#define fh_session_ctrl_decrypt_context _ZN21SVFootHillSessionCtrl14decryptContextERKNSt6__ndk112basic_stringIcNS0_11char_traitsIcEENS0_9allocatorIcEEEERKN11SVDecryptor15SVDecryptorTypeERKb
#define fh_pcontext_get_kd_context _ZNK18SVFootHillPContext9kdContextEv
#define fh_decrypt_sample NfcRKVnxuKZy04KWbdFu71Ou

// --- Playback & Asset Request ---
extern void _ZN22SVPlaybackLeaseManagerC2ERKNSt6__ndk18functionIFvRKiEEERKNS1_IFvRKNS0_10shared_ptrIN17storeservicescore19StoreErrorConditionEEEEEE(void *this_ptr, void *end_lease_cb, void *pb_err_cb);
extern void _ZN22SVPlaybackLeaseManager25refreshLeaseAutomaticallyERKb(void *this_ptr, uint8_t *b);
extern void _ZN22SVPlaybackLeaseManager12requestLeaseERKb(void *this_ptr, uint8_t *b);
extern void _ZN22SVPlaybackLeaseManager12requestAssetERKmRKNSt6__ndk16vectorINS2_12basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEENS7_IS9_EEEERKb(am_shared_ptr_t *ret, void *this_ptr, unsigned long *adam_id, am_vector_t *vec, uint8_t *b);
extern int _ZNK23SVPlaybackAssetResponse13hasValidAssetEv(void *this_ptr);
extern am_shared_ptr_t *_ZNK23SVPlaybackAssetResponse13playbackAssetEv(void *this_ptr);
extern void _ZNK17storeservicescore13PlaybackAsset9URLStringEv(am_string_t *ret, void *this_ptr);

#define plm_ctor _ZN22SVPlaybackLeaseManagerC2ERKNSt6__ndk18functionIFvRKiEEERKNS1_IFvRKNS0_10shared_ptrIN17storeservicescore19StoreErrorConditionEEEEEE
#define plm_refresh_lease_auto _ZN22SVPlaybackLeaseManager25refreshLeaseAutomaticallyERKb
#define plm_request_lease _ZN22SVPlaybackLeaseManager12requestLeaseERKb
#define plm_request_asset _ZN22SVPlaybackLeaseManager12requestAssetERKmRKNSt6__ndk16vectorINS2_12basic_stringIcNS2_11char_traitsIcEENS2_9allocatorIcEEEENS7_IS9_EEEERKb
#define ssc_playback_asset_response_has_valid_asset _ZNK23SVPlaybackAssetResponse13hasValidAssetEv
#define ssc_playback_asset_response_get_asset _ZNK23SVPlaybackAssetResponse13playbackAssetEv
#define ssc_playback_asset_get_url _ZNK17storeservicescore13PlaybackAsset9URLStringEv


// --- VTable Pointers for Manual Object Construction ---
// These are pointers to the vtables for C++ classes, which are needed when
// manually constructing C representations of C++ objects on the stack.
extern void *_ZTVNSt6__ndk120__shared_ptr_emplaceIN17storeservicescore22ProtocolDialogResponseENS_9allocatorIS2_EEEE;
extern void *_ZTVNSt6__ndk120__shared_ptr_emplaceIN17storeservicescore19CredentialsResponseENS_9allocatorIS2_EEEE;
extern void *_ZTVNSt6__ndk120__shared_ptr_emplaceIN17storeservicescore20RequestContextConfigENS_9allocatorIS2_EEEE;

#define VTABLE_SSC_PROTOCOL_DIALOG_RESPONSE _ZTVNSt6__ndk120__shared_ptr_emplaceIN17storeservicescore22ProtocolDialogResponseENS_9allocatorIS2_EEEE
#define VTABLE_SSC_CREDENTIALS_RESPONSE _ZTVNSt6__ndk120__shared_ptr_emplaceIN17storeservicescore19CredentialsResponseENS_9allocatorIS2_EEEE
#define VTABLE_SSC_REQUEST_CONTEXT_CONFIG _ZTVNSt6__ndk120__shared_ptr_emplaceIN17storeservicescore20RequestContextConfigENS_9allocatorIS2_EEEE


// --- Global Constants ---
extern const char *const ANDROID_ID;
extern const char *const FAIRPLAY_CERTIFICATE;

#ifdef __cplusplus
}
#endif
