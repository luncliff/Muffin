#include <android/api-level.h>
#include <android/hardware_buffer.h>
#include <android/native_window_jni.h>
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <string>
#include <string_view>

#include "muffin.hpp"
#include "ndk_camera.hpp"

using namespace std::experimental;

void get_field(JNIEnv* env,  //
               jclass _type, jobject target, const char* field_name, jlong& ref) noexcept {
    jfieldID _field = env->GetFieldID(_type, field_name, "J");  // long
    ref = env->GetLongField(target, _field);
}

void set_field(JNIEnv* env,  //
               jclass _type, jobject target, const char* field_name, jlong value) noexcept {
    jfieldID _field = env->GetFieldID(_type, field_name, "J");  // long
    env->SetLongField(target, _field, value);
}

/**
 * @brief Find exception class information (type info)
 * @see java/lang/RuntimeException
 */
void store_runtime_exception(JNIEnv* env, const char* message) noexcept {
    const char* class_name = "java/lang/RuntimeException";
    jclass _type = env->FindClass(class_name);
    if (_type == nullptr) return spdlog::error("No Java class: {}", class_name);
    env->ThrowNew(_type, message);
}

static_assert(sizeof(void*) <= sizeof(jlong), "`jlong` must be able to contain `void*` pointer");

extern "C" {

JNIEXPORT
jboolean Java_dev_luncliff_muffin_NativeRunnable_resume(JNIEnv*, jobject, void* handle) noexcept {
    auto task = coroutine_handle<>::from_address(handle);
    task.resume();
    if (task.done()) {
        task.destroy();
        return JNI_TRUE;
    }
    return JNI_FALSE;  // next run will continue the work
}

JNIEXPORT
jlong Java_dev_luncliff_muffin_Renderer1_create1(JNIEnv*, jclass, jlong d, jlong c) noexcept {
    auto es_display = reinterpret_cast<EGLDisplay>(d);
    auto es_config = get_default_config(es_display);
    auto es_context = reinterpret_cast<EGLContext>(c);
    auto context = std::make_unique<egl_context_t>(es_display, es_config, es_context);
    if (context->handle() == EGL_NO_CONTEXT) {
        spdlog::error("context is created but not valid");
        return 0;
    }
    return reinterpret_cast<jlong>(context.release());
}

JNIEXPORT
void Java_dev_luncliff_muffin_Renderer1_destroy1(JNIEnv*, jclass, jlong c) noexcept {
    auto context = reinterpret_cast<egl_context_t*>(c);
    delete context;
}

JNIEXPORT
jlong Java_dev_luncliff_muffin_Renderer1_create2(JNIEnv* env, jclass, jlong c, jobject _surface) noexcept {
    try {
        auto context = reinterpret_cast<egl_context_t*>(c);
        auto surface = make_egl_surface(context->get_display(), env, _surface);
        return reinterpret_cast<jlong>(surface.release());
    } catch (const std::exception& ex) {
        store_runtime_exception(env, ex.what());
        return 0;
    }
}

JNIEXPORT
void Java_dev_luncliff_muffin_Renderer1_destroy2(JNIEnv*, jclass, jlong s) noexcept {
    auto surface = reinterpret_cast<egl_surface_t*>(s);
    delete surface;
}

JNIEXPORT
jint Java_dev_luncliff_muffin_Renderer1_resume(JNIEnv*, jclass, jlong c, jlong s) noexcept {
    auto context = reinterpret_cast<egl_context_t*>(c);
    auto surface = reinterpret_cast<egl_surface_t*>(s);
    return context->resume(surface->handle());
}

JNIEXPORT
jint Java_dev_luncliff_muffin_Renderer1_suspend(JNIEnv*, jclass, jlong c) noexcept {
    auto context = reinterpret_cast<egl_context_t*>(c);
    return context->suspend();
}

JNIEXPORT
jint Java_dev_luncliff_muffin_Renderer1_present(JNIEnv*, jclass, jlong c) noexcept {
    auto context = reinterpret_cast<egl_context_t*>(c);
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    return context->swap();
}

JNIEXPORT
jstring Java_dev_luncliff_muffin_Renderer1_toString(JNIEnv* env, jobject self) noexcept {
    jclass _type = env->FindClass("dev/luncliff/muffin/Renderer1");
    jlong value = 0;
    get_field(env, _type, self, "ptr", value);
    constexpr auto cap = 32 - sizeof(jlong);
    char txt[cap]{};
    snprintf(txt, cap, "%p", reinterpret_cast<void*>(value));
    return env->NewStringUTF(txt);
}

}  // extern "C"

