#include "ndk_camera.hpp"

#include <spdlog/spdlog.h>

using namespace std;

void camera_group_t::release() noexcept {
    // close all devices
    for (uint16_t id = 0u; id < max_camera_count; ++id) close_device(id);

    // release all metadata
    for (auto& meta : metadata_set)
        if (meta) {
            ACameraMetadata_free(meta);
            meta = nullptr;
        }

    // remove id list
    if (id_list) ACameraManager_deleteCameraIdList(id_list);
    id_list = nullptr;

    // release manager (camera service)
    if (manager) ACameraManager_delete(manager);
    manager = nullptr;
}

camera_status_t camera_group_t::open_device(uint16_t id, ACameraDevice_StateCallbacks& callbacks) noexcept {
    auto& device = this->device_set[id];
    auto status = ACameraManager_openCamera(          //
        this->manager, this->id_list->cameraIds[id],  //
        addressof(callbacks), addressof(device));
    return status;
}

// Notice that this routine doesn't free metadata
void camera_group_t::close_device(uint16_t id) noexcept {
    // close session
    auto& session = this->session_set[id];
    if (session) {
        spdlog::warn("session for device {} is alive. abort/closing...", id);

        // Abort all kind of requests
        ACameraCaptureSession_abortCaptures(session);
        ACameraCaptureSession_stopRepeating(session);
        // close
        ACameraCaptureSession_close(session);
        session = nullptr;
    }
    // close device
    auto& device = this->device_set[id];
    if (device) {
        // Producing meesage like following
        // W/ACameraCaptureSession: Device is closed but session 0 is not
        // notified
        //
        // Seems like ffmpeg also has same issue, but can't sure about its
        // comment...
        //
        spdlog::warn("closing device {} ...", id);

        ACameraDevice_close(device);
        device = nullptr;
    }
}

camera_status_t camera_group_t::start_repeat(uint16_t id, ANativeWindow* window,
                                             ACameraCaptureSession_stateCallbacks& on_session_changed,
                                             ACameraCaptureSession_captureCallbacks& on_capture_event) noexcept {
    camera_status_t status = ACAMERA_OK;

    // ---- target surface for camera ----
    auto target = camera_output_target_ptr{[=]() {
                                               ACameraOutputTarget* target{};
                                               ACameraOutputTarget_create(window, addressof(target));
                                               return target;
                                           }(),
                                           ACameraOutputTarget_free};
    assert(target.get() != nullptr);

    // ---- capture request (preview) ----
    auto request = capture_request_ptr{[=]() {
                                           ACaptureRequest* ptr{};
                                           // capture as a preview
                                           // TEMPLATE_RECORD, TEMPLATE_PREVIEW, TEMPLATE_MANUAL,
                                           const auto status = ACameraDevice_createCaptureRequest(
                                               this->device_set[id], TEMPLATE_PREVIEW, &ptr);
                                           assert(status == ACAMERA_OK);
                                           return ptr;
                                       }(),
                                       ACaptureRequest_free};
    assert(request.get() != nullptr);

    // `ACaptureRequest` == how to capture
    // detailed config comes here...
    // ACaptureRequest_setEntry_*
    // - ACAMERA_REQUEST_MAX_NUM_OUTPUT_STREAMS
    // -

    // designate target surface in request
    status = ACaptureRequest_addTarget(request.get(), target.get());
    assert(status == ACAMERA_OK);
    // ---- session output ----

    // container for multiplexing of session output
    auto container = capture_session_output_container_ptr{[]() {
                                                              ACaptureSessionOutputContainer* container{};
                                                              ACaptureSessionOutputContainer_create(&container);
                                                              return container;
                                                          }(),
                                                          ACaptureSessionOutputContainer_free};
    assert(container.get() != nullptr);

    // session output
    auto output = capture_session_output_ptr{[=]() {
                                                 ACaptureSessionOutput* output{};
                                                 ACaptureSessionOutput_create(window, &output);
                                                 return output;
                                             }(),
                                             ACaptureSessionOutput_free};
    assert(output.get() != nullptr);

    status = ACaptureSessionOutputContainer_add(container.get(), output.get());
    assert(status == ACAMERA_OK);

    // ---- create a session ----
    status = ACameraDevice_createCaptureSession(this->device_set[id], container.get(), addressof(on_session_changed),
                                                addressof(this->session_set[id]));
    assert(status == ACAMERA_OK);

    // ---- set request ----
    array<ACaptureRequest*, 1> batch_request{};
    batch_request[0] = request.get();

    status = ACameraCaptureSession_setRepeatingRequest(this->session_set[id], addressof(on_capture_event),
                                                       batch_request.size(), batch_request.data(),
                                                       addressof(this->seq_id_set[id]));
    assert(status == ACAMERA_OK);

    status = ACaptureSessionOutputContainer_remove(container.get(), output.get());
    assert(status == ACAMERA_OK);
    status = ACaptureRequest_removeTarget(request.get(), target.get());
    assert(status == ACAMERA_OK);

    return status;
}

