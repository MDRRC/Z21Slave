/*
  Test.h - Test library for Wiring - implementation
  Copyright (c) 2006 John Doe.  All right reserved.
*/

// include this library's description file
#include "Z21Slave.h"
#include <string.h>

// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

Z21Slave::Z21Slave()
{
    m_SerialNumber    = 0;
    m_evtManagerSet   = false;
    m_FirmwareVersion = 0;
    m_XBusVersionInfo = 0;
    memset(m_BufferTx, 0, Z21_SLAVE_BUFFER_TX_SIZE);
}

// Public Methods //////////////////////////////////////////////////////////////

void Z21Slave::SetEventDestination(EventManager EventMngr, const char* EventMngDestStr, uint16_t NameStrSize)
{
    m_evtManagerSet = true;
    m_evtManager    = EventMngr;
    memcpy(m_EventMngDestStr, EventMngDestStr, NameStrSize);
}

bool Z21Slave::ProcesDataRx(const uint8_t* DataRxPtr, const uint16_t DataRxLength)
{
    bool Result = true;

    // See Anhang A � Befehls�bersicht for the case values.
    switch (DataRxPtr[2])
    {
    case 0x10:
        // LAN_GET_SERIAL_NUMBER
        break;
    case 0x1A:
        // LAN_GET_HWINFO
        break;
    case 0x30:
        // LAN_LOGOFF
        break;
    case 0x40:
        // Run through list of supported commands, if available handle it and
        // generate event.
        DecodeRxMessage(DataRxPtr, DataRxLength);
        break;
    case 0x50:
        // LAN_SET_BROADCASTFLAGS
        break;
    case 0x51:
        // LAN_GET_BROADCASTFLAGS
        break;
    case 0x60:
        // LAN_GET_LOCOMODE
        break;
    case 0x61:
        // LAN_SET_LOCOMODE
        break;
    case 0x70:
        // LAN_GET_TURNOUTMODE
        break;
    case 0x71:
        // LAN_SET_TURNOUTMODE
        break;
    case 0x81:
        // LAN_RMBUS_GETDATA
        break;
    case 0x82:
        // LAN_RMBUS_PROGRAMMODULE
        break;
    case 0x85:
        // LAN_SYSTEMSTATE_GETDATA
        break;
    case 0x89:
        // LAN_RAILCOM_GETDATA
        break;
    case 0xA2:
        // LAN_LOCONET_FROM_LAN
        break;
    case 0xA3:
        // LAN_LOCONET_DISPATCH_ADDR
        break;
    case 0xA4:
        // LAN_LOCONET_DETECTOR
        break;
    default: Result = false; break;
    }

    return (Result);
}

uint8_t* Z21Slave::GetDataTx() { return (m_BufferTx); }

void Z21Slave::LanGetSerialNumber() {}

void Z21Slave::LanLogOff() {}

void Z21Slave::LanGetVersion()
{
    uint8_t DataTx[2];

    DataTx[0] = 0x21;
    DataTx[1] = 0x21;

    ComposeTxMessage(0x40, DataTx, 2, true);
}

void Z21Slave::LanGetStatus()
{
    uint8_t DataTx[2];

    DataTx[0] = 0x21;
    DataTx[1] = 0x24;

    ComposeTxMessage(0x40, DataTx, 2, true);
}

void Z21Slave::LanSetTrackPowerOff()
{
    uint8_t DataTx[2];

    DataTx[0] = 0x21;
    DataTx[1] = 0x80;

    ComposeTxMessage(0x40, DataTx, 2, true);
}

void Z21Slave::LanSetTrackPowerOn()
{
    uint8_t DataTx[2];

    DataTx[0] = 0x21;
    DataTx[1] = 0x81;

    ComposeTxMessage(0x40, DataTx, 2, true);
}

void Z21Slave::LanGetFirmwareVersion()
{
    uint8_t DataTx[2];

    DataTx[0] = 0xf1;
    DataTx[1] = 0x0a;

    ComposeTxMessage(0x40, DataTx, 2, true);
}

void Z21Slave::LanSetBroadCastFlags(uint32_t Flags)
{
    uint8_t DataTx[4];

    DataTx[0] = Flags & 0xFF;
    DataTx[1] = (Flags >> 8) & 0xFF;
    DataTx[2] = (Flags >> 16) & 0xFF;
    DataTx[3] = (Flags >> 24) & 0xFF;

    ComposeTxMessage(0x50, DataTx, 4, true);
}

