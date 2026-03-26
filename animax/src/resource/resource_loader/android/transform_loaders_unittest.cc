// Copyright 2024 The Lynx Authors. All rights reserved.
// Licensed under the Apache License Version 2.0 that can be found in the
// LICENSE file in the root directory of this source tree.
#include <cstring>
#include <fstream>
#include <future>
#include <memory>

#include "base/include/platform/android/jni_convert_helper.h"
#include "base/include/platform/android/jni_utils.h"
#include "base/include/platform/android/scoped_java_ref.h"
#include "gtest/gtest.h"
#include "include/resource/resource_task.h"
#include "src/resource/loader/lambda_loader.h"
#include "src/resource/loader/pipe_loader.h"
#include "src/resource/resource_loader.h"
#include "src/resource/resource_loader/android/animax_closeable_bitmap_reference.h"
#include "src/resource/resource_loader/android/transform_loader_android.h"
#include "src/resource/resource_loader/android/transform_loaders_util.h"

using namespace lynx::animax;

class AndroidTransformLoadersTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Set up any necessary resources or state for the test here
  }

  void TearDown() override {
    // Clean up any resources or state after the test here
  }

 protected:
  std::shared_ptr<TransformLoaderAndroid> loader_{
      std::shared_ptr<TransformLoaderAndroid>(new TransformLoaderAndroid())};
};

TEST_F(AndroidTransformLoadersTest, ValidJStringToNativeString) {
  auto specific_loader =
      loader_->GetLoader(TransformRequestInputTypeAndroid::kJStringFilePath,
                         ResourcePayloadType::kFilePath);
  JNIEnv* env = lynx::base::android::AttachCurrentThread();

  const char* native_c_string = "Hello from JNI!";
  auto native_std_string = std::string(native_c_string);
  auto j_string = env->NewStringUTF(native_c_string);

  auto promise = std::promise<void>{};
  auto future = promise.get_future();

  auto callback = TransformLoaderBaseAndroid::CallbackType{
      [native_std_string, &promise](ResourceResponse response,
                                    LoaderError err) {
        ASSERT_EQ(response.payload.path, native_std_string);
        ASSERT_EQ(response.payload.type, ResourcePayloadType::kFilePath);
        promise.set_value();
      }};
  specific_loader->Load(
      TransformRequestAndroid{
          .input =
              lynx::base::android::ScopedGlobalJavaRef<jobject>{env, j_string},
          .input_type = TransformRequestInputTypeAndroid::kJStringFilePath,
          .output_type = ResourcePayloadType::kFilePath},
      std::move(callback));
  EXPECT_NE(std::future_status::timeout,
            future.wait_for(std::chrono::seconds(1)));
}

// Function to transfer pixel data from C++ to Android's Bitmap via JNI
void transferPixelsToAndroid(JNIEnv* env, jclass bitmapClass, jobject bitmap,
                             uint8_t* pixels, int width, int height) {
  jintArray pixelArray = env->NewIntArray(width * height);
  if (pixelArray == nullptr) {
    return;  // Failed to allocate the array
  }

  // Convert pixels from RGBA (C++) to ARGB_8888 (Android)
  jint* pixelData = new jint[width * height];
  for (int i = 0; i < width * height; ++i) {
    uint8_t r = pixels[i * 4 + 0];
    uint8_t g = pixels[i * 4 + 1];
    uint8_t b = pixels[i * 4 + 2];
    uint8_t a = pixels[i * 4 + 3];
    pixelData[i] = (a << 24) | (r << 16) | (g << 8) | b;  // ARGB_8888 format
  }

  // Set the pixel data in the jintArray
  env->SetIntArrayRegion(pixelArray, 0, width * height, pixelData);
  delete[] pixelData;

  // Get the method ID for setPixels
  jmethodID setPixelsMethod =
      env->GetMethodID(bitmapClass, "setPixels", "([IIIIIII)V");
  if (setPixelsMethod == nullptr) {
    return;  // Handle error
  }

  // Set the pixels on the Android Bitmap
  env->CallVoidMethod(bitmap, setPixelsMethod, pixelArray, 0, width, 0, 0,
                      width, height);

  // Cleanup
  env->DeleteLocalRef(pixelArray);
}

