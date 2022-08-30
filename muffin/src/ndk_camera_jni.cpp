#include <android/api-level.h>
#include <android/hardware_buffer.h>
#include <android/native_window_jni.h>
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>
#include <spdlog/spdlog.h>

#include <mutex>

#include "ndk_camera.hpp"

void get_field(JNIEnv* env, jclass type, jobject target, const char* field_name, jlong& ref) noexcept;
void set_field(JNIEnv* env, jclass type, jobject target, const char* field_name, jlong value) noexcept;
void store_runtime_exception(JNIEnv* env, const char* message) noexcept;

std::unique_ptr<ndk_camera_manager_t> camera_manager = nullptr;

void init_camera_manager([[maybe_unused]] JNIEnv* env, [[maybe_unused]] jclass clazz) {
    camera_manager = std::make_unique<ndk_camera_manager_t>();
}

ndk_camera_manager_t& get_camera_manager() {
    if (camera_manager == nullptr) throw std::runtime_error{"camera_manager is not initialized"};
    return *camera_manager;
}

/**
 * @param idx Index of the existing camera
 * @return uint16_t ACAMERA_LENS_FACING_FRONT, ACAMERA_LENS_FACING_BACK, ACAMERA_LENS_FACING_EXTERNAL
 */
uint16_t get_facing(ACameraMetadata* metadata) noexcept {
    ACameraMetadata_const_entry entry{};
    ACameraMetadata_getConstEntry(metadata, ACAMERA_LENS_FACING, &entry);
    const auto facing = *(entry.data.u8);
    switch (facing) {
        case ACAMERA_LENS_FACING_FRONT:
        case ACAMERA_LENS_FACING_BACK:
        case ACAMERA_LENS_FACING_EXTERNAL:
            return facing;
        default:
            spdlog::warn("{}: {}", "unexpected ACAMERA_LENS_FACING", facing);
            return ACAMERA_LENS_FACING_EXTERNAL;
    }
}

auto get_preferred_size(ACameraMetadata* metadata) noexcept {
    ACameraMetadata_const_entry entry{};
    ACameraMetadata_getConstEntry(metadata, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry);
    int width = 1920;
    int height = 1080;
    for (int i = 0; i < entry.count; ++i) {
        auto flow = entry.data.i32[i + 3];
        if (flow != ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT) continue;
        auto format = entry.data.i32[i + 0];
        switch (format) {
            case AIMAGE_FORMAT_PRIVATE:
            case AIMAGE_FORMAT_YUV_420_888:
                break;
            default:
                continue;
        }
        if (auto w = entry.data.i32[i + 1]; w >= width) {
            width = w;
            height = entry.data.i32[i + 2];
        }
    }
    return std::make_tuple(width, height);
}

ndk_camera_session_t* cast_device_handle(JNIEnv* env, jobject self) {
    jlong ptr = 0;
    get_field(env, env->GetObjectClass(self), self, "ptr", ptr);
    return reinterpret_cast<ndk_camera_session_t*>(ptr);
}

void set_device_handle(JNIEnv* env, jobject self, ndk_camera_session_t* ptr) {
    set_field(env, env->GetObjectClass(self), self, "ptr", reinterpret_cast<jlong>(ptr));
}

using native_window_ptr = std::unique_ptr<ANativeWindow, void (*)(ANativeWindow*)>;

