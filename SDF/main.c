#include <coleco.h>
#include <getput1.h>
#include "fonctions.h"

#define SPR_PLYR_H  0
#define SPR_PLYR_V  8
#define SPR_SHOT_H  16
#define SPR_SHOT_V  24

#define INACTIF     204
#define INVISIBLE   203

#define HORIZONTAL  0
#define VERTICAL    1

#define MAXBULLET   7
#define MAXALIEN    16
#define MAXCAPSULE  6
#define MAXANIM     6

#define FIRE_RATE   16

#define ANIM_EXPLOSION  0
#define ANIM_CAPSULE    1
#define ANIM_CRASH      2
#define ANIM_P10        3

#define WORKFLOW_TITLE      0
#define WORKFLOW_MENU       1
#define WORKFLOW_INGAME     2
#define WORKFLOW_LOOSE_LIFE 3
#define WORKFLOW_GAMEOVER   4

#define DUREE_ROUND 2700
#define TIMERANIMATION  16

#define true 1
#define false 0
#define NULL 0x0

#define LOOP_ON 1
#define LOOP_OFF 0

/*
        Durée d'un niveau : 1m30s --> 5400 ticks !
        ------------------------------------------

        - une capsule récupérée : 10 points
        - une capsule shootée : -20 points
        - un alien détruit : 5 points

        - un niveau terminé : 200 points

        - 5 collisions aliens --> 1 Vie en moins
        - 3 vies au départ
*/
extern const byte LEVEL_NAMERLE[];
extern const byte LEVEL_PATTERNRLE[];
extern const byte LEVEL_COLORRLE[];
extern const byte SPATTERNRLE[];

static const byte yellow_font[8]= { 0xa1,0xb1,0xe1,0xf1,0xe1,0xb1,0xa1,0xa1};
static const byte red_font[8]= { 0x61,0x61,0x81,0x91,0x91,0x81,0x61,0x61};

/* VARIABLES GESTION DE LA MUSIQUE */
byte loopMusic;

extern const void direct_sound(byte sound);

extern const byte music_nextLevel[];
extern const byte music_nextLevel2[];
extern const byte music_gameOver[];
extern const byte music_ingame1[];
extern const byte music_bonus[];

void startmusic(unsigned char *pDat,byte loop);

volatile char vola=0;
volatile unsigned char *pPlayList=NULL;		// audio playlist
volatile unsigned char *pPlayListForLoop=NULL;		// audio playlist
unsigned char nPlayDelay=1;
volatile unsigned char *pSfxList=NULL;		// sfx audio playlist
//const byte snd_table[]={0};
//byte *current_music;
/* FIN VARIABLES GESTION DE LA MUSIQUE */

typedef struct
{
    byte type;
    byte sprno;
} s_bullet;

typedef struct
{
    byte type;
    byte sprno;
    byte sprno2;
    byte speed;
} s_alien;

typedef struct
{
    byte type;
    byte sprno;
} s_capsule;

typedef struct
{
    byte type;
    byte sprno;
    byte animRate;
    byte animRate_cst;
} s_anim;

s_bullet bullet[MAXBULLET];
s_alien alien[MAXALIEN];
s_capsule capsule[MAXCAPSULE];
s_anim anim[MAXANIM];

// Shift sur 1 caractères
byte zoneCaractere[1<<3];
byte car2scroll;
byte canShoot;
byte ticks;
byte nmi_score=1;
int timer_ticks;        // 5400 --> 1m30s.

byte counter;
byte alienH_rate;
byte alienH_rate_counter;
byte alienV_rate;
byte alienV_rate_counter;
byte alienR_rate;


byte capsule_rate;
byte capsule_rate_counter;

unsigned int score;
unsigned int hiscore;
byte touche;
byte nblives;
byte workflow;
byte activateNmi;
byte currentLevel;
//byte levelStart=10;
byte timerAnimation=TIMERANIMATION;
byte alienBasePattern;

/* PROCEDURES MUSIQUE */
void stopmusic() {
	pPlayList=NULL;
	pPlayListForLoop=NULL;
	pSfxList=NULL;
	direct_sound(0x9F);
	direct_sound(0xBF);
	direct_sound(0xDF);
	direct_sound(0xFF);
}

void mutemusic()
{
	direct_sound(0x9F);
	direct_sound(0xBF);
	direct_sound(0xDF);
	direct_sound(0xFF);
}

void sound_nmi(void)
{
	unsigned char nDat;

	if (NULL != pPlayList) {
		nPlayDelay++;

		if (nPlayDelay == *(pPlayList))
		{
    		while (nPlayDelay==*(pPlayList))
			{
				nDat=*(++pPlayList);

				if (0 == nDat) {

					if (loopMusic==LOOP_OFF) {pPlayList=NULL;mutemusic();}
					else {pPlayList=pPlayListForLoop;nPlayDelay=(*pPlayListForLoop)-1;}
					break;
				}
				switch (nDat&0x90) {
					case 0x80:		// load sample
						//audport=nDat;
						//audport=*(++pPlayList);
						direct_sound(nDat);
						if ((nDat&0xe0) != 0xe0) direct_sound(*(++pPlayList));
						break;
					case 0x90:		// load volume
						//audport=nDat;
						direct_sound(nDat);
						break;
					default:		// don't know what this is, should abort
						pPlayList=NULL;
						break;
				}
				// point to next frame count
				pPlayList++;
			}
		}
	}

}