// Another C++ function that creates the bitmap and calls
// transferPixelsToAndroid
jobject createAndTransferBitmap(JNIEnv* env) {
  int width = 1;
  int height = 1;

  // Create a buffer for pixel data (RGBA format in C++)
  std::unique_ptr<uint8_t[]> pixels(new uint8_t[width * height * 4]);
  for (int i = 0; i < width * height * 4; i += 4) {
    pixels[i] = 255;      // Red
    pixels[i + 1] = 0;    // Green
    pixels[i + 2] = 0;    // Blue
    pixels[i + 3] = 255;  // Alpha (fully opaque)
  }

  // Find the Bitmap class
  jclass bitmapClass = env->FindClass("android/graphics/Bitmap");

  // Find the static method Bitmap.createBitmap(width, height, Bitmap.Config)
  jmethodID createBitmapMethod = env->GetStaticMethodID(
      bitmapClass, "createBitmap",
      "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");

  // Find the Bitmap.Config.ARGB_8888 field
  jclass bitmapConfigClass = env->FindClass("android/graphics/Bitmap$Config");
  jfieldID argb8888Field = env->GetStaticFieldID(
      bitmapConfigClass, "ARGB_8888", "Landroid/graphics/Bitmap$Config;");
  jobject argb8888 =
      env->GetStaticObjectField(bitmapConfigClass, argb8888Field);

  // Create a Bitmap object with ARGB_8888 config
  jobject bitmap = env->CallStaticObjectMethod(bitmapClass, createBitmapMethod,
                                               width, height, argb8888);

  // Transfer the pixel data to the created Bitmap
  transferPixelsToAndroid(env, bitmapClass, bitmap, pixels.get(), width,
                          height);

  // Now the Java bitmap contains the pixel data from the C++ world
  return bitmap;
}

TEST_F(AndroidTransformLoadersTest,
       JAnimaXCloseableBitmapReferenceToNativeBitmap) {
  auto specific_loader = loader_->GetLoader(
      TransformRequestInputTypeAndroid::kJAnimaXCloseableBitmapReference,
      ResourcePayloadType::kBitmap);
  JNIEnv* env = lynx::base::android::AttachCurrentThread();

  // create the java-side bitmap
  auto j_bitmap = createAndTransferBitmap(env);
  jclass simple_closeable_bitmap_class =
      env->FindClass("com/lynx/animax/loader/SimpleCloseableBitmapReference");
  jmethodID simpleCloseableBitmapReferenceConstructor = env->GetMethodID(
      simple_closeable_bitmap_class, "<init>", "(Landroid/graphics/Bitmap;)V");
  jobject closeable_bitmap =
      env->NewObject(simple_closeable_bitmap_class,
                     simpleCloseableBitmapReferenceConstructor, j_bitmap);
  jclass closeable_bitmap_native_adapter_class = env->FindClass(
      "com/lynx/animax/loader/AnimaXCloseableBitmapReferenceNativeAdapter");
  jmethodID closeable_bitmap_native_adapter_create = env->GetStaticMethodID(
      closeable_bitmap_native_adapter_class, "create",
      "(Lcom/lynx/animax/loader/IAnimaXCloseableBitmapReference;)Lcom/lynx/"
      "animax/loader/AnimaXCloseableBitmapReferenceNativeAdapter;");
  jobject closeable_bitmap_native_adapter = env->CallStaticObjectMethod(
      closeable_bitmap_native_adapter_class,
      closeable_bitmap_native_adapter_create, closeable_bitmap);
  auto closeable_bitmap_native_adapter_ref =
      lynx::base::android::ScopedGlobalJavaRef<jobject>{
          env, closeable_bitmap_native_adapter};

  auto promise = std::promise<void>{};
  auto future = promise.get_future();
  auto callback = TransformLoaderBaseAndroid::CallbackType{
      [&promise, closeable_bitmap_native_adapter_ref](ResourceResponse response,
                                                      LoaderError err) {
        ASSERT_EQ(response.payload.type, ResourcePayloadType::kBitmap);
        ASSERT_NE(nullptr, response.payload.bitmap);
        ASSERT_EQ(1, response.payload.bitmap->Height());
        ASSERT_EQ(1, response.payload.bitmap->Width());
        auto* pixels =
            static_cast<const uint8_t*>(response.payload.bitmap->Pixels());
        ASSERT_EQ(pixels[0], 255);
        ASSERT_EQ(pixels[1], 0);
        ASSERT_EQ(pixels[2], 0);
        ASSERT_EQ(pixels[3], 255);

        auto bitmap_ref = AnimaXCloseableBitmapReference{
            lynx::base::android::ScopedLocalJavaRef<jobject>{
                closeable_bitmap_native_adapter_ref}};
        // This bitmap should have been released.
        EXPECT_FALSE(bitmap_ref.IsValid());
        promise.set_value();
      }};

  specific_loader->Load(
      TransformRequestAndroid{
          .input =
              lynx::base::android::ScopedGlobalJavaRef<jobject>{
                  env, closeable_bitmap_native_adapter},
          .input_type = TransformRequestInputTypeAndroid::
              kJAnimaXCloseableBitmapReference,
          .output_type = ResourcePayloadType::kBitmap},
      std::move(callback));
  EXPECT_NE(std::future_status::timeout,
            future.wait_for(std::chrono::seconds(1)));
}

