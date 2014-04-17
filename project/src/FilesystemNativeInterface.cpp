#define IMPLEMENT_API
#include <hx/CFFI.h>


#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <types/NativeData.h>

#include <jni.h>

#include <FileHandle.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

/// ======
/// APK AssetHandling
/// ======

#ifdef __GNUC__
  #define JAVA_EXPORT __attribute__ ((visibility("default"))) JNIEXPORT
#else
  #define JAVA_EXPORT JNIEXPORT
#endif

static AAssetManager *nativeAssetManager = 0;

extern "C" JAVA_EXPORT void JNICALL
Java_org_haxe_hxfilesystem_NativeInterface_setupNativeAssetManager(JNIEnv * env, jclass cls, jobject assetManager) {

    nativeAssetManager = AAssetManager_fromJava(env, assetManager);
}

/// ======
/// FILESYSTEM
/// ======

static value filesystem_android_init() {
	
	return alloc_null();
	
}
DEFINE_PRIM (filesystem_android_init, 0);

static const char* staticFilePrefix = "assets:/";
static const int sizeOfStaticFilePrefix = 8;

static bool isStaticFile(const char* filename)
{
	return strncmp(filename, staticFilePrefix, sizeOfStaticFilePrefix) == 0;
}

#define CHECK_STATIC_FILE_EXIT_FALSE(filename) if(isStaticFile(filename)) return alloc_bool(false);
#define CHECK_STATIC_FILE_EXIT_NULL(filename) if(isStaticFile(filename)) return alloc_null();


static value filesystem_android_create_file(value url) 
{
	const char *c_str = val_string(url);
	CHECK_STATIC_FILE_EXIT_FALSE(c_str);

	struct stat fileStat;
	int result = stat(c_str, &fileStat);
	if(result >= 0)
		return alloc_bool(false); /// file exists

	FILE *f = fopen(c_str, "w");
	if(!f)
	{
		return alloc_bool(false);
	}

	fclose(f);
	
	return alloc_bool(true);
}
DEFINE_PRIM (filesystem_android_create_file, 1);

static value filesystem_android_create_folder(value url) 
{
	const char *c_str = val_string(url);
	CHECK_STATIC_FILE_EXIT_FALSE(c_str);

	int result = mkdir(c_str, 0777);
	return alloc_bool(result == 0);
}
DEFINE_PRIM (filesystem_android_create_folder, 1);

static value filesystem_android_open_file_write(value url) 
{
	const char *c_str = val_string(url);
	CHECK_STATIC_FILE_EXIT_NULL(c_str);

	FILE *file = fopen(c_str, "w");

	if(!file)
	{
		return alloc_null();
	}

	value hxFileHandle = FileHandle::createHaxePointer();
	FileHandle* filehandle = ((FileHandle*)val_data(hxFileHandle));

	if(!filehandle)
	{
		return alloc_null();
	}

	filehandle->fileHandle = file;

	return hxFileHandle;
}
DEFINE_PRIM (filesystem_android_open_file_write, 1);

static value filesystem_android_open_file_read(value url) 
{
	const char *c_str = val_string(url);

	if(isStaticFile(c_str))
	{
        
		const char *withoutStaticFilePrefix = c_str + sizeOfStaticFilePrefix;

		///will crash if this is not done
		while(withoutStaticFilePrefix[0] == '/')
			withoutStaticFilePrefix++;

		AAsset* asset = AAssetManager_open(nativeAssetManager, withoutStaticFilePrefix, AASSET_MODE_RANDOM);

		if(!asset)
		{

			return alloc_null();
		}


		value hxFileHandle = FileHandle::createHaxePointer();
		FileHandle* filehandle = ((FileHandle*)val_data(hxFileHandle));

		if(!filehandle)
		{
			return alloc_null();
		}

		filehandle->staticFile = asset;

		return hxFileHandle;
	}
	else
	{


		FILE *file = fopen(c_str, "r");

		if(!file)
		{
			return alloc_null();
		}

		value hxFileHandle = FileHandle::createHaxePointer();
		FileHandle* filehandle = ((FileHandle*)val_data(hxFileHandle));

		if(!filehandle)
		{
			return alloc_null();
		}

		filehandle->fileHandle = file;

		return hxFileHandle;

	}
}
DEFINE_PRIM (filesystem_android_open_file_read, 1);

static value filesystem_android_delete_file(value str) 
{
	const char *c_str = val_string(str);
	CHECK_STATIC_FILE_EXIT_FALSE(c_str);

    int ret_code = remove(c_str);
	return alloc_bool(ret_code == 0);
}
DEFINE_PRIM (filesystem_android_delete_file, 1);


/// Done in Java
///static value filesystem_android_delete_folder(value str)

static value filesystem_android_url_exists(value str) 
{
	struct stat fileStat;
	const char *c_str = val_string(str);

	if(isStaticFile(c_str))
	{
		const char *withoutStaticFilePrefix = c_str + sizeOfStaticFilePrefix;
		AAsset* asset = AAssetManager_open(nativeAssetManager, withoutStaticFilePrefix, AASSET_MODE_RANDOM);

		if(asset)
		{
			AAsset_close(asset);
			return alloc_bool(true);
		}

		AAssetDir* assetDir = AAssetManager_openDir(nativeAssetManager, withoutStaticFilePrefix);

		if(assetDir)
		{
			AAssetDir_close(assetDir);
			return alloc_bool(true);
		}

		return alloc_bool(false);
	}
	else
	{
		int result = stat(c_str, &fileStat);
		return alloc_bool(result >= 0);
	}
}
DEFINE_PRIM (filesystem_android_url_exists, 1);