extern "C" {

JNIEXPORT void JNICALL Java_dev_luncliff_muffin_DeviceManager_Init(JNIEnv* env, jclass clazz) {
    static std::once_flag flag{};
    return std::call_once(flag, init_camera_manager, env, clazz);
}

JNIEXPORT
jint Java_dev_luncliff_muffin_DeviceManager_GetDeviceCount(JNIEnv* env, jclass) noexcept {
    try {
        return static_cast<jint>(camera_manager->count());
    } catch (const std::exception& ex) {
        spdlog::error("{:s}: {:s}", __func__, ex.what());
        store_runtime_exception(env, ex.what());
        return 0;
    }
}

JNIEXPORT
void Java_dev_luncliff_muffin_DeviceManager_SetDeviceData(JNIEnv* env, jclass, jobjectArray devices) noexcept {
    // env->FindClass("dev/luncliff/muffin/DeviceHandle");
    int num_devices = static_cast<int>(camera_manager->count());
    // https://developer.android.com/ndk/reference/group/camera
    for (int index = 0; index < num_devices; ++index) {
        jobject target = env->GetObjectArrayElement(devices, index);
        auto ptr = new ndk_camera_session_t{};
        ptr->index = static_cast<uint16_t>(index);
        set_device_handle(env, target, ptr);
    }
}

JNIEXPORT
int Java_dev_luncliff_muffin_DeviceHandle_facing(JNIEnv* env, jobject self) noexcept {
    ndk_camera_session_t* ptr = cast_device_handle(env, self);
    ACameraMetadata* metadata = camera_manager->get_metadata(ptr->device);
    if (metadata == nullptr) return ACAMERA_LENS_FACING_EXTERNAL;
    return get_facing(metadata);
}

JNIEXPORT
int Java_dev_luncliff_muffin_DeviceHandle_maxWidth(JNIEnv* env, jobject self) noexcept {
    ndk_camera_session_t* ptr = cast_device_handle(env, self);
    ACameraMetadata* metadata = camera_manager->get_metadata(ptr->device);
    if (metadata == nullptr) return 0;
    auto [w, _] = get_preferred_size(metadata);
    return w;
}

JNIEXPORT
int Java_dev_luncliff_muffin_DeviceHandle_maxHeight(JNIEnv* env, jobject self) noexcept {
    ndk_camera_session_t* ptr = cast_device_handle(env, self);
    ACameraMetadata* metadata = camera_manager->get_metadata(ptr->device);
    if (metadata == nullptr) return 0;
    auto [_, h] = get_preferred_size(metadata);
    return h;
}

JNIEXPORT
void Java_dev_luncliff_muffin_DeviceHandle_open(JNIEnv* env, jobject self) noexcept {
    try {
        ndk_camera_session_t* ptr = cast_device_handle(env, self);

        ACameraDevice_StateCallbacks callbacks{};
        callbacks.context = camera_manager.get();
        callbacks.onDisconnected = reinterpret_cast<ACameraDevice_StateCallback>(context_on_device_disconnected);
        callbacks.onError = reinterpret_cast<ACameraDevice_ErrorStateCallback>(context_on_device_error);

        if (auto status = camera_manager->open_device(ptr->index, *ptr, callbacks); status != ACAMERA_OK)
            throw std::system_error(status, get_ndk_camera_errors(), "open_device");
    } catch (const std::exception& ex) {
        spdlog::error("{:s}: {:s}", __func__, ex.what());
        store_runtime_exception(env, ex.what());
    }
}

JNIEXPORT
void Java_dev_luncliff_muffin_DeviceHandle_close(JNIEnv* env, jobject self) noexcept {
    ndk_camera_session_t* ptr = cast_device_handle(env, self);
    if (ptr == nullptr) return;
    camera_manager->close_device(*ptr);
}

JNIEXPORT
void Java_dev_luncliff_muffin_DeviceHandle_startRepeat(JNIEnv* env, jobject self, jobject surface) noexcept {
    auto ptr = cast_device_handle(env, self);

    // `ANativeWindow_fromSurface` acquires a reference
    // `ANativeWindow_release` releases it
    auto window = native_window_ptr{ANativeWindow_fromSurface(env, surface), ANativeWindow_release};

    ACameraCaptureSession_stateCallbacks on_state_changed{};
    on_state_changed.context = camera_manager.get();
    on_state_changed.onReady = reinterpret_cast<ACameraCaptureSession_stateCallback>(context_on_session_ready);
    on_state_changed.onClosed = reinterpret_cast<ACameraCaptureSession_stateCallback>(context_on_session_closed);
    on_state_changed.onActive = reinterpret_cast<ACameraCaptureSession_stateCallback>(context_on_session_active);

    ACameraCaptureSession_captureCallbacks on_capture_event{};
    on_capture_event.context = camera_manager.get();
    on_capture_event.onCaptureStarted =
        reinterpret_cast<ACameraCaptureSession_captureCallback_start>(context_on_capture_started);
    on_capture_event.onCaptureBufferLost =
        reinterpret_cast<ACameraCaptureSession_captureCallback_bufferLost>(context_on_capture_buffer_lost);
    on_capture_event.onCaptureProgressed =
        reinterpret_cast<ACameraCaptureSession_captureCallback_result>(context_on_capture_progressed);
    on_capture_event.onCaptureCompleted =
        reinterpret_cast<ACameraCaptureSession_captureCallback_result>(context_on_capture_completed);
    on_capture_event.onCaptureFailed =
        reinterpret_cast<ACameraCaptureSession_captureCallback_failed>(context_on_capture_failed);
    on_capture_event.onCaptureSequenceAborted =
        reinterpret_cast<ACameraCaptureSession_captureCallback_sequenceAbort>(context_on_capture_sequence_abort);
    on_capture_event.onCaptureSequenceCompleted =
        reinterpret_cast<ACameraCaptureSession_captureCallback_sequenceEnd>(context_on_capture_sequence_complete);

    try {
        if (auto status = camera_manager->start_repeat(*ptr, on_state_changed, on_capture_event, window.get());
            status != ACAMERA_OK)
            throw std::system_error(status, get_ndk_camera_errors(), "start_repeat");
    } catch (const std::exception& ex) {
        spdlog::error("{}: {}", __func__, ex.what());
        store_runtime_exception(env, ex.what());
    }
}

JNIEXPORT
void Java_dev_luncliff_muffin_DeviceHandle_stopRepeat(JNIEnv* env, jobject self) noexcept {
    try {
        auto ptr = cast_device_handle(env, self);
        camera_manager->close_session(*ptr);
    } catch (const std::exception& ex) {
        spdlog::error("{}: {}", __func__, ex.what());
        store_runtime_exception(env, ex.what());
    }
}

JNIEXPORT
void Java_dev_luncliff_muffin_DeviceHandle_startCapture(JNIEnv* env, jobject self, jobject surface) noexcept {
    auto ptr = cast_device_handle(env, self);

    // `ANativeWindow_fromSurface` acquires a reference
    // `ANativeWindow_release` releases it
    auto window = native_window_ptr{ANativeWindow_fromSurface(env, surface), ANativeWindow_release};

    ACameraCaptureSession_stateCallbacks on_state_changed{};
    on_state_changed.context = camera_manager.get();
    on_state_changed.onReady = reinterpret_cast<ACameraCaptureSession_stateCallback>(context_on_session_ready);
    on_state_changed.onClosed = reinterpret_cast<ACameraCaptureSession_stateCallback>(context_on_session_closed);
    on_state_changed.onActive = reinterpret_cast<ACameraCaptureSession_stateCallback>(context_on_session_active);

    ACameraCaptureSession_captureCallbacks on_capture_event{};
    on_capture_event.context = camera_manager.get();
    on_capture_event.onCaptureStarted =
        reinterpret_cast<ACameraCaptureSession_captureCallback_start>(context_on_capture_started);
    on_capture_event.onCaptureBufferLost =
        reinterpret_cast<ACameraCaptureSession_captureCallback_bufferLost>(context_on_capture_buffer_lost);
    on_capture_event.onCaptureProgressed =
        reinterpret_cast<ACameraCaptureSession_captureCallback_result>(context_on_capture_progressed);
    on_capture_event.onCaptureCompleted =
        reinterpret_cast<ACameraCaptureSession_captureCallback_result>(context_on_capture_completed);
    on_capture_event.onCaptureFailed =
        reinterpret_cast<ACameraCaptureSession_captureCallback_failed>(context_on_capture_failed);
    on_capture_event.onCaptureSequenceAborted =
        reinterpret_cast<ACameraCaptureSession_captureCallback_sequenceAbort>(context_on_capture_sequence_abort);
    on_capture_event.onCaptureSequenceCompleted =
        reinterpret_cast<ACameraCaptureSession_captureCallback_sequenceEnd>(context_on_capture_sequence_complete);

    try {
        if (auto status = camera_manager->start_capture(*ptr, on_state_changed, on_capture_event, window.get());
            status != ACAMERA_OK)
            throw std::system_error(status, get_ndk_camera_errors(), "start_capture");
    } catch (const std::exception& ex) {
        spdlog::error("{}: {}", __func__, ex.what());
        store_runtime_exception(env, ex.what());
    }
}

JNIEXPORT
void Java_dev_luncliff_muffin_DeviceHandle_stopCapture(JNIEnv* env, jobject self) noexcept {
    try {
        auto ptr = cast_device_handle(env, self);
        camera_manager->close_session(*ptr);
    } catch (const std::exception& ex) {
        spdlog::error("{}: {}", __func__, ex.what());
        store_runtime_exception(env, ex.what());
    }
}

}  // extern "C"
