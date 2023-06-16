#include <stdio.h>
#include <sys/io.h>
#include <locale.h>
#include "pci.h"

#define ushort unsigned short
#define uchar unsigned char
#define uint unsigned int

#define BUS_MAX_NUM 255
#define DEVICE_MAX_NUM 31
#define FUNC_MAX_NUM 7

#define LAST_BIT 1
#define LAST_BIT_SHIFT 31
#define BUS_NUM_SHIFT 16
#define DEVICE_NUM_SHIFT 11
#define FUNC_NUM_SHIFT 8
#define REG_NUM_SHIFT 2

#define ID_REGS 0
#define HEADER_TYPE_REG 3
#define INTERRUPT_PIN_REG 15
#define CLASS_CODE_REG 2

#define SHIFT_2_BYTE 16
#define SHIFT_1_BYTE 8

#define CONTROL_PORT 0xCF8
#define LOAD_32_BIT 0xCFC

#define UNDEFINED 0xFFFFFFFF

#define START_IO_REG 4
#define FINISH_IO_REG 10

#define GET_1_BYTE(a) ((a) & 0x000000FF)
#define GET_2_BYTE(a) ((a) & 0x0000FFFF)
#define START_BIT_ZERO(a) (!((a) & 0x00000001))
#define GET_BASE_ADDRESS(a) ((a) >> 2)

void getDevInfo(uint reg, uchar bus, uchar device, uchar func) {
    ushort dev_ID = (ushort) (reg >> SHIFT_2_BYTE);
    ushort ven_ID = (ushort) (GET_2_BYTE(reg));
    printf("\nBus number: %d\n", bus);
    printf("Device number: %d\n", device);
    printf("Function number: %d\n", func);
    printf("Vendor ID: %X\n ", ven_ID);
    for (int i = 0; i < PCI_VENTABLE_LEN; i++)
        if (ven_ID == PciVenTable[i].VendorId)
            printf("Vendor name: %s\n", PciVenTable[i].VendorName);

    printf("Device ID: %X\n ", dev_ID);
    for (int i = 0; i < PCI_DEVTABLE_LEN; i++)
        if (ven_ID == PciDevTable[i].VendorId && dev_ID == PciDevTable[i].DeviceId)
            printf("Device: %s\n", PciDevTable[i].DeviceName);
}

void getInterruptPIN(uchar bus, uchar device, uchar func){
    uint address = (LAST_BIT << LAST_BIT_SHIFT) | (bus << BUS_NUM_SHIFT) | (device << DEVICE_NUM_SHIFT) |
                   (func << FUNC_NUM_SHIFT) | (INTERRUPT_PIN_REG << REG_NUM_SHIFT);
    outl(address, CONTROL_PORT);
    uint reg = inl(LOAD_32_BIT);
    uchar interrupt_pin = (uchar) (reg >> SHIFT_1_BYTE);
    printf("Interrupt pin = %d : ", interrupt_pin);
    switch (interrupt_pin) {
        case 0:
            printf("not used\n");
            break;
        case 1:
            printf("INTA\n");
            break;
        case 2:
            printf("INTB\n");
            break;
        case 3:
            printf("INTC\n");
            break;
        case 4:
            printf("INTD\n");
            break;
        case 0xFF:
            printf("reserved\n");
            break;
        default:
            break;
    }
}

void getIORegs(uchar bus, uchar device, uchar func){
    for (int i = START_IO_REG; i < FINISH_IO_REG; i++) {
        uint address = (LAST_BIT << LAST_BIT_SHIFT) | (bus << BUS_NUM_SHIFT) | (device << DEVICE_NUM_SHIFT) |
                       (func << FUNC_NUM_SHIFT) | (i << REG_NUM_SHIFT);
        outl(address, CONTROL_PORT);
        uint reg = inl(LOAD_32_BIT);
        if (!START_BIT_ZERO(reg)) {
            printf("Base address = %X\n", GET_BASE_ADDRESS(reg));
        }
    }
}

void getClassCode(uchar bus, uchar device, uchar func){
    uint address = (LAST_BIT << LAST_BIT_SHIFT) | (bus << BUS_NUM_SHIFT) | (device << DEVICE_NUM_SHIFT) |
              (func << FUNC_NUM_SHIFT) | (CLASS_CODE_REG << REG_NUM_SHIFT);
    outl(address, CONTROL_PORT);
    uint reg = inl(LOAD_32_BIT);
    uchar class_code = reg >> SHIFT_1_BYTE;
    uchar base_class, subclass, interface;
    base_class = (uchar) (GET_1_BYTE(class_code >> SHIFT_2_BYTE));
    subclass = (uchar) (GET_1_BYTE(class_code >> SHIFT_1_BYTE));
    interface = (uchar) (GET_1_BYTE(class_code));
    for (int i = 0; i < PCI_CLASSCODETABLE_LEN; i++)
        if (base_class == PciClassCodeTable[i].BaseClass &&
            subclass == PciClassCodeTable[i].SubClass && interface == PciClassCodeTable[i].ProgIf)
            printf("%s %s %s\n", PciClassCodeTable[i].BaseDesc,
                   PciClassCodeTable[i].SubDesc, PciClassCodeTable[i].ProgDesc);
}

int main() {
    setlocale(LC_ALL, "Rus");
    uint address, reg;
    uchar header_type;

    if (iopl(3)) {
        printf("I/O Privilege level change error\nTry running under ROOT user\n");
        return 1;
    }

    for (uchar bus_num = 0; bus_num < BUS_MAX_NUM; bus_num++) {
        for (uchar device_num = 0; device_num < DEVICE_MAX_NUM; device_num++) {
            for (uchar func_num = 0; func_num < FUNC_MAX_NUM; func_num++) {

                //base
                address = (LAST_BIT << LAST_BIT_SHIFT) | (bus_num << BUS_NUM_SHIFT) | (device_num << DEVICE_NUM_SHIFT) |
                          (func_num << FUNC_NUM_SHIFT) | (ID_REGS << REG_NUM_SHIFT);
                outl(address, CONTROL_PORT);
                reg = inl(LOAD_32_BIT);
                if (reg != UNDEFINED) {
                    getDevInfo(reg, bus_num, device_num, func_num);
                }

                address = (LAST_BIT << LAST_BIT_SHIFT) | (bus_num << BUS_NUM_SHIFT) | (device_num << DEVICE_NUM_SHIFT) |
                          (func_num << FUNC_NUM_SHIFT) | (HEADER_TYPE_REG << REG_NUM_SHIFT);
                outl(address, CONTROL_PORT);
                reg = inl(LOAD_32_BIT);

                header_type = (uchar) (GET_1_BYTE(reg >> SHIFT_2_BYTE));
                if (START_BIT_ZERO(header_type)) {
                    // Task 6
                    getInterruptPIN(bus_num, device_num, func_num);
                    // Task 3
                    getIORegs(bus_num, device_num, func_num);
                    // Task 1
                    getClassCode(bus_num, device_num, func_num);
                }
            }
        }
    }
}