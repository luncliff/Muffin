#pragma once
#include <media/NdkImage.h>
#include <media/NdkImageReader.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>

#include "egl_context.hpp"

class image_analyzer_t final {
   private:
    static void on_image(image_analyzer_t& self, AImageReader* reader) noexcept(false);

   public:
    AImageReader_ImageListener make_listener() noexcept;
};

class async_image_analyzer_t final {
    int epfd = 0;
    int events[3]{};
};
