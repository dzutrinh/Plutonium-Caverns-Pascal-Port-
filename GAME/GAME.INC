{*--------------------------------------------------------------------------*}
{*                             G A M E . I N C                              *}
{*--------------------------------------------------------------------------*}
{*                                                                          *}
{* Purposes   : Main game scene                                             *}
{* Prequisites: GLOBALS.INC, Core game engine                               *}
{*                                                                          *}
{*--------------------------------------------------------------------------*}
{* Origin     : Jani Nyk„nen, 2019                                          *}
{* Conversion : Trinh D.D. Nguyen, 2021                                     *}
{*--------------------------------------------------------------------------*}

{$ifndef __PLUTONIUM_CAVERNS_GAME__}
{$define __PLUTONIUM_CAVERNS_GAME__}

const	MAX_BOULDERS	= 64;

type    { boulder and bomb type }
	BOULDER = record
        	pos		: BYTE2;
                target		: BYTE2;

                exist		: boolean;
                redraw		: boolean;
                old_player_state: boolean;

                move_timer	: uint8;
                moving		: boolean;
                bomb_timer	: int8;
                anim_timer	: uint8;

                btype		: uint8;
	end;
	BOULDER_LIST = array[0..MAX_BOULDERS-1] of BOULDER;

        { player object }
        PLAYER = record
                pos		: BYTE2;
                target		: BYTE2;

                move_timer	: int8;
                moving		: boolean;
                spr		: SPRITE;
                direction	: uint8;

                redraw		: boolean;
                flip		: boolean;
                acting		: boolean;
                force_release	: boolean;
                victory		: boolean;

                pickaxe		: uint8;
                shovel		: uint8;
                bombs		: uint8;
                keys		: uint8;
                gems		: uint8;
                max_gems	: uint8;
        end;

        { stage object }
        STAGE = record
                top_left	: VEC2;

                bmp_frame	: BITMAP;
                bmp_tileset	: BITMAP;
                bmp_items	: BITMAP;
                bmp_anim	: BITMAP;
                bmp_exp		: BITMAP;
                bmp_ship	: BITMAP;

                initialized	: boolean;

                tmap		: TILEMAP;
                data		: RASTER;
                solid		: RASTER;
                w, h		: uint8;

                lava_timer	: uint16;
                glow_timer	: uint16;

                frame_drawn	: boolean;
                static_drawn	: boolean;

                boulders	: BOULDER_LIST;
                bcount		: uint8;
                pl		: PLAYER;

                anim_timer	: int8;
                anim_mode	: uint8;
                anim_pos	: BYTE2;
                anim_frame	: int8;
        end;

const	BHOLE_ANIM_MAX		= 30;
        INITIAL_ANIM_TIME	= 32;
        ANIM_SKIP		= 8;
	MOVE_TIME		= 32;

        GAME_CLEAR_TIME		= 120;

var     bmp_tileset		: BITMAP;
	bmp_items		: BITMAP;
        bmp_player		: BITMAP;

        game_font		: BITMAP;
        game_items		: BITMAP;
	game_stage		: STAGE;
        game_pmenu		: MENU;

	game_redraw_HUD		: boolean;
        game_redraw_clear	: boolean;
        game_stage_clear	: boolean;
	game_assets_loaded	: boolean;
        game_clear_timer	: int16;
        game_stage_index	: uint8;

{ BOULDER }
procedure	boulder_init;                                                                   forward;
procedure	boulder_create          (var b: BOULDER; x, y, btype: uint8);                   forward;
procedure	boulder_update          (var b: BOULDER; var pl: PLAYER; var s: STAGE;
			steps: int16);                                                          forward;
procedure	boulder_draw            (var b: BOULDER; dx, dy: int16);                        forward;
procedure	boulder_check_detonation(var b: BOULDER; var s: STAGE; dx, dy: uint8);          forward;

{ PLAYER }
procedure	player_init;                                                                    forward;
procedure	player_create(var pl: PLAYER; x, y: uint8);                                     forward;
procedure	player_update(var pl: PLAYER; var s: STAGE; steps: int16);                      forward;
procedure	player_draw  (var pl: PLAYER; redraw: boolean; var s: STAGE; dx, dy: int16);    forward;

{ STAGE }
procedure	stage_create        (var s: STAGE);                                             forward;
procedure       stage_destroy       (var s: STAGE);                                             forward;
procedure	stage_refactor      (var s: STAGE);                                             forward;
function	stage_init          (var s: STAGE; map: string): boolean;                       forward;
procedure	stage_init_assets   (var s: STAGE);                                             forward;
procedure	stage_update        (var s: STAGE; steps: int16);                               forward;
procedure	stage_draw          (var s: STAGE);                                             forward;
procedure	stage_draw_static   (var s: STAGE;
			startx, starty, ex, ey: uint8; dx, dy, skip: int16); 	                forward;
procedure	stage_update_solid  (var s: STAGE; x, y, value: uint8);				forward;
function	stage_get_solid_data(var s: STAGE; x, y: uint8): uint8;                        	forward;
procedure	stage_update_tile   (var s: STAGE; x, y, value: uint8);                         forward;
function	stage_get_tile_data (var s: STAGE; x, y: uint8): uint8;                      	forward;
procedure	stage_item_collision(var pl: PLAYER; var s: STAGE);                             forward;
function	stage_activate_tile (var pl: PLAYER; tx, ty: uint8; var s: STAGE): boolean;	forward;
procedure	stage_set_animation (var s: STAGE; mode, x, y: uint8);                         	forward;
procedure	stage_detonate      (var s: STAGE; dx, dy: uint8);                              forward;
procedure	stage_reset         (var s: STAGE);                                             forward;
procedure	stage_redraw        (var s: STAGE);                                             forward;

{ GAME SCENE }
function	game_setup_scene: SCENE;                                                        forward;
procedure	game_redraw_info(var pl: PLAYER);                                               forward;
function	game_load_assets: boolean;                                                      forward;
procedure	game_clear_assets;                                                              forward;

{*----------------------------- B O U L D E R ------------------------------*}

{ move boulder }
procedure	move_boulder(var b: BOULDER; var pl: PLAYER;
			var s: STAGE; steps: int16);
var	dist	: int16;
	dir	: BYTE2;
begin
	{ move }
	if b.moving then
        begin
        	b.redraw     := TRUE;
                b.move_timer := pl.move_timer;
        end
        else
        begin
        	{ movement }
                if pl.moving then
                begin
                	if (pl.target.x = b.pos.x) and
			   (pl.target.y = b.pos.y) then
                        begin
                        	dir.x := uint8(pl.target.x - pl.pos.x);
                                dir.y := uint8(pl.target.y - pl.pos.y);

                                b.target.x := uint8(b.pos.x + dir.x);
                                b.target.y := uint8(b.pos.y + dir.y);

                                { check if not a free tile }
                                if (b.target.x < 1) or (b.target.x >= s.w-1) or
                                   (b.target.y < 1) or (b.target.y >= s.h-1) or
                                   ((b.btype <> 2) and
                                    (stage_get_solid_data(s, b.target.x, b.target.y) > 0) and
                                    (stage_get_solid_data(s, b.target.x, b.target.y) <> 3)) then
                                begin
                                	pl.target     := pl.pos;
                                        pl.move_timer := 0;
                                        pl.moving     := FALSE;
                                        b.target      := b.pos;
                                end
                                else
                                begin
                                	b.moving     := TRUE;
                                        b.move_timer := pl.move_timer;

                                        { update solid data }
                                        stage_update_solid(s, b.pos.x, b.pos.y, 0);

                                        SND_play(SND_MOVE);
                                end;
                        end;
                end;

                { redrawing check }
                dist := maxi(absi(int16(pl.target.x)-int16(b.pos.x)),
		             absi(int16(pl.target.y)-int16(b.pos.y)));

                if dist <= 2 then b.redraw := TRUE;
        end;