void startmusic(unsigned char *pDat,byte loop) {
	// order here is important
	loopMusic = loop;
	if (NULL != pPlayList) {
		stopmusic();
	}

	nPlayDelay=(*pDat)-1;
	pPlayList=pDat;
	pPlayListForLoop=pDat;

}


/* FIN PROCEDURES MUSIQUE */

void setYellowColor(void)
{
		put_vram_pattern (coltab+65*8,yellow_font,8,26);
		put_vram_pattern (coltab+2048+65*8,yellow_font,8,26);
		put_vram_pattern (coltab+4096+65*8,yellow_font,8,26);
}

void setRedNumber(void)
{
		put_vram_pattern (coltab+48*8,red_font,8,10);
		put_vram_pattern (coltab+2048+48*8,red_font,8,10);
		put_vram_pattern (coltab+4096+48*8,red_font,8,10);
}

char getFreeBullet(void)
{
    byte i;

    for (i=0; i<MAXBULLET; i++)
    {
        if (bullet[i].type==INACTIF)
        {
            return i;
        }
    }
    return -1;
}

char getFreeAlien(void)
{
    byte i;

    for (i=0; i<MAXALIEN; i++)
    {
        if (alien[i].type==INACTIF)
        {
            return i;
        }
    }
    return -1;
}

char getFreeCapsule(void)
{
    byte i;

    for (i=0; i<MAXCAPSULE; i++)
    {
        if (capsule[i].type==INACTIF)
        {
            return i;
        }
    }
    return -1;
}

char getFreeAnim(void)
{
    byte i;

    for (i=0; i<MAXANIM; i++)
    {
        if (anim[i].type==INACTIF)
        {
            return i;
        }
    }
    return -1;
}

// pc : premier caractère
// nb : nombre de caractères
// On part sur les ligne et sur le dernier caractère !
void shiftLeftBoucle(byte pc,byte nb)
{

    byte	lcc; // ligne du caractère courant
    byte	reste,restedernier;
    byte 	snb;
    byte *zc,*oldzc;

    for (lcc=0; lcc<8; lcc++)
    {
        zc=zoneCaractere+(pc<<3)+lcc;
        snb=nb;
        /* On traite spécifiquement le caractère le plus à gauche */
        restedernier = ((*(zc)) & 128) >>7;			// On sauvegarde le pixel à reclaquer sur le dernier caractère
        *(zc) = *(zc) << 1;		// On décale la ligne de pixel
        snb--; // 1 caractère de traité
        while (snb>0)	// Tant qu'il reste des caractères à traiter
        {
            oldzc = zc;
            zc+=8; 	// On se positionne sur la ligne courant du prochain caractère
            reste = ((*(zc)) & 128) >>7; // On récupère le pixel le plus à gauche de cette ligne
            *(oldzc) = *(oldzc) | reste; // On le claque sur la ligne courant du caractère précédent
            *(zc) = *(zc) << 1;		// On décale la ligne de pixel
            snb--;
        }
        *(zc) = *(zc) | restedernier;	// On reclaque le pixel récupéré sur le caractère de gauche, sur celui de droite
    }
}

void movePlayerShoot(void)
{
    int i;
    for (i=0; i<MAXBULLET; i++)
    {
        if (bullet[i].type==HORIZONTAL)
        {
            sprites[bullet[i].sprno].x-=4;
            if (sprites[bullet[i].sprno].x<=4)
            {
                sprites[bullet[i].sprno].y=INACTIF;
                sprites[bullet[i].sprno].x=INACTIF;
                bullet[i].type=INACTIF;
            }
        }
        else if (bullet[i].type==VERTICAL)
        {
            sprites[bullet[i].sprno].y-=4;
            if (sprites[bullet[i].sprno].y<=4)
            {
                sprites[bullet[i].sprno].y=INACTIF;
                sprites[bullet[i].sprno].x=INACTIF;
                bullet[i].type=INACTIF;
            }
        }
    }
}

void createAnim(byte type,byte x,byte y)
{
    char s,a;
    s = getFreeSprite();
    if (s==-1) return;
    a = getFreeAnim();
    if (a==-1) return;

    anim[a].type = type;
    anim[a].sprno = s;
    sprites[s].x = x;
    sprites[s].y = y;

    if (type==ANIM_EXPLOSION)
    {
        anim[a].animRate_cst=4;
        sprites[s].pattern = 59<<2;
        sprites[s].colour = 8;
    }
    else if (type==ANIM_CAPSULE)
    {
        anim[a].animRate_cst=4;
        sprites[s].pattern = 50<<2;
        sprites[s].colour = 8;
    }
    else if (type==ANIM_CRASH)
    {
        anim[a].animRate_cst=4;
        sprites[s].pattern = 56<<2;
        sprites[s].colour = 8;
    }
    else if (type==ANIM_P10)
    {
        anim[a].animRate_cst=16;
        sprites[s].pattern = 49<<2;
        sprites[s].colour = 8;
    }

    anim[a].animRate = anim[a].animRate_cst;
}