constexpr size_t kLengthByteArray = 10;

// helper
jbyteArray MakeJByteArray() {
  // set up the byteArray
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  jbyteArray j_byte_array = env->NewByteArray(kLengthByteArray);

  jbyte* j_bytes = new jbyte[kLengthByteArray];
  for (int i = 0; i < kLengthByteArray; i++) {
    j_bytes[i] =
        static_cast<jbyte>(i);  // Filling with dummy data for illustration
  }

  env->SetByteArrayRegion(j_byte_array, 0, kLengthByteArray, j_bytes);
  delete[] j_bytes;

  return j_byte_array;
}

TEST_F(AndroidTransformLoadersTest, ValidJByteArrayToNativeRawData) {
  auto j_byte_array = MakeJByteArray();
  auto specific_loader =
      loader_->GetLoader(TransformRequestInputTypeAndroid::kJByteArray,
                         ResourcePayloadType::kRawData);
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  auto promise = std::promise<void>{};
  auto future = promise.get_future();
  auto callback = TransformLoaderBaseAndroid::CallbackType{
      [&promise](ResourceResponse response, LoaderError err) {
        ASSERT_EQ(response.payload.type, ResourcePayloadType::kRawData);
        ASSERT_EQ(err.code, 0);
        promise.set_value();
      }};
  specific_loader->Load(
      TransformRequestAndroid{
          .input =
              lynx::base::android::ScopedGlobalJavaRef<jobject>{env,
                                                                j_byte_array},
          .input_type = TransformRequestInputTypeAndroid::kJByteArray,
          .output_type = ResourcePayloadType::kRawData},
      std::move(callback));
  EXPECT_NE(std::future_status::timeout,
            future.wait_for(std::chrono::seconds(1)));
}

