#include "ndk_camera.hpp"

#include <camera/NdkCameraMetadata.h>
#include <camera/NdkCameraMetadataTags.h>
#include <spdlog/spdlog.h>

void context_on_device_disconnected([[maybe_unused]] ndk_camera_manager_t& context,  //
                                    ACameraDevice* device) noexcept {
    const char* id = ACameraDevice_getId(device);
    spdlog::warn("on_device_disconnect: {}", id);
}

/// @see https://developer.android.com/ndk/reference/group/camera#acameradevice_errorstatecallback
void context_on_device_error([[maybe_unused]] ndk_camera_manager_t& context,  //
                             ACameraDevice* device, int code) noexcept {
    const char* id = ACameraDevice_getId(device);
    const char* message = [error = code]() {
        switch (error) {
            case ERROR_CAMERA_DEVICE:
                return "Fatal error. The camera device needs to be re-opened to be used again.";
            case ERROR_CAMERA_DISABLED:
                return "The camera is disabled due to a device policy, and cannot be opened.";
            case ERROR_CAMERA_IN_USE:
                return "The camera device is in use already.";
            case ERROR_CAMERA_SERVICE:
                return "The device may need to be shut down and restarted to restore camera function.";
            case ERROR_MAX_CAMERAS_IN_USE:
                return "The system-wide limit for number of open cameras or camera resources has been reached.";
            default:
                return "Unknown error code";
        }
    }();
    spdlog::error("on_device_error: id {} code {}: {}", id, code, message);
}

// session state callbacks

void context_on_session_active(ndk_camera_manager_t&, ACameraCaptureSession*) noexcept {
    spdlog::debug("on_session_active");
}

void context_on_session_closed(ndk_camera_manager_t&, ACameraCaptureSession*) noexcept {
    spdlog::warn("on_session_closed");
}

void context_on_session_ready(ndk_camera_manager_t&, ACameraCaptureSession*) noexcept {
    spdlog::debug("on_session_ready");
}

// capture callbacks

void context_on_capture_started(ndk_camera_manager_t&, ACameraCaptureSession*,
                                [[maybe_unused]] const ACaptureRequest* request, uint64_t time_point) noexcept {
    spdlog::debug("context_on_capture_started  : {}", time_point);
}

void context_on_capture_progressed(ndk_camera_manager_t&, ACameraCaptureSession*,
                                   [[maybe_unused]] ACaptureRequest* request, const ACameraMetadata* result) noexcept {
    camera_status_t status = ACAMERA_OK;
    ACameraMetadata_const_entry entry{};
    uint64_t time_point = 0;
    // ACAMERA_SENSOR_TIMESTAMP
    // ACAMERA_SENSOR_INFO_TIMESTAMP_SOURCE
    // ACAMERA_SENSOR_FRAME_DURATION
    status = ACameraMetadata_getConstEntry(result, ACAMERA_SENSOR_TIMESTAMP, &entry);
    if (status == ACAMERA_OK) time_point = static_cast<uint64_t>(*(entry.data.i64));

    spdlog::debug("context_on_capture_progressed: {}", time_point);
}

void context_on_capture_completed(ndk_camera_manager_t&, ACameraCaptureSession*,
                                  [[maybe_unused]] ACaptureRequest* request, const ACameraMetadata* result) noexcept {
    camera_status_t status = ACAMERA_OK;
    ACameraMetadata_const_entry entry{};
    uint64_t time_point = 0;
    // ACAMERA_SENSOR_TIMESTAMP
    // ACAMERA_SENSOR_INFO_TIMESTAMP_SOURCE
    // ACAMERA_SENSOR_FRAME_DURATION
    status = ACameraMetadata_getConstEntry(result, ACAMERA_SENSOR_TIMESTAMP, &entry);
    if (status == ACAMERA_OK) time_point = static_cast<uint64_t>(*(entry.data.i64));

    spdlog::debug("context_on_capture_completed: {}", time_point);
}

void context_on_capture_failed(ndk_camera_manager_t&, ACameraCaptureSession*, [[maybe_unused]] ACaptureRequest* request,
                               ACameraCaptureFailure* failure) noexcept {
    spdlog::error("context_on_capture_failed {} {} {} {}", failure->frameNumber, failure->reason, failure->sequenceId,
                  failure->wasImageCaptured);
}

void context_on_capture_buffer_lost(ndk_camera_manager_t&, ACameraCaptureSession*,
                                    [[maybe_unused]] ACaptureRequest* request, ANativeWindow*,
                                    [[maybe_unused]] int64_t frame) noexcept {
    spdlog::error("context_on_capture_buffer_lost");
}

void context_on_capture_sequence_abort(ndk_camera_manager_t&, ACameraCaptureSession*,
                                       [[maybe_unused]] int sequence) noexcept {
    spdlog::error("context_on_capture_sequence_abort");
}

