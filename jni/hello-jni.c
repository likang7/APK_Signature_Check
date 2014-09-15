#include <string.h>
#include <jni.h>
#include <stdio.h>
#include<android/log.h>

#define LOG_TAG "debug log"
#define LOGI(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)
#define LOGE(fmt, args...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt, ##args)

void native_jniCheckAPP(JNIEnv* env, jobject thiz);

#define TEST_HASH_CODE 1

static JNINativeMethod methods[] = {
	{
		"jniCheckAPP",
		"()V",
		(void*) native_jniCheckAPP
	}
};

static const char *classPathName = "com/example/hellojni/HelloJni";

//合法的APP包名
const char *global_app_packageName = "com.example.hellojni";

//合法的hashcode
const int global_app_signature_hash_code = -401017341;

//10进制的公钥， 这个可以换成校验hashcode或存放在服务器上
const char *global_app_signature_public_key = "\
163002450248501877979451923114076470408521299184290880555725676206478812\
396297015382361623011458590271101795741213543248834070559689844392199662\
184469837911327429301316378176688177366475493400109592764129698892321155\
738593453757347549473977575874259887752652200140584344546424137153728092\
165153023797675065355220808340568930556899891109291645486152987780567730\
249593220917233124089182239906007576408526866224016070992178601327488769\
843680405801044384371583240149107393154575476449466199403189470887667647\
811954972307395221116343051066800181634891617507012629903754919717509460\
04935400868449061562287317243123121722377";

// 16进制的
// 811f70a9799a03bfedf411b6ecfa99a347e2f12b9b5ab38a0a12a799f28d398bd\
// b2dbbb2fc492397b82a0a8152e6035ed726756a8d985b618022fe3db2fd876636\
// 40f37d0ed0e79dfbbc1a5ffdd470f17fbdfa23e4c7db04863c2a98c4262fc0a50\
// d5d7431b00422d881736d6306c13513f3f87b043f07fcccb505caa49ec35bc477\
// 1931e2add28813408a83817b3cb4058a423577f5126b780b23caafa980bdb74a9\
// 4141480666830c08bc2d462d8ca74bddb073b4c86d6883b7e8e024281e403114e\
// 4b54856ef0ce4ca75aad56e5c1777bc414b92bae222440db775305f6e69db8408\
// ea5486f615bebba3ca9fd6ee7ca9ac2f531f293f49047a57f36771409";

// 常用调用的封装
jvalue JNU_CallMethodByName(JNIEnv *env, jobject obj,
        const char *name, const char *descriptor, ...) {
    va_list args;
    jclass clazz;
    jmethodID mid;
    jvalue result;
    if ((*env)->EnsureLocalCapacity(env, 2) == JNI_OK) {
        clazz = (*env)->GetObjectClass(env, obj);
        mid = (*env)->GetMethodID(env, clazz, name, descriptor);
        if (mid) {
            const char *p = descriptor;
            /* skip over argument types to find out the
              return type */
            while (*p != ')')
                p++;
            /* skip ')' */
            p++;
            va_start(args, descriptor);
            switch (*p) {
            case 'V':
                (*env)->CallVoidMethodV(env, obj, mid, args);
                break;
            case '[':
            case 'L':
                result.l = (*env)->CallObjectMethodV(env, obj, mid, args);
                break;
            case 'Z':
                result.z = (*env)->CallBooleanMethodV(env, obj, mid, args);
                break;
            case 'B':
                result.b = (*env)->CallByteMethodV(env, obj, mid, args);
                break;
            case 'C':
                result.c = (*env)->CallCharMethodV(env, obj, mid, args);
                break;
            case 'S':
                result.s = (*env)->CallShortMethodV(env, obj, mid, args);
                break;
            case 'I':
                result.i = (*env)->CallIntMethodV(env, obj, mid, args);
                break;
            case 'J':
                result.j = (*env)->CallLongMethodV(env, obj, mid, args);
                break;
            case 'F':
                result.f = (*env)->CallFloatMethodV(env, obj, mid, args);
                break;
            case 'D':
                result.d = (*env)->CallDoubleMethodV(env, obj, mid, args);
                break;
            default:
                (*env)->FatalError(env, "illegaldescriptor");
            }
            va_end(args);
        }
        (*env)->DeleteLocalRef(env, clazz);
    }
    return result;
}