end;

{ update bomb }
procedure	update_boulder_bomb(var b: BOULDER; var pl: PLAYER;
			var s: STAGE; steps: int16);
begin
	{ check if a turn has passed }
	if (b.btype = 1) and (pl.moving) and (not b.old_player_state) then
        begin
        	b.redraw := TRUE;
                dec(b.bomb_timer);
                SND_play(SND_BEEP5);
        end;

        { detonate }
        if (b.bomb_timer <= 0) and (not pl.moving) then
        begin
        	b.exist := FALSE;

                { detonate }
                stage_detonate(s, b.pos.x, b.pos.y);

                SND_play(SND_EXPLOSION);

                exit;
        end;

        b.old_player_state := pl.moving;
end;

{ check collision }
procedure	check_boulder_collision(var b: BOULDER; var pl: PLAYER;
			var s: STAGE; steps: int16);
begin
	{ only move if the player is moving }
	if (not pl.moving) and b.moving then
        begin
                b.moving     := FALSE;
                b.pos        := b.target;
                b.move_timer := 0;

                { check if in lava }
                if (b.btype <> 2) and
		   (stage_get_solid_data(s, b.pos.x, b.pos.y) = 3) then
                begin
                	{ remove lava }
                	stage_update_tile (s, b.pos.x, b.pos.y, 0);
                        stage_update_solid(s, b.pos.x, b.pos.y, 0);

                        SND_play(SND_DISAPPEAR);

                        { stop existing }
                        b.exist := FALSE;
                        exit;
                end else
                if b.btype = 2 then
                begin
                	{ destroy everything }
                        s.anim_timer := 0;
                        s.data^ [b.pos.y * s.w + b.pos.x] := 0;
                        s.solid^[b.pos.y * s.w + b.pos.x] := 0;
                end;

                { update solid data }
                stage_update_solid(s, b.pos.x, b.pos.y, 2);
        end;
end;

procedure	boulder_init;
begin
	bmp_tileset := AST_get('tileset');
        bmp_items   := AST_get('items');
end;

procedure	boulder_create(var b: BOULDER; x, y, btype: uint8);
begin
	b.pos.x  	:= x;
        b.pos.y  	:= y;
        b.target 	:= b.pos;
        b.btype  	:= btype;
        b.bomb_timer	:= 5;

        { set default s}
        b.moving	   := FALSE;
        b.move_timer	   := 0;
        b.redraw	   := TRUE;
        b.exist		   := TRUE;
        b.old_player_state := FALSE
end;

procedure	boulder_update(var b: BOULDER; var pl: PLAYER;
			var s: STAGE; steps: int16);
begin
	if not b.exist then exit;

        { animate black hole }
        if b.btype = 2 then
        begin
                inc(b.anim_timer, steps);

                if b.anim_timer >= BHOLE_ANIM_MAX then
                        dec(b.anim_timer, BHOLE_ANIM_MAX);

                b.redraw := TRUE;
        end;

        { check collision }
        check_boulder_collision(b, pl, s, steps);

        { move }
        move_boulder(b, pl, s, steps);

        { update bomb }
        update_boulder_bomb(b, pl, s, steps);
end;

procedure	boulder_draw(var b: BOULDER; dx, dy: int16);
var	x, y,
	frame	: int16;
begin
	if (not b.exist) or (not b.redraw) then exit;

        { determine render position }
        x := b.target.x * 16;
        y := b.target.y * 16;

        if b.moving then
        begin
                inc(x, int16(int16(b.pos.x) - b.target.x) * (b.move_timer shr 1));
                inc(y, int16(int16(b.pos.y) - b.target.y) * (b.move_timer shr 1));
        end;

	if b.btype = 0 then
                VGA_draw_bitmap_region(bmp_tileset, 112, 0, 16, 16,
                	dx + x, dy + y, FALSE)
        else
	if b.btype = 1 then
        	VGA_draw_bitmap_region(bmp_items,
			(5 - b.bomb_timer) * 16, 16, 16, 16,
			dx + x, dy + y, FALSE)
        else
        if b.btype = 2 then
        begin
        	frame := b.anim_timer div (BHOLE_ANIM_MAX shr 2);

                if frame > 3 then frame := 3;

                VGA_fill_rect(dx + x, dy + y, 16, 16, 0);

                VGA_draw_bitmap_region_fast(bmp_items,
				frame * 16, 48, 16, 16,
                		dx + x, dy + y);
        end;

        b.redraw := FALSE;
end;

procedure	boulder_check_detonation(var b: BOULDER; var s: STAGE; dx, dy: uint8);
begin
	if not b.exist then exit;

        if (b.pos.x >= dx-1) and (b.pos.x <= dx+1) and
           (b.pos.y >= dy-1) and (b.pos.y <= dy+1) then
        begin
        	b.exist := FALSE;
                stage_update_solid(s, b.pos.x, b.pos.y, 0);
        end;
end;

{*------------------------------ P L A Y E R -------------------------------*}

{ check if free tile }
function        player_check_free_tile(var pl: PLAYER; var s: STAGE; tx, ty: uint8): boolean;
var	t	: uint8;
begin
	t := stage_get_solid_data(s, tx, ty);

        player_check_free_tile := (t = 0) or (t = 2) or (t = 8);
end;

{ get key down state }
function	get_down_state(arrow: int16): int16;
var	state	: int16;
begin
	state := KBD_get_arrow_key(arrow);
	if (state = KBD_UP) or (state = KBD_RELEASED) then
        	get_down_state := -1
        else
        begin
                if state = KBD_PRESSED then
                	get_down_state := 1
                else	get_down_state := 0;
        end;
end;

{ control player }
procedure	player_control(var pl: PLAYER; var s: STAGE);
var	tx, ty,
	dir	: uint8;
	state	: int16;
	flip	: boolean;
