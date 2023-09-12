//
// Created by Radzivon Bartoshyk on 04/09/2023.
//

#include <jni.h>
#include <vector>
#include "JxlDecoding.h"
#include "jniExceptions.h"
#include "colorspace.h"
#include "HalfFloats.h"
#include "stb_image_resize.h"
#include <libyuv.h>
#include "android/bitmap.h"
#include "Rgba16bitCopy.h"
#include "F32ToRGB1010102.h"

int androidOSVersion() {
    return android_get_device_api_level();
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_awxkee_jxlcoder_JxlCoder_decodeSampledImpl(JNIEnv *env, jobject thiz,
                                                    jbyteArray byte_array, jint scaledWidth,
                                                    jint scaledHeight, jboolean preferF16HDR) {
    auto totalLength = env->GetArrayLength(byte_array);
    std::shared_ptr<void> srcBuffer(static_cast<char *>(malloc(totalLength)),
                                    [](void *b) { free(b); });
    env->GetByteArrayRegion(byte_array, 0, totalLength, reinterpret_cast<jbyte *>(srcBuffer.get()));

    std::vector<uint8_t> rgbaPixels;
    std::vector<uint8_t> iccProfile;
    size_t xsize = 0, ysize = 0;
    bool useBitmapFloats = false;
    bool alphaPremultiplied = false;
    int osVersion = androidOSVersion();
    if (!DecodeJpegXlOneShot(reinterpret_cast<uint8_t *>(srcBuffer.get()), totalLength, &rgbaPixels,
                             &xsize, &ysize,
                             &iccProfile, &useBitmapFloats, &alphaPremultiplied,
                             osVersion >= 26)) {
        throwInvalidJXLException(env);
        return nullptr;
    }

    if (!iccProfile.empty()) {
        int stride = (int) xsize * 4 *
                     (int) (useBitmapFloats ? sizeof(uint32_t)
                                            : sizeof(uint8_t));
        convertUseDefinedColorSpace(rgbaPixels,
                                    stride,
                                    (int) xsize,
                                    (int) ysize, iccProfile.data(),
                                    iccProfile.size(),
                                    useBitmapFloats);
    }

    bool useRGBA1010102 = false;

//    if (osVersion >= 33 && !preferF16HDR && useBitmapFloats) {
//        int dstStride;
//        coder::F16ToRGBA1010102(rgbaPixels,
//                                (int) xsize * 4 * (int) sizeof(float),
//                                &dstStride,
//                                (int) xsize, (int) ysize);
//        useBitmapFloats = false;
//        useRGBA1010102 = true;
//    }

    bool useSampler = scaledWidth > 0 && scaledHeight > 0;

    int finalWidth = useSampler ? (int) scaledWidth : (int) xsize;
    int finalHeight = useSampler ? (int) scaledHeight : (int) ysize;

    auto bitmapConfigStr = useRGBA1010102 ? "RGBA_1010102"
                                          : (useBitmapFloats ? "RGBA_F16"
                                                             : "ARGB_8888");

    jclass bitmapConfig = env->FindClass("android/graphics/Bitmap$Config");
    jfieldID rgba8888FieldID = env->GetStaticFieldID(bitmapConfig,
                                                     bitmapConfigStr,
                                                     "Landroid/graphics/Bitmap$Config;");
    jobject rgba8888Obj = env->GetStaticObjectField(bitmapConfig, rgba8888FieldID);

    jclass bitmapClass = env->FindClass("android/graphics/Bitmap");
    jmethodID createBitmapMethodID = env->GetStaticMethodID(bitmapClass, "createBitmap",
                                                            "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    jobject bitmapObj = env->CallStaticObjectMethod(bitmapClass, createBitmapMethodID,
                                                    static_cast<jint>(finalWidth),
                                                    static_cast<jint>(finalHeight),
                                                    rgba8888Obj);
    if (useSampler) {
        std::vector<uint8_t> newImageData(finalWidth * finalHeight * 4 *
                                          (useBitmapFloats ? sizeof(uint32_t) : sizeof(uint8_t)));

        if (useBitmapFloats) {
            // We we'll use Mitchell because we want less artifacts in our HDR image
            int result = stbir_resize_float_generic(
                    reinterpret_cast<const float *>(rgbaPixels.data()), (int) xsize,
                    (int) ysize, 0,
                    reinterpret_cast<float *>(newImageData.data()), scaledWidth,
                    scaledHeight, 0,
                    4, 3, alphaPremultiplied ? STBIR_FLAG_ALPHA_PREMULTIPLIED : 0,
                    STBIR_EDGE_CLAMP, STBIR_FILTER_MITCHELL, STBIR_COLORSPACE_SRGB,
                    nullptr
            );
            if (result != 1) {
                std::string s("Failed to resample an image");
                throwException(env, s);
                return static_cast<jobject>(nullptr);
            }
        } else {
            libyuv::ARGBScale(rgbaPixels.data(), static_cast<int>(xsize * 4),
                              static_cast<int>(xsize),
                              static_cast<int>(ysize),
                              newImageData.data(), scaledWidth * 4, scaledWidth, scaledHeight,
                              libyuv::kFilterBilinear);
        }

        rgbaPixels.clear();
        rgbaPixels = newImageData;
    }

    if (useBitmapFloats) {
        std::vector<uint8_t> newImageData(finalWidth * finalHeight * 4 * sizeof(uint16_t));
        auto startPixels = reinterpret_cast<float *>(rgbaPixels.data());
        auto dstPixels = reinterpret_cast<uint16_t *>(newImageData.data());
        coder::RgbaF32ToF16(startPixels, (int) finalWidth * 4 * (int) sizeof(uint32_t), dstPixels,
                            (int) finalWidth * 4 * (int) sizeof(uint16_t), finalWidth, finalHeight);
        rgbaPixels.clear();
        rgbaPixels = newImageData;
    }

    AndroidBitmapInfo info;
    if (AndroidBitmap_getInfo(env, bitmapObj, &info) < 0) {
        throwPixelsException(env);
        return static_cast<jbyteArray>(nullptr);
    }

    void *addr;
    if (AndroidBitmap_lockPixels(env, bitmapObj, &addr) != 0) {
        throwPixelsException(env);
        return static_cast<jobject>(nullptr);
    }

    if (useBitmapFloats) {
        coder::CopyRGBA16(reinterpret_cast<uint16_t *>(rgbaPixels.data()),
                          finalWidth * 4 * (int) sizeof(uint16_t),
                          reinterpret_cast<uint16_t *>(addr), (int) info.stride, (int) info.width,
                          (int) info.height);
    } else {
        libyuv::ARGBCopy(reinterpret_cast<uint8_t *>(rgbaPixels.data()),
                         (int) finalWidth * 4 * (int) sizeof(uint8_t),
                         reinterpret_cast<uint8_t *>(addr), (int) info.stride, (int) info.width,
                         (int) info.height);
    }

    if (AndroidBitmap_unlockPixels(env, bitmapObj) != 0) {
        throwPixelsException(env);
        return static_cast<jobject>(nullptr);
    }

    rgbaPixels.clear();

    return bitmapObj;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_awxkee_jxlcoder_JxlCoder_getSizeImpl(JNIEnv *env, jobject thiz, jbyteArray byte_array) {
    auto totalLength = env->GetArrayLength(byte_array);
    std::shared_ptr<void> srcBuffer(static_cast<char *>(malloc(totalLength)),
                                    [](void *b) { free(b); });
    env->GetByteArrayRegion(byte_array, 0, totalLength, reinterpret_cast<jbyte *>(srcBuffer.get()));

    std::vector<uint8_t> rgbaPixels;
    std::vector<uint8_t> icc_profile;
    size_t xsize = 0, ysize = 0;
    if (!DecodeBasicInfo(reinterpret_cast<uint8_t *>(srcBuffer.get()), totalLength, &rgbaPixels,
                         &xsize, &ysize)) {
        return nullptr;
    }

    jclass sizeClass = env->FindClass("android/util/Size");
    jmethodID methodID = env->GetMethodID(sizeClass, "<init>", "(II)V");
    auto sizeObject = env->NewObject(sizeClass, methodID, static_cast<jint >(xsize),
                                     static_cast<jint>(ysize));
    return sizeObject;
}