jobject getSignature(JNIEnv* env, jobject thiz){
	//获取包名
    jstring jstr_packageName = (jstring) JNU_CallMethodByName(env,
            thiz, "getPackageName", "()Ljava/lang/String;").l;

   	if ((*env)->ExceptionCheck(env) || jstr_packageName == NULL) {
    	LOGI("can't get jstr of getPackageName");
        return NULL;
   	}

   	//获取包名的字符串
   	const char* loc_str_app_packageName = (*env)->GetStringUTFChars(env,
           jstr_packageName, NULL);
   	if (loc_str_app_packageName == NULL) {
       	LOGI("can't get packagename from jstring");
       	return NULL;
   	}
   	//当前应用包名与合法包名对比
   	if (strcmp(loc_str_app_packageName, global_app_packageName) != 0) {
       	LOGI("this app is illegal");
       	return NULL;
   	}

   	//释放loc_str_app_packageName
   	(*env)->ReleaseStringUTFChars(env, jstr_packageName,
           loc_str_app_packageName);

   	// 获得应用包的管理器
   	jobject package_manager = JNU_CallMethodByName(env, thiz,
           "getPackageManager", "()Landroid/content/pm/PackageManager;").l;
   	if ((*env)->ExceptionCheck(env) || package_manager == NULL) {
       	LOGI("can't get obj of getPackageManager");
       	return NULL;
   	}

   	// 获得应用包的信息
   	jobject package_info = JNU_CallMethodByName(env,
           	package_manager, "getPackageInfo",
           	"(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;",
           	(*env)->NewStringUTF(env, global_app_packageName), 64).l;
   	if ((*env)->ExceptionCheck(env) || package_info == NULL) {
       	(*env)->ExceptionClear(env);
       	LOGI("can't get obj of package_info");
       	return NULL;
   	}

   	// 获得 PackageInfo 类
   	jclass pi_clazz = (*env)->GetObjectClass(env, package_info);

   	// 获得签名数组属性的 ID
   	jfieldID fieldID_signatures = (*env)->GetFieldID(env, pi_clazz,
           "signatures", "[Landroid/content/pm/Signature;");
   	(*env)->DeleteLocalRef(env, pi_clazz);
   	// 得到签名数组，待修改
   	jobjectArray signatures = (*env)->GetObjectField(env, package_info,
           fieldID_signatures);

   	if ((*env)->ExceptionCheck(env) || signatures == NULL) {
       LOGI("can't get jobjectArray of signatures");
       return NULL;
   	}

   	// 得到签名
   	jobject signature = (*env)->GetObjectArrayElement(env, signatures, 0);
   	if ((*env)->ExceptionCheck(env) || signature == NULL) {
       	LOGI("can't get obj of signature");
       	return NULL;
   	}

   return signature;
}

jobject getPublicKey(JNIEnv* env, jobject signature){
	// 获得 Signature 类
	jclass Signature = (*env)->GetObjectClass(env, signature);

	//实例化证书
	//相当于: CertificateFactory.getInstance("X.509")
	jclass CertificateFactory = (*env)->FindClass(env, "java/security/cert/CertificateFactory");
	jmethodID certId = (*env)->GetStaticMethodID(env, CertificateFactory,
			"getInstance", "(Ljava/lang/String;)Ljava/security/cert/CertificateFactory;");
	jobject cf = (*env)->CallStaticObjectMethod(env, CertificateFactory,
			certId, (*env)->NewStringUTF(env, "X.509"));
	if ((*env)->ExceptionCheck(env) || cf == NULL) {
       LOGI("can't get obj of certificate");
       return NULL;
   	}
	//签名的byteArray
	//相当于：signs[0].toByteArray()
	jmethodID getSignByteArrayID = (*env)->GetMethodID(env, Signature, "toByteArray", "()[B");
	jbyteArray signByteArray = (*env)->CallObjectMethod(env, signature, getSignByteArrayID);
	if ((*env)->ExceptionCheck(env) || signByteArray == NULL) {
       LOGI("can't get jbyteArray of signByteArray");
       return NULL;
   	}

	//new ByteArrayInputStream(signs[0].toByteArray())
	jclass ByteArrayInputStream = (*env)->FindClass(env, "java/io/ByteArrayInputStream");
 	jmethodID BAIS_INIT_ID = (*env)->GetMethodID(env, ByteArrayInputStream, "<init>", "([B)V");  
    jobject bais = (*env)->NewObject(env, ByteArrayInputStream, BAIS_INIT_ID, signByteArray);  
    if ((*env)->ExceptionCheck(env) || bais == NULL) {
       LOGI("can't get obj of bais");
       return NULL;
   	}

    // X509Certificate cert = (X509Certificate)cf.generateCertificate(bais)
	jobject cert = JNU_CallMethodByName(env, cf, "generateCertificate", "(Ljava/io/InputStream;)Ljava/security/cert/Certificate;", bais).l;
	if ((*env)->ExceptionCheck(env) || cert == NULL) {
       LOGI("can't get obj of cert");
       return NULL;
   	}

	//PublicKey key = cert.getPublicKey();
	jobject publicKey = JNU_CallMethodByName(env, cert, "getPublicKey", "()Ljava/security/PublicKey;").l;
	if ((*env)->ExceptionCheck(env) || publicKey == NULL) {
       LOGI("can't get obj of publicKey");
       return NULL;
   	}

	return publicKey;
}