void context_on_capture_sequence_complete(ndk_camera_manager_t&, ACameraCaptureSession*, [[maybe_unused]] int sequence,
                                          [[maybe_unused]] int64_t frame) noexcept {
    spdlog::debug("context_on_capture_sequence_complete");
}

const char* ndk_camera_error_category_t::name() const noexcept { return "ACAMERA_ERROR"; }

std::string ndk_camera_error_category_t::message(int status) const {
    return get_message(static_cast<camera_status_t>(status));
}

const char* ndk_camera_error_category_t::get_message(camera_status_t status) noexcept {
#define CASE_RETURN(label) \
    case label:            \
        return #label;
    switch (status) {
        CASE_RETURN(ACAMERA_OK);
        CASE_RETURN(ACAMERA_ERROR_INVALID_PARAMETER);
        CASE_RETURN(ACAMERA_ERROR_CAMERA_DISCONNECTED);
        CASE_RETURN(ACAMERA_ERROR_NOT_ENOUGH_MEMORY);
        CASE_RETURN(ACAMERA_ERROR_METADATA_NOT_FOUND);
        CASE_RETURN(ACAMERA_ERROR_CAMERA_DEVICE);
        CASE_RETURN(ACAMERA_ERROR_CAMERA_SERVICE);
        CASE_RETURN(ACAMERA_ERROR_SESSION_CLOSED);
        CASE_RETURN(ACAMERA_ERROR_INVALID_OPERATION);
        CASE_RETURN(ACAMERA_ERROR_STREAM_CONFIGURE_FAIL);
        CASE_RETURN(ACAMERA_ERROR_CAMERA_IN_USE);
        CASE_RETURN(ACAMERA_ERROR_MAX_CAMERA_IN_USE);
        CASE_RETURN(ACAMERA_ERROR_CAMERA_DISABLED);
        CASE_RETURN(ACAMERA_ERROR_PERMISSION_DENIED);
        CASE_RETURN(ACAMERA_ERROR_UNSUPPORTED_OPERATION);
        default:
            return "ACAMERA_ERROR_UNKNOWN";
    }
#undef CASE_RETURN
}

ndk_camera_error_category_t& get_ndk_camera_errors() noexcept {
    static ndk_camera_error_category_t singleton{};
    return singleton;
}

ndk_camera_manager_t::ndk_camera_manager_t() noexcept(false) : manager{ACameraManager_create()} {
    if (auto status = ACameraManager_getCameraIdList(manager, &id_list); status != ACAMERA_OK)
        throw std::system_error{status, get_ndk_camera_errors(), "ACameraManager_getCameraIdList"};
    auto count = id_list->numCameras;
    for (int i = 0; i < count; ++i) {
        const char* camera = id_list->cameraIds[i];
        camera_status_t status = ACameraManager_getCameraCharacteristics(manager, camera, &metadatas[i]);
        if (status != ACAMERA_OK) spdlog::warn("{}: {}", "ACameraManager_getCameraCharacteristics", status);
    }
    callbacks0.context = this;
    callbacks0.onCameraAvailable =
        reinterpret_cast<ACameraManager_AvailabilityCallback>(&ndk_camera_manager_t::camera_available);
    callbacks0.onCameraUnavailable =
        reinterpret_cast<ACameraManager_AvailabilityCallback>(&ndk_camera_manager_t::camera_unavailable);
    if (auto status = ACameraManager_registerAvailabilityCallback(manager, &callbacks0); status != ACAMERA_OK)
        spdlog::error("{}: {}", "ACameraManager_registerAvailabilityCallback", get_ndk_camera_errors().message(status));
}

ndk_camera_manager_t::~ndk_camera_manager_t() noexcept {
    ACameraManager_unregisterAvailabilityCallback(manager, &callbacks0);
    for (auto meta : metadatas)
        if (meta) ACameraMetadata_free(meta);
    if (id_list) ACameraManager_deleteCameraIdList(id_list);
    if (manager) ACameraManager_delete(manager);
}

void ndk_camera_manager_t::camera_available(ndk_camera_manager_t& self, const char* id) {
    return self.on_camera_available(id);
}
void ndk_camera_manager_t::camera_unavailable(ndk_camera_manager_t& self, const char* id) {
    return self.on_camera_unavailable(id);
}

void ndk_camera_manager_t::on_camera_available(const char* id) {
    auto idx = get_index(id);
    spdlog::info("{}: id {} index {}", __func__, id, idx);
}

void ndk_camera_manager_t::on_camera_unavailable(const char* id) {
    auto idx = get_index(id);
    spdlog::info("{}: id {} index {}", __func__, id, idx);
}

uint32_t ndk_camera_manager_t::count() const noexcept { return static_cast<uint32_t>(id_list->numCameras); }