begin
	tx    := pl.pos.x;
        ty    := pl.pos.y;
        flip  := FALSE;

        { check arrow keys }
        state := get_down_state(ARROW_LEFT);
        if state >= 0 then
        begin
        	dec(tx);
                dir := 3;
                flip := TRUE;
        end
        else
        begin
	        state := get_down_state(ARROW_RIGHT);
        	if state >= 0 then
	        begin
        	        inc(tx);
                	dir := 2;
	        end
        	else
                begin
		        state := get_down_state(ARROW_UP);
			if state >= 0 then
		        begin
				dec(ty);
                		dir := 1;
		        end
                        else
                        begin
				state := get_down_state(ARROW_DOWN);
			        if state >= 0 then
			        begin
		        		inc(ty);
		        	        dir := 0;
			        end;
                        end;
                end;
        end;

        if pl.force_release then
        begin
        	if state = 1 then
                	pl.force_release := FALSE
                else	exit;
        end;

        { if movement }
        if (tx <> pl.pos.x) or (ty <> pl.pos.y) then
        begin
        	pl.direction := dir;
                pl.flip      := flip;
                pl.redraw    := TRUE;

                { check if free }
                if ((tx > 0) and (ty > 0) and
		    (tx < s.w-1) and (ty < s.h-1)) and
                   player_check_free_tile(pl, s, tx, ty) then
                begin
                	{ start moving }
                	pl.move_timer := MOVE_TIME;
                        pl.moving     := TRUE;
                        pl.redraw     := TRUE;

                        pl.target.x   := tx;
                        pl.target.y   := ty;
                end else
                { otherwise activate possibly solid tile }
                if (state = 1) then
                begin
                	if stage_activate_tile(pl, tx, ty, s) then
                        begin
                                pl.acting    := TRUE;
                                pl.direction := dir;
                                pl.flip      := flip;
                        end;
                end;

                { special check, if a bombing place }
	        if stage_get_solid_data(s, tx, ty) = 8 then
        	begin
        		if stage_activate_tile(pl, tx, ty, s) then
	                begin
        	                pl.moving        := FALSE;
                	        pl.move_timer    := 0;
	                        pl.target        := pl.pos;
        	                pl.force_release := TRUE;
                	end;
	        end;
        end;
end;

{ animate player }
procedure	player_animate(var pl: PLAYER; steps: int16);
const	ANIM_SPEED	= 8;
begin
	{ acting }
	if pl.acting then
        begin
        	pl.redraw     := TRUE;
                pl.spr.frame  := 4;
                pl.spr.row    := mini(pl.direction, 2);
                pl.acting     := FALSE;

                exit;
        end;

        if pl.moving then
		SPR_animate(pl.spr, mini(pl.direction, 2),
			0, 3, ANIM_SPEED, steps)
        else
        begin
        	if pl.spr.frame <> 0 then
			pl.redraw := TRUE;

                pl.spr.frame := 0;
                pl.spr.row   := mini(pl.direction, 2);
        end;
end;

{ intialize player }
procedure	player_init;
begin
	bmp_player := AST_get('player');
end;

{ create player }
procedure	player_create(var pl: PLAYER; x, y: uint8);
begin
	pl.pos.x    := x;
        pl.pos.y    := y;
        pl.target.x := x;
        pl.target.y := y;

	SPR_create(pl.spr, 16, 16);

        { set defaults }
        pl.direction	 := 0;
        pl.move_timer	 := 0;
	pl.moving	 := FALSE;
        pl.redraw	 := TRUE;
        pl.flip		 := FALSE;
        pl.acting	 := FALSE;
        pl.force_release := FALSE;
        pl.victory	 := FALSE;
end;

{ update player }
procedure	player_update(var pl: PLAYER; var s: STAGE; steps: int16);
begin
	{ check input }
        if pl.move_timer <= 0 then
		player_control(pl, s);

        { animate }
        player_animate(pl, steps);

        { update move timer }
        if (pl.move_timer > 0) then
        begin
        	dec(pl.move_timer, 2 * steps);
		pl.redraw := TRUE;

		if pl.move_timer <= 0 then
		begin
                        pl.move_timer := 0;

                        { move to the target }
                        pl.pos        := pl.target;
                        pl.moving     := FALSE;

                        { check item collision }
                        stage_item_collision(pl, s);
		end;
        end;
end;

{ draw player }
procedure	player_draw(var pl: PLAYER; redraw: boolean; var s: STAGE; dx, dy: int16);
var	x, y	: int16;
begin
        if not pl.redraw then exit;

        { determine render position }
        x := pl.target.x * 16;
        y := pl.target.y * 16;

        if pl.moving then
        begin
        	inc(x, (pl.pos.x - pl.target.x) * (pl.move_timer shr 1));
                inc(y, (pl.pos.y - pl.target.y) * (pl.move_timer shr 1));
        end;

        { redraw bottom tiles }
        if redraw then
		stage_draw_static(s,
			pl.pos.x - 1, pl.pos.y - 1,
                	pl.pos.x + 1, pl.pos.y + 1,
			dx, dy, 0);

        { draw sprite }
        SPR_draw(pl.spr, bmp_player, dx + x, dy + y, pl.flip);

        pl.redraw := FALSE;
end;

{*------------------------------- S T A G E --------------------------------*}

{ draw frame }
procedure	draw_box_frame(bmp: BITMAP; dx, dy, w, h: int16; c: uint8);
const	TILE_W	= 8;
	TILE_H	= 8;
var	x, y,
	sx, sy	: int16;
begin
        sx := 0;
        sy := 0;

        for y := 0 to h-1 do
        	for x := 0 to w-1 do
                begin
                	{ skip if empty }
                	if (x = 1) and (y > 0) and (y < h-1) then
                        	inc(x, w-2);

                        { determine source tile position }
                        if (x = 0) and (y = 0) then
                        begin
                        	sx := 0; sy := 0;
                        end else
                        if (x = w-1) and (y = 0) then
                        begin
                                sx := 16; sy := 0;
                        end else
                        if (x = w-1) and (y = h-1) then
                        begin
                        	sx := 16; sy := 16;
                        end else
                        if (x = 0) and (y = h-1) then
                        begin
                        	sx := 0; sy := 16;
                        end else
                        if y = 0 then
                        begin
                        	sx := 8; sy := 0;
                        end else
                        if y = h-1 then
                        begin
                                sx := 8; sy := 16;
                        end else
                        if x = 0 then
                        begin
                        	sx := 0; sy := 8;
                        end else
                        if x = w-1 then
                        begin
                        	sx := 16; sy := 8;
                        end
                        else	continue;

                        { draw tile }
                        VGA_draw_bitmap_region_fast(bmp, sx, sy,
                        	TILE_W, TILE_H,
				dx + x * TILE_W,
				dy + y * TILE_H);
                end;

        { fill with black }
        VGA_fill_rect(dx+TILE_W, dy+TILE_H,
		(w-2) * TILE_W, (h-2) * TILE_H, c);

        { draw shadows }
        VGA_fill_rect(dx + w * TILE_W, dy + TILE_H,
		TILE_W, h * TILE_H, 51);
        VGA_fill_rect(dx + TILE_W, dy + h * TILE_H,
		(w-1) * TILE_W, TILE_H, 51);
end;

{ draw animation }
procedure       stage_draw_animation(var s: STAGE; topx, topy: int16);
var	sx, sy,
	sw, sh,
	dx, dy, skip	: int16;