/// @brief Group of java native type variables
/// @see https://programming.guide/java/list-of-java-exceptions.html
struct java_type_set_t final {
    jclass runtime_exception{};
    jclass illegal_argument_exception{};
    jclass illegal_state_exception{};
    jclass unsupported_operation_exception{};
    jclass index_out_of_bounds_exception{};

    jclass device_t{};
    jfieldID device_id_f{};
};

java_type_set_t java{};

void context_on_device_disconnected([[maybe_unused]] camera_group_t& context, ACameraDevice* device) noexcept {
    const char* id = ACameraDevice_getId(device);
    spdlog::error("on_device_disconnect: {}", id);
}

void context_on_device_error([[maybe_unused]] camera_group_t& context, ACameraDevice* device, int error) noexcept {
    const char* id = ACameraDevice_getId(device);
    spdlog::error("on_device_error: {} {}", id, error);
}

// session state callbacks

void context_on_session_active(camera_group_t&, ACameraCaptureSession*) noexcept { spdlog::debug("on_session_active"); }

void context_on_session_closed(camera_group_t&, ACameraCaptureSession*) noexcept { spdlog::warn("on_session_closed"); }

void context_on_session_ready(camera_group_t&, ACameraCaptureSession*) noexcept { spdlog::debug("on_session_ready"); }

// capture callbacks

void context_on_capture_started(camera_group_t&, ACameraCaptureSession*,
                                [[maybe_unused]] const ACaptureRequest* request, uint64_t time_point) noexcept {
    spdlog::debug("context_on_capture_started  : {}", time_point);
}