uint32_t ndk_camera_manager_t::get_index(const char* id) const noexcept {
    std::string_view expected{id, strnlen(id, 250)};
    for (int i = 0; i < id_list->numCameras; ++i) {
        std::string_view name{id_list->cameraIds[i], strnlen(id_list->cameraIds[i], 250)};
        if (name == expected) return static_cast<uint32_t>(i);
    }
    return UINT32_MAX;
}

uint32_t ndk_camera_manager_t::get_index(ACameraDevice* device) const noexcept {
    return get_index(ACameraDevice_getId(device));
}

ACameraMetadata* ndk_camera_manager_t::get_metadata(ACameraDevice* device) const noexcept {
    auto idx = get_index(device);
    if (idx == UINT32_MAX) return nullptr;
    return metadatas[idx];
}

camera_status_t ndk_camera_manager_t::open_device(uint32_t idx, ndk_camera_session_t& info,
                                                  ACameraDevice_StateCallbacks& callbacks) noexcept {
    return ACameraManager_openCamera(manager, id_list->cameraIds[idx], &callbacks, &info.device);
}

void ndk_camera_manager_t::close_device(ndk_camera_session_t& info) noexcept {
    if (info.device == nullptr) return;
    if (info.session) close_session(info);
    /// @note Seems like ffmpeg also has same issue, but can't sure about it.
    if (info.device) {
        spdlog::warn("ACameraDevice {:p}: closing...", static_cast<void*>(info.device));
        //
        // W/ACameraCaptureSession: Device is closed but session 0 is not
        // notified
        //
        ACameraDevice_close(info.device);
        info.device = nullptr;
    }
}

/// @brief Designate target surface in request
struct capture_request_t {
    ACaptureRequest* handle = nullptr;

   public:
    /// @param type TEMPLATE_RECORD, TEMPLATE_PREVIEW, TEMPLATE_MANUAL
    capture_request_t(ACameraDevice* device, ACameraDevice_request_template type) {
        if (auto status = ACameraDevice_createCaptureRequest(device, type, &handle); status != ACAMERA_OK)
            throw std::system_error{status, get_ndk_camera_errors(), "ACameraDevice_createCaptureRequest"};
    }
    ~capture_request_t() { ACaptureRequest_free(handle); }
};

struct camera_output_target_t {
    ACameraOutputTarget* handle = nullptr;
    ACaptureRequest* request = nullptr;

   public:
    explicit camera_output_target_t(ANativeWindow* window) { ACameraOutputTarget_create(window, &handle); }
    ~camera_output_target_t() {
        if (request) ACaptureRequest_removeTarget(request, handle);
        ACameraOutputTarget_free(handle);
    }

    void bind(ACaptureRequest* r) {
        request = r;
        auto status = ACaptureRequest_addTarget(request, handle);
        if (status != ACAMERA_OK) throw std::system_error{status, get_ndk_camera_errors(), "ACaptureRequest_addTarget"};
    }
};

/// @brief container for multiplexing of session output
struct session_output_container_t {
    ACaptureSessionOutputContainer* handle = nullptr;

   public:
    session_output_container_t() {
        auto status = ACaptureSessionOutputContainer_create(&handle);
        if (status != ACAMERA_OK)
            throw std::system_error{status, get_ndk_camera_errors(), "ACaptureSessionOutputContainer_create"};
    }
    ~session_output_container_t() { ACaptureSessionOutputContainer_free(handle); }
};

struct session_output_t {
    ACaptureSessionOutput* handle = nullptr;
    ACaptureSessionOutputContainer* container = nullptr;

   public:
    explicit session_output_t(ANativeWindow* window) {
        auto status = ACaptureSessionOutput_create(window, &handle);
        if (status != ACAMERA_OK)
            throw std::system_error{status, get_ndk_camera_errors(), "ACaptureSessionOutput_create"};
    }
    ~session_output_t() {
        if (container) ACaptureSessionOutputContainer_remove(container, handle);
        ACaptureSessionOutput_free(handle);
    }

    void bind(ACaptureSessionOutputContainer* c) noexcept(false) {
        container = c;
        auto status = ACaptureSessionOutputContainer_add(container, handle);
        if (status != ACAMERA_OK)
            throw std::system_error{status, get_ndk_camera_errors(), "ACaptureSessionOutputContainer_add"};
    }
};