void moveAnim(void)
{
    int i;

    for (i=0; i<MAXANIM; i++)
    {
        if (anim[i].type!=INACTIF)
        {
            if (anim[i].animRate>0 )anim[i].animRate--;
            else
            {
                anim[i].animRate=anim[i].animRate_cst;
            }

            if (anim[i].type==ANIM_EXPLOSION)
            {
                if (sprites[anim[i].sprno].colour<15)
                    sprites[anim[i].sprno].colour++;
                else
                    sprites[anim[i].sprno].colour=2;

                if (anim[i].animRate==anim[i].animRate_cst)
                {
                    if (sprites[anim[i].sprno].pattern==63<<2)
                    {
                        sprites[anim[i].sprno].y = INACTIF;
                        anim[i].type = INACTIF;
                    }
                    else
                    {
                        sprites[anim[i].sprno].pattern+=4;
                    }
                }
            }
            else if (anim[i].type==ANIM_CAPSULE)
            {
                if (sprites[anim[i].sprno].colour<15)
                    sprites[anim[i].sprno].colour++;
                else
                    sprites[anim[i].sprno].colour=2;

                if (anim[i].animRate==anim[i].animRate_cst)
                {
                    if (sprites[anim[i].sprno].pattern==55<<2)
                    {
                        sprites[anim[i].sprno].y = INACTIF;
                        anim[i].type = INACTIF;
                    }
                    else
                    {
                        sprites[anim[i].sprno].pattern+=4;
                    }
                }
            }
            else if (anim[i].type==ANIM_CRASH)
            {
                if (sprites[anim[i].sprno].colour<15)
                    sprites[anim[i].sprno].colour++;
                else
                    sprites[anim[i].sprno].colour=2;

                if (anim[i].animRate==anim[i].animRate_cst)
                {
                    if (sprites[anim[i].sprno].pattern==56<<2)
                    {
                        sprites[anim[i].sprno].y = INACTIF;
                        anim[i].type = INACTIF;
                    }
                    else
                    {
                        sprites[anim[i].sprno].pattern+=4;
                    }
                }
            }
            else if (anim[i].type==ANIM_P10)
            {
                if (sprites[anim[i].sprno].colour<15)
                    sprites[anim[i].sprno].colour++;
                else
                    sprites[anim[i].sprno].colour=2;

                if (anim[i].animRate==anim[i].animRate_cst)
                {
                    sprites[anim[i].sprno].y = INACTIF;
                    anim[i].type = INACTIF;
                }
            }
        }
    }
}
void createAlien(void)
{
    int r,x,y,t;
    char s,s2,a;
    byte change_color;
    change_color = rnd_byte(0,100);

    alienBasePattern = 13<<2;
    if ((currentLevel&3)==0) alienBasePattern = 4<<2;
    else if ((currentLevel&3)==1) alienBasePattern = 7<<2;
    else if ((currentLevel&3)==2) alienBasePattern = 10<<2;


    r = rnd_byte(0,100);
    if (r<50) // horizontal
    {
        x=0;
        y = rnd_byte(0,152);
        t = HORIZONTAL;
    }
    else   // vertical
    {
        x = rnd_byte(0,208);
        y = 0;
        t = VERTICAL;
    }

    s=getFreeConsecutiveSprite();
    if (s==-1) return;
    s2=s+1;
    a=getFreeAlien();
    if (a==-1) return;

    alien[a].sprno = s;
    alien[a].sprno2 = s2;
    alien[a].type = t;
    alien[a].speed = 1;
    sprites[s].x = x;
    sprites[s].y = y;
    sprites[s].pattern = alienBasePattern;

    if (change_color<(currentLevel<<1)) sprites[s].colour = 2;
    else
        sprites[s].colour = 7;

    sprites[s2].x = x;
    sprites[s2].y = y;

    if (alienBasePattern==4<<2)    sprites[s2].pattern = 32<<2;
    else if (alienBasePattern==7<<2)    sprites[s2].pattern = 33<<2;
    else if (alienBasePattern==10<<2)    sprites[s2].pattern = 34<<2;
    else if (alienBasePattern==13<<2)    sprites[s2].pattern = 35<<2;

    sprites[s2].colour = 1;

    r = rnd_byte(0,100);
    if (r<currentLevel) alien[a].speed = 2;
}


void createCapsuleH(byte xs,byte ys)
{
    int x,y,t;
    char s,a;

    x = xs;
    y = ys;
    t = HORIZONTAL;

    s=getFreeSprite();
    if (s==-1) return;
    a=getFreeCapsule();
    if (a==-1) return;

    capsule[a].sprno = s;
    capsule[a].type = t;
    sprites[s].x = x;
    sprites[s].y = y;
    sprites[s].pattern = 50<<2;
    sprites[s].colour = 7;
}

void createCapsuleV(byte xs,byte ys)
{
    int x,y,t;
    char s,a;

    x = xs;
    y = ys;
    t = VERTICAL;

    s=getFreeSprite();
    if (s==-1) return;
    a=getFreeCapsule();
    if (a==-1) return;

    capsule[a].sprno = s;
    capsule[a].type = t;
    sprites[s].x = x;
    sprites[s].y = y;
    sprites[s].pattern = 50<<2;
    sprites[s].colour = 7;
}