void camera_group_t::stop_repeat(uint16_t id) noexcept {
    auto& session = this->session_set[id];
    if (session) {
        spdlog::warn("stop_repeat for session {} ", id);

        // follow `ACameraCaptureSession_setRepeatingRequest`
        ACameraCaptureSession_stopRepeating(session);

        ACameraCaptureSession_close(session);
        session = nullptr;
    }
    this->seq_id_set[id] = CAPTURE_SEQUENCE_ID_NONE;
}

camera_status_t camera_group_t::start_capture(uint16_t id, ANativeWindow* window,
                                              ACameraCaptureSession_stateCallbacks& on_session_changed,
                                              ACameraCaptureSession_captureCallbacks& on_capture_event) noexcept {
    camera_status_t status = ACAMERA_OK;

    // ---- target surface for camera ----
    auto target = camera_output_target_ptr{[=]() {
                                               ACameraOutputTarget* target{};
                                               ACameraOutputTarget_create(window, addressof(target));
                                               return target;
                                           }(),
                                           ACameraOutputTarget_free};
    assert(target.get() != nullptr);

    // ---- capture request (preview) ----
    auto request = capture_request_ptr{[](ACameraDevice* device) {
                                           ACaptureRequest* ptr{};
                                           // capture as a preview
                                           // TEMPLATE_RECORD, TEMPLATE_PREVIEW,
                                           // TEMPLATE_MANUAL,
                                           const auto status =
                                               ACameraDevice_createCaptureRequest(device, TEMPLATE_STILL_CAPTURE, &ptr);
                                           assert(status == ACAMERA_OK);
                                           return ptr;
                                       }(this->device_set[id]),
                                       ACaptureRequest_free};
    assert(request.get() != nullptr);

    // `ACaptureRequest` == how to capture
    // detailed config comes here...
    // ACaptureRequest_setEntry_*
    // - ACAMERA_REQUEST_MAX_NUM_OUTPUT_STREAMS
    // -

    // designate target surface in request
    status = ACaptureRequest_addTarget(request.get(), target.get());
    assert(status == ACAMERA_OK);
    // defer    ACaptureRequest_removeTarget;

    // ---- session output ----

    // container for multiplexing of session output
    auto container = capture_session_output_container_ptr{[]() {
                                                              ACaptureSessionOutputContainer* container{};
                                                              ACaptureSessionOutputContainer_create(&container);
                                                              return container;
                                                          }(),
                                                          ACaptureSessionOutputContainer_free};
    assert(container.get() != nullptr);

    // session output
    auto output = capture_session_output_ptr{[=]() {
                                                 ACaptureSessionOutput* output{};
                                                 ACaptureSessionOutput_create(window, &output);
                                                 return output;
                                             }(),
                                             ACaptureSessionOutput_free};
    assert(output.get() != nullptr);

    status = ACaptureSessionOutputContainer_add(container.get(), output.get());
    assert(status == ACAMERA_OK);
    // defer ACaptureSessionOutputContainer_remove

    // ---- create a session ----
    status = ACameraDevice_createCaptureSession(this->device_set[id], container.get(), addressof(on_session_changed),
                                                addressof(this->session_set[id]));
    assert(status == ACAMERA_OK);

    // ---- set request ----
    array<ACaptureRequest*, 1> batch_request{};
    batch_request[0] = request.get();

    status = ACameraCaptureSession_capture(this->session_set[id], addressof(on_capture_event), batch_request.size(),
                                           batch_request.data(), addressof(this->seq_id_set[id]));
    assert(status == ACAMERA_OK);

    status = ACaptureSessionOutputContainer_remove(container.get(), output.get());
    assert(status == ACAMERA_OK);

    status = ACaptureRequest_removeTarget(request.get(), target.get());
    assert(status == ACAMERA_OK);

    return status;
}