int do_check_signature(JNIEnv* env, jobject thiz){
	int equal = -1;
    jobject signature = getSignature(env, thiz);
    if(signature == NULL){
    	return equal;
    }
	//PublicKey key 
	jobject publicKey = getPublicKey(env, signature);
	if(publicKey == NULL){
		return equal;
	}

	//BigInteger modulus = ((RSAPublicKey)key).getModulus().hashCode();
	jobject modulus = JNU_CallMethodByName(env, publicKey, "getModulus", "()Ljava/math/BigInteger;").l;
	//String strModulus = modulus.toString(10)
	jstring strKey = (jstring)JNU_CallMethodByName(env, modulus, "toString", "(I)Ljava/lang/String;", 10).l;

#ifndef TEST_HASH_CODE
	const char *nativeKeyString = (*env)->GetStringUTFChars(env, strKey, 0);
	LOGI("this app publicKey of signature is %s", nativeKeyString);
    equal = 0 == strncmp(nativeKeyString, global_app_signature_public_key, 1000);
   	(*env)->ReleaseStringUTFChars(env, strKey, nativeKeyString);
#else
   	int hash_code = (int)JNU_CallMethodByName(env, modulus, "hashCode", "()I").i;
   	LOGI("this app hash_code of signature is %d", hash_code);
   	equal = hash_code == global_app_signature_hash_code;
#endif
	
   	//合法返回1
    return equal == 1;
}

JNIEXPORT void native_jniCheckAPP(JNIEnv* env, jobject thiz) {
    LOGI("start jniCheckAPP");
    if(do_check_signature(env, thiz) != 1){
    	JNU_CallMethodByName(env, thiz, "popAlarm", "()V");
    }
}

typedef union {
    JNIEnv* env;
    void* venv;
} UnionJNIEnvToVoid;

static int registerNativeMethods(JNIEnv* env, const char* className,
        JNINativeMethod* gMethods, int numMethods) {
    jclass clazz;
    clazz = (*env)->FindClass(env, className);
    if (clazz == NULL) {
        return JNI_FALSE;
    }
    if ((*env)->RegisterNatives(env, clazz, gMethods, numMethods) < 0) {
        LOGE("RegisterNatives failed");
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

static int registerNatives(JNIEnv* env) {
    if (!registerNativeMethods(env, classPathName, methods,
            sizeof(methods) / sizeof(methods[0]))) {
        return JNI_FALSE;
    }
    return JNI_TRUE;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;
    jint result = -1;
    JNIEnv* env = NULL;
    if ((*vm)->GetEnv(vm, &uenv.venv, JNI_VERSION_1_4) != JNI_OK) {
        goto bail;
    }
    env = uenv.env;
    if (registerNatives(env) != JNI_TRUE) {

        goto bail;
    }
    result = JNI_VERSION_1_4;
    bail: LOGI("JNI_ONload result '%d' ", result);
    return result;
}