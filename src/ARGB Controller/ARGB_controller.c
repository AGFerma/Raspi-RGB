#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include "ws2811.h"
#include <signal.h>
#include <fcntl.h>

#include <sys/stat.h> // pour mkfifo
#include <errno.h>    // pour errno et ENOENT

#define GPIO_PIN_1      18  // PWM0
#define GPIO_PIN_2      13  // PWM1
#define DMA              10
#define TARGET_FREQ      WS2811_TARGET_FREQ
#define MAX_SECTIONS      6

enum Mode {
    MODE_STATIC = 0,
    MODE_BLINK = 1,
    MODE_CHASE = 2,
    MODE_FILL = 3,
    MODE_FILLREV = 4,
    MODE_FLOW = 5,
    MODE_BREATHE = 6,

    MODE_IDLE = 7,
    MODE_RAINBOW = 8,
    MODE_COMET = 9,
    MODE_WAVE = 10

};

// Canal principal
int nb_led[MAX_SECTIONS]       = {0};
int r[MAX_SECTIONS]            = {255}, v[MAX_SECTIONS] = {255}, b[MAX_SECTIONS] = {255};
enum Mode mode[MAX_SECTIONS]   = { MODE_STATIC };
int taux_cpu[MAX_SECTIONS]     = { 1, 30, 65, 43, 75, 23 };

// Canal parallèle
int parallel                   = 0;
int p_nb_led[MAX_SECTIONS]     = {0};
int p_r[MAX_SECTIONS]          = {255}, p_v[MAX_SECTIONS] = {255}, p_b[MAX_SECTIONS] = {255};
enum Mode p_mode[MAX_SECTIONS] = { MODE_STATIC };
int p_taux_cpu[MAX_SECTIONS]   = { 10, 30, 65, 43, 75, 23 };

int nb_sections = 1;
int LED_COUNT   = 0;
int P_LED_COUNT = 0;

// ligne de commande : --fifo-prefix FIFO-SUN --pfifo-prefix FIFO-MOON

char fifo_prefix[256] = "FIFO-SUN";     // canal 0
char p_fifo_prefix[256] = "FIFO-MOON";  // canal 1
int fifo_fd[MAX_SECTIONS] = {-1};     // pour les sections du canal principal
int p_fifo_fd[MAX_SECTIONS] = {-1};   // pour les sections du canal parallèle
/*
int temp_value[MAX_SECTIONS]     = { -1 };
int p_temp_value[MAX_SECTIONS]   = { -1 };
*/
enum Mode default_mode[MAX_SECTIONS];   // pour le canal principal
enum Mode p_default_mode[MAX_SECTIONS]; // pour le canal parallèle

int r_def[MAX_SECTIONS], v_def[MAX_SECTIONS], b_def[MAX_SECTIONS];
int p_r_def[MAX_SECTIONS], p_v_def[MAX_SECTIONS], p_b_def[MAX_SECTIONS];


typedef struct { int debut, fin; } Section;
Section sections[MAX_SECTIONS], p_sections[MAX_SECTIONS];

ws2811_t ledstring;

static enum Mode parse_mode(const char *s) {
    if (!strcmp(s, "blink"))    return MODE_BLINK;
    if (!strcmp(s, "chase"))    return MODE_CHASE;
    if (!strcmp(s, "fill"))     return MODE_FILL;
    if (!strcmp(s, "fillrev"))  return MODE_FILLREV;
    if (!strcmp(s, "flow"))     return MODE_FLOW;
    if (!strcmp(s, "breathe"))  return MODE_BREATHE;
    if (!strcmp(s, "idle"))     return MODE_IDLE;
    if (!strcmp(s, "rainbow"))  return MODE_RAINBOW;
    if (!strcmp(s, "comet"))    return MODE_COMET;
    if (!strcmp(s, "wave"))     return MODE_WAVE;

    return MODE_STATIC;
}