begin
	{ disappearing tile }
	if (s.anim_mode = 1) or (s.anim_mode = 2) then
        begin
        	skip := s.anim_timer div ANIM_SKIP;

                if skip > 0 then
                begin
                	VGA_fill_rect(topx + s.anim_pos.x * 16,
                        	      topy + s.anim_pos.y * 16,
                                      16, 16, 0);

                        { draw boulder behind if a frozen boulder }
                        if s.anim_mode = 2 then
                        	VGA_draw_bitmap_region(s.bmp_tileset,
                                	112, 0, 16, 16,
                                        topx + s.anim_pos.x * 16,
                                        topy + s.anim_pos.y * 16, FALSE);

                        { draw the disappearing tile }
                        stage_draw_static(s, s.anim_pos.x, s.anim_pos.y,
                        		  s.anim_pos.x, s.anim_pos.y,
					  topx, topy, skip);
                end;
        end
        else	{ explosion }
        if s.anim_mode = 5 then
        begin
        	dx := int16(s.anim_pos.x) - 1;
                dy := int16(s.anim_pos.y) - 1;
                sw := dx + 2;
                sh := dy + 2;
                if dx < 0 then dx := 0; if sw > s.w-1 then sw := s.w-1;
                if dy < 0 then dy := 0; if sh > s.h-1 then sh := s.h-1;

                stage_draw_static(s, dx, dy, sw, sh, topx, topy, 0);

                { redraw the player }
                s.pl.redraw := TRUE;
                player_draw(s.pl, FALSE, s, topx, topy);

                { draw explosion }
                skip := 4 - s.anim_timer div ANIM_SKIP;
                case skip of
                0:	begin
                		sx :=  0; sy :=  0;
                                dx := -8; dy := -8;
                                sw := 16; sh := 16;
                	end;
                1:	begin
                		sx :=  16; sy :=   0;
                                dx := -16; dy := -16;
                                sw :=  32; sh :=  32;
                	end;
                2:	begin
                		sx :=  48; sy :=   0;
                                dx := -24; dy := -24;
                                sw :=  48; sh :=  48;
                	end;
                3:  	begin
                		sx :=  96; sy :=  0;
                                dx := -24; dy := -24;
                                sw :=  48; sh :=  48;
                	end;
                end;
                VGA_draw_bitmap_region(s.bmp_exp,
				sx, sy, sw, sh,
                		topx+s.anim_pos.x * 16 + 8 + dx,
                                topy+s.anim_pos.y * 16 + 8 + dy, FALSE);
        end;

        { draw item animation }
        if s.anim_frame >= 0 then
        begin
        	skip := 3 - (s.anim_timer div ANIM_SKIP);
                if skip < 0 then skip := 0 else
                if skip > 2 then skip := 2;

                VGA_draw_bitmap_region(s.bmp_anim,
                	(s.anim_frame + skip) * 16, s.pl.spr.row * 16, 16, 16,
                        topx + s.anim_pos.x * 16,
			topy + s.anim_pos.y * 16, s.pl.flip);
        end;
end;

{ draw lava }
procedure	stage_draw_lava(var s: STAGE; dx, dy: int16);
var	skip, t,
	x, y, p,
	sx, sy	: int16;
begin
        t  := s.lava_timer div FIXED_PREC;
        sx := 16 + t;
        sy := 16 - t;
        p  := s.glow_timer div FIXED_PREC;

        if p >= 3 then p := 1;
        inc(sx, 32 * p);

        for y := 0 to s.h-1 do
        begin
                for x := 0 to s.w-1 do
                begin
                	if s.data^[y * s.w + x] <> 4 then continue;

                        { check if disappearing lava }
                        if (s.anim_timer > 0) and
			   (s.anim_mode = 4) and
                           (x = s.anim_pos.x) and (y = s.anim_pos.y) then
                        begin
                        	skip := s.anim_timer div ANIM_SKIP;

                                VGA_fill_rect(dx + x * 16, dy + y * 16,
					16, 16, 0);

                                if skip > 0 then
                                	VGA_draw_bitmap_region_skip(
						s.bmp_tileset,
                                        	sx, sy, 16, 16,
                                                dx + x * 16,
                                                dy + y * 16, skip, FALSE);
                        end
                        else	VGA_draw_bitmap_region_fast(s.bmp_tileset,
                        		sx, sy, 16, 16,
					dx + x * 16, dy + y * 16);
                end;
        end;
end;

{ add a boulder }
procedure	stage_add_boulder(var s: STAGE; x, y, btype: uint8);
var	i	: uint8;
begin
	{ find the first boulder that doest not exist }
        for i := 0 to s.bcount-1 do
        begin
                if not s.boulders[i].exist then
                begin
                	boulder_create(s.boulders[i], x, y, btype);
                        s.solid^[y * s.w + x] := 2;
                	break;
                end;
        end;
end;

