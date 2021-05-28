// clang-format off
#include <cstddef>
#include <cstdint>
#include <cstdio>

#include <jerror.h>
#include <jpeglib.h>
#include <libexif/exif-data.h>
#include <libexif/exif-loader.h>
#include <png.h>
#include <webp/types.h>
#include <webp/decode.h>
#include <webp/encode.h>
// clang-format on

#include <fmt/printf.h>
#include <spdlog/spdlog.h>
#include <gsl/gsl>

void on_error_JPEG(j_common_ptr ptr) noexcept {
    jpeg_error_mgr* em = ptr->err;
    const auto ec = em->msg_code;
    if (ec > em->last_jpeg_message) {
        spdlog::warn("Unknown libjpeg error: {}", ec);
        return;
    }
    const auto txt = fmt::sprintf(em->jpeg_message_table[ec], ec);
    spdlog::warn(txt);
}

uint32_t decode_JPEG(FILE* fp) {
    struct reader_t final {
        jpeg_decompress_struct ctx{};
        jpeg_error_mgr em{};

       public:
        reader_t() noexcept {
            jpeg_create_decompress(&ctx);
            ctx.err = jpeg_std_error(&em);
            em.error_exit = em.output_message = on_error_JPEG;
        }
        ~reader_t() noexcept { jpeg_destroy_decompress(&ctx); }

        [[nodiscard]] uint32_t get_error_code() const noexcept {
            return static_cast<uint32_t>(em.msg_code);
        }

        [[nodiscard]] uint32_t bind(FILE* fin, int& width, int& height,
                                    int& num_component) noexcept {
            jpeg_stdio_src(&ctx, fin);
            switch (const auto code = jpeg_read_header(&ctx, TRUE)) {
                case JPEG_HEADER_TABLES_ONLY:
                    spdlog::debug("header table only");
                case JPEG_HEADER_OK:
                    break;
                case JPEG_SUSPENDED:
                    spdlog::warn("suspended");
                    return EXIT_FAILURE;
                default:
                    return code;
            }
            width = ctx.image_width;
            height = ctx.image_height;
            num_component = ctx.num_components;
            return EXIT_SUCCESS;
        }
        void read() noexcept {
            jpeg_start_decompress(&ctx);
            const int row_stride = ctx.output_width * ctx.output_components;
            JSAMPARRAY buffer = (*ctx.mem->alloc_sarray)(
                (j_common_ptr)&ctx, JPOOL_IMAGE, row_stride, 1);
            while (ctx.output_scanline < ctx.output_height) {
                jpeg_read_scanlines(&ctx, buffer, 1);
                // put_scanline_someplace(buffer[0], row_stride);
            }
            jpeg_finish_decompress(&ctx);
        }
    };
    spdlog::debug("Decode:");
    spdlog::debug("  JPEG: {}", JPEG_LIB_VERSION);
    spdlog::debug("  jpeg_decompress: {}", sizeof(jpeg_decompress_struct));
    reader_t reader{};
    // ...
    return reader.get_error_code();
}

uint32_t encode_JPEG(FILE* fp) {
    struct writer_t final {
        jpeg_compress_struct ctx{};
        jpeg_error_mgr em{};

       public:
        writer_t() noexcept {
            jpeg_create_compress(&ctx);
            ctx.err = jpeg_std_error(&em);
            em.error_exit = em.output_message = on_error_JPEG;
        }
        ~writer_t() noexcept { jpeg_destroy_compress(&ctx); }

        [[nodiscard]] uint32_t get_error_code() const noexcept {
            return static_cast<uint32_t>(em.msg_code);
        }

        [[nodiscard]] uint32_t bind(FILE* fout, int width, int height,
                                    int num_component) {
            jpeg_stdio_dest(&ctx, fout);
            ctx.image_width = width;
            ctx.image_height = height;
            ctx.input_components = num_component;
            switch (num_component) {
                case 1:
                    ctx.in_color_space = JCS_GRAYSCALE;
                    break;
                case 3:
                    ctx.in_color_space = JCS_RGB;
                    break;
                case 4:
                    ctx.in_color_space = JCS_EXT_RGBX;
                    break;
                default:
                    return EINVAL;
            }
            jpeg_set_defaults(&ctx);
            jpeg_set_quality(&ctx, 90, TRUE);
            return 0;
        }
        void write(JSAMPLE* image_buffer, int row_stride) noexcept {
            jpeg_start_compress(&ctx, TRUE);
            if (row_stride == 0)
                row_stride = ctx.image_width * ctx.input_components;
            JSAMPROW row_pointer[1]{};
            while (ctx.next_scanline < ctx.image_height) {
                row_pointer[0] = &image_buffer[ctx.next_scanline * row_stride];
                jpeg_write_scanlines(&ctx, row_pointer, 1);
            }
            jpeg_finish_compress(&ctx);
        }
    };
    spdlog::debug("Encode:");
    spdlog::debug("  JPEG: {}", JPEG_LIB_VERSION);
    spdlog::debug("  jpeg_compress: {}", sizeof(jpeg_compress_struct));
    writer_t writer{};
    // ...
    return writer.get_error_code();
}

