#include <coleco.h>
#include <getput1.h>
#include "fonctions.h"


char odd;
char oddNmi;
//sprite_t bsprites[32];
// Pour collision dÚtection
byte cd[96];
byte p_source;
byte nb_source;


unsigned char isCollision(int x1,int y1,unsigned char h1,unsigned char l1,int x2,int y2,unsigned char h2,unsigned char l2)
{
	if(x1 > x2+l2) return 0;
	if(x1+l1 < x2) return 0;
	if(y1 > y2+h2) return 0;
	if(y1+h1 < y2) return 0;

	return 1;
}

void initSprites()
{
	unsigned char i;
	for (i=0;i<32;i++)
	{
		sprites[i].y = 204;
	}
}

char getFreeSprite()
{
	char i;

	odd+=8;
	if (odd>=32) odd = 0;

	for (i=odd;i<32;i++)
	if (sprites[i].y==204)
	return i;
	for (i=0;i<odd;i++)
	if (sprites[i].y==204)
	return i;

	return -1;
}

char getFreeConsecutiveSprite()
{
	char i;

	for (i=0;i<31;i++)
	if ((sprites[i].y==204) && (sprites[i+1].y==204) & ((i&1)==0))
    {
        return i;
    }

	return -1;
}


/*
void showSprites()
{
	put_vram (0x1b00,sprites,128);
}
*/
void showSprites()
{
    nb_source = 32-p_source;
    put_vram(sprgen,&sprites[p_source],nb_source<<2);
    if (p_source!=0)
        put_vram(sprgen+(nb_source<<2),sprites,p_source<<2);

    p_source +=4; // 4 = A priori valeur optimale
    if (p_source>32) p_source = 0;
}

/*
void showSprites()
{
	if (oddNmi) {put_vram (0x1b00,sprites,128); oddNmi=0;}
	else
	{
		memcpyb(bsprites,sprites+16,64);
		memcpyb(bsprites+16,sprites,64);
		put_vram (0x1b00,bsprites,128);
		oddNmi=1;
	}
}
*/
void LoadPatternAndshowAScreen(char* p,char *c,char *n,char show,unsigned dest)
{
			cls();
			load_patternrle(p);
			duplicate_pattern();
			rle2vram(c,coltab);
			if (show==1) rle2vram(n,dest);
}

unsigned char get_char_protected(unsigned char x,unsigned char y)
{
	if (x>31) return 107;
	if (y>23) return 107;
	return get_char(x,y);
}

void copyTileXY(unsigned char sx,unsigned char sy,unsigned char dx,unsigned char dy)
{
	unsigned char c[4];
	unsigned char i;

	for (i=0;i<4;i++)
	{
		get_vram(buffer+((sy+i)<<5)+sx,c,4);
		put_vram(chrtab+((dy+i)<<5)+dx,c,4);
	}
}

unsigned char peekVram(int offset)
{
	unsigned char c[1];
	get_vram(freevr+offset,c,1);
	return c[0];
}

void pokeVram(int offset,int v)
{
	unsigned char c[1];
	c[0]=v;
	put_vram(freevr+offset,c,1);
}

void setSprite(unsigned char ss,unsigned char xs,unsigned char ys,unsigned char cs,unsigned char ps)
{
	sprites[ss].x=xs;sprites[ss].y=ys;sprites[ss].colour=cs;sprites[ss].pattern=ps;
}

void moveSprite(unsigned char ss,unsigned char xs,unsigned char ys)
{
	sprites[ss].x=xs;sprites[ss].y=ys;
}

void initObstacle(void)
{
	byte i;
	// Mise Ó 0 des 12 octets du tableau des obstacles
	for (i=0;i<96;i++)
		*(cd+i)=0;
}

void setObstacle(unsigned char x,unsigned char y)
{
	int nocar;
	byte octet,bit;

	nocar = (y<<5)+x;
	octet = nocar>>3;
	bit = nocar - (octet <<3);
	*(cd+octet) = *(cd+octet) | (128>>bit);

}

byte isCarObstacle(unsigned char x,unsigned char y)
{
	int nocar;
	byte octet,bit;
	nocar = (y<<5)+x;
	octet = nocar>>3;
	bit =  nocar - (octet <<3);
	return ((*(cd+octet)) & (128>>bit));
}