void createCapsule(void)
{
    int r,x,y,t;
    char s,a;
    r = rnd_byte(0,100);
    if (r<50) // horizontal
    {
        x=0;
        y = rnd_byte(0,152);
        t = HORIZONTAL;
    }
    else   // vertical
    {
        x = rnd_byte(0,208);
        y = 0;
        t = VERTICAL;
    }

    s=getFreeSprite();
    if (s==-1) return;
    a=getFreeCapsule();
    if (a==-1) return;

    capsule[a].sprno = s;
    capsule[a].type = t;
    sprites[s].x = x;
    sprites[s].y = y;
    sprites[s].pattern = 50<<2;
    sprites[s].colour = 7;
}

void createAlienV(void)
{
    int x,y,t;
    byte r;
    char s,s2,a;
    byte change_color;
    change_color = rnd_byte(0,100);


    alienBasePattern = 13<<2;
    if ((currentLevel&3)==0) alienBasePattern = 4<<2;
    else if ((currentLevel&3)==1) alienBasePattern = 7<<2;
    else if ((currentLevel&3)==2) alienBasePattern = 10<<2;

    x = rnd_byte(0,208);
    y = 0;

    t = VERTICAL;

    s=getFreeConsecutiveSprite();
    if (s==-1) return;
    s2=s+1;
    a=getFreeAlien();
    if (a==-1) return;

    alien[a].sprno = s;
    alien[a].sprno2 = s2;
    alien[a].type = t;
    alien[a].speed = 1;

    sprites[s].x = x;
    sprites[s].y = y;
    sprites[s].pattern = alienBasePattern;
    if (change_color<(currentLevel<<1)) sprites[s].colour = 2;
    else
        sprites[s].colour = 7;


    sprites[s2].x = x;
    sprites[s2].y = y;
    if (alienBasePattern==4<<2)    sprites[s2].pattern = 32<<2;
    else if (alienBasePattern==7<<2)    sprites[s2].pattern = 33<<2;
    else if (alienBasePattern==10<<2)    sprites[s2].pattern = 34<<2;
    else if (alienBasePattern==13<<2)    sprites[s2].pattern = 35<<2;
    sprites[s2].colour = 1;

    r = rnd_byte(0,100);
    if (r<currentLevel) alien[a].speed = 2;
}

void createAlienH(void)
{
    int x,y,t;
    byte r;
    char s,s2,a;
    byte change_color;
    change_color = rnd_byte(0,100);


    alienBasePattern = 13<<2;
    if ((currentLevel&3)==0) alienBasePattern = 4<<2;
    else if ((currentLevel&3)==1) alienBasePattern = 7<<2;
    else if ((currentLevel&3)==2) alienBasePattern = 10<<2;

    x=0;
    y = rnd_byte(0,152);

    t = HORIZONTAL;

    s=getFreeConsecutiveSprite();
    if (s==-1) return;
    s2=s+1;
    a=getFreeAlien();
    if (a==-1) return;

    alien[a].sprno = s;
    alien[a].sprno2 = s2;
    alien[a].type = t;
    alien[a].speed = 1;
    sprites[s].x = x;
    sprites[s].y = y;
    sprites[s].pattern = alienBasePattern;

        if (change_color<(currentLevel<<1)) sprites[s].colour = 2;
    else
        sprites[s].colour = 7;


    sprites[s2].x = x;
    sprites[s2].y = y;
    if (alienBasePattern==4<<2)    sprites[s2].pattern = 32<<2;
    else if (alienBasePattern==7<<2)    sprites[s2].pattern = 33<<2;
    else if (alienBasePattern==10<<2)    sprites[s2].pattern = 34<<2;
    else if (alienBasePattern==13<<2)    sprites[s2].pattern = 35<<2;
    sprites[s2].colour = 1;

    r = rnd_byte(0,100);
    if (r<currentLevel) alien[a].speed = 2;
}


void controlPlayer()
{
    char s1,s2;
    char b1,b2;
    char tmp_color;
    tmp_color=0;

    if ((joypad_1&LEFT) && (sprites[SPR_PLYR_H].x>0))
    {
        sprites[SPR_PLYR_H].x-=2;
    }
    else if ((joypad_1&RIGHT) && (sprites[SPR_PLYR_H].x<224))
    {
        sprites[SPR_PLYR_H].x+=2;
    }

    if ((joypad_1&UP) && (sprites[SPR_PLYR_V].y>0))
    {
        sprites[SPR_PLYR_V].y-=2;
    }
    else if ((joypad_1&DOWN) && (sprites[SPR_PLYR_V].y<158))
    {
        sprites[SPR_PLYR_V].y+=2;
    }

    // Le tir horizontal est tiré par le sprite vertical !!

    if ((joypad_1&FIRE1) || (joypad_1&FIRE2))
    {
        if (canShoot==0)
        {
            if (joypad_1&FIRE2) tmp_color = 7;
            if (joypad_1&FIRE1) tmp_color = 2;

            canShoot = FIRE_RATE;
            // Tentative de création d'un bullet horizontal
            s1 = getFreeSprite();
            if (s1==-1) return;
            b1 = getFreeBullet();
            if (b1==-1) return;

            bullet[b1].type = HORIZONTAL;
            bullet[b1].sprno = s1;
            sprites[s1].x = sprites[SPR_PLYR_V].x;
            sprites[s1].y = sprites[SPR_PLYR_V].y;
            sprites[s1].pattern = 2<<2;
            sprites[s1].colour = tmp_color;

            s2 = getFreeSprite();
            if (s2==-1) return;
            b2 = getFreeBullet();
            if (b2==-1) return;

            bullet[b2].type = VERTICAL;
            bullet[b2].sprno = s2;
            sprites[s2].x = sprites[SPR_PLYR_H].x;
            sprites[s2].y = sprites[SPR_PLYR_H].y;
            sprites[s2].pattern = 3<<2;
            sprites[s2].colour = tmp_color;
        }
    }
}

