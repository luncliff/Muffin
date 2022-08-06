#include "inference.hpp"

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

#define expect(expr) \
    if ((expr) == false) return;
#define MODEL_DIR ""

void test_runtime_version() {
    std::string version = TfLiteVersion();
    expect(version == "2.8.0");
}

void try_model_load(fs::path model_path) {
    expect(fs::exists(model_path));
    const auto p = model_path.generic_string();
    TfLiteModel* model = TfLiteModelCreateFromFile(p.c_str());
    expect(model != nullptr);
    TfLiteModelDelete(model);
}

void test_model_loading() {
    const auto models = fs::path{MODEL_DIR};
    try_model_load(models / "face_detection_short_range.tflite");
    try_model_load(models / "face_landmark.tflite");
    try_model_load(models / "iris_landmark.tflite");
    try_model_load(models / "selfie_segmentation_landscape.tflite");
}

void on_message(FILE*, const char* f, va_list args) {
    std::string txt{};
    txt.resize(1000);
    int len = snprintf(txt.data(), txt.size(), f, args);
    if (len < 0) return;  // failed to generate the message
    txt.resize(len);
}

void test_interpreter_options() {
    TfLiteInterpreterOptions* options = TfLiteInterpreterOptionsCreate();
    expect(options != nullptr);
    TfLiteInterpreterOptionsSetNumThreads(options, 1);
    using reporter_t = void (*)(void*, const char*, va_list);
    TfLiteInterpreterOptionsSetErrorReporter(options, reinterpret_cast<reporter_t>(&on_message), stdout);
    // TfLiteInterpreterOptionsAddDelegate(options, nullptr);
    TfLiteInterpreterOptionsDelete(options);
}

void try_interpreter(TfLiteInterpreterOptions* options,  //
                     fs::path model_path, int32_t input_count, int32_t output_count) {
    expect(fs::exists(model_path));
    const auto p = model_path.generic_string();
    TfLiteModel* model = TfLiteModelCreateFromFile(p.c_str());
    expect(model != nullptr);
    TfLiteInterpreter* interpreter = TfLiteInterpreterCreate(model, options);
    TfLiteModelDelete(model);
    expect(interpreter != nullptr);
    expect(TfLiteInterpreterGetInputTensorCount(interpreter) == input_count);
    expect(TfLiteInterpreterGetOutputTensorCount(interpreter) == output_count);
    TfLiteInterpreterDelete(interpreter);
}

void test_interpreter_from_models() {
    TfLiteInterpreterOptions* options = TfLiteInterpreterOptionsCreate();
    expect(options != nullptr);
    using reporter_t = void (*)(void* user_data, const char* format, va_list args);
    TfLiteInterpreterOptionsSetErrorReporter(options, reinterpret_cast<reporter_t>(&vfprintf), stdout);

    const auto models = fs::path{MODEL_DIR};
    try_interpreter(options, models / "face_detection_short_range.tflite", 1, 2);
    try_interpreter(options, models / "face_landmark.tflite", 1, 2);
    try_interpreter(options, models / "iris_landmark.tflite", 1, 2);
    try_interpreter(options, models / "selfie_segmentation_landscape.tflite", 1, 1);

    TfLiteInterpreterOptionsDelete(options);
}
