#include "../Inc/defines.h"
#include "../Inc/remoteIbus.h"

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

// Poziva se automatski svaki put kad stigne bajt na serijski port
void RemoteCallback(void) {
    uint8_t val = usart_data_receive(USART0);

    // Svaki iBUS paket počinje s 0x20 (duljina 32) i 0x40 (naredba)
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

        // Izračun kontrolnog zbroja (Checksum)
        uint16_t chksum = 0xFFFF;
        for (int i = 0; i < 30; i++) {
            chksum -= ibusBuffer[i];
        }
        uint16_t rxsum = (uint16_t)ibusBuffer[30] | ((uint16_t)ibusBuffer[31] << 8);

        // Ako je paket ispravan, spremi vrijednosti svih kanala
        if (chksum == rxsum) {
            for (int i = 0; i < IBUS_MAXCHANNELS; i++) {
                rcValue[i] = (uint16_t)ibusBuffer[2 + i * 2] | ((uint16_t)ibusBuffer[3 + i * 2] << 8);
            }
            iTimeLastRx = msTicks;
        }
    }
}

// Poziva se u glavnoj petlji i pretvara poziciju palica u brzinu i skretanje
void RemoteUpdate(void) {
    // Sigurnosno zaustavljanje (Failsafe) ako se izgubi signal na duže od 250 ms
    if (msTicks - iTimeLastRx > 250) {
        speed = 0;
        steer = 0;
        return;
    }

    // rcValue[0] = Kanal 1 (desna palica lijevo-desno -> skretanje)
    // rcValue[1] = Kanal 2 (desna palica gore-dolje -> brzina)
    int16_t ch_steer = (int16_t)rcValue[0] - 1500;
    int16_t ch_speed = (int16_t)rcValue[1] - 1500;

    // Mrtva zona (deadband) u sredini da motori miruju kad pustiš palicu
    if (ch_steer > -20 && ch_steer < 20) ch_steer = 0;
    if (ch_speed > -20 && ch_speed < 20) ch_speed = 0;

    // Pretvaranje raspona [-500 .. +500] u [-1000 .. +1000]
    steer = CLAMP(ch_steer * 2, -1000, 1000);
    speed = CLAMP(ch_speed * 2, -1000, 1000);
}

#endif // REMOTE_IBUS