static value filesystem_android_is_folder(value str) 
{
	struct stat fileStat;
	const char *c_str = val_string(str);

	if(isStaticFile(c_str))
	{
		const char *withoutStaticFilePrefix = c_str + sizeOfStaticFilePrefix;
		AAssetDir* assetDir = AAssetManager_openDir(nativeAssetManager, withoutStaticFilePrefix);

		if(assetDir)
		{
			AAssetDir_close(assetDir);
			return alloc_bool(true);
		}

		return alloc_bool(false);
	}
	else
	{
		if(stat(c_str, &fileStat) < 0)
			return alloc_bool(false);

  	 	return alloc_bool(S_ISDIR(fileStat.st_mode));
  	}
}
DEFINE_PRIM (filesystem_android_is_folder, 1);

static value filesystem_android_is_file(value str) 
{
	struct stat fileStat;
	const char *c_str = val_string(str);

	if(isStaticFile(c_str))
	{
		const char *withoutStaticFilePrefix = c_str + sizeOfStaticFilePrefix;
		AAsset* asset = AAssetManager_open(nativeAssetManager, withoutStaticFilePrefix, AASSET_MODE_RANDOM);

		if(asset)
		{
			AAsset_close(asset);
			return alloc_bool(true);
		}

		return alloc_bool(false);
	}
	else
	{
		if(stat(c_str, &fileStat) < 0)
			return alloc_bool(false);

   		return alloc_bool(S_ISREG(fileStat.st_mode));
   	}
}
DEFINE_PRIM (filesystem_android_is_file, 1);

/// ======
/// FILEHANDLE
/// ======

static value filesystem_android_get_seek(value hxFileHandle) 
{
	FileHandle* fileHandle = ((FileHandle*)val_data(hxFileHandle));

	if(fileHandle->staticFile)
	{
		return alloc_int(AAsset_seek(fileHandle->staticFile, 0, SEEK_CUR));
	}
	else
	{
		return alloc_int(ftell(fileHandle->fileHandle));
	}
}
DEFINE_PRIM (filesystem_android_get_seek, 1);

static value filesystem_android_set_seek(value hxFileHandle, value seek) 
{
	FileHandle* fileHandle = ((FileHandle*)val_data(hxFileHandle));

	if(fileHandle->staticFile)
	{
		return alloc_int(AAsset_seek(fileHandle->staticFile, val_int(seek), SEEK_SET));
	}
	else
	{
		return alloc_int(fseek(fileHandle->fileHandle, val_int(seek), SEEK_SET));
	}
}
DEFINE_PRIM (filesystem_android_set_seek, 2);

static value filesystem_android_seek_end_of_file(value hxFileHandle) 
{
	FileHandle* fileHandle = ((FileHandle*)val_data(hxFileHandle));

	if(fileHandle->staticFile)
	{
		AAsset_seek(fileHandle->staticFile, 0, SEEK_END);
	}
	else
	{
		fseek(fileHandle->fileHandle, 0, SEEK_END);
	}

}
DEFINE_PRIM (filesystem_android_seek_end_of_file, 1);

static value filesystem_android_file_write(value hxFileHandle, value nativeData) 
{
	FileHandle* fileHandle = ((FileHandle*)val_data(hxFileHandle));
	NativeData* ptr = ((NativeData*)val_data(nativeData));

	fwrite(ptr->ptr + ptr->offset, 1, ptr->offsetLength, fileHandle->fileHandle); 
    fflush(fileHandle->fileHandle);

	return alloc_null();
}
DEFINE_PRIM (filesystem_android_file_write, 2);

static value filesystem_android_file_read(value hxFileHandle, value nativeData) 
{
	FileHandle* fileHandle = ((FileHandle*)val_data(hxFileHandle));
	NativeData* ptr = ((NativeData*)val_data(nativeData));

	if(fileHandle->staticFile)
	{
		AAsset_read(fileHandle->staticFile, ptr->ptr + ptr->offset, ptr->offsetLength);
	}
	else
	{
		fread(ptr->ptr + ptr->offset, 1, ptr->offsetLength, fileHandle->fileHandle); 
	}
	return alloc_null();
}
DEFINE_PRIM (filesystem_android_file_read, 2);

static value filesystem_android_file_close(value hxFileHandle) 
{
	FileHandle* fileHandle = ((FileHandle*)val_data(hxFileHandle));

	fileHandle->close();

	return alloc_null();
}
DEFINE_PRIM (filesystem_android_file_close, 1);



/// ======
/// OTHER
/// ======
extern "C" void filesystem_android_main () {
	
	val_int(0); // Fix Neko init
	
}
DEFINE_ENTRY_POINT (filesystem_android_main);

extern "C" int filesystem_android_register_prims () { return 0; }