void Z21Slave::LanGetLocoMode(uint16_t Address)
{
    uint8_t DataTx[4];
    uint16_t AddressLocal;

    AddressLocal = ConvertLocAddressToZ21(Address);

    DataTx[0] = 0x00;
    DataTx[1] = 0x00;
    DataTx[2] = (AddressLocal >> 8) & 0xFF;
    DataTx[3] = (AddressLocal)&0xFF;

    ComposeTxMessage(0x60, DataTx, 4, false);
}

void Z21Slave::LanXGetLocoInfo(uint16_t Address)
{
    uint8_t DataTx[4];
    uint16_t AddressLocal;

    AddressLocal = ConvertLocAddressToZ21(Address);

    DataTx[0] = 0xE3;
    DataTx[1] = 0xF0;
    DataTx[2] = (AddressLocal >> 8) & 0xFF;
    DataTx[3] = (AddressLocal)&0xFF;

    ComposeTxMessage(0x40, DataTx, 4, true);
}

// 4.2 LAN_X_SET_LOCO_DRIVE
void Z21Slave::LanXSetLocoDrive(locInfo* LocInfoPtr)
{
    uint8_t DataTx[5];
    uint16_t AddressLocal;

    DataTx[0] = 0xE4;

    AddressLocal = ConvertLocAddressToZ21(LocInfoPtr->Address);
    DataTx[2]    = (AddressLocal >> 8) & 0xFF;
    DataTx[3]    = (AddressLocal)&0xFF;

    if (LocInfoPtr->Direction == locDirectionForward)
    {
        DataTx[4] = 0x80;
    }
    else
    {
        DataTx[4] = 0;
    }

    switch (LocInfoPtr->Steps)
    {
    case locDecoderSpeedSteps14:
        if (LocInfoPtr->Speed > 0)
        {
            LocInfoPtr->Speed++;
        }
        DataTx[1] = 0x10;
        DataTx[4] |= LocInfoPtr->Speed;
        break;
    case locDecoderSpeedSteps28:
        DataTx[1] = 0x12;
        DataTx[4] |= SpeedStep28TableToDcc[LocInfoPtr->Speed];
        break;
    case locDecoderSpeedSteps128:
        DataTx[1] = 0x13;
        DataTx[4] |= (LocInfoPtr->Speed & 0x7F);
        break;
    case locDecoderSpeedStepsUnknown: break;
    }
    if (LocInfoPtr->Steps != locDecoderSpeedStepsUnknown)
    {
        ComposeTxMessage(0x40, DataTx, 5, true);
    }
}

// 4.3 LAN_X_SET_LOCO_FUNCTION
void Z21Slave::LanXSetLocoFunction(uint16_t Address, uint8_t Function, functionSet Set)
{
    uint8_t DataTx[5];
    uint16_t AddressLocal;

    AddressLocal = ConvertLocAddressToZ21(Address);

    DataTx[0] = 0xE4;
    DataTx[1] = 0xF8;
    DataTx[2] = (AddressLocal >> 8) & 0xFF;
    DataTx[3] = (AddressLocal)&0xFF;

    switch (Set)
    {
    case off: DataTx[4] = 0; break;
    case on: DataTx[4] = 0x40; break;
    case toggle: DataTx[4] = 0x80; break;
    }

    DataTx[4] |= Function;

    ComposeTxMessage(0x40, DataTx, 5, true);
}

Z21Slave::locInfo* Z21Slave::LanXLocoInfo() { return (&m_locInfo); }

// Private Methods /////////////////////////////////////////////////////////////

void Z21Slave::ComposeTxMessage(uint8_t Header, uint8_t* TxDataPtr, uint16_t TxLength, bool ChecksumCalc)
{
    uint16_t Index   = 0;
    uint8_t Checksum = 0;

    // Fill DataLen and Header
    // DataLen is header length + data length + XOR-Byte (if XOR byte is
    // required).
    if (ChecksumCalc == true)
    {
        m_BufferTx[0] = 4 + TxLength + 1;
    }
    else
    {
        m_BufferTx[0] = 4 + TxLength;
    }

    m_BufferTx[1] = 0x00;
    m_BufferTx[2] = Header;
    m_BufferTx[3] = 0x00;

    // Copy data to be transmitted.
    memcpy(&m_BufferTx[4], TxDataPtr, TxLength);

    // Calculate XOR byte of the data.
    if (ChecksumCalc == true)
    {
        for (Index = 0; Index < TxLength; Index++)
        {
            if (Index == 0)
            {
                Checksum = TxDataPtr[Index];
            }
            else
            {
                Checksum ^= TxDataPtr[Index];
            }
        }

        // Store Xor byte
        m_BufferTx[4 + TxLength] = Checksum;
    }

    EventSend(txDataPresent);
}