TEST_F(AndroidTransformLoadersTest, InvalidJByteArrayToNativeRawData) {
  auto j_byte_array = nullptr;
  auto specific_loader =
      loader_->GetLoader(TransformRequestInputTypeAndroid::kJByteArray,
                         ResourcePayloadType::kRawData);
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  auto promise = std::promise<void>{};
  auto future = promise.get_future();
  auto callback = TransformLoaderBaseAndroid::CallbackType{
      [&promise](ResourceResponse response, LoaderError err) {
        ASSERT_EQ(err.code, kInvalidRawData);
        promise.set_value();
      }};
  specific_loader->Load(
      TransformRequestAndroid{
          .input =
              lynx::base::android::ScopedGlobalJavaRef<jobject>{env,
                                                                j_byte_array},
          .input_type = TransformRequestInputTypeAndroid::kJByteArray,
          .output_type = ResourcePayloadType::kRawData},
      std::move(callback));
  EXPECT_NE(std::future_status::timeout,
            future.wait_for(std::chrono::seconds(1)));
}

jbyteArray ReadJByteArrayFromFile(std::string file_path) {
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  // Open the file in binary mode
  std::ifstream file(
      file_path, std::ios::binary | std::ios::ate);  // `ate` to move to the end
  if (!file.is_open()) {
    return nullptr;  // Return nullptr on failure
  }

  // Get the size of the file
  std::streamsize fileSize = file.tellg();
  file.seekg(0, std::ios::beg);  // Move to the beginning

  // Read the file into a vector
  std::vector<char> buffer(fileSize);
  if (!file.read(buffer.data(), fileSize)) {
    std::cerr << "Failed to read file: " << file_path << std::endl;
    return nullptr;  // Return nullptr on failure
  }
  file.close();

  // Create a jbyteArray
  jbyteArray byteArray = env->NewByteArray(fileSize);
  if (byteArray == nullptr) {
    std::cerr << "Failed to create jbyteArray" << std::endl;
    return nullptr;  // Return nullptr on failure
  }

  // Copy the file content into the jbyteArray
  env->SetByteArrayRegion(byteArray, 0, fileSize,
                          reinterpret_cast<const jbyte*>(buffer.data()));

  // Return the populated jbyteArray
  return byteArray;
}

bool CompareByteArrays(JNIEnv* env, jbyteArray array1, jbyteArray array2) {
  // Get the lengths of the two arrays
  jsize length1 = env->GetArrayLength(array1);
  jsize length2 = env->GetArrayLength(array2);

  // If lengths are different, arrays are not equal
  if (length1 != length2) {
    return false;
  }

  // Get the elements of the arrays
  jbyte* data1 = env->GetByteArrayElements(array1, nullptr);
  jbyte* data2 = env->GetByteArrayElements(array2, nullptr);

  // Compare the contents of the two arrays
  bool result = std::memcmp(data1, data2, length1) == 0;

  // Release the elements
  env->ReleaseByteArrayElements(array1, data1, JNI_ABORT);
  env->ReleaseByteArrayElements(array2, data2, JNI_ABORT);

  return result;
}

TEST_F(AndroidTransformLoadersTest, ValidJByteArrayToNativeString) {
  auto j_byte_array = MakeJByteArray();
  auto specific_loader =
      loader_->GetLoader(TransformRequestInputTypeAndroid::kJByteArray,
                         ResourcePayloadType::kFilePath);
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  auto promise = std::promise<void>{};
  auto future = promise.get_future();
  auto callback = TransformLoaderBaseAndroid::CallbackType{
      [j_byte_array =
           lynx::base::android::ScopedGlobalJavaRef<jobject>{env, j_byte_array},
       &promise](ResourceResponse response, LoaderError err) {
        JNIEnv* env = lynx::base::android::AttachCurrentThread();
        ASSERT_EQ(response.payload.type, ResourcePayloadType::kFilePath);
        ASSERT_EQ(err.code, 0);
        ASSERT_TRUE(CompareByteArrays(
            env, ReadJByteArrayFromFile(response.payload.path),
            static_cast<jbyteArray>(j_byte_array.Get())));
        promise.set_value();
      }};
  specific_loader->Load(
      TransformRequestAndroid{
          .input =
              lynx::base::android::ScopedGlobalJavaRef<jobject>{env,
                                                                j_byte_array},
          .input_type = TransformRequestInputTypeAndroid::kJByteArray,
          .output_type = ResourcePayloadType::kFilePath},
      std::move(callback));
  EXPECT_NE(std::future_status::timeout,
            future.wait_for(std::chrono::seconds(1)));
}

