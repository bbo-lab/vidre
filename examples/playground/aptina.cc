#include <vidre/log.hh>
#include <midlib2.h>
#include <apbase.h>

#include <unistd.h>

using namespace vidre;

// === Logging ================================================================

//   MI_NO_ERROR_LOG    Error logging is turned off
//   MI_ERROR_SEVERE    Log Severe errors
//   MI ERROR_MINOR     Log Minor errors
//   MI_ALL_ERRORS      Logs all error messages (Severe and Minor)
//   MI_LOG             Logs General logging message
//   MI_LOG_SHIP        Log Serial I/O messages (SHIP)
//   MI_LOG_USB         Log USB transactions
//   MI_LOG_DEBUG       Log Debug messages

static mi_s32 mi_log_error(mi_u32, mi_s32 errorCode, const char *logMsg, const char *, const char *, mi_u32) {
    log::error() << std::format("({:#x}) ", errorCode) << logMsg << log::endl;
    return 0;
}

static mi_s32 mi_log_info(mi_u32, mi_s32 errorCode, const char *logMsg, const char *, const char *, mi_u32) {
    log::info() << std::format("({:#x}) ", errorCode) << logMsg << log::endl;
    return 0;
}

static mi_s32 mi_log_debug(mi_u32, mi_s32 errorCode, const char *logMsg, const char *szSource, const char *szFunc, mi_u32 nLine) {
    log::debug() << "errorCode: " << std::format("{:#x}", errorCode) << "\n"  //
                 << logMsg << "\n"                                            //
                 << "in " << szSource << ", " << szFunc << "(" << nLine << ")\n";

    return 0;
}

int main(int argc, char **argv) {
    mi_OpenErrorLogCallback(MI_ALL_ERRORS, mi_log_error);
    mi_OpenErrorLogCallback(MI_ALL_LOGS - MI_LOG_DEBUG, mi_log_info);
    mi_OpenErrorLogCallback(MI_LOG_DEBUG, mi_log_debug);
    mi_SetErrorLogLevel(MI_ERROR_LOG_ALL);

    log::notice() << "Home: " << mi_Home() << log::endl;
    log::notice() << "Sensor data: " << mi_SensorData() << log::endl;

    log::notice() << "Querying cameras:" << log::endl;

    mi_camera_t *pCameras[MI_MAX_CAMERAS] = {};
    mi_s32 nNumCameras;

    auto retval = mi_OpenCameras(pCameras, &nNumCameras, "/opt/Aptina_Imaging/sensor_data");
    log::info() << "returned " << retval << log::endl;

    for (mi_s32 i = 0; i < nNumCameras; ++i) {
        mi_camera_t &cam = *pCameras[i];

        log::debug() << "mi_startTransport " << mi_startTransport(&cam) << log::endl;

        log::debug() << "mi_DetectPartNumber " << mi_DetectPartNumber(&cam) << log::endl;
        log::debug() << "mi_GetDriverInfoString " << mi_GetDriverInfoString(&cam) << log::endl;
        log::debug() << "mi_GetSerdesPartNumber " << mi_GetSerdesPartNumber(&cam) << log::endl;

        log::debug() << "=== camera " << i << log::endl;
        log::debug() << std::format("productID: {:#x}", (uint32_t)cam.productID) << log::endl             //
                     << "productName: " << cam.productName << log::endl                                   //
                     << std::format("productVersion: {:x}", (uint32_t)cam.productVersion) << log::endl    //
                     << std::format("firmwareVersion: {:x}", (uint32_t)cam.firmwareVersion) << log::endl  //
                     << "productSerialNumber: " << cam.productSerialNumber << log::endl                   //
                     << "transportName: " << cam.transportName << log::endl                               //
                     << "transportType: " << cam.transportType << log::endl                               //
                     << "num_chips: " << cam.num_chips << log::endl                                       //
            ;

        auto &sensor = *cam.sensor;
        log::debug()                                             //
            << "sensorName: " << sensor.sensorName << log::endl  //
            ;

        for (mi_s32 c = 0; c < cam.num_chips; ++c) {
            auto &chip = cam.chip[c];
            log::debug() << "--- chip " << c << log::endl;
            log::debug()                                                              //
                << "chipName: " << chip.chipName << log::endl                         //
                << std::format("version: {:x}", (uint32_t)chip.version) << log::endl  //
                ;
        }

        log::debug() << "mi_setMode " << mi_setMode(&cam, MI_RX_TYPE, MI_RX_LVDS) << log::endl;
        log::debug() << "mi_setMode " << mi_setMode(&cam, MI_RX_BIT_DEPTH, 10) << log::endl;

        //    MI_RX_TYPE,              //  mi_rx_types
        //    MI_RX_LANES,
        //    MI_RX_BIT_DEPTH,
        //    MI_RX_MODE,              //  mi_rx_modes
        //    MI_RX_CLASS,
        //    MI_RX_SYNC_CODE,
        //    MI_RX_EMBEDDED_DATA,
        //    MI_RX_VIRTUAL_CHANNEL,
        //    MI_RX_MSB_FIRST,

        log::debug() << "--- preset" << log::endl;
        log::debug() << "mi_LoadINIPreset "
                     << mi_LoadINIPreset(&cam, "/opt/Aptina_Imaging/apps_data/MT9V024-REV4.ini", "Serial(LVDS) 10-bit Mode ON")
                     << log::endl;

        uint8_t cambuf[722432];
        for (int i = 0; i < 100; ++i) {
            mi_grabFrame(&cam, cambuf, sizeof(cambuf));
            usleep(500);
        }

        mi_stopTransport(&cam);
    }

    mi_CloseCameras();
    return 0;
}