void parse_args(int argc, char **argv) {
    static struct option long_options[] = {
        { "sections", required_argument, 0, 'n' },
        { "parallel", required_argument, 0, 'p' },
        { "l1", required_argument, 0, 'a' },{ "l2", required_argument, 0, 'b' },
        { "l3", required_argument, 0, 'c' },{ "l4", required_argument, 0, 'd' },
        { "l5", required_argument, 0, 'e' },{ "l6", required_argument, 0, 'f' },
        { "r1", required_argument, 0, 1001 },{ "g1", required_argument, 0, 1002 },{ "b1", required_argument, 0, 1003 },
        { "r2", required_argument, 0, 1004 },{ "g2", required_argument, 0, 1005 },{ "b2", required_argument, 0, 1006 },
        { "r3", required_argument, 0, 1007 },{ "g3", required_argument, 0, 1008 },{ "b3", required_argument, 0, 1009 },
        { "r4", required_argument, 0, 1010 },{ "g4", required_argument, 0, 1011 },{ "b4", required_argument, 0, 1012 },
        { "r5", required_argument, 0, 1013 },{ "g5", required_argument, 0, 1014 },{ "b5", required_argument, 0, 1015 },
        { "r6", required_argument, 0, 1016 },{ "g6", required_argument, 0, 1017 },{ "b6", required_argument, 0, 1018 },
        { "m1", required_argument, 0, 5001 },{ "m2", required_argument, 0, 5002 },
        { "m3", required_argument, 0, 5003 },{ "m4", required_argument, 0, 5004 },
        { "m5", required_argument, 0, 5005 },{ "m6", required_argument, 0, 5006 },
        { "tcpu1", required_argument, 0, 4001 },{ "tcpu2", required_argument, 0, 4002 },
        { "tcpu3", required_argument, 0, 4003 },{ "tcpu4", required_argument, 0, 4004 },
        { "tcpu5", required_argument, 0, 4005 },{ "tcpu6", required_argument, 0, 4006 },
        { "pl1", required_argument, 0, 2001 },{ "pl2", required_argument, 0, 2002 },
        { "pl3", required_argument, 0, 2003 },{ "pl4", required_argument, 0, 2004 },
        { "pl5", required_argument, 0, 2005 },{ "pl6", required_argument, 0, 2006 },
        { "pr1", required_argument, 0, 3001 },{ "pg1", required_argument, 0, 3002 },{ "pb1", required_argument, 0, 3003 },
        { "pr2", required_argument, 0, 3004 },{ "pg2", required_argument, 0, 3005 },{ "pb2", required_argument, 0, 3006 },
        { "pr3", required_argument, 0, 3007 },{ "pg3", required_argument, 0, 3008 },{ "pb3", required_argument, 0, 3009 },
        { "pr4", required_argument, 0, 3010 },{ "pg4", required_argument, 0, 3011 },{ "pb4", required_argument, 0, 3012 },
        { "pr5", required_argument, 0, 3013 },{ "pg5", required_argument, 0, 3014 },{ "pb5", required_argument, 0, 3015 },
        { "pr6", required_argument, 0, 3016 },{ "pg6", required_argument, 0, 3017 },{ "pb6", required_argument, 0, 3018 },
        { "pm1", required_argument, 0, 6001 },{ "pm2", required_argument, 0, 6002 },
        { "pm3", required_argument, 0, 6003 },{ "pm4", required_argument, 0, 6004 },
        { "pm5", required_argument, 0, 6005 },{ "pm6", required_argument, 0, 6006 },
        { "ptcpu1", required_argument, 0, 4007 },{ "ptcpu2", required_argument, 0, 4008 },
        { "ptcpu3", required_argument, 0, 4009 },{ "ptcpu4", required_argument, 0, 4010 },
        { "ptcpu5", required_argument, 0, 4011 },{ "ptcpu6", required_argument, 0, 4012 },{ "fifo-prefix", required_argument, 0, 9001 },{ "pfifo-prefix", required_argument, 0, 9002 },
        { 0,0,0,0 }
    };

    int opt, idx = 0;
    while ((opt = getopt_long(argc, argv, "n:p:a:b:c:d:e:f:", long_options, &idx)) != -1) {
        if (opt == 'n') {
            nb_sections = atoi(optarg);
            if (nb_sections > MAX_SECTIONS) nb_sections = MAX_SECTIONS;
        } else if (opt == 'p') {
            parallel = atoi(optarg);
        } else {
            int val = atoi(optarg);
            switch (opt) {
                case 'a': nb_led[0] = val; break; case 'b': nb_led[1] = val; break;
                case 'c': nb_led[2] = val; break; case 'd': nb_led[3] = val; break;
                case 'e': nb_led[4] = val; break; case 'f': nb_led[5] = val; break;
                case 1001: r[0] = val; break; case 1002: v[0] = val; break; case 1003: b[0] = val; break;
                case 1004: r[1] = val; break; case 1005: v[1] = val; break; case 1006: b[1] = val; break;
                case 1007: r[2] = val; break; case 1008: v[2] = val; break; case 1009: b[2] = val; break;
                case 1010: r[3] = val; break; case 1011: v[3] = val; break; case 1012: b[3] = val; break;
                case 1013: r[4] = val; break; case 1014: v[4] = val; break; case 1015: b[4] = val; break;
                case 1016: r[5] = val; break; case 1017: v[5] = val; break; case 1018: b[5] = val; break;
                case 5001: mode[0] = parse_mode(optarg); break; case 5002: mode[1] = parse_mode(optarg); break;
                case 5003: mode[2] = parse_mode(optarg); break; case 5004: mode[3] = parse_mode(optarg); break;
                case 5005: mode[4] = parse_mode(optarg); break; case 5006: mode[5] = parse_mode(optarg); break;
                case 4001: taux_cpu[0] = val; break; case 4002: taux_cpu[1] = val; break;
                case 4003: taux_cpu[2] = val; break; case 4004: taux_cpu[3] = val; break;
                case 4005: taux_cpu[4] = val; break; case 4006: taux_cpu[5] = val; break;
                case 2001: p_nb_led[0] = val; break; case 2002: p_nb_led[1] = val; break;
                case 2003: p_nb_led[2] = val; break; case 2004: p_nb_led[3] = val; break;
                case 2005: p_nb_led[4] = val; break; case 2006: p_nb_led[5] = val; break;
                case 3001: p_r[0] = val; break; case 3002: p_v[0] = val; break; case 3003: p_b[0] = val; break;
                case 3004: p_r[1] = val; break; case 3005: p_v[1] = val; break; case 3006: p_b[1] = val; break;
                case 3007: p_r[2] = val; break; case 3008: p_v[2] = val; break; case 3009: p_b[2] = val; break;
                case 3010: p_r[3] = val; break; case 3011: p_v[3] = val; break; case 3012: p_b[3] = val; break;
                case 3013: p_r[4] = val; break; case 3014: p_v[4] = val; break; case 3015: p_b[4] = val; break;
                case 3016: p_r[5] = val; break; case 3017: p_v[5] = val; break; case 3018: p_b[5] = val; break;
                case 6001: p_mode[0] = parse_mode(optarg); break; case 6002: p_mode[1] = parse_mode(optarg); break;
                case 6003: p_mode[2] = parse_mode(optarg); break; case 6004: p_mode[3] = parse_mode(optarg); break;
                case 6005: p_mode[4] = parse_mode(optarg); break; case 6006: p_mode[5] = parse_mode(optarg); break;
                case 4007: p_taux_cpu[0] = val; break; case 4008: p_taux_cpu[1] = val; break;
                case 4009: p_taux_cpu[2] = val; break; case 4010: p_taux_cpu[3] = val; break;
                case 4011: p_taux_cpu[4] = val; break; case 4012: p_taux_cpu[5] = val; break;
                case 9001:strncpy(fifo_prefix, optarg, sizeof(fifo_prefix) - 1);break;
                case 9002:strncpy(p_fifo_prefix, optarg, sizeof(p_fifo_prefix) - 1);break;
                default:
                    fprintf(stderr, "Usage: %s --sections N --parallel 0|1 \
                            --l1..6 --r1..6 --m1..6 [--tcpu1..6 --pl1..6 ...]\n", argv[0]);
                    exit(EXIT_FAILURE);
            }
        }
    }

    for (int i = 0; i < nb_sections; i++) {
        LED_COUNT   += nb_led[i];
        P_LED_COUNT += p_nb_led[i];
    }
    if (LED_COUNT == 0) {
        fprintf(stderr, "Erreur: aucune LED définie sur canal principal\n");
        exit(1);
    }
}
void clear_leds_and_exit(int signum) {
    // Éteindre toutes les LEDs
    for (int i = 0; i < LED_COUNT; i++) ledstring.channel[0].leds[i] = 0;
    for (int i = 0; i < P_LED_COUNT; i++) ledstring.channel[1].leds[i] = 0;
    ws2811_render(&ledstring); // appliquer la mise à jour
    ws2811_fini(&ledstring);   // libérer proprement
    // Supprimer les FIFO créés
    for (int i = 0; i < nb_sections; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s%d", fifo_prefix, i+1);
        unlink(path);  // supprimer le FIFO

        if (parallel) {
            snprintf(path, sizeof(path), "%s%d", p_fifo_prefix, i+1);
            unlink(path);  // supprimer le FIFO parallèle
        }
    }
    printf("\nLEDs éteintes. Sortie propre.\n");
    exit(0); // sortie normale
}