void moveAlien()
{
    byte i;

    if (timerAnimation>0) timerAnimation--;

    for (i=0; i<MAXALIEN; i++)
    {
        if (alien[i].type!=INACTIF)
        {
            if (timerAnimation==0)
            {
                sprites[alien[i].sprno].pattern+=4;
                if (sprites[alien[i].sprno].pattern>(alienBasePattern+8)) sprites[alien[i].sprno].pattern = alienBasePattern;
            }
        }
        if (alien[i].type==HORIZONTAL)
        {
            sprites[alien[i].sprno].x+=alien[i].speed;
            sprites[alien[i].sprno2].x+=alien[i].speed;
            if ((sprites[alien[i].sprno].x)>224)
            {
                createAnim(ANIM_CRASH,sprites[alien[i].sprno].x,sprites[alien[i].sprno].y);
                delay(1);
                disable_nmi();
                if (get_char_protected(30,(sprites[alien[i].sprno].y+8)>>3)==car2scroll)
                {
                    workflow = WORKFLOW_GAMEOVER;
                    sprites[alien[i].sprno].x = 240;
                    sprites[alien[i].sprno2].x = 240;
                    delay(1);
                    return;
                }

                put_char(30,(sprites[alien[i].sprno].y+8)>>3,car2scroll);
                put_char(31,(sprites[alien[i].sprno].y+8)>>3,car2scroll);
                if (score>5) {score-=5;nmi_score=1;} else {score=0;nmi_score=1;}
                enable_nmi();
                sprites[alien[i].sprno].y = INACTIF;
                sprites[alien[i].sprno2].y = INACTIF;
                alien[i].type = INACTIF;
            }
        }
        else if (alien[i].type==VERTICAL)
        {
            sprites[alien[i].sprno].y+=alien[i].speed;
            sprites[alien[i].sprno2].y+=alien[i].speed;
            if ((sprites[alien[i].sprno].y)>160)
            {
                createAnim(ANIM_CRASH,sprites[alien[i].sprno].x,sprites[alien[i].sprno].y);
                delay(1);
                disable_nmi();
                if (get_char_protected((sprites[alien[i].sprno].x+8)>>3,22)==car2scroll)
                {
                    workflow = WORKFLOW_GAMEOVER;
                    sprites[alien[i].sprno].y=176;
                    sprites[alien[i].sprno2].y=176;
                    delay(1);
                    return;
                }
                put_char((sprites[alien[i].sprno].x+8)>>3,22,car2scroll);
                put_char((sprites[alien[i].sprno].x+8)>>3,23,car2scroll);
                if (score>5) {score-=5;nmi_score=1;} else {score=0;nmi_score=1;}
                enable_nmi();
                sprites[alien[i].sprno].y = INACTIF;
                sprites[alien[i].sprno2].y = INACTIF;
                alien[i].type = INACTIF;
            }
        }
    }

    if (timerAnimation==0) timerAnimation = TIMERANIMATION;
}

void moveCapsule()
{
    byte i;

    for (i=0; i<MAXCAPSULE; i++)
    {
        if (capsule[i].type==HORIZONTAL)
        {
            if ((sprites[capsule[i].sprno].colour)<15) sprites[capsule[i].sprno].colour++;
            else sprites[capsule[i].sprno].colour=2;
            sprites[capsule[i].sprno].x+=1;
            if ((sprites[capsule[i].sprno].x)>224)
            {
                createAnim(ANIM_P10,sprites[capsule[i].sprno].x,sprites[capsule[i].sprno].y);
                delay(1);
                disable_nmi();
                put_char(30,(sprites[capsule[i].sprno].y+8)>>3,252);
                put_char(31,(sprites[capsule[i].sprno].y+8)>>3,253);
                enable_nmi();
                sprites[capsule[i].sprno].y = INACTIF;
                capsule[i].type = INACTIF;
                score+=10;
                nmi_score=1;
            }
        }
        else if (capsule[i].type==VERTICAL)
        {
            if ((sprites[capsule[i].sprno].colour)<15) sprites[capsule[i].sprno].colour++;
            else sprites[capsule[i].sprno].colour=2;
            sprites[capsule[i].sprno].y+=1;
            if ((sprites[capsule[i].sprno].y)>160)
            {
                createAnim(ANIM_P10,sprites[capsule[i].sprno].x,sprites[capsule[i].sprno].y);
                delay(1);
                disable_nmi();
                put_char((sprites[capsule[i].sprno].x)>>3,22,254);
                put_char((sprites[capsule[i].sprno].x)>>3,23,255);
                put_char((sprites[capsule[i].sprno].x+8)>>3,22,254);
                put_char((sprites[capsule[i].sprno].x+8)>>3,23,255);
                enable_nmi();
                sprites[capsule[i].sprno].y = INACTIF;
                capsule[i].type = INACTIF;
                score+=10;
                nmi_score=1;
            }
        }
    }
}