void camera_group_t::stop_capture(uint16_t id) noexcept {
    auto& session = this->session_set[id];
    if (session) {
        spdlog::warn("stop_capture for session {} ", id);

        // follow `ACameraCaptureSession_capture`
        ACameraCaptureSession_abortCaptures(session);

        ACameraCaptureSession_close(session);
        session = nullptr;
    }
    this->seq_id_set[id] = 0;
}

auto camera_group_t::get_facing(uint16_t id) noexcept -> uint16_t {
    // const ACameraMetadata*
    const auto* metadata = metadata_set[id];

    ACameraMetadata_const_entry entry{};
    ACameraMetadata_getConstEntry(metadata, ACAMERA_LENS_FACING, &entry);

    // lens facing
    const auto facing = *(entry.data.u8);
    assert(facing == ACAMERA_LENS_FACING_FRONT ||  // ACAMERA_LENS_FACING_FRONT
           facing == ACAMERA_LENS_FACING_BACK ||   // ACAMERA_LENS_FACING_BACK
           facing == ACAMERA_LENS_FACING_EXTERNAL  // ACAMERA_LENS_FACING_EXTERNAL
    );
    return facing;
}

/// @see  NdkCameraError.h
const char* camera_error_message(camera_status_t status) noexcept {
    switch (status) {
        case ACAMERA_ERROR_UNKNOWN:
            return "Camera operation has failed due to an unspecified cause.";
        case ACAMERA_ERROR_INVALID_PARAMETER:
            return "Camera operation has failed due to an invalid parameter being "
                   "passed to the method.";
        case ACAMERA_ERROR_CAMERA_DISCONNECTED:
            return "Camera operation has failed because the camera device has been "
                   "closed, possibly because a higher-priority client has taken "
                   "ownership of the camera device.";
        case ACAMERA_ERROR_NOT_ENOUGH_MEMORY:
            return "Camera operation has failed due to insufficient memory.";
        case ACAMERA_ERROR_METADATA_NOT_FOUND:
            return "Camera operation has failed due to the requested metadata tag "
                   "cannot be found in input. ACameraMetadata or ACaptureRequest";
        case ACAMERA_ERROR_CAMERA_DEVICE:
            return "Camera operation has failed and the camera device has "
                   "encountered a fatal error and needs to be re-opened before it "
                   "can be used again.";
        case ACAMERA_ERROR_CAMERA_SERVICE:
            /**
             * Camera operation has failed and the camera service has encountered a
             * fatal error.
             *
             * <p>The Android device may need to be shut down and restarted to
             * restore camera function, or there may be a persistent hardware
             * problem.</p>
             *
             * <p>An attempt at recovery may be possible by closing the
             * ACameraDevice and the ACameraManager, and trying to acquire all
             * resources again from scratch.</p>
             */
            return "Camera operation has failed and the camera service has "
                   "encountered a fatal error.";
        case ACAMERA_ERROR_SESSION_CLOSED:
            return "The ACameraCaptureSession has been closed and cannot perform "
                   "any operation other than ACameraCaptureSession_close.";
        case ACAMERA_ERROR_INVALID_OPERATION:
            return "Camera operation has failed due to an invalid internal "
                   "operation. Usually this is due to a low-level problem that may "
                   "resolve itself on retry";
        case ACAMERA_ERROR_STREAM_CONFIGURE_FAIL:
            return "Camera device does not support the stream configuration "
                   "provided by application in ACameraDevice_createCaptureSession.";
        case ACAMERA_ERROR_CAMERA_IN_USE:
            return "Camera device is being used by another higher priority camera "
                   "API client.";
        case ACAMERA_ERROR_MAX_CAMERA_IN_USE:
            return "The system-wide limit for number of open cameras or camera "
                   "resources has been reached, and more camera devices cannot be "
                   "opened until previous instances are closed.";
        case ACAMERA_ERROR_CAMERA_DISABLED:
            return "The camera is disabled due to a device policy, and cannot be "
                   "opened.";
        case ACAMERA_ERROR_PERMISSION_DENIED:
            return "The application does not have permission to open camera.";
        default:
            return "ACAMERA_OK";
    }
}
