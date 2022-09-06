#pragma once
#include <android/hardware_buffer.h>
#include <android/native_window_jni.h>
#include <camera/NdkCameraCaptureSession.h>
#include <camera/NdkCameraDevice.h>
#include <camera/NdkCameraError.h>
#include <camera/NdkCameraManager.h>
#include <camera/NdkCaptureRequest.h>
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>

#include <array>
#include <memory>
#include <system_error>
#include <vector>

/**
 * @see https://developer.android.com/ndk/reference/group/camera
 * @see NdkCameraError.h
 */
struct ndk_camera_error_category_t final : public std::error_category {
    const char* name() const noexcept override;
    std::string message(int status) const override;

   public:
    /// @brief Change ACamera error codes to string
    static const char* get_message(camera_status_t status) noexcept;
};

ndk_camera_error_category_t& get_ndk_camera_errors() noexcept;

/**
 * @see https://developer.android.com/ndk/reference/group/camera
 */
struct ndk_camera_session_t final {
    ACameraDevice* device = nullptr;
    ACameraCaptureSession* session = nullptr;
    uint16_t index = UINT16_MAX;
    bool repeating = false;                      // flag to indicate if the session is repeating
    int sequence_id = CAPTURE_SEQUENCE_ID_NONE;  // sequence ID from capture session
};

struct ndk_capture_configuration_t final {
    using handler_t = void (*)(void*, ACaptureRequest*);
    void* context;
    handler_t handler;
};

/**
 * @brief Wrapper of `ACameraManager`, After the instance is initialized,
 *  All of the members must be non-null.
 *  If there is no external camera, it assumes there are 2 cameras(back + front).
 * @details Even though android system limits the number of maximum open camera device, we will consider multiple camera are working concurrently.
 * @see https://developer.android.com/ndk/reference/group/camera
 */
class ndk_camera_manager_t final {
    ACameraManager* manager = nullptr;
    ACameraIdList* id_list = nullptr;
    std::array<ACameraMetadata*, 4> metadatas{};  // cached metadata
    ACameraManager_AvailabilityCallbacks callbacks0{};

   public:
    ndk_camera_manager_t() noexcept(false);
    ~ndk_camera_manager_t() noexcept;
    ndk_camera_manager_t(const ndk_camera_manager_t&) = delete;
    ndk_camera_manager_t(ndk_camera_manager_t&&) = delete;
    ndk_camera_manager_t& operator=(const ndk_camera_manager_t&) = delete;
    ndk_camera_manager_t& operator=(ndk_camera_manager_t&&) = delete;

   private:
    static void camera_available(ndk_camera_manager_t& self, const char* id);
    static void camera_unavailable(ndk_camera_manager_t& self, const char* id);
    void on_camera_available(const char* id);
    void on_camera_unavailable(const char* id);

   public:
    uint32_t count() const noexcept;

    camera_status_t open_device(uint32_t idx, ndk_camera_session_t& info,
                                ACameraDevice_StateCallbacks& callbacks) noexcept;

    /// @note The function doesn't free metadata
    void close_device(ndk_camera_session_t& info) noexcept;

    /// @see https://developer.android.com/ndk/reference/group/camera#acameradevice_createcapturesession
    camera_status_t start_capture(ndk_camera_session_t& info, ACameraCaptureSession_stateCallbacks& on_state_change,
                                  ACameraCaptureSession_captureCallbacks& on_capture_event,  //
                                  ANativeWindow* window) noexcept(false);
    camera_status_t start_capture(ndk_camera_session_t& info, ndk_capture_configuration_t& config,
                                  ACameraCaptureSession_stateCallbacks& on_state_change,
                                  ACameraCaptureSession_captureCallbacks& on_capture_event,  //
                                  ANativeWindow* window) noexcept(false);
    /// @see https://developer.android.com/ndk/reference/group/camera#acameradevice_createcapturesession
    camera_status_t start_repeat(ndk_camera_session_t& info, ACameraCaptureSession_stateCallbacks& on_state_change,
                                 ACameraCaptureSession_captureCallbacks& on_capture_event,  //
                                 ANativeWindow* window) noexcept(false);
    camera_status_t start_repeat(ndk_camera_session_t& info, ndk_capture_configuration_t& config,
                                 ACameraCaptureSession_stateCallbacks& on_state_change,
                                 ACameraCaptureSession_captureCallbacks& on_capture_event,  //
                                 ANativeWindow* window) noexcept(false);
    void close_session(ndk_camera_session_t& info) noexcept(false);

    uint32_t get_index(const char* id) const noexcept;
    uint32_t get_index(ACameraDevice* device) const noexcept;
    ACameraMetadata* get_metadata(ACameraDevice* device) const noexcept;

    static void print(const char* device, ACameraMetadata* metadata) noexcept;
};

/**
 * @defgroup DeviceCallbacks
 */

void context_on_device_disconnected(ndk_camera_manager_t& context, ACameraDevice* device) noexcept;

void context_on_device_error(ndk_camera_manager_t& context, ACameraDevice* device, int error) noexcept;

/**
 * @defgroup SessionStateCallbacks
 */

void context_on_session_active(ndk_camera_manager_t& context, ACameraCaptureSession* session) noexcept;

void context_on_session_closed(ndk_camera_manager_t& context, ACameraCaptureSession* session) noexcept;

void context_on_session_ready(ndk_camera_manager_t& context, ACameraCaptureSession* session) noexcept;

/**
 * @defgroup CaptureCallbacks
 */

void context_on_capture_started(ndk_camera_manager_t& context, ACameraCaptureSession* session,
                                const ACaptureRequest* request, uint64_t time_point) noexcept;

void context_on_capture_progressed(ndk_camera_manager_t& context, ACameraCaptureSession* session,
                                   ACaptureRequest* request, const ACameraMetadata* result) noexcept;

void context_on_capture_completed(ndk_camera_manager_t& context, ACameraCaptureSession* session,
                                  ACaptureRequest* request, const ACameraMetadata* result) noexcept;

void context_on_capture_failed(ndk_camera_manager_t& context, ACameraCaptureSession* session, ACaptureRequest* request,
                               ACameraCaptureFailure* failure) noexcept;

void context_on_capture_buffer_lost(ndk_camera_manager_t& context, ACameraCaptureSession* session,
                                    ACaptureRequest* request, ANativeWindow* window, int64_t frame_number) noexcept;

void context_on_capture_sequence_abort(ndk_camera_manager_t& context, ACameraCaptureSession* session,
                                       int sequence_id) noexcept;

void context_on_capture_sequence_complete(ndk_camera_manager_t& context, ACameraCaptureSession* session,
                                          int sequence_id, int64_t frame_number) noexcept;