void moveCapsuleFast()
{
    byte i;

    for (i=0; i<MAXCAPSULE; i++)
    {
        if (capsule[i].type==HORIZONTAL)
        {
            if ((sprites[capsule[i].sprno].colour)<15) sprites[capsule[i].sprno].colour++;
            else sprites[capsule[i].sprno].colour=2;
            sprites[capsule[i].sprno].x+=2;
            if ((sprites[capsule[i].sprno].x)>224)
            {
                createAnim(ANIM_P10,sprites[capsule[i].sprno].x,sprites[capsule[i].sprno].y);
                delay(1);
                disable_nmi();
                put_char(30,(sprites[capsule[i].sprno].y+8)>>3,252);
                put_char(31,(sprites[capsule[i].sprno].y+8)>>3,253);
                enable_nmi();
                sprites[capsule[i].sprno].y = INACTIF;
                capsule[i].type = INACTIF;
                score+=10;
                nmi_score=1;
            }
        }
        else if (capsule[i].type==VERTICAL)
        {
            if ((sprites[capsule[i].sprno].colour)<15) sprites[capsule[i].sprno].colour++;
            else sprites[capsule[i].sprno].colour=2;
            sprites[capsule[i].sprno].y+=2;
            if ((sprites[capsule[i].sprno].y)>160)
            {
                createAnim(ANIM_P10,sprites[capsule[i].sprno].x,sprites[capsule[i].sprno].y);
                delay(1);
                disable_nmi();
                put_char((sprites[capsule[i].sprno].x)>>3,22,254);
                put_char((sprites[capsule[i].sprno].x)>>3,23,255);
                put_char((sprites[capsule[i].sprno].x+8)>>3,22,254);
                put_char((sprites[capsule[i].sprno].x+8)>>3,23,255);
                enable_nmi();
                sprites[capsule[i].sprno].y = INACTIF;
                capsule[i].type = INACTIF;
                score+=10;
                nmi_score=1;
            }
        }
    }
}

void checkCollisionShootAlien(void)
{
    int a,b,r;
    int xb,yb;
    int xa,ya;

    if ((vdp_status&32)==0) return;

    for (a=0; a<MAXALIEN; a++)
    {
        if (alien[a].type!=INACTIF)
        {
            xa = sprites[alien[a].sprno].x;
            ya = sprites[alien[a].sprno].y;
            for (b=0; b<MAXBULLET; b++)
            {
                if (bullet[b].type!=INACTIF)
                {
                    r=0;
                    xb = sprites[bullet[b].sprno].x;
                    yb = sprites[bullet[b].sprno].y;

                    if (bullet[b].type==HORIZONTAL)
                    {
                        r = isCollision(xb,yb+7,2,16,xa,ya,16,16);
                    }
                    else if (bullet[b].type==VERTICAL)
                    {
                        r = isCollision(xb+7,yb,16,2,xa,ya,16,16);
                    }

                    if (r==1)
                    {

                            score+=5;
                            nmi_score=1;
                            createAnim(ANIM_EXPLOSION,sprites[alien[a].sprno].x,sprites[alien[a].sprno].y);
                            // Destruction bullet
                            sprites[bullet[b].sprno].y = INACTIF;
                            bullet[b].type = INACTIF;
                            if (sprites[bullet[b].sprno].colour==sprites[alien[a].sprno].colour)
                            {
                                // Destruction alien
                                sprites[alien[a].sprno].y = INACTIF;
                                sprites[alien[a].sprno2].y = INACTIF;
                                alien[a].type = INACTIF;
                            }
                    }
                }
            }
        }
    }
}

void checkCollisionShootCapsule(void)
{
    int a,b,r;
    int xb,yb;
    int xa,ya;

    if ((vdp_status&32)==0) return;

    for (a=0; a<MAXCAPSULE; a++)
    {
        if (capsule[a].type!=INACTIF)
        {
            xa = sprites[capsule[a].sprno].x;
            ya = sprites[capsule[a].sprno].y;
            for (b=0; b<MAXBULLET; b++)
            {
                if (bullet[b].type!=INACTIF)
                {
                    r=0;
                    xb = sprites[bullet[b].sprno].x;
                    yb = sprites[bullet[b].sprno].y;

                    if (bullet[b].type==HORIZONTAL)
                    {
                        r = isCollision(xb,yb+7,2,16,xa,ya,16,16);
                    }
                    else if (bullet[b].type==VERTICAL)
                    {
                        r = isCollision(xb+7,yb,16,2,xa,ya,16,16);
                    }

                    if (r==1)
                    {
                        createAnim(ANIM_CAPSULE,sprites[capsule[a].sprno].x,sprites[capsule[a].sprno].y);
                        // Destruction bullet
                        sprites[bullet[b].sprno].y = INACTIF;
                        bullet[b].type = INACTIF;
                        // Destruction capsule
                        sprites[capsule[a].sprno].y = INACTIF;
                        capsule[a].type = INACTIF;

                        if (score>20) {score-=20;nmi_score=1;}
                        else {score=0;nmi_score=1;}
                    }
                }
            }
        }
    }
}

