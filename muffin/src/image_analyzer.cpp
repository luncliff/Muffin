#include "image_analyzer.hpp"

#include <spdlog/spdlog.h>

#include <memory>
#include <system_error>

#include "ndk_buffer.hpp"

using ndk_image_owner = std::unique_ptr<AImage, void (*)(AImage*)>;

ndk_image_owner acquire_latest(AImageReader* reader) noexcept(false) {
    AImage* image = nullptr;
    auto ec = AImageReader_acquireLatestImage(reader, &image);
    if (ec != 0) throw std::system_error{ec, std::generic_category(), "AImageReader_acquireLatestImage"};
    return {image, &AImage_delete};
}

void image_analyzer_t::on_image(image_analyzer_t& self, AImageReader* reader) noexcept(false) {
    try {
        auto image = acquire_latest(reader);
        ndk_hardware_buffer_t ahwb{image.get()};
        AHardwareBuffer_Desc desc{};
        ahwb.get(desc);
        spdlog::debug("{}: {:x} {} {}", "on_image", desc.format, desc.width, desc.height);
    } catch (const std::exception& ex) {
        spdlog::error("{}: {}", "on_image", ex.what());
    }
}

AImageReader_ImageListener image_analyzer_t::make_listener() noexcept {
    AImageReader_ImageListener output{};
    output.context = this;
    output.onImageAvailable = reinterpret_cast<AImageReader_ImageCallback>(&on_image);
    return output;
}