void Z21Slave::DecodeRxMessage(const uint8_t* RxData, uint16_t RxLength)
{
    RxLength = RxLength;

    switch (RxData[4])
    {
    case 0x61: Status(RxData); break;
    case 0x62: TrackPower(RxData); break;
    case 0x63: GetVersion(RxData); break;
    case 0xF3: GetFirmwareInfo(RxData); break;
    case 0xEF: ProcessGetLocInfo(RxData); break;
    }
}

void Z21Slave::Status(const uint8_t* RxData)
{
    switch (RxData[5])
    {
    case 0x00: EventSend(trackPowerOff); break;
    case 0x01: EventSend(trackPowerOn); break;
    default: EventSend(unknown); break;
    }
}
void Z21Slave::TrackPower(const uint8_t* RxData)
{
    switch (RxData[6])
    {
    case 0x00: EventSend(trackPowerOn); break;
    case 0x02: EventSend(trackPowerOff); break;
    case 0xFF: EventSend(trackPowerOff); break;
    default: EventSend(unknown); break;
    }
}

void Z21Slave::GetFirmwareInfo(const uint8_t* RxData)
{
    m_FirmwareVersion = ((uint16_t)(RxData[6]) << 8) | (uint16_t)(RxData[7]);
    EventSend(fwVersionInfoResponse);
}

void Z21Slave::GetVersion(const uint8_t* RxData)
{
    m_XBusVersionInfo = ((uint16_t)(RxData[6]) << 8) | (uint16_t)(RxData[7]);
    EventSend(lanVersionResponse);
}

void Z21Slave::ProcessGetLocInfo(const uint8_t* RxData)
{
    uint16_t Address;

    Address = (uint16_t)(RxData[5]) << 8;
    Address |= RxData[6];

    m_locInfo.Address = ConvertLocAddressFromZ21(Address);

    switch (RxData[7] & 0x07)
    {
    case 0:
        m_locInfo.Steps = locDecoderSpeedSteps14;
        m_locInfo.Speed = RxData[8] & 0x7F;
        if (m_locInfo.Speed > 0)
        {
            m_locInfo.Speed--;
        }
        break;
    case 2:
        m_locInfo.Steps = locDecoderSpeedSteps28;
        m_locInfo.Speed = SpeedStep28TableFromDcc[RxData[8] & 0x7F];
        break;
    case 4:
        m_locInfo.Steps = locDecoderSpeedSteps128;
        m_locInfo.Speed = RxData[8] & 0x7F;
        if (m_locInfo.Speed > 0)
        {
            m_locInfo.Speed--;
        }
        break;
    default: m_locInfo.Steps = locDecoderSpeedStepsUnknown; break;
    }

    if (RxData[8] & 0x80)
    {
        m_locInfo.Direction = locDirectionForward;
    }
    else
    {
        m_locInfo.Direction = locDirectionBackward;
    }

    if (RxData[9] & 0x10)
    {
        m_locInfo.Light = locLightOn;
    }
    else
    {
        m_locInfo.Light = locLightOff;
    }

    m_locInfo.Functions = RxData[9] & 0x0F;
    m_locInfo.Functions |= (uint32_t)(RxData[10]) << 4;
    m_locInfo.Functions |= (uint32_t)(RxData[11]) << 12;
    m_locInfo.Functions |= (uint32_t)(RxData[12]) << 20;

    EventSend(locinfo);
}

void Z21Slave::EventSend(EventData Data)
{
    m_z21Event.label = m_EventMngDestStr;
    m_z21Event.extra = (uint16_t)(Data);

    if (m_evtManagerSet == true)
    {
        m_evtManager.trigger(m_z21Event);
    }
}

uint16_t Z21Slave::ConvertLocAddressToZ21(uint16_t Address)
{
    uint16_t AddressLocal = Address;
    if (AddressLocal >= 128)
    {
        AddressLocal += 0xC000;
    }

    return (AddressLocal);
}

uint16_t Z21Slave::ConvertLocAddressFromZ21(uint16_t Address)
{
    uint16_t AddressLocal = Address;
    if (AddressLocal >= 128)
    {
        AddressLocal -= 0xC000;
    }

    return (AddressLocal);
}