//
// Created by zkw on 20-11-16.
//

#include "mmap_tracer.h"
#include <cerrno>

extern "C" {
#include "log.h"
}

/**
 * log 模式：每次 mmap 或 munmap 都保存一条日志（可以是文件），之后出报表分析曲线，用于观察分配趋势
 */
#define MODE_LOG 1
/**
 * dump 模式：在内存里统一维护，mmap 时增，munmap 时减，在某个时间点保存到文件，用于观察当前进程内存分配状态
 */
#define MODE_DUMP 2

#define INFO_MMAP "mmap"
#define INFO_MUNMMAP "munmap"

using namespace std;

FILE *dumpFile = nullptr;
static const int FLUSH_THRESHOLD = 3 * 1024;

int modeFlag = MODE_LOG;

// 几种处理模式：
// 一、每次 mmap 或 munmap 都保存一条日志（可以是文件），之后出报表分析曲线，用于观察分配趋势
// 二、同上，不过阈值调高，只打印到终端，用于观察大内存分配堆栈
// 三、在内存里统一维护，mmap 时增，munmap 时减，在某个时间点保存到文件，用于观察当前进程内存分配状态

void writeLine(char *line, size_t size) {
    static atomic_size_t wroteSize(0);
    int sizeChar = sizeof(char);
    size_t count = fwrite(line, sizeChar, size / sizeChar, dumpFile);
    wroteSize += count * sizeChar;
    if (wroteSize > FLUSH_THRESHOLD) {
        LOGI("----------wrote: %d", wroteSize.load());
        fflush(dumpFile);
        wroteSize = 0;
    }
}

/**
 *  MODE_LOG，log 模式：每次 mmap 或 munmap 都保存一条日志（可以是文件），之后出报表分析曲线，用于观察分配趋势
 * @param msg
 */
void mmapForModeLog(mmap_info *data) {
    char *content;
    // 样例：mmap[]0xee6891[]104800[]prot[]flag[]fd[]fdLink[]java/lang/String.get|com/zhihu/A.mmm|xxx
    size_t size = asprintf(&content, "%s[]%u[]%d[]%d[]%d[]%d[]%s[]%s\n", INFO_MMAP,
                           data->address, data->length, data->prot, data->flag, data->fd,
                           data->fdLink.c_str(), data->stack.c_str());
    if (size > 0) {
        writeLine(content, size);
        free(content);
    }
}

/**
 *  MODE_LOG，log 模式：每次 mmap 或 munmap 都保存一条日志（可以是文件），之后出报表分析曲线，用于观察分配趋势
 * @param msg
 */
void munmapForModeLog(munmap_info *data) {
    char *content;
    // 样例：munmmap[]0xee6891[]104800
    size_t size = asprintf(&content, "%s[]%u[]%d\n", INFO_MUNMMAP, data->address, data->length);
    if (size > 0) {
        writeLine(content, size);
        free(content);
    }
}

void createFile(char *dumpDir) {
    if (dumpFile == nullptr) {
        struct timeval stamp{};
        gettimeofday(&stamp, nullptr);
        char *filePath;
        asprintf(&filePath, "%s/trace_%d_%ld.himem", dumpDir, modeFlag, stamp.tv_sec);
        dumpFile = fopen(filePath, "ae");
        if (dumpFile == nullptr) {
            LOGE("文件打开错误：%s，错误原因：%s", filePath, strerror(errno));
        }
    }
}

void tracerStart(char *dumpDir) {
    createFile(dumpDir);
}

void postOnMmap(mmap_info *data) {
    mmapForModeLog(data);
}

void postOnMunmap(munmap_info *data) {
    munmapForModeLog(data);
}

void dumpToFile() {
    if (dumpFile != nullptr) {
        fflush(dumpFile);
    }
}

void tracerDestroy() {
    if (dumpFile != nullptr) {
        fflush(dumpFile);
        if (fclose(dumpFile)) {
            LOGI("%s", "文件关闭成功");
        }
        dumpFile = nullptr;
    }
}
