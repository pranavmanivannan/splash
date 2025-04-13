/*
 * Utils to determine which system `splash` is currently running on. This allows
 * for `pool` to enable CPU affinity through thread pinning, a core functionality
 * of this lightweight thread pool implementation.
 */

#pragma once

#include <iostream>
#include <thread>

namespace splash {

#if defined(__APPLE__)
    #include <pthread.h>
    #include <sys/qos.h>
    #define SYSTEM_T 0
#elif defined(__linux__)
    #define SYSTEM_T 1
#elif defined(WIN32)
    #define SYSTEM_T 2
#endif

const uint N_THREADS = std::thread::hardware_concurrency();

/*
 * @param core_id ID of the core thread should be pinned to.
 *
 * Pins thread to cores to take advantage of CPU affinity. MacOS does not support
 * thread pinning, and thus use `set_qos_affinity` for MacOS systems instead.
 */
void pin_thread_to_core(int core_id) {
    if (SYSTEM_T == 0) {
        // TODO
    } else if (SYSTEM_T == 1) {
        // TODO
    } else if (SYSTEM_T == 2) {
        // TODO
    }
    return;
}

/*
 * @param affinity_level Range between 0 and 3, with 3 being highest priority.
 *
 * @return 0 if successful, -1 otherwise.
 * 
 * Sets QoS affinity on MacOS systems. 
 */
int set_qos_affinity(int affinity_level) {
    if (SYSTEM_T == 0) {
        qos_class_t qos;
        switch (affinity_level) {
            case 0:
                qos = QOS_CLASS_BACKGROUND;
                break;
            case 1:
                qos = QOS_CLASS_UTILITY;
                break;
            case 2:
                qos = QOS_CLASS_USER_INITIATED;
                break;
            case 3:
                qos = QOS_CLASS_USER_INTERACTIVE;
                break;
            default:
                qos = QOS_CLASS_DEFAULT;
                break;
        }
        return pthread_set_qos_class_self_np(qos, affinity_level);
    } else {
        return -1;
    }
}
    
} // namespace splash