TEST_F(AndroidTransformLoadersTest, InvalidJByteArrayToNativeString) {
  auto j_byte_array = nullptr;
  auto specific_loader =
      loader_->GetLoader(TransformRequestInputTypeAndroid::kJByteArray,
                         ResourcePayloadType::kFilePath);
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  auto promise = std::promise<void>{};
  auto future = promise.get_future();
  auto callback = TransformLoaderBaseAndroid::CallbackType{
      [&promise](ResourceResponse response, LoaderError err) {
        ASSERT_NE(err.code, 0);
        promise.set_value();
      }};
  specific_loader->Load(
      TransformRequestAndroid{
          .input =
              lynx::base::android::ScopedGlobalJavaRef<jobject>{env,
                                                                j_byte_array},
          .input_type = TransformRequestInputTypeAndroid::kJByteArray,
          .output_type = ResourcePayloadType::kFilePath},
      std::move(callback));
  EXPECT_NE(std::future_status::timeout,
            future.wait_for(std::chrono::seconds(1)));
}

// helper
bool compareMemory(const void* ptr1, const void* ptr2, std::size_t size) {
  // Use memcmp to compare memory regions
  return std::memcmp(ptr1, ptr2, size) == 0;
}

TEST_F(AndroidTransformLoadersTest, ValidJStringToNativeRawData) {
  auto j_byte_array = MakeJByteArray();
  JNIEnv* env = lynx::base::android::AttachCurrentThread();
  // JByteArrayToNativeString
  auto l1 = loader_->GetLoader(TransformRequestInputTypeAndroid::kJByteArray,
                               ResourcePayloadType::kFilePath);
  // NativeStringToJString
  auto l2 = MakeLambdaLoader<TransformResponseAndroid, TransformRequestAndroid>(
      [](auto input, auto callback) {
        JNIEnv* env = lynx::base::android::AttachCurrentThread();
        callback(
            TransformRequestAndroid{
                .input =
                    lynx::base::android::ScopedGlobalJavaRef<jobject>{
                        env, lynx::base::android::JNIConvertHelper::
                                 ConvertToJNIStringUTF(env, input.payload.path)
                                     .Get()},
                .input_type = lynx::animax::TransformRequestInputTypeAndroid::
                    kJStringFilePath,
                .output_type = lynx::animax::ResourcePayloadType::kRawData},
            LoaderError{});
      });
  // JStringToNativeRawData
  auto l3 =
      loader_->GetLoader(TransformRequestInputTypeAndroid::kJStringFilePath,
                         ResourcePayloadType::kRawData);

  auto p = l1 | l2 | l3;
  auto promise = std::promise<void>{};
  auto future = promise.get_future();
  p->Load(
      TransformRequestAndroid{
          .input =
              lynx::base::android::ScopedGlobalJavaRef<jobject>{env,
                                                                j_byte_array},
          .input_type = TransformRequestInputTypeAndroid::kJByteArray,
          .output_type = ResourcePayloadType::kFilePath},
      [j_byte_array =
           lynx::base::android::ScopedGlobalJavaRef<jobject>{env, j_byte_array},
       &promise](auto response, auto err) {
        JNIEnv* env = lynx::base::android::AttachCurrentThread();
        ASSERT_EQ(response.payload.type, ResourcePayloadType::kRawData);
        ASSERT_EQ(err.code, 0);
        ASSERT_EQ(
            compareMemory(response.payload.raw_data->Data(),
                          JavaByteArrayToRawData(
                              env, static_cast<jbyteArray>(j_byte_array.Get()))
                              ->Data(),
                          response.payload.raw_data->Length()),
            true);
        promise.set_value();
      });
  EXPECT_NE(std::future_status::timeout,
            future.wait_for(std::chrono::seconds(1)));
}
