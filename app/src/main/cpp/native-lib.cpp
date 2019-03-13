
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <string>
#include <pthread.h>
#define MEM_ZERO(pDest, destSize) memset(pDest, 0, destSize)
#define LOG_TAG "onEvent"
//LOG宏定义
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, fmt, ##args)

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_user_uninstallfeedback_1master_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
void* th_fun(void* arg){
    while(true){
        sleep(1);
        LOGD("while");
    }
};
extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_user_uninstallfeedback_1master_MainActivity_init(JNIEnv *env, jobject instance) {

     //初始化log
    LOGD("init start...");
    //fork子进程，以执行轮询任务
    pid_t pid = fork();
    if (pid < 0) {
        //出错log
        LOGD("fork failed...");
    } else if (pid == 0){
        pthread_t tid;
        pthread_create(&tid, NULL,th_fun,(void*)"NO1");
        //子进程注册"/data/data/pym.test.uninstalledobserver"目录监听器
        int fileDescriptor = inotify_init();
        if (fileDescriptor < 0) {
            LOGD("inotify_init failed...");
            exit(1);
        }
        int watchDescriptor;
        watchDescriptor = inotify_add_watch(fileDescriptor,"/data/data/com.example.user.uninstallfeedback_master", IN_DELETE);
        LOGD("watchDescriptor=%d",watchDescriptor);
        if (watchDescriptor < 0) {
            LOGD("inotify_add_watch failed...");
            exit(1);
        }

        //分配缓存，以便读取event，缓存大小=一个struct inotify_event的大小，这样一次处理一个event
        void *p_buf = malloc(sizeof(struct inotify_event));
        if (p_buf == NULL) {
            LOGD("malloc failed...");
            exit(1);
        }

        //开始监听
        LOGD("start observer...");
        size_t readBytes = read(fileDescriptor, p_buf,sizeof(struct inotify_event));

        //read会阻塞进程，走到这里说明收到目录被删除的事件，注销监听器
        free(p_buf);
        inotify_rm_watch(fileDescriptor, IN_DELETE);

        //目录不存在log
        LOGD("uninstall");

        //执行命令am start -a android.intent.action.VIEW -d http://shouji.360.cn/web/uninstall/uninstall.html
//        execlp(
//                "am", "am", "start", "-a", "android.intent.action.VIEW", "-d",
//                "http://shouji.360.cn/web/uninstall/uninstall.html", (char *)NULL);
        //4.2以上的系统由于用户权限管理更严格，需要加上 --user 0
        execlp("am", "am", "start", "--user", "0", "-a",
        "android.intent.action.VIEW", "-d", "https://www.google.com",(char *) NULL);

    } else {
        //父进程直接退出，使子进程被init进程领养，以避免子进程僵死
    }

    return env->NewStringUTF( "Hello from JNI !");
}