void bonusLevel()
{
    byte i,x,y;
    char sens_h,sens_v;
    byte canShoot,rebound;

    rebound=20;
    canShoot = 0;

    sens_h = 4;
    sens_v = 4;



    initSprites();
    for (i=0; i<MAXBULLET; i++) bullet[i].type = INACTIF;
    for (i=0; i<MAXALIEN; i++) alien[i].type = INACTIF;
    for (i=0; i<MAXCAPSULE; i++) capsule[i].type = INACTIF;
    for (i=0; i<MAXANIM; i++) anim[i].type = INACTIF;

    car2scroll = currentLevel%8;

    for (x=0; x<32; x++)
    {
        for (y=0; y<24; y++)
        {
            if (get_char_protected(x,y)<10)
                put_char(x,y,car2scroll);
        }
    }

    get_vram(chrgen+(car2scroll<<3)+4096,zoneCaractere,1<<3);

    sprites[SPR_PLYR_H].x = 16;
    sprites[SPR_PLYR_H].y = 16;
    sprites[SPR_PLYR_H].pattern = 21<<2;
    sprites[SPR_PLYR_H].colour = 15;

    sprites[SPR_PLYR_V].x = 16;
    sprites[SPR_PLYR_V].y = 16;
    sprites[SPR_PLYR_V].pattern = 20<<2;
    sprites[SPR_PLYR_V].colour = 15;

    SCREEN_ON
    startmusic(music_bonus,LOOP_ON);
    while(rebound>0)
    {
        if (canShoot>0) canShoot--;

        if ((sprites[SPR_PLYR_H].x>(240-16)) || (sprites[SPR_PLYR_H].x<4)) {sens_h = sens_h *-1;rebound--;}
        sprites[SPR_PLYR_H].x+=sens_h;

        if ((sprites[SPR_PLYR_V].y>(192-32)) || (sprites[SPR_PLYR_V].y<4)) sens_v = sens_v *-1;
        sprites[SPR_PLYR_V].y+=sens_v;

        if ((joypad_1&FIRE1) || (joypad_1&FIRE2))
        {
            if ((canShoot==0) && (rebound>2))
            {
                canShoot=30;
                createCapsuleH(sprites[SPR_PLYR_V].x,sprites[SPR_PLYR_V].y);
                createCapsuleV(sprites[SPR_PLYR_H].x,sprites[SPR_PLYR_H].y);
            }
        }
        moveCapsuleFast();
        moveAnim();
        delay(1);
        center_string(1,str(rebound));
    }
    stop_music();
    SCREEN_OFF
    nmi_score=1;
}

void initLevel(byte level)
{
    int x,y,i;

    timer_ticks=DUREE_ROUND;
    if (level>30) level = 30;
    initSprites();
    car2scroll = currentLevel%8;

    for (x=0; x<32; x++)
    {
        for (y=0; y<24; y++)
        {
            if (get_char_protected(x,y)<10)
                put_char(x,y,car2scroll);
        }
    }

    get_vram(chrgen+(car2scroll<<3)+4096,zoneCaractere,1<<3);

    sprites[SPR_PLYR_H].x = 128;
    sprites[SPR_PLYR_H].y = 159;
    sprites[SPR_PLYR_H].pattern = 1<<2;
    sprites[SPR_PLYR_H].colour = 15;

    sprites[SPR_PLYR_V].x = 224;
    sprites[SPR_PLYR_V].y = 80;
    sprites[SPR_PLYR_V].pattern = 0<<2;
    sprites[SPR_PLYR_V].colour = 15;

    for (i=0; i<MAXBULLET; i++) bullet[i].type = INACTIF;
    for (i=0; i<MAXALIEN; i++) alien[i].type = INACTIF;
    for (i=0; i<MAXCAPSULE; i++) capsule[i].type = INACTIF;
    for (i=0; i<MAXANIM; i++) anim[i].type = INACTIF;

    alienH_rate_counter = 235 - (level<<3);
    alienV_rate_counter = 255 - (level<<3);
    alienR_rate = 255 - (level<<3);
    capsule_rate_counter = 215 - (level<<2);

    alienH_rate = alienH_rate_counter;
    alienV_rate = alienV_rate_counter;
    capsule_rate = capsule_rate_counter;
}

