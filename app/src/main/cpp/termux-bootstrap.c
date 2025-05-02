#include <jni.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

extern jbyte blob[];
extern int blob_size;

JNIEXPORT jbyteArray JNICALL Java_com_termux_app_TermuxInstaller_getZip(JNIEnv *env, __attribute__((__unused__)) jobject This)
{
    jbyteArray ret = (*env)->NewByteArray(env, blob_size);
    (*env)->SetByteArrayRegion(env, ret, 0, blob_size, blob);
    return ret;
}

JNIEXPORT jbyteArray JNICALL Java_com_termux_app_TermuxInstaller_getZipChunk(JNIEnv *env, __attribute__((__unused__)) jobject This, jint i) {
    size_t chunk_size = 1024 * 4096;
    jint num_chunks = blob_size / chunk_size + (blob_size % chunk_size != 0);

    if (i < 0 || i >= num_chunks) {
        return (*env)->NewByteArray(env, 0);
    }

    size_t offset = i * chunk_size;
    size_t current_chunk_size = (i == num_chunks - 1) ? (blob_size - offset) : chunk_size;

    jbyteArray chunk = (*env)->NewByteArray(env, current_chunk_size);
    (*env)->SetByteArrayRegion(env, chunk, 0, current_chunk_size, blob + offset);

    return chunk;
}

JNIEXPORT jobject JNICALL Java_com_termux_app_TermuxInstaller_getZipAsMappedBuffer(
        JNIEnv *env,
        __attribute__((__unused__)) jobject This,
        jstring javaFilePath
) {
    const char *filePath = (*env)->GetStringUTFChars(env, javaFilePath, NULL);
    int fd = open(filePath, O_RDONLY);
    if (fd == -1) return NULL;

    void *mapped = mmap(NULL, blob_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    (*env)->ReleaseStringUTFChars(env, javaFilePath, filePath);

    if (mapped == MAP_FAILED) return NULL;

    return (*env)->NewDirectByteBuffer(env, mapped, blob_size);
}
