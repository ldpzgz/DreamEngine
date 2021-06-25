#ifndef __LOG_H__
#define __LOG_H__

#define LOG_ALL 0
#define LOG_VERBOSE 1
#define LOG_DEBUG 2
#define LOG_INFO 3
#define LOG_WARN 4
#define LOG_ERROR 5
#define LOG_FATAL 6
#define LOG_CLOSE 7

#define KPLAYERLOGLEVEL LOG_DEBUG


#ifdef _WIN32
void OutputDebugPrintf(const char* strOutputString, ...);
#endif


#ifdef ANDROID

// android
#include <android/log.h>

#ifndef LOG_TAG
#define LOG_TAG "JNI"
#endif

#define KUGOU_LOG_TAG	"KugouPlayer/"

#if KPLAYERLOGLEVEL <= LOG_VERBOSE
#define LOGV( ... ) __android_log_print( ANDROID_LOG_VERBOSE, KUGOU_LOG_TAG LOG_TAG, __VA_ARGS__ )
#else
#define LOGV( ... )
#endif

#if KPLAYERLOGLEVEL <= LOG_DEBUG
#define LOGD( ... ) __android_log_print( ANDROID_LOG_DEBUG, KUGOU_LOG_TAG LOG_TAG, __VA_ARGS__ )
#else
#define LOGD( ... )
#endif

#if KPLAYERLOGLEVEL <= LOG_INFO
#define LOGI( ... ) __android_log_print( ANDROID_LOG_INFO, KUGOU_LOG_TAG LOG_TAG, __VA_ARGS__ )
#else
#define LOGI( ... )
#endif

#if KPLAYERLOGLEVEL <= LOG_WARN
#define LOGW( ... ) __android_log_print( ANDROID_LOG_WARN, KUGOU_LOG_TAG LOG_TAG, __VA_ARGS__ )
#else
#define LOGW( ... )
#endif

#if KPLAYERLOGLEVEL <= LOG_ERROR
#define LOGE( ... ) __android_log_print( ANDROID_LOG_ERROR, KUGOU_LOG_TAG LOG_TAG, __VA_ARGS__ )
#else
#define LOGE( ... )
#endif

#if KPLAYERLOGLEVEL <= LOG_FATAL
#define LOGF( ... ) __android_log_print( ANDROID_LOG_FATAL, KUGOU_LOG_TAG LOG_TAG, __VA_ARGS__ )
#else
#define LOGF( ... )
#endif

// end of android
#elif defined _WIN32
	#include <stdio.h>

		// other system
	#if KPLAYERLOGLEVEL <= LOG_VERBOSE
	#define LOGV( ... ) OutputDebugPrintf( __VA_ARGS__ )
	#else
	#define LOGV( ... )
	#endif

	#if KPLAYERLOGLEVEL <= LOG_DEBUG
	#define LOGD( ... ) OutputDebugPrintf( __VA_ARGS__ )
	#else
	#define LOGD( ... )
	#endif

	#if KPLAYERLOGLEVEL <= LOG_INFO
	#define LOGI( ... ) OutputDebugPrintf( __VA_ARGS__ )
	#else
	#define LOGI( ... )
	#endif

	#if KPLAYERLOGLEVEL <= LOG_WARN
	#define LOGW( ... ) OutputDebugPrintf( __VA_ARGS__ )
	#else
	#define LOGW( ... )
	#endif

	#if KPLAYERLOGLEVEL <= LOG_ERROR
	#define LOGE( ... ) OutputDebugPrintf( __VA_ARGS__ )
	#else
	#define LOGE( ... )
	#endif

	#if KPLAYERLOGLEVEL <= LOG_FATAL
	#define LOGF( ... ) OutputDebugPrintf( __VA_ARGS__ )
	#else
	#define LOGF( ... )
	#endif
#else

#include <stdio.h>

// other system
#if KPLAYERLOGLEVEL <= LOG_VERBOSE
#define LOGV( ... ) printf( __VA_ARGS__ )
#else
#define LOGV( ... )
#endif

#if KPLAYERLOGLEVEL <= LOG_DEBUG
#define LOGD( ... ) printf( __VA_ARGS__ )
#else
#define LOGD( ... )
#endif

#if KPLAYERLOGLEVEL <= LOG_INFO
#define LOGI( ... ) printf( __VA_ARGS__ )
#else
#define LOGI( ... )
#endif

#if KPLAYERLOGLEVEL <= LOG_WARN
#define LOGW( ... ) printf( __VA_ARGS__ )
#else
#define LOGW( ... )
#endif

#if KPLAYERLOGLEVEL <= LOG_ERROR
#define LOGE( ... ) printf( __VA_ARGS__ )
#else
#define LOGE( ... )
#endif

#if KPLAYERLOGLEVEL <= LOG_FATAL
#define LOGF( ... ) printf( __VA_ARGS__ )
#else
#define LOGF( ... )
#endif

// end of other system
#endif


#endif