void menu(void)
{
    byte sortie;

    SCREEN_OFF
    center_string(12-6,"CHOOSE DIFFICULTY :");

    center_string(14-6,"1 - EASY");
    center_string(16-6,"2 - NORMAL");
    center_string(18-6,"3 - HARD");

    setYellowColor();
    setRedNumber();
    SCREEN_ON

    sortie=0;
    while (sortie==0)
    {
         if (keypad_1==1)
         {
             currentLevel = 0;
             sortie = 1;
         } else if (keypad_1==2)
         {
             currentLevel = 10;
             sortie = 1;
          } else if (keypad_1==3)
          {
             currentLevel = 20;
             sortie = 1;
          }
    }


}
void endRound(void)
{
    byte i;

    // on détruit tout
    for (i=0; i<MAXALIEN; i++)
    {
        if (alien[i].type!=INACTIF)
        {
            createAnim(ANIM_EXPLOSION,sprites[alien[i].sprno].x,sprites[alien[i].sprno].y);
            alien[i].type=INACTIF;
            sprites[alien[i].sprno].y=INACTIF;
            sprites[alien[i].sprno2].y=INACTIF;
        }
    }

    for (i=0; i<MAXCAPSULE; i++)
    {
        if (capsule[i].type!=INACTIF)
        {
            createAnim(ANIM_EXPLOSION,sprites[capsule[i].sprno].x,sprites[capsule[i].sprno].y);
            capsule[i].type=INACTIF;
            sprites[capsule[i].sprno].y=INACTIF;
        }
    }

    for (i=0; i<64; i++)
    {
        moveAnim();
        delay(1);
    }


}
void main(void)
{
    byte t=0;
    byte x,y;
    workflow = WORKFLOW_TITLE;
    while(1)
    {
        switch(workflow)
        {
        case WORKFLOW_TITLE:
            activateNmi=0;
            car2scroll = 2;
            canShoot = 0;
            score = 0;
            counter = 0;
            touche=0;
            nblives=3;
            SCREEN_OFF
            initSprites();
            showSprites();
            screen_mode_2_text();
            rle2vram(SPATTERNRLE,sprtab);
            LoadPatternAndshowAScreen(LEVEL_PATTERNRLE,LEVEL_COLORRLE,LEVEL_NAMERLE,1,chrtab);
            setYellowColor();
            setRedNumber();
            center_string(5,"STARSHIP DEFENCE FORCE");
            center_string(9,"HISCORE");
            center_string(11,str(hiscore));
			center_string(18,"MICHEL LOUVET");
			center_string(19,"FOR");
			center_string(20,"COLLECTORVISION 2015");
            SCREEN_ON
            pause();
            workflow = WORKFLOW_MENU;
            break;
        case WORKFLOW_MENU:
            activateNmi=0;
            SCREEN_OFF
            screen_mode_2_text();
            rle2vram(SPATTERNRLE,sprtab);
            LoadPatternAndshowAScreen(LEVEL_PATTERNRLE,LEVEL_COLORRLE,LEVEL_NAMERLE,1,chrtab);
            SCREEN_ON
            menu();
            activateNmi=1;
            nmi_score=1;
            SCREEN_OFF
            LoadPatternAndshowAScreen(LEVEL_PATTERNRLE,LEVEL_COLORRLE,LEVEL_NAMERLE,1,chrtab);
            for (x=0; x<30; x++)
            {
                for (y=0; y<22; y++)
                {
                    put_char(x,y,0);
                }
            }

            initLevel(currentLevel);
            SCREEN_ON
            workflow = WORKFLOW_INGAME;
            startmusic(music_ingame1,LOOP_ON);
            break;
        case WORKFLOW_INGAME:
            ticks=0;
            controlPlayer();
            movePlayerShoot();
            moveAlien();
            moveCapsule();
            if ((ticks&1)==0)
            {
                checkCollisionShootAlien();
                checkCollisionShootCapsule();
            }
            moveAnim();

            if (canShoot>0) canShoot--;

            if (alienH_rate>0)
            {
                alienH_rate --;
            }
            else
            {
                alienH_rate = alienH_rate_counter;
                // Créer un nouvel alien !
                createAlienH();
            }

            if (alienV_rate>0)
            {
                alienV_rate --;
            }
            else
            {
                alienV_rate = alienV_rate_counter;
                // Créer un nouvel alien !
                createAlienV();
            }

            if (alienR_rate>0)
            {
                alienR_rate --;
            }
            else
            {
                if (currentLevel<30)
                    t = 250 - (currentLevel<<3);
                else
                    t = 250 - (30<<3);

                //alienR_rate = rnd_byte(t,255);
                alienR_rate = t;
                // Créer un nouvel alien aléatoire !
                createAlien();
            }

            if (capsule_rate>0)
            {
                capsule_rate--;
            }
            else
            {
                capsule_rate = capsule_rate_counter;
                // Créer une nouvelle capsule
                createCapsule();
            }

            if (keypad_1==5) timer_ticks=0;

            if (timer_ticks==0)
            {
                if ((currentLevel&1)==0) startmusic(music_nextLevel2,LOOP_OFF);
                else startmusic(music_nextLevel,LOOP_OFF);
                timer_ticks=DUREE_ROUND;
                endRound();
                currentLevel++;
                delay(200);
                SCREEN_OFF
                if ((currentLevel&3)==0) bonusLevel();
                initLevel(currentLevel);
                SCREEN_ON
                startmusic(music_ingame1,LOOP_ON);
            }
            if (ticks==0) delay(1);
            break;
        case WORKFLOW_GAMEOVER:
            startmusic(music_gameOver,LOOP_OFF);
            delay(1);
            center_string(16,"SECURITY BREACH");
            center_string(18,"GAME OVER");
            if (score>hiscore) hiscore=score;
            delay(50);
            pause();
            workflow = WORKFLOW_TITLE;
            break;

        }
    }
}

void nmi(void)
{
    sound_nmi();
    if (activateNmi==1)
    {
        ++ticks;
        showSprites();
        if (timer_ticks>0) timer_ticks--;
        if (nmi_score==1) {nmi_score=0;center_string(1,str(score));}

        shiftLeftBoucle(0,1);
        put_vram(chrgen+(car2scroll<<3)+0,zoneCaractere,1<<3);
        put_vram(chrgen+(car2scroll<<3)+2048,zoneCaractere,1<<3);
        put_vram(chrgen+(car2scroll<<3)+4096,zoneCaractere,1<<3);
    }



}