void context_on_capture_progressed(camera_group_t&, ACameraCaptureSession*, [[maybe_unused]] ACaptureRequest* request,
                                   const ACameraMetadata* result) noexcept {
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

void context_on_capture_completed(camera_group_t&, ACameraCaptureSession*, [[maybe_unused]] ACaptureRequest* request,
                                  const ACameraMetadata* result) noexcept {
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

void context_on_capture_failed(camera_group_t&, ACameraCaptureSession*, [[maybe_unused]] ACaptureRequest* request,
                               ACameraCaptureFailure* failure) noexcept {
    spdlog::error("context_on_capture_failed {} {} {} {}", failure->frameNumber, failure->reason, failure->sequenceId,
                  failure->wasImageCaptured);
}

void context_on_capture_buffer_lost(camera_group_t&, ACameraCaptureSession*, [[maybe_unused]] ACaptureRequest* request,
                                    ANativeWindow*, [[maybe_unused]] int64_t frame) noexcept {
    spdlog::error("context_on_capture_buffer_lost");
}

void context_on_capture_sequence_abort(camera_group_t&, ACameraCaptureSession*,
                                       [[maybe_unused]] int sequence) noexcept {
    spdlog::error("context_on_capture_sequence_abort");
}

void context_on_capture_sequence_complete(camera_group_t&, ACameraCaptureSession*, [[maybe_unused]] int sequence,
                                          [[maybe_unused]] int64_t frame) noexcept {
    spdlog::debug("context_on_capture_sequence_complete");
}

camera_group_t context{};

extern "C" {

void Java_dev_luncliff_muffin_DeviceQuery_Init(JNIEnv* env, jclass) noexcept {
    uint16_t num_camera = 0;
    camera_status_t status = ACAMERA_OK;

    // already initialized. do nothing
    if (context.manager != nullptr) return;

    // Find exception class information (type info)
    java.runtime_exception = env->FindClass("java/lang/RuntimeException");
    java.illegal_argument_exception = env->FindClass("java/lang/IllegalArgumentException");
    java.illegal_state_exception = env->FindClass("java/lang/IllegalStateException");
    java.unsupported_operation_exception = env->FindClass("java/lang/UnsupportedOperationException");
    java.index_out_of_bounds_exception = env->FindClass("java/lang/IndexOutOfBoundsException");

    // !!! Since we can't throw if this info is null, call assert !!!
    assert(java.runtime_exception != nullptr);
    assert(java.illegal_argument_exception != nullptr);
    assert(java.illegal_state_exception != nullptr);
    assert(java.unsupported_operation_exception != nullptr);
    assert(java.index_out_of_bounds_exception != nullptr);

    context.release();

    context.manager = ACameraManager_create();
    assert(context.manager != nullptr);

    status = ACameraManager_getCameraIdList(context.manager, &context.id_list);
    if (status != ACAMERA_OK) goto ThrowJavaException;

    // https://developer.android.com/reference/android/hardware/camera2/CameraMetadata
    // https://android.googlesource.com/platform/frameworks/av/+/2e19c3c/services/camera/libcameraservice/camera2/CameraMetadata.h
    num_camera = context.id_list->numCameras;
    // library must reserve enough space to support
    assert(num_camera <= camera_group_t::max_camera_count);

    for (uint16_t i = 0u; i < num_camera; ++i) {
        const char* cam_id = context.id_list->cameraIds[i];
        status =
            ACameraManager_getCameraCharacteristics(context.manager, cam_id, std::addressof(context.metadata_set[i]));
        if (status == ACAMERA_OK) continue;
        spdlog::error("{}: {}", "ACameraManager_getCameraCharacteristics", status);
        goto ThrowJavaException;
    }
ThrowJavaException:
    env->ThrowNew(java.illegal_argument_exception, camera_error_message(status));
}

jint Java_dev_luncliff_muffin_DeviceQuery_GetDeviceCount(JNIEnv*, jclass) noexcept {
    if (context.manager == nullptr)  // not initialized
        return 0;
    return context.id_list->numCameras;
}

void Java_dev_luncliff_muffin_DeviceQuery_SetDeviceData(JNIEnv* env, jclass, jobjectArray devices) noexcept {
    if (context.manager == nullptr)  // not initialized
        return;

    java.device_t = env->FindClass("ndcam/Device");
    assert(java.device_t != nullptr);

    const auto count = context.id_list->numCameras;
    assert(count == env->GetArrayLength(devices));

    // https://developer.android.com/ndk/reference/group/camera
    for (short index = 0; index < count; ++index) {
        ACameraMetadata_const_entry entry{};
        ACameraMetadata_getConstEntry(context.metadata_set[index], ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS,
                                      &entry);
        for (auto i = 0u; i < entry.count; i += 4) {
            const int32_t direction = entry.data.i32[i + 3];
            if (direction == ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_INPUT)
                ;
            if (direction == ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS_OUTPUT)
                ;

            const int32_t format = entry.data.i32[i + 0];
            const int32_t width = entry.data.i32[i + 1];
            const int32_t height = entry.data.i32[i + 2];

            if (format == AIMAGE_FORMAT_PRIVATE) spdlog::debug("Private: {} {} ", width, height);
            if (format == AIMAGE_FORMAT_YUV_420_888) spdlog::debug("YUV_420_888: {} {} ", width, height);
            if (format == AIMAGE_FORMAT_JPEG) spdlog::debug("JPEG: {} {} ", width, height);
            if (format == AIMAGE_FORMAT_RAW16) spdlog::debug("Raw16: {} {} ", width, height);
        }

        jobject device = env->GetObjectArrayElement(devices, index);
        assert(device != nullptr);

        java.device_id_f = env->GetFieldID(java.device_t, "id", "S");  // short
        assert(java.device_id_f != nullptr);

        env->SetShortField(device, java.device_id_f, index);
    }
}

jbyte Java_dev_luncliff_muffin_DeviceHandle_facing(JNIEnv* env, jobject self) noexcept {
    if (context.manager == nullptr)  // not initialized
        return JNI_FALSE;

    auto device_id = env->GetShortField(self, java.device_id_f);
    assert(device_id != -1);

    const auto facing = context.get_facing(static_cast<uint16_t>(device_id));
    return static_cast<jbyte>(facing);
}

void Java_dev_luncliff_muffin_DeviceHandle_open(JNIEnv* env, jobject self) noexcept {
    camera_status_t status = ACAMERA_OK;
    if (context.manager == nullptr)  // not initialized
        return;

    const auto id = env->GetShortField(self, java.device_id_f);
    assert(id != -1);

    ACameraDevice_StateCallbacks callbacks{};
    callbacks.context = std::addressof(context);
    callbacks.onDisconnected = reinterpret_cast<ACameraDevice_StateCallback>(context_on_device_disconnected);
    callbacks.onError = reinterpret_cast<ACameraDevice_ErrorStateCallback>(context_on_device_error);

    context.close_device(id);
    status = context.open_device(id, callbacks);

    if (status == ACAMERA_OK) return;

    // throw exception
    env->ThrowNew(java.runtime_exception, fmt::format("ACameraManager_openCamera: {}", status).c_str());
}

void Java_dev_luncliff_muffin_DeviceHandle_close(JNIEnv* env, jobject self) noexcept {
    if (context.manager == nullptr)  // not initialized
        return;

    const auto id = env->GetShortField(self, java.device_id_f);
    assert(id != -1);

    context.close_device(id);
}

// TODO: Resource cleanup
void Java_dev_luncliff_muffin_DeviceHandle_startRepeat(JNIEnv* env, jobject self, jobject surface) noexcept {
    camera_status_t status = ACAMERA_OK;
    if (context.manager == nullptr)  // not initialized
        return;
    const auto id = env->GetShortField(self, java.device_id_f);
    assert(id != -1);

    // `ANativeWindow_fromSurface` acquires a reference
    // `ANativeWindow_release` releases it
    // ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    auto window = native_window_ptr{ANativeWindow_fromSurface(env, surface), ANativeWindow_release};
    assert(window.get() != nullptr);

    ACameraCaptureSession_stateCallbacks on_state_changed{};
    on_state_changed.context = std::addressof(context);
    on_state_changed.onReady = reinterpret_cast<ACameraCaptureSession_stateCallback>(context_on_session_ready);
    on_state_changed.onClosed = reinterpret_cast<ACameraCaptureSession_stateCallback>(context_on_session_closed);
    on_state_changed.onActive = reinterpret_cast<ACameraCaptureSession_stateCallback>(context_on_session_active);

    ACameraCaptureSession_captureCallbacks on_capture_event{};
    on_capture_event.context = std::addressof(context);
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

    status = context.start_repeat(id, window.get(), on_state_changed, on_capture_event);
    assert(status == ACAMERA_OK);
}

void Java_dev_luncliff_muffin_DeviceHandle_stopRepeat(JNIEnv* env, jobject self) noexcept {
    if (context.manager == nullptr)  // not initialized
        return;

    const auto id = env->GetShortField(self, java.device_id_f);
    assert(id != -1);

    context.stop_repeat(id);
}

void Java_dev_luncliff_muffin_DeviceHandle_startCapture(JNIEnv* env, jobject self, jobject surface) noexcept {
    camera_status_t status = ACAMERA_OK;
    if (context.manager == nullptr)  // not initialized
        return;

    auto id = env->GetShortField(self, java.device_id_f);
    assert(id != -1);

    // `ANativeWindow_fromSurface` acquires a reference
    // `ANativeWindow_release` releases it
    // ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    auto window = native_window_ptr{ANativeWindow_fromSurface(env, surface), ANativeWindow_release};
    assert(window.get() != nullptr);

    ACameraCaptureSession_stateCallbacks on_state_changed{};
    on_state_changed.context = std::addressof(context);
    on_state_changed.onReady = reinterpret_cast<ACameraCaptureSession_stateCallback>(context_on_session_ready);
    on_state_changed.onClosed = reinterpret_cast<ACameraCaptureSession_stateCallback>(context_on_session_closed);
    on_state_changed.onActive = reinterpret_cast<ACameraCaptureSession_stateCallback>(context_on_session_active);

    ACameraCaptureSession_captureCallbacks on_capture_event{};
    on_capture_event.context = std::addressof(context);
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

    status = context.start_capture(id, window.get(), on_state_changed, on_capture_event);
    assert(status == ACAMERA_OK);
}

void Java_dev_luncliff_muffin_DeviceHandle_stopCapture(JNIEnv* env, jobject self) noexcept {
    if (context.manager == nullptr)  // not initialized
        return;

    const auto id = env->GetShortField(self, java.device_id_f);
    assert(id != -1);

    context.stop_capture(id);
}

}  // extern "C"