void apply_mode(ws2811_channel_t *ch, Section *sec, enum Mode m,
                int red, int green, int blue, int tick, int rate) {
    uint8_t R = red   * 0.01 * 255;
    uint8_t G = green * 0.01 * 255;
    uint8_t B = blue  * 0.01 * 255;
    int start = sec->debut, end = sec->fin;
    int len   = end - start + 1;
    rate=100-rate;
    if(rate<0) exit(1);

    switch (m) {
        case MODE_STATIC:
            for (int j = start; j <= end; j++) ch->leds[j] = (R<<16)|(G<<8)|B;
            break;
        case MODE_BLINK:
            if ((tick % (2*rate)) < rate)
                for (int j = start; j <= end; j++) ch->leds[j] = (R<<16)|(G<<8)|B;
            break;
        case MODE_CHASE: {
            int idx = start + ((tick / rate) % len);
            ch->leds[idx] = (R<<16)|(G<<8)|B;
            break;
        }
        case MODE_FILL: {
            int count = (tick / rate) % (len + 1);
            for (int j = start; j < start + count; j++) ch->leds[j] = (R<<16)|(G<<8)|B;
            break;
        }
        case MODE_FILLREV: {
            int phase = (tick / rate) % (2 * len);
            if (phase < len) {
                for (int j = start; j <= start + phase; j++) ch->leds[j] = (R<<16)|(G<<8)|B;
            } else {
                int off = phase - len;
                for (int j = start + off + 1; j <= end; j++) ch->leds[j] = (R<<16)|(G<<8)|B;
            }
            break;
        }
        case MODE_FLOW:{
            for(int j=start;j<=end;j++) ch->leds[j]=(R<<16)|(G<<8)|B;
            float pos = tick / (float)rate;
            int front = start + ((int)pos % len);
            for(int i=0;i<len;i++) {
                int led = start + i;
                int dist = (front - led + len) % len;
                float alpha = 1.0f - (dist / (float)len);
                uint8_t r2 = R * alpha, g2 = G * alpha, b2 = B * alpha;
                ch->leds[led] = (r2<<16)|(g2<<8)|b2;
            }
            break;
        }
        case MODE_BREATHE: {
            // respiration: lum pulsing sur toute la section
            float phase = (float)(tick % (rate*2)) / (rate*2);
            float brightness = (phase < 0.5f ? phase : 1 - phase) * 2;  // va de 0 à 1 et retour
            uint8_t r2 = R * brightness;
            uint8_t g2 = G * brightness;
            uint8_t b2 = B * brightness;
            for (int j = start; j <= end; j++) ch->leds[j] = (r2<<16)|(g2<<8)|b2;
            break;
        }
        case MODE_IDLE: {
            // instant breath: very short ON, long OFF, ignores rate
            const int CYCLE = 200;
            const int ON_TIME = 10;
            if ((tick % CYCLE) < ON_TIME) {
                for (int j = start; j <= end; j++) ch->leds[j] = (R<<16)|(G<<8)|B;
            }
            // else remains off
            break;
        }
        case MODE_RAINBOW: {
            // cycle rainbow hues across LEDs
            for (int i = 0; i < len; i++) {
                int idx = start + i;
                float hue = fmodf((tick*0.5f + (360.0f/len)*i), 360.0f);
                // convert HSV(hue,1,1) to RGB
                int h = (int)(hue/60) % 6;
                float f = (hue/60) - h;
                float p = 0;
                float q = 1 - f;
                float t = f;
                float r_, g_, b_;
                switch(h) {
                    case 0: r_=1; g_=t; b_=p; break;
                    case 1: r_=q; g_=1; b_=p; break;
                    case 2: r_=p; g_=1; b_=t; break;
                    case 3: r_=p; g_=q; b_=1; break;
                    case 4: r_=t; g_=p; b_=1; break;
                    case 5: default: r_=1; g_=p; b_=q; break;
                }
                uint8_t R2 = (uint8_t)(r_*255);
                uint8_t G2 = (uint8_t)(g_*255);
                uint8_t B2 = (uint8_t)(b_*255);
                ch->leds[idx] = (R2<<16)|(G2<<8)|B2;
            }
            break;
        }
        case MODE_COMET: {
            int head = start + ((tick / rate) % len);
            int tail = rate / 5;
            if (tail < 1) tail = 1;
            for (int i = 0; i < len; i++) {
                int idx = start + i;
                int dist = (head - idx + len) % len;
                float brightness = (dist < tail) ? (1.0f - (dist / (float)tail)) : 0.0f;
                uint8_t r2 = R * brightness;
                uint8_t g2 = G * brightness;
                uint8_t b2 = B * brightness;
                ch->leds[idx] = (r2<<16)|(g2<<8)|b2;
            }
            break;
        }
        case MODE_WAVE: {
            /* Effet onde sinusoïdale d'intensité inversé */
            for (int i = 0; i < len; i++) {
                /* on inverser la direction en prenant (len-1 - i) */
                float offset = (float)(len - 1 - i) / len;
                float phase = offset + ((float)tick / rate);
                float lum = 0.5f * (1.0f + sinf(2.0f * M_PI * phase));
                uint8_t r2 = (uint8_t)(R * lum);
                uint8_t g2 = (uint8_t)(G * lum);
                uint8_t b2 = (uint8_t)(B * lum);
                ch->leds[start + i] = (r2<<16)|(g2<<8)|b2;
            }
            break;
        }
    }
}

