/**
 **********************************************************************************************************************
 * @file  Loclib.h
 * @brief Implementation for decodering the Z21 protocol of Roco
 *http://www.roco.cc/en/home/index.html. <br> Code is based on
 *Z21_LAN_Protokoll-V1.05.
 ***********************************************************************************************************************
 */

#ifndef Z21_SLAVE_H
#define Z21_SLAVE_H

#include <Arduino.h>

// include types & constants of Wiring core API
#define Z21_SLAVE_BUFFER_TX_SIZE 30 /* Buffer size transmit buffer. */
#define Z21_SLAVE_COMMAND_BUFFER_SIZE 3

/**
 * Typedef for call back function of Z21Lan process commands table.
 */
typedef void TZ21LanProcessCommandHandler(const uint8_t* DataRx);

class Z21Slave
{
public:
    /**
     * Type of received data.
     */
    enum dataType
    {
        none = 0,
        trackPowerOn,
        trackPowerOff,
        locinfo,
        lanVersionResponse,
        fwVersionInfoResponse,
        unknown
    };

    /**
     * Decoder step.
     */
    enum locDecoderSteps
    {
        locDecoderSpeedSteps14 = 0,
        locDecoderSpeedSteps28,
        locDecoderSpeedSteps128,
        locDecoderSpeedStepsUnknown,
    };

    /**
     * Locomotive direction.
     */
    enum locDirection
    {
        locDirectionForward = 0,
        locDirectionBackward
    };

    /**
     * Light of locomotive.
     */
    enum locLight
    {
        locLightOn = 0,
        locLightOff
    };

    /**
     * Function setting.
     */
    enum functionSet
    {
        off = 0,
        on,
        toggle,
    };

    /**
     * Structure with received locotive data.
     */
    struct locInfo
    {
        uint16_t Address;
        uint8_t Speed;
        locDecoderSteps Steps;
        locDirection Direction;
        locLight Light;
        uint32_t Functions;
    };

    /**
     * Typedef struct for Z21 command handling.
     */
    typedef struct
    {
        uint8_t CommandBytes[Z21_SLAVE_COMMAND_BUFFER_SIZE];
        uint8_t CommandBytesSize;
        TZ21LanProcessCommandHandler* FunctionPtr;
    } ProcessCommandsTable;

    /**
     * Constructor
     */
    Z21Slave();

    /**
     * Process received data.
     */
    Z21Slave::dataType ProcesDataRx(const uint8_t* DataRxPtr, const uint16_t DataRxLength);

    /**
     * Get data to be transmitted.
     */
    uint8_t* GetDataTx();

    /**
     * Check if Tx data is present.
     */
    bool txDataPresent();

    /**
     * 2.1 LAN_GET_SERIAL_NUMBER
     */
    void LanGetSerialNumber();

    /**
     * 2.2 LAN_LOGOFF
     */
    void LanLogOff();

    /**
     * 2.3 LAN_X_GET_VERSION
     */
    void LanGetVersion();

    /**
     * 2.4 LAN_X_GET_STATUS
     */
    void LanGetStatus();

    /**
     * 2.5 LAN_X_SET_TRACK_POWER_OFF
     */
    void LanSetTrackPowerOff();

    /**
     * 2.6 LAN_X_SET_TRACK_POWER_ON
     */
    void LanSetTrackPowerOn();

    /**
     * 2.15 LAN_X_GET_FIRMWARE_VERSION
     */
    void LanGetFirmwareVersion();

    /**
     * 2.16 LAN_SET_BROADCASTFLAGS
     */
    void LanSetBroadCastFlags(uint32_t Flags);

    /**
     * 3.1 LAN_GET_LOCOMODE
     */
    void LanGetLocoMode(uint16_t Address);

    /**
     * 4.1 LAN_X_GET_LOCO_INFO
     */
    void LanXGetLocoInfo(uint16_t Address);

    /**
     * 4.2 LAN_X_SET_LOCO_DRIVE
     */
    void LanXSetLocoDrive(locInfo* LocInfoPtr);

    /**
     * 4.3 LAN_X_SET_LOCO_FUNCTION
     */
    void LanXSetLocoFunction(uint16_t Address, uint8_t Function, functionSet Set);

    /**
     * 4.4 LAN_X_LOCO_INFO
     */
    Z21Slave::locInfo* LanXLocoInfo();

private:
    uint8_t m_BufferTx[Z21_SLAVE_BUFFER_TX_SIZE]; /* Transmit buffer. */
    uint16_t m_FirmwareVersion;                   /* Firmware version of Z21 central. */
    uint16_t m_XBusVersionInfo;                   /* XBus version info of Z21 central. */
    uint32_t m_SerialNumber;                      /* Serial number of Z21 central. */

    /* Conversion table for normal speed to 28 steps DCC speed. */
    const uint8_t SpeedStep28TableToDcc[29] = { 16, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23, 8, 24, 9, 25, 10, 26, 11,
        27, 12, 28, 13, 29, 14, 30, 15, 31 };

    /* Conversion table for 28 steps DCC speed to normal speed. */
    const uint8_t SpeedStep28TableFromDcc[32] = { 0, 0, 1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 0, 0, 2, 4,
        6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28 };

    locInfo m_locInfo; /* Actual received locinfo. */
    bool m_txDataPresent;

    /**
     * Compose the data to be transmitted.
     */
    void ComposeTxMessage(uint8_t Header, uint8_t* TxData, uint16_t TxLength, bool ChecksumCalc);
    /**
     * Decode the received data.
     */
    dataType DecodeRxMessage(const uint8_t* RxData, uint16_t RxLength);

    /**
     * Decode the status message.
     */
    dataType Status(const uint8_t* RxData);

    /**
     * Decoder the status message for track power.
     */
    dataType TrackPower(const uint8_t* RxData);

    /**
     * Compose the version info.
     */
    dataType GetFirmwareInfo(const uint8_t* RxData);

    /**
     * Get the version.
     */
    dataType GetVersion(const uint8_t* RxData);

    /**
     * Decode the received loc info message.
     */
    dataType ProcessGetLocInfo(const uint8_t* RxData);

    /**
     * Convert loc adresses to Z21 format.
     */
    uint16_t ConvertLocAddressToZ21(uint16_t Address);

    /**
     * Convert loc adresses from Z21 format.
     */
    uint16_t ConvertLocAddressFromZ21(uint16_t Address);
};

#endif