void on_output_EXIF(ExifLog*, ExifLogCode code, gsl::czstring<> domain,
                    gsl::czstring<> format, va_list args, void*) noexcept {
    gsl::czstring<> title = exif_log_code_get_title(code);
    gsl::czstring<> message = exif_log_code_get_message(code);
    spdlog::debug("{} {}", title, message);
}

uint32_t decode_EXIF(gsl::span<std::byte> mem) noexcept {
    spdlog::debug("Decode:");
    spdlog::debug("  EXIF");
    auto alloc = exif_mem_new_default();
    auto loader = std::unique_ptr<ExifLoader, void (*)(ExifLoader*)>{
        exif_loader_new_mem(alloc), &exif_loader_reset};
    auto logging = std::unique_ptr<ExifLog, void (*)(ExifLog*)>{
        exif_log_new_mem(alloc), &exif_log_free};
    exif_log_set_func(logging.get(), on_output_EXIF, nullptr);
    exif_loader_log(loader.get(), logging.get());
    if (exif_loader_write(loader.get(),
                          reinterpret_cast<unsigned char*>(mem.data()),
                          mem.size()) == 0) {
        spdlog::error("failed: {}", "exif_loader_write");
        return EINVAL;
    }
    auto data = std::unique_ptr<ExifData, void (*)(ExifData*)>{
        exif_loader_get_data(loader.get()), &exif_data_free};
    exif_data_get_byte_order(data.get());
    exif_data_get_mnote_data(data.get());
    exif_data_get_data_type(data.get());
    return 0;
}

/// @see https://developers.google.com/speed/webp/docs/api
uint32_t decode_WEBP(FILE* fin) noexcept {
    spdlog::debug("Decode:");
    spdlog::debug("  WEBP: {:#x}", WebPGetDecoderVersion());
    WebPDecoderConfig config{};
    if (WebPInitDecoderConfig(&config) == false) {
        spdlog::error("failed: {}", "WebPInitDecoderConfig");
        return ENOTSUP;
    }
    WebPDecoderOptions& options = config.options;
    // ...
    auto decoder = std::unique_ptr<WebPIDecoder, void (*)(WebPIDecoder*)>{
        WebPINewDecoder(&config.output), &WebPIDelete};
    bool errored = false;
    while (errored == false) {
        switch (WebPIAppend(decoder.get(), nullptr, 0)) {
            case VP8_STATUS_SUSPENDED:
                errored = true;
                continue;
            case VP8_STATUS_OK:
            default:
                break;
        }
    }
    WebPFreeDecBuffer(&config.output);
    return ENOTSUP;
}

/// @see https://developers.google.com/speed/webp/docs/api
uint32_t encode_WEBP(FILE* fout) noexcept {
    spdlog::debug("Encode:");
    spdlog::debug("  WEBP: {:#x}", WebPGetEncoderVersion());
    WebPConfig config;
    if (WebPConfigInit(&config) == false) {
        spdlog::error("failed: {}", "WebPConfigInit");
        return ENOTSUP;
    }
    return ENOTSUP;
}
