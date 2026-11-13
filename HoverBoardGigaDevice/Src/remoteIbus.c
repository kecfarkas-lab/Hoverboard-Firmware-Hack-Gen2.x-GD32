#include "../Inc/remoteIbus.h"
#include "../Inc/comms.h"

#ifdef REMOTE_IBUS

#define IBUS_BUFFSIZE 32
#define IBUS_MAXCHANNELS 14

static uint8_t ibusIndex = 0;
static uint8_t ibusBuffer[IBUS_BUFFSIZE] = {0};
static uint16_t rcValue[IBUS_MAXCHANNELS] = {1500, 1500, 1500, 1500};

extern int16_t speed;
extern int16_t steer;
extern uint32_t msTicks;
static uint32_t iTimeLastRx = 0;

// Ova funkcija se poziva svaki put kad stigne 1 bajt na PB7 (RX)
void RemoteCallback(void) {
    uint8_t val = usart_data_receive(USART0);

    // Početak iBUS paketa uvijek je 0x20 (duljina 32 bajta), drugi bajt je 0x40
    if (ibusIndex == 0 && val != 0x20) {
        return;
    }
    if (ibusIndex == 1 && val != 0x40) {
        ibusIndex = 0;
        return;
    }

    ibusBuffer[ibusIndex++] = val;

    if (ibusIndex == IBUS_BUFFSIZE) {
        ibusIndex = 0;

        // Provjera ispravnosti paketa (Checksum)
        uint16_t chksum = 0xFFFF;
        for (int i = 0; i < 30; i++) {
            chksum -= ibusBuffer[i];
        }
        uint16_t rxsum = ibusBuffer[30] | (ibusBuffer[31] << 8);

        if (chksum == rxsum) {
            for (int i = 0; i < IBUS_MAXCHANNELS; i++) {
                rcValue[i] = ibusBuffer[2 + i * 2] | (ibusBuffer[3 + i * 2] << 8);
            }
            iTimeLastRx = msTicks;
        }
    }
}

// Ova funkcija pretvara kanale (1000-2000) u brzinu i skretanje (-1000 do 1000)
void RemoteUpdate(void) {
    // Sigurnosni prekid (Failsafe) ako nema signala duže od 250 ms
    if (msTicks - iTimeLastRx > 250) {
        speed = 0;
        steer = 0;
        return;
    }

    // Kanal 1 (desna palica lijevo-desno) -> Skretanje (steer)
    // Kanal 2 (desna palica gore-dolje) -> Brzina (speed)
    int16_t ch_steer = (int16_t)rcValue[0] - 1500;
    int16_t ch_speed = (int16_t)rcValue[1] - 1500;

    // Mala mrtva zona (deadband) oko sredine palice da motori ne zuje u mjestu
    if (ch_steer > -20 && ch_steer < 20) ch_steer = 0;
    if (ch_speed > -20 && ch_speed < 20) ch_speed = 0;

    // Skaliranje s [-500..500] na [-1000..1000]
    steer = CLAMP(ch_steer * 2, -1000, 1000);
    speed = CLAMP(ch_speed * 2, -1000, 1000);
}

#endif // REMOTE_IBUS
