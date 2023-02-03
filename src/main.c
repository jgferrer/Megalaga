#include <genesis.h>
#include <resources.h>

#define LEFT_EDGE 0
#define RIGHT_EDGE 320
#define ANIM_STRAIGHT 0
#define ANIM_MOVE 1

typedef struct {
	int x;
	int y;
	int w;
	int h;
	int velx;
	int vely;
	int health;
	Sprite* sprite;
	char name[6];
} Entity;

Entity player = {0, 0, 16, 16, 0, 0, 0, NULL, "PLAYER" };
#define MAX_ENEMIES 6
Entity enemies[MAX_ENEMIES];
u16 enemiesLeft = 0;
u16 bulletsOnScreen = 0;

#define MAX_BULLETS	3
Entity bullets[MAX_BULLETS];

void killEntity(Entity* e);
void reviveEntity(Entity* e);
void positionEnemies();
void positionPlayer();
void myJoyHandler( u16 joy, u16 changed, u16 state);
void shootBullet();
void positionBullets();

int main(bool hard)
{
    JOY_init();
    JOY_setEventHandler(&myJoyHandler);
    SYS_disableInts();
    VDP_loadTileSet(background.tileset,1,DMA);
    PAL_setPalette(PAL1, background.palette->data,DMA);
    PAL_setPalette(PAL2, background.palette->data,DMA);
    int i = 0;
    int thex = 0;
    int they = 0;
    int val = 1;
    for( i=0; i < 1280; i++){
        thex = i % 40;
        they = i / 40;
        val = (random() % (10-1+1))+1;
        if(val > 3) val = 1;
        VDP_setTileMapXY(BG_B,TILE_ATTR_FULL(PAL1,0,0,0,val), thex, they );
    }
    int offset = 0;
    VDP_setScrollingMode(HSCROLL_PLANE,VSCROLL_PLANE);
    SYS_enableInts();
    
    SPR_init();
    /*Add the player*/
    player.x = 152;
    player.y = 192;
    player.health = 1;
    player.sprite = SPR_addSprite(&ship,player.x,player.y,TILE_ATTR(PAL1,0,FALSE,FALSE));
    SPR_update();
    /*Create all enemy sprites*/
    Entity* e = enemies;
    for(i = 0; i < MAX_ENEMIES; i++){
        e->x = i*32;
        e->y = 32;
        e->w = 16;
        e->h = 16;
        e->velx = 1;
        e->health = 1;
        e->sprite = SPR_addSprite(&ship,e->x,e->y,TILE_ATTR(PAL2,0,TRUE,FALSE));
        sprintf(e->name, "En%d",i);
        enemiesLeft++;
        e++;
    }
    PAL_setColor(34,RGB24_TO_VDPCOLOR(0x0078f8));

    /*Create all bullet sprites*/
    Entity* b = bullets;
    for(i = 0; i < MAX_BULLETS; i++){
        b->x = 0;
        b->y = -10;
        b->w = 8;
        b->h = 8;
        b->sprite = SPR_addSprite(&bullet,bullets[0].x,bullets[0].y,TILE_ATTR(PAL1,0,FALSE,FALSE));
        sprintf(b->name, "Bu%d",i);
        b++;
    }

    while(1)
    {
        VDP_setVerticalScroll(BG_B,offset -= 2);
        if(offset <= -256) offset = 0;
        positionPlayer();
        positionBullets();
        positionEnemies();
        SPR_update();
        SYS_doVBlankProcess();
    }
    return (0);
}

void killEntity(Entity* e){
	e->health = 0;
	SPR_setVisibility(e->sprite,HIDDEN);
}

void reviveEntity(Entity* e){
	e->health = 1;
	SPR_setVisibility(e->sprite,VISIBLE);
}

void positionEnemies(){
    u16 i = 0;
    for(i = 0; i < MAX_ENEMIES; i++){
        Entity* e = &enemies[i];
        if(e->health > 0){
            e->x += e->velx;
            SPR_setPosition(e->sprite,e->x,e->y);
            if( (e->x+e->w) > RIGHT_EDGE){
                e->velx = -1;
            }
            else if(e->x < LEFT_EDGE){
                e->velx = 1;
            }
        }
    }
}

void positionPlayer(){
    /*Add the player's velocity to its position*/
    player.x += player.velx;

    /*Keep the player within the bounds of the screen*/
    if(player.x < LEFT_EDGE) player.x = LEFT_EDGE;
    if(player.x + player.w > RIGHT_EDGE) player.x = RIGHT_EDGE - player.w;

    /*Let the Sprite engine position the sprite*/
    SPR_setPosition(player.sprite,player.x,player.y);
}

void myJoyHandler( u16 joy, u16 changed, u16 state)
{
    if (joy == JOY_1)
    {
        if (state & BUTTON_RIGHT)
	    {
            player.velx = 2;
            SPR_setAnim(player.sprite,ANIM_MOVE);
            SPR_setHFlip(player.sprite,TRUE);
        }
        else if (state & BUTTON_LEFT)
        {
            player.velx = -2;
            SPR_setAnim(player.sprite,ANIM_MOVE);
            SPR_setHFlip(player.sprite,FALSE);
        }
        else{
            if( (changed & BUTTON_RIGHT) | (changed & BUTTON_LEFT) ){
                player.velx = 0;
                SPR_setAnim(player.sprite,ANIM_STRAIGHT);
            }
        }
        if (state & BUTTON_B & changed)
        {
            shootBullet();
        }
    }
}

void shootBullet(){
    if(bulletsOnScreen < MAX_BULLETS){
        Entity* b;
        u16 i = 0;
        for(i=0; i<MAX_BULLETS; i++){
            b = &bullets[i];
            if(b->health == 0){
                b->x = player.x+4;
                b->y = player.y;
                reviveEntity(b);
                b->vely = -3;
                SPR_setPosition(b->sprite,b->x,b->y);
                bulletsOnScreen++;
                break;
            }
        }
    }
}

void positionBullets(){
    u16 i = 0;
    Entity *b;
    for(i = 0; i < MAX_BULLETS; i++){
        b = &bullets[i];
        if(b->health > 0){
            b->y += b->vely;
            if(b->y + b->h < 0){
                killEntity(b);
                bulletsOnScreen--;
            } else{
                SPR_setPosition(b->sprite,b->x,b->y);
            }
        }
    }
}