int main(int argc, char **argv) {
    parse_args(argc, argv);
    ledstring.freq   = TARGET_FREQ;
    ledstring.dmanum = DMA;
    ledstring.channel[0].gpionum    = GPIO_PIN_1;
    ledstring.channel[0].count      = LED_COUNT;
    ledstring.channel[0].invert     = 0;
    ledstring.channel[0].brightness = 255;
    ledstring.channel[0].strip_type = WS2811_STRIP_GRB;
    ledstring.channel[1].gpionum    = GPIO_PIN_2;
    ledstring.channel[1].count      = P_LED_COUNT;
    ledstring.channel[1].invert     = 0;
    ledstring.channel[1].brightness = 255;
    ledstring.channel[1].strip_type = WS2811_STRIP_GRB;

    if (ws2811_init(&ledstring) != WS2811_SUCCESS) {
        fprintf(stderr, "Erreur init WS2811\n");
        return 1;
    }
    signal(SIGINT, clear_leds_and_exit);
    for (int i = 0; i < nb_sections; i++) {
        char path[512];
        snprintf(path, sizeof(path), "%s%d", fifo_prefix, i+1);
        fifo_fd[i] = open(path, O_RDONLY | O_NONBLOCK);
        if (fifo_fd[i] == -1) {
            if (errno == ENOENT) {  // FIFO n'existe pas
                if (mkfifo(path, 0666) == -1) {
                    perror("mkfifo canal principal");
                } else {
                    fifo_fd[i] = open(path, O_RDONLY | O_NONBLOCK);
                    if (fifo_fd[i] == -1) perror("open après mkfifo principal");
                }
            } else {
                perror(path);
            }
        }


        if (parallel) {
            snprintf(path, sizeof(path), "%s%d", p_fifo_prefix, i+1);
            p_fifo_fd[i] = open(path, O_RDONLY | O_NONBLOCK);
            if (p_fifo_fd[i] == -1) {
                if (errno == ENOENT) {
                    if (mkfifo(path, 0666) == -1) {
                        perror("mkfifo canal parallèle");
                    } else {
                        p_fifo_fd[i] = open(path, O_RDONLY | O_NONBLOCK);
                        if (p_fifo_fd[i] == -1) perror("open après mkfifo parallèle");
                    }
                } else {
                    perror(path);
                }
            }

        }
    }

    for (int i = 0; i < MAX_SECTIONS; i++) {
        default_mode[i]    = mode[i];
        p_default_mode[i]  = p_mode[i];
        r_def[i]  = r[i];   v_def[i]  = v[i];   b_def[i]  = b[i];
        p_r_def[i] = p_r[i]; p_v_def[i] = p_v[i]; p_b_def[i] = p_b[i];
    }

    int offset = 0, poffset = 0;
    for (int i = 0; i < nb_sections; i++) {
        sections[i].debut = offset;
        sections[i].fin   = offset + nb_led[i] - 1;
        offset += nb_led[i];
        p_sections[i].debut = poffset;
        p_sections[i].fin   = poffset + p_nb_led[i] - 1;
        poffset += p_nb_led[i];
    }

    int tick = 0;
    while (1) {


        for (int i = 0; i < LED_COUNT; i++) ledstring.channel[0].leds[i] = 0;
        if (parallel) for (int i = 0; i < P_LED_COUNT; i++) ledstring.channel[1].leds[i] = 0;
        for (int i = 0; i < nb_sections; i++) {
            // CANAL PRINCIPAL
            if (fifo_fd[i] != -1) {
                char buf[128]; int zeta = -1, cpu = -1;
                int n = read(fifo_fd[i], buf, sizeof(buf) - 1);
                if (n > 0) {
                    buf[n] = '\0';
                    if (sscanf(buf, "%d,%d", &zeta, &cpu) == 2) {
                        if (cpu >= 1 && cpu <= 100) taux_cpu[i] = cpu;

                        switch (zeta) {
                            case 0: mode[i] = MODE_STATIC; r[i] = v[i] = b[i] = 0; break;
                            case 1: mode[i] = MODE_IDLE; r[i] = r_def[i]; v[i] = v_def[i]; b[i] = b_def[i];break;
                            case 2: 
                                //mode[i] = default_mode[i]; 
                                r[i] = r_def[i]; v[i] = v_def[i]; b[i] = b_def[i];
                                if (cpu >= 0 && cpu < 20) {
                                    mode[i] = MODE_BREATHE;
                                }
                                else if (cpu < 40) {
                                    mode[i] =MODE_FILLREV;
                                }
                                else if (cpu < 60) {
                                    mode[i] =MODE_COMET;
                                }
                                else if (cpu < 80) {
                                    mode[i] =MODE_WAVE;
                                }
                                else{
                                    mode[i] =MODE_RAINBOW;
                                }

                            break;
                        }
                    }
                }
            }

            // CANAL PARALLÈLE
            if (parallel && p_fifo_fd[i] != -1) {
                char buf[128]; int zeta = -1, cpu = -1;
                int n = read(p_fifo_fd[i], buf, sizeof(buf) - 1);
                if (n > 0) {
                    buf[n] = '\0';
                    if (sscanf(buf, "%d,%d", &zeta, &cpu) == 2) {
                        if (cpu >= 1 && cpu <= 100) p_taux_cpu[i] = cpu;

                        switch (zeta) {
                            case 0: p_mode[i] = MODE_STATIC; p_r[i] = p_v[i] = p_b[i] = 0; break;
                            case 1: p_mode[i] = MODE_IDLE; p_r[i] = p_r_def[i]; p_v[i] = p_v_def[i]; p_b[i] = p_b_def[i];break;
                            case 2:  
                            //p_mode[i] = p_default_mode[i]; 
                            p_r[i] = p_r_def[i]; p_v[i] = p_v_def[i]; p_b[i] = p_b_def[i];
                                if (cpu >= 0 && cpu < 20) {
                                    p_mode[i] = MODE_BREATHE;
                                }
                                else if (cpu < 40) {
                                    p_mode[i] =MODE_FILLREV;
                                }
                                else if (cpu < 60) {
                                    p_mode[i] =MODE_COMET;
                                }
                                else if (cpu < 80) {
                                    p_mode[i] =MODE_WAVE;
                                }
                                else{
                                    p_mode[i] =MODE_RAINBOW;
                                }
                            break;
                        }
                    }
                }
            }
        }


        for (int i = 0; i < nb_sections; i++) {
            apply_mode(&ledstring.channel[0], &sections[i], mode[i], r[i], v[i], b[i], tick, taux_cpu[i]);
            if (parallel) apply_mode(&ledstring.channel[1], &p_sections[i], p_mode[i], p_r[i], p_v[i], p_b[i], tick, p_taux_cpu[i]);
        }

        ws2811_render(&ledstring);
        usleep(10000);
        tick = (tick + 1) % 1000000;
    }

    ws2811_fini(&ledstring);//Elle stoppe les modules matériels (DMA, PWM ou PCM) que la bibliothèque utilise pour générer le signal WS281x.
    return 0;
}