camera_status_t ndk_camera_manager_t::start_capture(ndk_camera_session_t& info,
                                                    ACameraCaptureSession_stateCallbacks& on_state_change,
                                                    ACameraCaptureSession_captureCallbacks& on_capture_event,
                                                    ANativeWindow* window) noexcept(false) {
    session_output_container_t outputs{};
    session_output_t output{window};
    output.bind(outputs.handle);
    if (auto status = ACameraDevice_createCaptureSession(info.device, outputs.handle, &on_state_change, &info.session);
        status != ACAMERA_OK) {
        spdlog::error("{}: {}", "ACameraDevice_createCaptureSession", status);
        return status;
    }
    info.repeating = false;

    capture_request_t request{info.device, TEMPLATE_STILL_CAPTURE};
    camera_output_target_t target{window};
    target.bind(request.handle);
    if (auto status =
            ACameraCaptureSession_capture(info.session, &on_capture_event, 1, &request.handle, &info.sequence_id);
        status != ACAMERA_OK) {
        spdlog::error("{}: {}", "ACameraCaptureSession_capture", status);
        return status;
    }
    return ACAMERA_OK;
}
camera_status_t ndk_camera_manager_t::start_capture(ndk_camera_session_t& info, ndk_capture_configuration_t& config,
                                                    ACameraCaptureSession_stateCallbacks& on_state_change,
                                                    ACameraCaptureSession_captureCallbacks& on_capture_event,
                                                    ANativeWindow* window0) noexcept(false) {
    session_output_container_t outputs{};
    session_output_t output0{window0};
    output0.bind(outputs.handle);
    if (auto status = ACameraDevice_createCaptureSession(info.device, outputs.handle, &on_state_change, &info.session);
        status != ACAMERA_OK) {
        spdlog::error("{}: {}", "ACameraDevice_createCaptureSession", status);
        return status;
    }
    info.repeating = false;

    capture_request_t request{info.device, TEMPLATE_STILL_CAPTURE};
    if (config.handler) config.handler(config.context, request.handle);
    camera_output_target_t target0{window0};
    target0.bind(request.handle);

    if (auto status =
            ACameraCaptureSession_capture(info.session, &on_capture_event, 1, &request.handle, &info.sequence_id);
        status != ACAMERA_OK) {
        spdlog::error("{}: {}", "ACameraCaptureSession_capture", status);
        return status;
    }
    return ACAMERA_OK;
}

camera_status_t ndk_camera_manager_t::start_repeat(ndk_camera_session_t& info,
                                                   ACameraCaptureSession_stateCallbacks& on_state_change,
                                                   ACameraCaptureSession_captureCallbacks& on_capture_event,
                                                   ANativeWindow* window) noexcept(false) {
    session_output_container_t outputs{};
    session_output_t output{window};
    output.bind(outputs.handle);
    if (auto status = ACameraDevice_createCaptureSession(info.device, outputs.handle, &on_state_change, &info.session);
        status != ACAMERA_OK) {
        spdlog::error("{}: {}", "ACameraDevice_createCaptureSession", status);
        return status;
    }
    info.repeating = true;

    capture_request_t request{info.device, TEMPLATE_PREVIEW};
    camera_output_target_t target{window};
    target.bind(request.handle);

    if (auto status = ACameraCaptureSession_setRepeatingRequest(info.session, &on_capture_event, 1, &request.handle,
                                                                &info.sequence_id);
        status != ACAMERA_OK) {
        spdlog::error("{}: {}", "ACameraCaptureSession_setRepeatingRequest", status);
        return status;
    }
    return ACAMERA_OK;
}

camera_status_t ndk_camera_manager_t::start_repeat(ndk_camera_session_t& info, ndk_capture_configuration_t& config,
                                                   ACameraCaptureSession_stateCallbacks& on_state_change,
                                                   ACameraCaptureSession_captureCallbacks& on_capture_event,
                                                   ANativeWindow* window0) noexcept(false) {
    session_output_container_t outputs{};
    session_output_t output0{window0};
    output0.bind(outputs.handle);
    if (auto status = ACameraDevice_createCaptureSession(info.device, outputs.handle, &on_state_change, &info.session);
        status != ACAMERA_OK) {
        spdlog::error("{}: {}", "ACameraDevice_createCaptureSession", status);
        return status;
    }
    info.repeating = true;

    capture_request_t request{info.device, TEMPLATE_PREVIEW};
    if (config.handler) config.handler(config.context, request.handle);
    camera_output_target_t target0{window0};
    target0.bind(request.handle);

    if (auto status = ACameraCaptureSession_setRepeatingRequest(info.session, &on_capture_event, 1, &request.handle,
                                                                &info.sequence_id);
        status != ACAMERA_OK) {
        spdlog::error("{}: {}", "ACameraCaptureSession_setRepeatingRequest", status);
        return status;
    }
    return ACAMERA_OK;
}

void ndk_camera_manager_t::close_session(ndk_camera_session_t& info) noexcept(false) {
    if (info.session == nullptr) return;
    spdlog::warn("ACameraCaptureSession {:p}: aborting...", static_cast<void*>(info.session));
    if (info.repeating) ACameraCaptureSession_stopRepeating(info.session);
    ACameraCaptureSession_abortCaptures(info.session);
    ACameraCaptureSession_close(info.session);
    info.session = nullptr;
}