{ parse objects (plus pas data to certain player object }
procedure	stage_parse_objects(var s: STAGE);
var	x, y, t,
	max_gems: uint8;
begin
        if s.tmap = nil then
        begin
        	ERR_throw_no_param('Map parsing error.');
        	APP_terminate;
                exit;
        end;

	max_gems := 0;

        for y := 0 to s.h-1 do
        begin
        	for x := 0 to s.w-1 do
                begin
                	t := s.data^[y*s.w+x];

                        if t = 5 then	{ boulder }
                        begin
                        	stage_add_boulder(s, x, y, 0);
                                s.data^[y * s.w + x] := 0;
                        end else
                        if t = 17 then	{ player }
                        begin
                        	player_create(s.pl, x, y);
                                s.data^[y * s.w + x] := 0;
                        end else
                        if t = 22 then	{ gem }
                        begin
                        	inc(max_gems)
			end else
                        if t = 23 then	{ black hole }
                        begin
                                stage_add_boulder(s, x, y, 2);
                                s.data^[y * s.w + x] := 0;
                        end;
                end;
        end;
	s.pl.pickaxe := s.tmap^.layers[0]^[0]-1;
        s.pl.shovel  := s.tmap^.layers[0]^[1]-1;
        s.pl.bombs   := s.tmap^.layers[0]^[2]-1;
        s.pl.keys    := 0;
        s.pl.gems    := 0;
        s.pl.max_gems:= max_gems;
end;

{ set solid }
procedure	stage_set_solid(var s: STAGE);
var	x, y, t	: uint8;
	p	: int16;
begin
	for y := 0 to s.h-1 do
        begin
        	for x := 0 to s.w-1 do
                begin
                	t := 0;
                        p := y * s.w + x;

                        case s.data^[p] of
                        1, 8,		{ wall and color blocks }
			9, 10:	t := 1;
                        4:	t := 3;	{ lava }
                        14, 15,         { switches }
                        16, 30,
                        31, 32:	t := 4;
                        7:	t := 6; { lock }
                        2:	t := 5; { frozen wall }
                        3:	t := 7; { frozen boulder }
                        6:	t := 8;	{ bomb placed }
                        end;
                        s.solid^[p] := t;
                end;
        end;
end;

{ create a stage object }
procedure	stage_create(var s: STAGE);
begin
	MEM_set(s, 0, sizeof(STAGE));
        s.initialized := FALSE;
end;

{ destroy a stage object }
procedure       stage_destroy(var s: STAGE);
begin
        stage_refactor(s);
end;

{ destroy stage data }
procedure	stage_refactor(var s: STAGE);
begin
	if not s.initialized then exit;
        if s.data  <> nil then MEM_free(s.data);
        if s.solid <> nil then MEM_free(s.solid);
        if s.tmap  <> nil then TLM_destroy(s.tmap);
end;

{ initialize a stage }
function	stage_init(var s: STAGE; map: string): boolean;
var	i, size,
	tileid	: int16;
	t	: TILEMAP;
begin
	stage_init := FALSE;

	t := TLM_load(map);
        if t = nil then
	begin
                ERR_throw('Cannot load map ', map);
                APP_terminate;
		exit;
        end;

        s.tmap := t;

        { copy data and compute the amount of boulder }
        size    := int16(t^.w) * (t^.h-1);
        s.data  := RASTER(MEM_malloc(size * sizeof(uint8)));
        s.solid := RASTER(MEM_malloc(size * sizeof(uint8)));
        if (s.data = nil) or (s.solid = nil) then
	begin
                THROW_MALLOC_ERR;
		exit;
        end;

        s.bcount := 1;
        for i := 0 to size-1 do
        begin
                tileid := uint8(maxi(t^.layers[0]^[i + t^.w], 16)-16);
                s.data^[i]  := tileid;
                s.solid^[i] := 0;

                { if boulder }
                if (tileid = 5) or (tileid = 3) or
		   (tileid = 6) or (tileid = 23) then
			inc(s.bcount);
        end;

        s.w := t^.w;
        s.h := t^.h-1;	{ excluded first row of the map }

        { set defaults }
        s.frame_drawn  := FALSE;
        s.static_drawn := FALSE;
        s.lava_timer   := 0;
        s.glow_timer   := 0;
        s.anim_timer   := 0;
        s.anim_pos.x   := 0;
        s.anim_pos.y   := 0;
        s.anim_mode    := 0;
        s.top_left.x   := 24;
        s.top_left.y   := 16;

        { clear boulder data }
        MEM_set(s.boulders, 0, sizeof(s.boulders));

        { no boulders inactive }
        for i := 0 to s.bcount-1 do s.boulders[i].exist := FALSE;

        stage_set_solid(s);		{ set solid data }
        stage_parse_objects(s);		{ parse objects  }

        s.initialized := TRUE;

        stage_init := TRUE;
end;

{ initialize stage assets }
procedure	stage_init_assets(var s: STAGE);
begin
	s.bmp_frame	:= AST_get('frame');
        s.bmp_tileset	:= AST_get('tileset');
        s.bmp_items	:= AST_get('items');
        s.bmp_anim	:= AST_get('anim');
        s.bmp_exp	:= AST_get('exp');
        s.bmp_ship	:= AST_get('ship');
end;

{ update stage }
procedure	stage_update(var s: STAGE; steps: int16);
const	LAVA_SPEED 	= 8;
	GLOW_SPEED	= 4;
var	i	: uint8;
begin
	if not s.initialized then exit;

	{ update lava timer }
	inc(s.lava_timer, LAVA_SPEED * steps);
        s.lava_timer := s.lava_timer mod (16 * FIXED_PREC);

	inc(s.glow_timer, GLOW_SPEED * steps);
        s.glow_timer := s.glow_timer mod (4 * FIXED_PREC);

        { update animation timer }
        if (s.anim_timer > 0) then
        begin
        	dec(s.anim_timer, steps);
                if s.anim_timer <= 0 then
                begin
                	s.anim_timer := 0;
                        if s.anim_mode <> 5 then
                        	s.data^[s.anim_pos.y * s.w + s.anim_pos.x] := 0
                        else
                        begin
                        	{ need to get rid of certain artifacts }
                        	stage_draw_static(s,
                                	s.anim_pos.x-1, s.anim_pos.y-1,
                                        s.anim_pos.x+1, s.anim_pos.y+1,
                                        s.top_left.x, s.top_left.y, 0);
                                s.pl.redraw := TRUE;
                        end;

                        { ensure the player is not moving }
                        s.pl.moving     := FALSE;
                        s.pl.move_timer := 0;
                        s.pl.target     := s.pl.pos;

                        { create a boulder from a destroyed frozen one }
                        if s.anim_mode = 2 then
                        	stage_add_boulder(s, s.anim_pos.x, s.anim_pos.y, 0);
                end;
                exit;
        end;

        { update boulders }
        for i := 0 to s.bcount-1 do
        	boulder_update(s.boulders[i], s.pl, s, steps);

        { update player }
        player_update(s.pl, s, steps);
end;

{ draw stage }
procedure	stage_draw(var s: STAGE);
const	RIGHT_FRAME_WIDTH	= 12;
var     i		: uint8;
	topx, topy, w	: int16;
begin
	if not s.initialized then exit;

	topx	:= s.top_left.x;
        topy	:= s.top_left.y;

        VGA_toggle_clipping(FALSE);

        if not s.frame_drawn then
        begin
        	{ clear background }
                VGA_clear(123);

                { left }
                w := s.w * 2 + 2;
                draw_box_frame(s.bmp_frame, 16, 8, w, s.h * 2 + 2, 0);

                { right }
                draw_box_frame(s.bmp_frame, (w + 2) * 8 + 16, 8,
			RIGHT_FRAME_WIDTH, s.h * 2 + 2, 0);

                s.frame_drawn := TRUE;
        end;

        { draw static tiles }
        if not s.static_drawn then
        begin
                stage_draw_static(s,
			0, 0, s.w-1, s.h-1, topx, topy, 0);
                s.static_drawn := TRUE;
        end;

        { draw lava }
        stage_draw_lava(s, topx, topy);

        { draw animation }
        if s.anim_timer > 0 then
        	stage_draw_animation(s, topx, topy);

        { draw player }
        player_draw(s.pl, TRUE, s, topx, topy);

        { draw boulders }
        for i := 0 to s.bcount-1 do
        	boulder_draw(s.boulders[i], topx, topy);

        VGA_toggle_clipping(TRUE);
end;

{ draw static tiles }
procedure	stage_draw_static(var s: STAGE;
			startx, starty, ex, ey: uint8;
			dx, dy, skip: int16);
var	x, y, t	: uint8;
	sx, sy,
	sw, sh	: int16;
	jx, jy	: int8;
        bmp	: BITMAP;
begin
	if not s.initialized then exit;

	for y := starty to ey do
        begin
        	for x := startx to ex do
                begin
                	sw := 16;	sh := 16;
                        jx := 0;	jy := 0;
			sx := 0;	sy := 0;

                        t  := s.data^[y * s.w + x];

                        if t = 0 then
                        begin
                        	VGA_fill_rect(dx + x * 16, dy + y * 16,
					16, 16, 0);
                                continue;
                        end;

                        bmp := s.bmp_tileset;
                        case t of
                        1:	begin	{ wall }
                        		sx := 0; sy := 0;
                        	end;
                        2:	begin	{ ice wall }
                        		sx := 0; sy := 16;
                        	end;
                        3:	begin	{ frozen boulder }
                        		sx := 0; sy := 32;
                        	end;
                        6:	begin	{ bomb place }
                        		sx := 112; sy := 16;
                        	end;
                        7: 	begin	{ lock }
                        		sx := 112; sy := 48;
                        	end;
			8, 9, 10,
			11, 12, 13:
                        	begin	{ color blocks }
                                	sx := 16 + (t - 8) * 16;
                                	sy := 48;
                                end;
                        14, 15, 16,
                        30, 31, 32:
                        	begin	{ switches }
                                	if t <= 16 then
                                        	sx := 16 + (t - 14) * 32
                                        else	sx := 32 + (t - 30) * 32;
                                        sy := 32;
                                end;
                        18, 19,		{ items }
                        20, 21, 22:
                        	begin
                                	sx := (t - 18) * 16;
                                        sy := 0;
                                        bmp := s.bmp_items;
                                end;
                        24:	begin	{ ship }
                        		sx := 0;
                                        sy := 0;
                                        sw := 32;
                                        sh := 32;
                                        jx := -1;
                                        jy := -1;
                                        bmp := s.bmp_ship;
                        	end;
                        else	continue;
                        end;
                        if skip > 0 then
                        	VGA_draw_bitmap_region_skip(bmp,
                                	sx, sy, 16, 16,
					dx + x * 16, dy + y * 16, skip, FALSE)
                        else	VGA_draw_bitmap_region_fast(bmp,
                        		sx, sy, sw, sh,
					dx + (x + jx) * 16,
					dy + (y + jy) * 16);
                end;
        end;
end;

{ update solid data }
procedure	stage_update_solid(var s: STAGE; x, y, value: uint8);
begin
	if (x > s.w - 1) or (y > s.h - 1) then exit;

        s.solid^[y * s.w + x] := value;
end;

{ get solid tile data }
function	stage_get_solid_data(var s: STAGE; x, y: uint8): uint8;
begin
	if (x > s.w-1) or (y > s.h-1) then
	begin
		stage_get_solid_data := 1;
		exit;
        end;

        stage_get_solid_data := s.solid^[y * s.w + x];
end;

{ update tile data }
procedure	stage_update_tile(var s: STAGE; x, y, value: uint8);
begin
	if (x > s.w - 1) or (y > s.h - 1) then exit;

        { active lava death animation }
        if (s.data^[y * s.w + x] = 4) and (value = 0) then
        begin
        	stage_set_animation(s, 4, x, y);
                s.anim_frame := -1;

                exit;
        end;
        s.data^[y * s.w + x] := value;
end;

{ get tile data }
function	stage_get_tile_data(var s: STAGE; x, y: uint8): uint8;
begin
	if (x > s.w - 1) or (y > s.h - 1) then
	begin
		stage_get_tile_data := 0;
		exit;
        end;
        stage_get_tile_data := s.data^[y * s.w + x];
end;

{ item collision }
procedure	stage_item_collision(var pl: PLAYER; var s: STAGE);
var     t	: uint8;
	remove	: boolean;
begin
	t := s.data^[pl.pos.y * s.w + pl.pos.x];
        remove := FALSE;

        case t of
        18:	begin
        		inc(pl.keys);
                        remove := TRUE;
        	end;
        19:	begin
                	pl.pickaxe := uint8(mini(5, pl.pickaxe + 3));
                        remove := TRUE;
        	end;
        20:	begin
                	pl.shovel := uint8(mini(5, pl.shovel + 3));
                        remove := TRUE;
		end;
        21:	begin
        		inc(pl.bombs);
                        remove := TRUE;
        	end;
        22:	begin
        		inc(pl.gems);
                        remove := TRUE;
        	end;
        24:	pl.victory := TRUE;
        end;

        if remove then
        begin
        	s.data^[pl.pos.y * s.w + pl.pos.x] := 0;

                game_redraw_info(s.pl);

                SND_play(SND_ITEM);
        end;
end;

{ activate event }
function	stage_activate_tile(var pl: PLAYER; tx, ty: uint8; var s: STAGE): boolean;
var	i, j	: uint16;
	dif	: int16;
	t	: uint8;
begin
	j := ty * s.w + tx;
        t := s.solid^[j];

        s.anim_frame := -1;
        case t of
        4:      begin	{ toggle blocks }
        		for i := 0 to s.w * s.h - 1 do
                        begin
                        	dif := ((s.data^[j]-1) mod 16) -
				       ((s.data^[i]-1) mod 16);
                                if dif = 3 then
                                begin
                                	s.solid^[i] := 1;
                                        dec(s.data^[i], 3);
                                end else
                                if dif = 6 then
                                begin
                                	s.solid^[i] := 0;
                                        inc(s.data^[i], 3);
                                end;
                        end;

                        { make sure boulders are redrawn }
                        for i := 0 to s.bcount-1 do
                        	s.boulders[i].redraw := TRUE;

                        { toggle switch }
                        if s.data^[j] < 17 then
                        	inc(s.data^[j], 16)
                        else	dec(s.data^[j], 16);

                        stage_draw_static(s, 1, 1, s.w-2, s.h-2,
                        	s.top_left.x, s.top_left.y, 0);

                        SND_play(SND_BEEP2);
        	end;

	6:      if (pl.keys > 0) then	{ lock }
        	begin
                	stage_update_solid(s, tx, ty, 0);
                        stage_set_animation(s, 1, tx, ty);

                        dec(pl.keys);
                        game_redraw_info(pl);

                        SND_play(SND_ACTIVATE);

                        stage_activate_tile := TRUE;
                        exit;
        	end;

        5, 7:	if pl.pickaxe > 0 then	{ frozen }
        	begin
                        stage_update_solid(s, tx, ty, 0);

                        if t = 5 then
                        	stage_set_animation(s, 1, tx, ty)
                        else	stage_set_animation(s, 2, tx, ty);

                        dec(pl.pickaxe);
                        game_redraw_info(pl);

                        s.anim_frame := 0;

                        SND_play(SND_BREAK);

                        stage_activate_tile := TRUE;
                        exit;
                end;

        3:	if pl.shovel > 0 then	{ lava }
        	begin

                	stage_update_solid(s, tx, ty, 0);
                        stage_set_animation(s, 4, tx, ty);

                        dec(pl.shovel);
                        game_redraw_info(pl);

                        s.anim_frame := 3;

                        SND_play(SND_BREAK);

                        stage_activate_tile := TRUE;
                        exit;
                end;

        8:      if pl.bombs > 0 then	{ bomb place }
        	begin
                	{ add a bomb }
                	stage_add_boulder (s, tx, ty, 1);
                        stage_update_solid(s, tx, ty, 0);
                        stage_update_tile (s, tx, ty, 0);

                        pl.force_release := TRUE;
                        dec(pl.bombs);

                        game_redraw_info(pl);

                        SND_play(SND_ACTIVATE);

                        stage_activate_tile := TRUE;
                        exit;
                end;
        end;
        stage_activate_tile := FALSE;
end;

{ set animation }
procedure	stage_set_animation(var s: STAGE; mode, x, y: uint8);
begin
	s.anim_mode	:= mode;
        s.anim_timer	:= INITIAL_ANIM_TIME;
        s.anim_pos.x	:= x;
        s.anim_pos.y	:= y;
end;

{ detonate a bomb }
procedure	stage_detonate(var s: STAGE; dx, dy: uint8);
var	x, y, t	: uint8;
	p, i	: int16;
begin
	stage_update_solid(s, dx, dy, 0);

        for y := dy-1 to dy+1 do
        begin
        	for x := dx-1 to dx+1 do
                begin
                        { do not destroy the borders or the tile under the bomb }
                	if (x = 0)     or (y = 0)     or
                           (x = s.w-1) or (y = s.h-1) then continue;

                        p := y * s.w + x;
                        t := s.data^[p];
                        case t of
                        1, 7,	{ wall or solid color block }
			8, 9,
			10:	begin
                                        s.data^[p]  := 4;
                                        s.solid^[p] := 3;
                                end;
                        2, 3:	begin	{ ice }
                        		s.data^[p]  := 0;
                                        s.solid^[p] := 0;
                        	end;
                        end;
                end;
        end;

        { destroy boulders, if nearby, redraw boulders }
        for i := 0 to s.bcount-1 do
        	boulder_check_detonation(s.boulders[i], s, dx, dy);

        { set animation }
        stage_set_animation(s, 5, dx, dy);
end;

{ reset stage }
procedure	stage_reset(var s: STAGE);
var	i	: int16;
	tileid	: uint8;
begin
	if not s.initialized then exit;

	{ reset tile & solid data }
	for i := 0 to s.w * s.h - 1 do
        begin
                tileid := uint8(maxi(s.tmap^.layers[0]^[i + s.tmap^.w], 16))-16;
                s.data^[i]  := tileid;
                s.solid^[i] := 0;
        end;

        { set defaults }
        s.static_drawn := FALSE;
        s.lava_timer := 0;
        s.glow_timer := 0;
        s.anim_timer := 0;
        s.anim_pos.x := 0;
        s.anim_pos.y := 0;
        s.anim_mode  := 0;
        s.top_left.x := 24;
        s.top_left.y := 16;

        { make boulders inactive }
        for i := 0 to s.bcount-1 do
		s.boulders[i].exist := FALSE;

        { set solid data }
        stage_set_solid(s);

        { parse objects }
        stage_parse_objects(s);
end;

{ redraw }
procedure	stage_redraw(var s: STAGE);
var	i	: int16;
begin
	if not s.initialized then exit;

	{ set the render flags for the tiles }
	s.static_drawn := FALSE;

        { set the render flags for the o bjects }
        s.pl.redraw    := TRUE;
        for i := 0 to s.bcount-1 do
		s.boulders[i].redraw := TRUE;
end;

{*------------------------------ G A M E -----------------------------------*}

procedure	game_draw_energy_bar(x, y: int16; max, val: uint8);
var	i	: uint8;
	sx	: int16;
begin
	sx := 3 + val * 16;
        { draw corresponding energy bars }
	for i := 0 to max-1 do
        begin
        	if i < val then
	                VGA_draw_bitmap_region_fast(game_items,
				sx, 33,
				10, 14, x + i * 9, y + 1)
                else	VGA_draw_bitmap_region_fast(game_items,  3, 33,
				10, 14, x + i * 9, y + 1)
        end;
end;

{draw stage clear }
procedure	game_draw_stage_clear;
const	WIDTH	= 8 * 14;
	HEIGHT	= 8 * 3;
var	dx, dy	: int16;
begin
	dx := game_stage.top_left.x + game_stage.w * 8;
        dy := game_stage.top_left.y + game_stage.h * 8;

        { draw box }
        VGA_fill_rect(dx-WIDTH div 2-2, dy-HEIGHT div 2-2, WIDTH+4, HEIGHT+4, 146);
        VGA_fill_rect(dx-WIDTH div 2-1, dy-HEIGHT div 2-1, WIDTH+2, HEIGHT+2, 255);
        VGA_fill_rect(dx-WIDTH div 2-0, dy-HEIGHT div 2-0, WIDTH+0, HEIGHT+0, 0);

        { draw text }
	VGA_draw_text_fast(game_font, 'STAGE CLEAR', dx, dy-4, 0, 0, TRUE);
end;

{ callback for reset }
procedure	cb_reset_stage;
begin
	game_redraw_HUD := TRUE;
        stage_reset(game_stage);
end;

{ callback for scene changing }
procedure	cb_change;
var	state	: boolean;
begin
        state := game_stage.pl.gems = game_stage.pl.max_gems;
        app_change_scene(SCENE_SMENU, pointer(uint8(state)));
end;

{ menu callbacks }
procedure       cb_resume;
begin
        game_pmenu.active := FALSE;
        stage_redraw(game_stage);
end;

procedure	cb_reset;
begin
	game_pmenu.active := FALSE;
        stage_redraw(game_stage);
        TRN_activate(FADE_IN, 2, cb_reset_stage);
end;

procedure	cb_audio;
var	audio_enabled	: boolean;
begin
	audio_enabled := SND_is_enabled;
        SND_toggle;
        if not audio_enabled then
        begin
                game_pmenu.text[2, 10] := 'N';
                game_pmenu.text[2, 11] := ' ';
        end
        else
        begin
                game_pmenu.text[2, 10] := 'F';
                game_pmenu.text[2, 11] := 'F';
        end;
	game_pmenu.redraw := TRUE;
end;

procedure	cb_quit;
begin
	game_pmenu.active := FALSE;
        stage_redraw(game_stage);
        TRN_activate(FADE_IN, 2, cb_change);
end;

{ victory callback }
procedure	cb_win;
begin
	{ clear assets }
	game_clear_assets;

        { go to the story }
        app_change_scene(SCENE_STORY, pointer(uint8(1)));
end;

procedure	game_create_pause_menu;
begin
	{ create menu }
        MNU_create(game_pmenu, 0);

        { add buttons }
        MNU_add_button(game_pmenu, 'RESUME    ', cb_resume);
        MNU_add_button(game_pmenu, 'RESTART   ', cb_reset);
        MNU_add_button(game_pmenu, 'AUDIO: ON ', cb_audio);
        MNU_add_button(game_pmenu, 'QUIT      ', cb_quit);
end;

{ initialize game }
function	game_init: boolean;
begin
	{ create main components }
        stage_create(game_stage);

        { set defaults }
        game_redraw_HUD    := FALSE;
	game_clear_timer   := 0;
        game_stage_clear   := FALSE;
        game_redraw_clear  := FALSE;
        game_assets_loaded := FALSE;

        { create pause menu }
        game_create_pause_menu;

        game_init := TRUE;
end;

procedure	game_update(steps: int16);
begin
	if TRN_is_active then exit;

        { stage clear }
        if game_stage_clear then
        begin
        	dec(game_clear_timer, 1 * steps);
                if game_clear_timer <= 0 then
			TRN_activate(FADE_IN, 2, cb_change);
                exit;
        end;

        { check pause }
        if (game_pmenu.active) then
        begin
        	MNU_update(game_pmenu, steps);
                exit;
        end
        else
        { pause game }
        if (KBD_get_button(2) = KBD_PRESSED) or
           (KBD_get_button(3) = KBD_PRESSED) then
        begin
        	MNU_activate(game_pmenu, 0);

                SND_play(SND_BEEP1);

                exit;
        end;

        { reset stage }
        if (KBD_get_button(1) = KBD_PRESSED) then
        begin
                TRN_activate(FADE_IN, 2, cb_reset_stage);
                SND_play(SND_BEEP1);
                exit;
        end;

        { update stage }
        stage_update(game_stage, steps);

        { check if the stage is clear }
        if (game_stage.pl.max_gems > 0) and
	   (game_stage.pl.gems = game_stage.pl.max_gems) then
        begin
        	game_stage_clear  := TRUE;
                game_redraw_clear := TRUE;
                game_clear_timer  := GAME_CLEAR_TIME;

                SND_play(SND_VICTORY);
        end;

        { check if the game is beaten }
        if (game_stage.pl.victory) then
		TRN_activate(FADE_IN, 1, cb_win);
end;

{ draw game }
procedure	game_draw;
begin
	if not game_stage.initialized then exit;

	{ draw stage clear }
	if game_stage_clear then
        begin
        	if game_redraw_clear then
                begin
                        game_draw_stage_clear;
                        game_redraw_clear := FALSE;
                end;
                exit;
        end;

        { draw pause menu }
        if game_pmenu.active then
        begin
        	MNU_draw(game_pmenu,
			 game_stage.top_left.x + game_stage.w * 8,
			 game_stage.top_left.y + game_stage.h * 8);
                exit;
        end;

        { draw stage }
        stage_draw(game_stage);

        { redraw info }
        if game_redraw_HUD then
        begin
        	game_redraw_info(game_stage.pl);
                game_redraw_HUD := FALSE;
        end;
end;

{ dispose main components }
procedure	game_dispose;
begin
	{ free all allocated data }
	stage_destroy(game_stage);
end;

{ change scene }
procedure	game_on_change(param: pointer);
var	path	: string[32];
begin
	path := ASSETS_MAPS + ltoa(uint8(param)) + '.BIN';

        stage_refactor(game_stage);
        if not stage_init(game_stage, path) then exit;

        { set re-render flags }
        game_redraw_HUD   := TRUE;
        game_redraw_clear := FALSE;
        game_stage_clear  := FALSE;
        game_stage_index  := uint8(param);

        { toggle audio and set button text }
        SND_toggle;
        cb_audio;
end;

{ create game scene }
function	game_setup_scene: SCENE;
var	s	: SCENE;
begin
	new(s);
        s^.init    := game_init;
        s^.update  := game_update;
        s^.draw    := game_draw;
        s^.dispose := game_dispose;
        s^.change  := game_on_change;
        s^.name    := SCENE_GAME;
        game_setup_scene := s;
end;

{ draw info }
procedure	game_redraw_info(var pl: PLAYER);
const	TOP_X		= 24 * 8 + 24;
	TOP_Y		= 24;
        FRAME_WIDTH	= 80;
        ITEM_START_Y    = 28;
        ITEM_X		= 2;
        ITEM_OFF_X	= 20;
        ITEM_OFF_Y	= 22;
        BAR_MAX		= 5;
var	i, t	: int8;
	buf	: string[16];
begin
	{ draw stage name }
	VGA_draw_text_fast(game_font, 'STAGE '+ltoa(game_stage_index),
        	TOP_X + FRAME_WIDTH shr 1, TOP_Y + 8, 0, 0, TRUE);

        { draw consumable items }
        VGA_draw_bitmap_region_fast(game_items, 16, 0, 16, 16,
        	TOP_X + ITEM_X, TOP_Y + ITEM_START_Y);
        VGA_draw_bitmap_region_fast(game_items, 32, 0, 16, 16,
        	TOP_X + ITEM_X, TOP_Y + ITEM_START_Y + ITEM_OFF_Y);

        { draw corresponding energy bars }
        game_draw_energy_bar(TOP_X + ITEM_X + ITEM_OFF_X,
		TOP_Y + ITEM_START_Y + 1, BAR_MAX, pl.pickaxe);
        game_draw_energy_bar(TOP_X + ITEM_X + ITEM_OFF_X,
		TOP_Y + ITEM_START_Y + ITEM_OFF_Y + 1, BAR_MAX, pl.shovel);

        { draw collectible items }
	VGA_draw_bitmap_region_fast(game_items, 0, 0, 16, 16,
        	TOP_X + ITEM_X, TOP_Y + ITEM_START_Y + ITEM_OFF_Y * 2);
        VGA_draw_bitmap_region_fast(game_items, 48, 0, 16, 16,
        	TOP_X + ITEM_X, TOP_Y + ITEM_START_Y + ITEM_OFF_Y * 3);

        { draw collectible item counts }
        buf := #2+ltoa(int16(pl.keys));
        VGA_draw_text_fast(game_font, buf,
        	TOP_X + ITEM_X + ITEM_OFF_X,
                TOP_Y + ITEM_START_Y + ITEM_OFF_Y * 2 + 4, 0, 0, FALSE);
	buf := #2+ltoa(int16(pl.bombs));
        VGA_draw_text_fast(game_font, buf,
        	TOP_X + ITEM_X + ITEM_OFF_X,
                TOP_Y + ITEM_START_Y + ITEM_OFF_Y * 3 + 4, 0, 0, FALSE);

        { draw gems }
        for i := 0 to int8(pl.max_gems)-1 do
        begin
        	if i < pl.gems then t := 64 else t := 80;

                VGA_draw_bitmap_region_fast(game_items, t, 0, 16, 16,
			TOP_X + ITEM_X + i * ITEM_OFF_X,
			TOP_Y + ITEM_START_Y + ITEM_OFF_Y * 4)
        end;
end;

{ clear game assets }
procedure	game_clear_assets;
begin
	if game_assets_loaded then
        begin
        	AST_remove('frame');
                AST_remove('tileset');
                AST_remove('anim');
                AST_remove('items');
                AST_remove('player');
                AST_remove('exp');
                AST_remove('ship');
        end;
        game_assets_loaded := FALSE;
end;

{ load assets }
function	game_load_assets: boolean;
begin
        if game_assets_loaded then
	begin
        	game_load_assets := TRUE;
		exit;
        end;

        if not AST_add_bitmap(F_BMP_FRAME    , 'frame'  ) or
           not AST_add_bitmap(F_BMP_TILESET  , 'tileset') or
           not AST_add_bitmap(F_BMP_ANIM     , 'anim'   ) or
           not AST_add_bitmap(F_BMP_ITEMS    , 'items'  ) or
           not AST_add_bitmap(F_BMP_PLAYER   , 'player' ) or
           not AST_add_bitmap(F_BMP_EXPLOSION, 'exp'    ) or
           not AST_add_bitmap(F_BMP_SHIP     , 'ship'   ) then
        begin
        	ERR_throw_no_param('Several game assets are missing.');
                APP_terminate;
                exit;
        end;

        { get bitmaps }
        game_font  := AST_get('font');
        game_items := AST_get('items');

        { pass bitmaps to the game components }
        stage_init_assets(game_stage);
        boulder_init;
        player_init;

        game_load_assets := TRUE;
end;

{$endif}
